/** \file
* \brief Tabs Control
*
* See Copyright Notice in "iup.h"
*/

#include <windows.h>
#include <commctrl.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <memory.h>
#include <stdarg.h>

#include "iup.h"
#include "iupcbs.h"

#include "iup_object.h"
#include "iup_attrib.h"
#include "iup_str.h"
#include "iup_drv.h"
#include "iup_stdcontrols.h"
#include "iup_tabs.h"
#include "iup_image.h"
#include "iup_array.h"
#include "iup_assert.h"

#include "iupwin_drv.h"
#include "iupwin_handle.h"
#include "iupwin_draw.h"
#include "iupwin_info.h"


#ifndef WS_EX_COMPOSITED
#define WS_EX_COMPOSITED        0x02000000L
#endif


static Iarray* winTabsGetVisibleArray(Ihandle* ih)
{
  int init = 0;
  Iarray* visible_array = (Iarray*)iupAttribGet(ih, "_IUPWIN_VISIBLEARRAY");
  int count = IupGetChildCount(ih);
  if (!visible_array)
  {
    /* create the array if does not exist */
    visible_array = iupArrayCreate(count, sizeof(int));
    iupAttribSetStr(ih, "_IUPWIN_VISIBLEARRAY", (char*)visible_array);

    iupArrayAdd(visible_array, count);
    init = 1;
  }
  else if (count != iupArrayCount(visible_array))
  {
    iupArrayRemove(visible_array, 0, iupArrayCount(visible_array));

    iupArrayAdd(visible_array, count);
    init = 1;
  }

  if (init)
  {
    int pos;
    int* visible_array_array_data = (int*)iupArrayGetData(visible_array);
    for (pos=0; pos<count; pos++)
      visible_array_array_data[pos] = pos;
  }
  return visible_array;
}

static void winTabSetVisibleArrayItem(Ihandle* ih, int pos, int visible)
{
  int i, p;
  Iarray* visible_array = winTabsGetVisibleArray(ih);
  int* visible_array_array_data = (int*)iupArrayGetData(visible_array);
  int count = iupArrayCount(visible_array);

  p = 0;
  for (i=0; i<count; i++)
  {
    if (i == pos)
    {
      if (visible)
        visible_array_array_data[i] = p;
      else
        visible_array_array_data[i] = -1;
    }

    if (i > pos && visible_array_array_data[i] != -1)
    {
      if (visible)
        visible_array_array_data[i]++;
      else
        visible_array_array_data[i]--;
    }

    if (visible_array_array_data[i] != -1)
      p++;
  }

  if (i == p)
    ih->data->has_invisible = 0;
  else
    ih->data->has_invisible = 1;
}

static void winTabInsertVisibleArrayItem(Ihandle* ih, int pos)
{
  if (ih->data->has_invisible)
  {
    Iarray* visible_array = winTabsGetVisibleArray(ih);
    iupArrayInsert(visible_array, pos, 1);
    winTabSetVisibleArrayItem(ih, pos, 1);
  }
}

static void winTabDeleteVisibleArrayItem(Ihandle* ih, int pos)
{
  if (ih->data->has_invisible)
  {
    Iarray* visible_array = winTabsGetVisibleArray(ih);
    winTabSetVisibleArrayItem(ih, pos, 0);
    iupArrayRemove(visible_array, pos, 1);
  }
}

static int winTabsPosFixToWin(Ihandle* ih, int pos)
{
  if (ih->data->has_invisible)
  {
    Iarray* visible_array = (Iarray*)iupAttribGet(ih, "_IUPWIN_VISIBLEARRAY");
    int* visible_array_array_data = (int*)iupArrayGetData(visible_array);
    return visible_array_array_data[pos];
  }
  else
    return pos;
}

