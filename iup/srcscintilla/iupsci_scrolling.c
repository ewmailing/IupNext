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

/***** SCROLLING AND AUTOMATIC SCROLLING *****/

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


