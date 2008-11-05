/** \file
 * \brief HSI Color Manipulation
 * Copied and adapted from IM
 *
 * See Copyright Notice in iup.h
 */


#include <math.h>
#include <stdio.h>
#include "iup_colorhsi.h"

static const float rad60 =  1.0471975f;
static const float rad120 = 2.0943951f;
static const float rad180 = 3.1415926f;
static const float rad240 = 4.1887902f;
static const float rad300 = 5.2359877f;
static const float rad360 = 6.2831853f;
static const float sqrt3 = 1.7320508f;
static const float rad2deg = 57.2957795131f;
static float costab[361];
static float sintab[361];
static int init_tab = 0;

static void iColorBuildTables(void)
{
  int theta;
  for (theta=0; theta<=360; theta++)
  {
    float th = ((float)theta)/rad2deg;
    costab[theta] = cosf(th);
    sintab[theta] = sinf(th);
  }
  init_tab = 1;
}

static void iColorSinCos(float H, float *sinH, float *cosH)
{
  int theta;

  H *= rad2deg;
  theta = (int)(H + 0.5f);  /* Round */

  if (!init_tab)
    iColorBuildTables();

  if (theta<0) theta = 0;
  if (theta>360) theta = 360;

  *cosH = costab[theta];
  *sinH = sintab[theta];
}

static float iColorNormHue(float H)
{
  while (H < 0.0f) 
    H += rad360;

  if (H > rad360)  
    H = fmodf(H, rad360);

  return H;
}

static unsigned char iColorQuantize(float value)
{
  if (value >= 1.0f) return 255;
  if (value <= 0.0f) return 0;
  return (unsigned char)(value*256);  
}                               

static float iColorReconstruct(unsigned char value)
{
  if (value <= 0) return 0.0f;
  if (value >= 255) return 1.0f;
  return (((float)value + 0.5f)/256.0f);
}

static void iColorSmax01(float h, float hr, float hb, float hg, float *h0, float *h1)
{
  if (h < rad60)
  {
    *h0 = hb;
    *h1 = hr;
  }
  else if (h < rad120)
  {
    *h0 = hb;
    *h1 = hg;
  }
  else if (h < rad180)
  {
    *h0 = hr;
    *h1 = hg;
  }
  else if (h < rad240)
  {
    *h0 = hr;
    *h1 = hb;
  }
  else if (h < rad300)
  {
    *h0 = hg;
    *h1 = hb;
  }
  else
  {
    *h0 = hg;
    *h1 = hr;
  }
}

static float iColorHSI_Smax(float h, float cosH, float sinH, float i)
{
  float hr, hb, hg, imax, h0, h1;

  if (i == 0.0f || i == 1.0f)
    return 0.0f;

  if (i == 1.0f/3.0f || i == 2.0f/3.0f)
    return 1.0f;

  /* at bottom */
  if (i < 1.0f/3.0f)
  {
    /* face B=0 */
    if (h < rad120)
    {
      hb = (-cosH - sinH*sqrt3)/3.0f;
      return -i/hb;
    }

    /* face R=0 */
    if (h < rad240)
    {
      hr = cosH/1.5f;
      return -i/hr;
    }

    /* face G=0 */
    {
      hg = (-cosH + sinH*sqrt3)/3.0f;
      return -i/hg;
    }
  }

  /* at top */
  if (i > 2.0f/3.0f)
  {
    /* face R=1 */
    if (h < rad60 || h > rad300)
    {
      hr = cosH/1.5f;
      return (1.0f-i)/hr;
    }

    /* face G=1 */
    if (h < rad180)
    {
      hg = (-cosH + sinH*sqrt3)/3.0f;
      return (1.0f-i)/hg;
    }

    /* face B=1 */
    {
      hb = (-cosH - sinH*sqrt3)/3.0f;
      return (1.0f-i)/hb;
    }
  }

  /* in the middle */
  hr = cosH/1.5f;
  hg = (-cosH + sinH*sqrt3)/3.0f;
  hb = (-cosH - sinH*sqrt3)/3.0f;

  iColorSmax01(h, hr, hb, hg, &h0, &h1);

  if (h == 0.0f || h == rad120 || h == rad240)
    imax = 1.0f/3.0f;
  else if (h == rad60 || h == rad180 || h == rad300)
    imax = 2.0f/3.0f;
  else
    imax = h0/(h0 - h1);

  if (i < imax) 
    return -i/h0;
  else
    return (1.0f-i)/h1;
}

