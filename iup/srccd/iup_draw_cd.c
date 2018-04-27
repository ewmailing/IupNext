/** \file
 * \brief IupDraw driver
 *
 * See Copyright Notice in cd.h
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <cd.h>
#include <cd_private.h>

#include "iup.h"
#include "iupdraw_cd.h"

#include "iup_drvdraw.h"
#include "iup_drvfont.h"


struct _cdCtxCanvas
{
  cdCanvas* canvas;
  Ihandle* ih;
  IdrawCanvas* dc;
};

static char *getFontName(cdCtxCanvas *ctxcanvas)
{
  int is_italic = 0, is_bold = 0;   /* default is CD_PLAIN */
  int is_strikeout = 0, is_underline = 0;
  static char font[1024];

  strcpy(font, ctxcanvas->canvas->native_font);

  if (font[0] == 0)
  {
    if (ctxcanvas->canvas->font_style & CD_BOLD)
      is_bold = 1;

    if (ctxcanvas->canvas->font_style & CD_ITALIC)
      is_italic = 1;

    if (ctxcanvas->canvas->font_style & CD_UNDERLINE)
      is_underline = 1;

    if (ctxcanvas->canvas->font_style & CD_STRIKEOUT)
      is_strikeout = 1;

    sprintf(font, "%s, %s%s%d", ctxcanvas->canvas->font_type_face, is_bold ? "Bold " : "", is_italic ? "Italic " : "", ctxcanvas->canvas->font_size);
  }

  return font;
}

static void cdkillcanvas(cdCtxCanvas *ctxcanvas)
{
  memset(ctxcanvas, 0, sizeof(cdCtxCanvas));
  free(ctxcanvas);
}

static int cdactivate(cdCtxCanvas *ctxcanvas)
{
  int w, h;

  ctxcanvas->canvas->bpp = IupGetInt(NULL, "SCREENDEPTH");
  ctxcanvas->canvas->xres = IupGetDouble(NULL, "SCREENDPI") / 25.4;
  ctxcanvas->canvas->yres = ctxcanvas->canvas->xres;

  if (ctxcanvas->dc)
    iupdrvDrawKillCanvas(ctxcanvas->dc);

  ctxcanvas->dc = iupdrvDrawCreateCanvas(ctxcanvas->ih);

  iupdrvDrawGetSize(ctxcanvas->dc, &w, &h);

  ctxcanvas->canvas->w = w;
  ctxcanvas->canvas->h = h;
  ctxcanvas->canvas->w_mm = ((double)w) / ctxcanvas->canvas->xres;
  ctxcanvas->canvas->h_mm = ((double)h) / ctxcanvas->canvas->yres;
  ctxcanvas->canvas->invert_yaxis = 1;

  return CD_OK;
}

static void cddeactivate(cdCtxCanvas *ctxcanvas)
{
  if (ctxcanvas->dc)
    iupdrvDrawKillCanvas(ctxcanvas->dc);
  ctxcanvas->dc = NULL;
}

static void cdflush(cdCtxCanvas *ctxcanvas)
{
  if (ctxcanvas->dc)
    iupdrvDrawFlush(ctxcanvas->dc);
}

static void cdclear(cdCtxCanvas* ctxcanvas)
{
  if (ctxcanvas->dc)
  {
    int w, h;
    iupdrvDrawGetSize(ctxcanvas->dc, &w, &h);
    iupdrvDrawRectangle(ctxcanvas->dc, 0, 0, w - 1, h - 1, cdCanvasBackground(ctxcanvas->canvas, CD_QUERY), CD_FILL, 1);
  }
}

static int cdfont(cdCtxCanvas *ctxcanvas, const char *type_face, int style, int size)
{
  /* dummy - must be defined */
  (void)ctxcanvas;
  (void)type_face;
  (void)style;
  (void)size;
  return 1;
}

