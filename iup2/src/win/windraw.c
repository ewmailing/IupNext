/** \file
* \brief Windows Driver Owner-drawn controls drawing (labels and buttons)
*
*  Used always for buttons, used only for labels with images.
*
* See Copyright Notice in iup.h
* $Id: windraw.c,v 1.2 2013-05-20 20:13:53 scuri Exp $
*/

#include <windows.h>
#include <assert.h> 

#undef  _WIN32_WINNT
#define _WIN32_WINNT 0x0501
#include <uxtheme.h>

#if (_MSC_VER >= 1700)  /* Visual C++ 11.0 ( Visual Studio 2012) */
#include <vssym32.h>
#else
#include <tmschema.h>
#endif

#include "iglobal.h"
#include "idrv.h"
#include "win.h"
#include "wincreat.h"
#include "winhandle.h"

#define IMAGE_DRAW      1
#define IMPRESS_DRAW    2
#define IMINACTIVE_DRAW 3
#define FILL_DRAW       4

#define rect_top(_x)      _x->rcItem.top
#define rect_bottom(_x)   _x->rcItem.bottom
#define rect_left(_x)     _x->rcItem.left
#define rect_right(_x)    _x->rcItem.right

#ifndef ODS_HOTLIGHT
#define ODS_HOTLIGHT        0x0040
#endif

#ifndef TMT_CONTENTMARGINS
#define TMT_CONTENTMARGINS 3602
#endif

typedef HTHEME (STDAPICALLTYPE *winOpenThemeData)(HWND hwnd, LPCWSTR pszClassList);
typedef HRESULT (STDAPICALLTYPE *winCloseThemeData)(HTHEME hTheme);
typedef HRESULT (STDAPICALLTYPE *winDrawThemeBackground)(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, const RECT *pRect, const RECT *pClipRect);
typedef HRESULT (STDAPICALLTYPE *winGetThemeMargins)(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, int iPropId, RECT *prc, MARGINS *pMargins);
typedef HRESULT (STDAPICALLTYPE *winDrawThemeParentBackground)(HWND hwnd, HDC hdc, RECT* prc);
typedef HRESULT (STDAPICALLTYPE *winGetThemeColor)(HTHEME hTheme, int iPartId, int iStateId, int iPropId, COLORREF *pColor);

static winOpenThemeData myOpenThemeData = NULL;
static winCloseThemeData myCloseThemeData = NULL;
static winDrawThemeBackground myDrawThemeBackground = NULL;
static winGetThemeMargins myGetThemeMargins = NULL;
static winDrawThemeParentBackground myDrawThemeParentBackground = NULL;
static winGetThemeColor myGetThemeColor = NULL;

static int winDrawInitTheme(void)
{
  static int first = 1;
  if (!myOpenThemeData && first && iupwinUseComCtl32Ver6())
  {
    HMODULE hinstDll = LoadLibrary("uxtheme.dll");
    if (hinstDll)
    {
      myOpenThemeData = (winOpenThemeData)GetProcAddress(hinstDll, "OpenThemeData");
      myCloseThemeData = (winCloseThemeData)GetProcAddress(hinstDll, "CloseThemeData");
      myDrawThemeBackground = (winDrawThemeBackground)GetProcAddress(hinstDll, "DrawThemeBackground");
      myGetThemeMargins = (winGetThemeMargins)GetProcAddress(hinstDll, "GetThemeMargins");
      myDrawThemeParentBackground = (winDrawThemeParentBackground)GetProcAddress(hinstDll, "DrawThemeParentBackground");
      myGetThemeColor = (winGetThemeColor)GetProcAddress(hinstDll, "GetThemeColor");
    }
    first = 0;
  }
  return myOpenThemeData? 1: 0;
}

static void winDrawFillAround(HDC hdc, RECT* rc, int x, int y, int w, int h, int border, COLORREF bgcolor)
{
  HRGN cliprgn = CreateRectRgn(rc->left+border, rc->top+border, rc->right-border, rc->bottom-border);
  HBRUSH hBrush = CreateSolidBrush(bgcolor);

  x += rc->left;
  y += rc->top;

  SelectClipRgn(hdc, cliprgn);
  ExcludeClipRect(hdc, x+border, y+border, x+w-border, y+h-border);
  FillRect(hdc, rc, hBrush);

  DeleteObject(hBrush);

  SelectClipRgn(hdc, NULL);
  DeleteObject(cliprgn);
}

static void winDrawFillBorder(HDC hdc, RECT* rc, int border, COLORREF bgcolor)
{
  HRGN cliprgn = CreateRectRgn(rc->left, rc->top, rc->right, rc->bottom);
  HBRUSH hBrush = CreateSolidBrush(bgcolor);

  SelectClipRgn(hdc, cliprgn);
  ExcludeClipRect(hdc, rc->left+border, rc->top+border, rc->right-border, rc->bottom-border);
  FillRect(hdc, rc, hBrush);

  DeleteObject(hBrush);

  SelectClipRgn(hdc, NULL);
  DeleteObject(cliprgn);
}

