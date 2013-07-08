/** \file
 * \brief Scintilla control: Margin
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

#include "iupsci_margin.h"
#include "iupsci.h"

/***** MARGIN *****
SCI_SETMARGINTYPEN(int margin, int type)
SCI_GETMARGINTYPEN(int margin)
SCI_SETMARGINWIDTHN(int margin, int pixelWidth)
SCI_GETMARGINWIDTHN(int margin)
SCI_SETMARGINMASKN(int margin, int mask)
SCI_GETMARGINMASKN(int margin)
SCI_SETMARGINSENSITIVEN(int margin, bool sensitive)
SCI_GETMARGINSENSITIVEN(int margin)
--SCI_SETMARGINCURSORN(int margin, int cursor)
--SCI_GETMARGINCURSORN(int margin)
SCI_SETMARGINLEFT(<unused>, int pixels)
SCI_GETMARGINLEFT
SCI_SETMARGINRIGHT(<unused>, int pixels)
SCI_GETMARGINRIGHT
--SCI_SETFOLDMARGINCOLOUR(bool useSetting, int colour)
--SCI_SETFOLDMARGINHICOLOUR(bool useSetting, int colour)
SCI_MARGINSETTEXT(int line, char *text)
SCI_MARGINGETTEXT(int line, char *text)
SCI_MARGINSETSTYLE(int line, int style)
SCI_MARGINGETSTYLE(int line)
--SCI_MARGINSETSTYLES(int line, char *styles)
--SCI_MARGINGETSTYLES(int line, char *styles)
SCI_MARGINTEXTCLEARALL
--SCI_MARGINSETSTYLEOFFSET(int style)
--SCI_MARGINGETSTYLEOFFSET
--SCI_SETMARGINOPTIONS(int marginOptions)
--SCI_GETMARGINOPTIONS
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
  return iupStrReturnInt(pixelWidth);
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
    return "YES";
  else
    return "NO";
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
  if (iupStrBoolean(value))
    iupScintillaSendMessage(ih, SCI_SETMARGINSENSITIVEN, margin, 1);
  else
    iupScintillaSendMessage(ih, SCI_SETMARGINSENSITIVEN, margin, 0);

  return 0;
}

char* iupScintillaGetMarginLeftAttrib(Ihandle* ih)
{
  int pixels = iupScintillaSendMessage(ih, SCI_GETMARGINLEFT, 0, 0);
  return iupStrReturnInt(pixels);
}

int iupScintillaSetMarginLeftAttrib(Ihandle* ih, const char* value)
{
  int pixels;

  iupStrToInt(value, &pixels);
  
  if(pixels < 1)
    pixels = 16;

  iupScintillaSendMessage(ih, SCI_SETMARGINLEFT, 0, pixels);

  return 0;
}

char* iupScintillaGetMarginRightAttrib(Ihandle* ih)
{
  int pixels = iupScintillaSendMessage(ih, SCI_GETMARGINRIGHT, 0, 0);
  return iupStrReturnInt(pixels);
}

int iupScintillaSetMarginRightAttrib(Ihandle* ih, const char* value)
{
  int pixels;

  iupStrToInt(value, &pixels);
  
  if(pixels < 1)
    pixels = 16;

  iupScintillaSendMessage(ih, SCI_SETMARGINRIGHT, 0, pixels);

  return 0;
}

char* iupScintillaGetMarginTextAttribId(Ihandle* ih, int line)
{
  int len = iupScintillaSendMessage(ih, SCI_MARGINGETTEXT, line, 0);
  char* str = iupStrGetMemory(len+1);
  iupScintillaSendMessage(ih, SCI_MARGINGETTEXT, line, (sptr_t)str);
  return str;
}

int iupScintillaSetMarginTextAttribId(Ihandle* ih, int line, const char* value)
{
  iupScintillaSendMessage(ih, SCI_MARGINSETTEXT, line, (sptr_t)value);
  return 0;
}

int iupScintillaSetMarginTextClearAllAttrib(Ihandle* ih, const char* value)
{
  (void)value;
  iupScintillaSendMessage(ih, SCI_MARGINTEXTCLEARALL, 0, 0);
  return 0;
}

char* iupScintillaGetMarginTextStyleAttribId(Ihandle* ih, int line)
{
  int style = iupScintillaSendMessage(ih, SCI_MARGINGETSTYLE, line, 0);
  return iupStrReturnInt(style);
}

int iupScintillaSetMarginTextStyleAttribId(Ihandle* ih, int line, const char* value)
{
  int style;

  iupStrToInt(value, &style);
  
  iupScintillaSendMessage(ih, SCI_MARGINSETSTYLE, line, style);

  return 0;
}

char* iupScintillaGetMarginCursorAttribId(Ihandle* ih, int margin)
{
  if(iupScintillaSendMessage(ih, SCI_GETMARGINCURSORN, margin, 0) == SC_CURSORARROW)
    return "ARROW";
  else
    return "REVERSEARROW";
}

int iupScintillaSetMarginCursorAttribId(Ihandle* ih, int margin, const char* value)
{
  if (iupStrEqualNoCase(value, "ARROW"))
    iupScintillaSendMessage(ih, SCI_SETMARGINCURSORN, margin, SC_CURSORARROW);
  else  /* REVERSEARROW */
    iupScintillaSendMessage(ih, SCI_SETMARGINCURSORN, margin, SC_CURSORREVERSEARROW);

  return 0;
}

