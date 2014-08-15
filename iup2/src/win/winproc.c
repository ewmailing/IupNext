/** \file
 * \brief Windows Driver windows procedures
 *
 * See Copyright Notice in iup.h
 *  */

#include <stdio.h>
#include <stdlib.h> 
#include <assert.h>

#include <windows.h>
#include <winuser.h>
#include <commctrl.h>

#include "iupkey.h"
#include "iglobal.h"
#include "imask.h"
#include "idrv.h"
#include "win.h"
#include "winproc.h"
#include "wincreat.h"
#include "winhandle.h"
#include "winbutton.h"
#include "winicon.h"
#include "winhook.h"
#include "wintoggle.h"

#ifndef WM_UNINITMENUPOPUP
#define WM_UNINITMENUPOPUP              0x0125
#endif

#ifndef BCN_HOTITEMCHANGE
typedef struct tagNMBCHOTITEM
{
    NMHDR   hdr;
    DWORD   dwFlags;           
} NMBCHOTITEM, * LPNMBCHOTITEM;
#define BCN_FIRST               (0U-1250U)
#define BCN_HOTITEMCHANGE       (BCN_FIRST + 0x0001)
#endif

#ifndef HICF_ENTERING
#define HICF_ENTERING       0x00000010          
#endif

/* tipos de dados utilizados internamente */

static void track_mouse(HWND hwnd, int in);

/*
 * Funcoes que centralizam codigo de chamadas de callbacks
 */

static void callGetfocusCb(Ihandle *n)
{
  Icallback cb = (Icallback)IupGetCallback(n, "GETFOCUS_CB");
  if (cb) cb(n);

  if (n && type(n) != DIALOG_)
    iupSetEnv(IupGetDialog(n), "_IUPWIN_LASTFOCUS", (char*)n);

  if (type(n) == CANVAS_)
  {
    IFni cb2 = (IFni)IupGetCallback(n, "FOCUS_CB");
    if (cb2) cb2(n, 1);
  }
}

static void callKillfocusCb(Ihandle *n)
{
  Icallback cb = IupGetCallback(n, IUP_KILLFOCUS_CB);
  if(cb) cb(n);

  if (type(n) == CANVAS_)
  {
    IFni cb2 = (IFni)IupGetCallback(n, "FOCUS_CB");
    if (cb2) cb2(n, 0);
  }
}

void iupwinCallEnterLeaveWindow(Ihandle *h, int enter)
{
  Icallback cb = NULL;

  if (enter)
  {
    track_mouse((HWND)handle(h), 0);

    if (!iupGetEnvInt(h, "_IUPWIN_ENTERWIN"))
    {
      cb = (Icallback) IupGetCallback(h,IUP_ENTERWINDOW_CB);
      iupSetEnv(h, "_IUPWIN_ENTERWIN", "1");

      if (type(h) == BUTTON_)
      {
        iupSetEnv(h, "_IUPWIN_MOUSEOVER", "YES");
        if (iupwinUseComCtl32Ver6() || iupCheck(h, "FLAT")==YES)
          RedrawWindow((HWND)handle(h), NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_FRAME);
      }
    }
  }
  else 
  {
    cb = (Icallback) IupGetCallback(h,IUP_LEAVEWINDOW_CB);
    iupSetEnv(h, "_IUPWIN_ENTERWIN", NULL);

    if (type(h) == BUTTON_)
    {
      iupSetEnv(h, "_IUPWIN_MOUSEOVER", NULL);
      if (iupwinUseComCtl32Ver6() || iupCheck(h, "FLAT")==YES)
        RedrawWindow((HWND)handle(h), NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_FRAME);
    }
  }

  if (cb && cb(h) == IUP_CLOSE )
  {
	  IupExitLoop();
    iupSetEnv(h, "_IUPWIN_ENTERWIN", NULL);
  }
}

/********************************************************************
** This function is called when the program terminates normally. 
** This is called at IupClose().
********************************************************************/
void iupwinFinish(void)
{
 /* libera hooks */
 iupwinHookFinish();
 
 /* deleta fontes windows alocadas */
 while(iupwin_fontinfo.num_winfonts)
 {
   DeleteObject(iupwin_fontinfo.fontlist[iupwin_fontinfo.num_winfonts-1].font);
   iupwin_fontinfo.num_winfonts--;
 }

 free(iupwin_fontinfo.fontlist);
 iupwin_fontinfo.fontlist = NULL;

 iupwinSetFinish(); /* Frees all brushes */
}

static void dlgmsgSize( Ihandle* h, int width, int height )
{
	char newsize[40];
	HWND hwnd = (HWND)handle(h);
  
  sprintf(newsize, "%ix%i", width, height);

  iupStoreEnv(h, IUP_RASTERSIZE, newsize);
  iupSetEnv(h, IUP_SIZE, NULL);

  if (iupSetSize (h)!=IUP_ERROR)
    iupdrvResizeObjects (child(h));

  InvalidateRect(hwnd, NULL, TRUE);

  if (iupCheck(h,"CLIPCHILDREN"))
  {
    /* Invalidate children and redraw itself with default window proc */
    RedrawWindow(hwnd, NULL, NULL, RDW_ALLCHILDREN | 
                                   RDW_INVALIDATE);
  }
}

int iupwinButtonCallCb(HWND hwnd, Ihandle *n)
{
  IFn cb = (IFn) IupGetCallback(n,IUP_ACTION);
	if (cb && cb(n) == IUP_CLOSE)
  {
	  IupExitLoop();
    return 0;
  }
  return 1;
}

static void callMaskCallback(Ihandle* h, int tipo)
{
  IFni cb;

  cb=(IFni)IupGetCallback(h,"MASK_CB");

  if (cb) cb(h,tipo);
  /* Se tipo = 0, um caracter invalido foi digitado */
  /* Se tipo = 1, o campo perdeu o foco, mas a mascara esta incompleta */
}

static void callListCallback(Ihandle *n, HWND hwnd, IFnsii cb, int item, int state)
{
  char *text = NULL;
  char item_name[10];

  if(item<=0)
    return;

  sprintf(item_name, "%d", item);
  text = IupGetAttribute(n, item_name);

  if (cb(n, text, item, state) == IUP_CLOSE)
    IupExitLoop();
}