static void winDrawFillBackground(HDC hdc, RECT *rc, COLORREF bgcolor)
{
  HBRUSH hBrush = CreateSolidBrush(bgcolor);
  FillRect(hdc, rc, hBrush);
  DeleteObject(hBrush);
}

static void winDrawText( Ihandle* h, LPDRAWITEMSTRUCT iteminfo, char* text)
{
  if (text && *text)
  {
    SIZE size;
    TEXTMETRIC tm;
    int delta = (iteminfo->itemState & ODS_SELECTED)?1:0;
    int dx, dy;
    HRGN cliprgn;
    HFONT oldhf, hf = NULL;
    hf = (HFONT)SendMessage((HWND)handle(h),WM_GETFONT,(WPARAM)NULL,(LPARAM)NULL);
    if (hf==NULL) hf = (HFONT)GetStockObject( SYSTEM_FONT );

    cliprgn = CreateRectRgn(iteminfo->rcItem.left, iteminfo->rcItem.top, 
                            iteminfo->rcItem.right, iteminfo->rcItem.bottom);
    SelectClipRgn(iteminfo->hDC, cliprgn);

    oldhf = SelectObject(iteminfo->hDC, hf);

    SetTextAlign(iteminfo->hDC, TA_CENTER);
    GetTextMetrics(iteminfo->hDC, &tm);
    SetBkMode(iteminfo->hDC, TRANSPARENT);

    if (iteminfo->itemState & ODS_DISABLED)
      SetTextColor(iteminfo->hDC, GetSysColor(COLOR_GRAYTEXT));
    else
    {
      COLORREF cr;
      if (iupwinGetColorRef (h, IUP_FGCOLOR, &cr))
        SetTextColor(iteminfo->hDC, cr);
    }

    dx = (rect_right(iteminfo)-rect_left(iteminfo))/2 + delta;
    dy = delta + (rect_bottom(iteminfo)-rect_top(iteminfo) + 1 - tm.tmHeight)/2;
  
    GetTextExtentPoint32(iteminfo->hDC, text, strlen(text), &size);

    TextOut(iteminfo->hDC, 
            rect_left(iteminfo) + dx,
            rect_top(iteminfo) + dy,
            text, strlen(text) ); 

    SelectObject(iteminfo->hDC, oldhf);
    SelectClipRgn(iteminfo->hDC, NULL);
    DeleteObject(cliprgn);
  }
}

static void winDrawBitmap(LPDRAWITEMSTRUCT iteminfo, HBITMAP hbm, int width, int height, int dx, int dy)
{
  HRGN cliprgn = CreateRectRgn(iteminfo->rcItem.left, iteminfo->rcItem.top, 
                          iteminfo->rcItem.right, iteminfo->rcItem.bottom);

  HDC hMemDC = CreateCompatibleDC(iteminfo->hDC);
  HBITMAP hOldbm = SelectObject(hMemDC, hbm);

  SelectClipRgn(iteminfo->hDC, cliprgn);

  BitBlt(iteminfo->hDC,
    rect_left(iteminfo) + dx,
    rect_top(iteminfo)  + dy,
    width,
    height,
    hMemDC,
    0, 0,
    SRCCOPY);

  SelectObject(hMemDC, hOldbm);
  DeleteDC(hMemDC);

  SelectClipRgn(iteminfo->hDC, NULL);
  DeleteObject(cliprgn);
}

int iupwinThemeBorder(HWND wnd)
{
  MARGINS marg;
  HTHEME hTheme;
  if (!winDrawInitTheme()) return 0;
  hTheme = myOpenThemeData(wnd, L"BUTTON");
  if (!hTheme) return 0;
  myGetThemeMargins(hTheme, NULL, BP_PUSHBUTTON, PBS_NORMAL, TMT_CONTENTMARGINS, NULL, &marg);
  myCloseThemeData(hTheme);
  return marg.cxLeftWidth;
}

static int winDrawGetThemeStateId(int itemState)
{
  if (itemState & ODS_DISABLED)
    return PBS_DISABLED; 
  else if (itemState & ODS_SELECTED)
    return PBS_PRESSED;  
  else if (itemState & ODS_HOTLIGHT)
    return PBS_HOT;      
  else
    return PBS_NORMAL;   
}

