/** \file
 * \brief Scintilla control: Cut, copy and paste
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

#include "iupsci_clipboard.h"
#include "iup_scintilla.h"

/***** CUT, COPY AND PASTE *****
Attributes not implement yet:
SCI_CANPASTE
SCI_COPYRANGE(int start, int end)
SCI_COPYTEXT(int length, const char *text)
SCI_COPYALLOWLINE
SCI_SETPASTECONVERTENDINGS(bool convert)
SCI_GETPASTECONVERTENDINGS
*/

int iupScintillaSetClipboardAttrib(Ihandle *ih, const char *value)
{
  if (iupStrEqualNoCase(value, "COPY"))
    IUP_SSM(ih->handle, SCI_COPY, 0, 0);
  else if (iupStrEqualNoCase(value, "CUT"))
    IUP_SSM(ih->handle, SCI_CUT, 0, 0);
  else if (iupStrEqualNoCase(value, "PASTE"))
    IUP_SSM(ih->handle, SCI_PASTE, 0, 0);
  else if (iupStrEqualNoCase(value, "CLEAR"))
    IUP_SSM(ih->handle, SCI_CLEAR, 0, 0);
  else if (iupStrEqualNoCase(value, "UNDO"))
    IUP_SSM(ih->handle, SCI_UNDO, 0, 0);
  else if (iupStrEqualNoCase(value, "REDO"))
    IUP_SSM(ih->handle, SCI_REDO, 0, 0);

  return 0;
}
