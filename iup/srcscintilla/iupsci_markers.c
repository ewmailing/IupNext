/** \file
 * \brief Scintilla control: Markers
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

#include "iupsci_markers.h"
#include "iup_scintilla.h"

/***** MARKERS *****
SCI_MARKERDEFINEPIXMAP(int markerNumber, const char *xpm)
SCI_RGBAIMAGESETWIDTH(int width)
SCI_RGBAIMAGESETHEIGHT(int height)
SCI_RGBAIMAGESETSCALE(int scalePercent)
SCI_MARKERDEFINERGBAIMAGE(int markerNumber, const char *pixels)
SCI_MARKERSYMBOLDEFINED(int markerNumber) 
SCI_MARKERSETFORE(int markerNumber, int colour)
SCI_MARKERSETBACK(int markerNumber, int colour)
SCI_MARKERSETBACKSELECTED(int markerNumber, int colour)
SCI_MARKERENABLEHIGHLIGHT(int enabled)
SCI_MARKERSETALPHA(int markerNumber, int alpha)
SCI_MARKERADD(int line, int markerNumber)
SCI_MARKERADDSET(int line, int markerMask)
SCI_MARKERDELETE(int line, int markerNumber)
SCI_MARKERDELETEALL(int markerNumber)
SCI_MARKERGET(int line)
SCI_MARKERNEXT(int lineStart, int markerMask)
SCI_MARKERPREVIOUS(int lineStart, int markerMask)
SCI_MARKERLINEFROMHANDLE(int handle)
SCI_MARKERDELETEHANDLE(int handle)
*/