static int winDrawThemeBorder(HWND wnd, LPDRAWITEMSTRUCT iteminfo, COLORREF bgcolor, int fill, COLORREF pbgcolor)
{
  MARGINS marg;
  HTHEME hTheme;
  if (!winDrawInitTheme()) return 0;      
  hTheme = myOpenThemeData(wnd, L"BUTTON");
  if (!hTheme) return 0;
  myGetThemeMargins(hTheme, NULL, BP_PUSHBUTTON, PBS_NORMAL, TMT_CONTENTMARGINS, NULL, &marg);
  winDrawFillBorder(iteminfo->hDC, &iteminfo->rcItem, marg.cxLeftWidth, pbgcolor);
  myDrawThemeBackground(hTheme, iteminfo->hDC, BP_PUSHBUTTON, winDrawGetThemeStateId(iteminfo->itemState), &iteminfo->rcItem, NULL);
  if (fill) 
    winDrawFillAround(iteminfo->hDC, &iteminfo->rcItem, 0, 0, 0, 0, marg.cxLeftWidth, bgcolor);
  myCloseThemeData(hTheme);
  return 1;
}

static void winDrawBorder(HWND wnd, LPDRAWITEMSTRUCT iteminfo, COLORREF bgcolor, int fill, COLORREF pbgcolor)
{
  if (!winDrawThemeBorder(wnd, iteminfo, bgcolor, fill, pbgcolor))
  {
    winDrawFillBackground(iteminfo->hDC, &iteminfo->rcItem, bgcolor);
    if (iteminfo->itemState & ODS_SELECTED)
      DrawEdge(iteminfo->hDC, &iteminfo->rcItem, EDGE_SUNKEN, BF_RECT|BF_SOFT);
    else
      DrawEdge(iteminfo->hDC, &iteminfo->rcItem, EDGE_RAISED, BF_RECT|BF_SOFT);
  }
}

static void winDrawFocus( LPDRAWITEMSTRUCT iteminfo, int border )
{
  if(iteminfo->itemState & ODS_FOCUS)
  {
    RECT rect;

    rect.left = rect_left(iteminfo) + border;
    rect.right = rect_right(iteminfo) - border;
    rect.bottom = rect_bottom(iteminfo) - border;
    rect.top = rect_top(iteminfo) + border;

    DrawFocusRect(iteminfo->hDC, &rect);
  }
}

static COLORREF winDrawGetBgColor(Ihandle* n)
{
  COLORREF bgcolor;
  char* bgcolorattr = IupGetAttribute(n, IUP_BGCOLOR);

  if (bgcolorattr)
  {
    unsigned int r, g, b;
    iupGetColor(n,IUP_BGCOLOR, &r, &g, &b);
    bgcolor = RGB(r,g,b);
  }
  else
    bgcolor = GetSysColor(COLOR_BTNFACE);

  return bgcolor;
}