static void cdgetfontdim(cdCtxCanvas* ctxcanvas, int *max_width, int *line_height, int *ascent, int *descent)
{
  iupdrvFontGetFontDim(getFontName(ctxcanvas), max_width, line_height, ascent, descent);
}

static void cdgettextsize(cdCtxCanvas* ctxcanvas, const char *s, int len, int *width, int *height)
{
  (void)len;
  iupdrvFontGetTextSize(getFontName(ctxcanvas), s, width, height);
}


/******************************************************/
/* primitives                                         */
/******************************************************/


static void cdline(cdCtxCanvas *ctxcanvas, int px1, int py1, int px2, int py2)
{
  long color = cdCanvasForeground(ctxcanvas->canvas, CD_QUERY);
  int lineStyle = cdCanvasLineStyle(ctxcanvas->canvas, CD_QUERY);
  int lineWidth = cdCanvasLineWidth(ctxcanvas->canvas, CD_QUERY);
  int drvStyle;
  if (lineStyle == CD_DASHED)
    drvStyle = IUP_DRAW_STROKE_DASH;
  else if (lineStyle == CD_DOTTED)
    drvStyle = IUP_DRAW_STROKE_DOT;
  else
    drvStyle = IUP_DRAW_STROKE;
  if (ctxcanvas->dc)
    iupdrvDrawLine(ctxcanvas->dc, px1, py1, px2, py2, color, drvStyle, lineWidth);
}

static void cdrect(cdCtxCanvas *ctxcanvas, int xmin, int xmax, int ymin, int ymax)
{
  long color = cdCanvasForeground(ctxcanvas->canvas, CD_QUERY);
  int lineStyle = cdCanvasLineStyle(ctxcanvas->canvas, CD_QUERY);
  int lineWidth = cdCanvasLineWidth(ctxcanvas->canvas, CD_QUERY);
  int drvStyle;
  if (lineStyle == CD_DASHED)
    drvStyle = IUP_DRAW_STROKE_DASH;
  else if (lineStyle == CD_DOTTED)
    drvStyle = IUP_DRAW_STROKE_DOT;
  else
    drvStyle = IUP_DRAW_STROKE;
  if (ctxcanvas->dc)
    iupdrvDrawRectangle(ctxcanvas->dc, xmin, ymin, xmax, ymax, color, drvStyle, lineWidth);
}

static void cdbox(cdCtxCanvas *ctxcanvas, int xmin, int xmax, int ymin, int ymax)
{
  long color = cdCanvasForeground(ctxcanvas->canvas, CD_QUERY);
  int interiorStyle = cdCanvasInteriorStyle(ctxcanvas->canvas, CD_QUERY);
  int style = IUP_DRAW_FILL;
  int lineWidth = cdCanvasLineWidth(ctxcanvas->canvas, CD_QUERY);

  if (interiorStyle != CD_SOLID)
    style = IUP_DRAW_STROKE;

  if (ctxcanvas->dc)
    iupdrvDrawRectangle(ctxcanvas->dc, xmin, ymin, xmax, ymax, color, style, lineWidth);
}

static void arc(double xc, double yc, double w, double h, double a1, double a2,
                double *center, double *first_end_point,
                double *second_end_point, double *dx_start, double *dy_start,
                double *dx_end, double *dy_end)
{
  double width, height;

  center[0] = xc;
  center[1] = yc;

  width = w / 2;
  height = h / 2;

  first_end_point[0] = center[0] + width;
  first_end_point[1] = center[1];

  second_end_point[0] = center[0];
  second_end_point[1] = center[1] + height;

  *dx_start = width*cos(a1*CD_DEG2RAD);
  *dy_start = height*sin(a1*CD_DEG2RAD);

  *dx_end = width*cos(a2*CD_DEG2RAD);
  *dy_end = height*sin(a2*CD_DEG2RAD);
}

