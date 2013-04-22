/** \file
 * \brief Scintilla control: Lexer
 *
 * See Copyright Notice in "iup.h"
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <Scintilla.h>
#include <SciLexer.h>

#include "iup.h"

#include "iup_object.h"
#include "iup_attrib.h"
#include "iup_str.h"

#include "iupsci_lexer.h"
#include "iupsci.h"

/***** LEXER *****
Attributes not implement yet:
SCI_LOADLEXERLIBRARY(<unused>, const char *path)
SCI_COLOURISE(int start, int end)
SCI_CHANGELEXERSTATE(int start, int end)
SCI_PROPERTYNAMES(<unused>, char *names)
SCI_PROPERTYTYPE(const char *name)
SCI_DESCRIBEPROPERTY(const char *name, char *description)
SCI_GETPROPERTYEXPANDED(const char *key, char *value)
SCI_GETPROPERTYINT(const char *key, int default)
SCI_DESCRIBEKEYWORDSETS(<unused>, char *descriptions)
SCI_GETSTYLEBITSNEEDED
SCI_GETSUBSTYLEBASES(<unused>, char *styles)
SCI_DISTANCETOSECONDARYSTYLES
SCI_ALLOCATESUBSTYLES(int styleBase, int numberStyles)
SCI_FREESUBSTYLES
SCI_GETSUBSTYLESSTART(int styleBase)
SCI_GETSUBSTYLESLENGTH(int styleBase)
SCI_SETIDENTIFIERS(int style, const char *identifiers)
*/


char* iupScintillaGetLexerLanguageAttrib(Ihandle* ih)
{
  int len = iupScintillaSendMessage(ih, SCI_GETLEXERLANGUAGE, 0, (sptr_t)NULL);
  char *str = iupStrGetMemory(len+1);
  len = iupScintillaSendMessage(ih, SCI_GETLEXERLANGUAGE, 0, (sptr_t)str);
  if (len)
  {
    if (!iupStrEqual(str, "null"))
      return str;
  }
  return NULL;
}

int iupScintillaSetLexerLanguageAttrib(Ihandle* ih, const char* value)
{
  if (!value)
    iupScintillaSendMessage(ih, SCI_SETLEXER, SCLEX_NULL, 0);
  else
    iupScintillaSendMessage(ih, SCI_SETLEXERLANGUAGE, 0, (sptr_t)value);
  return 0;
}

int iupScintillaSetKeyWordsAttrib(Ihandle* ih, int keyWordSet, const char* value)
{
  /* Note: You can set up to 9 lists of keywords for use by the current lexer */
  if(keyWordSet >= 0 && keyWordSet < 9)
    iupScintillaSendMessage(ih, SCI_SETKEYWORDS, keyWordSet, (sptr_t)value);

  return 0;
}

char* iupScintillaGetPropertyAttrib(Ihandle* ih)
{
  char* strKey = iupAttribGetStr(ih, "PROPERTYNAME");
  if (strKey)
  {
    int len = (int)iupScintillaSendMessage(ih, SCI_GETPROPERTY, (uptr_t)strKey, (sptr_t)NULL);
    char *str = iupStrGetMemory(len+1);

    len = iupScintillaSendMessage(ih, SCI_GETPROPERTY, (uptr_t)strKey, (sptr_t)str);
    if (len)
      return str;
  }

  return NULL;
}

int iupScintillaSetPropertyAttrib(Ihandle* ih, const char* value)
{
  char strKey[50];
  char strVal[50];

  iupStrToStrStr(value, strKey, strVal, '=');

  iupScintillaSendMessage(ih, SCI_SETPROPERTY, (uptr_t)strKey, (sptr_t)strVal);

  return 0;
}
