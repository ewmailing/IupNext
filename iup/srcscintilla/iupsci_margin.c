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

#include "iupsci_margin.h"
#include "iup_scintilla.h"

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

char* iupScintillaGetMarginTypeNAttrib(Ihandle* ih, int margin)
{
  int type = IUP_SSM(ih->handle, SCI_GETMARGINTYPEN, margin, 0);

  if (type == SC_MARGIN_NUMBER)
    return "MARGIN_NUMBER";
  if (type == SC_MARGIN_TEXT)
    return "MARGIN_TEXT";
  if (type == SC_MARGIN_RTEXT)
    return "MARGIN_RTEXT";
  if (type == SC_MARGIN_BACK)
    return "MARGIN_BACK";
  if (type == SC_MARGIN_FORE)
    return "MARGIN_FORE";

  return "MARGIN_SYMBOL";
}

int iupScintillaSetMarginTypeNAttrib(Ihandle* ih, int margin, const char* value)
{
  if (iupStrEqualNoCase(value, "MARGIN_NUMBER"))
    IUP_SSM(ih->handle, SCI_SETMARGINTYPEN, margin, SC_MARGIN_NUMBER);
  else if (iupStrEqualNoCase(value, "MARGIN_TEXT"))
    IUP_SSM(ih->handle, SCI_SETMARGINTYPEN, margin, SC_MARGIN_TEXT);
  else if (iupStrEqualNoCase(value, "MARGIN_RTEXT"))
    IUP_SSM(ih->handle, SCI_SETMARGINTYPEN, margin, SC_MARGIN_RTEXT);
  else if (iupStrEqualNoCase(value, "MARGIN_BACK"))
    IUP_SSM(ih->handle, SCI_SETMARGINTYPEN, margin, SC_MARGIN_BACK);
  else if (iupStrEqualNoCase(value, "MARGIN_FORE"))
    IUP_SSM(ih->handle, SCI_SETMARGINTYPEN, margin, SC_MARGIN_FORE);
  else  /* MARGIN_SYMBOL */
    IUP_SSM(ih->handle, SCI_SETMARGINTYPEN, margin, SC_MARGIN_SYMBOL);

  return 0;
}

char* iupScintillaGetMarginWidthNAttrib(Ihandle* ih, int margin)
{
  int pixelWidth = IUP_SSM(ih->handle, SCI_GETMARGINWIDTHN, margin, 0);
  char* str = iupStrGetMemory(15);

  sprintf(str, "%d", pixelWidth);

  return str;
}

int iupScintillaSetMarginWidthNAttrib(Ihandle* ih, int margin, const char* value)
{
  int pixelWidth;

  iupStrToInt(value, &pixelWidth);
  
  if(pixelWidth < 1)
    pixelWidth = 16;

  IUP_SSM(ih->handle, SCI_SETMARGINWIDTHN, margin, pixelWidth);

  return 0;
}

char* iupScintillaGetMarginMaskNAttrib(Ihandle* ih, int margin)
{
  if(IUP_SSM(ih->handle, SCI_GETMARGINMASKN, margin, 0) == SC_MASK_FOLDERS)
    return "MASK_FOLDERS";
  else
    return "NO_MASK_FOLDERS";
}

int iupScintillaSetMarginMaskNAttrib(Ihandle* ih, int margin, const char* value)
{
  if (iupStrEqualNoCase(value, "MASK_FOLDERS"))
    IUP_SSM(ih->handle, SCI_SETMARGINMASKN, margin, SC_MASK_FOLDERS);
  else
    IUP_SSM(ih->handle, SCI_SETMARGINMASKN, margin, ~SC_MASK_FOLDERS);

  return 0;
}

char* iupScintillaGetMarginSensitiveNAttrib(Ihandle* ih, int margin)
{
  if(IUP_SSM(ih->handle, SCI_SETMARGINSENSITIVEN, margin, 0))
    return "YES";
  else
    return "NO";
}

int iupScintillaSetMarginSensitiveNAttrib(Ihandle* ih, int margin, const char* value)
{
  if (iupStrEqualNoCase(value, "YES"))
    IUP_SSM(ih->handle, SCI_SETMARGINSENSITIVEN, margin, 1);
  else
    IUP_SSM(ih->handle, SCI_SETMARGINSENSITIVEN, margin, 0);

  return 0;
}
