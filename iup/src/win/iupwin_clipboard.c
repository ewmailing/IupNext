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


static int winClipboardSetTextAttrib(Ihandle *ih, const char *value)
{
  HGLOBAL handle;
  void* clip_str;
  int size = strlen(value)+1;
  (void)ih;

  if (!OpenClipboard(NULL))
    return 0;

  handle = GlobalAlloc(GMEM_MOVEABLE, size); 
  clip_str = GlobalLock(handle);
  CopyMemory(clip_str, value, size);
  GlobalUnlock(handle);

  EmptyClipboard();
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
  (void)ih;
  return 0;
}

static char* winClipboardGetImageAttrib(Ihandle *ih)
{
  (void)ih;
  return NULL;
}

static char* winClipboardGetTextAvailableAttrib(Ihandle *ih)
{
  (void)ih;
  if (IsClipboardFormatAvailable(CF_TEXT))
    return "YES";
  else
    return "NO";
}

static char* winClipboardGetImageAvailableAttrib(Ihandle *ih)
{
  (void)ih;
  if (IsClipboardFormatAvailable(CF_DIB)||IsClipboardFormatAvailable(CF_BITMAP))
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
  iupClassRegisterAttribute(ic, "IMAGE", winClipboardGetImageAttrib, winClipboardSetImageAttrib, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "TEXTAVAILABLE", winClipboardGetTextAvailableAttrib, NULL, NULL, NULL, IUPAF_READONLY|IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "IMAGEAVAILABLE", winClipboardGetImageAvailableAttrib, NULL, NULL, NULL, IUPAF_READONLY|IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);

  return ic;
}
