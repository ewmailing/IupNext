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

/* iupwinButtonProc is necessary only because of the BUTTON_CB callback,
   and enterwindow_cb/leavewindow_cb callbacks. */

static void winButtonCallButtonCb(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp, Ihandle *n)
{
  static char report[12];
  int is_dblclk = 0;
  IFniiiis cb;
     
  cb = (IFniiiis) IupGetCallback(n,IUP_BUTTON_CB);
  if (cb == NULL) return;

  switch (msg)
  {
    case WM_LBUTTONDBLCLK:
    case WM_MBUTTONDBLCLK:
    case WM_RBUTTONDBLCLK:
      is_dblclk = 1;
    case WM_LBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_RBUTTONDOWN:
    {
      char *r=report;
      int b=0;
      short int x, y;

      x = LOWORD(lp);
      y = HIWORD(lp);

      iupwinSetFocus(n);

      if (wp & MK_SHIFT)   *r++= 'S';
      else                 *r++= ' ';
      if (wp & MK_CONTROL) *r++= 'C';
      else                 *r++= ' ';
      if (wp & MK_LBUTTON) *r++= IUP_BUTTON1;
      else                 *r++= ' ';
      if (wp & MK_MBUTTON) *r++= IUP_BUTTON2;
      else                 *r++= ' ';
      if (wp & MK_RBUTTON) *r++= IUP_BUTTON3;
      else                 *r++= ' ';

      if (msg==WM_LBUTTONDOWN || msg==WM_LBUTTONDBLCLK)
        b = IUP_BUTTON1;
      else if (msg==WM_MBUTTONDOWN || msg==WM_MBUTTONDBLCLK)
        b = IUP_BUTTON2;
      else if (msg==WM_RBUTTONDOWN || msg==WM_RBUTTONDBLCLK)
        b = IUP_BUTTON3;

      if (is_dblclk)
        *r++ = 'D';

      *r='\0';

      if (cb (n, b, 1, x, y, report) == IUP_CLOSE)
      {
         ReleaseCapture();
         IupExitLoop();
      }

      break;
    }
    case WM_LBUTTONUP:
    case WM_MBUTTONUP:
    case WM_RBUTTONUP:
    {
      char *r=report;
      int b=0;
      short int x, y;

      x = LOWORD(lp);
      y = HIWORD(lp);

      if (wp & MK_SHIFT)   *r++= 'S';  
      else                 *r++= ' ';
      if (wp & MK_CONTROL) *r++= 'C';
      else                 *r++= ' ';
      if (wp & MK_LBUTTON) *r++= IUP_BUTTON1;
      else                 *r++= ' ';
      if (wp & MK_MBUTTON) *r++= IUP_BUTTON2;
      else                 *r++= ' ';
      if (wp & MK_RBUTTON) *r++= IUP_BUTTON3;
      else                 *r++= ' ';

      if (msg==WM_LBUTTONUP)
      {
        b = IUP_BUTTON1;
		    report[2] = IUP_BUTTON1;
      }
      else
      {
        if (msg==WM_MBUTTONUP)
        {
           b = IUP_BUTTON2;
		       report[3] = IUP_BUTTON2;
        }
        else
        {
           if (msg==WM_RBUTTONUP)
           {
              b = IUP_BUTTON3;
		       report[4] = IUP_BUTTON3;
           }
        }
      }

      *r='\0';

      if (cb (n, b, 0, x, y, report) == IUP_CLOSE)
        IupExitLoop();
    } 
  } 
}

LRESULT CALLBACK iupwinButtonProc (HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
  Ihandle *n;
  WNDPROC buttonoldproc = NULL;

  n = iupwinHandleGet(hwnd);
  if(n == NULL) 
    return DefWindowProc(hwnd, msg, wp, lp);

  buttonoldproc = (WNDPROC)IupGetAttribute(n, "_IUPWIN_BUTTONOLDPROC__");
  assert(buttonoldproc);
  if(!buttonoldproc)
    return 0;

  switch (msg)
  {
  case WM_ERASEBKGND:
    {
       LONG style = GetWindowLong(handle(n), GWL_STYLE);
       if (style & BS_OWNERDRAW || style & SS_OWNERDRAW)
         return 1;
      break;
    }

  case WM_NCHITTEST:
    if (type(n) == LABEL_)
      return HTCLIENT;  /* The static control returns an HTTRANSPARENT */
    break; 
  case WM_MOUSELEAVE:
    iupwinCallEnterLeaveWindow(n, 0);
    break;
  case WM_MOUSEMOVE:
    iupwinCallEnterLeaveWindow(n, 1);
    break;
  case WM_LBUTTONDBLCLK:
  case WM_MBUTTONDBLCLK:
  case WM_RBUTTONDBLCLK:
  case WM_LBUTTONDOWN:
  case WM_MBUTTONDOWN:
  case WM_RBUTTONDOWN:
  case WM_MBUTTONUP:
  case WM_RBUTTONUP:
  case WM_LBUTTONUP:
    if (type(n) == BUTTON_)
      winButtonCallButtonCb(hwnd, msg, wp, lp, n);
    break;
 }

  if (buttonoldproc)
    return CallWindowProc( buttonoldproc, hwnd, msg, wp, lp );
	else
    return 0;
}
