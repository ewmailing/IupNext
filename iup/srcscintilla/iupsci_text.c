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

#include "iupsci_text.h"
#include "iup_scintilla.h"

/***** TEXT RETRIEVAL AND MODIFICATION *****
Attributes not implement yet:
SCI_GETTEXTRANGE(<unused>, Sci_TextRange *tr)
SCI_SETSAVEPOINT
SCI_ALLOCATE(int bytes, <unused>)
SCI_ADDSTYLEDTEXT(int length, cell *s)
SCI_GETSTYLEDTEXT(<unused>, Sci_TextRange *tr)
SCI_GETSTYLEAT(int position)
SCI_SETSTYLEBITS(int bits)
SCI_GETSTYLEBITS
SCI_RELEASEALLEXTENDEDSTYLES
SCI_ALLOCATEEXTENDEDSTYLES(int numberStyles)
SCI_TARGETASUTF8(<unused>, char *s)  // only GTK
SCI_ENCODEDFROMUTF8(const char *utf8, char *encoded)  // only GTK
SCI_SETLENGTHFORENCODE(int bytes)  // only GTK
*/
char* iupScintillaGetValueAttrib(Ihandle* ih)
{
  int len = IUP_SSM(ih->handle, SCI_GETTEXTLENGTH, 0, 0);
  char* str = iupStrGetMemory(len);

  IUP_SSM(ih->handle, SCI_GETTEXT, len, (sptr_t)str);
  
  return str;
}

int iupScintillaSetValueAttrib(Ihandle* ih, const char* value)
{
  IUP_SSM(ih->handle, SCI_SETTEXT, 0, (sptr_t)value);
  return 0;
}

char* iupScintillaGetLineAttrib(Ihandle* ih, int line)
{
  int len = IUP_SSM(ih->handle, SCI_LINELENGTH, line, 0);
  char* str = iupStrGetMemory(len); 
  IUP_SSM(ih->handle, SCI_GETLINE, line, (sptr_t)str);
  return str;
}

char* iupScintillaGetReadOnlyAttrib(Ihandle* ih)
{
  if(IUP_SSM(ih->handle, SCI_GETREADONLY, 0, 0))
    return "YES";
  else
    return "NO";
}

int iupScintillaSetReadOnlyAttrib(Ihandle* ih, const char* value)
{
  if (iupStrEqualNoCase(value, "YES"))
    IUP_SSM(ih->handle, SCI_SETREADONLY, 1, 0);
  else
    IUP_SSM(ih->handle, SCI_SETREADONLY, 0, 0);

  return 0;
}

int iupScintillaSetPrependTextAttrib(Ihandle* ih, const char* value)
{
  int len = strlen(value);

  IUP_SSM(ih->handle, SCI_ADDTEXT, len, (sptr_t)value);

  if(ih->data->append_newline)
    IUP_SSM(ih->handle, SCI_ADDTEXT, 1, (sptr_t)"\n");

  return 0;
}

int iupScintillaSetAppendTextAttrib(Ihandle* ih, const char* value)
{
  int len = strlen(value);

  if(ih->data->append_newline)
    IUP_SSM(ih->handle, SCI_APPENDTEXT, 1, (sptr_t)"\n");

  IUP_SSM(ih->handle, SCI_APPENDTEXT, len, (sptr_t)value);
  return 0;
}

int iupScintillaSetInsertTextAttrib(Ihandle* ih, int pos, const char* value)
{
  IUP_SSM(ih->handle, SCI_INSERTTEXT, pos, (sptr_t)value);
  return 0;
}

int iupScintillaSetClearAllAttrib(Ihandle* ih, const char* value)
{
  (void)value;

  IUP_SSM(ih->handle, SCI_CLEARALL, 0, 0);
  return 0;
}

int iupScintillaSetClearDocumentAttrib(Ihandle* ih, const char* value)
{
  (void)value;

  IUP_SSM(ih->handle, SCI_CLEARDOCUMENTSTYLE, 0, 0);
  return 0;
}

int iupScintillaSetDeleteRangeAttrib(Ihandle* ih, const char* value)
{
  int pos, len;
  iupStrToIntInt(value, &pos, &len, ',');

  IUP_SSM(ih->handle, SCI_DELETERANGE, pos, len);
  return 0;
}

char* iupScintillaGetCharAtAttrib(Ihandle* ih, int pos)
{
  return (char*)IUP_SSM(ih->handle, SCI_GETCHARAT, pos, 0);
}
