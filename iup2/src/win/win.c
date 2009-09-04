/** \file
 * \brief Windows Driver Core
 *
 * See Copyright Notice in iup.h
 * $Id: win.c,v 1.3 2009-09-04 13:32:43 scuri Exp $
 */

#include <stdio.h>              /* NULL */
#include <stdlib.h>
#include <assert.h>
#include <string.h>             /* strlen */

#include <windows.h>
#include <commctrl.h>

#include "iglobal.h"
#include "ifunc.h"
#include "iglobalenv.h"
#include "idrv.h"
#include "icpi.h"
#include "idlglist.h"

#include "win.h"
#include "winproc.h"
#include "wincreat.h"
#include "winhandle.h"
#include "winhook.h"
#include "winver.h"


/************************************************************************
 *                       Message Loop Management
 ************************************************************************/

static int win_globalmouse = 0;
static int win_nvisiblewin = 0;  /* number of visible windows */
static IFidle win_idle_cb = NULL;

typedef enum
{
  WIN_NOMSG,
  WIN_MSG,
  WIN_CLOSE,
  WIN_IDLE
} tMsgResult;

typedef enum
{
  WIN_WAIT,
  WIN_NOWAIT
} tMsgMode;


int iupwinSetMouseHook(int v)
{
  int globalmouse = win_globalmouse;

  if (v == -1)
    return globalmouse;

  win_globalmouse = v;

  return globalmouse;
}

static int winIgnoreMessage( MSG msg )
{
  char name[30]="";
  char parentname[30]="";
  HWND hwnd = msg.hwnd;
  int result = 0;
  int bypass = 1;

  if (hwnd)
  {
    HWND hparent = GetParent(msg.hwnd);
    int m=0, n = GetClassName(msg.hwnd, name, 30);
    if (hparent)
    {
      m=GetClassName(hparent, parentname, 30);
      hwnd = hparent;
    }
    if ( (n && strnicmp(name, "IUP", 3)==0) ||
      (m && strnicmp(parentname, "IUP", 3)==0))
    {
      bypass = 0;
    }
  }

  /* Why to call IsDialogMessage:
  Although the IsDialogMessage function is 
  intended for modeless dialog boxes, 
  you can use it with any window that contains controls, 
  enabling the windows to provide 
  the same keyboard selection as is used in a dialog box. 
  IupDialog is a standard window, but should behave as modeless dialog boxes. */

  /* If IsDialogMessage is called, then: 
  Because the IsDialogMessage function performs 
  all necessary translating and dispatching of messages, 
  a message processed by IsDialogMessage 
  must not be passed to the TranslateMessage or DispatchMessage functions. */

  /* But do it only for Registered IUP classes. */
  if(!bypass && hwnd && IsWindow(hwnd) && IsDialogMessage(hwnd,&msg)) 
    result = 1;

  return result;
}

