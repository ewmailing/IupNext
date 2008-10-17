/** \file
* \brief Windows Driver windows procedures
*
* See Copyright Notice in iup.h
* $Id: winedit.c,v 1.1 2008-10-17 06:19:26 scuri Exp $
*/

#include <stdio.h>
#include <stdlib.h> 
#include <assert.h>

#include <windows.h>
#include <winuser.h>

#include "iupkey.h"
#include "iglobal.h"
#include "imask.h"
#include "idrv.h"
#include "win.h"
#include "winproc.h"
#include "winhandle.h"
#include "winbutton.h"


/* tipos de dados utilizados internamente */

typedef struct _Iwinedit
{
  Ihandle *handle;
  char *value;
  POINT caretpos;
  HWND hwnd;
  DWORD start;
  DWORD end;
  SCROLLINFO horzScroll;
  SCROLLINFO vertScroll;
} Iwinedit;

static void winEditCallMaskCallback(Ihandle* h, int tipo)
{
  IFni cb;

  cb=(IFni)IupGetCallback(h,"MASK_CB");

  if (cb) cb(h,tipo);
  /* Se tipo = 0, um caracter invalido foi digitado */
  /* Se tipo = 1, o campo perdeu o foco, mas a mascara esta incompleta */
}

/* salva o "estado" de um campo de texto */
static void winEditSaveState(Iwinedit *ed, Ihandle *handle, HWND hwnd)
{
  ed->handle = handle;
  ed->hwnd = hwnd;

  /* salva texto */
  ed->value = iupStrDup(IupGetAttribute(ed->handle, IUP_VALUE));

  /* obtem selecao, para poder posteriormente restaura-la */
  SendMessage(ed->hwnd, EM_GETSEL, (WPARAM)&ed->start, (LPARAM)&ed->end);

  /* salva posicao do caret */
  GetCaretPos(&ed->caretpos);

  if(type(ed->handle) == MULTILINE_)
  {
    /* obtem informacoes da scrollbar para poder restaurar
    estado do controle de texto se o usuario quiser ignorar
    tecla */

    ed->vertScroll.fMask = SIF_PAGE | SIF_POS;
    ed->vertScroll.cbSize = sizeof(SCROLLINFO);
    GetScrollInfo(ed->hwnd, SB_VERT, &ed->vertScroll);	
  }

  ed->horzScroll.fMask = SIF_PAGE | SIF_POS;
  ed->horzScroll.cbSize = sizeof(SCROLLINFO);
  GetScrollInfo(ed->hwnd, SB_HORZ, &ed->horzScroll);	
}

static void winEditRestoreState(Iwinedit *ed, char *new_value)
{
  char *current_value = IupGetAttribute(ed->handle, IUP_VALUE);

  /* restaura texto se ele nao tiver sido alterado no meio do caminho */
  if(new_value == NULL || (new_value != NULL && iupStrEqual(current_value, new_value)))
  {
    IupSetAttribute(ed->handle, IUP_VALUE, ed->value);
  }

  /* restaura selecao */
  SendMessage(ed->hwnd, EM_SETSEL, (WPARAM) ed->start, (LPARAM) ed->end);

  if(type(ed->handle) == MULTILINE_)
  {
    SendMessage(ed->hwnd, WM_VSCROLL, MAKEWPARAM(SB_THUMBPOSITION, ed->vertScroll.nPos),0);
    SetScrollInfo(ed->hwnd, SB_VERT, &ed->vertScroll, TRUE);
    SetScrollInfo(ed->hwnd, SB_HORZ, &ed->horzScroll, TRUE);
    SendMessage(ed->hwnd, WM_HSCROLL, MAKEWPARAM(SB_THUMBPOSITION, ed->horzScroll.nPos),0);
  }

  /* restaura posicao do caret */
  SetCaretPos(ed->caretpos.x, ed->caretpos.y);
}

