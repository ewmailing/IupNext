/** \file
* \brief Windows Driver hooks
*
* See Copyright Notice in iup.h
* $Id: winhook.c,v 1.1 2008-10-17 06:19:26 scuri Exp $
*/

#include <stdio.h>
#include <stdlib.h> 
#include <assert.h>

#include <windows.h>
#include <winuser.h>

#include "iglobal.h"
#include "imask.h"
#include "win.h"
#include "winproc.h"
#include "winhandle.h"
#include "winhook.h"

/* hook for K_ANY management */

static int winhook_disablehooks = 0;
static HHOOK winhook_OldGetMessageHook = NULL;

void iupwinUnhook(void)
{
  winhook_disablehooks = 1;
}

void iupwinRehook(void)
{
  winhook_disablehooks = 0;
}

static LRESULT CALLBACK winHookGetMessageProc ( int code, WPARAM gm_wp, LPARAM gm_lp )
{
  MSG* gm_msg = (MSG*)gm_lp;
  HWND hwnd = gm_msg->hwnd;
  UINT msg = gm_msg->message;
  WPARAM wp = gm_msg->wParam;
  LPARAM lp = gm_msg->lParam;
  Ihandle *n;

  if (winhook_disablehooks || !hwnd || code < 0)
    return CallNextHookEx(winhook_OldGetMessageHook, code, gm_wp, gm_lp);

  n = iupwinHandleGet(hwnd);
  if (!n)
    return CallNextHookEx(winhook_OldGetMessageHook, code, gm_wp, gm_lp);

  switch(msg)
  {
  case WM_KEYDOWN:
  case WM_SYSKEYDOWN:
    {
      /* ESC and ENTER keys are not being forwarded to canvas and text, so let's do it... */           
      if((type(n) == CANVAS_ || type(n) == TEXT_))
      {
        if(wp == VK_RETURN || wp == VK_ESCAPE)
          SendMessage(hwnd, msg, wp, lp);
      }

      if ((type(n)!=LABEL_) && (type(n)!=FRAME_) &&
          (type(n)!=ITEM_)  && (type(n)!=RADIO_) && 
          (type(n)!=MULTILINE_) && (type(n)!=TEXT_))
      {
        int result = iupwinKeyProcess ( n, (int)wp );
        if (result==IUP_CLOSE)
        {
          IupExitLoop();
        }
        else if (result==IUP_IGNORE)
        {
          msg = WM_DEADCHAR;
        }
      }

      break;
    }
  }

  return CallNextHookEx(winhook_OldGetMessageHook, code, gm_wp, gm_lp);
}

void iupwinHookInit(void)
{
  winhook_OldGetMessageHook = SetWindowsHookEx(WH_GETMESSAGE, 
                                               (HOOKPROC)winHookGetMessageProc,
                                               NULL, GetCurrentThreadId() );
}

void iupwinHookFinish(void)
{
  UnhookWindowsHookEx(winhook_OldGetMessageHook);
  winhook_OldGetMessageHook = NULL;
}
