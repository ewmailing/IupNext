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

#include "iup.h"

#include "iup_object.h"
#include "iup_attrib.h"
#include "iup_str.h"

#include "iupsci_markers.h"
#include "iupsci.h"

/***** MARKERS *****
SCI_MARKERDEFINE(int markerNumber, int markerSymbols)
--SCI_MARKERDEFINEPIXMAP(int markerNumber, const char *xpm)
--SCI_RGBAIMAGESETWIDTH(int width)
--SCI_RGBAIMAGESETHEIGHT(int height)
--SCI_RGBAIMAGESETSCALE(int scalePercent)
--SCI_MARKERDEFINERGBAIMAGE(int markerNumber, const char *pixels)
SCI_MARKERSYMBOLDEFINED(int markerNumber) 
--SCI_MARKERSETFORE(int markerNumber, int colour)
--SCI_MARKERSETBACK(int markerNumber, int colour)
--SCI_MARKERSETBACKSELECTED(int markerNumber, int colour)
--SCI_MARKERENABLEHIGHLIGHT(int enabled)
--SCI_MARKERSETALPHA(int markerNumber, int alpha)
--SCI_MARKERADD(int line, int markerNumber)
--SCI_MARKERADDSET(int line, int markerMask)
--SCI_MARKERDELETE(int line, int markerNumber)
--SCI_MARKERDELETEALL(int markerNumber)
--SCI_MARKERGET(int line)
--SCI_MARKERNEXT(int lineStart, int markerMask)
--SCI_MARKERPREVIOUS(int lineStart, int markerMask)
--SCI_MARKERLINEFROMHANDLE(int handle)
--SCI_MARKERDELETEHANDLE(int handle)
*/

int iupScintillaSetMarkerSymbolAttribId(Ihandle* ih, int markerNumber, const char* value)
{
  int markerSymbol;

  /* Setting marker symbols */
  if (iupStrEqualNoCase(value, "CIRCLE"))
    markerSymbol = SC_MARK_CIRCLE;
  else if (iupStrEqualNoCase(value, "ROUNDRECT"))
    markerSymbol = SC_MARK_ROUNDRECT;
  else if (iupStrEqualNoCase(value, "ARROW"))
    markerSymbol = SC_MARK_ARROW;
  else if (iupStrEqualNoCase(value, "SMALLRECT"))
    markerSymbol = SC_MARK_SMALLRECT;
  else if (iupStrEqualNoCase(value, "SHORTARROW"))
    markerSymbol = SC_MARK_SHORTARROW;
  else if (iupStrEqualNoCase(value, "EMPTY"))    /* invisible */
    markerSymbol = SC_MARK_EMPTY;
  else if (iupStrEqualNoCase(value, "ARROWDOWN"))
    markerSymbol = SC_MARK_ARROWDOWN;
  else if (iupStrEqualNoCase(value, "MINUS"))
    markerSymbol = SC_MARK_MINUS;
  else if (iupStrEqualNoCase(value, "PLUS"))
    markerSymbol = SC_MARK_PLUS;
  else if (iupStrEqualNoCase(value, "VLINE"))
    markerSymbol = SC_MARK_VLINE;
  else if (iupStrEqualNoCase(value, "LCORNER"))
    markerSymbol = SC_MARK_LCORNER;
  else if (iupStrEqualNoCase(value, "TCORNER"))
    markerSymbol = SC_MARK_TCORNER;
  else if (iupStrEqualNoCase(value, "BOXPLUS"))
    markerSymbol = SC_MARK_BOXPLUS;
  else if (iupStrEqualNoCase(value, "BOXPLUSCONNECTED"))
    markerSymbol = SC_MARK_BOXPLUSCONNECTED;
  else if (iupStrEqualNoCase(value, "BOXMINUS"))
    markerSymbol = SC_MARK_BOXMINUS;
  else if (iupStrEqualNoCase(value, "BOXMINUSCONNECTED"))
    markerSymbol = SC_MARK_BOXMINUSCONNECTED;
  else if (iupStrEqualNoCase(value, "LCORNERCURVE"))
    markerSymbol = SC_MARK_LCORNERCURVE;
  else if (iupStrEqualNoCase(value, "TCORNERCURVE"))
    markerSymbol = SC_MARK_TCORNERCURVE;
  else if (iupStrEqualNoCase(value, "CIRCLEPLUS"))
    markerSymbol = SC_MARK_CIRCLEPLUS;
  else if (iupStrEqualNoCase(value, "CIRCLEPLUSCONNECTED"))
    markerSymbol = SC_MARK_CIRCLEPLUSCONNECTED;
  else if (iupStrEqualNoCase(value, "CIRCLEMINUS"))
    markerSymbol = SC_MARK_CIRCLEMINUS;
  else if (iupStrEqualNoCase(value, "CIRCLEMINUSCONNECTED"))
    markerSymbol = SC_MARK_CIRCLEMINUSCONNECTED;
  else if (iupStrEqualNoCase(value, "BACKGROUND"))   /* line background color */
    markerSymbol = SC_MARK_BACKGROUND;
  else if (iupStrEqualNoCase(value, "DOTDOTDOT"))
    markerSymbol = SC_MARK_DOTDOTDOT;
  else if (iupStrEqualNoCase(value, "ARROWS"))
    markerSymbol = SC_MARK_ARROWS;
/*  else if (iupStrEqualNoCase(value, "PIXMAP"))
    markerSymbol = SC_MARK_PIXMAP; */
  else if (iupStrEqualNoCase(value, "FULLRECT"))   /* margin background color */
    markerSymbol = SC_MARK_FULLRECT;
  else if (iupStrEqualNoCase(value, "LEFTRECT"))
    markerSymbol = SC_MARK_LEFTRECT;
/*  else if (iupStrEqualNoCase(value, "AVAILABLE"))
    markerSymbol = SC_MARK_AVAILABLE; */
  else if (iupStrEqualNoCase(value, "UNDERLINE"))  /* underline across the line. */
    markerSymbol = SC_MARK_UNDERLINE;
  else if (iupStrEqualNoCase(value, "RGBAIMAGE"))
    markerSymbol = SC_MARK_RGBAIMAGE;
  else if (iupStrEqualPartial(value, "CHARACTER"))
  {
    int c = (int)value[9];
    markerSymbol = SC_MARK_CHARACTER+c;
  }
  else
    return 0;

  iupScintillaSendMessage(ih, SCI_MARKERDEFINE, markerNumber, markerSymbol);
  return 0;
}

