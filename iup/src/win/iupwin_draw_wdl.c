/** \file
* \brief Draw Functions for DirectD2 using WinDrawLib
*
* See Copyright Notice in "iup.h"
*/

#include <windows.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <math.h>

#include "iup.h"

#include "iup_attrib.h"
#include "iup_class.h"
#include "iup_image.h"
#include "iup_drvdraw.h"
#include "iup_draw.h"
#include "iup_str.h"

#include "iupwin_drv.h"
#include "iupwin_info.h"
#include "iupwin_draw.h"
#include "iupwin_str.h"

#include "wdl.h"


struct _IdrawCanvas{
  Ihandle* ih;
  int w, h;

  HWND hWnd;
  WD_HCANVAS hCanvas;
};

/* must be the same in wdInitialize and wdTerminate */
const DWORD wdl_flags = WD_INIT_COREAPI | WD_INIT_IMAGEAPI | WD_INIT_STRINGAPI;

void iupwinDrawInit(void)
{
  iupwinDrawThemeInit();

  /* uncomment to force GDI+ always */
#if 0
  wdPreInitialize(NULL, NULL, WD_DISABLE_D2D);
#endif

  wdInitialize(wdl_flags);
}

void iupwinDrawFinish(void)
{
  wdTerminate(wdl_flags);
}

IdrawCanvas* iupdrvDrawCreateCanvas(Ihandle* ih)
{
  IdrawCanvas* dc = calloc(1, sizeof(IdrawCanvas));
  PAINTSTRUCT ps;
  RECT rect;
  int x, y, w, h;
  char *rcPaint;

  dc->ih = ih;

  dc->hWnd = (HWND)IupGetAttribute(ih, "HWND");  /* Use the attribute, so it can work with FileDlg preview area */

  GetClientRect(dc->hWnd, &rect);
  dc->w = rect.right - rect.left;
  dc->h = rect.bottom - rect.top;

  ps.hdc = (HDC)iupAttribGet(ih, "HDC_WMPAINT");
  rcPaint = iupAttribGetStr(ih, "CLIPRECT");
  sscanf(rcPaint, "%d %d %d %d", &x, &y, &w, &h);
  ps.rcPaint.left = x;
  ps.rcPaint.top = y;
  ps.rcPaint.right = x + w;
  ps.rcPaint.bottom = y + h;

  dc->hCanvas = wdCreateCanvasWithPaintStruct(dc->hWnd, &ps, WD_CANVAS_DOUBLEBUFFER);

  if (wdBackend() == WD_BACKEND_D2D)
    iupAttribSet(ih, "DRAWDRIVER", "D2D");
  else
    iupAttribSet(ih, "DRAWDRIVER", "GDI+");

  wdBeginPaint(dc->hCanvas);

  return dc;
}

void iupdrvDrawKillCanvas(IdrawCanvas* dc)
{
  wdDestroyCanvas(dc->hCanvas);
  free(dc);
}

void iupdrvDrawUpdateSize(IdrawCanvas* dc)
{
  int w, h;
  RECT rect;

  GetClientRect(dc->hWnd, &rect);
  w = rect.right - rect.left;
  h = rect.bottom - rect.top;

  if (w != dc->w || h != dc->h)
  {
    dc->w = w;
    dc->h = h;

    wdResizeCanvas(dc->hCanvas, w, h);
  }
}

void iupdrvDrawFlush(IdrawCanvas* dc)
{
  wdEndPaint(dc->hCanvas);
}

void iupdrvDrawGetSize(IdrawCanvas* dc, int *w, int *h)
{
  if (w) *w = dc->w;
  if (h) *h = dc->h;
}

static WD_HSTROKESTYLE iDrawSetLineStyle(int style)
{
  if (style == IUP_DRAW_STROKE_DASH)
  {
    float dashes[2] = { 12.0f, 4.0f };
    return wdCreateStrokeStyleCustom(dashes, 2, WD_LINECAP_FLAT, WD_LINEJOIN_MITER);
  }
  else if (style == IUP_DRAW_STROKE_DOT)
  {
    float dashes[2] = { 2.0f, 2.0f };
    return wdCreateStrokeStyleCustom(dashes, 2, WD_LINECAP_FLAT, WD_LINEJOIN_MITER);
  }
  else
    return NULL;
}

static void iDrawRelaseStyle(WD_HSTROKESTYLE stroke_style)
{
  if (stroke_style)
    wdDestroyStrokeStyle(stroke_style);
}

