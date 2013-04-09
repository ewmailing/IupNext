/** \file
 * \brief Scintilla control: Overtype
 *
 * See Copyright Notice in "iup.h"
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <Scintilla.h>
#include <SciLexer.h>

#ifdef GTK
#include <gtk/gtk.h>
#include <ScintillaWidget.h>
#else
#include <windows.h>
#endif

#include "iup.h"

#include "iup_object.h"
#include "iup_attrib.h"
#include "iup_str.h"
#include "iup_stdcontrols.h"

#include "iupsci_overtype.h"
#include "iup_scintilla.h"

/***** OVERTYPE *****/

char* iupScintillaGetOvertypeAttrib(Ihandle *ih)
{
  if(IUP_SSM(ih->handle, SCI_GETOVERTYPE, 0, 0))
    return "YES";
  else
    return "NO";
}

int iupScintillaSetOvertypeAttrib(Ihandle *ih, const char *value)
{
  if (iupStrEqualNoCase(value, "YES"))
    IUP_SSM(ih->handle, SCI_SETOVERTYPE, 1, 0);
  else
    IUP_SSM(ih->handle, SCI_SETOVERTYPE, 0, 0);

  return 0;
}
