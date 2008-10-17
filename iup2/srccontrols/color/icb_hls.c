/** \file
 * \brief iupcb control
 * HLS color model
 *
 * See Copyright Notice in iup.h
 * $Id: icb_hls.c,v 1.1 2008-10-17 06:20:15 scuri Exp $
 */
                  
#include <stdlib.h>                  
#include <stdio.h>
#include <math.h>
#include <string.h>

#include <cd.h>
#include <cdirgb.h>

#include "icb_rgb.h"
#include "icb_hls.h"

/************************************************************\
* Constantes e variaveis globais.                            *
\************************************************************/
static const float pi_180 = 180.0f/3.1415926f;
static const float s60 = 0.8660254f;
static const float c60 = 0.5f;
static uchar shade_r, shade_g, shade_b;
static uchar shade_lr, shade_lg, shade_lb;
static uchar shade_dr, shade_dg, shade_db;

static void rot(float *x1, float *y1, float *x2, float *y2, int t);

/************************************************************\
* Constantes e variaveis globais.                            *
\************************************************************/
#define sq(_) ((_)*(_))
#ifndef max
#define max(a, b)   ((a > b) ? (a) : (b))
#endif
#ifndef min
#define min(a, b)   ((a < b) ? (a) : (b))
#endif

/************************************************************\
* Converte do modelo HLS para o modelo RGB.                  *
\************************************************************/
static void hls2rgbstart(float h, float *pr, float *pg, float *pb)
{
  float r, g, b;
  int i; 

  h = (float) fmod(h, 360.0f);
  if (h < 0.0f)
    h += 360.0f;

  i = (int) (h / 60.0);

  switch (i) {
    case 0:
      r = 1.0f;
      g = h / 60.0f;
      b = 0.0f;
      break;
    case 1:
      r = (120.0f - h) / 60.0f;
      g = 1.0f;
      b = 0.0f;
      break;
    case 2:
      r = 0.0f;
      g = 1.0f;
      b = (h - 120.0f) / 60.0f;
      break;
    case 3:
      r = 0.0f;
      g = (240.0f - h) / 60.0f;
      b = 1.0f;
      break;
    case 4:
      r = (h - 240.0f) / 60.0f;
      g = 0.0f;
      b = 1.0f;
      break;
    case 5:
      r = 1.0f;
      g = 0.0f;
      b = (360.0f - h) / 60.0f;
      break;
    default:
      /* nao devia chegar aqui */
      r = g = b = 0.0f;
      break;
  }

  *pr = r;
  *pg = g;
  *pb = b;
}

static void hls2rgbend(float l, float s, float r, float g, float b, float *pr, float *pg, float *pb)
{
  r = r * s + 0.5f * (1.0f - s); 
  g = g * s + 0.5f * (1.0f - s); 
  b = b * s + 0.5f * (1.0f - s); 

  if (l > 0.5f) {
    l -= 0.5f; l *= 2.0f;
    r = r * (1.0f - l) + l;
    g = g * (1.0f - l) + l;
    b = b * (1.0f - l) + l;
  } else {
    l *= 2.0f;
    r = r * l;
    g = g * l;
    b = b * l;
  }
  
  *pr = r;
  *pg = g;
  *pb = b;
}

/************************************************************\
* Converte do modelo HLS para o modelo RGB.                  *
\************************************************************/
void hls_hls2rgb(float h, float l, float s, float *pr, float *pg, float *pb)
{
  float r, g, b;
  int i; 

  h = (float) fmod(h, 360.0f);
  if (h < 0.0f)
    h += 360.0f;

  i = (int) (h / 60.0);

  switch (i) {
    case 0:
      r = 1.0f;
      g = h / 60.0f;
      b = 0.0f;
      break;
    case 1:
      r = (120.0f - h) / 60.0f;
      g = 1.0f;
      b = 0.0f;
      break;
    case 2:
      r = 0.0f;
      g = 1.0f;
      b = (h - 120.0f) / 60.0f;
      break;
    case 3:
      r = 0.0f;
      g = (240.0f - h) / 60.0f;
      b = 1.0f;
      break;
    case 4:
      r = (h - 240.0f) / 60.0f;
      g = 0.0f;
      b = 1.0f;
      break;
    case 5:
      r = 1.0f;
      g = 0.0f;
      b = (360.0f - h) / 60.0f;
      break;
    default:
      /* nao devia chegar aqui */
      r = g = b = 0.0f;
      break;
  }

  r = r * s + 0.5f * (1.0f - s); 
  g = g * s + 0.5f * (1.0f - s); 
  b = b * s + 0.5f * (1.0f - s); 

  if (l > 0.5f) {
    l -= 0.5f; l *= 2.0f;
    r = r * (1.0f - l) + l;
    g = g * (1.0f - l) + l;
    b = b * (1.0f - l) + l;
  } else {
    l *= 2.0f;
    r = r * l;
    g = g * l;
    b = b * l;
  }
  
  *pr = r;
  *pg = g;
  *pb = b;
}

