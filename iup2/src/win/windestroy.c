/** \file
 * \brief Windows Driver Controls Destruction
 *
 * See Copyright Notice in iup.h
 *  */

#include <stdio.h>    /* sprintf */
#include <windows.h>
#include <commctrl.h>
#include <assert.h>

#include "iglobal.h"
#include "idrv.h"
#include "wincreat.h"
#include "win.h"
#include "winproc.h"
#include "winhandle.h"


static void winDestroyImage(Ihandle *image)
{
  HICON         icon   = NULL;
  HBITMAP       bmp    = NULL;
  IwinBitmap* himage = NULL;

  assert(image);
  if(!image)
    return;

  himage = handle(image);
  if(himage->bmpinfo) 
  {
    free(himage->bmpinfo);
    himage->bmpinfo = NULL;
  }

  if( himage->bitmap)  
  {
    free(himage->bitmap);
    himage->bitmap = NULL;
  }

  if( himage->bitmask)  
  {
    free(himage->bitmask);
    himage->bitmask = NULL;
  }

  free(himage);
  
  icon = (HICON)IupGetAttribute(image, "_IUPWIN_HICON_CURSOR");
  if(icon)
  {
    DestroyIcon(icon);
    IupSetAttribute(image, "_IUPWIN_HICON_CURSOR", NULL);
  }

  icon = (HICON)IupGetAttribute(image, "_IUPWIN_HICON_ICON");
  if(icon)
  {
    int test = DestroyIcon(icon);
    assert(test);   
    (void)test;
    IupSetAttribute(image, "_IUPWIN_HICON_ICON", NULL);
  }

  bmp = (HBITMAP) IupGetAttribute(image, "_IUPWIN_HBITMAP");
  if(bmp)
  {
    int test = DeleteObject(bmp);
    assert(test);
    (void)test;
    IupSetAttribute(image, "_IUPWIN_HBITMAP", NULL);
  }

  bmp = (HBITMAP) IupGetAttribute(image, "_IUPWIN_HBITMAP_INACTIVE");
  if(bmp)
  {
    int test = DeleteObject(bmp);
    assert(test);
    (void)test;
    IupSetAttribute(image, "_IUPWIN_HBITMAP_INACTIVE", NULL);
  }

  {
    HIMAGELIST himl = (HIMAGELIST)IupGetAttribute(image, "_IUPWIN_HIMAGELIST");
    if (himl)
      ImageList_Destroy(himl);
  }
}

/********************************************************************
** Frees window and class. Also destroys Ihandle information stored
** inside the window USERDATA atribute.
********************************************************************/
static void winDestroyFreeWindowClass(Ihandle *n)
{
  char cname[255];

  SetLastError(0);
  GetClassName(handle(n), cname, 255);
  assert(GetLastError() == 0);

  /* Ihandle will not be found when WM_DESTROY is called */
  iupwinHandleRemove(n);

  if (iupCheck(n, "MDICLIENT")==YES) /* mdiclient class is not a IUP class */
  {
    HWND hwndT;

    /* hide the MDI client window to avoid multiple repaints */
    ShowWindow((HWND)handle(n), SW_HIDE);

    /* As long as the MDI client has a child, destroy it */
    while (hwndT = GetWindow((HWND)handle(n), GW_CHILD))
    {
      Ihandle* child;

	    /* Skip the icon title windows */
	    while (hwndT && GetWindow (hwndT, GW_OWNER))
	      hwndT = GetWindow (hwndT, GW_HWNDNEXT);

	    if (!hwndT)
	        break;

      child = iupwinHandleGet(hwndT); 
      if (child)
        IupDestroy(child);
    }

    return;
  }

  SetClassLongPtr((HWND)handle(n), GCLP_HBRBACKGROUND, 0);
  SetClassLongPtr((HWND)handle(n), GCLP_HCURSOR,       0);
  SetWindowLongPtr((HWND)handle(n), GWLP_WNDPROC,      0);

  /* Destroys window, so we can destroy the class */
  SetLastError(0);
  if (iupCheck(n, "MDICHILD")==YES) 
  {
    Ihandle* client = (Ihandle*)iupGetEnv(n, "_IUPWIN_MDICLIENT");
    SendMessage((HWND)handle(client), WM_MDIDESTROY, (WPARAM)handle(n), 0);
  }
  else
    DestroyWindow(handle(n));

/*  iupwinShowLastError(); */

  SetLastError(0);
  UnregisterClass(cname, iupwin_hinstance);
}

static void winDestroyRemoveProc(Ihandle* n, HWND hwnd, char* name)
{
  void *oldfuncproc = NULL;
  oldfuncproc = IupGetAttribute(n, name);
  if(oldfuncproc)
  {
    SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR) oldfuncproc);
    IupSetAttribute(n, name,  NULL);
  }
}

