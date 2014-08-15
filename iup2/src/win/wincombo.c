/** \file
 * \brief Windows Driver Button management
 *
 * See Copyright Notice in iup.h
 *  */

#include <assert.h>
#include <windows.h> 

#include "iglobal.h"
#include "win.h"
#include "winhandle.h"
#include "winbutton.h"
#include "winproc.h"

/* iupwinComboListProc is necessary because of comctl32 lib version 6.
   For some reason areas of the list box are not updated. So we have to redraw it manually.
   Unlike IupFrame this independs from the CLIPCHILDREN */

LRESULT CALLBACK iupwinComboListProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
  WNDPROC oldproc = NULL;
  Ihandle *n = NULL;

  n = iupwinHandleGet(hwnd);
  if(n == NULL)
    return DefWindowProc(hwnd, msg, wp, lp);

  if (msg==WM_ERASEBKGND)
  { 
    HDC hdc = (HDC)wp;
    COLORREF c;
    RECT rc;
    GetClientRect(hwnd, &rc); 
    iupwinGetColorRef(n, IUP_BGCOLOR, &c);
    FillRect(hdc, &rc, iupwinGetBrush(iupwinGetBrushIndex(c))); 
    return 1;
  }

  oldproc = (WNDPROC)IupGetAttribute(n, "_IUPWIN_COMBOLISTOLDPROC__");
  assert(oldproc);

  if (oldproc)
    return CallWindowProc( oldproc, hwnd, msg, wp, lp );
  else 
    return 0;
}
