/** \file
 * \brief Scintilla control: Scrolling and automatic scrolling
 *
 * See Copyright Notice in "iup.h"
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <Scintilla.h>

#include "iup.h"
#include "iup_scintilla.h"

#include "iup_object.h"
#include "iup_attrib.h"
#include "iup_str.h"

#include "iupsci_scrolling.h"
#include "iupsci.h"

/***** SCROLLING AND AUTOMATIC SCROLLING 
SCI_LINESCROLL(int column, int line)
SCI_SCROLLCARET
--SCI_SETXCARETPOLICY(int caretPolicy, int caretSlop)
--SCI_SETYCARETPOLICY(int caretPolicy, int caretSlop)
--SCI_SETVISIBLEPOLICY(int caretPolicy, int caretSlop)
SCI_SETHSCROLLBAR(bool visible)
SCI_GETHSCROLLBAR
SCI_SETVSCROLLBAR(bool visible)
SCI_GETVSCROLLBAR
--SCI_GETXOFFSET
--SCI_SETXOFFSET(int xOffset)
--SCI_SETSCROLLWIDTH(int pixelWidth)
--SCI_GETSCROLLWIDTH
--SCI_SETSCROLLWIDTHTRACKING(bool tracking)
--SCI_GETSCROLLWIDTHTRACKING
--SCI_SETENDATLASTLINE(bool endAtLastLine)
--SCI_GETENDATLASTLINE
*/

int iupScintillaSetScrollToAttrib(Ihandle *ih, const char *value)
{
  int lin, col;
  iupStrToIntInt(value, &lin, &col, ',');

  iupScintillaSendMessage(ih, SCI_LINESCROLL, col, lin);

  return 0;
}

int iupScintillaSetScrollToPosAttrib(Ihandle *ih, const char *value)
{
  int pos, lin, col;
  iupStrToInt(value, &pos);

  IupScintillaConvertPosToLinCol(ih, pos, &lin, &col);

  iupScintillaSendMessage(ih, SCI_LINESCROLL, col, lin);

  return 0;
}

int iupScintillaSetScrollCaretAttrib(Ihandle *ih, const char *value)
{
  (void)value;
  iupScintillaSendMessage(ih, SCI_SCROLLCARET, 0, 0);
  return 0;
}

char* iupScintillaGetScrollWidthAttrib(Ihandle* ih)
{
  int pixelWidth = iupScintillaSendMessage(ih, SCI_GETSCROLLWIDTH, 0, 0);
  char* str = iupStrGetMemory(15);

  sprintf(str, "%d", pixelWidth);

  return str;
}

int iupScintillaSetScrollWidthAttrib(Ihandle* ih, const char* value)
{
  int pixelWidth;

  iupStrToInt(value, &pixelWidth);
  
  if(pixelWidth < 1)
    pixelWidth = 2000;

  iupScintillaSendMessage(ih, SCI_SETSCROLLWIDTH, pixelWidth, 0);

  return 0;
}

