/** \file
 * \brief Windows Focus
 *
 * See Copyright Notice in "iup.h"
 */

#include <stdlib.h>
#include <stdio.h>

#include <windows.h>

#include "iup.h"
#include "iup_object.h"
#include "iup_focus.h"
#include "iup_assert.h" 
#include "iup_drv.h" 
#include "iup_attrib.h" 

#include "iupwin_drv.h"
#include "iupwin_handle.h"


void iupdrvSetFocus(Ihandle *ih)
{
  SetFocus(ih->handle);
}

void iupwinWmSetFocus(Ihandle *ih)
{
  Ihandle* dialog = IupGetDialog(ih);
  if (ih != dialog)
    iupAttribSetStr(dialog, "_IUPWIN_LASTFOCUS", (char*)ih);  /* used by IupMenu */

  iupCallGetFocusCb(ih);
}
