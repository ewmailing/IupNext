/** \file
 * \brief Scintilla control: Selection and information
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
#include "iup_scintilla.h"

#include "iup_object.h"
#include "iup_attrib.h"
#include "iup_str.h"

#include "iupsci_selection.h"
#include "iupsci.h"

/***** SELECTION AND INFORMATION *****
Attributes not implement yet:
SCI_SETFIRSTVISIBLELINE(int lineDisplay)
SCI_GETFIRSTVISIBLELINE
SCI_GOTOLINE(int line)
SCI_SETCURRENTPOS(int position)
SCI_GETCURRENTPOS
SCI_SETANCHOR(int position)
SCI_GETANCHOR
SCI_SETSELECTIONSTART(int position)
SCI_GETSELECTIONSTART
SCI_SETSELECTIONEND(int position)
SCI_GETSELECTIONEND
SCI_SETEMPTYSELECTION(int pos)
SCI_LINEFROMPOSITION(int position)
SCI_POSITIONFROMLINE(int line)
SCI_GETLINEENDPOSITION(int line)
SCI_LINELENGTH(int line)
SCI_GETCOLUMN(int position)
SCI_FINDCOLUMN(int line, int column)
SCI_POSITIONFROMPOINT(int x, int y)
SCI_POSITIONFROMPOINTCLOSE(int x, int y)
SCI_CHARPOSITIONFROMPOINT(int x, int y)
SCI_CHARPOSITIONFROMPOINTCLOSE(int x, int y)
SCI_POINTXFROMPOSITION(<unused>, int position)
SCI_POINTYFROMPOSITION(<unused>, int position)
SCI_HIDESELECTION(bool hide)
SCI_SELECTIONISRECTANGLE
SCI_SETSELECTIONMODE(int mode)
SCI_GETSELECTIONMODE
SCI_GETLINESELSTARTPOSITION(int line)
SCI_GETLINESELENDPOSITION(int line)
SCI_WORDENDPOSITION(int position, bool onlyWordCharacters)
SCI_WORDSTARTPOSITION(int position, bool onlyWordCharacters)
SCI_POSITIONBEFORE(int position)
SCI_POSITIONAFTER(int position)
SCI_COUNTCHARACTERS(int startPos, int endPos)
SCI_TEXTWIDTH(int styleNumber, const char *text)
SCI_TEXTHEIGHT(int line)
SCI_CHOOSECARETX
SCI_MOVESELECTEDLINESUP
SCI_MOVESELECTEDLINESDOWN
*/

char* iupScintillaGetCurrentLineAttrib(Ihandle* ih)
{
  int textLen = iupScintillaSendMessage(ih, SCI_GETLINECOUNT, 0, 0);
  char* str = iupStrGetMemory(textLen+1);
  
  iupScintillaSendMessage(ih, SCI_GETCURLINE, textLen, (sptr_t)str);
  
  return str;
}

char* iupScintillaGetCountAttrib(Ihandle* ih)
{
  char* str = iupStrGetMemory(100);
  int count = iupScintillaSendMessage(ih, SCI_GETTEXTLENGTH, 0, 0);
  
  sprintf(str, "%d", count);

  return str;
}

char* iupScintillaGetLineCountAttrib(Ihandle* ih)
{
  char* str = iupStrGetMemory(100);
  int count = iupScintillaSendMessage(ih, SCI_GETLINECOUNT, 0, 0);
  
  sprintf(str, "%d", count);

  return str;
}

char* iupScintillaGetCaretAttrib(Ihandle* ih)
{
  int col, lin, pos;
  char* str = iupStrGetMemory(100);

  pos = iupScintillaSendMessage(ih, SCI_GETCURRENTPOS, 0, 0);
  iupScintillaConvertPosToLinCol(ih, pos, &lin, &col);

  sprintf(str, "%d,%d", lin, col);

  return str;
}

int iupScintillaSetCaretAttrib(Ihandle* ih, const char* value)
{
  int pos, lin = 1, col = 1;
  iupStrToIntInt(value, &lin, &col, ',');  /* be permissive in SetCaret, do not abort if invalid */
  if (lin < 1) lin = 1;
  if (col < 1) col = 1;
  
  iupScintillaConvertLinColToPos(ih, lin, col, &pos);

  iupScintillaSendMessage(ih, SCI_GOTOPOS, pos, 0);

  return 0;
}

char* iupScintillaGetCaretPosAttrib(Ihandle* ih)
{
  char* str = iupStrGetMemory(100);

  sprintf(str, "%d", (int)iupScintillaSendMessage(ih, SCI_GETCURRENTPOS, 0, 0));

  return str;
}

int iupScintillaSetCaretPosAttrib(Ihandle* ih, const char* value)
{
  int pos = 0;

  if (!value)
    return 0;

  sscanf(value,"%i",&pos);    /* be permissive in SetCaret, do not abort if invalid */
  if (pos < 0) pos = 0;

  iupScintillaSendMessage(ih, SCI_GOTOPOS, pos, 0);

  return 0;
}

