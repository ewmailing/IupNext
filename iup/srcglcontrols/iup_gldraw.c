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

void iupGLDrawFrameRect(Ihandle* ih, int xmin, int xmax, int ymin, int ymax, float linewidth, const char* color, int active, int title_x, int title_width, int title_height)
{
  unsigned char r = 0, g = 0, b = 0, a = 255;
  int d = 2;

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

  glBegin(GL_LINE_STRIP);

  ymin -= title_height/2;

  glVertex2i(xmin + title_x + title_width, ymin);

  glVertex2i(xmax - d, ymin);
  glVertex2i(xmax, ymin - d);

  glVertex2i(xmax, ymax + d);
  glVertex2i(xmax - d, ymax);

  glVertex2i(xmin + d, ymax);
  glVertex2i(xmin, ymax + d);

  glVertex2i(xmin, ymin - d);
  glVertex2i(xmin + d, ymin);

  glVertex2i(xmin + title_x, ymin);

  glEnd();
}

void iupGLDrawRect(Ihandle* ih, int xmin, int xmax, int ymin, int ymax, float linewidth, const char* color, int active, int round)
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

  if (round)
  {
    int r = 2;

    glVertex2i(xmin, ymin-r);
    glVertex2i(xmin+r, ymin);

    glVertex2i(xmax-r, ymin);
    glVertex2i(xmax, ymin-r);

    glVertex2i(xmax, ymax+r);
    glVertex2i(xmax-r, ymax);

    glVertex2i(xmin+r, ymax);
    glVertex2i(xmin, ymax+r);
  }
  else
  {
    glVertex2i(xmin, ymin);
    glVertex2i(xmax, ymin);
    glVertex2i(xmax, ymax);
    glVertex2i(xmin, ymax);
  }

  glEnd();
}

void iupGLDrawBox(Ihandle* ih, int xmin, int xmax, int ymin, int ymax, const char* color, int active)
{
  unsigned char r = 0, g = 0, b = 0, a = 255;

  if (!color || xmin == xmax || ymin == ymax)
    return;

  iupStrToRGBA(color, &r, &g, &b, &a);

  /* y is oriented top to bottom in IUP */
  ymin = ih->currentheight - 1 - ymin;
  ymax = ih->currentheight - 1 - ymax;

  if (!active)
    iupGLColorMakeInactive(&r, &g, &b);
  glColor4ub(r, g, b, a);

  glRecti(xmin, ymax, xmax + 1, ymin + 1);
}

void iupGLDrawPolygon(Ihandle* ih, const int* points, int count, const char* color, int active)
{
  unsigned char r = 0, g = 0, b = 0, a = 255;
  int i, x, y;

  if (!color || count < 3)
    return;

  iupStrToRGBA(color, &r, &g, &b, &a);

  if (!active)
    iupGLColorMakeInactive(&r, &g, &b);
  glColor4ub(r, g, b, a);

  glBegin(GL_POLYGON);

  for (i = 0; i < count; i++)
  {
    x = points[2 * i + 0];
    y = points[2 * i + 1];

    /* y is oriented top to bottom in IUP */
    y = ih->currentheight - 1 - y;

    glVertex2i(x, y);
  }

  glEnd();
}

void iupGLDrawPolyline(Ihandle* ih, const int* points, int count, float linewidth, const char* color, int active)
{
  unsigned char r = 0, g = 0, b = 0, a = 255;
  int i, x, y;

  if (!color || count < 3)
    return;

  iupStrToRGBA(color, &r, &g, &b, &a);

  if (!active)
    iupGLColorMakeInactive(&r, &g, &b);
  glColor4ub(r, g, b, a);

  glLineWidth(linewidth);

  glBegin(GL_LINE_LOOP);

  for (i = 0; i < count; i++)
  {
    x = points[2 * i + 0];
    y = points[2 * i + 1];

    /* y is oriented top to bottom in IUP */
    y = ih->currentheight - 1 - y;

    glVertex2i(x, y);
  }

  glEnd();
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

void iupGLDrawArrow(Ihandle *ih, int x, int y, const char* color, int active, int dir, int size, int space)
{
  int points[6];

  /* fix for smooth triangle */
  int delta = (size - 2 * space) / 2;

  switch (dir)
  {
  case IUPGL_ARROW_LEFT:  /* arrow points left */
    x += space;  /* fix center */
    points[0] = x + size - space - delta;
    points[1] = y + space;
    points[2] = x + size - space - delta;
    points[3] = y + size - space;
    points[4] = x + space;
    points[5] = y + size / 2;
    break;
  case IUPGL_ARROW_TOP:    /* arrow points top */
    y += space;  /* fix center */
    points[0] = x + space;
    points[1] = y + size - space - (delta - 1);
    points[2] = x + size - space;
    points[3] = y + size - space - (delta - 1);
    points[4] = x + size / 2;
    points[5] = y + space;
    break;
  case IUPGL_ARROW_RIGHT:  /* arrow points right */
    x += space - 1;  /* fix center */
    y += 1;
    points[0] = x + space;
    points[1] = y + space;
    points[2] = x + space;
    points[3] = y + size - space;
    points[4] = x + size - space - delta;
    points[5] = y + size / 2;
    break;
  case IUPGL_ARROW_BOTTOM:  /* arrow points bottom */
    y += space;  /* fix center */
    points[0] = x + space;
    points[1] = y + space;
    points[2] = x + size - space;
    points[3] = y + space;
    points[4] = x + size / 2;
    points[5] = y + size - space - (delta - 1);
    break;
  }

  iupGLDrawPolygon(ih, points, 3, color, active);
  iupGLDrawPolyline(ih, points, 3, 1, color, active);
}