static void winEditCallCaretCb(Ihandle *n)
{
  int old_col, old_row, col, row=1, pos;
  POINT point;

  IFnii cb = (IFnii) IupGetCallback(n,"CARET_CB");
  if(!cb) return;

  if (!GetCaretPos(&point))
    return;

  pos = SendMessage((HWND)handle(n), EM_CHARFROMPOS, 0, MAKELPARAM(point.x, point.y));
  col = LOWORD(pos);

  if (type(n) == TEXT_ || type(n) == LIST_)
  {
    col++;
  }
  else
  {
    row = HIWORD(pos);
    pos = SendMessage((HWND)handle(n), EM_LINEINDEX, (WPARAM)row, (LPARAM)0L);

    row++;
    col = col - pos + 1;
  }

  old_col = IupGetInt(n, "_IUPWIN_CARETCOL");
  old_row = IupGetInt(n, "_IUPWIN_CARETROW");
  if (row != old_row || col != old_col)
  {
    IupSetfAttribute(n, "_IUPWIN_CARETCOL", "%d", col);
    IupSetfAttribute(n, "_IUPWIN_CARETROW", "%d", row);

    cb(n, row, col);
  }
}

static int winEditCallTextCb(Ihandle *n,
                      IFnis cb,
                      WNDPROC textoldproc,
                      HWND hwnd,
                      UINT msg,
                      WPARAM wp,
                      LPARAM lp,
                      char *old_value,
                      int iupcode
                      )
{
  Iwinedit editstate;
  int ch = 0;
  char *new_value;
  int ret = 0;
  char *old_caret, *old_selection;

  assert(n && cb && textoldproc && hwnd && old_value);

  winEditSaveState(&editstate, n, hwnd);

  old_caret     = iupStrDup(IupGetAttribute(n, IUP_CARET));
  old_selection = iupStrDup(IupGetAttribute(n, IUP_SELECTION));

  /* Updates control's value.
  If a text and the key is VK_RETURN, do not call CallWindowProc 
  or Windows will beep
  */
  if(type(n) != TEXT_ || msg != WM_CHAR || wp != VK_RETURN)
    CallWindowProc( textoldproc, hwnd, msg, wp, lp );

  new_value = iupStrDup(IupGetAttribute(n, IUP_VALUE));

  iupSetEnv(n, IUP_VALUE,     old_value);
  iupSetEnv(n, IUP_CARET,     old_caret);
  iupSetEnv(n, IUP_SELECTION, old_selection);
  iupSetEnv(n, "_IUPWIN_IGNORE_EDITTEXT", IUP_YES);

  if(iupcode == 0)         
    ch = cb(n, (int)wp, new_value);
  else
    ch = cb(n, iupcode, new_value);

  iupSetEnv(n, "_IUPWIN_IGNORE_EDITTEXT", NULL);
  iupSetEnv(n, IUP_VALUE,     NULL);
  iupSetEnv(n, IUP_CARET,     NULL);
  iupSetEnv(n, IUP_SELECTION, NULL);

  if (old_selection) free(old_selection);
  if (old_caret) free(old_caret);

  if (ch==IUP_IGNORE)
  {
    winEditRestoreState(&editstate, new_value);
    ret = 0;
  }
  else if (ch==IUP_CLOSE)
  {
    winEditRestoreState(&editstate, new_value);

    IupExitLoop();
    ret = 0;
  }
  else if (ch != IUP_DEFAULT)
  {
    winEditRestoreState(&editstate, NULL);

    if(msg == WM_CHAR)
      CallWindowProc( textoldproc, hwnd, msg, ch, lp );
    else
      CallWindowProc( textoldproc, hwnd, WM_CHAR, ch, 0);

    ret = 1;
  }
  else
    ret = 0;

  free(editstate.value);
  free(new_value);
  return ret;
}

