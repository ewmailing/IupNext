/** \file
 * \brief Scintilla control: Searching
 *
 * See Copyright Notice in "iup.h"
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#undef SCI_NAMESPACE
#include <Scintilla.h>

#include "iup.h"

#include "iup_object.h"
#include "iup_attrib.h"
#include "iup_str.h"

#include "iupsci.h"


/***** Searching *****
-- SCI_FINDTEXT(int flags, Sci_TextToFind *ttf)
-- SCI_SEARCHANCHOR
-- SCI_SEARCHNEXT(int searchFlags, const char *text)
-- SCI_SEARCHPREV(int searchFlags, const char *text)
SCI_SETTARGETSTART(int pos)
SCI_GETTARGETSTART
SCI_SETTARGETEND(int pos)
SCI_GETTARGETEND
SCI_TARGETFROMSELECTION
SCI_SETSEARCHFLAGS(int searchFlags)
SCI_GETSEARCHFLAGS
SCI_SEARCHINTARGET(int length, const char *text)
SCI_REPLACETARGET(int length, const char *text)
-- SCI_REPLACETARGETRE(int length, const char *text)
-- SCI_GETTAG(int tagNumber, char *tagValue)
*/

static int iScintillaSetTargetFromSelectionAttrib(Ihandle* ih, const char* value)
{
  iupScintillaSendMessage(ih, SCI_TARGETFROMSELECTION, 0, 0);
  (void)value;
  return 0;
}

static char* iScintillaGetSearchFlagsAttrib(Ihandle* ih)
{
  int flags = iupScintillaSendMessage(ih, SCI_GETSEARCHFLAGS, 0, 0);
  char* str = iupStrGetMemory(50);

  sprintf(str, "");
  if (flags & SCFIND_MATCHCASE) sprintf(str, "MATCHCASE");

  if(strlen(str) > 0) sprintf(str, "%s|", str);
  if (flags & SCFIND_WHOLEWORD) sprintf(str, "%sWHOLEWORD", str);

  if(strlen(str) > 0) sprintf(str, "%s|", str);
  if (flags & SCFIND_WORDSTART) sprintf(str, "%sWORDSTART", str);

  if(strlen(str) > 0) sprintf(str, "%s|", str);
  if (flags & SCFIND_REGEXP) sprintf(str, "%sREGEXP", str);

  if(strlen(str) > 0) sprintf(str, "%s|", str);
  if (flags & SCFIND_POSIX) sprintf(str, "%sPOSIX", str);

  sprintf(str, "%s\0", str);

  return str;
}

static int iScintillaSetSearchFlagsAttrib(Ihandle* ih, const char* value)
{
  if (iupStrEqualNoCase(value, "MATCHCASE"))
    iupScintillaSendMessage(ih, SCI_SETSEARCHFLAGS, SCFIND_MATCHCASE, 0);
  else if (iupStrEqualNoCase(value, "WHOLEWORD"))
    iupScintillaSendMessage(ih, SCI_SETSEARCHFLAGS, SCFIND_WHOLEWORD, 0);
  else if (iupStrEqualNoCase(value, "WORDSTART"))
    iupScintillaSendMessage(ih, SCI_SETSEARCHFLAGS, SCFIND_WORDSTART, 0);
  else if (iupStrEqualNoCase(value, "REGEXP"))
    iupScintillaSendMessage(ih, SCI_SETSEARCHFLAGS, SCFIND_REGEXP, 0);
  else if (iupStrEqualNoCase(value, "POSIX"))
    iupScintillaSendMessage(ih, SCI_SETSEARCHFLAGS, SCFIND_POSIX, 0);
  else
    iupScintillaSendMessage(ih, SCI_SETSEARCHFLAGS, 0, 0);

  return 0;
}

static char* iScintillaGetTargetEndAttrib(Ihandle* ih)
{
  return iupStrReturnInt(iupScintillaSendMessage(ih, SCI_GETTARGETEND, 0, 0));
}

static int iScintillaSetTargetEndAttrib(Ihandle* ih, const char* value)
{
  int end;

  if (!value)
    return 0;

  iupStrToInt(value, &end);
  if (end < 1) end = iupScintillaSendMessage(ih, SCI_GETTEXTLENGTH, 0, 0);

  iupScintillaSendMessage(ih, SCI_SETTARGETEND, end, 0);

  return 0;
}

static char* iScintillaGetTargetStartAttrib(Ihandle* ih)
{
  return iupStrReturnInt(iupScintillaSendMessage(ih, SCI_GETTARGETSTART, 0, 0));
}

static int iScintillaSetTargetStartAttrib(Ihandle* ih, const char* value)
{
  int start;

  if (!value)
    return 0;

  iupStrToInt(value, &start);
  if (start < 1) start = 1;

  iupScintillaSendMessage(ih, SCI_SETTARGETSTART, start, 0);

  return 0;
}

static int iScintillaSetSearchInTargetAttrib(Ihandle* ih, const char* value)
{
  int len = strlen(value);

  if (!value)
    return 0;

  iupScintillaSendMessage(ih, SCI_SEARCHINTARGET, len, (sptr_t)value);

  return 0;
}

static int iScintillaSetReplaceTargetAttrib(Ihandle* ih, const char* value)
{
  if (!value)
    return 0;

  iupScintillaSendMessage(ih, SCI_REPLACETARGET, (uptr_t)-1, (sptr_t)value);
  return 0;
}

void iupScintillaRegisterSearching(Iclass* ic)
{
  iupClassRegisterAttribute(ic, "SEARCHFLAGS", iScintillaGetSearchFlagsAttrib, iScintillaSetSearchFlagsAttrib, NULL, NULL, IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "TARGETEND", iScintillaGetTargetEndAttrib, iScintillaSetTargetEndAttrib, NULL, NULL, IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "TARGETSTART", iScintillaGetTargetStartAttrib, iScintillaSetTargetStartAttrib, NULL, NULL, IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "TARGETFROMSELECTION", NULL, iScintillaSetTargetFromSelectionAttrib, NULL, NULL, IUPAF_WRITEONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "SEARCHINTARGET", NULL, iScintillaSetSearchInTargetAttrib, NULL, NULL, IUPAF_WRITEONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "REPLACETARGET", NULL, iScintillaSetReplaceTargetAttrib, NULL, NULL, IUPAF_WRITEONLY|IUPAF_NO_INHERIT);
}
