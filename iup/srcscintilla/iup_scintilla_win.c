/*
* IupScintilla component
*
* Description : A source code editing component, 
* derived from Scintilla (http://www.scintilla.org/)
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>


#include <Scintilla.h>
#include <SciLexer.h>

#include <windows.h>

#include "iup.h"
#include "iup_scintilla.h"
#include "iupcbs.h"
#include "iup_key.h"

#include "iup_class.h"
#include "iup_object.h"
#include "iup_attrib.h"
#include "iup_str.h"
#include "iup_drv.h"
#include "iup_drvfont.h"
#include "iup_register.h"
#include "iup_layout.h"
#include "iup_assert.h"

#include "iupwin_drv.h"

#include "iupsci.h"


#define WM_IUPCARET WM_APP+1   /* Custom IUP message */

void iupdrvScintillaRefreshCaret(Ihandle* ih)
{
  PostMessage(ih->handle, WM_IUPCARET, 0, 0L);
}

int iupdrvScintillaGetBorder(void)
{
  int border_size = 2 * 3;
  return border_size;
}

void iupdrvScintillaOpen(void)
{
  Scintilla_RegisterClasses(IupGetGlobal("HINSTANCE"));
}

sptr_t IupScintillaSendMessage(Ihandle* ih, unsigned int iMessage, uptr_t wParam, sptr_t lParam)
{
  return SendMessage(ih->handle, iMessage, wParam, lParam);
}

static int winScintillaWmNotify(Ihandle* ih, NMHDR* msg_info, int *result)
{
  SCNotification *pMsg = (SCNotification*)msg_info;

  iupScintillaNotify(ih, pMsg);

  (void)result;
  return 0; /* result not used */
}

static int winScintillaMsgProc(Ihandle* ih, UINT msg, WPARAM wp, LPARAM lp, LRESULT *result)
{
  if (msg==WM_KEYDOWN) /* process K_ANY before text callbacks */
  {
    int ret = iupwinBaseMsgProc(ih, msg, wp, lp, result);
    if (ret) 
    {
      iupAttribSet(ih, "_IUPWIN_IGNORE_CHAR", "1");
      *result = 0;
      return 1;
    }
    else
      iupAttribSet(ih, "_IUPWIN_IGNORE_CHAR", NULL);
  }

  switch (msg)
  {
  case WM_CHAR:
    {
      /* even aborting WM_KEYDOWN, a WM_CHAR will be sent, so ignore it also */
      /* if a dialog was shown, the loop will be processed, so ignore out of focus WM_CHAR messages */
      if (GetFocus() != ih->handle || iupAttribGet(ih, "_IUPWIN_IGNORE_CHAR"))
      {
        iupAttribSet(ih, "_IUPWIN_IGNORE_CHAR", NULL);
        *result = 0;
        return 1;
      }

      break;
    }
  case WM_KEYDOWN:
    {
      PostMessage(ih->handle, WM_IUPCARET, 0, 0L);
      return 0;  /* already processed at the begining of this function */
    }
  case WM_KEYUP:
    {
      PostMessage(ih->handle, WM_IUPCARET, 0, 0L);
      break;
    }
  case WM_LBUTTONDBLCLK:
  case WM_MBUTTONDBLCLK:
  case WM_RBUTTONDBLCLK:
  case WM_LBUTTONDOWN:
  case WM_MBUTTONDOWN:
  case WM_RBUTTONDOWN:
    {
      if (iupwinButtonDown(ih, msg, wp, lp)==-1)
      {
        *result = 0;
        return 1;
      }
      PostMessage(ih->handle, WM_IUPCARET, 0, 0L);
      break;
    }
  case WM_MBUTTONUP:
  case WM_RBUTTONUP:
  case WM_LBUTTONUP:
    {
      if (iupwinButtonUp(ih, msg, wp, lp)==-1)
      {
        *result = 0;
        return 1;
      }
      PostMessage(ih->handle, WM_IUPCARET, 0, 0L);
      break;
    }
  case WM_IUPCARET:
    {
      iupScintillaCallCaretCb(ih);
      break;
    }
  case WM_MOUSEMOVE:
    {
      iupwinMouseMove(ih, msg, wp, lp);
      break;
    }
  }

  return iupwinBaseMsgProc(ih, msg, wp, lp, result);
}

/*****************************************************************************/

int idrvScintillaMap(Ihandle* ih)
{
  DWORD dwStyle = WS_CHILD | WS_CLIPSIBLINGS;
  DWORD dwExStyle = 0;

  if (!ih->parent)
    return IUP_ERROR;

  if (iupAttribGetBoolean(ih, "CANFOCUS"))
    dwStyle |= WS_TABSTOP;

  if (iupAttribGetBoolean(ih, "BORDER"))
    dwExStyle |= WS_EX_CLIENTEDGE;
  
  if (!iupwinCreateWindow(ih, TEXT("Scintilla"), dwExStyle, dwStyle, NULL))
    return IUP_ERROR;

  /* Process Scintilla Notifications */
  IupSetCallback(ih, "_IUPWIN_NOTIFY_CB", (Icallback)winScintillaWmNotify);

  /* Process BUTTON_CB, MOTION_CB and CARET_CB */
  IupSetCallback(ih, "_IUPWIN_CTRLMSGPROC_CB", (Icallback)winScintillaMsgProc);

  return IUP_NOERROR;
}

void iupdrvScintillaReleaseMethod(Iclass* ic)
{
  (void)ic;
  Scintilla_ReleaseResources();
}