/************************************************************\
* Converte do modelo RGB para o modelo HLS.                  *
\************************************************************/
void hls_rgb2hls(float r, float g, float b, float *ph, float *pl, float *ps)
{
  float h, l, s;
  float ma, mi, d;

  ma = max(r, max(g, b));
  mi = min(r, min(g, b));
  d = ma - mi;

  l = (ma + mi) / 2.0f;

  /* caso cromatico */
  if (d != 0.0f) {
    s = (l < 0.5f) ? d / (ma + mi) : d / (2.0f - ma - mi);
        
    /* entre magenta e amarelo */
    if (ma == r)
      h = (g - b) / d;
    /* entre amarelo e ciano */
    else if (ma == g)
      h = 2.0f + (b - r) / d;
    /* entre ciano e magenta */
    else 
      h = 4.0f + (r - g) / d;

    h *= 60.0f;
    if (h < 0.0f)  
      h += 360.0f;

  /* caso acromatico */
  } else {
    s = 0.0f;
    /* escolhemos h = 0.0f */
    h = 0.0f;
  }
  
  *ph = h;
  *pl = l;
  *ps = s;
}

/************************************************************\
* Gera um plasma de todos os valores de h em um circulo.     *
\************************************************************/
void hls_hplasma(int iw, int ih, int cl, uchar *ur, uchar *ug, uchar *ub)
{
  int i, j;
  float h;
  float fr, fg, fb;
  int r, g, b;
  long int os;
  int x, y;
  float x1, y1, x2, y2;
  int rd;
  int rs, rb;
  cdCanvas *rgb;
  char buf[200];

  sprintf(buf, "%dx%d %p %p %p", iw, ih, ur, ug, ub);
  rgb = cdCreateCanvas(CD_IMAGERGB, buf);

  /* desenha o plasma */
  os = 0;
  for (i = 0; i < ih; i++) {
    for (j = 0; j < iw; j++) {
      x = iw/2 - j; y = ih/2 - i;
      rd = sq(x) + sq(y);
      rb = sq(iw/2); rs = sq(iw/2 - cl);
      if ((rd < rb) && (rd > rs)) {;
        h = (float) (atan2(i - ih/2, j - iw/2) * pi_180);
        hls_hls2rgb(h, 0.5f, 1.0f, &fr, &fg, &fb);
        r = (int) (fr * 255.0f);
        g = (int) (fg * 255.0f);
        b = (int) (fb * 255.0f);
        ur[os + j] = touchar(r);
        ug[os + j] = touchar(g);
        ub[os + j] = touchar(b);
      } else {
        ur[os + j] = shade_r;
        ug[os + j] = shade_g;
        ub[os + j] = shade_b;
      }
    }
    os += iw;
  }

  /* desenha marcaccoes de 60 em 60 */
  cdCanvasForeground(rgb, cdEncodeColor(shade_dr, shade_dg, shade_db));
  x1 = 0.0f; y1 = ih/2.0f; x2 = 6.0f; y2 = ih/2.0f;
  cdCanvasLine(rgb, (int) x1, (int) y1, (int) x2, (int) y2);
  rot(&x1, &y1, &x2, &y2, ih);
  cdCanvasForeground(rgb, cdEncodeColor(shade_lr, shade_lg, shade_lb));
  cdCanvasLine(rgb, (int) x1, (int) y1, (int) x2, (int) y2);
  rot(&x1, &y1, &x2, &y2, ih);
  cdCanvasForeground(rgb, cdEncodeColor(shade_dr, shade_dg, shade_db));
  cdCanvasLine(rgb, (int) x1, (int) y1, (int) x2, (int) y2);
  rot(&x1, &y1, &x2, &y2, ih);
  cdCanvasForeground(rgb, cdEncodeColor(shade_lr, shade_lg, shade_lb));
  cdCanvasLine(rgb, (int) x1, (int) y1, (int) x2, (int) y2);
  rot(&x1, &y1, &x2, &y2, ih);
  cdCanvasForeground(rgb, cdEncodeColor(shade_dr, shade_dg, shade_db));
  cdCanvasLine(rgb, (int) x1, (int) y1, (int) x2, (int) y2);
  rot(&x1, &y1, &x2, &y2, ih);
  cdCanvasLine(rgb, (int) x1, (int) y1, (int) x2, (int) y2);

  /* desenha a borda */
  cdCanvasForeground(rgb, cdEncodeColor(shade_dr, shade_dg, shade_db));
  cdCanvasArc(rgb, iw/2, iw/2, iw, iw, 45.0, 225.0);
  cdCanvasForeground(rgb, cdEncodeColor(shade_lr, shade_lg, shade_lb));
  cdCanvasArc(rgb, iw/2, iw/2, iw, iw, 225.0, 45.0);

  cdCanvasForeground(rgb, cdEncodeColor(shade_lr, shade_lg, shade_lb));
  cdCanvasArc(rgb, iw/2, iw/2, iw-2*cl, iw-2*cl, 45.0, 225.0);
  cdCanvasForeground(rgb, cdEncodeColor(shade_dr, shade_dg, shade_db));
  cdCanvasArc(rgb, iw/2, iw/2, iw-2*cl, iw-2*cl, 225.0, 45.0);

  cdKillCanvas(rgb);
}