static float iColorHSI_ImaxS(float h, float cosH, float sinH)
{
  float i, h0, h1;
  float hr, hb, hg;

  if (h == 0.0f || h == rad120 || h == rad240)
    return 1.0f/3.0f;

  if (h == rad60 || h == rad180 || h == rad300)
    return 2.0f/3.0f;

  hr = cosH/1.5f;
  hg = (-cosH + sinH*sqrt3)/3.0f;
  hb = (-cosH - sinH*sqrt3)/3.0f;

  iColorSmax01(h, hr, hb, hg, &h0, &h1);

  i = h0 / (h0 - h1);

  return i;
}

static void iColorRGB2HSI(float r, float g, float b, float *h, float *s, float *i)
{            
  float v, u, ImaxS; 

  v = r - (g + b)/2.0f;
  u = (g - b) * (sqrt3/2.0f);

  *i = (r + g + b)/3.0f;
  *s = sqrtf(v*v + u*u);
  
  if (*s == 0.0f)
  {
    *h = 360.0f;  /* by definition */
    ImaxS = 1.0f/3.0f;
  }
  else
  {
    float Smax; 
    float H, cosH, sinH;

    H = atan2f(u, v);
    H = iColorNormHue(H);

    iColorSinCos(H, &sinH, &cosH);

    /* must scale S from 0-Smax to 0-1 */
    Smax = iColorHSI_Smax(H, cosH, sinH, *i);
    if (Smax == 0.0f)
      *s = 0.0f;
    else
      *s /= Smax;

    ImaxS = iColorHSI_ImaxS((float)H, cosH, sinH);

    *h = H*rad2deg;
  }

  /* must convert I from linear scale to non-linear scale */
  if (*i < 0.5f) /* half I is I=ImaxS, not I=0.5 */
    *i = ((*i)*0.5f)/ImaxS;
  else
    *i = (((*i) - ImaxS)*0.5f)/(1.0f - ImaxS) + 0.5f;
}

static void iColorHSI2RGB(float h, float s, float i, float *r, float *g, float *b)
{
  float cosH, sinH, H, v, u;
  float Smax, ImaxS; 

  if (i < 0) i = 0;
  else if (i > 1) i = 1;

  H = h/rad2deg;
  H = iColorNormHue(H);

  iColorSinCos(H, &sinH, &cosH);
    
  /* must convert I from non-linear scale to linear scale */
  ImaxS = iColorHSI_ImaxS(H, cosH, sinH);
  if (i < 0.5f) /* half I is I=ImaxS, not I=0.5 */
    i  = ImaxS * (i / 0.5f);
  else
    i = (1.0f - ImaxS) * (i - 0.5f)/0.5f + ImaxS;

  /* must scale S from 0-1 to 0-Smax */
  Smax = iColorHSI_Smax(H, cosH, sinH, i);
  s *= Smax;

  if (s < 0.0f) s = 0.0f;
  else if (s > 1.0f) s = 1.0f;

  if (s == 0.0f || i == 1.0f || i == 0.0f || (int)h == 360)
  {
    *r = i;
    *g = i;
    *b = i;
    return;
  }

  v = s * cosH;
  u = s * sinH * sqrt3;

  *r = i + v/1.5f;
  *g = i - (v - u)/3.0f;
  *b = i - (v + u)/3.0f;

  if (*r < 0.0f) *r = -*r;
  if (*g < 0.0f) *g = -*g;
  if (*b < 0.0f) *b = -*b;

  if (*r > 1.0f) *r = 1.0f;
  if (*g > 1.0f) *g = 1.0f;
  if (*b > 1.0f) *b = 1.0f;
}

/*********************************************************************************************/

void iupColorRGB2HSI(unsigned char r, unsigned char g, unsigned char b, float *h, float *s, float *i)
{
  float fr = iColorReconstruct(r);
  float fg = iColorReconstruct(g);
  float fb = iColorReconstruct(b);
  
  iColorRGB2HSI(fr, fg, fb, h, s, i);
}

void iupColorHSI2RGB(float h, float s, float i, unsigned char *r, unsigned char *g, unsigned char *b)
{
  float fr, fg, fb;
  
  iColorHSI2RGB(h, s, i, &fr, &fg, &fb);
  
  *r = iColorQuantize(fr);
  *g = iColorQuantize(fg);
  *b = iColorQuantize(fb);
}

int iupStrToHSI(const char *str, float *h, float *s, float *i)
{
  float fh, fs, fi;
  if (!str) return 0;
  if (sscanf(str, "%f %f %f", &fh, &fs, &fi) != 3) return 0;
  if (fh > 360 || fs > 1 || fi > 1) return 0;
  if (fh < 0 || fs < 0 || fi < 0) return 0;
  *h = fh;
  *s = fs;
  *i = fi;
  return 1;
}