static Ihandle *commandgetIhandle( HWND hwnd, WPARAM wp, LPARAM lp )
{
  Ihandle *n = NULL;

  /* HWND   - the dialog
     WPARAM - HIWORD is 0 if the message is from a menu.
              HIWORD is 1 if the message is from an accelerator.
              HIWORD is the notification code if the message is from a control.
              LOWORD is the identifier.
     LPARAM - the control sending the message or NULL.
  */

  if (HIWORD(wp)==0 && lp==0 && LOWORD(wp)>10)
  {
    n = iupwinGetMenuIhandle(LOWORD(wp));
    return n;
  }
  else /* control or accelerator */
  {
    if (lp==0)
    {
      if(hwnd)
        n = iupwinHandleGet(hwnd);  /* dialog */
      else
        n = NULL;
    }
    else
      n = iupwinHandleGet((HWND)lp); /* control */

    return n;
  }
}

static long iupwinDlgMsgCommand( HWND hwnd, WPARAM wp, LPARAM lp )
{
  Ihandle *n = commandgetIhandle(hwnd, wp, lp);
  if (n == NULL)
    return -1;
 
  if(type(n) == MULTILINE_ || type(n) == TEXT_)
  {
    switch (HIWORD(wp))
    {
    case EN_SETFOCUS:
      callGetfocusCb(n);
      break;
    case EN_KILLFOCUS:
      if (type(n) == TEXT_) /* CHECK FOR OLD MASK */
      {
        ITextInfo* txtinfo = (ITextInfo*) IupGetAttribute(n,"_IUPWIN_TXTINFO__");
        if (txtinfo)
        {
          if (txtinfo->status == 2) 
            callMaskCallback(n, 1);
          else
          {
            char* v = IupGetAttribute(n,IUP_VALUE);
            txtinfo->status = iupCheckMask(txtinfo->mask,v,NULL);
            if (txtinfo->status == 2)
              callMaskCallback(n, 1);
          }
        }
      }
      callKillfocusCb(n);
      break;
    }
  }
  else if(type(n) == DIALOG_)
  {
    if (HIWORD(wp) == 0)
    {
      char *button_name = NULL;
      char* default_but = LOWORD(wp) == IDOK? IUP_DEFAULTENTER: LOWORD(wp) == IDCANCEL? IUP_DEFAULTESC: NULL;

      if (LOWORD(wp) == IDOK)
      {
        Ihandle* focus = IupGetFocus();
        if (focus && type(focus) == BUTTON_)
        {
          iupwinButtonCallCb(hwnd, focus);
          return 0;
        }
      }

      button_name = IupGetAttribute(n, default_but);
      if(button_name)
      {
        Ihandle* bt = IupGetHandle(button_name);
        if(bt && type(bt) == BUTTON_)
          iupwinButtonCallCb(hwnd, bt);
      }
    }
  }
  else if(type(n) == TOGGLE_ || type(n) == BUTTON_)
  {
    switch (HIWORD(wp))
    {
    case BN_SETFOCUS:
      callGetfocusCb(n);
      break;
    case BN_KILLFOCUS:
      callKillfocusCb(n);
      break;
    case BN_DOUBLECLICKED:
      if (type(n) == BUTTON_)
        iupwinButtonCallCb(hwnd, n);
      break;
    case BN_CLICKED:
      {
        /* NOTICE: For automatic radio buttons, 
                   when it receives a WM_FOCUS message 
                   the parent window is sent a BN_CLICKED notification message. */

        if (type(n) == TOGGLE_)
        {
          IFni cb;
          Ihandle* radio = iupGetRadio(n);
          int check = (int) SendMessage((HWND)handle(n), BM_GETCHECK, 0, 0L);

          if (radio) 
          {
            if (check) 
            {
              /* If it is a group of toggles, take care of the unselected */
              Ihandle* last_tg = (Ihandle*)iupGetEnv(radio, "_IUPWIN_LASTTOGGLE");
              if (last_tg && last_tg != n)
              {
                iupSetEnv(last_tg, IUP_VALUE, IUP_OFF);
                SendMessage ((HWND)handle(last_tg), BM_SETCHECK, BST_UNCHECKED, 0L);
                iupwinUpdateAttr(last_tg, IUP_IMAGE);
                SetWindowLong((HWND)handle(last_tg), GWL_STYLE, (LONG)GetWindowLongPtr((HWND)handle(last_tg), GWL_STYLE)&(~WS_TABSTOP));

                cb = (IFni) IupGetCallback(last_tg,IUP_ACTION);
                if (cb)
                  if (cb (last_tg, 0) == IUP_CLOSE)
                    IupExitLoop();
              }
              iupSetEnv(radio, "_IUPWIN_LASTTOGGLE", (char*)n);

              /* sets the tabstop only for the checked toggle */
              SetWindowLong((HWND)handle(n), GWL_STYLE, (LONG)GetWindowLongPtr((HWND)handle(n), GWL_STYLE)|WS_TABSTOP);

              /* now handle the checked toggle */
              iupwinUpdateAttr(n, IUP_IMAGE);

              cb = (IFni) IupGetCallback(n,IUP_ACTION);
              if (cb)
                if (cb (n, check) == IUP_CLOSE)
                  IupExitLoop();
            }
            else
            {
              iupwinUpdateAttr(n, IUP_IMAGE);
            }
          }
          else
          {
            if (check == BST_INDETERMINATE)
              check = -1;

            /* simple toggle */
            iupwinUpdateAttr(n, IUP_IMAGE);

            cb = (IFni) IupGetCallback(n,IUP_ACTION);
            if (cb)
              if (cb (n, check) == IUP_CLOSE)
                IupExitLoop();
          }
        }
        else
        {
          iupwinButtonCallCb(hwnd, n);
        }
      }
    }
  }
  else if(type(n) == LIST_)
  {
    int haseditbox = iupCheck(n, "EDITBOX")==YES? 1: 0;
    int isdropdown = iupCheck(n, IUP_DROPDOWN)==YES? 1: 0;
    int iscbn = haseditbox || isdropdown;

    if ((iscbn && HIWORD(wp) == CBN_SETFOCUS) || (!iscbn && HIWORD(wp) == LBN_SETFOCUS))
      callGetfocusCb(n);
    else if ((iscbn && HIWORD(wp) == CBN_KILLFOCUS) || (!iscbn && HIWORD(wp) == LBN_KILLFOCUS))
      callKillfocusCb(n);
    else if ((iscbn && HIWORD(wp) == CBN_SELCHANGE) || (!iscbn && HIWORD(wp) == LBN_SELCHANGE) ||
             (iscbn && HIWORD(wp) == CBN_DBLCLK) || (!iscbn && HIWORD(wp) == LBN_DBLCLK))
    {
      IFnsii cb;
      IFns ms_cb;

      /* "_IUPWIN_LISTSEL" contains the value of the selection before
          the change, so that the user may receive information about
          the value that has changed */
      char *old_value = iupStrDup(iupGetEnv(n, "_IUPWIN_LISTSEL"));

      /* IUP_VALUE is calculated based on the current selection of 
          the list control */
      char *new_value = iupStrDup(IupGetAttribute(n, IUP_VALUE));

      if (haseditbox)
      {
        /* replace the VALUE returned by the EDITBOX by a normal index in the list. */
        char ed_value[50];
		    sprintf(ed_value, "%d",(int)(1+SendMessage((HWND)handle(n), CB_GETCURSEL, 0, 0L)));
        if (new_value) free(new_value);
        new_value = iupStrDup(ed_value);
      }

      cb = (IFnsii) IupGetCallback(n,IUP_ACTION);
      ms_cb = (IFns)IupGetCallback(n,"MULTISELECT_CB");
      if (cb || ms_cb)
      {
        if (iupCheck(n, IUP_MULTIPLE) == YES && !haseditbox)
        {
          int pos = 0;

          if (ms_cb)
          {
            char* ms_value = iupStrDup(new_value);
            if (old_value)
            {
              while(ms_value[pos] != '\0')
              {
                if(ms_value[pos] == old_value[pos])
                  ms_value[pos] = 'x';    /* mark unchanged values */
                pos++;
              }
            }

            if (ms_cb(n, ms_value) == IUP_CLOSE)
              IupExitLoop();

            free(ms_value);
          }
          else
          {
            /* call list callback for each changed item */
            while(new_value[pos] != '\0')
            {
              if (!old_value)
              {
                if(new_value[pos] == '+')
                  callListCallback(n, hwnd, cb, pos+1, 1);
              }
              else if(old_value &&
                  new_value[pos] == '+' &&
                  old_value[pos] == '-')
                callListCallback(n, hwnd, cb, pos+1, 1);
              else if(old_value &&
                  new_value[pos] == '-' &&
                  old_value[pos] == '+')
                callListCallback(n, hwnd, cb, pos+1, 0);
              pos++;
            }
          }
        }
        else if (cb)
        {
          int item;
          if (old_value)
          {
            item = atoi(old_value);
            callListCallback(n, hwnd, cb, item, 0);
          }
          item = atoi(new_value);
          callListCallback(n, hwnd, cb, item, 1);
        }
      }

      iupStoreEnv(n, "_IUPWIN_LISTSEL", new_value);
      if (old_value) free(old_value);
      if (new_value) free(new_value);
    }
  }
  else if(type(n) == ITEM_)
  {
    IFni cb = (IFni) IupGetCallback(n, IUP_ACTION);

    assert(number(n)>0);
    if (cb)
    {
      if (cb(n, 
        GetMenuState ((HMENU)handle(n), number(n), MF_BYCOMMAND) & MF_CHECKED) == IUP_CLOSE)
          IupExitLoop();
    }
  }

  return 0;
}

