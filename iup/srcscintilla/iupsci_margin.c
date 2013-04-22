/** \file
 * \brief Scintilla control: Margin
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

#include "iupsci_margin.h"
#include "iupsci.h"

/***** MARGIN *****
SCI_SETMARGINCURSORN(int margin, int cursor)
SCI_GETMARGINCURSORN(int margin)
SCI_SETMARGINLEFT(<unused>, int pixels)
SCI_GETMARGINLEFT
SCI_SETMARGINRIGHT(<unused>, int pixels)
SCI_GETMARGINRIGHT
SCI_SETFOLDMARGINCOLOUR(bool useSetting, int colour)
SCI_SETFOLDMARGINHICOLOUR(bool useSetting, int colour)
SCI_MARGINSETTEXT(int line, char *text)
SCI_MARGINGETTEXT(int line, char *text)
SCI_MARGINSETSTYLE(int line, int style)
SCI_MARGINGETSTYLE(int line)
SCI_MARGINSETSTYLES(int line, char *styles)
SCI_MARGINGETSTYLES(int line, char *styles)
SCI_MARGINTEXTCLEARALL
SCI_MARGINSETSTYLEOFFSET(int style)
SCI_MARGINGETSTYLEOFFSET
SCI_SETMARGINOPTIONS(int marginOptions)
SCI_GETMARGINOPTIONS
*/

char* iupScintillaGetMarginTypeAttribId(Ihandle* ih, int margin)
{
  int type = iupScintillaSendMessage(ih, SCI_GETMARGINTYPEN, margin, 0);

  if (type == SC_MARGIN_NUMBER)
    return "NUMBER";
  if (type == SC_MARGIN_TEXT)
    return "TEXT";
  if (type == SC_MARGIN_RTEXT)
    return "RTEXT";
  if (type == SC_MARGIN_BACK)
    return "BACKGROUND";
  if (type == SC_MARGIN_FORE)
    return "FOREGROUND";

  return "SYMBOL";
}

int iupScintillaSetMarginTypeAttribId(Ihandle* ih, int margin, const char* value)
{
  if (iupStrEqualNoCase(value, "NUMBER"))
    iupScintillaSendMessage(ih, SCI_SETMARGINTYPEN, margin, SC_MARGIN_NUMBER);
  else if (iupStrEqualNoCase(value, "TEXT"))
    iupScintillaSendMessage(ih, SCI_SETMARGINTYPEN, margin, SC_MARGIN_TEXT);
  else if (iupStrEqualNoCase(value, "RTEXT"))
    iupScintillaSendMessage(ih, SCI_SETMARGINTYPEN, margin, SC_MARGIN_RTEXT);
  else if (iupStrEqualNoCase(value, "BACKGROUND"))
    iupScintillaSendMessage(ih, SCI_SETMARGINTYPEN, margin, SC_MARGIN_BACK);
  else if (iupStrEqualNoCase(value, "FOREGROUND"))
    iupScintillaSendMessage(ih, SCI_SETMARGINTYPEN, margin, SC_MARGIN_FORE);
  else  /* SYMBOL */
    iupScintillaSendMessage(ih, SCI_SETMARGINTYPEN, margin, SC_MARGIN_SYMBOL);

  return 0;
}

char* iupScintillaGetMarginWidthAttribId(Ihandle* ih, int margin)
{
  int pixelWidth = iupScintillaSendMessage(ih, SCI_GETMARGINWIDTHN, margin, 0);
  char* str = iupStrGetMemory(15);

  sprintf(str, "%d", pixelWidth);

  return str;
}

int iupScintillaSetMarginWidthAttribId(Ihandle* ih, int margin, const char* value)
{
  int pixelWidth;

  iupStrToInt(value, &pixelWidth);
  
  if(pixelWidth < 1)
    pixelWidth = 16;

  iupScintillaSendMessage(ih, SCI_SETMARGINWIDTHN, margin, pixelWidth);

  return 0;
}

char* iupScintillaGetMarginMaskFoldersAttribId(Ihandle* ih, int margin)
{
  int mask = iupScintillaSendMessage(ih, SCI_GETMARGINMASKN, margin, 0);
  if(mask & SC_MASK_FOLDERS)
    return "Yes";
  else
    return "No";
}

int iupScintillaSetMarginMaskFoldersAttribId(Ihandle* ih, int margin, const char* value)
{
  if (iupStrBoolean(value))
    iupScintillaSendMessage(ih, SCI_SETMARGINMASKN, margin, SC_MASK_FOLDERS);
  else
    iupScintillaSendMessage(ih, SCI_SETMARGINMASKN, margin, ~SC_MASK_FOLDERS);

  return 0;
}

char* iupScintillaGetMarginSensitiveAttribId(Ihandle* ih, int margin)
{
  if(iupScintillaSendMessage(ih, SCI_SETMARGINSENSITIVEN, margin, 0))
    return "YES";
  else
    return "NO";
}

int iupScintillaSetMarginSensitiveAttribId(Ihandle* ih, int margin, const char* value)
{
  if (iupStrEqualNoCase(value, "YES"))
    iupScintillaSendMessage(ih, SCI_SETMARGINSENSITIVEN, margin, 1);
  else
    iupScintillaSendMessage(ih, SCI_SETMARGINSENSITIVEN, margin, 0);

  return 0;
}
