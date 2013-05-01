/** \file
 * \brief Scintilla control: White space
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

#include "iupsci_whitespace.h"
#include "iupsci.h"


/***** WHITE SPACE *****
SCI_SETVIEWWS(int wsMode)
SCI_GETVIEWWS
SCI_SETWHITESPACEFORE(bool useWhitespaceForeColour, int colour)
SCI_SETWHITESPACEBACK(bool useWhitespaceBackColour, int colour)
SCI_SETWHITESPACESIZE(int size)
SCI_GETWHITESPACESIZE
SCI_SETEXTRAASCENT(int extraAscent)
SCI_GETEXTRAASCENT
SCI_SETEXTRADESCENT(int extraDescent)
SCI_GETEXTRADESCENT
*/

int iupScintillaSetViewWSAttrib(Ihandle *ih, const char *value)
{
  if (iupStrEqualNoCase(value, "INVISIBLE"))
    iupScintillaSendMessage(ih, SCI_SETVIEWWS, SCWS_INVISIBLE, 0);
  if (iupStrEqualNoCase(value, "VISIBLEALWAYS"))
    iupScintillaSendMessage(ih, SCI_SETVIEWWS, SCWS_VISIBLEALWAYS, 0);
  else if (iupStrEqualNoCase(value, "VISIBLEAFTERINDENT"))
    iupScintillaSendMessage(ih, SCI_SETVIEWWS, SCWS_VISIBLEAFTERINDENT, 0);

  return 0;
}

char* iupScintillaGetViewWSAttrib(Ihandle* ih)
{
  if (iupScintillaSendMessage(ih, SCI_GETVIEWWS, 0, 0) == SCWS_INVISIBLE)
    return "INVISIBLE";
  else if (iupScintillaSendMessage(ih, SCI_GETVIEWWS, 0, 0) == SCWS_VISIBLEALWAYS)
    return "VISIBLEALWAYS";
  else if (iupScintillaSendMessage(ih, SCI_GETVIEWWS, 0, 0) == SCWS_VISIBLEAFTERINDENT)
    return "VISIBLEAFTERINDENT";

  return "UNDEFINED";
}

int iupScintillaSetWSFgColorAttrib(Ihandle *ih, const char *value)
{
  unsigned char r, g, b;
  if (!iupStrToRGB(value, &r, &g, &b))
    return 0;

  iupScintillaSendMessage(ih, SCI_SETWHITESPACEFORE, ih->data->useWSForeColour, iupScintillaEncodeColor(r, g, b));

  return 0;
}

int iupScintillaSetWSBgColorAttrib(Ihandle *ih, const char *value)
{
  unsigned char r, g, b;
  if (!iupStrToRGB(value, &r, &g, &b))
    return 0;

  iupScintillaSendMessage(ih, SCI_SETWHITESPACEBACK, ih->data->useWSForeColour, iupScintillaEncodeColor(r, g, b));

  return 0;
}

int iupScintillaSetWSSizeAttrib(Ihandle *ih, const char *value)
{
  int size;
  if (!iupStrToInt(value, &size))
    return 0;

  iupScintillaSendMessage(ih, SCI_SETWHITESPACESIZE, size, 0);

  return 0;
}

char* iupScintillaGetWSSizeAttrib(Ihandle* ih)
{
  int size = iupScintillaSendMessage(ih, SCI_GETWHITESPACESIZE, 0, 0);
  char* str = iupStrGetMemory(15);

  sprintf(str, "%d", size);

  return str;
}

int iupScintillaSetWSExtraAscentAttrib(Ihandle *ih, const char *value)
{
  int asc;
  if (!iupStrToInt(value, &asc))
    return 0;

  iupScintillaSendMessage(ih, SCI_SETEXTRAASCENT, asc, 0);

  return 0;
}

char* iupScintillaGetWSExtraAscentAttrib(Ihandle* ih)
{
  int asc = iupScintillaSendMessage(ih, SCI_GETEXTRAASCENT, 0, 0);
  char* str = iupStrGetMemory(15);

  sprintf(str, "%d", asc);

  return str;
}

int iupScintillaSetWSExtraDescentAttrib(Ihandle *ih, const char *value)
{
  int desc;
  if (!iupStrToInt(value, &desc))
    return 0;

  iupScintillaSendMessage(ih, SCI_SETEXTRADESCENT, desc, 0);

  return 0;
}

char* iupScintillaGetWSExtraDescentAttrib(Ihandle* ih)
{
  int desc = iupScintillaSendMessage(ih, SCI_GETEXTRADESCENT, 0, 0);
  char* str = iupStrGetMemory(15);

  sprintf(str, "%d", desc);

  return str;
}

int iupScintillaSetWSUseFgColorAttrib(Ihandle *ih, const char *value)
{
  if (iupStrBoolean(value))
    ih->data->useWSForeColour = 1;
  else
    ih->data->useWSForeColour = 0;
  return 0;
}

char* iupScintillaGetWSUseFgColorAttrib(Ihandle* ih)
{
  if (ih->data->useWSForeColour)
    return "YES";
  else
    return "NO";
}

int iupScintillaSetWSUseBgColorAttrib(Ihandle *ih, const char *value)
{
  if (iupStrBoolean(value))
    ih->data->useWSBackColour = 1;
  else
    ih->data->useWSBackColour = 0;
  return 0;
}

char* iupScintillaGetWSUseBgColorAttrib(Ihandle* ih)
{
  if (ih->data->useWSBackColour)
    return "YES";
  else
    return "NO";
}