void iupwinDrawButton( Ihandle* n, LPDRAWITEMSTRUCT iteminfo)
{
  HBITMAP hbm = NULL;
  unsigned int dx = 0, dy = 0;
  Ihandle* image      = IupGetAttributeHandle( n, IUP_IMAGE );
  Ihandle* iminactive = IupGetAttributeHandle( n, IUP_IMINACTIVE );
  Ihandle* impress    = IupGetAttributeHandle( n, IUP_IMPRESS );
  COLORREF bgcolor, pbgcolor;
  Ihandle* himage = NULL;
  int modedraw = FILL_DRAW;
  int border, width, height, 
    draw_border = 0, 
    is_flat = iupCheck(n, "FLAT")==YES? 1: 0;

  pbgcolor = winDrawGetBgColor(IupGetParent(n));
  bgcolor = winDrawGetBgColor(n);

  if (iteminfo->CtlType != ODT_BUTTON)
  {
    /* failed loading the image, just fill the background */
    winDrawFillBackground(iteminfo->hDC, &iteminfo->rcItem, bgcolor);
    return;
  }

  if(image && impress)
    border = 0;
  else
  {
    int tb = iupwinThemeBorder(handle(n));
    if (tb)
      border = tb;
    else
      border = 2;

    draw_border = 1;
    if (is_flat)
    {
      if (iupCheck(n, "_IUPWIN_MOUSEOVER")!=YES)
        draw_border = 0;
    }
  }

  if (iupCheck(n, "_IUPWIN_MOUSEOVER")==YES)
    iteminfo->itemState |= ODS_HOTLIGHT;

  if (image && !iupCheck(n, IUP_ACTIVE))
  {
    modedraw = IMINACTIVE_DRAW;
    if (iminactive)
      himage = iminactive;
    else
    {
      himage = image;
      iupSetEnv(himage, "_IUPWIN_MAKE_INACTIVE", "1");
    }
  }
  else if (impress && ((iteminfo->itemAction & ODA_FOCUS) || 
                       (iteminfo->itemAction & ODA_DRAWENTIRE) || 
                       (iteminfo->itemAction & ODA_SELECT)) 
                       && (iteminfo->itemState & ODS_SELECTED))
  {
    modedraw = IMPRESS_DRAW;
    himage = impress;
  }
  else if (image && ((iteminfo->itemAction & ODA_FOCUS) || 
                     (iteminfo->itemAction & ODA_DRAWENTIRE) || 
                     (iteminfo->itemAction & ODA_SELECT) ))
  {
    modedraw = IMAGE_DRAW;   
    himage = image;
  }

  if (modedraw != FILL_DRAW)
  {
    hbm = iupwinCreateBitmap(himage, iteminfo->hDC, n);
    if (!hbm) 
    {
      /* failed loading the image, just fill the background */
      winDrawFillBackground(iteminfo->hDC, &iteminfo->rcItem, bgcolor);
      return;
    }
  }

  width = IupGetInt(himage,IUP_WIDTH);
  height = IupGetInt(himage,IUP_HEIGHT);

  switch (modedraw)
  {
  case FILL_DRAW:
    {
      char* text = IupGetAttribute(n,IUP_TITLE);
      winDrawBorder((HWND)handle(n), iteminfo, bgcolor, (text && *text)? 0: 1, pbgcolor); /* text always has border */
      winDrawText(n, iteminfo, text);
      if (!is_flat) winDrawFocus(iteminfo, border);
      break;
    }
  case IMAGE_DRAW:
  case IMINACTIVE_DRAW:
    if (draw_border)
      winDrawBorder((HWND)handle(n), iteminfo, bgcolor, 0, pbgcolor);
    else
      winDrawFillBackground(iteminfo->hDC, &iteminfo->rcItem, bgcolor);

    if (border)
    {
      dx = ((rect_right(iteminfo) - rect_left(iteminfo)) - width)/2;
      dy = ((rect_bottom(iteminfo) - rect_top(iteminfo)) - height)/2;

      if (iteminfo->itemState & ODS_SELECTED)
      {
        dx++;
        dy++;
      }
    }

    winDrawBitmap(iteminfo, hbm, width, height, dx, dy);
    if (!is_flat) winDrawFocus(iteminfo, border);
    break;
  case IMPRESS_DRAW:
    if (border)
    {
      dx = ((rect_right(iteminfo) - rect_left(iteminfo)) - width)/2;
      dy = ((rect_bottom(iteminfo) - rect_top(iteminfo)) - height)/2;
    }

    winDrawBitmap(iteminfo, hbm, width, height, dx, dy);
    break;
  }

  if (modedraw != FILL_DRAW)
  {
    DeleteObject(hbm);
  }

  if (!iupCheck(n, IUP_ACTIVE) && !iminactive)
    iupSetEnv(himage, "_IUPWIN_MAKE_INACTIVE", NULL);
}

void iupwinDrawLabel( Ihandle* n, LPDRAWITEMSTRUCT iteminfo)
{
  HBITMAP hbm = NULL;
  Ihandle* himage;
  COLORREF bgcolor;

  if (iteminfo->CtlType != ODT_STATIC) 
    return;

  bgcolor = winDrawGetBgColor(n);

  himage = IupGetAttributeHandle(n, IUP_IMAGE);

  if (himage && !iupCheck(n, IUP_ACTIVE))
    iupSetEnv(himage, "_IUPWIN_MAKE_INACTIVE", "1");
  if (himage) hbm = iupwinCreateBitmap(himage, iteminfo->hDC, n);

  if (!himage || !hbm) 
  {
    /* failed loading the image, just fill the background */
    winDrawFillBackground(iteminfo->hDC, &iteminfo->rcItem, bgcolor);
    return;
  }

  if ((iteminfo->itemAction & ODA_DRAWENTIRE) ||
      (iteminfo->itemAction & ODA_SELECT))
  {
    char *align = IupGetAttribute(n, IUP_ALIGNMENT);
    unsigned int dx = 0, dy = 0;  /* default is left aligned */
    int width = IupGetInt(himage,IUP_WIDTH);
    int height = IupGetInt(himage,IUP_HEIGHT);

    if (align && iupStrEqual(align, IUP_ARIGHT))
    {
      dx = (rect_right(iteminfo) - rect_left(iteminfo)) - width;
      dy = (rect_bottom(iteminfo) - rect_top(iteminfo)) - height;
    }
    else if (align && iupStrEqual(align, IUP_ACENTER))
    {
      dx = ((rect_right(iteminfo) - rect_left(iteminfo)) - width)/2;
      dy = ((rect_bottom(iteminfo) - rect_top(iteminfo)) - height)/2;
    }

    winDrawFillAround(iteminfo->hDC, &iteminfo->rcItem, dx, dy, width, height, 0, bgcolor);

    winDrawBitmap(iteminfo, hbm, width, height, dx, dy);
  }

  if (himage && !iupCheck(n, IUP_ACTIVE))
    iupSetEnv(himage, "_IUPWIN_MAKE_INACTIVE", NULL);

  DeleteObject(hbm);
}
