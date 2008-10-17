/** \file
 * \brief Frame Control
 *
 * See Copyright Notice in iup.h
 */

#include <windows.h>
#include <commctrl.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <stdarg.h>

#include "iup.h"

#include "iup_object.h"
#include "iup_attrib.h"
#include "iup_str.h"
#include "iup_drv.h"
#include "iup_stdcontrols.h"
#include "iup_frame.h"

#include "iupwin_drv.h"
#include "iupwin_handle.h"
#include "iupwin_draw.h"


void iupdrvFrameGetDecorOffset(Ihandle* ih, int *x, int *y)
{
  if (iupwin_comctl32ver6)
  {
    *x = 3;
    *y = 3;
  }
  else
  {
    *x = 2;
    *y = 2;
  }

  if (iupAttribGetStr(ih, "_IUPFRAME_HAS_TITLE") || iupAttribGetStr(ih, "TITLE"))
  {
    (*y) += iupFrameGetTitleHeight(ih);
  }
}

static void winFrameDrawText(HDC hDC, const char* text, int x, int y, COLORREF fgcolor)
{
  COLORREF oldcolor;

  SetTextAlign(hDC, TA_TOP|TA_LEFT);
  SetBkMode(hDC, TRANSPARENT);
  oldcolor = SetTextColor(hDC, fgcolor);

  TextOut(hDC, x, y, text, strlen(text));

  SetTextColor(hDC, oldcolor);
  SetBkMode(hDC, OPAQUE);
}

static void winFrameDrawItem(Ihandle* ih, DRAWITEMSTRUCT *drawitem)
{ 
  iupwinBitmapDC bmpDC;
  HDC hDC = iupwinDrawCreateBitmapDC(&bmpDC, drawitem->hDC, drawitem->rcItem.right-drawitem->rcItem.left, 
                                                            drawitem->rcItem.bottom-drawitem->rcItem.top);

  iupwinDrawParentBackground(ih, hDC, &drawitem->rcItem);

  if (iupAttribGetStr(ih, "_IUPFRAME_HAS_TITLE"))
  {
    int x, y;
    HFONT hOldFont, hFont = (HFONT)iupwinGetHFontAttrib(ih);
    int txt_height = iupFrameGetTitleHeight(ih);
    COLORREF fgcolor;
    SIZE size;

    char* title = iupdrvBaseGetTitleAttrib(ih);
    if (!title) title = "";

    x = drawitem->rcItem.left+7;
    y = drawitem->rcItem.top;

    hOldFont = SelectObject(hDC, hFont);
    GetTextExtentPoint32(hDC, title, strlen(title), &size);
    ExcludeClipRect(hDC, x-2, y, x+size.cx+2, y+size.cy);

    drawitem->rcItem.top += txt_height/2;
    if (iupwin_comctl32ver6)
      iupwinDrawThemeFrameBorder(ih->handle, hDC, &drawitem->rcItem, drawitem->itemState);
    else
      DrawEdge(hDC, &drawitem->rcItem, EDGE_ETCHED, BF_RECT);

    SelectClipRgn(hDC, NULL);

    if (drawitem->itemState & ODS_DISABLED)
      fgcolor = GetSysColor(COLOR_GRAYTEXT);
    else
    {
      unsigned char r, g, b;
      char* color = iupAttribGetStrInherit(ih, "FGCOLOR");
      if (!color)
      {
        if (!iupwinDrawGetThemeFrameFgColor(ih->handle, &fgcolor))
          fgcolor = 0;  /* black */
      }
      else
      {
        if (iupStrToRGB(color, &r, &g, &b))
          fgcolor = RGB(r,g,b);
        else
          fgcolor = 0;  /* black */
      }
    }

    winFrameDrawText(hDC, title, x, y, fgcolor);

    SelectObject(hDC, hOldFont);
  }
  else
  {
    char* value = iupAttribGetStr(ih, "SUNKEN");
    if (iupStrBoolean(value))
      DrawEdge(hDC, &drawitem->rcItem, EDGE_SUNKEN, BF_RECT);
    else
      DrawEdge(hDC, &drawitem->rcItem, EDGE_ETCHED, BF_RECT);
  }

  iupwinDrawDestroyBitmapDC(&bmpDC);
}

static int winFrameProc(Ihandle* ih, UINT msg, WPARAM wp, LPARAM lp, LRESULT *result)
{
  switch (msg)
  {
  case WM_ERASEBKGND:
    {
      /* just to ignore the internal processing */
      *result = 1;
      return 1;
    }
  }

  return iupwinBaseContainerProc(ih, msg, wp, lp, result);
}

static int winFrameMapMethod(Ihandle* ih)
{
  char *title;
  DWORD dwStyle = WS_CHILD|WS_CLIPSIBLINGS|
                  BS_OWNERDRAW|BS_NOTIFY,   /* NOTIFY is necessary because of the base messages */ 
      dwStyleEx = WS_EX_CONTROLPARENT; 

  if (!ih->parent)
    return IUP_ERROR;

  /* When using Visual Styles, if a frame is inside another
     then the title font of the inner frame was wrong.
     Also when in Vista COMPOSITE=YES is not working,
     to reduce flicker we implement our own frame with double buffer. */

  title = iupAttribGetStr(ih, "TITLE");
  if (title)
    iupAttribSetStr(ih, "_IUPFRAME_HAS_TITLE", "1");

  if (iupAttribGetIntDefault(IupGetDialog(ih), "COMPOSITED"))
    dwStyleEx |= WS_EX_COMPOSITED;
  else
    dwStyle |= WS_CLIPCHILDREN;

  if (!iupwinCreateWindowEx(ih, "BUTTON", dwStyleEx, dwStyle))
    return IUP_ERROR;

  /* replace the WinProc to handle base callbacks */
  IupSetCallback(ih, "_IUPWIN_CTRLPROC_CB", (Icallback)winFrameProc);

  /* Process WM_DRAWITEM */
  IupSetCallback(ih, "_IUPWIN_DRAWITEM_CB", (Icallback)winFrameDrawItem);  

  return IUP_NOERROR;
}

void iupdrvFrameInitClass(Iclass* ic)
{
  /* Driver Dependent Class functions */
  ic->Map = winFrameMapMethod;

  /* Driver Dependent Attribute functions */

  /* Special */
  iupClassRegisterAttribute(ic, "FGCOLOR", NULL, NULL, "0 0 0", IUP_NOT_MAPPED, IUP_INHERIT);  /* black */    
  iupClassRegisterAttribute(ic, "TITLE", iupdrvBaseGetTitleAttrib, iupdrvBaseSetTitleAttrib, NULL, IUP_MAPPED, IUP_NO_INHERIT);
}
