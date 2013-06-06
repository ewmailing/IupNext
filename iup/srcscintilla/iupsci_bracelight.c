/** \file
 * \brief Scintilla control: Brace highlighting
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

#include "iupsci_bracelight.h"
#include "iupsci.h"


/***** BRACE HIGHLIGHTING ****
SCI_BRACEHIGHLIGHT(int pos1, int pos2)
SCI_BRACEBADLIGHT(int pos1)
SCI_BRACEHIGHLIGHTINDICATOR(bool useBraceHighlightIndicator, int indicatorNumber)
SCI_BRACEBADLIGHTINDICATOR(bool useBraceBadLightIndicator, int indicatorNumber)
SCI_BRACEMATCH(int position, int maxReStyle)
*/

int iupScintillaSetBraceHighlightAttrib(Ihandle* ih, const char* value)
{
  int pos1, pos2;

  if (iupStrToIntInt(value, &pos1, &pos2, ':') != 2) 
    return 0;

  iupScintillaSendMessage(ih, SCI_BRACEHIGHLIGHT, pos1, pos2);

  return 0;
}

int iupScintillaSetBraceBadlightAttrib(Ihandle* ih, const char* value)
{
  int pos1;

  if (!iupStrToInt(value, &pos1))
    return 0;

  iupScintillaSendMessage(ih, SCI_BRACEBADLIGHT, pos1, 0);

  return 0;
}

int iupScintillaSetBraceHighlightIndicatorAttrib(Ihandle* ih, const char* value)
{
  int indicatorNumber;

  if (!iupStrToInt(value, &indicatorNumber))
    return 0;

  iupScintillaSendMessage(ih, SCI_BRACEHIGHLIGHTINDICATOR, ih->data->useBraceHLIndicator, indicatorNumber);

  return 0;
}

int iupScintillaSetBraceBadlightIndicatorAttrib(Ihandle* ih, const char* value)
{
  int indicatorNumber;

  if (!iupStrToInt(value, &indicatorNumber))
    return 0;

  iupScintillaSendMessage(ih, SCI_BRACEBADLIGHTINDICATOR, ih->data->useBraceBLIndicator, indicatorNumber);

  return 0;
}

int iupScintillaSetUseBraceHLIndicatorAttrib(Ihandle *ih, const char *value)
{
  if (iupStrBoolean(value))
    ih->data->useBraceHLIndicator = 1;
  else
    ih->data->useBraceHLIndicator = 0;
  return 0;
}

char* iupScintillaGetUseBraceHLIndicatorAttrib(Ihandle* ih)
{
  if (ih->data->useBraceHLIndicator)
    return "YES";
  else
    return "NO";
}

int iupScintillaSetUseBraceBLIndicatorAttrib(Ihandle *ih, const char *value)
{
  if (iupStrBoolean(value))
    ih->data->useBraceBLIndicator = 1;
  else
    ih->data->useBraceBLIndicator = 0;
  return 0;
}

char* iupScintillaGetUseBraceBLIndicatorAttrib(Ihandle* ih)
{
  if (ih->data->useBraceBLIndicator)
    return "YES";
  else
    return "NO";
}

char* iupScintillaGetBraceMatchAttribId(Ihandle* ih, int pos)
{
  int value;
  char* str = iupStrGetMemory(15);

  if (pos < 0)
    return "-1";

  value = iupScintillaSendMessage(ih, SCI_BRACEMATCH, pos, 0);

  sprintf(str, "%d", value);

  return str;
}