static int winTabsPosFixFromWin(Ihandle* ih, int p)
{
  if (ih->data->has_invisible)
  {
    Iarray* visible_array = (Iarray*)iupAttribGet(ih, "_IUPWIN_VISIBLEARRAY");
    int* visible_array_array_data = (int*)iupArrayGetData(visible_array);
    int pos, count = iupArrayCount(visible_array);
    for (pos=0; pos<count; pos++)
    {
      if (visible_array_array_data[pos] == p)
        return pos;
    }
    iupERROR("IupTabs Error. Invalid internal visible state.");
    return 0;  /* INTERNAL ERROR: must always be found */
  }
  else
    return p;
}

int iupdrvTabsExtraDecor(Ihandle* ih)
{
  (void)ih;
  return 0;
}

int iupdrvTabsGetLineCountAttrib(Ihandle* ih)
{
  return (int)SendMessage(ih->handle, TCM_GETROWCOUNT, 0, 0);
}

static HWND winTabsGetPageWindow(Ihandle* ih, int pos)
{
  int p = winTabsPosFixToWin(ih, pos);
  if (p >= 0)
  {
    TCITEM tie;
    tie.mask = TCIF_PARAM;
    SendMessage(ih->handle, TCM_GETITEM, p, (LPARAM)&tie);
    return (HWND)tie.lParam;
  }
  else
    return NULL;  /* invisible */
}

void iupdrvTabsSetCurrentTab(Ihandle* ih, int pos)
{
  int p = winTabsPosFixToWin(ih, pos);
  if (p >= 0)
  {
    int prev_pos = iupdrvTabsGetCurrentTab(ih);
    HWND tab_page = winTabsGetPageWindow(ih, prev_pos);
    if (tab_page)
      ShowWindow(tab_page, SW_HIDE);

    SendMessage(ih->handle, TCM_SETCURSEL, p, 0);

    tab_page = winTabsGetPageWindow(ih, pos);
    if (tab_page)
      ShowWindow(tab_page, SW_SHOW);
  }
}

int iupdrvTabsGetCurrentTab(Ihandle* ih)
{
  return winTabsPosFixFromWin(ih, (int)SendMessage(ih->handle, TCM_GETCURSEL, 0, 0));
}

static int winTabsGetImageIndex(Ihandle* ih, const char* name)
{
  HIMAGELIST image_list;
  int count, i, bpp, ret;
  Iarray* bmp_array;
  HBITMAP *bmp_array_data, hMask=NULL;
  HBITMAP bmp = iupImageGetImage(name, ih, 0);
  if (!bmp)
    return -1;

  /* the array is used to avoi adding the same bitmap twice */
  bmp_array = (Iarray*)iupAttribGet(ih, "_IUPWIN_BMPARRAY");
  if (!bmp_array)
  {
    /* create the array if does not exist */
    bmp_array = iupArrayCreate(50, sizeof(HBITMAP));
    iupAttribSetStr(ih, "_IUPWIN_BMPARRAY", (char*)bmp_array);
  }

  bmp_array_data = iupArrayGetData(bmp_array);

  image_list = (HIMAGELIST)SendMessage(ih->handle, TCM_GETIMAGELIST, 0, 0);
  if (!image_list)
  {
    int width, height;
    UINT flags = ILC_COLOR32|ILC_MASK;

    /* must use this info, since image can be a driver image loaded from resources */
    iupdrvImageGetInfo(bmp, &width, &height, &bpp);

    /* create the image list if does not exist */
    image_list = ImageList_Create(width, height, flags, 0, 50);
    SendMessage(ih->handle, TCM_SETIMAGELIST, 0, (LPARAM)image_list);
  }
  else
    iupdrvImageGetInfo(bmp, NULL, NULL, &bpp);

  /* check if that bitmap is already added to the list,
     but we can not compare with the actual bitmap at the list since it is a copy */
  count = ImageList_GetImageCount(image_list);
  for (i=0; i<count; i++)
  {
    if (bmp_array_data[i] == bmp)
      return i;
  }

  if (bpp == 8)
  {
    Ihandle* image = IupGetHandle(name);
    if (image)
    {
      iupAttribSetStr(image, "_IUPIMG_NO_INVERT", "1");
      hMask = iupdrvImageCreateMask(image);
      iupAttribSetStr(image, "_IUPIMG_NO_INVERT", NULL);
    }
  }

  bmp_array_data = iupArrayInc(bmp_array);
  bmp_array_data[i] = bmp;
  ret = ImageList_Add(image_list, bmp, hMask);  /* the bmp is duplicated at the list */
  DeleteObject(hMask);
  return ret;
}

