/** \file
 * \brief Clipboard for the Windows Driver.
 *
 * See Copyright Notice in "iup.h"
 */

#include <windows.h>
 
#include <stdio.h>
#include <stdlib.h>

#include "iup.h"
#include "iupcbs.h"

#include "iup_object.h"
#include "iup_attrib.h"
#include "iup_str.h"
#include "iup_image.h"


static HBITMAP winCopyBitmap(HBITMAP hBitmap)
{
  BITMAP bm;
  HBITMAP hNewBitmap, old, old1;

  HDC hdcMem = CreateCompatibleDC(NULL);
  HDC hdcSrc = CreateCompatibleDC(NULL);

  old = (HBITMAP)SelectObject(hdcSrc, hBitmap);
  GetObject(hBitmap, sizeof(BITMAP), (LPSTR)&bm);

  hNewBitmap = CreateBitmapIndirect(&bm);
  if (!hNewBitmap)
  {
    SelectObject(hdcSrc, old);
    DeleteDC(hdcMem);
    DeleteDC(hdcSrc);
    return NULL;
  }

  old1 = (HBITMAP) SelectObject(hdcMem, hNewBitmap);
  BitBlt(hdcMem, 0, 0, bm.bmWidth, bm.bmHeight, hdcSrc, 0, 0, SRCCOPY);

  SelectObject(hdcMem, old1);

  SelectObject(hdcSrc, old);
  DeleteDC(hdcSrc);
  DeleteDC(hdcMem);

  return hNewBitmap;
}

static HANDLE winCopyHandle(HANDLE handle)
{
  SIZE_T size = GlobalSize(handle);
  HANDLE newHandle = GlobalAlloc(GMEM_MOVEABLE, size); 
  if (newHandle)
  {
    void* mem = GlobalLock(handle);
    void* newMem = GlobalLock(newHandle);
    CopyMemory(newMem, mem, size);
    GlobalUnlock(handle);
    GlobalUnlock(newHandle);
  }
  return newHandle;
}

static int winClipboardSetTextAttrib(Ihandle *ih, const char *value)
{
  HANDLE handle;
  void* clip_str;
  int size = strlen(value)+1;
  (void)ih;

  if (!OpenClipboard(NULL))
    return 0;

  handle = GlobalAlloc(GMEM_MOVEABLE, size); 
  if (!handle)
    return 0;

  clip_str = GlobalLock(handle);
  CopyMemory(clip_str, value, size);
  GlobalUnlock(handle);

  SetClipboardData(CF_TEXT, handle);
  CloseClipboard();

  return 0;
}

static char* winClipboardGetTextAttrib(Ihandle *ih)
{
  HANDLE handle;
  char* str;
  (void)ih;

  if (!OpenClipboard(NULL))
    return NULL;

  handle = GetClipboardData(CF_TEXT);
  if (!handle)
  {
    CloseClipboard();
    return NULL;
  }
  
  str = iupStrGetMemoryCopy((char*)GlobalLock(handle));
 
  GlobalUnlock(handle);
  CloseClipboard();
  return str;
}

static int winClipboardSetImageAttrib(Ihandle *ih, const char *value)
{
  HBITMAP hBitmap;

  if (!OpenClipboard(NULL))
    return 0;

  hBitmap = (HBITMAP)iupImageGetImage(value, ih, 0);

  SetClipboardData(CF_BITMAP, winCopyBitmap(hBitmap));
  CloseClipboard();

  return 0;
}

static int winClipboardSetNativeImageAttrib(Ihandle *ih, const char *value)
{
  if (!OpenClipboard(NULL))
    return 0;

  SetClipboardData(CF_DIB, (HANDLE)value);
  CloseClipboard();

  (void)ih;
  return 0;
}

static char* winClipboardGetNativeImageAttrib(Ihandle *ih)
{
  HANDLE handle;

  if (!OpenClipboard(NULL))
    return 0;

  handle = GetClipboardData(CF_DIB);
  if (!handle)
  {
    CloseClipboard();
    return NULL;
  }

  handle = winCopyHandle(handle);
  CloseClipboard();
  
  (void)ih;
  return handle;
}

static char* winClipboardGetTextAvailableAttrib(Ihandle *ih)
{
  int check;
  (void)ih;
  OpenClipboard(NULL);
  check = IsClipboardFormatAvailable(CF_TEXT);
  CloseClipboard();
  if (check)
    return "YES";
  else
    return "NO";
}

static char* winClipboardGetImageAvailableAttrib(Ihandle *ih)
{
  int check;
  (void)ih;
  OpenClipboard(NULL);
  check = IsClipboardFormatAvailable(CF_DIB);
  CloseClipboard();
  if (check)
    return "YES";
  else
    return "NO";
}

/******************************************************************************/

Ihandle* IupClipboard(void)
{
  return IupCreate("clipboard");
}

Iclass* iupClipboardGetClass(void)
{
  Iclass* ic = iupClassNew(NULL);

  ic->name = "clipboard";
  ic->format = NULL;  /* no parameters */
  ic->nativetype = IUP_TYPECONTROL;
  ic->childtype = IUP_CHILDNONE;
  ic->is_interactive = 0;

  /* Attribute functions */
  iupClassRegisterAttribute(ic, "TEXT", winClipboardGetTextAttrib, winClipboardSetTextAttrib, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "NATIVEIMAGE", winClipboardGetNativeImageAttrib, winClipboardSetNativeImageAttrib, NULL, NULL, IUPAF_NO_STRING|IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "IMAGE", NULL, winClipboardSetImageAttrib, NULL, NULL, IUPAF_WRITEONLY|IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "TEXTAVAILABLE", winClipboardGetTextAvailableAttrib, NULL, NULL, NULL, IUPAF_READONLY|IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "IMAGEAVAILABLE", winClipboardGetImageAvailableAttrib, NULL, NULL, NULL, IUPAF_READONLY|IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);

  return ic;
}