/************************************************************\
* Gera um plasma de todos os valores de l e s para um valor  *
* de h fixo. Num triangulo.                                  *
\************************************************************/
#define in(x, y) (((iw - x) > (2*y - ih)) && ((x-iw) < (2*y - ih)))
void hls_lsplasma(int iw, int ih, float h, uchar *ur, uchar *ug, uchar *ub)
{
  float fr, fg, fb;
  int r, g, b;
  float tr, tg, tb;
  int x, y;
  float l, s, dl, ds;
  long int os;

  hls2rgbstart(h, &tr, &tg, &tb);

  /* desenha o triangulo */
  dl = 1.0f/(ih - 1);
  l = 0.0f;
  os = 0;
  for (y = 0; y < ih; y++) {
    ds = (float)(iw - abs(2*y - ih));
    if (ds != 0.0f) ds = 1.0f / ds;
    else ds = 1.0f;
    s = 0.0f;
    for (x = 0; x < iw; x++) {
      if (in(x, y)) {
        hls2rgbend(l, s, tr, tg, tb, &fr, &fg, &fb);
        r = (int) (fr * 255.0f);
        g = (int) (fg * 255.0f);
        b = (int) (fb * 255.0f);
        ur[os+x] = touchar(r);
        ug[os+x] = touchar(g);
        ub[os+x] = touchar(b);
      }
      s += ds;
    }
    l += dl;
    os += iw;
  }

  /* desenha a borda */
  os = 0;
  for (y = 0; y < ih/2; y++) {
    x = 2*y;
    ur[os+x] = shade_lr; ug[os+x] = shade_lr; ub[os+x] = shade_lr;
    x++;
    ur[os+x] = shade_lr; ug[os+x] = shade_lr; ub[os+x] = shade_lr;
    os += iw;
  }
  for (y++, os+=iw; y < ih; y++) {
    x = 2*(ih - y);
    ur[os+x] = shade_dr; ug[os+x] = shade_dr; ub[os+x] = shade_dr;
    x--;
    ur[os+x] = shade_dr; ug[os+x] = shade_dr; ub[os+x] = shade_dr;
    os += iw;
  }

  os = 0;
  for (y = 0; y < ih; y++) {
    ur[os] = shade_dr; ug[os] = shade_dg; ub[os] = shade_db;
    os += iw;
  }
}

