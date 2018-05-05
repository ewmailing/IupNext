/** \file
 * \brief Draw Functions for GDI
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

#include "iupwin_drv.h"
#include "iupwin_info.h"
#include "iupwin_draw.h"
#include "iupwin_str.h"


struct _IdrawCanvas{
  Ihandle* ih;
  int w, h;

  HWND hWnd;
  int release_dc;
  HBITMAP hBitmap, hOldBitmap;
  HDC hBitmapDC, hDC;

  int clip_x1, clip_y1, clip_x2, clip_y2;
};

void iupwinDrawInit(void)
{
  iupwinDrawThemeInit();
}

void iupwinDrawFinish(void)
{
}

IdrawCanvas* iupdrvDrawCreateCanvas(Ihandle* ih)
{
  IdrawCanvas* dc = calloc(1, sizeof(IdrawCanvas));
  RECT rect;

  dc->ih = ih;

  dc->hWnd = (HWND)IupGetAttribute(ih, "HWND");  /* Use the attribute, so it can work with FileDlg preview area */

  /* valid only inside the ACTION callback of an IupCanvas */
  dc->hDC = (HDC)IupGetAttribute(ih, "HDC_WMPAINT");
  if (!dc->hDC)
  {
    dc->hDC = GetDC(dc->hWnd);
    dc->release_dc = 1;
  }

  GetClientRect(dc->hWnd, &rect);
  dc->w = rect.right - rect.left;
  dc->h = rect.bottom - rect.top;

  dc->hBitmap = CreateCompatibleBitmap(dc->hDC, dc->w, dc->h);
  dc->hBitmapDC = CreateCompatibleDC(dc->hDC);
  dc->hOldBitmap = SelectObject(dc->hBitmapDC, dc->hBitmap);

  SetBkMode(dc->hBitmapDC, TRANSPARENT);
  SetTextAlign(dc->hBitmapDC, TA_TOP|TA_LEFT);

  iupAttribSet(ih, "DRAWDRIVER", "GDI");

  return dc;
}

void iupdrvDrawKillCanvas(IdrawCanvas* dc)
{
  SelectObject(dc->hBitmapDC, dc->hOldBitmap);
  DeleteObject(dc->hBitmap);
  DeleteDC(dc->hBitmapDC);
  if (dc->release_dc)
    DeleteDC(dc->hDC);

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

    SelectObject(dc->hBitmapDC, dc->hOldBitmap);
    DeleteObject(dc->hBitmap);
    DeleteDC(dc->hBitmapDC);

    dc->hBitmap = CreateCompatibleBitmap(dc->hDC, dc->w, dc->h);
    dc->hBitmapDC = CreateCompatibleDC(dc->hDC);
    dc->hOldBitmap = SelectObject(dc->hBitmapDC, dc->hBitmap);

    SetBkMode(dc->hBitmapDC, TRANSPARENT);
    SetTextAlign(dc->hBitmapDC, TA_TOP|TA_LEFT);
  }
}

void iupdrvDrawFlush(IdrawCanvas* dc)
{
  BitBlt(dc->hDC, 0, 0, dc->w, dc->h, dc->hBitmapDC, 0, 0, SRCCOPY);
}

void iupdrvDrawGetSize(IdrawCanvas* dc, int *w, int *h)
{
  if (w) *w = dc->w;
  if (h) *h = dc->h;
}

static HPEN iDrawCreatePen(long color, int style, int line_width)
{
  DWORD dashes[2] = { 9-1, 3+1 };
  DWORD dots[2] = { 1-1, 2+1 };
  DWORD* dash_array = NULL;
  LOGBRUSH LogBrush;
  LogBrush.lbStyle = BS_SOLID;
  LogBrush.lbColor = RGB(iupDrawRed(color), iupDrawGreen(color), iupDrawBlue(color));
  LogBrush.lbHatch = 0;

  if (style == IUP_DRAW_STROKE_DASH)
    dash_array = dashes;
  else if (style == IUP_DRAW_STROKE_DOT)
    dash_array = dots;

  if (style == IUP_DRAW_STROKE)
    return ExtCreatePen(PS_GEOMETRIC | PS_SOLID, line_width, &LogBrush, 0, NULL);
  else
    return ExtCreatePen(PS_GEOMETRIC | PS_USERSTYLE, line_width, &LogBrush, 2, dash_array);
}