#define iupInt2Float(_x) ((float)_x)
#define iupInt2FloatW(_x) ((float)_x)
#define iupColor2ARGB(_c) WD_ARGB(iupDrawAlpha(_c), iupDrawRed(_c), iupDrawGreen(_c), iupDrawBlue(_c))

void iupdrvDrawRectangle(IdrawCanvas* dc, int x1, int y1, int x2, int y2, long color, int style, int line_width)
{
  WD_HBRUSH brush = wdCreateSolidBrush(dc->hCanvas, iupColor2ARGB(color));

  iupDrawCheckSwapCoord(x1, x2);
  iupDrawCheckSwapCoord(y1, y2);

  if (style == IUP_DRAW_FILL)
    wdFillRect(dc->hCanvas, brush, iupInt2Float(x1 - 0.5f), iupInt2Float(y1 - 0.5f), iupInt2Float(x2 + 0.5f), iupInt2Float(y2 + 0.5f));
  else
  {
    WD_HSTROKESTYLE stroke_style = iDrawSetLineStyle(style);
    wdDrawRectStyled(dc->hCanvas, brush, iupInt2Float(x1), iupInt2Float(y1), iupInt2Float(x2), iupInt2Float(y2), iupInt2FloatW(line_width), stroke_style);
    iDrawRelaseStyle(stroke_style);
  }

  wdDestroyBrush(brush);
}

void iupdrvDrawLine(IdrawCanvas* dc, int x1, int y1, int x2, int y2, long color, int style, int line_width)
{
  WD_HBRUSH brush = wdCreateSolidBrush(dc->hCanvas, iupColor2ARGB(color));
  WD_HSTROKESTYLE stroke_style = iDrawSetLineStyle(style);
  if (wdBackend() == WD_BACKEND_D2D && line_width == 1)
  {
    /* compensate Direct2D horizontal and vertical lines when line_width == 1 */
    if (x1 == x2)
      wdDrawLineStyled(dc->hCanvas, brush, iupInt2Float(x1), iupInt2Float(y1 - 0.5f), iupInt2Float(x2), iupInt2Float(y2 + 0.5f), iupInt2FloatW(line_width), stroke_style);
    else if (y1 == y2)
      wdDrawLineStyled(dc->hCanvas, brush, iupInt2Float(x1 - 0.5f), iupInt2Float(y1), iupInt2Float(x2 + 0.5f), iupInt2Float(y2), iupInt2FloatW(line_width), stroke_style);
    else
      wdDrawLineStyled(dc->hCanvas, brush, iupInt2Float(x1), iupInt2Float(y1), iupInt2Float(x2), iupInt2Float(y2), iupInt2FloatW(line_width), stroke_style);
  }
  else
    wdDrawLineStyled(dc->hCanvas, brush, iupInt2Float(x1), iupInt2Float(y1), iupInt2Float(x2), iupInt2Float(y2), iupInt2FloatW(line_width), stroke_style);
  iDrawRelaseStyle(stroke_style);
  wdDestroyBrush(brush);
}

void iupdrvDrawArc(IdrawCanvas* dc, int x1, int y1, int x2, int y2, double a1, double a2, long color, int style, int line_width)
{
  float xc, yc, rx, ry;
  float baseAngle, sweepAngle;
  WD_HBRUSH brush = wdCreateSolidBrush(dc->hCanvas, 0);
  wdSetSolidBrushColor(brush, iupColor2ARGB(color));

  iupDrawCheckSwapCoord(x1, x2);
  iupDrawCheckSwapCoord(y1, y2);

  rx = (x2 - x1) / 2.0f;
  ry = (y2 - y1) / 2.0f;

  xc = iupInt2Float(x1) + rx;
  yc = iupInt2Float(y1) + ry;

  baseAngle = (float)(360.0 - a2);
  sweepAngle = (float)(a2 - a1);

  if (style == IUP_DRAW_FILL)
  {
    if (sweepAngle == 360.0f)
      wdFillEllipse(dc->hCanvas, brush, xc, yc, rx, ry);
    else
      wdFillEllipsePie(dc->hCanvas, brush, xc, yc, rx, ry, baseAngle, sweepAngle);
  }
  else
  {
    WD_HSTROKESTYLE stroke_style = iDrawSetLineStyle(style);
    if (sweepAngle == 360.0f)
      wdDrawEllipseStyled(dc->hCanvas, brush, xc, yc, rx, ry, iupInt2FloatW(line_width), stroke_style);
    else
      wdDrawEllipseArcStyled(dc->hCanvas, brush, xc, yc, rx, ry, baseAngle, sweepAngle, iupInt2FloatW(line_width), stroke_style);
    iDrawRelaseStyle(stroke_style);
  }

  wdDestroyBrush(brush);
}

