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
#include <SciLexer.h>

#ifdef GTK
#include <gtk/gtk.h>
#include <ScintillaWidget.h>
#else
#include <windows.h>
#endif

#include "iup.h"
#include "iup_scintilla.h"

#include "iup_object.h"
#include "iup_attrib.h"
#include "iup_str.h"
#include "iup_stdcontrols.h"

#include "iupsci_scrolling.h"
#include "iup_scintilla.h"

/***** SCROLLING AND AUTOMATIC SCROLLING *****/

int iupScintillaSetScrollToAttrib(Ihandle *ih, const char *value)
{
  int lin, col;
  iupStrToIntInt(value, &lin, &col, ',');

  IUP_SSM(ih->handle, SCI_LINESCROLL, col, lin);

  return 0;
}

int iupScintillaSetScrollToPosAttrib(Ihandle *ih, const char *value)
{
  int pos, lin, col;
  iupStrToInt(value, &pos);

  IupScintillaConvertPosToLinCol(ih, pos, &lin, &col);

  IUP_SSM(ih->handle, SCI_LINESCROLL, col, lin);

  return 0;
}