void iupdrvDrawRectangle(IdrawCanvas* dc, int x1, int y1, int x2, int y2, long color, int style, int line_width)
{
  SetDCBrushColor(dc->hBitmapDC, RGB(iupDrawRed(color),iupDrawGreen(color),iupDrawBlue(color)));

  if (style == IUP_DRAW_FILL)
  {
    RECT rect;
    iupDrawCheckSwapCoord(x1, x2);
    iupDrawCheckSwapCoord(y1, y2);
    SetRect(&rect, x1, y1, x2 + 1, y2 + 1);
    FillRect(dc->hBitmapDC, &rect, (HBRUSH)GetStockObject(DC_BRUSH));
  }
  else if (style == IUP_DRAW_STROKE && line_width == 1)
  {
    RECT rect;
    iupDrawCheckSwapCoord(x1, x2);
    iupDrawCheckSwapCoord(y1, y2);
    SetRect(&rect, x1, y1, x2 + 1, y2 + 1);
    FrameRect(dc->hBitmapDC, &rect, (HBRUSH)GetStockObject(DC_BRUSH));
  }
  else
  {
    POINT line_poly[5];
    HPEN hPen = iDrawCreatePen(color, style, line_width);
    HPEN hPenOld = SelectObject(dc->hBitmapDC, hPen);
    line_poly[0].x = x1;
    line_poly[0].y = y1;
    line_poly[1].x = x1;
    line_poly[1].y = y2;
    line_poly[2].x = x2;
    line_poly[2].y = y2;
    line_poly[3].x = x2;
    line_poly[3].y = y1;
    line_poly[4].x = x1;
    line_poly[4].y = y1;
    Polyline(dc->hBitmapDC, line_poly, 5);
    SelectObject(dc->hBitmapDC, hPenOld);
    DeleteObject(hPen);
  }
}

void iupdrvDrawLine(IdrawCanvas* dc, int x1, int y1, int x2, int y2, long color, int style, int line_width)
{
  POINT line_poly[2];
  HPEN hPen = iDrawCreatePen(color, style, line_width);
  HPEN hPenOld = SelectObject(dc->hBitmapDC, hPen);

  line_poly[0].x = x1;
  line_poly[0].y = y1;
  line_poly[1].x = x2;
  line_poly[1].y = y2;
  Polyline(dc->hBitmapDC, line_poly, 2);
  SetPixelV(dc->hBitmapDC, x2, y2, RGB(iupDrawRed(color),iupDrawGreen(color),iupDrawBlue(color)));

  SelectObject(dc->hBitmapDC, hPenOld);
  DeleteObject(hPen);
}

static int winDrawCalcArc(int c1, int c2, double a, int horiz)
{
  double proj;
  int pos;
  if (horiz)
    proj = cos(IUP_DEG2RAD * a);
  else
    proj = - sin(IUP_DEG2RAD * a);
  pos = (c2 + c1) / 2 + iupROUND((c2 - c1 + 1) * proj / 2.0);
  return pos;
}

void iupdrvDrawArc(IdrawCanvas* dc, int x1, int y1, int x2, int y2, double a1, double a2, long color, int style, int line_width)
{
  int XStartArc, XEndArc, YStartArc, YEndArc;

  iupDrawCheckSwapCoord(x1, x2);
  iupDrawCheckSwapCoord(y1, y2);

  XStartArc = winDrawCalcArc(x1, x2, a1, 1);
  XEndArc = winDrawCalcArc(x1, x2, a2, 1);
  YStartArc = winDrawCalcArc(y1, y2, a1, 0);
  YEndArc = winDrawCalcArc(y1, y2, a2, 0);

  if (style==IUP_DRAW_FILL)
  {
    HBRUSH hBrush = CreateSolidBrush(RGB(iupDrawRed(color),iupDrawGreen(color),iupDrawBlue(color)));
    HPEN hBrushOld = SelectObject(dc->hBitmapDC, hBrush); 
    BeginPath(dc->hBitmapDC); 
    Pie(dc->hBitmapDC, x1, y1, x2 + 1, y2 + 1, XStartArc, YStartArc, XEndArc, YEndArc);
    EndPath(dc->hBitmapDC);
    FillPath(dc->hBitmapDC);
    SelectObject(dc->hBitmapDC, hBrushOld);
    DeleteObject(hBrush);
  }
  else
  {
    HPEN hPen = iDrawCreatePen(color, style, line_width);
    HPEN hPenOld = SelectObject(dc->hBitmapDC, hPen);
    Arc(dc->hBitmapDC, x1, y1, x2 + 1, y2 + 1, XStartArc, YStartArc, XEndArc, YEndArc);
    SelectObject(dc->hBitmapDC, hPenOld);
    DeleteObject(hPen);
  }
}

void iupdrvDrawPolygon(IdrawCanvas* dc, int* points, int count, long color, int style, int line_width)
{
  if (style==IUP_DRAW_FILL)
  {
    HBRUSH hBrush = CreateSolidBrush(RGB(iupDrawRed(color),iupDrawGreen(color),iupDrawBlue(color)));
    HPEN hBrushOld = SelectObject(dc->hBitmapDC, hBrush); 
    BeginPath(dc->hBitmapDC); 
    Polygon(dc->hBitmapDC, (POINT*)points, count);
    EndPath(dc->hBitmapDC);
    FillPath(dc->hBitmapDC);
    SelectObject(dc->hBitmapDC, hBrushOld);
    DeleteObject(hBrush);
  }
  else
  {
    HPEN hPen = iDrawCreatePen(color, style, line_width);
    HPEN hPenOld = SelectObject(dc->hBitmapDC, hPen);
    Polyline(dc->hBitmapDC, (POINT*)points, count);
    SelectObject(dc->hBitmapDC, hPenOld);
    DeleteObject(hPen);
  }
}