int iupScintillaSetMarkerDefineAttrib(Ihandle* ih, const char* value)
{
  char *strNumb = iupStrGetMemory(25);
  char *strSymb = iupStrGetMemory(25);
  int markerNumber, markerSymbol;

  iupStrToStrStr(value, strNumb, strSymb, ',');

  /* Setting marker numbers */
  if (iupStrEqualNoCase(strNumb, "FOLDEREND"))
    markerNumber = SC_MARKNUM_FOLDEREND;
  else if (iupStrEqualNoCase(strNumb, "FOLDEROPENMID"))
    markerNumber = SC_MARKNUM_FOLDEROPENMID;
  else if (iupStrEqualNoCase(strNumb, "FOLDERMIDTAIL"))
    markerNumber = SC_MARKNUM_FOLDERMIDTAIL;
  else if (iupStrEqualNoCase(strNumb, "FOLDERTAIL"))
    markerNumber = SC_MARKNUM_FOLDERTAIL;
  else if (iupStrEqualNoCase(strNumb, "FOLDERSUB"))
    markerNumber = SC_MARKNUM_FOLDERSUB;
  else if (iupStrEqualNoCase(strNumb, "FOLDER"))
    markerNumber = SC_MARKNUM_FOLDER;
  else if (iupStrEqualNoCase(strNumb, "FOLDEROPEN"))
    markerNumber = SC_MARKNUM_FOLDEROPEN;
  else
    return 0;

  /* Setting marker symbols */
  if (iupStrEqualNoCase(strSymb, "CIRCLE"))
    markerSymbol = SC_MARK_CIRCLE;
  else if (iupStrEqualNoCase(strSymb, "ROUNDRECT"))
    markerSymbol = SC_MARK_ROUNDRECT;
  else if (iupStrEqualNoCase(strSymb, "ARROW"))
    markerSymbol = SC_MARK_ARROW;
  else if (iupStrEqualNoCase(strSymb, "SMALLRECT"))
    markerSymbol = SC_MARK_SMALLRECT;
  else if (iupStrEqualNoCase(strSymb, "SHORTARROW"))
    markerSymbol = SC_MARK_SHORTARROW;
  else if (iupStrEqualNoCase(strSymb, "EMPTY"))
    markerSymbol = SC_MARK_EMPTY;
  else if (iupStrEqualNoCase(strSymb, "ARROWDOWN"))
    markerSymbol = SC_MARK_ARROWDOWN;
  else if (iupStrEqualNoCase(strSymb, "MINUS"))
    markerSymbol = SC_MARK_MINUS;
  else if (iupStrEqualNoCase(strSymb, "PLUS"))
    markerSymbol = SC_MARK_PLUS;
  else if (iupStrEqualNoCase(strSymb, "VLINE"))
    markerSymbol = SC_MARK_VLINE;
  else if (iupStrEqualNoCase(strSymb, "LCORNER"))
    markerSymbol = SC_MARK_LCORNER;
  else if (iupStrEqualNoCase(strSymb, "TCORNER"))
    markerSymbol = SC_MARK_TCORNER;
  else if (iupStrEqualNoCase(strSymb, "BOXPLUS"))
    markerSymbol = SC_MARK_BOXPLUS;
  else if (iupStrEqualNoCase(strSymb, "BOXPLUSCONNECTED"))
    markerSymbol = SC_MARK_BOXPLUSCONNECTED;
  else if (iupStrEqualNoCase(strSymb, "BOXMINUS"))
    markerSymbol = SC_MARK_BOXMINUS;
  else if (iupStrEqualNoCase(strSymb, "BOXMINUSCONNECTED"))
    markerSymbol = SC_MARK_BOXMINUSCONNECTED;
  else if (iupStrEqualNoCase(strSymb, "LCORNERCURVE"))
    markerSymbol = SC_MARK_LCORNERCURVE;
  else if (iupStrEqualNoCase(strSymb, "TCORNERCURVE"))
    markerSymbol = SC_MARK_TCORNERCURVE;
  else if (iupStrEqualNoCase(strSymb, "CIRCLEPLUS"))
    markerSymbol = SC_MARK_CIRCLEPLUS;
  else if (iupStrEqualNoCase(strSymb, "CIRCLEPLUSCONNECTED"))
    markerSymbol = SC_MARK_CIRCLEPLUSCONNECTED;
  else if (iupStrEqualNoCase(strSymb, "CIRCLEMINUS"))
    markerSymbol = SC_MARK_CIRCLEMINUS;
  else if (iupStrEqualNoCase(strSymb, "CIRCLEMINUSCONNECTED"))
    markerSymbol = SC_MARK_CIRCLEMINUSCONNECTED;
  else if (iupStrEqualNoCase(strSymb, "BACKGROUND"))
    markerSymbol = SC_MARK_BACKGROUND;
  else if (iupStrEqualNoCase(strSymb, "DOTDOTDOT"))
    markerSymbol = SC_MARK_DOTDOTDOT;
  else if (iupStrEqualNoCase(strSymb, "ARROWS"))
    markerSymbol = SC_MARK_ARROWS;
  else if (iupStrEqualNoCase(strSymb, "PIXMAP"))
    markerSymbol = SC_MARK_PIXMAP;
  else if (iupStrEqualNoCase(strSymb, "FULLRECT"))
    markerSymbol = SC_MARK_FULLRECT;
  else if (iupStrEqualNoCase(strSymb, "LEFTRECT"))
    markerSymbol = SC_MARK_LEFTRECT;
  else if (iupStrEqualNoCase(strSymb, "AVAILABLE"))
    markerSymbol = SC_MARK_AVAILABLE;
  else if (iupStrEqualNoCase(strSymb, "UNDERLINE"))
    markerSymbol = SC_MARK_UNDERLINE;
  else if (iupStrEqualNoCase(strSymb, "RGBAIMAGE"))
    markerSymbol = SC_MARK_RGBAIMAGE;
  else if (iupStrEqualNoCase(strSymb, "CHARACTER"))
    markerSymbol = SC_MARK_CHARACTER;
  else
    return 0;

  IUP_SSM(ih->handle, SCI_MARKERDEFINE, markerNumber, markerSymbol);

  return 0;
}