static void winTabSetPageWindowPos(HWND tab_page, RECT *rect) 
{ 
  SetWindowPos(tab_page, NULL, 
                rect->left, rect->top,  
                rect->right - rect->left, rect->bottom - rect->top, 
                SWP_NOACTIVATE|SWP_NOZORDER);
}

static void winTabsPlacePageWindows(Ihandle* ih, int w, int h)
{
  TCITEM tie;
  RECT rect; 
  Ihandle* child;

  tie.mask = TCIF_PARAM;

  /* Calculate the display rectangle, assuming the 
     tab control is the size of the client area. */
  SetRect(&rect, 0, 0, w, h); 
  SendMessage(ih->handle, TCM_ADJUSTRECT, FALSE, (LPARAM)&rect);

  for (child = ih->firstchild; child; child = child->brother)
  {
    HWND tab_page = (HWND)iupAttribGet(child, "_IUPTAB_CONTAINER");
    winTabSetPageWindowPos(tab_page, &rect);
  }
}

static int winTabsUsingXPStyles(Ihandle* ih)
{
  return iupwin_comctl32ver6 && ih->data->type == ITABS_TOP;
}

static void winTabsDrawPageBackground(Ihandle* ih, HDC hDC, RECT* rect)
{
  unsigned char r=0, g=0, b=0;
  char* color = iupAttribGetInheritNativeParent(ih, "BGCOLOR");
  if (!color) color = iupAttribGetInheritNativeParent(ih, "BACKGROUND");
  if (!color) color = iupAttribGet(ih, "BACKGROUND");
  if (!color) color = IupGetGlobal("DLGBGCOLOR");
  iupStrToRGB(color, &r, &g, &b);
  SetDCBrushColor(hDC, RGB(r,g,b));
  FillRect(hDC, rect, (HBRUSH)GetStockObject(DC_BRUSH));
}

static LRESULT CALLBACK winTabsPageWinProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{   
  switch (msg)
  {
  case WM_ERASEBKGND:
    {
      RECT rect;
      HDC hDC = (HDC)wp;
      Ihandle* ih = iupwinHandleGet(hWnd);
      GetClientRect(ih->handle, &rect); 
      winTabsDrawPageBackground(ih, hDC, &rect);

      /* return non zero value */
      return 1;
    }
  case WM_COMMAND:
  case WM_CTLCOLORSCROLLBAR:
  case WM_CTLCOLORBTN:
  case WM_CTLCOLOREDIT:
  case WM_CTLCOLORLISTBOX:
  case WM_CTLCOLORSTATIC:
  case WM_DRAWITEM:
  case WM_HSCROLL:
  case WM_NOTIFY:
  case WM_VSCROLL:
    /* Forward the container messages to its parent. */
    return SendMessage(GetParent(hWnd), msg, wp, lp);
  }

  return DefWindowProc(hWnd, msg, wp, lp);
}

static HWND winTabCreatePageWindow(Ihandle* ih) 
{ 
  HWND hWnd;
  DWORD dwStyle = WS_CHILD|WS_CLIPSIBLINGS, 
      dwExStyle = 0;

  iupwinGetNativeParentStyle(ih, &dwExStyle, &dwStyle);

  hWnd = CreateWindowEx(dwExStyle, "IupTabsPage", NULL, dwStyle, 
                        0, 0, CW_USEDEFAULT, CW_USEDEFAULT, 
                        ih->handle, NULL, iupwin_hinstance, NULL); 

  iupwinHandleAdd(ih, hWnd);

  return hWnd;
} 

