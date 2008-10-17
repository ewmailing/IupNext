/** \file
 * \brief Windows Driver Frame redraw management
 *
 * See Copyright Notice in iup.h
 * $Id: winframe.c,v 1.1 2008-10-17 06:19:26 scuri Exp $
 */

#include <assert.h>

#include <windows.h>

#include "iglobal.h"
#include "win.h"
#include "winhandle.h"
#include "winframe.h"
#include "winproc.h"

/* iupwinFrameProc is necessary only when CLIPCHILDREN=YES.
   Necessary because it does not paint its own background.
 */

static void winFrameClipChildren(Ihandle* n, HDC hdc, int xr, int yr)
{
  Ihandle* c;
  foreachchild(c,n)
  {
    int x = posx(c)-xr, y = posy(c)-yr,
        w = currentwidth(c), h = currentheight(c);

    if (iupCheck(c,"VISIBLE") && type(c) != HBOX_ && type(c) != VBOX_ && 
        type(c) != ZBOX_ && type(c) != RADIO_ && type(c) != FILL_)
    {
      if (type(c) == LIST_)
      {
        RECT rc;
        GetWindowRect((HWND)handle(c), &rc); 
        w = rc.right - rc.left;
        h = rc.bottom - rc.top;
      }

      ExcludeClipRect(hdc, x, y, x+w, y+h);
    }

    if (type(c) == ZBOX_)
    {
      Ihandle* active = IupGetAttributeHandle(c, "VALUE");
      winFrameClipChildren(active, hdc, xr, yr);
    }
    else if (type(c) != FRAME_ && type(c) != CANVAS_)
      winFrameClipChildren(c, hdc, xr, yr);
  }
}

LRESULT CALLBACK iupwinFrameProc (HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
  WNDPROC frameoldproc = NULL;
  Ihandle *n = NULL;

  n = iupwinHandleGet(hwnd);
  if(n == NULL)
    return DefWindowProc(hwnd, msg, wp, lp);

  switch (msg)
  { 
    case WM_NCHITTEST:
    {
      if (iupwinSetMouseHook(-1))
      {
        POINT p;
        RECT r;
        int w, h;
        p.x = LOWORD(lp);
        p.y = HIWORD(lp);
        ScreenToClient(hwnd, &p);
        GetClientRect(hwnd, &r);
        w = r.right-r.left;
        h = r.bottom-r.top;

        if (p.x < 2 || p.x > w-2 ||
            p.y < 2 || p.y > h-2 ||
            ((iupGetEnv(n, IUP_TITLE)) && p.y < 12))  /* FIXME use TEXT HEIGHT */
          return HTCLIENT;  /* The static control returns always an HTTRANSPARENT */
        else
          return HTTRANSPARENT;
      }
      else
        return HTTRANSPARENT;
    }
  case WM_ERASEBKGND: 
    {
      if (iupCheck(n,IUP_CLIPCHILDREN))
      {
        /* Necessary because in this case it does not paint its own background. */
        RECT rc;
        HDC hdc = (HDC)wp;
        COLORREF c;
        HRGN cliprgn;

        GetClientRect(hwnd, &rc); 

        if (iupGetEnv(n, IUP_TITLE))
          cliprgn = CreateRectRgn(rc.left, rc.top, rc.right, rc.bottom);
        else
          cliprgn = CreateRectRgn(rc.left+2, rc.top+2, rc.right-2, rc.bottom-2);
        SelectClipRgn(hdc, cliprgn);
        winFrameClipChildren(n, hdc, posx(n), posy(n));

        iupwinGetColorRef(n, IUP_BGCOLOR, &c);
        FillRect(hdc, &rc, iupwinGetBrush(iupwinGetBrushIndex(c))); 

        SelectClipRgn(hdc, NULL);
        DeleteObject(cliprgn);

        return 1L; 
      }
      break;
    }
    default:
	    break;
  }

  frameoldproc = (WNDPROC)IupGetAttribute(n, "_IUPWIN_FRAMEOLDPROC__");
  assert(frameoldproc);

  if (frameoldproc)
    return CallWindowProc( frameoldproc, hwnd, msg, wp, lp );
  else 
    return 0;
}

