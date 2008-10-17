/** \file
 * \brief Windows Driver resize windows objects
 *
 * See Copyright Notice in iup.h
 * $Id: winresiz.c,v 1.1 2008-10-17 06:19:26 scuri Exp $
 */

#include <windows.h>

#include "iglobal.h"
#include "idrv.h"
#include "win.h"
#include "wincreat.h"

static void winResizeBasicElement (Ihandle *n, int x, int y)
{
  if (type(n)==LIST_)
  {
    if (iupCheck(n, IUP_DROPDOWN)==YES)
    {
      RECT rect;
      int calc_h, win_h, win_w;
      int charheight,voptions = IupGetInt(n, IUP_VISIBLE_ITEMS);
      if (voptions <= 0)
        voptions = 10; /* less than this gives weird results */
      else
        voptions += 2; /* compensate for the text box also */
      iupdrvGetCharSize(n,NULL,&charheight);
      calc_h = voptions*charheight+7;

      SendMessage((HWND)handle(n), CB_GETDROPPEDCONTROLRECT, 0, (LPARAM)&rect);
      win_h = rect.bottom-rect.top;
      win_w = rect.right-rect.left;

      if (currentwidth(n) != win_w || calc_h != win_h)
        MoveWindow((HWND)handle(n), x, y, currentwidth(n), calc_h, 0);
      else
        SetWindowPos((HWND)handle(n), HWND_TOP, x, y, 
                     0, 0, SWP_NOSIZE|SWP_NOACTIVATE|SWP_NOZORDER);
      return;
    }
  }

  MoveWindow ((HWND)handle(n), x, y, currentwidth(n), currentheight(n), 0);
}

static void winresizeCanvas (Ihandle *n, int x, int y)
{
  MoveWindow ((HWND)handle(n), x, y, currentwidth(n), currentheight(n), TRUE);
}

void iupdrvResizeObjects (Ihandle *n)
{
  Ihandle *c;

  if(!n)
    return;

  if(type(n) == DIALOG_)
  {
    iupSetEnv(n, "_IUPWIN_NOT_WM_SIZE", IUP_YES);

    SetWindowPos((HWND)handle(n), (HWND) NULL, 0, 0,
      currentwidth(n) + iupwinDialogDecorX(n),
      currentheight(n) + iupwinDialogDecorY(n),
      SWP_NOMOVE | SWP_NOZORDER |
      SWP_NOACTIVATE | SWP_NOOWNERZORDER |
      SWP_NOSENDCHANGING);

    iupSetEnv(n, "_IUPWIN_NOT_WM_SIZE", NULL);

    iupdrvResizeObjects (child(n));

    InvalidateRect ((HWND)handle(n), NULL, TRUE);

    if (iupCheck(n,"CLIPCHILDREN"))
    {
      /* Invalidate children and redraw itself with default window proc */
      RedrawWindow((HWND)handle(n), NULL, NULL, RDW_ALLCHILDREN | 
                                    RDW_INVALIDATE);
    }
  }
  else if(type(n) == HBOX_ || type(n) == VBOX_ || type(n) == ZBOX_)
  {
    foreachchild(c,n)
      iupdrvResizeObjects (c);
  }
  else if(type(n) == FRAME_)
  {
    winResizeBasicElement (n, posx(n), posy(n));
    iupdrvResizeObjects (child(n));
  }
  else if(type(n) == RADIO_)
  {
    iupdrvResizeObjects (child(n));
  }
  else if(type(n) == LIST_  || type(n) == TOGGLE_    || type(n) == BUTTON_ || 
          type(n) == LABEL_ || type(n) == MULTILINE_ || type(n) == TEXT_ )
  {
    winResizeBasicElement (n, posx(n), posy(n));
  }
  else if(type(n) == CANVAS_)
  {
    winresizeCanvas (n, posx(n), posy(n));
  }
}

