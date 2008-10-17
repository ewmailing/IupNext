/** \file
 * \brief iupcdutil. CD and IUP utilities for the IupControls
 *
 * See Copyright Notice in iup.h
 */


#include <stdlib.h>
#include <stdarg.h>

#include <iup.h>
#include <cd.h>
#include <iupcompat.h>
#include "iupcdutil.h"

long cdIupConvertColor(char *color)
{
  unsigned int ri, gi, bi;
  iupGetRGB(color, &ri, &gi, &bi);
  return cdEncodeColor((unsigned char)ri,(unsigned char)gi,(unsigned char)bi);
}

void cdIupCalcShadows(long bgcolor, long *light_shadow, long *mid_shadow, long *dark_shadow)
{
  int r, bg_r = cdRed(bgcolor);
  int g, bg_g = cdGreen(bgcolor);
  int b, bg_b = cdBlue(bgcolor);

  /* light_shadow */

  int max = bg_r;
  if (bg_g > max) max = bg_g;
  if (bg_b > max) max = bg_b;

  if (255-max < 64)
  {
    r = 255;
    g = 255;
    b = 255;
  }
  else
  {
    /* preserve some color information */
    if (bg_r == max) r = 255;
    else             r = bg_r + (255-max);
    if (bg_g == max) g = 255;
    else             g = bg_g + (255-max);
    if (bg_b == max) b = 255;
    else             b = bg_b + (255-max);
  }

  if (light_shadow) *light_shadow = cdEncodeColor((unsigned char)r, (unsigned char)g, (unsigned char)b); 

  /* dark_shadow */

  r = bg_r - 128;
  g = bg_g - 128;
  b = bg_b - 128;
  if (r < 0) r = 0;
  if (g < 0) g = 0;
  if (b < 0) b = 0;

  if (dark_shadow) *dark_shadow = cdEncodeColor((unsigned char)r, (unsigned char)g, (unsigned char)b); 

  /* mid_shadow = (dark_shadow+bgcolor)/2*/

  if (mid_shadow) *mid_shadow = cdEncodeColor((unsigned char)((bg_r+r)/2), (unsigned char)((bg_g+g)/2), (unsigned char)((bg_b+b)/2));
}

void cdIupDrawSunkenRect(cdCanvas *c, int x1, int y1, int x2, int y2, long light_shadow, long mid_shadow, long dark_shadow)
{
  cdCanvasForeground(c, mid_shadow );
  cdCanvasLine( c, x1, y1+1,   x1, y2 );
  cdCanvasLine( c, x1,  y2, x2-1, y2 );

  cdCanvasForeground(c, dark_shadow );
  cdCanvasLine( c, x1+1, y1+2, x1+1, y2-1 );
  cdCanvasLine( c, x1+1, y2-1, x2-2, y2-1 );

  cdCanvasForeground(c, light_shadow );
  cdCanvasLine( c, x1, y1, x2, y1 );
  cdCanvasLine( c, x2, y1, x2, y2 );
}

void cdIupDrawRaisenRect(cdCanvas *c, int x1, int y1, int x2, int y2, long light_shadow, long mid_shadow, long dark_shadow)
{
  cdCanvasForeground(c, light_shadow );
  cdCanvasLine( c, x1, y1+1,   x1, y2 );
  cdCanvasLine( c, x1,  y2, x2-1, y2 );

  cdCanvasForeground(c, dark_shadow );
  cdCanvasLine( c, x1, y1, x2, y1 );
  cdCanvasLine( c, x2, y1, x2, y2 );

  cdCanvasForeground(c, mid_shadow );
  cdCanvasLine( c, x1+1, y1+1, x2-1, y1+1 );
  cdCanvasLine( c, x2-1, y1+2, x2-1, y2-1 );
}

void cdIupDrawVertSunkenMark(cdCanvas *c, int x, int y1, int y2, long light_shadow, long dark_shadow)
{
  cdCanvasForeground(c, dark_shadow );
  cdCanvasLine( c, x-1, y1, x-1, y2 );
  cdCanvasForeground(c, light_shadow );
  cdCanvasLine( c,   x, y1,   x, y2 );
}

void cdIupDrawHorizSunkenMark(cdCanvas *c, int x1, int x2, int y, long light_shadow, long dark_shadow)
{
  cdCanvasForeground(c, dark_shadow );
  cdCanvasLine( c, x1, y+1, x2, y+1 );
  cdCanvasForeground(c, light_shadow );
  cdCanvasLine( c, x1, y, x2, y );
}

#ifdef WIN32
#include <windows.h>
#else
#include <Xm/XmP.h>
#include <Xm/DrawP.h>
#endif

void cdIupDrawFocusRect(Ihandle* self, cdCanvas *c, int x1, int y1, int x2, int y2)
{
  int y, x, w, h;

  cdCanvasUpdateYAxis(c,&y1);
  cdCanvasUpdateYAxis(c,&y2);
  y = y1;
  if (y2<y1) y = y2;
  x = x1;
  if (x2<x1) x = x2;

  w = abs(x2-x1)+1;
  h = abs(y2-y1)+1;

#ifdef WIN32
  {
    HDC hDC = (HDC)cdCanvasGetAttribute(c,"HDC");
    RECT rect;
    (void)self;

    rect.left = x;  
    rect.top = y;  
    rect.right = x+w;  
    rect.bottom = y+h;

    DrawFocusRect(hDC, &rect);
  }
#else
  {
    Display *dpy = (Display *)IupGetAttribute(self,"XDISPLAY");
    Drawable wnd = (Drawable)IupGetAttribute(self,"XWINDOW");
    GC gc = (GC)cdCanvasGetAttribute(c,"GC");

#if (XmVERSION < 2)
    _XmDrawHighlight(dpy, wnd, gc, x, y, w, h, 1, LineOnOffDash);
#else
    XmeDrawHighlight(dpy, wnd, gc, x, y, w, h, 1);
#endif
  }
#endif
}
