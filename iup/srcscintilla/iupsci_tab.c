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
SCI_SETHIGHLIGHTGUIDE(int column)
SCI_GETHIGHLIGHTGUIDE
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