/********************************************************************
** This function is called whenever a window informs that the 
** background and foreground color of an element should be set.
** Unfortunatly, it is impossible to change the text color without
** changing the background color (the function uses the return
** value to verify that the user processes this message.) 
********************************************************************/
static LRESULT dlgmsgCtlColor( HWND hwnd, HWND hctl, HDC hdc )
{
	Ihandle *n = iupwinHandleGet(hctl); /* Gets Ihandle of window */

	if (n)
	{
		POINT p;
		COLORREF cr;
    int b = -1;

		if (iupwinGetColorRef (n, IUP_FGCOLOR, &cr))
      SetTextColor (hdc, cr);

		if (iupwinGetColorRef (n, IUP_BGCOLOR, &cr))
		  b = iupwinGetBrushIndex (cr);

		if (b >= 0)
		{
			SetBkColor (hdc, cr);
			UnrealizeObject (iupwinGetBrush(b));
			p.x=p.y=0;
			ClientToScreen(hwnd, &p);
			SetBrushOrgEx (hdc, p.x, p.y, NULL);
			return (LRESULT) iupwinGetBrush(b);
		}
	}

	return 0;
}

/********************************************************************
** This function deals drop files done on canvas and dialogs.
********************************************************************/
static void deal_dropfiles(HDROP hDrop, Ihandle *root)
{
  char *filename = NULL;
  int i, numFiles, numchar, ret;
  POINT point;
  numFiles = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);
  DragQueryPoint(hDrop, &point);  
  for (i=0; i < numFiles; i++)
  {
    numchar = DragQueryFile(hDrop, i, NULL, 0);
    filename = malloc(numchar+1); 
    if (filename==NULL)
      return;
    DragQueryFile(hDrop, i, filename, numchar+1);
    {
      IFnsiii cb = (IFnsiii)IupGetCallback(root, IUP_DROPFILES_CB);
      if (!cb) return; 

      ret = cb(root, filename, numFiles-i-1, (int) point.x, (int) point.y); 
      if (ret != IUP_DEFAULT)
        break;  /* for */
    }
    free(filename);
  }
  DragFinish(hDrop);
}

static Ihandle *GetSubMenuIhandle(Ihandle *n, HMENU hmenu)
{
  if(type(n) == MENU_)
  {
    Ihandle *c = NULL;
    foreachchild(c, n)
    {
      Ihandle *ret = GetSubMenuIhandle(c, hmenu);
      if(ret)
        return ret;
    }
  }
  else if(type(n) == SUBMENU_)
  {
    if(hmenu == handle(n))
      return n;
    else
      return GetSubMenuIhandle(child(n), hmenu);
  }
  return NULL;
}

static int callMenuSelectCb(Ihandle *n)
{
  Icallback cb = NULL;
  if(n && IupGetClassName(n) == ITEM_)
  {
    cb = IupGetCallback(n, IUP_HIGHLIGHT_CB);
    if(cb && cb(n) == IUP_CLOSE)
      return IUP_CLOSE;
  }
  return IUP_DEFAULT;
}

static void call_trayclick_cb(Ihandle *n, int button, int pressed, int dclick)
{
  IFniii cb = (IFniii)IupGetCallback(n, "TRAYCLICK_CB");
  if(cb && cb(n, button, pressed, dclick) == IUP_CLOSE)
    IupExitLoop();
}