void iupdrvDrawPolygon(IdrawCanvas* dc, int* points, int count, long color, int style, int line_width)
{
  WD_HBRUSH brush = wdCreateSolidBrush(dc->hCanvas, iupColor2ARGB(color));
  int i;

  WD_HPATH path = wdCreatePath(dc->hCanvas);
  WD_PATHSINK sink;
  wdOpenPathSink(&sink, path);
  wdBeginFigure(&sink, iupInt2Float(points[0]), iupInt2Float(points[1]));

  for (i = 2; i < count * 2; i = i + 2)
    wdAddLine(&sink, iupInt2Float(points[i]), iupInt2Float(points[i + 1]));

  wdEndFigure(&sink, FALSE);
  wdClosePathSink(&sink);

  if (style == IUP_DRAW_FILL)
    wdFillPath(dc->hCanvas, brush, path);
  else
  {
    WD_HSTROKESTYLE stroke_style = iDrawSetLineStyle(style);
    wdDrawPathStyled(dc->hCanvas, brush, path, iupInt2FloatW(line_width), stroke_style);
    iDrawRelaseStyle(stroke_style);
  }

  wdDestroyPath(path);
  wdDestroyBrush(brush);
}

void iupdrvDrawSetClipRect(IdrawCanvas* dc, int x1, int y1, int x2, int y2)
{
  WD_RECT rect;
  rect.x0 = iupInt2Float(x1);
  rect.y0 = iupInt2Float(y1);
  rect.x1 = iupInt2Float(x2);
  rect.y1 = iupInt2Float(y2);
  wdSetClip(dc->hCanvas, &rect, NULL);
}

void iupdrvDrawResetClip(IdrawCanvas* dc)
{
  wdSetClip(dc->hCanvas, NULL, NULL);
}

static int iCompensatePosX(float font_height)
{
  return iupRound(font_height / 7.);  /* 15% */
}

void iupdrvDrawText(IdrawCanvas* dc, const char* text, int len, int x, int y, int w, int h, long color, const char* font, int align)
{
  WD_RECT rect;
  WCHAR *wtext = iupwinStrChar2Wide(text);
  DWORD flag;
  WD_HBRUSH brush = wdCreateSolidBrush(dc->hCanvas, iupColor2ARGB(color));
 
  HFONT hFont = (HFONT)iupwinGetHFont(font);
  WD_HFONT wdFont = wdCreateFontWithGdiHandle(hFont);

  rect.x0 = iupInt2Float(x);
  rect.x1 = iupInt2Float(x + w);
  rect.y0 = iupInt2Float(y);
  rect.y1 = iupInt2Float(y + h);

  flag = WD_STR_LEFTALIGN;
  if (align == IUP_ALIGN_ARIGHT)
    flag = WD_STR_RIGHTALIGN;
  else if (align == IUP_ALIGN_ACENTER)
    flag = WD_STR_CENTERALIGN;

  if (wdBackend() == WD_BACKEND_GDIPLUS)
  {
    /* compensate GDI+ internal padding */
    WD_FONTMETRICS metrics;
    wdFontMetrics(wdFont, &metrics);
    rect.x0 -= iCompensatePosX(metrics.fLeading);
  }

  wdDrawString(dc->hCanvas, wdFont, &rect, wtext, len, brush, flag | WD_STR_TOPALIGN | WD_STR_NOWRAP);

  wdDestroyBrush(brush);
  wdDestroyFont(wdFont);
}