static void cdarc(cdCtxCanvas *ctxcanvas, int xc, int yc, int w, int h, double a1, double a2)
{
  int rx = w / 2;
  int ry = h / 2;
  int x1 = xc - rx;
  int y1 = yc - ry;
  int x2 = xc + rx;
  int y2 = yc + ry;
  long color = cdCanvasForeground(ctxcanvas->canvas, CD_QUERY);
  int lineStyle = cdCanvasLineStyle(ctxcanvas->canvas, CD_QUERY);
  int lineWidth = cdCanvasLineWidth(ctxcanvas->canvas, CD_QUERY);
  int drvStyle;
  if (lineStyle == CD_DASHED)
    drvStyle = IUP_DRAW_STROKE_DASH;
  else if (lineStyle == CD_DOTTED)
    drvStyle = IUP_DRAW_STROKE_DOT;
  else
    drvStyle = IUP_DRAW_STROKE;
  if (ctxcanvas->dc)
    iupdrvDrawArc(ctxcanvas->dc, x1, y1, x2, y2, a1, a2, color, drvStyle, lineWidth);
}

static void cdsector(cdCtxCanvas *ctxcanvas, int xc, int yc, int w, int h, double a1, double a2)
{
  int rx = w / 2;
  int ry = h / 2;
  int x1 = xc - rx;
  int y1 = yc - ry;
  int x2 = xc + rx;
  int y2 = yc + ry;
  long color = cdCanvasForeground(ctxcanvas->canvas, CD_QUERY);
  int lineWidth = cdCanvasLineWidth(ctxcanvas->canvas, CD_QUERY);
  if (ctxcanvas->dc)
    iupdrvDrawArc(ctxcanvas->dc, x1, y1, x2, y2, a1, a2, color, IUP_DRAW_FILL, lineWidth);
}

static void cdchord(cdCtxCanvas *ctxcanvas, int xc, int yc, int w, int h, double a1, double a2)
{
  /* can NOT be NULL, but it is not supported */
  (void)ctxcanvas;
  (void)xc;
  (void)yc;
  (void)w;
  (void)h;
  (void)a1;
  (void)a2;
}

static void cdtext(cdCtxCanvas *ctxcanvas, int x, int y, const char *s, int len)
{
  int w, h, xmin, xmax, ymin, ymax;
  long color = cdCanvasForeground(ctxcanvas->canvas, CD_QUERY);

  cdCanvasGetTextBox(ctxcanvas->canvas, x, y, s, &xmin, &xmax, &ymin, &ymax);

  w = xmax - xmin;
  h = ymax - ymin;

  if (ctxcanvas->dc)
    iupdrvDrawText(ctxcanvas->dc, s, len, x, y, w, h, color, getFontName(ctxcanvas), 1);
}

static void cdpoly(cdCtxCanvas *ctxcanvas, int mode, cdPoint* poly, int n)
{
  int *points;
  int i, j;
  int style;
  int nVertex = n;
  long color = cdCanvasForeground(ctxcanvas->canvas, CD_QUERY);
  int lineStyle = cdCanvasLineStyle(ctxcanvas->canvas, CD_QUERY);
  int lineWidth = cdCanvasLineWidth(ctxcanvas->canvas, CD_QUERY);
  int interiorStyle = cdCanvasInteriorStyle(ctxcanvas->canvas, CD_QUERY);
  int drvStyle;
  if (lineStyle == CD_DASHED)
    drvStyle = IUP_DRAW_STROKE_DASH;
  else if (lineStyle == CD_DOTTED)
    drvStyle = IUP_DRAW_STROKE_DOT;
  else
    drvStyle = IUP_DRAW_STROKE;

  if (mode == CD_CLOSED_LINES || interiorStyle != CD_SOLID)
    nVertex++;

  points = (int*)malloc(nVertex * 2 * sizeof(int));

  for (i = 0, j = 0; i < n; i++)
  {
    points[j] = poly[i].x;
    points[j + 1] = poly[i].y;
    j = j + 2;
  }

  if (mode == CD_CLOSED_LINES || interiorStyle != CD_SOLID)
  {
    points[j] = poly[0].x;
    points[j+1] = poly[0].y;
  }

  if (mode == CD_FILL && interiorStyle == CD_SOLID)
    style = IUP_DRAW_FILL;
  else
    style = drvStyle;

  if (ctxcanvas->dc)
    iupdrvDrawPolygon(ctxcanvas->dc, points, nVertex, color, style, lineWidth);

  free(points);
}


