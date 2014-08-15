/** \file
 * \brief Windows Driver IupMessage
 *
 * See Copyright Notice in iup.h
 *  */

#include <windows.h>
#include <commdlg.h>

#include "iglobal.h"
#include "win.h"
#include "winproc.h"
#include "winhook.h"

void IupMessage(const char* title, const char* msg)
{
  Ihandle* parent = IupGetHandle(IupGetGlobal(IUP_PARENTDIALOG));
  HWND hwndOwner;

  if (parent && handle(parent))
    hwndOwner = (HWND)handle(parent);
  else
    hwndOwner = GetActiveWindow();

  iupwinUnhook();
  MessageBox(hwndOwner, msg, title, MB_TASKMODAL | MB_OK);
  iupwinRehook();
}
