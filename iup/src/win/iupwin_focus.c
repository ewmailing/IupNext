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
  else
  {
    /* if a control inside that dialog had the focus, then reset to it when the dialog gets the focus */
    Ihandle* lastfocus = (Ihandle*)iupAttribGet(dialog, "_IUPWIN_LASTFOCUS");
    if (lastfocus) IupSetFocus(lastfocus);
  }

  iupCallGetFocusCb(ih);
}