static WD_HIMAGE wdlGetImage(const char* name, Ihandle* ih_parent, int make_inactive, const char* bgcolor)
{
  WD_HIMAGE hImage = NULL;
#if 0
  HBITMAP hBitmap;

  /* GDI+ does not support HBITMAP with alpha */
  if (wdBackend() == WD_BACKEND_GDIPLUS)
    iupAttribSet(ih_parent, "FLAT_ALPHA", "1");

  hBitmap = (HBITMAP)iupImageGetImage(name, ih_parent, make_inactive, bgcolor);

  if (wdBackend() == WD_BACKEND_GDIPLUS)
    iupAttribSet(ih_parent, "FLAT_ALPHA", NULL);

  if (!hBitmap)
    return NULL;

  hImage = wdCreateImageFromHBITMAP(hBitmap);
#else
  int width, height, channels;
  unsigned char* data;
  Ihandle* ih = IupGetHandle(name);
  if (!ih)
    return NULL;

  width = IupGetInt(ih, "WIDTH");
  height = IupGetInt(ih, "HEIGHT");
  channels = IupGetInt(ih, "CHANNELS");
  data = (unsigned char*)iupAttribGet(ih, "WID");

  if (channels == 4)
    hImage = wdCreateImageFromBuffer(width, height, data, TRUE, NULL);
  else if (channels == 3)
    hImage = wdCreateImageFromBuffer(width, height, data, FALSE, NULL);
  else
  {
    COLORREF cPalette[256];
    iupColor colors[256];
    int i, colors_count;
    unsigned char bg_r = 0, bg_g = 0, bg_b = 0;

    char* img_bgcolor = iupAttribGet(ih, "BGCOLOR");
    if (ih_parent && !img_bgcolor)
    {
      if (!bgcolor)
        bgcolor = IupGetAttribute(ih_parent, "BGCOLOR"); /* Use IupGetAttribute to use inheritance and native implementation */
    }
    else
      bgcolor = img_bgcolor;

    iupStrToRGB(bgcolor, &bg_r, &bg_g, &bg_b);
    iupImageInitColorTable(ih, colors, &colors_count);

    for (i = 0; i < colors_count; i++)
    {
      if (colors[i].a == 0) /* full transparent alpha */
        cPalette[i] = RGB(bg_r, bg_g, bg_b);
      else
        cPalette[i] = RGB(colors[i].r, colors[i].g, colors[i].b);

      if (make_inactive)
        iupImageColorMakeInactive(&(colors[i].r), &(colors[i].g), &(colors[i].b), bg_r, bg_g, bg_b);
    }

    hImage = wdCreateImageFromBuffer(width, height, data, FALSE, cPalette);
  }

#endif
  return hImage;
}

void iupdrvDrawImage(IdrawCanvas* dc, const char* name, int make_inactive, const char* bgcolor, int x, int y)
{
  WD_HIMAGE hImage = wdlGetImage(name, dc->ih, make_inactive, bgcolor);
  if (hImage)
  {
#if 1
    UINT width, height;
    WD_RECT rect;
    wdGetImageSize(hImage, &width, &height);

    rect.x0 = iupInt2Float(x);
    rect.y0 = iupInt2Float(y);
    rect.x1 = iupInt2Float(x + width);
    rect.y1 = iupInt2Float(y + height);

    wdBitBltImage(dc->hCanvas, hImage, &rect, NULL);

    wdDestroyImage(hImage);
#else
    WD_HCACHEDIMAGE hCachedImage = wdCreateCachedImage(dc->hCanvas, hImage);
    wdDestroyImage(hImage);

    if (!hCachedImage)
      return;

    wdBitBltCachedImage(dc->hCanvas, hCachedImage, x, y);   /* TODO bug in GetPixelSize */

    wdDestroyCachedImage(hCachedImage);
#endif
  }
}

void iupdrvDrawSelectRect(IdrawCanvas* dc, int x1, int y1, int x2, int y2)
{
  WD_HBRUSH brush;

  iupDrawCheckSwapCoord(x1, x2);
  iupDrawCheckSwapCoord(y1, y2);

  brush = wdCreateSolidBrush(dc->hCanvas, WD_ARGB(153, 0, 0, 255));
  wdFillRect(dc->hCanvas, brush, iupInt2Float(x1 - 0.5f), iupInt2Float(y1 - 0.5f), iupInt2Float(x2 + 0.5f), iupInt2Float(y2 + 0.5f));
  wdDestroyBrush(brush);
}

void iupdrvDrawFocusRect(IdrawCanvas* dc, int x1, int y1, int x2, int y2)
{
#if 0
  RECT rect;
  HDC hDC;

  iupDrawCheckSwapCoord(x1, x2);
  iupDrawCheckSwapCoord(y1, y2);

  rect.left = x1;
  rect.right = x2;
  rect.top = y1;
  rect.bottom = y2;

  hDC = wdStartGdi(dc->hCanvas, TRUE);  /* TODO wdStartGdi is crashing for D2D */
  DrawFocusRect(hDC, &rect);
  wdEndGdi(dc->hCanvas, hDC);
#else
  iupdrvDrawRectangle(dc, x1, y1, x2, y2, iupDrawColor(0, 0, 0, 255), IUP_DRAW_STROKE_DOT, 1);
#endif
}