static void winTabInsertItem(Ihandle* ih, Ihandle* child, int pos, HWND tab_page)
{
  TCITEM tie;
  char *tabtitle, *tabimage;
  int old_rowcount;

  tabtitle = iupTabsAttribGetStrId(ih, "TABTITLE", pos);
  if (!tabtitle) 
  {
    tabtitle = iupAttribGet(child, "TABTITLE");
    if (tabtitle)
      iupTabsAttribSetStrId(ih, "TABTITLE", pos, tabtitle);
  }
  tabimage = iupTabsAttribGetStrId(ih, "TABIMAGE", pos);
  if (!tabimage) 
  {
    tabimage = iupAttribGet(child, "TABIMAGE");
    if (tabimage)
      iupTabsAttribSetStrId(ih, "TABIMAGE", pos, tabimage);
  }
  if (!tabtitle && !tabimage)
    tabtitle = "     ";

  old_rowcount = (int)SendMessage(ih->handle, TCM_GETROWCOUNT, 0, 0);

  tie.mask = TCIF_PARAM;

  if (tabtitle)
  {
    tie.mask |= TCIF_TEXT;
    tie.pszText = tabtitle;
    tie.cchTextMax = strlen(tabtitle);

    iupwinSetMnemonicTitle(ih, pos, tabtitle);
  }

  if (tabimage)
  {
    tie.mask |= TCIF_IMAGE;
    tie.iImage = winTabsGetImageIndex(ih, tabimage);
  }

  /* create tabs and label them */
  tie.lParam = (LPARAM)tab_page;
  SendMessage(ih->handle, TCM_INSERTITEM, winTabsPosFixToWin(ih, pos), (LPARAM)&tie);

  if (ih->data->is_multiline)
  {
    if (ih->data->type == ITABS_LEFT || ih->data->type == ITABS_RIGHT)
    {
      int rowcount = (int)SendMessage(ih->handle, TCM_GETROWCOUNT, 0, 0);
      if (rowcount != old_rowcount)
      {
        RECT rect;
        GetClientRect(ih->handle, &rect);
        winTabsPlacePageWindows(ih, rect.right - rect.left, rect.bottom - rect.top);
      }
    }

    iupdrvRedrawNow(ih);
  }
}

static void winTabDeleteItem(Ihandle* ih, int p)
{
  SendMessage(ih->handle, TCM_DELETEITEM, p, 0);
}

/* ------------------------------------------------------------------------- */
/* winTabs - Sets and Gets accessors                                         */
/* ------------------------------------------------------------------------- */

static int winTabsSetPaddingAttrib(Ihandle* ih, const char* value)
{
  iupStrToIntInt(value, &ih->data->horiz_padding, &ih->data->vert_padding, 'x');

  if (ih->handle)
  {
    SendMessage(ih->handle, TCM_SETPADDING, 0, MAKELPARAM(ih->data->horiz_padding, ih->data->vert_padding));
    return 0;
  }
  else
    return 1; /* store until not mapped, when mapped will be set again */
}

static int winTabsSetMultilineAttrib(Ihandle* ih, const char* value)
{
  if (ih->handle) /* allow to set only before mapping */
    return 0;

  if (iupStrBoolean(value))
    ih->data->is_multiline = 1;
  else
  {
    if (ih->data->type == ITABS_BOTTOM || ih->data->type == ITABS_TOP)
      ih->data->is_multiline = 0;
    else
      ih->data->is_multiline = 1;   /* always true if left/right */
  }

  return 0;
}

static char* winTabsGetMultilineAttrib(Ihandle* ih)
{
  if (ih->data->is_multiline)
    return "YES";
  else
    return "NO";
}

