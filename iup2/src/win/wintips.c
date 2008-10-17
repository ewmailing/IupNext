/** \file
 * \brief Windows Driver TIPS management
 *
 * See Copyright Notice in iup.h
 * $Id: wintips.c,v 1.1 2008-10-17 06:19:26 scuri Exp $
 */

#include <windows.h>
#include <commctrl.h>
#include <assert.h>

#include "iglobal.h"
#include "win.h"
#include "winTips.h"

#ifndef TTM_SETMAXTIPWIDTH
#define TTM_SETMAXTIPWIDTH  (WM_USER + 24)
#endif

static HWND wintips_hwnd = NULL;

static void winTipsInit(void)
{
  wintips_hwnd = CreateWindowEx(WS_EX_TOPMOST,TOOLTIPS_CLASS, (LPSTR) NULL, TTS_ALWAYSTIP, 
    CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 
    NULL, (HMENU) NULL, iupwin_hinstance, NULL); 
}

static void winTipsAdd( Ihandle* h, char* msg )
{
  int shouldfree = 0;
  char *v = NULL;
  TOOLINFO ti;
  v = iupwinTransMultiline(msg, &shouldfree);

  ti.cbSize = sizeof(TOOLINFO); 
  ti.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
  ti.hwnd = NULL; 
  ti.hinst = iupwin_hinstance; 
  ti.uId = (UINT_PTR) handle(h); 
  ti.lpszText = (LPSTR) NULL;

  if(SendMessage(wintips_hwnd, TTM_GETCURRENTTOOL, 0, (LPARAM) (LPTOOLINFO) &ti) == 0)
  {
    ti.cbSize = sizeof(TOOLINFO); 
    ti.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
    ti.hwnd = NULL; 
    ti.hinst = iupwin_hinstance; 
    ti.uId = (UINT_PTR) handle(h); 
    ti.lpszText = (LPSTR) v;

    SendMessage(wintips_hwnd, TTM_ADDTOOL, 0, (LPARAM) (LPTOOLINFO) &ti);
    SendMessage(wintips_hwnd, TTM_UPDATETIPTEXT, 0, (LPARAM) (LPTOOLINFO) &ti);
    SendMessage(wintips_hwnd, TTM_SETMAXTIPWIDTH, 0, (LPARAM)(INT) 3000); 
  }
  else
  {
    ti.cbSize = sizeof(TOOLINFO); 
    ti.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
    ti.hwnd = NULL; 
    ti.hinst = iupwin_hinstance; 
    ti.uId = (UINT_PTR) handle(h); 
    ti.lpszText = (LPSTR) v;

    SendMessage(wintips_hwnd, TTM_ADDTOOL, 0, (LPARAM) (LPTOOLINFO) &ti);
    SendMessage(wintips_hwnd, TTM_UPDATETIPTEXT, 0, (LPARAM) (LPTOOLINFO) &ti);
    SendMessage(wintips_hwnd, TTM_SETMAXTIPWIDTH, 0, (LPARAM)(INT) 3000); 
  }

  if(shouldfree && v) free(v);
}

void iupwinTipsSet(Ihandle* h, char* msg)
{
  OSVERSIONINFO vi;

  if (handle(h) == NULL || type(h)==FILL_ || type(h)==HBOX_ || type(h)==VBOX_ || type(h)==FRAME_)
    return;

  vi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
  GetVersionEx(&vi);
  /* so tem efeito no Win32*/
  if (vi.dwPlatformId == VER_PLATFORM_WIN32s)
    return;

  if (wintips_hwnd == NULL)
    winTipsInit();

  if (msg != NULL)
    winTipsAdd(h,msg);
  else
    ShowWindow(wintips_hwnd, FALSE);
}

void iupwinTipsUpdate(Ihandle* h)
{
  char* tip = NULL;

  /* no native representation (frame?) */
  if (handle(h) == NULL || type(h)==FILL_ || type(h)==HBOX_ || type(h)==VBOX_ || type(h)==FRAME_)
    return;

  tip = iupGetEnv(h,"TIP");
  if (tip)
    iupwinTipsSet(h,tip);

}
