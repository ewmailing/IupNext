/** \file
 * \brief Scintilla control: Cut, copy and paste, and Undo
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
SCI_CUT
SCI_COPY
SCI_PASTE
SCI_CLEAR
SCI_CANPASTE
   --SCI_COPYRANGE(int start, int end)
   --SCI_COPYTEXT(int length, const char *text)
   --SCI_COPYALLOWLINE
   --SCI_SETPASTECONVERTENDINGS(bool convert)
   --SCI_GETPASTECONVERTENDINGS
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

  return 0;
}

char* iupScintillaGetCanPasteAttrib(Ihandle* ih)
{
  if (iupScintillaSendMessage(ih, SCI_CANPASTE, 0, 0))
    return "YES";
  else
    return "NO";
}

/***** Undo and Redo *****
SCI_UNDO
SCI_CANUNDO
SCI_EMPTYUNDOBUFFER
SCI_REDO
SCI_CANREDO
SCI_SETUNDOCOLLECTION(bool collectUndo)
SCI_GETUNDOCOLLECTION
--SCI_BEGINUNDOACTION
--SCI_ENDUNDOACTION
--SCI_ADDUNDOACTION(int token, int flags)
*/

int iupScintillaSetUndoAttrib(Ihandle *ih, const char *value)
{
  if (iupStrBoolean(value))
    iupScintillaSendMessage(ih, SCI_UNDO, 0, 0);
  else
    iupScintillaSendMessage(ih, SCI_EMPTYUNDOBUFFER, 0, 0);
  return 0;
}

char* iupScintillaGetUndoAttrib(Ihandle* ih)
{
  if (iupScintillaSendMessage(ih, SCI_CANUNDO, 0, 0))
    return "YES";
  else
    return "NO";
}

int iupScintillaSetRedoAttrib(Ihandle *ih, const char *value)
{
  if (iupStrBoolean(value))
    iupScintillaSendMessage(ih, SCI_REDO, 0, 0);
  else
    iupScintillaSendMessage(ih, SCI_EMPTYUNDOBUFFER, 0, 0);
  return 0;
}

char* iupScintillaGetRedoAttrib(Ihandle* ih)
{
  if (iupScintillaSendMessage(ih, SCI_CANREDO, 0, 0))
    return "YES";
  else
    return "NO";
}

int iupScintillaSetUndoCollectAttrib(Ihandle *ih, const char *value)
{
  if (iupStrBoolean(value))
    iupScintillaSendMessage(ih, SCI_SETUNDOCOLLECTION, 1, 0);
  else
    iupScintillaSendMessage(ih, SCI_SETUNDOCOLLECTION, 0, 0);
  return 0;
}

char* iupScintillaGetUndoCollectAttrib(Ihandle* ih)
{
  if (iupScintillaSendMessage(ih, SCI_GETUNDOCOLLECTION, 0, 0))
    return "YES";
  else
    return "NO";
}

