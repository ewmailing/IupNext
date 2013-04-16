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

#include "iup.h"

#include "iup_object.h"
#include "iup_attrib.h"
#include "iup_str.h"

#include "iupsci_clipboard.h"
#include "iupsci.h"


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
    iupScintillaSendMessage(ih, SCI_COPY, 0, 0);
  else if (iupStrEqualNoCase(value, "CUT"))
    iupScintillaSendMessage(ih, SCI_CUT, 0, 0);
  else if (iupStrEqualNoCase(value, "PASTE"))
    iupScintillaSendMessage(ih, SCI_PASTE, 0, 0);
  else if (iupStrEqualNoCase(value, "CLEAR"))
    iupScintillaSendMessage(ih, SCI_CLEAR, 0, 0);
  else if (iupStrEqualNoCase(value, "UNDO"))
    iupScintillaSendMessage(ih, SCI_UNDO, 0, 0);
  else if (iupStrEqualNoCase(value, "REDO"))
    iupScintillaSendMessage(ih, SCI_REDO, 0, 0);

  return 0;
}