char* iupScintillaGetMarkerSymbolAttribId(Ihandle* ih, int markerNumber)
{
  char* markerSymbolStr[32] = {
    "CIRCLE", "ROUNDRECT", "ARROW", "SMALLRECT", "SHORTARROW", "EMPTY", "ARROWDOWN", "MINUS", 
    "PLUS", "VLINE", "LCORNER", "TCORNER", "BOXPLUS", "BOXPLUSCONNECTED", "BOXMINUS", 
    "BOXMINUSCONNECTED", "LCORNERCURVE", "TCORNERCURVE", "CIRCLEPLUS", "CIRCLEPLUSCONNECTED", 
    "CIRCLEMINUS", "CIRCLEMINUSCONNECTED", "BACKGROUND", "DOTDOTDOT", "ARROWS", "PIXMAP", 
    "FULLRECT", "LEFTRECT", "AVAILABLE", "UNDERLINE", "RGBAIMAGE", "CHARACTER"};

  int markerSymbol = iupScintillaSendMessage(ih, SCI_MARKERSYMBOLDEFINED, markerNumber, 0);
    
  if (markerSymbol>SC_MARK_CHARACTER)
  {
    char* str = iupStrGetMemory(15);
    int c = markerSymbol-SC_MARK_CHARACTER;
    sprintf(str, "%s%c", markerSymbolStr[31], (char)c);
    return str;
  }

  if (markerSymbol<0 || markerSymbol>31)
    return NULL;

  return markerSymbolStr[markerSymbol];
}

int iupScintillaSetMarkerDefineAttrib(Ihandle* ih, const char* value)
{
  char strNumb[25];
  char strSymb[25];
  int markerNumber;

  iupStrToStrStr(value, strNumb, strSymb, '=');

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
  {
    if (!iupStrToInt(strNumb, &markerNumber))
      return 0;

    if (markerNumber<0 || markerNumber>31)
      return 0;
  }

  iupScintillaSetMarkerSymbolAttribId(ih, markerNumber, strSymb);
  return 0;
}