static int winTabsSetTabTypeAttrib(Ihandle* ih, const char* value)
{
  if (ih->handle) /* allow to set only before mapping */
    return 0;

  if(iupStrEqualNoCase(value, "BOTTOM"))
  {
    ih->data->type = ITABS_BOTTOM;
    ih->data->orientation = ITABS_HORIZONTAL;  /* TABTYPE controls TABORIENTATION in Windows */
  }
  else if(iupStrEqualNoCase(value, "LEFT"))
  {
    ih->data->type = ITABS_LEFT;
    ih->data->orientation = ITABS_VERTICAL;  /* TABTYPE controls TABORIENTATION in Windows */
    ih->data->is_multiline = 1; /* VERTICAL works only with MULTILINE */
  }
  else if(iupStrEqualNoCase(value, "RIGHT"))
  {
    ih->data->type = ITABS_RIGHT;
    ih->data->orientation = ITABS_VERTICAL;  /* TABTYPE controls TABORIENTATION in Windows */
    ih->data->is_multiline = 1; /* VERTICAL works only with MULTILINE */
  }
  else /* "TOP" */
  {
    ih->data->type = ITABS_TOP;
    ih->data->orientation = ITABS_HORIZONTAL;  /* TABTYPE controls TABORIENTATION in Windows */
  }

  return 0;
}

static int winTabsSetTabTitleAttrib(Ihandle* ih, int pos, const char* value)
{
  if (value)
  {
    int p = winTabsPosFixToWin(ih, pos);
    if (p >= 0)
    {
      TCITEM tie;

      tie.mask = TCIF_TEXT;
      tie.pszText = (char*)value;
      tie.cchTextMax = strlen(value);

      iupwinSetMnemonicTitle(ih, pos, value);

      SendMessage(ih->handle, TCM_SETITEM, p, (LPARAM)&tie);
    }
  }
  return 1;
}

static int winTabsSetTabImageAttrib(Ihandle* ih, int pos, const char* value)
{
  if (value)
  {
    int p = winTabsPosFixToWin(ih, pos);
    if (p >= 0)
    {
      TCITEM tie;

      tie.mask = TCIF_IMAGE;
      tie.iImage = winTabsGetImageIndex(ih, value);

      SendMessage(ih->handle, TCM_SETITEM, p, (LPARAM)&tie);
    }
  }
  return 1;
}

static int winTabsSetTabVisibleAttrib(Ihandle* ih, int pos, const char* value)
{
  Ihandle* child = IupGetChild(ih, pos);
  HWND tab_page = (HWND)iupAttribGet(child, "_IUPTAB_CONTAINER");
  int p = winTabsPosFixToWin(ih, pos);
  if (iupStrBoolean(value))
  {
    if (p < 0)  /* invisible */
    {
      winTabSetVisibleArrayItem(ih, pos, 1);  /* to visible */
      winTabInsertItem(ih, child, pos, tab_page);
    }
  }
  else
  {
    if (p >= 0)  /* visible */
    {
      iupTabsCheckCurrentTab(ih, pos);
      winTabSetVisibleArrayItem(ih, pos, 0);  /* to invisible */
      winTabDeleteItem(ih, p);
    }
  }
  return 1;
}

static char* winTabsGetBgColorAttrib(Ihandle* ih)
{
  /* the most important use of this is to provide
     the correct background for images */
  if (iupwin_comctl32ver6)
  {
    COLORREF cr;
    if (iupwinDrawGetThemeTabsBgColor(ih->handle, &cr))
    {
      char* str = iupStrGetMemory(20);
      sprintf(str, "%d %d %d", (int)GetRValue(cr), (int)GetGValue(cr), (int)GetBValue(cr));
      return str;
    }
  }

  return IupGetGlobal("DLGBGCOLOR");
}

static int winTabsSetBgColorAttrib(Ihandle *ih, const char *value)
{
  (void)value;
  iupdrvPostRedraw(ih);
  return 1;
}


/* ------------------------------------------------------------------------- */
/* winTabs - Calls the user callback to change of tab                        */
/* ------------------------------------------------------------------------- */

static int winTabsCtlColor(Ihandle* ih, HDC hdc, LRESULT *result)
{
  /* works only when NOT winTabsUsingXPStyles */
  unsigned char r, g, b;
  char* color = iupBaseNativeParentGetBgColorAttrib(ih);
  if (iupStrToRGB(color, &r, &g, &b))
  {
    SetDCBrushColor(hdc, RGB(r,g,b));
    *result = (LRESULT)GetStockObject(DC_BRUSH);
    return 1;
  }
  return 0;
}