static void callMenuCb(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp, char* cb_name)
{
  Ihandle* dg = iupwinHandleGet(hwnd);
  if(dg)
  {
    char* menu_name = IupGetAttribute(dg, IUP_MENU);
    if(menu_name)
    {
      Ihandle *main_menu = IupGetHandle(menu_name);
      if(main_menu)
      {
        HMENU hmenu = (HMENU)wp;
        Ihandle *n = GetSubMenuIhandle(main_menu, hmenu);
        if(n)
        {
          Icallback cb = (Icallback) IupGetCallback(n, cb_name);
          if(cb) cb(n);  /* do not handle IUP_CLOSE here */
        }
      }
    }
  }
}

static int callCloseCB(Ihandle* root)
{
  Icallback cb = IupGetCallback(root,"CLOSE_CB");
  if (cb)
  {
    int rst = cb(root);
    if (rst == IUP_CLOSE)
      IupExitLoop();
    return rst;
  
  }
  return IUP_DEFAULT;
}

int iupwinCloseChildren(Ihandle* client)
{
  HWND hwndT;

  /* As long as the MDI client has a child, close it */
  while (hwndT = GetWindow((HWND)handle(client), GW_CHILD))
  {
    Ihandle* child;

	  /* Skip the icon title windows */
	  while (hwndT && GetWindow (hwndT, GW_OWNER))
	    hwndT = GetWindow (hwndT, GW_HWNDNEXT);

	  if (!hwndT)
	      break;

    child = iupwinHandleGet(hwndT); 
    if (child)
    {
      if (callCloseCB(child) == IUP_IGNORE)
        return 0;
      else
        IupDestroy(child);
    }
  }

  return 1;
}

static LRESULT callDefaultWindowProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam, Ihandle* n)
{
  char cname[255];
  GetClassName(hWnd, cname, 255);

  if (cname[3] == 'F') /* "IUPFrameDlg" */
  {
    HWND hWndClient = NULL;
    if (n) 
    {
      Ihandle* client = (Ihandle*)iupGetEnv(n, "_IUPWIN_MDICLIENT");
      if (client) hWndClient = (HWND)handle(client);
    }

    return DefFrameProc(hWnd, hWndClient, Msg, wParam,lParam);
  }

  if (cname[3] == 'C') /* "IUPChildDlg" */
    return DefMDIChildProc(hWnd, Msg, wParam,lParam);

  return DefWindowProc(hWnd, Msg, wParam,lParam);  /* "IUPDlg" */
}

/* FIXME: should add support for 
  WM_XBUTTONDOWN
  WM_XBUTTONUP
  WM_XBUTTONDBLCLK
*/