/******************************************************/
/* client images                                      */
/******************************************************/


static void sFixImageY(cdCanvas* canvas, int *topdown, int *y, int h)
{
  if (canvas->invert_yaxis)
    *topdown = 0;
  else
    *topdown = 1;

  if (!(*topdown))
    *y -= (h - 1);  /* move Y to top-left corner, since it was at the bottom of the image */
}

static void cdputimagerectrgba(cdCtxCanvas* ctxcanvas, int width, int height, const unsigned char *red,
                               const unsigned char *green, const unsigned char *blue, const unsigned char *alpha, int x, int y, int w, int h, int xmin, int xmax, int ymin, int ymax)
{
  int i, j, count;
  Ihandle *image;
  int topdown, pos;
  unsigned char *rgba;
  int dy = y;

  sFixImageY(ctxcanvas->canvas, &topdown, &dy, height);

  image = IupImageRGBA(width, height, NULL);
  IupSetHandle("_IUPDRAW_CD_IMAGE", image);
  rgba = (unsigned char*)IupGetAttribute(ctxcanvas->ih, "WID");

  count = 0;
  for (i = ymin; i <= ymax; i++)
  {
    for (j = xmin; j <= xmax; j++)
    {
      if (topdown)
        pos = i*width + j;
      else
        pos = (ymax + ymin - i)*width + j;

      rgba[count++] = red[pos];
      rgba[count++] = green[pos];
      rgba[count++] = blue[pos];
      rgba[count++] = alpha[pos];
    }
  }

  if (ctxcanvas->dc)
    iupdrvDrawImage(ctxcanvas->dc, "_IUPDRAW_CD_IMAGE", 0, NULL, x, dy);
  IupDestroy(image);

  /* zoom is not supported */
  (void)w;
  (void)h;
}

static void cdputimagerectrgb(cdCtxCanvas* ctxcanvas, int width, int height, const unsigned char *red,
                              const unsigned char *green, const unsigned char *blue, int x, int y, int w, int h, int xmin, int xmax, int ymin, int ymax)
{
  int i, j, count;
  Ihandle *image;
  int topdown, pos;
  unsigned char *rgb;
  int dy = y;

  sFixImageY(ctxcanvas->canvas, &topdown, &dy, height);

  image = IupImageRGB(width, height, NULL);
  IupSetHandle("_IUPDRAW_CD_IMAGE", image);
  rgb = (unsigned char*)IupGetAttribute(ctxcanvas->ih, "WID");

  count = 0;
  for (i = ymin; i <= ymax; i++)
  {
    for (j = xmin; j <= xmax; j++)
    {
      if (topdown)
        pos = i*width + j;
      else
        pos = (ymax + ymin - i)*width + j;

      rgb[count++] = red[pos];
      rgb[count++] = green[pos];
      rgb[count++] = blue[pos];
    }
  }

  if (ctxcanvas->dc)
    iupdrvDrawImage(ctxcanvas->dc, "_IUPDRAW_CD_IMAGE", 0, NULL, x, dy);

  IupDestroy(image);

  /* zoom is not supported */
  (void)w;
  (void)h;
}