static int winTabsWmNotify(Ihandle* ih, NMHDR* msg_info, int *result)
{
  (void)result;

  if (msg_info->code == TCN_SELCHANGING)
  {
    IFnnn cb = (IFnnn)IupGetCallback(ih, "TABCHANGE_CB");
    int prev_pos = iupdrvTabsGetCurrentTab(ih);
    iupAttribSetInt(ih, "_IUPTABS_PREV_CHILD_POS", prev_pos);

    /* save the previous handle if callback exists */
    if (cb)
    {
      Ihandle* prev_child = IupGetChild(ih, prev_pos);
      iupAttribSetStr(ih, "_IUPTABS_PREV_CHILD", (char*)prev_child);
    }
  }

  if (msg_info->code == TCN_SELCHANGE)
  {
    IFnnn cb = (IFnnn)IupGetCallback(ih, "TABCHANGE_CB");
    int pos = iupdrvTabsGetCurrentTab(ih);
    int prev_pos = iupAttribGetInt(ih, "_IUPTABS_PREV_CHILD_POS");
    HWND tab_page = winTabsGetPageWindow(ih, pos);
    if (tab_page)
      ShowWindow(tab_page, SW_SHOW);
    tab_page = winTabsGetPageWindow(ih, prev_pos);
    if (tab_page)
      ShowWindow(tab_page, SW_HIDE);

    if (cb)
    {
      Ihandle* child = IupGetChild(ih, pos);
      Ihandle* prev_child = (Ihandle*)iupAttribGet(ih, "_IUPTABS_PREV_CHILD");
      iupAttribSetStr(ih, "_IUPTABS_PREV_CHILD", NULL);

      /* avoid duplicate calls when a Tab is inside another Tab. */
      if (prev_child)
        cb(ih, child, prev_child);
    }
    else
    {
      IFnii cb2 = (IFnii)IupGetCallback(ih, "TABCHANGEPOS_CB");
      if (cb2)
        cb2(ih, pos, prev_pos);
    }
  }

  return 0; /* result not used */
}

static int winTabsProc(Ihandle* ih, UINT msg, WPARAM wp, LPARAM lp, LRESULT *result)
{
  switch(msg)
  {
  case WM_SIZE:
    {
      WNDPROC oldProc = (WNDPROC)IupGetCallback(ih, "_IUPWIN_OLDPROC_CB");
      CallWindowProc(oldProc, ih->handle, msg, wp, lp);

      winTabsPlacePageWindows(ih, LOWORD(lp), HIWORD(lp));

      *result = 0;
      return 1;
    }
  }

  return iupwinBaseContainerProc(ih, msg, wp, lp, result);
}

/* ------------------------------------------------------------------------- */
/* winTabs - Methods and Init Class                                          */
/* ------------------------------------------------------------------------- */

static void winTabsChildAddedMethod(Ihandle* ih, Ihandle* child)
{
  if (IupGetName(child) == NULL)
    iupAttribSetHandleName(child);

  if (ih->handle)
  {
    HWND tab_page;
    int pos;
    RECT rect; 

    pos = IupGetChildPos(ih, child);

    tab_page = winTabCreatePageWindow(ih);

    if (pos == 0)
      ShowWindow(tab_page, SW_SHOW);

    /* Calculate the display rectangle, assuming the 
       tab control is the size of the client area. */
    GetClientRect(ih->handle, &rect);
    SendMessage(ih->handle, TCM_ADJUSTRECT, FALSE, (LPARAM)&rect);

    winTabSetPageWindowPos(tab_page, &rect);

    iupAttribSetStr(child, "_IUPTAB_CONTAINER", (char*)tab_page);

    winTabInsertItem(ih, child, pos, tab_page);

    winTabInsertVisibleArrayItem(ih, pos);
  }
}

