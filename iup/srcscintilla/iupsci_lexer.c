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

#include "iupsci_lexer.h"
#include "iup_scintilla.h"

/***** LEXER *****
Attributes not implement yet:
SCI_SETLEXERLANGUAGE(<unused>, const char *name)
SCI_GETLEXERLANGUAGE(<unused>, char *name)
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

char* iupScintillaGetLexerAttrib(Ihandle* ih)
{
  int value = IUP_SSM(ih->handle, SCI_GETLEXER, 0, 0);

  if (value == SCLEX_CPP)
    return "CPP";
  else if (value == SCLEX_LUA)
    return "LUA";
  else
    return "NULL";
}

int iupScintillaSetLexerAttrib(Ihandle* ih, const char* value)
{
  if (iupStrEqualNoCase(value, "CPP"))
    IUP_SSM(ih->handle, SCI_SETLEXER, SCLEX_CPP, 0);
  else if (iupStrEqualNoCase(value, "LUA"))
    IUP_SSM(ih->handle, SCI_SETLEXER, SCLEX_LUA, 0);
  else
    IUP_SSM(ih->handle, SCI_SETLEXER, SCLEX_NULL, 0);

  return 0;
}

int iupScintillaSetKeyWordsAttrib(Ihandle* ih, int keyWordSet, const char* value)
{
  /* Note: You can set up to 9 lists of keywords for use by the current lexer */
  if(keyWordSet >= 0 && keyWordSet < 9)
    IUP_SSM(ih->handle, SCI_SETKEYWORDS, keyWordSet, (sptr_t)value);

  return 0;
}

char* iupScintillaGetPropertyAttrib(Ihandle* ih)
{
  char *strKey = iupStrGetMemory(50);
  char *strVal = iupStrGetMemory(50);
  char *str = iupStrGetMemory(101);

  IUP_SSM(ih->handle, SCI_GETPROPERTY, (sptr_t)strKey, (sptr_t)strVal);
  sprintf(str, "%s,%s", strKey, strVal);

  return str;
}

int iupScintillaSetPropertyAttrib(Ihandle* ih, const char* value)
{
  char *strKey = iupStrGetMemory(50);
  char *strVal = iupStrGetMemory(50);

  iupStrToStrStr(value, strKey, strVal, ',');

  IUP_SSM(ih->handle, SCI_SETPROPERTY, (sptr_t)strKey, (sptr_t)strVal);

  return 0;
}