static int winInterceptMsg(MSG* gm_msg)
{
  HWND hwnd = gm_msg->hwnd;
  UINT msg = gm_msg->message;
  WPARAM wp = gm_msg->wParam;
  LPARAM lp = gm_msg->lParam;
  Ihandle *n;
  static char report[12];
  int is_dblclk = 0, ret = 0;
  static int dragging = 0;

  if (!hwnd) return 0;

  switch(msg)
  {
  case WM_MOUSEMOVE:
    {
      IFniis cb;
      char *r=report;
      short int x, y;

      n = iupwinHandleGet(hwnd);
      if (!n) return 0;

      cb = (IFniis) IupGetCallback(n,"CMOTION_CB");
      if (!cb) return 0;

      x = LOWORD(lp);
      y = HIWORD(lp);

      if (!GetClassLongPtr ((HWND)handle(n), GCLP_HCURSOR))
        SetCursor(NULL);

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
      *r='\0';
      if (cb (n, x, y, report)==IUP_IGNORE)
        ret = 1;
      if (dragging)
      {
        if(!(wp & (MK_LBUTTON | MK_MBUTTON | MK_RBUTTON)))
        {
          dragging = 0;
          ReleaseCapture();
        }
      }
    }
    break;

  case WM_LBUTTONDBLCLK:
  case WM_MBUTTONDBLCLK:
  case WM_RBUTTONDBLCLK:
    is_dblclk = 1;
  case WM_LBUTTONDOWN:
  case WM_MBUTTONDOWN:
  case WM_RBUTTONDOWN:
    {
      IFniiiis cb;
      char *r=report;
      int b=0;
      short int x, y;

      n = iupwinHandleGet(hwnd);
      if (!n) return 0;

      cb = (IFniiiis) IupGetCallback(n,"CBUTTON_CB");
      if (!cb) return 0;

      x = LOWORD(lp);
      y = HIWORD(lp);

      if (msg==WM_LBUTTONDOWN || msg==WM_LBUTTONDBLCLK)
        b = IUP_BUTTON1;
      else if (msg==WM_MBUTTONDOWN || msg==WM_MBUTTONDBLCLK)
        b = IUP_BUTTON2;
      else if (msg==WM_RBUTTONDOWN || msg==WM_RBUTTONDBLCLK)
        b = IUP_BUTTON3;

      if (wp & MK_SHIFT)   *r++= 'S';
      else                 *r++= ' ';
      if (wp & MK_CONTROL) *r++= 'C';
      else                 *r++= ' ';
      if (wp & MK_LBUTTON) *r++= IUP_BUTTON1;
      else                *r++= ' ';
      if (wp & MK_MBUTTON) *r++= IUP_BUTTON2;
      else                *r++= ' ';
      if (wp & MK_RBUTTON) *r++= IUP_BUTTON3;
      else                 *r++= ' ';

      if (is_dblclk)
        *r++ = 'D';

      *r='\0';

      SetCapture(hwnd);
      dragging = 1;
      ret = cb (n, b, 1, x, y, report);
      if (ret == IUP_CLOSE)
      {
        IupExitLoop();
        ReleaseCapture();
        dragging = 0;
      }
      else if (ret == IUP_IGNORE)
        ret = 1;

      if (dragging)
        SendMessage(hwnd,WM_SETCURSOR,(WPARAM)hwnd,MAKELPARAM(1,WM_MOUSEMOVE));
    }
    break;
  case WM_LBUTTONUP:
  case WM_MBUTTONUP:
  case WM_RBUTTONUP:
    {
      IFniiiis cb;
      char *r=report;
      int b=0;
      short int x, y;

      n = iupwinHandleGet(hwnd);
      if (!n) return 0;

      cb = (IFniiiis) IupGetCallback(n,"CBUTTON_CB");
      if (!cb) return 0;

      x = LOWORD(lp);
      y = HIWORD(lp);

      if (msg==WM_LBUTTONUP)
        b = IUP_BUTTON1;
      else if (msg==WM_MBUTTONUP)
        b = IUP_BUTTON2;
      else if (msg==WM_RBUTTONUP)
        b = IUP_BUTTON3;

      if (wp & MK_SHIFT)   *r++= 'S';  
      else                 *r++= ' ';
      if (wp & MK_CONTROL) *r++= 'C';
      else                 *r++= ' ';
      if (wp & MK_LBUTTON) *r++= IUP_BUTTON1;
      else                *r++= ' ';
      if (wp & MK_MBUTTON) *r++= IUP_BUTTON2;
      else                *r++= ' ';
      if (wp & MK_RBUTTON) *r++= IUP_BUTTON3;
      else                 *r++= ' ';

      *r='\0';

      if (dragging)
      {
        ReleaseCapture();
        dragging=0;
      }

      ret = cb (n, b, 0, x, y, report);
      if (ret == IUP_CLOSE)
        IupExitLoop();
      else if (ret == IUP_IGNORE)
        ret = 1;

      SendMessage(hwnd,WM_SETCURSOR,(WPARAM)hwnd,MAKELPARAM(1,WM_MOUSEMOVE));
    }
    break;
  }

  return ret;
}