static int winTabsGetPageWindowPos(Ihandle* ih, HWND tab_page)
{
  TCITEM tie;
  int pos, num_tabs;

  num_tabs = (int)SendMessage(ih->handle, TCM_GETITEMCOUNT, 0, 0);
  tie.mask = TCIF_PARAM;

  for (pos=0; pos<num_tabs; pos++)
  {
    SendMessage(ih->handle, TCM_GETITEM, pos, (LPARAM)&tie);
    if (tab_page == (HWND)tie.lParam)
      return pos;
  }

  return -1;
}

static void winTabsChildRemovedMethod(Ihandle* ih, Ihandle* child)
{
  if (ih->handle)
  {
    HWND tab_page = (HWND)iupAttribGet(child, "_IUPTAB_CONTAINER");
    if (tab_page)
    {
      /* can not use IupGetChild here, because child has already been detached */
      int pos = winTabsGetPageWindowPos(ih, tab_page);
      int p = winTabsPosFixToWin(ih, pos);  

      iupTabsCheckCurrentTab(ih, pos);

      winTabDeleteVisibleArrayItem(ih, pos);

      if (p >= 0)
        winTabDeleteItem(ih, p);

      iupwinHandleRemove(tab_page);
      DestroyWindow(tab_page);

      iupAttribSetStr(child, "_IUPTAB_CONTAINER", NULL);
    }
  }
}

static int winTabsMapMethod(Ihandle* ih)
{
  DWORD dwStyle = WS_CHILD | WS_CLIPSIBLINGS | TCS_HOTTRACK | WS_TABSTOP,
      dwExStyle = 0;

  if (!ih->parent)
    return IUP_ERROR;

  if (ih->data->type == ITABS_BOTTOM)
    dwStyle |= TCS_BOTTOM;
  else if (ih->data->type == ITABS_RIGHT)
    dwStyle |= TCS_VERTICAL|TCS_RIGHT;  
  else if (ih->data->type == ITABS_LEFT)
    dwStyle |= TCS_VERTICAL;

  if (ih->data->is_multiline)
    dwStyle |= TCS_MULTILINE;

  iupwinGetNativeParentStyle(ih, &dwExStyle, &dwStyle);

  if (dwExStyle & WS_EX_COMPOSITED && !ih->data->is_multiline && iupwinIsVistaOrNew())
  {
    /* workaround for composite bug in Vista */
    ih->data->is_multiline = 1;  
    dwStyle |= TCS_MULTILINE;
  }

  if (!iupwinCreateWindowEx(ih, WC_TABCONTROL, dwExStyle, dwStyle))
    return IUP_ERROR;

  /* replace the WinProc to handle other messages */
  IupSetCallback(ih, "_IUPWIN_CTRLPROC_CB", (Icallback)winTabsProc);

  /* Process WM_NOTIFY */
  IupSetCallback(ih, "_IUPWIN_NOTIFY_CB", (Icallback)winTabsWmNotify);

  /* Process background color */
  IupSetCallback(ih, "_IUPWIN_CTLCOLOR_CB", (Icallback)winTabsCtlColor);

  if (iupwin_comctl32ver6 && ih->data->type != ITABS_TOP)
  {
    /* XP Styles support only TABTYPE=TOP */ 
    iupwinDrawRemoveTheme(ih->handle);
  }

  /* Change children background */
  if (winTabsUsingXPStyles(ih))
  {
    char* color = iupAttribGetInheritNativeParent(ih, "BGCOLOR");
    if (!color) 
      color = iupAttribGetInheritNativeParent(ih, "BACKGROUND");
    if (!color)
    {
      COLORREF cr;
      if (iupwinDrawGetThemeTabsBgColor(ih->handle, &cr))
        iupAttribSetStrf(ih, "BACKGROUND", "%d %d %d", (int)GetRValue(cr), (int)GetGValue(cr), (int)GetBValue(cr));
    }
  }

  /* Create pages and tabs */
  if (ih->firstchild)
  {
    Ihandle* child;
    for (child = ih->firstchild; child; child = child->brother)
      winTabsChildAddedMethod(ih, child);
  }

  return IUP_NOERROR;
}

