/** \file
 * \brief Scintilla control: Tabs and Indentation Guides
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

#include "iupsci_tab.h"
#include "iupsci.h"

/***** TABS AND INDENTATION GUIDES ****
Attributes not implement yet:
SCI_SETUSETABS(bool useTabs)
SCI_GETUSETABS
SCI_SETINDENT(int widthInChars)
SCI_GETINDENT
SCI_SETTABINDENTS(bool tabIndents)
SCI_GETTABINDENTS
SCI_SETBACKSPACEUNINDENTS(bool bsUnIndents)
SCI_GETBACKSPACEUNINDENTS
SCI_SETLINEINDENTATION(int line, int indentation)
SCI_GETLINEINDENTATION(int line)
SCI_GETLINEINDENTPOSITION(int line)
SCI_SETINDENTATIONGUIDES(int indentView)
SCI_GETINDENTATIONGUIDES
*/

char* iupScintillaGetTabSizeAttrib(Ihandle *ih)
{
  int widthInChars;
  char* str = iupStrGetMemory(100);

  widthInChars = iupScintillaSendMessage(ih, SCI_GETTABWIDTH, 0, 0);
  sprintf(str, "%d", widthInChars);

  return str;
}

int iupScintillaSetTabSizeAttrib(Ihandle *ih, const char *value)
{
  int widthInChars;
  iupStrToInt(value, &widthInChars);

  if(widthInChars < 2)
    widthInChars = 2;

  iupScintillaSendMessage(ih, SCI_SETTABWIDTH, widthInChars, 0);

  return 0;
}

char* iupScintillaGetHighlightGuideAttrib(Ihandle *ih)
{
  int col = iupScintillaSendMessage(ih, SCI_GETHIGHLIGHTGUIDE, 0, 0);
  char* str = iupStrGetMemory(15);

  sprintf(str, "%d", col);

  return str;
}

int iupScintillaSetHighlightGuideAttrib(Ihandle *ih, const char *value)
{
  int col;
  if (!iupStrToInt(value, &col))
    return 0;

  iupScintillaSendMessage(ih, SCI_SETHIGHLIGHTGUIDE, col, 0);

  return 0;
}