int iupScintillaSetCaretToViewAttrib(Ihandle *ih, const char *value)
{
  (void)value;
  iupScintillaSendMessage(ih, SCI_MOVECARETINSIDEVIEW, 0, 0);
  return 0;
}

char* iupScintillaGetSelectedTextAttrib(Ihandle* ih)
{
  int start = iupScintillaSendMessage(ih, SCI_GETSELECTIONSTART, 0, 0);
  int end   = iupScintillaSendMessage(ih, SCI_GETSELECTIONEND, 0, 0);
  char* str;
  
  if(start == end)
    return NULL;

  str = iupStrGetMemory(end - start + 1 + 1);
  iupScintillaSendMessage(ih, SCI_GETSELTEXT, 0, (sptr_t)str);

  return str;
}

int iupScintillaSetSelectedTextAttrib(Ihandle* ih, const char* value)
{
  int start = iupScintillaSendMessage(ih, SCI_GETSELECTIONSTART, 0, 0);
  int end   = iupScintillaSendMessage(ih, SCI_GETSELECTIONEND, 0, 0);
  
  if(start == end)
    return 0;

  iupScintillaSendMessage(ih, SCI_REPLACESEL, 0, (sptr_t)value);

  return 0;
}

char* iupScintillaGetSelectionAttrib(Ihandle* ih)
{
  char* str;
  int start = iupScintillaSendMessage(ih, SCI_GETSELECTIONSTART, 0, 0);
  int end   = iupScintillaSendMessage(ih, SCI_GETSELECTIONEND, 0, 0);
  int start_col, start_lin, end_col, end_lin;

  if (start == end)
    return NULL;

  str = iupStrGetMemory(100);

  start_lin = iupScintillaSendMessage(ih, SCI_LINEFROMPOSITION, start, 0);
  start_col = iupScintillaSendMessage(ih, SCI_GETCOLUMN, start, 0);

  end_lin = iupScintillaSendMessage(ih, SCI_LINEFROMPOSITION, end, 0);
  end_col = iupScintillaSendMessage(ih, SCI_GETCOLUMN, end, 0);

  sprintf(str,"%d,%d:%d,%d", start_lin, start_col, end_lin, end_col);

  return str;
}

int iupScintillaSetSelectionAttrib(Ihandle* ih, const char* value)
{
  int lin_start=1, col_start=1, lin_end=1, col_end=1;
  int anchorPos, currentPos;

  if (!value || iupStrEqualNoCase(value, "NONE"))
  {
    iupScintillaSendMessage(ih, SCI_SETEMPTYSELECTION, 0, 0);
    return 0;
  }

  if (iupStrEqualNoCase(value, "ALL"))
  {
    iupScintillaSendMessage(ih, SCI_SELECTALL, 0, 0);
    return 0;
  }

  if (sscanf(value, "%d,%d:%d,%d", &lin_start, &col_start, &lin_end, &col_end)!=4) return 0;
  if (lin_start<1 || col_start<1 || lin_end<1 || col_end<1) return 0;

  anchorPos  = iupScintillaSendMessage(ih, SCI_POSITIONFROMLINE, lin_start, 0) + col_start;
  currentPos = iupScintillaSendMessage(ih, SCI_POSITIONFROMLINE, lin_end, 0)   + col_end;

  iupScintillaSendMessage(ih, SCI_SETSEL, anchorPos, currentPos);

  return 0;
}

char* iupScintillaGetSelectionPosAttrib(Ihandle* ih)
{
  char* str;
  int start = iupScintillaSendMessage(ih, SCI_GETSELECTIONSTART, 0, 0);
  int end   = iupScintillaSendMessage(ih, SCI_GETSELECTIONEND, 0, 0);

  if (start == end)
    return NULL;

  str = iupStrGetMemory(100);

  sprintf(str,"%d:%d", start, end);

  return str;
}

int iupScintillaSetSelectionPosAttrib(Ihandle* ih, const char* value)
{
  int anchorPos = 0, currentPos = 0;

  if (!value || iupStrEqualNoCase(value, "NONE"))
  {
    iupScintillaSendMessage(ih, SCI_SETEMPTYSELECTION, 0, 0);
    return 0;
  }

  if (iupStrEqualNoCase(value, "ALL"))
  {
    iupScintillaSendMessage(ih, SCI_SELECTALL, 0, 0);
    return 0;
  }

  if (iupStrToIntInt(value, &anchorPos, &currentPos, ':') != 2) 
    return 0;

  if(anchorPos<0 || currentPos<0) 
    return 0;

  iupScintillaSendMessage(ih, SCI_SETSEL, anchorPos, currentPos);

  return 0;
}

char* iupScintillaGetVisibleLinesCountAttrib(Ihandle* ih)
{
  char* str = iupStrGetMemory(100);
  int count = iupScintillaSendMessage(ih, SCI_LINESONSCREEN, 0, 0);
  
  sprintf(str, "%d", count);

  return str;
}