static void winTabsUnMapMethod(Ihandle* ih)
{
  Iarray* iarray;

  HIMAGELIST image_list = (HIMAGELIST)SendMessage(ih->handle, TCM_GETIMAGELIST, 0, 0);
  if (image_list)
    ImageList_Destroy(image_list);

  iarray = (Iarray*)iupAttribGet(ih, "_IUPWIN_BMPARRAY");
  if (iarray)
    iupArrayDestroy(iarray);

  iarray = (Iarray*)iupAttribGet(ih, "_IUPWIN_VISIBLEARRAY");
  if (iarray)
    iupArrayDestroy(iarray);

  iupdrvBaseUnMapMethod(ih);
}

static void winTabsRegisterClass(void)
{
  WNDCLASS wndclass;
  ZeroMemory(&wndclass, sizeof(WNDCLASS));
  
  wndclass.hInstance      = iupwin_hinstance;
  wndclass.lpszClassName  = "IupTabsPage";
  wndclass.lpfnWndProc    = (WNDPROC)winTabsPageWinProc;
  wndclass.hCursor        = LoadCursor(NULL, IDC_ARROW);
  wndclass.style          = CS_PARENTDC;
  wndclass.hbrBackground  = NULL;  /* remove the background to optimize redraw */
   
  RegisterClass(&wndclass);
}

void iupdrvTabsInitClass(Iclass* ic)
{
  if (!iupwinClassExist("IupTabsPage"))
    winTabsRegisterClass();

  /* Driver Dependent Class functions */
  ic->Map = winTabsMapMethod;
  ic->UnMap = winTabsUnMapMethod;
  ic->ChildAdded     = winTabsChildAddedMethod;
  ic->ChildRemoved   = winTabsChildRemovedMethod;

  /* Driver Dependent Attribute functions */

  /* Visual */
  iupClassRegisterAttribute(ic, "BGCOLOR", winTabsGetBgColorAttrib, winTabsSetBgColorAttrib, IUPAF_SAMEASSYSTEM, "DLGBGCOLOR", IUPAF_NO_SAVE|IUPAF_DEFAULT);

  /* Special */
  iupClassRegisterAttribute(ic, "FGCOLOR", NULL, NULL, IUPAF_SAMEASSYSTEM, "DLGFGCOLOR", IUPAF_NOT_MAPPED);

  /* IupTabs only */
  iupClassRegisterAttribute(ic, "TABTYPE", iupTabsGetTabTypeAttrib, winTabsSetTabTypeAttrib, IUPAF_SAMEASSYSTEM, "TOP", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "TABORIENTATION", iupTabsGetTabOrientationAttrib, NULL, IUPAF_SAMEASSYSTEM, "HORIZONTAL", IUPAF_READONLY|IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);  /* can not be set, depends on TABTYPE in Windows */
  iupClassRegisterAttribute(ic, "MULTILINE", winTabsGetMultilineAttrib, winTabsSetMultilineAttrib, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "TABTITLE", NULL, winTabsSetTabTitleAttrib, IUPAF_NO_DEFAULTVALUE|IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "TABIMAGE", NULL, winTabsSetTabImageAttrib, IUPAF_IHANDLENAME|IUPAF_NO_DEFAULTVALUE|IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "TABVISIBLE", NULL, winTabsSetTabVisibleAttrib, IUPAF_NO_DEFAULTVALUE|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "PADDING", iupTabsGetPaddingAttrib, winTabsSetPaddingAttrib, IUPAF_SAMEASSYSTEM, "0x0", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);

  /* necessary because transparent background does not work when not using visual styles */
  if (!iupwin_comctl32ver6)  /* Used by iupdrvImageCreateImage */
    iupClassRegisterAttribute(ic, "FLAT_ALPHA", NULL, NULL, IUPAF_SAMEASSYSTEM, "YES", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
}
