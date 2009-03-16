/** \file
 * \brief IupFileDlg pre-defined dialog
 *
 * See Copyright Notice in "iup.h"
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <stdarg.h>
#include <limits.h>

#include "iup.h"

#include "iup_object.h"
#include "iup_attrib.h"
#include "iup_str.h"
#include "iup_stdcontrols.h"


Ihandle* IupFileDlg(void)
{
  return IupCreate("filedlg");
}

Iclass* iupFileDlgGetClass(void)
{
  Iclass* ic = iupClassNew(iupDialogGetClass());

  ic->name = "filedlg";
  ic->nativetype = IUP_TYPEDIALOG;
  ic->is_interactive = 1;

  iupClassRegisterCallback(ic, "FILE_CB", "ss");

  /* reset not used native dialog methods */
  ic->parent->LayoutUpdate = NULL;
  ic->parent->SetPosition = NULL;
  ic->parent->Map = NULL;
  ic->parent->UnMap = NULL;

  iupdrvFileDlgInitClass(ic);

  /* only the default value */
  iupClassRegisterAttribute(ic, "NOCHANGEDIR", NULL, NULL, "YES", NULL, IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "DIALOGTYPE", NULL, NULL, "OPEN", NULL, IUPAF_NO_INHERIT);

  iupClassRegisterAttribute(ic, "PREVIEWDC", NULL, NULL, NULL, NULL, IUPAF_NO_INHERIT|IUPAF_READONLY|IUPAF_NO_STRING);

  return ic;
}
