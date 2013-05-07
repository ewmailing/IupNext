/** \file
 * \brief Scintilla control: Text retrieval and modification
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

#include "iupsci_text.h"
#include "iupsci.h"

/***** TEXT RETRIEVAL AND MODIFICATION *****
SCI_GETTEXT(int length, char *text)
SCI_SETTEXT(<unused>, const char *text)
SCI_SETSAVEPOINT
SCI_GETLINE(int line, char *text)
   --SCI_REPLACESEL(<unused>, const char *text)
SCI_SETREADONLY(bool readOnly)
SCI_GETREADONLY
   --SCI_GETTEXTRANGE(<unused>, Sci_TextRange *tr)
   --SCI_ALLOCATE(int bytes, <unused>)
SCI_ADDTEXT(int length, const char *s)
   --SCI_ADDSTYLEDTEXT(int length, cell *s)
SCI_APPENDTEXT(int length, const char *s)
SCI_INSERTTEXT(int pos, const char *text)
SCI_CLEARALL
SCI_DELETERANGE(int pos, int deleteLength)
SCI_CLEARDOCUMENTSTYLE
SCI_GETCHARAT(int position)
   --SCI_GETSTYLEAT(int position)
   --SCI_GETSTYLEDTEXT(<unused>, Sci_TextRange *tr)
   --SCI_SETSTYLEBITS(int bits)
   --SCI_GETSTYLEBITS
   --SCI_TARGETASUTF8(<unused>, char *s)
   --SCI_ENCODEDFROMUTF8(const char *utf8, char *encoded)
   --SCI_SETLENGTHFORENCODE(int bytes)
*/

char* iupScintillaGetValueAttrib(Ihandle* ih)
{
  int len = iupScintillaSendMessage(ih, SCI_GETTEXTLENGTH, 0, 0);
  char* str = iupStrGetMemory(len+1);
  iupScintillaSendMessage(ih, SCI_GETTEXT, len, (sptr_t)str);
  return str;
}

int iupScintillaSetValueAttrib(Ihandle* ih, const char* value)
{
  ih->data->ignore_change = 1;
  iupScintillaSendMessage(ih, SCI_SETTEXT, 0, (sptr_t)value);
  ih->data->ignore_change = 0;
  return 0;
}

char* iupScintillaGetLineAttrib(Ihandle* ih, int line)
{
  int len = iupScintillaSendMessage(ih, SCI_LINELENGTH, line, 0);
  char* str = iupStrGetMemory(len+1); 
  iupScintillaSendMessage(ih, SCI_GETLINE, line, (sptr_t)str);
  return str;
}

char* iupScintillaGetReadOnlyAttrib(Ihandle* ih)
{
  if(iupScintillaSendMessage(ih, SCI_GETREADONLY, 0, 0))
    return "YES";
  else
    return "NO";
}

int iupScintillaSetReadOnlyAttrib(Ihandle* ih, const char* value)
{
  if (iupStrBoolean(value))
    iupScintillaSendMessage(ih, SCI_SETREADONLY, 1, 0);
  else
    iupScintillaSendMessage(ih, SCI_SETREADONLY, 0, 0);

  return 0;
}

int iupScintillaSetPrependTextAttrib(Ihandle* ih, const char* value)
{
  int len = strlen(value);

  ih->data->ignore_change = 1;
  iupScintillaSendMessage(ih, SCI_ADDTEXT, len, (sptr_t)value);

  if(ih->data->append_newline)
    iupScintillaSendMessage(ih, SCI_ADDTEXT, 1, (sptr_t)"\n");
  ih->data->ignore_change = 0;

  return 0;
}

int iupScintillaSetAppendTextAttrib(Ihandle* ih, const char* value)
{
  int len = strlen(value);

  ih->data->ignore_change = 1;
  if(ih->data->append_newline)
    iupScintillaSendMessage(ih, SCI_APPENDTEXT, 1, (sptr_t)"\n");

  iupScintillaSendMessage(ih, SCI_APPENDTEXT, len, (sptr_t)value);
  ih->data->ignore_change = 0;

  return 0;
}

int iupScintillaSetInsertTextAttrib(Ihandle* ih, int pos, const char* value)
{
  ih->data->ignore_change = 1;
  iupScintillaSendMessage(ih, SCI_INSERTTEXT, pos, (sptr_t)value);
  ih->data->ignore_change = 0;
  return 0;
}

int iupScintillaSetClearAllAttrib(Ihandle* ih, const char* value)
{
  (void)value;

  ih->data->ignore_change = 1;
  iupScintillaSendMessage(ih, SCI_CLEARALL, 0, 0);
  ih->data->ignore_change = 0;
  return 0;
}

int iupScintillaSetClearDocumentAttrib(Ihandle* ih, const char* value)
{
  (void)value;

  iupScintillaSendMessage(ih, SCI_CLEARDOCUMENTSTYLE, 0, 0);
  return 0;
}

int iupScintillaSetSavePointAttrib(Ihandle* ih, const char* value)
{
  (void)value;

  iupScintillaSendMessage(ih, SCI_SETSAVEPOINT, 0, 0);
  return 0;
}

char* iupScintillaGetModifyAttrib(Ihandle* ih)
{
  if (iupScintillaSendMessage(ih, SCI_GETMODIFY, 0, 0))
    return "YES";
  else
    return "NO";
}

int iupScintillaSetDeleteRangeAttrib(Ihandle* ih, const char* value)
{
  int pos, len;
  iupStrToIntInt(value, &pos, &len, ',');

  ih->data->ignore_change = 1;
  iupScintillaSendMessage(ih, SCI_DELETERANGE, pos, len);
  ih->data->ignore_change = 0;
  return 0;
}

char* iupScintillaGetCharAttrib(Ihandle* ih, int pos)
{
  return (char*)iupScintillaSendMessage(ih, SCI_GETCHARAT, pos, 0);
}
