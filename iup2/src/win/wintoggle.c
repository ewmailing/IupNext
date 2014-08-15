/** \file
 * \brief Windows Driver Toggle management
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
#include "wintoggle.h"

/* iupwinToggleProc is necessary only when image is used. */

static int winToggleInactive(Ihandle *n)
{
  if(type(n) == TOGGLE_)
  {
    char *active = iupGetEnv(n, "ACTIVE");
    if(active != NULL && iupStrEqual(active, "NO"))
      return 1;
  }
  return 0;
}

LRESULT CALLBACK iupwinToggleProc (HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
  Ihandle *n;
  WNDPROC oldproc = NULL;

  n = iupwinHandleGet(hwnd);
  if(n == NULL) 
    return DefWindowProc(hwnd, msg, wp, lp);
  
  oldproc = (WNDPROC) IupGetAttribute(n, "_IUPWIN_TOGGLEOLDPROC__");

  /* we call our own proc to avoid the default proc */

  switch (msg)
  {
    case WM_MOUSEACTIVATE:
      if(winToggleInactive(n))
        return MA_NOACTIVATEANDEAT;
      break;
    case WM_ACTIVATE:
    case WM_SETFOCUS:
    case BM_CLICK:
    case BM_SETSTATE:
    case BN_CLICKED:
    case BN_DBLCLK:
    case WM_KEYUP:
      if(winToggleInactive(n)) 
        return 0;
      break;
  }

  return CallWindowProc(oldproc, hwnd, msg, wp, lp );
}