static void cdputimagerectmap(cdCtxCanvas *ctxcanvas, int iw, int ih, const unsigned char *index, const long *colors,
                              int x, int y, int w, int h, int xmin, int xmax, int ymin, int ymax)
{
  int i, j, count;
  int topdown, pos;
  Ihandle *image;
  unsigned char *map;
  int dy = y;
  char name[60];

  sFixImageY(ctxcanvas->canvas, &topdown, &dy, ih);

  image = IupImage(iw, ih, NULL);
  IupSetHandle("_IUPDRAW_CD_IMAGE", image);
  map = (unsigned char*)IupGetAttribute(ctxcanvas->ih, "WID");

  for (i = 0; i < 256; i++)
  {
    sprintf(name, "%d", i);
    IupSetRGB(image, name, cdRed(colors[i]), cdGreen(colors[i]), cdBlue(colors[i]));
  }

  count = 0;
  for (i = ymin; i <= ymax; i++)
  {
    for (j = xmin; j <= xmax; j++)
    {
      if (topdown)
        pos = i*iw + j;
      else
        pos = (ymax + ymin - i)*iw + j;

      map[count++] = index[pos];
    }
  }

  if (ctxcanvas->dc)
    iupdrvDrawImage(ctxcanvas->dc, "_IUPDRAW_CD_IMAGE", 0, NULL, x, dy);

  IupDestroy(image);

  /* zoom is not supported */
  (void)w;
  (void)h;
}

/******************************************************/
/* server images                                      */
/******************************************************/


static void cdpixel(cdCtxCanvas *ctxcanvas, int x, int y, long color)
{
  if (ctxcanvas->dc)
    iupdrvDrawArc(ctxcanvas->dc, x, y, x, y, 0.0, 360.0, color, CD_FILL, 1);
}

static void cdcreatecanvas(cdCanvas* canvas, Ihandle *ih)
{
  cdCtxCanvas *ctxcanvas;

  ctxcanvas = (cdCtxCanvas *)malloc(sizeof(cdCtxCanvas));
  memset(ctxcanvas, 0, sizeof(cdCtxCanvas));

  ctxcanvas->canvas = canvas;
  ctxcanvas->ih = ih;
  canvas->ctxcanvas = ctxcanvas;

  ctxcanvas->canvas->bpp = IupGetInt(NULL, "SCREENDEPTH");
  ctxcanvas->canvas->xres = IupGetDouble(NULL, "SCREENDPI") / 25.4;
  ctxcanvas->canvas->yres = ctxcanvas->canvas->xres;
}

static void cdinittable(cdCanvas* canvas)
{
  /* initialize function table*/
  canvas->cxFlush = cdflush;
  canvas->cxClear = cdclear;
  canvas->cxPixel = cdpixel;
  canvas->cxLine = cdline;
  canvas->cxPoly = cdpoly;
  canvas->cxRect = cdrect;
  canvas->cxBox = cdbox;
  canvas->cxArc = cdarc;
  canvas->cxSector = cdsector;
  canvas->cxChord = cdchord;
  canvas->cxText = cdtext;
  canvas->cxPutImageRectRGBA = cdputimagerectrgba;
  canvas->cxPutImageRectRGB = cdputimagerectrgb;
  canvas->cxPutImageRectMap = cdputimagerectmap;
  canvas->cxFont = cdfont;

  canvas->cxGetFontDim = cdgetfontdim;
  canvas->cxGetTextSize = cdgettextsize;

  canvas->cxKillCanvas = cdkillcanvas;
  canvas->cxDeactivate = cddeactivate;
  canvas->cxActivate = cdactivate;
}

/******************************************************/

static cdContext cdIupDrawContext =
{
  CD_CAP_ALL & ~(CD_CAP_PLAY | CD_CAP_YAXIS | CD_CAP_FPRIMTIVES),
  CD_CTX_WINDOW,
  cdcreatecanvas,
  cdinittable,
  NULL,
  NULL,
};

cdContext* cdContextIupDraw(void)
{
  return &cdIupDrawContext;
}