LRESULT CALLBACK iupwinEditProc (HWND hwnd, UINT msg, WPARAM wp, LPARAM lp )
{
  static POINT caretpt = {0,0};
  static int flag = 0;
  static int carethided = 0;
  WNDPROC textoldproc = NULL;
  Ihandle* n = NULL;

  n = iupwinHandleGet(hwnd);
  if(n == NULL)
    return DefWindowProc(hwnd, msg, wp, lp);

  if (type(n)==TEXT_ || type(n)==MULTILINE_ || type(n)==LIST_)
  {
    char* action_name = IUP_ACTION;
    textoldproc = (WNDPROC)IupGetAttribute(n, "_IUPWIN_TEXTOLDPROC__");

    if (type(n)==LIST_)
      action_name = "EDIT_CB";

    switch (msg)
    {
    case WM_GETDLGCODE:
      if (type(n)==MULTILINE_)
      {
        MSG* msg = (MSG*)lp;
        if (msg && (msg->message == WM_KEYDOWN || msg->message == WM_SYSKEYDOWN))
        {
          if (msg->wParam == VK_ESCAPE)
          {
            Ihandle* button = IupGetAttributeHandle(IupGetDialog(n), IUP_DEFAULTESC);
            if (button)
              iupwinButtonCallCb(hwnd, button);
          }

          if (msg->message == WM_KEYDOWN && msg->wParam == VK_TAB && HIWORD(GetKeyState(VK_CONTROL)))
          {
            IupNextField(n);
            return 0;
          }
        }

        return DLGC_WANTALLKEYS;
      }
      else if (type(n)==LIST_)
      {
        MSG* msg = (MSG*)lp;
        if (msg && (msg->message == WM_KEYDOWN || msg->message == WM_SYSKEYDOWN))
        {
          if (msg->wParam == VK_ESCAPE)
          {
            Ihandle* button = IupGetAttributeHandle(IupGetDialog(n), IUP_DEFAULTESC);
            if (button)
              iupwinButtonCallCb(hwnd, button);
          }
          else if (msg->wParam == VK_RETURN)
          {
            Ihandle* button = IupGetAttributeHandle(IupGetDialog(n), IUP_DEFAULTENTER);
            if (button)
              iupwinButtonCallCb(hwnd, button);
          }
        }

        return DLGC_WANTTAB|DLGC_WANTCHARS|DLGC_WANTARROWS;
      }
      else
        return DLGC_WANTCHARS|DLGC_WANTARROWS;
    case WM_CHAR:
      {
        IFnis cb = (IFnis) IupGetCallback(n, action_name);
        char *ov = iupStrDup(IupGetAttribute(n,IUP_VALUE));

        if (cb)
        {
          int ret = winEditCallTextCb(n, cb, textoldproc, hwnd, msg, wp, lp, ov, 0);
          if(ret == 0)
          {
            free(ov);
            return 0;
          }
        }
        else
          CallWindowProc( textoldproc, hwnd, msg, wp, lp );

        if ( type(n) == TEXT_ || type(n)==LIST_)
        {
          ITextInfo* txtinfo;
          txtinfo = (ITextInfo*) IupGetAttribute(n,"_IUPWIN_TXTINFO__");
          if (txtinfo)
          {
            char fill[50] = {0};
            char* nv;
            nv = IupGetAttribute(n,IUP_VALUE);
            txtinfo->status = iupCheckMask(txtinfo->mask,nv,fill);
            if (txtinfo->status == 0) /*nao foi aceito*/
            {
              IupSetAttribute(n,IUP_VALUE,ov);
              flag = 1;
              if (carethided  == 0)
              {
                HideCaret(hwnd);
                carethided = 1;
              }
            }
            else if (wp==8 && fill[0])
            {
              fill[0] = 0;
              txtinfo->status = 2;
            }
            else if (fill[0])
            {
              char text[50];
              GetWindowText(hwnd,text,50);
              strcat(text,fill);
              SetWindowText(hwnd,text);
              fill[0] = 0;
              SendMessage(hwnd,WM_KEYDOWN,VK_END,0x4f);
              SendMessage(hwnd,WM_KEYUP,VK_END,0x4f);
            }
          }
        }
        free(ov);
        winEditCallCaretCb(n);
        return 0;
      }
    case WM_KEYDOWN:
      {
        if ((type(n)==MULTILINE_) || type(n) == TEXT_)  /* do what is not done in the hook */
        {
          int result = iupwinKeyProcess(n, (int)wp);
          if (result==IUP_CLOSE)
          {
            IupExitLoop();
          }
          else if (result==IUP_IGNORE)
          {
            return 0;
          }
        }

        if ((flag == 0) && (type(n) == TEXT_ || type(n)==LIST_))
        {
          GetCaretPos(&caretpt);
          if (wp==VK_RETURN || wp==VK_ESCAPE)
            PostMessage(hwnd,WM_CHAR,wp,lp);
        }

        if (type(n)==LIST_ && wp == VK_TAB)
        {
          IupNextField(n);
          return 0;
        }

        if (wp == VK_DELETE || wp == VK_UP || wp == VK_DOWN || wp == VK_LEFT || wp == VK_RIGHT ||
            wp == VK_PRIOR || wp == VK_HOME || wp == VK_END || wp == VK_NEXT)
        {
          IFnis cb = (IFnis) IupGetCallback(n, action_name);

          if(cb)
          {
            char *ov = iupStrDup(IupGetAttribute(n,IUP_VALUE));
            winEditCallTextCb(n, cb, textoldproc, hwnd, msg, wp, lp, ov, iupwinKeyDecode(wp));
            free(ov);
            return 0;
          }
        }

        winEditCallCaretCb(n);
        break;
      }
    case WM_KEYUP:
      {
        if ((type(n) == TEXT_ || type(n)==LIST_) && (flag == 1))
        {
          POINT pt;
          CallWindowProc( textoldproc, hwnd, msg, wp, lp );
          flag = 2;
          SendMessage(hwnd,WM_KEYDOWN,VK_HOME,0x47);
          SendMessage(hwnd,WM_KEYUP,VK_HOME,0x47);
          GetCaretPos(&pt);
          while(pt.x<caretpt.x)
          {
            SendMessage(hwnd,WM_KEYDOWN,VK_RIGHT,0x4d);
            SendMessage(hwnd,WM_KEYUP,VK_RIGHT,0x4d);
            GetCaretPos(&pt);
          }
          flag = 0;
          if (carethided)
          {
            ShowCaret(hwnd);
            carethided = 0;
          }
          winEditCallMaskCallback(n, 0);

          return 0;
        }
        winEditCallCaretCb(n);
        break;
      }
    case WM_CUT:
      {
        IFnis cb = (IFnis) IupGetCallback(n,action_name);

        if(cb)
        {
          char *ov = iupStrDup(IupGetAttribute(n,IUP_VALUE));
          winEditCallTextCb(n, cb, textoldproc, hwnd, msg, wp, lp, ov, K_cX);
          free(ov);
          return 0;
        }
        break;
      }
    case WM_PASTE:
      {
        IFnis cb = (IFnis) IupGetCallback(n,action_name);

        if(cb)
        {
          char *ov = iupStrDup(IupGetAttribute(n,IUP_VALUE));
          winEditCallTextCb(n, cb, textoldproc, hwnd, msg, wp, lp, ov, K_cV);
          free(ov);
          return 0;
        }
        break;
      }
    case WM_LBUTTONDBLCLK:
    case WM_MBUTTONDBLCLK:
    case WM_RBUTTONDBLCLK:
    case WM_LBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_MBUTTONUP:
    case WM_RBUTTONUP:
    case WM_LBUTTONUP:
      {
        winEditCallCaretCb(n);
        break;
      }
    }
  }

  if (textoldproc)
    return CallWindowProc( textoldproc, hwnd, msg, wp, lp );
  else
    return 0;
}