/********************************************************************
** Selects which elements should be destroy by the driver.
** FILL_ and RADIO_ can't be here because its handle is the same 
** as the dialog 
********************************************************************/
static int winDestroyIsNative(Ihandle *n)
{
  if (type(n) != BUTTON_ &&
      type(n) != CANVAS_    && type(n) != DIALOG_ && 
      type(n) != LABEL_     && type(n) != LIST_   &&
      type(n) != TEXT_      && type(n) != TOGGLE_ &&
      type(n) != MULTILINE_ && type(n) != FRAME_  &&
      type(n) != MENU_      && type(n) != IMAGE_ ) 
    return 0;
  else
    return 1;
}

static void winDestroyClearHandles(Ihandle* n)
{
  Ihandle* c = NULL;

  if (!n) return;

  if(type(n) == IMAGE_     || type(n) == BUTTON_ ||
     type(n) == LABEL_     || type(n) == LIST_||
     type(n) == MULTILINE_ || type(n) == ITEM_ ||
     type(n) == TEXT_      || type(n) == SUBMENU_ ||
     type(n) == TOGGLE_    || type(n) == SEPARATOR_ ||
     type(n) == CANVAS_    || type(n) == FRAME_ ||
     type(n) == HBOX_      || type(n) == ZBOX_ ||
     type(n) == VBOX_      || type(n) == FILL_ )
  {
    handle(n) = NULL;
  }
  else if(type(n) == DIALOG_)
  {
    handle(n) = NULL;
    winDestroyClearHandles(child(n));
  }
  else if(type(n) == MENU_)
  {
    handle(n) = NULL;
    foreachchild(c,n)
      winDestroyClearHandles(c);
  }
}

static void winDestroyWindow(Ihandle *n)
{
  assert(n);
  if(!n)
    return;

  if(type(n) == DIALOG_)
  {
    Ihandle *menu = IupGetAttributeHandle(n, IUP_MENU);
    if (menu) 
    {
      handle(menu) = NULL; /* the dialog will destroy the menu */
      IupDestroy(menu);  
    }

    /* if this is a popup window, this will also end the message loop */
    IupHide(n); 
    
    /* Deletes window and associated window class */
    winDestroyFreeWindowClass(n);
  }
  else if(type(n) == CANVAS_)
  {
    winDestroyFreeWindowClass(n);
  }
  else if(type(n) == MULTILINE_ || type(n) == TEXT_)
  {
    winDestroyRemoveProc(n, handle(n), "_IUPWIN_TEXTOLDPROC__");
    iupwinHandleRemove(n);
    DestroyWindow(handle(n));
  }
  else if(type(n) == LABEL_ || type(n) == LIST_ || type(n) == TOGGLE_)
  {
    if (type(n) == TOGGLE_)
      winDestroyRemoveProc(n, handle(n), "_IUPWIN_TOGGLEOLDPROC__");
    else if (type(n) == LIST_)
    {
      winDestroyRemoveProc(n, (HWND)IupGetAttribute(n, "_IUPWIN_EDITBOX"), "_IUPWIN_TEXTOLDPROC__");
      winDestroyRemoveProc(n, (HWND)IupGetAttribute(n, "_IUPWIN_COMBOBOX"), "_IUPWIN_COMBOLISTOLDPROC__");
    }
    else if (type(n) == LABEL_)
    {
      winDestroyRemoveProc(n, handle(n), "_IUPWIN_BUTTONOLDPROC__");
    }
    iupwinHandleRemove(n);
    DestroyWindow(handle(n));
  }
  else if(type(n) == BUTTON_)
  {
    winDestroyRemoveProc(n, handle(n), "_IUPWIN_BUTTONOLDPROC__");
    iupwinHandleRemove(n);
    DestroyWindow(handle(n));
  }
  else if(type(n) == IMAGE_)
  {
    winDestroyImage(n);
  }
  else if(type(n) == FRAME_)
  {
    winDestroyRemoveProc(n, handle(n), "_IUPWIN_FRAMEOLDPROC__");
    iupwinHandleRemove(n);
    DestroyWindow(handle(n));
  }
  else if(type(n) == MENU_)
  {
    assert(number(n)!=-1); 
    iupwinCleanidIhandle(number(n));
    if (handle(n)) DestroyMenu(handle(n));  /* Destroy Menu is recursive */
  }
  else if(type(n) == SUBMENU_)
  {
    assert(number(n)!=-1); 
    iupwinCleanidIhandle(number(n));
    /* DestroyWindows is automatically called by the system */
  }
  else if(type(n) == ITEM_ || type(n) == SEPARATOR_)
  {
    assert(number(n)!=-1); 
    iupwinCleanidIhandle(number(n));
    /* DestroyWindows is automatically called by the system */
  }
}

void iupdrvDestroyHandle (Ihandle *n)
{
  HWND hwnd = NULL;

  assert(n);
  if(n == NULL)
    return;

  hwnd = handle(n);

  if (hwnd && winDestroyIsNative(n)) 
  {
    winDestroyWindow(n);
  }

  winDestroyClearHandles(n);
}