/************************************************************\
* Gera um plasma de todos os valores de s para um h e um l   *
* fixos.                                                     *
\************************************************************/
void hls_splasma(int iw, int ih, float h, float l, uchar *ur, 
uchar *ug, uchar *ub)
{
  float inv_w;
  int x, y;
  float s;
  float fr, fg, fb;
  int r, g, b;
  long int os;

  /* cria uma linha de plasma */
  inv_w = 1.0f/((float)(iw));
  s = 0.0f;
  for (x = 0; x < iw; x++) {
    hls_hls2rgb(h, l, s, &fr, &fg, &fb);
    r = (int) (fr * 255.0f);
    g = (int) (fg * 255.0f);
    b = (int) (fb * 255.0f);
    ur[x] = touchar(r);
    ug[x] = touchar(g);
    ub[x] = touchar(b);
    s += inv_w;
  }
    
  /* duplica nas outras */
  os = 0;
  for (x = 1; x < ih; x++) {
    os += iw;
    memcpy(ur + os, ur, iw);
    memcpy(ug + os, ug, iw);
    memcpy(ub + os, ub, iw);
  }

  /* desenha a borda */
  os = (ih-1) * (iw);
  for (x = 0; x < iw; x++) {
    ur[x] = shade_lr; ug[x] = shade_lg; ub[x] = shade_lb;
    ur[x+os] = shade_dr; ug[x+os] = shade_dg; ub[x+os] = shade_db; 
  }

  os = 0;
  for (y = 0; y < ih; y++) {
    ur[os] = shade_dr; ug[os] = shade_dg; ub[os] = shade_db;
    ur[iw+os-1] = shade_lr; ug[iw+os-1] = shade_lg; ub[iw+os-1] = shade_lb; 
    os += iw;
  }

  ur[0] = shade_lr; ug[0] = shade_lg; ub[0] = shade_lb;
}

/************************************************************\
* Gera um plasma de todos os valores de l para um h e um s   *
* fixos.                                                     *
\************************************************************/
void hls_lplasma(int iw, int ih, float h, float s, uchar *ur, 
uchar *ug, uchar *ub)
{
  float inv_h;
  int y;
  float l;
  float fr, fg, fb;
  int r, g, b;
  long int os;

  inv_h = 1.0f/((float)(ih));
  l = 0.0f;
  os = 0;
  for (y = 0; y < ih; y++) {
    hls_hls2rgb(h, l, s, &fr, &fg, &fb);
    r = (int) (fr * 255.0f);
    g = (int) (fg * 255.0f);
    b = (int) (fb * 255.0f);
    memset(ur + os, touchar(r), iw);
    memset(ug + os, touchar(g), iw);
    memset(ub + os, touchar(b), iw);
    ur[os] = shade_dr; ug[os] = shade_dg; ub[os] = shade_db;
    ur[os+iw-1] = shade_lr; ug[os+iw-1] = shade_lg; ub[os+iw-1] = shade_lb;
    l += inv_h;
    os += iw;
  }

  os = iw * (ih-1);
  memset(ur, shade_lr, iw);
  memset(ur+os, shade_dr, iw);
  memset(ug, shade_lg, iw);
  memset(ug+os, shade_dg, iw);
  memset(ub, shade_lb, iw);
  memset(ub+os, shade_db, iw);
}

/************************************************************\
* Configura as cores de sombra e fundo do usuario.           *
\************************************************************/
void hls_shade(int r, int g, int b, int lr, int lg, int lb, int dr, int dg, int db)
{
  shade_r = r; shade_g = g; shade_b = b;
  shade_lr = lr; shade_lg = lg; shade_lb = lb;
  shade_dr = dr; shade_dg = dg; shade_db = db;
}

/************************************************************\
* Roda os pontos de 60 graus.                                *
\************************************************************/
static void rot(float *x1, float *y1, float *x2, float *y2, int t)
{
  float xt, yt;
  float nxt, nyt;

  xt = *x1 - t/2.0f; 
  yt = *y1 - t/2.0f;
  nxt = xt * c60 - yt * s60;
  nyt = xt * s60 + yt * c60;
  *x1 = nxt + t/2.0f; 
  *y1 = nyt + t/2.0f;

  xt = *x2 - t/2.0f; 
  yt = *y2 - t/2.0f;
  nxt = xt * c60 - yt * s60;
  nyt = xt * s60 + yt * c60;
  *x2 = nxt + t/2.0f;
  *y2 = nyt + t/2.0f;
}
