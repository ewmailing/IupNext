/** \file
 * \brief GL Drawing functions.
 *
 * See Copyright Notice in "iup.h"
 */

#ifdef WIN32
#include <windows.h>
#endif

#if defined (__APPLE__) || defined (OSX)
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif


#include <stdio.h>
#include <stdlib.h>

#include "iup.h"

#include "iup_assert.h"
#include "iup_object.h"
#include "iup_attrib.h"
#include "iup_str.h"
#include "iup_array.h"
#include "iup_image.h"

#include "iup_glcontrols.h"


void iupGLDrawLine(Ihandle* ih, int x1, int y1, int x2, int y2, float linewidth, const char* color, int active)
{
  unsigned char r = 0, g = 0, b = 0, a = 255;

  if (linewidth == 0)
    return;

  iupStrToRGBA(color, &r, &g, &b, &a);

  /* y is oriented top to bottom in IUP */
  y1 = ih->currentheight - 1 - y1;
  y2 = ih->currentheight - 1 - y2;

  if (!active)
    iupGLColorMakeInactive(&r, &g, &b);
  glColor4ub(r, g, b, a);

  glLineWidth(linewidth);

  glBegin(GL_LINES);
  glVertex2i(x1, y1);
  glVertex2i(x2, y2);
  glEnd();
}

void iupGLDrawRect(Ihandle* ih, int xmin, int xmax, int ymin, int ymax, float linewidth, const char* color, int active)
{
  unsigned char r = 0, g = 0, b = 0, a = 255;

  if (linewidth == 0 || xmin == xmax || ymin == ymax)
    return;

  iupStrToRGBA(color, &r, &g, &b, &a);

  /* y is oriented top to bottom in IUP */
  ymin = ih->currentheight - 1 - ymin;
  ymax = ih->currentheight - 1 - ymax;

  if (!active)
    iupGLColorMakeInactive(&r, &g, &b);
  glColor4ub(r, g, b, a);

  glLineWidth(linewidth);

  glBegin(GL_LINE_LOOP);
  glVertex2i(xmin, ymin);
  glVertex2i(xmax, ymin);
  glVertex2i(xmax, ymax);
  glVertex2i(xmin, ymax);
  glEnd();
}

void iupGLDrawBox(Ihandle* ih, int xmin, int xmax, int ymin, int ymax, const char* color)
{
  unsigned char r = 0, g = 0, b = 0, a = 255;

  if (!color || xmin == xmax || ymin == ymax)
    return;

  iupStrToRGBA(color, &r, &g, &b, &a);

  /* y is oriented top to bottom in IUP */
  ymin = ih->currentheight - 1 - ymin;
  ymax = ih->currentheight - 1 - ymax;

  glColor4ub(r, g, b, a);

  glRecti(xmin, ymax, xmax + 1, ymin + 1);
}

void iupGLDrawText(Ihandle* ih, int x, int y, const char* str, const char* color, int active)
{
  unsigned char r = 0, g = 0, b = 0, a = 255;

  if (!str)
    return;

  iupStrToRGBA(color, &r, &g, &b, &a);

  if (!active)
    iupGLColorMakeInactive(&r, &g, &b);
  glColor4ub(r, g, b, a);

  if (str[0])
  {
    int len, lineheight, ascent, baseline;
    const char *nextstr;
    const char *curstr = str;
    int underline = iupAttribGetInt(ih, "UNDERLINE");

    iupGLFontGetDim(ih, NULL, &lineheight, &ascent, NULL);
    baseline = lineheight - ascent;

    /* y is at top and oriented top to bottom in IUP */
    /* y is at baseline and oriented bottom to top in OpenGL */
    y = y + lineheight - baseline;  /* move to baseline */
    y = ih->currentheight - 1 - y; /* orient bottom to top */

    if (underline)
      glLineWidth(1.0f);

    glPushMatrix();
    glTranslated((double)x, (double)y, 0.0);

    do
    {
      nextstr = iupStrNextLine(curstr, &len);
      if (len)
      {
        iupGLFontRenderString(ih, curstr, len);

        if (underline)
        {
          int width = iupGLFontGetStringWidth(ih, curstr, len);
          glBegin(GL_LINES);
          glVertex2i(0, -2);
          glVertex2i(width-1, -2);
          glEnd();
        }
      }

      glTranslated(0.0, (double)-lineheight, 0.0);

      curstr = nextstr;
    } while (*nextstr);

    glPopMatrix();
  }
}

void iupGLDrawImage(Ihandle* ih, int x, int y, const char* name, int active)
{
  Ihandle* image = iupGLIconGetImageHandle(ih, name, active);
  if (image)
  {
    unsigned char* gldata = iupGLImageGetData(image, active);
    int depth = iupAttribGetInt(image, "GL_DEPTH");
    int format = GL_RGB;
    if (depth == 4)
      format = GL_RGBA;

    /* y is at top and oriented top to bottom in IUP */
    /* y is at bottom and oriented bottom to top in OpenGL */
    y = y + image->currentheight - 1;  /* move to bottom */
    y = ih->currentheight - 1 - y; /* orient bottom to top */

    glRasterPos2i(x, y);
    glDrawPixels(image->currentwidth, image->currentheight, format, GL_UNSIGNED_BYTE, gldata);
  }
}