LRESULT CALLBACK iupwinDialogProc (HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{ 
 Ihandle *root=NULL;
 long result = 0;

/* CBOX
 if (msg==WM_GETMINMAXINFO)
 {
   MINMAXINFO* minmax = (MINMAXINFO*)lp;
   minmax->ptMaxSize.x = 65535;
   minmax->ptMaxSize.y = 65535;
   minmax->ptMaxTrackSize.x = 65535;
   minmax->ptMaxTrackSize.y = 65535;
   return 0;
 }
*/

 root = iupwinHandleGet(hwnd); 
 if (!root)
   return callDefaultWindowProc(hwnd, msg, wp, lp, NULL);

 switch (msg)
 {
   case WM_MDIACTIVATE:
     {
       HWND hNewActive = (HWND)lp;
       if (hNewActive == hwnd)
       {
         Icallback cb = (Icallback) IupGetCallback(root, "MDIACTIVATE_CB");
         if (cb)
         {
           if(cb && cb(root) == IUP_CLOSE) 
             IupExitLoop();
         }
       }
       return callDefaultWindowProc(hwnd, msg, wp, lp, NULL);
     }
   case WM_HELP:
     {
       HELPINFO* help_info = (HELPINFO*)lp;
       Ihandle* n;
       if (help_info->iContextType == HELPINFO_MENUITEM)
         n = iupwinGetMenuIhandle((int) help_info->iCtrlId);
       else
         n = iupwinHandleGet((HWND)help_info->hItemHandle);

       if (n)
       {
         Icallback cb = (Icallback) IupGetCallback(n, IUP_HELP_CB);
         if(cb && cb(n) == IUP_CLOSE) 
           IupExitLoop();
       }
       break;
     }
   case WM_INITMENUPOPUP:
     {
       /* Implementation of IUP_OPEN_CB */
       callMenuCb(hwnd, msg, wp, lp, IUP_OPEN_CB);
       break;
     }
   case WM_UNINITMENUPOPUP:
     {
       /* Implementation of MENUCLOSE_CB para menu */
       callMenuCb(hwnd, msg, wp, lp, "MENUCLOSE_CB");
       break;
     }
   case WM_ENTERMENULOOP:
     {
       /* Simulate WM_KILLFOCUS when the menu interaction is started */
       Ihandle* lf = (Ihandle*)iupGetEnv(root, "_IUPWIN_LASTFOCUS");
       if (lf) callKillfocusCb(lf);
       break;
     }
   case WM_EXITMENULOOP:
     {
       /* Simulate WM_GETFOCUS when the menu interaction is stopped */
       Ihandle* lf = (Ihandle*)iupGetEnv(root, "_IUPWIN_LASTFOCUS");
       if (lf) callGetfocusCb(lf);
       break;
     }
   case WM_MENUSELECT:
     {
        Ihandle *n = NULL;
        UINT si = LOWORD(wp);

        if((HIWORD(wp) & MF_POPUP) || (HIWORD(wp) & MF_SYSMENU)) /* item */
        {
          HMENU hmenu = GetSubMenu((HMENU) lp, si);
          Ihandle *dg = iupwinHandleGet(hwnd);
          Ihandle *main_menu = IupGetAttributeHandle(dg, IUP_MENU);
          if(main_menu)
            n = GetSubMenuIhandle(main_menu, hmenu);
        }
        else /* menu or submenu */
          n = iupwinGetMenuIhandle((int) si);

        if(n)
        {
          if (callMenuSelectCb(n) == IUP_CLOSE)
            IupExitLoop();
        }
        else
          return callDefaultWindowProc(hwnd, msg, wp, lp, root);

       break;
     }

  case WM_DROPFILES:
    deal_dropfiles((HANDLE) wp, root);
    break;
    
  case WM_SIZE:
    if (iupCheck(root, "_IUPWIN_NOT_WM_SIZE") != YES)
	  {
      char* oldstate = IupGetAttribute(root,"_IUPWIN_MINIMIZE_STATE");
      IFni show_cb = (IFni)IupGetCallback(root, IUP_SHOW_CB);

      switch(wp)
      {
        case SIZE_MINIMIZED:
          if ( show_cb && (show_cb)(root, IUP_MINIMIZE)== IUP_CLOSE)
   	          IupExitLoop();
          IupSetAttribute(root,"_IUPWIN_MINIMIZE_STATE",IUP_YES);
          break;
        case SIZE_MAXIMIZED:
          if(oldstate)
          {
            if ( show_cb && (show_cb)(root, IUP_RESTORE)== IUP_CLOSE)
               IupExitLoop();
            IupSetAttribute(root,"_IUPWIN_MINIMIZE_STATE",NULL);
          }
	        dlgmsgSize(root,LOWORD(lp),HIWORD(lp));
          break;
        case SIZE_RESTORED:
          if(oldstate)
          {
            if ( show_cb && (show_cb)(root, IUP_RESTORE)== IUP_CLOSE)
               IupExitLoop();
            IupSetAttribute(root,"_IUPWIN_MINIMIZE_STATE",NULL);
          }
          else
		        dlgmsgSize(root,LOWORD(lp),HIWORD(lp));
          break;
        default:
		      dlgmsgSize(root,LOWORD(lp),HIWORD(lp));
          break;
      }
	  }

    /* must call DefaultWndProc because of MDI menu management */
    return callDefaultWindowProc(hwnd, msg, wp, lp, root);
  case WM_COMMAND:
    /* Should only execute WM_COMMAND messages if the Dialog is 
    ** already mapped */
    if( iupCheck(root,"_IUPWIN_IS_MAPPING") != YES )
    {
	    result = iupwinDlgMsgCommand(hwnd,wp,lp);
      if (result == -1)
  		  return callDefaultWindowProc(hwnd, msg, wp, lp, root);
    }
    else
		  return callDefaultWindowProc(hwnd, msg, wp, lp, root);
	  break;
  case WM_TRAY_NOTIFICATION:
  {
    int dclick  = 0;
    int button  = 0;
    int pressed = 0;
    int dont_call = 0;

		switch (lp)
    {
      case WM_LBUTTONDOWN:
        pressed = 1;
        button  = 1;
        break;
      case WM_MBUTTONDOWN:
        pressed = 1;
        button  = 2;
        break;
      case WM_RBUTTONDOWN:
        pressed = 1;
        button  = 3;
        break;
      case WM_LBUTTONDBLCLK:
        dclick = 1;
        button = 1;
        break;
      case WM_MBUTTONDBLCLK:
        dclick = 1;
        button = 2;
        break;
      case WM_RBUTTONDBLCLK:
        dclick = 1;
        button = 3;
        break;
      case WM_LBUTTONUP:
        button = 1;
        break;
      case WM_MBUTTONUP:
        button = 2;
        break;
      case WM_RBUTTONUP:
        button = 3;
        break;
      default:
        dont_call = 1;
    }

    if(!dont_call)
      call_trayclick_cb(root, button, pressed, dclick);
    
    break;
  }

  case WM_CTLCOLORBTN:
  case WM_CTLCOLOREDIT:
  case WM_CTLCOLORLISTBOX:
  case WM_CTLCOLORSTATIC:
	  return dlgmsgCtlColor( hwnd, (HWND)lp, (HDC)wp );
	  break;
  case WM_SYSCOMMAND:
  {
	  if (wp==SC_CLOSE)
	  {
      if (callCloseCB(root) == IUP_IGNORE)
        break;

      /* child mdi is automatically destroyed */
      if (iupCheck(root, "MDICHILD")==YES)
        IupDestroy(root);
      else
      {
        Ihandle* client = (Ihandle*)iupGetEnv(root, "_IUPWIN_MDICLIENT");
        if (client)
        {
          if (!iupwinCloseChildren(client))
            break;
        }

        IupHide(root); /* default processing */
      }
	  }
	  else 
		  return callDefaultWindowProc(hwnd, msg, wp, lp, root);
	  break;
  }
  case WM_DRAWITEM:
  {
    Ihandle *n;
    int ismenu = (UINT) wp;

    if(ismenu == 0) /* is a menu */
      n = iupwinGetMenuIhandle(wp);
    else
      n = iupwinHandleGet((HWND) (( (LPDRAWITEMSTRUCT)lp)->hwndItem ) ); 

    if(n && type(n) == BUTTON_)
      iupwinDrawButton(n, (LPDRAWITEMSTRUCT)lp);
		else if(n && type(n) == LABEL_)
      iupwinDrawLabel(n, (LPDRAWITEMSTRUCT)lp);

	  return TRUE;
  }
  case WM_SETFOCUS:
  {
    callGetfocusCb(root); /* dialog GETFOCUS */

    {
      /* this is updated in the getfocus_cb */
      Ihandle *last_focus_ctl = (Ihandle*)iupGetEnv(root, "_IUPWIN_LASTFOCUS");
      if (last_focus_ctl)
        IupSetFocus(last_focus_ctl);
    }
	  break;
  }
                        
  case WM_KILLFOCUS:
    callKillfocusCb(root);
	  break;

  case WOM_CLOSE:
  case WOM_DONE:
  case WOM_OPEN:
    {
      IFni cb = (IFni)IupGetCallback(root, IUP_WOM_CB);
      if(cb)
      {
        int v = -2; /* Error */
        switch(msg)
        {
        case WOM_OPEN:  v = 1;  break;
        case WOM_DONE:  v = 0;  break;
        case WOM_CLOSE: v = -1; break;
        }
        cb(root, v);
      }
      break;
    }

  default:
			 return callDefaultWindowProc(hwnd, msg, wp, lp, root);
 }

/* FIXME: Why not to alway call the DefaultWindowProc ?
  if (result == 0)
		return callDefaultWindowProc(hwnd, msg, wp, lp, root);
*/

 return result;
}

static void UpdateHorScroll( Ihandle* n, long op )
{
   HWND hwnd = (HWND)handle(n);
   IFniff cb = NULL;
   double posy, xmin, xmax;
   unsigned short page;

   xmax = IupGetFloat(n,IUP_XMAX);
   xmin = IupGetFloat(n,IUP_XMIN);
   page = (int) ((IupGetFloat(n,IUP_DX)/(xmax-xmin))*HORZ_SCROLLBAR_SIZE);
   if (page == 0)
      page = 1;

   posy = IupGetFloat(n,IUP_POSY);

   cb = (IFniff)IupGetCallback(n,IUP_SCROLL_CB);

   switch (LOWORD(op))
   {
      case SB_BOTTOM:
              break;
      case SB_ENDSCROLL:
      {
         char posattr[15];
         double posx;
         unsigned short realposx = GetScrollPos(hwnd, SB_HORZ);

		   if (realposx > (HORZ_SCROLLBAR_SIZE - page))
			   realposx = (HORZ_SCROLLBAR_SIZE-page);

         posx = xmin + realposx*(xmax-xmin)/HORZ_SCROLLBAR_SIZE;

         sprintf(posattr,"%f",posx);
         IupStoreAttribute(n,IUP_POSX,posattr);

         if (!cb)
            InvalidateRect(hwnd,NULL,FALSE);
         else
            cb(n,IUP_SBPOSH,(float)posx,(float)posy);
         break;
      }
      case SB_LINEDOWN:
      {
         int realposx = GetScrollPos(hwnd, SB_HORZ);
         realposx = realposx + (int)(0.1*page);
		   if (realposx > (HORZ_SCROLLBAR_SIZE - page))
			   realposx = (HORZ_SCROLLBAR_SIZE-page);

         SetScrollPos(hwnd,SB_HORZ,realposx,TRUE);

         if (cb)
         {
            double posx;
            posx = xmin + realposx*(xmax-xmin)/HORZ_SCROLLBAR_SIZE;
            cb(n,IUP_SBRIGHT,(float)posx,(float)posy);
         }
         break;
      }
      case SB_LINEUP:
      {
         int realposx = GetScrollPos(hwnd, SB_HORZ);
         realposx = realposx - (int)(0.1*page);
         if (realposx < 1)
            realposx = 1;
          
         SetScrollPos(hwnd,SB_HORZ,realposx,TRUE);

         if (cb)
         {
            double posx;
            posx = xmin + realposx*(xmax-xmin)/HORZ_SCROLLBAR_SIZE;
            cb(n,IUP_SBLEFT,(float)posx,(float)posy);
         }
         break;
      }
      case SB_PAGEDOWN:
      {
         int realposx = GetScrollPos(hwnd, SB_HORZ);
         realposx = realposx + page;
		   if (realposx > (HORZ_SCROLLBAR_SIZE - page))
			   realposx = (HORZ_SCROLLBAR_SIZE-page);

         SetScrollPos(hwnd,SB_HORZ,realposx,TRUE);
         if (cb)
         {
            double posx;
            posx = xmin + realposx*(xmax-xmin)/HORZ_SCROLLBAR_SIZE;
		      cb(n,IUP_SBPGRIGHT,(float)posx,(float)posy);
         }
         break;
      }
      case SB_PAGEUP:
      {
         int realposx = GetScrollPos(hwnd, SB_HORZ);
         realposx = realposx - page;
         if (realposx < 1)
            realposx = 1;
          
         SetScrollPos(hwnd,SB_HORZ,realposx,TRUE);
         if (cb)
         {
            double posx;
            posx = xmin + realposx*(xmax-xmin)/HORZ_SCROLLBAR_SIZE;
            cb(n,IUP_SBPGLEFT,(float)posx,(float)posy);
         }
         break;
      }
      case SB_THUMBPOSITION:
         break;
      case SB_THUMBTRACK:
      {
         unsigned short realposx = (unsigned short)HIWORD(op);
         SetScrollPos(hwnd,SB_HORZ,realposx,TRUE);
         if (cb)
         {
           char posattr[15];
           double posx;
           
           if (realposx > (HORZ_SCROLLBAR_SIZE - page))
  			     realposx = (HORZ_SCROLLBAR_SIZE-page);

           posx = xmin + realposx*(xmax-xmin)/HORZ_SCROLLBAR_SIZE;

           sprintf(posattr,"%f",posx);
           IupStoreAttribute(n,IUP_POSX,posattr);

           cb(n,IUP_SBDRAGH,(float)posx,(float)posy);
         }
         else
           InvalidateRect(hwnd,NULL,FALSE);

         break;
      }
      case SB_TOP:
         break;
   }
}

static void UpdateVerScroll( Ihandle* n, long op )
{
   HWND hwnd = (HWND)handle(n);
   int (*cb)(Ihandle*,int,float,float) = NULL;
   double posx, ymin, ymax;
   unsigned short page;

   ymax = IupGetFloat(n,IUP_YMAX);
   ymin = IupGetFloat(n,IUP_YMIN);
   page = (int) ((IupGetFloat(n,IUP_DY)/(ymax-ymin))*VERT_SCROLLBAR_SIZE);
   if (page == 0)
      page = 1;

   posx = IupGetFloat(n,IUP_POSX);

   cb = (int (*)(Ihandle*,int,float,float))IupGetCallback(n,IUP_SCROLL_CB);

   switch (LOWORD(op))
   {
      case SB_BOTTOM:
              break;
      case SB_ENDSCROLL:
      {
         char posattr[15];
         double posy;
         unsigned short realposy = GetScrollPos(hwnd, SB_VERT);

         if (realposy > (VERT_SCROLLBAR_SIZE - page))
            realposy = (VERT_SCROLLBAR_SIZE-page);

         posy = ymin + realposy*(ymax-ymin)/VERT_SCROLLBAR_SIZE;

         sprintf(posattr,"%f",posy);
         IupStoreAttribute(n,IUP_POSY,posattr);

         if (!cb)
            InvalidateRect(hwnd,NULL,FALSE);
         else
            cb(n,IUP_SBPOSV,(float)posx,(float)posy);
         break;
      }
      case SB_LINEDOWN:
      {
         int realposy = GetScrollPos(hwnd, SB_VERT);
         realposy = realposy + (int)(0.1*page);
         if (realposy > (VERT_SCROLLBAR_SIZE - page))
            realposy = (VERT_SCROLLBAR_SIZE-page);

         SetScrollPos(hwnd,SB_VERT,realposy,TRUE);

         if (cb)
         {
            double posy;
            posy = ymin + realposy*(ymax-ymin)/VERT_SCROLLBAR_SIZE;
            cb(n,IUP_SBDN,(float)posx,(float)posy);
         }
         break;
      }
      case SB_LINEUP:
      {
         int realposy = GetScrollPos(hwnd, SB_VERT);
         realposy = realposy - (int)(0.1*page);
         if (realposy < 1)
            realposy = 1;
          
         SetScrollPos(hwnd,SB_VERT,realposy,TRUE);

         if (cb)
         {
            double posy;
            posy = ymin + realposy*(ymax-ymin)/VERT_SCROLLBAR_SIZE;
            cb(n,IUP_SBUP,(float)posx,(float)posy);
         }
         break;
      }
      case SB_PAGEDOWN:
      {
         int realposy = GetScrollPos(hwnd, SB_VERT);
         realposy = realposy + page;
		   if (realposy > (VERT_SCROLLBAR_SIZE - page))
			   realposy = (VERT_SCROLLBAR_SIZE-page);

         SetScrollPos(hwnd,SB_VERT,realposy,TRUE);
         if (cb)
         {
            double posy;
            posy = ymin + realposy*(ymax-ymin)/VERT_SCROLLBAR_SIZE;
            cb(n,IUP_SBPGDN,(float)posx,(float)posy);
         }
         break;
      }
      case SB_PAGEUP:
      {
         int realposy = GetScrollPos(hwnd, SB_VERT);
         realposy = realposy - page;
         if (realposy < 1)
            realposy = 1;
          
         SetScrollPos(hwnd,SB_VERT,realposy,TRUE);
         if (cb)
         {
            double posy;
            posy = ymin + realposy*(ymax-ymin)/VERT_SCROLLBAR_SIZE;
            cb(n,IUP_SBPGUP,(float)posx,(float)posy);
         }
         break;
      }
      case SB_THUMBPOSITION:
         break;
      case SB_THUMBTRACK:
      {
         unsigned short realposy = (unsigned short)HIWORD(op);
         SetScrollPos(hwnd,SB_VERT,realposy,TRUE);
         if (cb)
         {
           char posattr[15];
           double posy;
           
           if (realposy > (VERT_SCROLLBAR_SIZE - page))
			       realposy = (VERT_SCROLLBAR_SIZE-page);

           posy = ymin + realposy*(ymax-ymin)/VERT_SCROLLBAR_SIZE;

           sprintf(posattr,"%f",posy);
           IupStoreAttribute(n,IUP_POSY,posattr);

           cb(n,IUP_SBDRAGV,(float)posx,(float)posy);
         }
         else
           InvalidateRect(hwnd,NULL,FALSE);

         break;
      }
      case SB_TOP:
         break;
   }
}

static int is_child( HWND p, HWND c)
{
   HWND p2 = GetParent(c);

   if (p2==NULL)
      return 0;
   if (p2 == p)
      return 1;
   return is_child(p,p2);
}

void iupwinSetFocus(Ihandle* n)
{
   HWND hwnd = GetFocus();

   if (hwnd != (HWND)handle(n) && !is_child((HWND)handle(n),hwnd))
   {
      SetFocus((HWND)handle(n));
   }
}

LRESULT CALLBACK iupwinCanvasProc (HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
  static int dragging = 0;
  static char report[12];
  LRESULT result = 0L;
  Ihandle *n = NULL;
  int is_dblclk = 0;

  n = iupwinHandleGet(hwnd);
  if (n == NULL)
    return DefWindowProc(hwnd, msg, wp, lp);

  /* remember that the canvas can be parent of other IUP controls, for ex: in IupTabs and IupMatrix,
     so it will receive notifications like the dialog. Like:
     WM_DRAWITEM, WM_CTLCOLOR*, WM_COMMAND, */

  switch (msg)
  {
  case WM_INITMENU:
    if (dragging)
    {
      dragging = 0;
      ReleaseCapture();
    }
    break;

  case WM_MOUSELEAVE:
    iupwinCallEnterLeaveWindow(n, 0);
    return 0;

  case WM_DROPFILES:
    deal_dropfiles((HANDLE) wp, n);
    break;

  case WM_COMMAND:
    /* Should only execute WM_COMMAND messages if the canvas is 
    ** already mapped */
    if( iupCheck(n,"_IUPWIN_IS_MAPPING") != YES )
      result = iupwinDlgMsgCommand(hwnd,wp,lp);
    break;
  case WM_VSCROLL:
    UpdateVerScroll( n, wp );
    break;
  case WM_HSCROLL:
    UpdateHorScroll( n, wp );
    break;
  case WM_ERASEBKGND: 
  {
    Icallback cb = (Icallback) IupGetCallback(n,IUP_ACTION);
    if (!cb) 
    {
      RECT rc;
      HDC hdc = (HDC)wp;
      COLORREF c;

      iupwinGetColorRef(n, IUP_BGCOLOR, &c);
      GetClientRect(hwnd, &rc); 
      FillRect(hdc, &rc, iupwinGetBrush(iupwinGetBrushIndex(c))); 
    }

    return 1L; 
  }
  case WM_PAINT:
    {
      IFnff cb = (IFnff) IupGetCallback(n,IUP_ACTION);
      PAINTSTRUCT ps;
      HDC hdc;

      hdc = BeginPaint(hwnd,&ps);
      IupSetAttribute(n,"HDC_WMPAINT", (char*)&hdc);

      if (cb) cb(n,IupGetFloat(n,IUP_POSX),IupGetFloat(n,IUP_POSY));

      IupSetAttribute(n,"HDC_WMPAINT", NULL);
      EndPaint (hwnd,&ps);
      break;
    }
  case WM_MOUSEMOVE:
    iupwinCallEnterLeaveWindow(n, 1);

    {
      char *r=report;
      IFniis cb = (IFniis) IupGetCallback(n,IUP_MOTION_CB);
      short int x, y;

      x = LOWORD(lp);
      y = HIWORD(lp);

      if (!GetClassLongPtr ((HWND)handle(n), GCLP_HCURSOR))
        SetCursor(NULL);

      if (cb == NULL)
        break;
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
      cb (n, x, y, report);
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
      char *r=report;
      int b=0;
      IFniiiis cb = (IFniiiis) IupGetCallback(n,IUP_BUTTON_CB);
      short int x, y;
      x = LOWORD(lp);
      y = HIWORD(lp);

      iupwinSetFocus(n);
      if (cb == NULL)
        break;
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

      SetCapture(hwnd);
      dragging = 1;
      if (cb (n, b, 1, x, y, report) == IUP_CLOSE)
      {
        IupExitLoop();
        ReleaseCapture();
        dragging = 0;
      }

      if (dragging)
        SendMessage(hwnd,WM_SETCURSOR,(WPARAM)hwnd,MAKELPARAM(1,WM_MOUSEMOVE));
    }
    break;
  case WM_LBUTTONUP:
  case WM_MBUTTONUP:
  case WM_RBUTTONUP:
    {
      char *r=report;
      int b=0;
      IFniiiis cb;
      short int x, y;

      x = LOWORD(lp);
      y = HIWORD(lp);

      if (!dragging)
        break;

      cb = (IFniiiis) IupGetCallback(n,IUP_BUTTON_CB);
      if (cb == NULL)
        break;

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

      ReleaseCapture();
      dragging=0;

      if (cb (n, b, 0, x, y, report) == IUP_CLOSE)
        IupExitLoop();

      SendMessage(hwnd,WM_SETCURSOR,(WPARAM)hwnd,MAKELPARAM(1,WM_MOUSEMOVE));
    }
    break;
  case WM_SIZE:
    {
      IFnii cb;

      /* Calls IUP_MAP_CB only for canvases (dialogs are treated elsewhere) */
      if (iupGetEnv(n,"_IUPWIN_IS_MAPPING") != NULL)
      {
        IFn map_cb = (IFn) IupGetCallback(n, IUP_MAP_CB);
        if(map_cb != NULL) map_cb(n);
      }

      /* this callback will be called also in MAPPING. */
      cb = (IFnii) IupGetCallback(n,IUP_RESIZE_CB);
      if (cb)
      {
        RECT rect;
        GetClientRect(handle(n), &rect);
        cb (n, rect.right-rect.left, rect.bottom-rect.top);
        /* cb (n, LOWORD (lp), HIWORD(lp)); This can not be used because of a bug 
                                            in the first time of WM_SIZE with scroolbars. */
      }

      return 0;
    }
  /* case WM_CTLCOLORDLG: this is done using SetClassLong(GCL_HBRBACKGROUND). */
  case WM_CTLCOLORBTN:
  case WM_CTLCOLOREDIT:
  case WM_CTLCOLORLISTBOX:
  case WM_CTLCOLORSTATIC:
    result = dlgmsgCtlColor( hwnd, (HWND)lp, (HDC)wp );
    break;
  case WM_DRAWITEM:
    {
      Ihandle *h;
      int ismenu = (UINT) wp;

      if(ismenu == 0) /* is a menu */
        h = iupwinGetMenuIhandle(wp);
      else
        h = iupwinHandleGet((HWND) (( (LPDRAWITEMSTRUCT)lp)->hwndItem ) ); 

      if(h && type(h) == BUTTON_)
        iupwinDrawButton(h, (LPDRAWITEMSTRUCT)lp);
      else if(h && type(h) == LABEL_)
        iupwinDrawLabel(h, (LPDRAWITEMSTRUCT)lp);

      result = TRUE;
      break;
    }

  case WM_SETFOCUS:
    callGetfocusCb(n);
    break;
  case WM_KILLFOCUS:
    callKillfocusCb(n);
    break;

  case WM_MOUSEWHEEL:
    {
      typedef int (*mwF)(Ihandle*, float, int, int, char*);
      mwF cb = (mwF)IupGetCallback(n, "WHEEL_CB");
      if(cb)
      {
        char *r=report;
        short int keys = LOWORD(wp);
        short int delta = HIWORD(wp);
        short int x = LOWORD(lp); 
        short int y = HIWORD(lp); 
        POINT p; p.x = x; p.y = y;
        ScreenToClient(hwnd, &p);

        if (keys & MK_SHIFT)   *r++= 'S';
        else                   *r++= ' ';
        if (keys & MK_CONTROL) *r++= 'C';
        else                   *r++= ' ';
        if (keys & MK_LBUTTON) *r++= IUP_BUTTON1;  /* this does not make much sense */
        else                   *r++= ' ';
        if (keys & MK_MBUTTON) *r++= IUP_BUTTON2;
        else                   *r++= ' ';
        if (keys & MK_RBUTTON) *r++= IUP_BUTTON3;
        else                   *r++= ' ';
        *r='\0';
        cb(n, delta/120.0f, p.x, p.y, report);
      }
      else
      {
        short int delta = HIWORD(wp);
        int realposy = GetScrollPos(hwnd, SB_VERT);
        unsigned short page = (int) ((IupGetFloat(n,IUP_DY)/(IupGetFloat(n,IUP_YMAX)-IupGetFloat(n,IUP_YMIN)))*VERT_SCROLLBAR_SIZE);
        if (page == 0) page = 1;
        realposy = realposy - (int)(0.1*page*delta/120.0);
        if (realposy < 1)
            realposy = 1;
        else if (realposy > (VERT_SCROLLBAR_SIZE - page))
            realposy = (VERT_SCROLLBAR_SIZE-page);

        UpdateVerScroll( n, MAKELONG(SB_THUMBTRACK, realposy) );
      }

      break;
    }

    /* evita beeps ao serem pressionadas teclas quando
    o foco esta´ sobre um canvas */
  case WM_GETDLGCODE:
    return DLGC_WANTCHARS|DLGC_WANTARROWS;

  case WOM_CLOSE:
  case WOM_DONE:
  case WOM_OPEN:
    {
      IFni cb = (IFni)IupGetCallback(n, IUP_WOM_CB);
      if(cb)
      {
        int v = -2; /* Error */
        switch(msg)
        {
        case WOM_OPEN:  v = 1;  break;
        case WOM_DONE:  v = 0;  break;
        case WOM_CLOSE: v = -1; break;
        }
        cb(n, v);
      }
      break;
    }
    /* WM_SYSKEYDOWN causes the ALT-F4 not to close app correctly. Removed.
    case WM_SYSKEYUP:
    case WM_SYSKEYDOWN:
    */
  case WM_KEYUP:
  case WM_KEYDOWN:
    {
      IFnii cb = (IFnii)IupGetCallback(n, IUP_KEYPRESS_CB);
      if(cb)
      {
        int iup_key = iupwinKeyDecode((int)wp);
        if (iup_key != 0) /* The IUP key code must exist */
        {
          int press = (msg==WM_KEYUP || msg==WM_SYSKEYUP)?0:1;
          if(cb(n, iup_key, press) == IUP_CLOSE)
            IupExitLoop();
        }
      }
      return 0;
    }

  default:
    return DefWindowProc(hwnd, msg, wp, lp);
  }
  return result;
}

static void track_mouse(HWND hwnd, int in)
{
  BOOL ret;
  TRACKMOUSEEVENT mouse;
  mouse.cbSize = sizeof(TRACKMOUSEEVENT);

  if(in)
    mouse.dwFlags = TME_HOVER;
  else
    mouse.dwFlags = TME_LEAVE;

  mouse.hwndTrack = hwnd;
  mouse.dwHoverTime = 1;
  ret = _TrackMouseEvent(&mouse);
  assert(ret != 0);
}