static tMsgResult winLoopStep(tMsgMode mode)
{                                          
  MSG Message;

  if (PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
  {
    if (win_globalmouse && winInterceptMsg(&Message))
      return WIN_MSG;
    if (Message.message == WM_QUIT)  /* IUP_CLOSE returned or IupHide in a popup dialog */
      return WIN_CLOSE;
    else if (!winIgnoreMessage(Message))
    {
      TranslateMessage(&Message);
      DispatchMessage(&Message);
      return WIN_MSG;
    }
    else
      return WIN_MSG;
  }
  else if (win_nvisiblewin==0) /* no visible dialogs ends the message loop */
  {
    char *ll = IupGetGlobal(IUP_LOCKLOOP);
    if(ll && iupStrEqual(ll, "YES"))
      return WIN_MSG;
    else
      return WIN_CLOSE;
  }
  else if (win_idle_cb)
  {
		if (win_idle_cb() == IUP_CLOSE) 
      return WIN_CLOSE;

    return WIN_IDLE;
  }
  else if (mode == WIN_WAIT)
  {
    int ret = GetMessage(&Message, NULL, 0, 0);
    if (ret != 0 && ret != -1)
    {
      if (win_globalmouse && winInterceptMsg(&Message))
        return WIN_MSG;

      if (!winIgnoreMessage(Message))
      {       
        TranslateMessage(&Message);
        DispatchMessage(&Message);
      }

      return WIN_MSG;
    }
    else
      return WIN_CLOSE;
  }
  else if(mode == WIN_NOWAIT)
    return WIN_NOMSG;

  assert(0); /* nao e' para chegar aqui */
  return WIN_MSG;
}

void iupdrvSetIdleFunction(Icallback f)
{
  win_idle_cb = (IFidle)f;
}

void IupFlush(void)
{
  while(winLoopStep(WIN_NOWAIT) == WIN_MSG);
}

int IupLoopStep(void)
{
  switch(winLoopStep(WIN_NOWAIT))
  {
  case WIN_NOMSG:
  case WIN_IDLE:
  case WIN_MSG:
    return IUP_DEFAULT;

  case WIN_CLOSE:
    return IUP_CLOSE;
  }

  assert(0); /* nao e' para chegar aqui */
  return IUP_DEFAULT;
}

int IupMainLoop (void)
{
  while (winLoopStep(WIN_WAIT) != WIN_CLOSE);
  return IUP_NOERROR;
}

void IupExitLoop(void)
{
  PostQuitMessage(0);
}

/************************************************************************
*                       Show/Hide/Popup Management
************************************************************************/

static int win_popup_level = 1;

static void winDisableVisible(Ihandle* ih_popup)
{
  Ihandle *ih;
  for (ih = iupDlgListFirst(); ih; ih = iupDlgListNext())
  {
    if (ih != ih_popup && 
        handle(ih) &&
        IsWindowVisible((HWND)handle(ih)) && 
        !iupGetEnv(ih, "_IUPWIN_POPUP_LEVEL"))
    {
      char str[50];
      EnableWindow((HWND)handle(ih),FALSE);
      sprintf(str, "%d", win_popup_level);
      iupStoreEnv(ih, "_IUPWIN_POPUP_LEVEL", str);
    }
  }
  win_popup_level++;
}

static void winEnableVisible(void)
{
  Ihandle *ih;
  for (ih = iupDlgListFirst(); ih; ih = iupDlgListNext())
  {
    if (handle(ih))
    {
      int popup_level = iupGetEnvInt(ih, "_IUPWIN_POPUP_LEVEL");
      if (popup_level == win_popup_level-1)
      {
        EnableWindow((HWND)handle(ih),TRUE);
        iupSetEnv(ih, "_IUPWIN_POPUP_LEVEL", NULL);
      }
    }
  }
  win_popup_level--;
}

void iupwinAdjustPos(Ihandle* n, int* x, int* y, int width, int height)
{
  RECT rect;
  POINT CursorPoint;
  HWND hClient = NULL;
  if (iupCheck(n,"MDICHILD")==YES)
  {
    Ihandle* client = (Ihandle*)iupGetEnv(n, "_IUPWIN_MDICLIENT");
    if (client)
    {
      if (*x == IUP_CURRENT || *y == IUP_CURRENT)
        return;

      hClient = (HWND)handle(client);
      GetWindowRect(hClient, &rect);
    }
    else
      GetWindowRect(GetDesktopWindow(), &rect);
  }
  else
    GetWindowRect(GetDesktopWindow(), &rect);

  GetCursorPos(&CursorPoint);

  switch (*x)
  {
  case IUP_CURRENT:
  case IUP_CENTER:
    *x = ((rect.right-rect.left) - width)/2 + rect.left;
    break;
  case IUP_LEFT:
    *x = 0;
    break;
  case IUP_RIGHT:
    *x = rect.right - width;
    break;
  case IUP_MOUSEPOS:
    *x = CursorPoint.x;
  }

  switch (*y)
  {
  case IUP_CURRENT:
  case IUP_CENTER:
    *y = ((rect.bottom-rect.top) - height)/2 + rect.top;
    break;
  case IUP_TOP:
    *y = 0;
    break;
  case IUP_BOTTOM:
    *y = rect.bottom - height;
    break;
  case IUP_MOUSEPOS:
    *y = CursorPoint.y;
  }

  if (hClient)
  {
    POINT p;
    p.x = *x;
    p.y = *y;
    ScreenToClient(hClient, &p);
    *x = p.x;
    *y = p.y;
  }
}

static void winRefreshChildSize( Ihandle* n)
{
  Ihandle* aux;
  foreachchild(aux, n)
  {
    if (type(aux)== HBOX_ || type(aux)==VBOX_ || type(aux)==ZBOX_ || type(aux)==FRAME_)
      winRefreshChildSize(aux);
    else if (type(aux) == CANVAS_)
    {
      RECT rect;
      GetWindowRect((HWND)handle(aux),&rect);
      SendMessage( (HWND)handle(aux), WM_SIZE, (WPARAM)SIZE_RESTORED, MAKELPARAM(rect.right-rect.left, rect.bottom-rect.top) );
    }
  }
}

static void winShowXY(Ihandle* n, int x, int y)
{
  int CmdShow = SW_SHOWNORMAL;
  int Restored = 0;
  int was_hidden;
  char* value;

  IupMap(n);

  value = IupGetAttribute(n, "PLACEMENT");
  if(value && iupStrEqual(value, "MAXIMIZED"))
  {
    CmdShow = SW_SHOWMAXIMIZED;
    IupSetAttribute(n, "PLACEMENT", "NORMAL");
  }
  else if(value && iupStrEqual(value, "MINIMIZED"))
  {
    CmdShow = SW_SHOWMINIMIZED;
    IupSetAttribute(n, "PLACEMENT", "NORMAL");
  }
  else if(value && iupStrEqual(value, "FULL"))
  {
    RECT rect;
    /* client area is the size of the screen */
    GetWindowRect(GetDesktopWindow(), &rect);
    currentwidth(n) = rect.right - rect.left;
    currentheight(n) = rect.bottom - rect.top;

    x = -iupwinDialogDecorLeft(n);
    y = -iupwinDialogDecorTop(n);

    IupSetAttribute(n, "PLACEMENT", "NORMAL");
  }

  if (iupCheck(n, "FULLSCREEN")!=YES)
  {
    /* if NOT full screen*/
    UINT flags = 0;
    int width  = currentwidth(n) + iupwinDialogDecorX(n);
    int height = currentheight(n) +  iupwinDialogDecorY(n);
    iupwinAdjustPos(n, &x, &y, width, height);
    if (x == IUP_CURRENT || y == IUP_CURRENT) /* if iupwinAdjustPos did not change x and y */ 
      flags = SWP_NOMOVE;
    winRefreshChildSize(n);
    SetWindowPos((HWND)handle(n), HWND_TOP, x, y, width, height, flags);
  }

  was_hidden = !IsWindowVisible((HWND)handle(n));

  if ((IsIconic((HWND)handle(n)) && CmdShow != SW_SHOWMINIMIZED) ||
    (IsZoomed((HWND)handle(n)) && CmdShow != SW_SHOWMAXIMIZED))
    Restored = 1;

  ShowWindow ((HWND)handle(n), CmdShow);
  UpdateWindow ((HWND)handle(n));

  if (was_hidden)
  {
    win_nvisiblewin++;

    iupSetEnv(n, "_IUPWIN_SHOWING", IUP_YES);
    iupdrvSetAttribute(n, IUP_VISIBLE, IUP_YES);
    iupSetEnv(n, "_IUPWIN_SHOWING", NULL);
  }

  if (iupGetEnv(n, "_IUPWIN_POPUP_LEVEL"))
  {
    /* was disabled by a Popup, re-enable and reposition */
    EnableWindow((HWND)handle(n),TRUE);
    iupSetEnv(n, "_IUPWIN_POPUP_LEVEL", NULL); /* is at the current popup level */
  }

  {
    IFni show_cb = (IFni)IupGetCallback(n, IUP_SHOW_CB);
    if(show_cb)
    {
      int status = (CmdShow==SW_SHOWMINIMIZED)? IUP_MINIMIZE: (Restored? IUP_RESTORE: IUP_SHOW);
      show_cb(n, status);
    }
  }
}

static int winPopupDialog(Ihandle* n, int x, int y)
{
  HWND last_hwnd;
  HWND popup_hwnd;

  Ihandle* parent = IupGetAttributeHandle(n, IUP_PARENTDIALOG);
  if (parent && handle(parent))
    last_hwnd = handle(parent);
  else
    last_hwnd = GetActiveWindow();

  winShowXY(n, x, y);

  if (iupGetEnv(n, "_IUPWIN_POPUP"))
    return IUP_ERROR;

  popup_hwnd = (HWND)handle(n);

  /* disable all visible dialogs, and mark popup level */
  /* will be enable by IupHide */
  winDisableVisible(n);

  iupSetEnv(n, "_IUPWIN_POPUP", "1");  /* mark window as popup so IupHide can also detect it */

  /* interrupt processing here */
  IupMainLoop();

  /* if window is still valid (IupDestroy not called) */
  if (IsWindow(popup_hwnd))
  {
    iupSetEnv(n, "_IUPWIN_POPUP", "2"); 
    IupHide(n); /* hide the popup to update the disabled windows */
                /* but do not call IupExitLoop again */

    iupSetEnv(n, "_IUPWIN_POPUP", NULL); /* unmark the window */
  }

  /* activate the previous active window */
  if (last_hwnd) 
    SetActiveWindow(last_hwnd);

  return IUP_NOERROR;
}

static int winPopupMenu( Ihandle* h, int x, int y )
{
  HWND activehwnd = GetActiveWindow();
  HMENU hmenu = (HMENU) handle(h);
  int tray_menu = 0;
  POINT pt;
  int menuId;

  if (!hmenu)
  {
    iupwinCreatePopupMenu(h);
    hmenu = (HMENU)handle(h);
    if (!hmenu)
      return IUP_ERROR;
  }

  GetCursorPos( &pt );
  if (x == IUP_MOUSEPOS)
    x = pt.x;
 if (y == IUP_MOUSEPOS)
    y = pt.y;

  if (!activehwnd)
  {
    /* search for a valid handle */
    Ihandle* dlg = iupDlgListFirst();

    do 
    {
      if (handle(dlg))
      {
        activehwnd = handle(dlg);
        if (iupCheck(dlg, "TRAY")==YES) /* keep searching for a "TRAY" dialog */
          break;
      }
      dlg = iupDlgListNext();
    } while (dlg);
  }

  /* Necessary to avoid tray dialogs to lock popup menus (they get sticky after the 1st time) */
  if (activehwnd)
  {
    Ihandle* dlg = iupwinHandleGet(activehwnd);
    if (dlg && iupCheck(dlg, "TRAY")==YES)
    {
      /* To display a context menu for a notification icon, 
         the current window must be the foreground window. */
      SetForegroundWindow(activehwnd);
      tray_menu = 1;
    }
  }

  /* stop processing here. messages will not go to the message loop */
  menuId = TrackPopupMenu(hmenu, TPM_LEFTALIGN|TPM_RIGHTBUTTON|TPM_RETURNCMD, x, y, 0, activehwnd, NULL);

  if (tray_menu)
  {
    /* You must force a task switch to the application that 
       called TrackPopupMenu at some time in the near future. 
       This is done by posting a benign message to the window. */
    PostMessage(activehwnd, WM_NULL, 0, 0);
  }

  if (menuId)
  {
    IFni cb;
    Ihandle* n = iupwinGetMenuIhandle(menuId);
    if (!n) return IUP_NOERROR;

    cb = (IFni) IupGetCallback(n, IUP_ACTION);

    assert(number(n)>0);
    if (cb)
    {
      int ret = cb(n, GetMenuState ((HMENU)handle(n), number(n), MF_BYCOMMAND) & MF_CHECKED);
      if (ret == IUP_CLOSE)
        IupExitLoop();
    }
  }

  return IUP_NOERROR;
}

int IupHide (Ihandle* n)
{
  assert(n != NULL);
  if (n == NULL)
    return IUP_ERROR;

  /* If not dialog, use default implementation */
  if (type(n) != DIALOG_)
  {
    IupSetAttribute(n, IUP_VISIBLE, IUP_NO);
    return IUP_NOERROR;
  }

  if (IsWindowVisible((HWND)handle(n)))
  {
    /* if called IupHide for a Popup window */
    int popup = iupGetEnvInt(n, "_IUPWIN_POPUP");
    if (popup)
    {
      /* must first enable all visible dialogs at the marked popup level */
      /* must be before hiding the window */
      winEnableVisible();

      if (popup == 1)
        IupExitLoop();
    }

    win_nvisiblewin--;
    ShowWindow((HWND)handle(n), SW_HIDE);
  }

  return IUP_NOERROR;
}

int IupShowXY (Ihandle* n, int x, int y)
{
  assert(n != NULL);
  if(n == NULL)
    return IUP_ERROR;

  /* If not dialog, use default implementation */
  if (type(n) != DIALOG_)
  {
    IupSetAttribute(n, IUP_VISIBLE, IUP_YES);
    return IUP_NOERROR;
  }

  winShowXY(n, x, y);

  return IUP_NOERROR;
}

int IupShow (Ihandle* n)
{
  assert(n != NULL);
  if(n == NULL)
    return IUP_ERROR;

  if (handle(n))
    return IupShowXY( n, IupGetInt(n, IUP_X), IupGetInt(n, IUP_Y));
  else
    return IupShowXY( n, IUP_CURRENT, IUP_CURRENT );
}

int IupPopup (Ihandle* n, int x, int y)
{
  assert(n != NULL);
  if(n == NULL)
    return IUP_ERROR;

  if (hclass(n) != NULL)
    return iupCpiPopup(n,x,y);
  else if (type(n) == MENU_)
    return winPopupMenu(n,x,y);
  else if (type(n) == DIALOG_)
    return winPopupDialog(n,x,y);

  return IUP_ERROR;
} 


/************************************************************************
*                       Open/Close/Map and others
************************************************************************/


int IupMap (Ihandle* n)
{
  assert(n != NULL);
  if (n == NULL)
    return IUP_ERROR;

  if (handle(n) == NULL)
  {
    iupSetEnv(n,"_IUPWIN_IS_MAPPING",IUP_YES);
    iupdrvCreateObjects (n);
    if (iupSetSize (n) == IUP_ERROR)
      return IUP_ERROR;
    iupdrvResizeObjects(n);

    if(type(n) == DIALOG_)
    {
      IFn map_cb = (IFn) IupGetCallback(n, IUP_MAP_CB);
      if(map_cb != NULL) map_cb(n);
    }

    iupSetEnv(n,"_IUPWIN_IS_MAPPING",NULL);
  }
  else if (child(n))
  {
    Ihandle* c = NULL ;

    foreachchild(c,n)
      IupMap(c);
  }

  return IUP_NOERROR;
}

Ihandle *IupSetFocus (Ihandle *n)
{
  Ihandle *h;
  HWND hwnd;

  if(n != NULL)
    hwnd = SetFocus( (HWND)handle(n) );
  else
    hwnd = SetFocus(NULL);

  if(hwnd == NULL)
    return NULL;

  h = iupwinHandleGet(hwnd);

  return h;
}

Ihandle *IupGetFocus (void)
{
  Ihandle *h;
  HWND hwnd = GetFocus();

  if(hwnd == NULL)
    return NULL;

  h = iupwinHandleGet(hwnd);

  return h;
}

static int iup_opened = 0;
int IupOpen (int *argc, char ***argv)
{
  (void)argc;
  (void)argv;

  if (iup_opened)
    return IUP_OPENED;
  iup_opened = 1;

  iupwin_hinstance = GetModuleHandle(NULL);

  CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

  {
    INITCOMMONCONTROLSEX InitCtrls;
    InitCtrls.dwSize = sizeof(INITCOMMONCONTROLSEX);
    InitCtrls.dwICC = ICC_WIN95_CLASSES;
    InitCommonControlsEx(&InitCtrls);  
  }

  iupNamesInit();
  iupFuncInit();
  iupLexInit();
  iupCpiInit();
  iupGlobalEnvInit(); 

  iupwinHandleInit();

  /* Iup default global atributes */
  if(IupGetLanguage() == NULL)
    IupSetLanguage(IUP_PORTUGUESE);

  IupSetGlobal("HINSTANCE", (char*) iupwin_hinstance);
  IupSetGlobal("VERSION", IupVersion());
  IupSetGlobal("COPYRIGHT",  IUP_COPYRIGHT );
  IupSetGlobal(IUP_DRIVER,  "Win32" );
  IupStoreGlobal( IUP_SYSTEM, iupwinGetSystemName());
  IupStoreGlobal( "SYSTEMVERSION", iupwinGetSystemVersion());
  IupStoreGlobal( IUP_SYSTEMLANGUAGE, iupwinGetSystemLanguage());
  IupStoreGlobal( IUP_COMPUTERNAME, iupwinGetComputerName());
  IupStoreGlobal( IUP_USERNAME, iupwinGetUserName());

  /* Windows default driver attribute */
  {
    static char sysfont[100];
    char* facename;
    int size = 8, dpi;
    HDC screen = GetDC(0);
    dpi = GetDeviceCaps(screen, LOGPIXELSX);
    ReleaseDC(0, screen);

    if(iupwinGetSystemMajorVersion() >= 4)
      facename = "Tahoma";
    else
      facename = "MS Sans Serif";

    if (dpi > 100) size += 2;

    sprintf(sysfont, "%s::%d", facename, size);
    IupSetGlobal("DEFAULTFONT", sysfont);
  }

  /* Initialization of internal CPI controls */
  IupFileDlgOpen();
  IupTimerOpen();
  IupSpinOpen();
  IupCboxOpen();
  IupSboxOpen();

  /* 
  * Gets dialog background color
  */
  {
    COLORREF color;
    int wincolor;
    unsigned char r, g, b;
    static char val[15];
    static char buffer[12];

    if(iupwinGetSystemMajorVersion() >= 4)
      wincolor = COLOR_BTNFACE;
    else
      wincolor = COLOR_WINDOW;

    sprintf(buffer,"%d", wincolor);
    IupSetGlobal("_IUPWIN_DLGBGCOLOR", buffer);

    color = GetSysColor(wincolor);
    r = GetRValue(color);
    g = GetGValue(color);
    b = GetBValue(color);
    sprintf(val,"%d %d %d", (int)r, (int)g, (int)b);
    IupSetGlobal("DLGBGCOLOR", val);
  }

  iupwinSetInit();
  iupwinHookInit();
  iupwinCreatInit();

  {
    static int first = 1;
    char* env = getenv("IUP_QUIET");
    if (first && env && strcmp(env, "NO")==0)
    {
      printf("IUP %s %s\n", IupVersion(), IUP_COPYRIGHT);
      first = 0;
    }
  }

  if (getenv("IUP_VERSION"))
  {
    iupwinVersion();
  }

#ifdef __WATCOMC__ 
  {
    /* this is used to force Watcom to link the winmain.c module. */
    void iupwinMainDummy(void);
    iupwinMainDummy();
  }
#endif

  return IUP_NOERROR;
}

void IupClose (void)
{
  if (!iup_opened)
    return;
  iup_opened = 0;

  CoUninitialize();

  iupStrGetMemory(-1); /* Frees internal buffer */

  IupTimerClose();
  IupSpinClose();

  iupwinHandleFinish();
  iupwinFinish();

  iupGlobalEnvFinish();
  iupNamesFinish();
  iupLexFinish();
  iupCpiFinish();
  iupFuncFinish();
}

void IupUpdate(Ihandle* n)
{
  if (type(n) == CANVAS_)
    RedrawWindow((HWND)handle(n),NULL,NULL,RDW_ERASE|RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
  else
    RedrawWindow((HWND)handle(n),NULL,NULL,RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
}