void iupdrvDrawGetClipRect(IdrawCanvas* dc, int *x1, int *y1, int *x2, int *y2)
{
  if (x1) *x1 = dc->clip_x1;
  if (y1) *y1 = dc->clip_y1;
  if (x2) *x2 = dc->clip_x2;
  if (y2) *y2 = dc->clip_y2;
}

void iupdrvDrawSetClipRect(IdrawCanvas* dc, int x1, int y1, int x2, int y2)
{
  HRGN clip_hrgn;

  if (x1 == 0 && y1 == 0 && x2 == 0 && y2 == 0)
  {
    iupdrvDrawResetClip(dc);
    return;
  }

  /* make it an empty region */
  if (x1 >= x2) x1 = x2;
  if (y1 >= y2) y1 = y2;

  clip_hrgn = CreateRectRgn(x1, y1, x2 + 1, y2 + 1);
  SelectClipRgn(dc->hBitmapDC, clip_hrgn);
  DeleteObject(clip_hrgn);

  dc->clip_x1 = x1;
  dc->clip_y1 = y1;
  dc->clip_x2 = x2;
  dc->clip_y2 = y2;
}

void iupdrvDrawResetClip(IdrawCanvas* dc)
{
  SelectClipRgn(dc->hBitmapDC, NULL);

  dc->clip_x1 = 0;
  dc->clip_y1 = 0;
  dc->clip_x2 = 0;
  dc->clip_y2 = 0;
}

void iupdrvDrawText(IdrawCanvas* dc, const char* text, int len, int x, int y, int w, int h, long color, const char* font, int flags)
{
  RECT rect;
  TCHAR* wtext;
  UINT uFormat = 0;

  HFONT hOldFont, hFont = (HFONT)iupwinGetHFont(font);
  SetTextColor(dc->hBitmapDC, RGB(iupDrawRed(color),iupDrawGreen(color),iupDrawBlue(color)));
  hOldFont = SelectObject(dc->hBitmapDC, hFont);

  rect.left = x;
  rect.right = x + w;
  rect.top = y;
  rect.bottom = y + h;

  wtext = iupwinStrToSystemLen(text, &len);

  uFormat |= DT_LEFT;
  if (flags & IUP_DRAW_RIGHT)
    uFormat = DT_RIGHT;
  else if (flags & IUP_DRAW_CENTER)
    uFormat = DT_CENTER;

  if (flags & IUP_DRAW_WRAP)
    uFormat |= DT_WORDBREAK;
  else if (flags & IUP_DRAW_ELLIPSIS)
    uFormat |= DT_END_ELLIPSIS;

  if (!(flags & IUP_DRAW_CLIP))
    uFormat |= DT_NOCLIP;

  DrawText(dc->hBitmapDC, wtext, len, &rect, uFormat);

  SelectObject(dc->hBitmapDC, hOldFont);
}

void iupdrvDrawImage(IdrawCanvas* dc, const char* name, int make_inactive, const char* bgcolor, int x, int y, int w, int h)
{
  int bpp, img_w, img_h;
  HBITMAP hBitmap = (HBITMAP)iupImageGetImage(name, dc->ih, make_inactive, bgcolor);
  if (!hBitmap)
    return;

  /* must use this info, since image can be a driver image loaded from resources */
  iupdrvImageGetInfo(hBitmap, &img_w, &img_h, &bpp);

  if (w == -1 || w == 0) w = img_w;
  if (h == -1 || h == 0) h = img_h;

  iupwinDrawBitmap(dc->hBitmapDC, hBitmap, x, y, w, h, img_w, img_h, bpp);
}

void iupdrvDrawSelectRect(IdrawCanvas* dc, int x1, int y1, int x2, int y2)
{
  iupDrawCheckSwapCoord(x1, x2);
  iupDrawCheckSwapCoord(y1, y2);

  BitBlt(dc->hBitmapDC, x1, y1, x2 - x1 + 1, y2 - y1 + 1, dc->hBitmapDC, x1, y1, DSTINVERT);
}

void iupdrvDrawFocusRect(IdrawCanvas* dc, int x1, int y1, int x2, int y2)
{
  RECT rect;

  iupDrawCheckSwapCoord(x1, x2);
  iupDrawCheckSwapCoord(y1, y2);

  rect.left = x1;  
  rect.right = x2 + 1;
  rect.top = y1;
  rect.bottom = y2 + 1;

  DrawFocusRect(dc->hBitmapDC, &rect);
}
