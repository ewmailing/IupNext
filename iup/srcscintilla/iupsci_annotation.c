/** \file
 * \brief Scintilla control: Annotation
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

#include "iupsci_annotation.h"
#include "iupsci.h"


/***** Annotation *****
SCI_ANNOTATIONSETTEXT(int line, char *text)
SCI_ANNOTATIONGETTEXT(int line, char *text)
SCI_ANNOTATIONSETSTYLE(int line, int style)
SCI_ANNOTATIONGETSTYLE(int line)
-- SCI_ANNOTATIONSETSTYLES(int line, char *styles)
-- SCI_ANNOTATIONGETSTYLES(int line, char *styles)
-- SCI_ANNOTATIONGETLINES(int line)
SCI_ANNOTATIONCLEARALL
SCI_ANNOTATIONSETVISIBLE(int visible)
SCI_ANNOTATIONGETVISIBLE
SCI_ANNOTATIONSETSTYLEOFFSET(int style)
SCI_ANNOTATIONGETSTYLEOFFSET
*/

int iupScintillaSetAnnotationTextAttribId(Ihandle* ih, int line, const char* value)
{
  iupScintillaSendMessage(ih, SCI_ANNOTATIONSETTEXT, line, (sptr_t)value);
  return 0;
}

char* iupScintillaGetAnnotationTextAttribId(Ihandle* ih, int line)
{
  int len = iupScintillaSendMessage(ih, SCI_ANNOTATIONGETTEXT, line, 0);
  char* str = iupStrGetMemory(len+1);
  iupScintillaSendMessage(ih, SCI_ANNOTATIONGETTEXT, line, (sptr_t)str);
  return str;
}

int iupScintillaSetAnnotationStyleAttribId(Ihandle* ih, int line, const char* value)
{
  int style;
  iupStrToInt(value, &style);
  iupScintillaSendMessage(ih, SCI_ANNOTATIONSETSTYLE, line, style);
  return 0;
}

char* iupScintillaGetAnnotationStyleAttribId(Ihandle* ih, int line)
{
  int style = iupScintillaSendMessage(ih, SCI_ANNOTATIONGETSTYLE, line, 0);
  return iupStrReturnInt(style);
}

int iupScintillaSetAnnotationStyleOffsetAttrib(Ihandle* ih, const char* value)
{
  int style_offset;
  iupStrToInt(value, &style_offset);
  iupScintillaSendMessage(ih, SCI_ANNOTATIONSETSTYLEOFFSET, style_offset, 0);
  return 0;
}

char* iupScintillaGetAnnotationStyleOffsetAttrib(Ihandle* ih)
{
  int style = iupScintillaSendMessage(ih, SCI_ANNOTATIONGETSTYLEOFFSET, 0, 0);
  return iupStrReturnInt(style);
}

int iupScintillaSetAnnotationVisibleAttrib(Ihandle *ih, const char *value)
{
  if (iupStrEqualNoCase(value, "STANDARD"))
    iupScintillaSendMessage(ih, SCI_ANNOTATIONSETVISIBLE, ANNOTATION_STANDARD, 0);
  else if (iupStrEqualNoCase(value, "BOXED"))
    iupScintillaSendMessage(ih, SCI_ANNOTATIONSETVISIBLE, ANNOTATION_BOXED, 0);
  else  /* "HIDDEN" */
    iupScintillaSendMessage(ih, SCI_ANNOTATIONSETVISIBLE, ANNOTATION_HIDDEN, 0);
  return 0;
}

char* iupScintillaGetAnnotationVisibleAttrib(Ihandle* ih)
{
  if (iupScintillaSendMessage(ih, SCI_ANNOTATIONGETVISIBLE, 0, 0) == ANNOTATION_STANDARD)
    return "STANDARD";
  else if (iupScintillaSendMessage(ih, SCI_ANNOTATIONGETVISIBLE, 0, 0) == ANNOTATION_BOXED)
    return "BOXED";
  else  /* ANNOTATION_HIDDEN */
    return "HIDDEN";
}

int iupScintillaSetAnnotationClearAllAttrib(Ihandle* ih, const char* value)
{
  (void)value;
  iupScintillaSendMessage(ih, SCI_ANNOTATIONCLEARALL, 0, 0);
  return 0;
}

