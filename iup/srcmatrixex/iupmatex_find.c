/** \file
 * \brief IupMatrix Expansion Library.
 *
 * See Copyright Notice in "iup.h"
 */

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>

#include "iup.h"
#include "iupcbs.h"

#include "iup_array.h"
#include "iup_object.h"
#include "iup_attrib.h"
#include "iup_str.h"
#include "iup_matrixex.h"


static int iMatrixExSetFindAttrib(Ihandle *ih, const char* value)
{
  return 0;
}

void iupMatrixExRegisterFind(Iclass* ic)
{
  iupClassRegisterAttribute(ic, "FIND", NULL, iMatrixExSetFindAttrib, NULL, NULL, IUPAF_WRITEONLY|IUPAF_NO_INHERIT);
//  iupClassRegisterAttribute(ic, "FINDNEXT", NULL, iMatrixExSetFindNextAttrib, NULL, NULL, IUPAF_WRITEONLY|IUPAF_NO_INHERIT);
//  iupClassRegisterAttribute(ic, "FINDPREVIOUS", NULL, iMatrixExSetFindPreviousAttrib, NULL, NULL, IUPAF_WRITEONLY|IUPAF_NO_INHERIT);

  iupClassRegisterAttribute(ic, "FINDMATCHCASE", NULL, NULL, NULL, NULL, IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "FINDMATCHWHOLECELL", NULL, NULL, NULL, NULL, IUPAF_NO_INHERIT);
}
