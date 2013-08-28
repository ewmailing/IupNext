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


static int iMatrixMatch(Ihandle *ih, const char* findvalue, int lin, int col, int matchcase, int matchcell, int utf8)
{
  char* value = iupMatrixExGetCellValue(ih, lin, col, 1);  /* get displayed value */
  if (!value || value[0] == 0)
    return 0;

  if (matchcell)
    return iupStrCompareEqual(value, findvalue, matchcase, utf8, 0);
  else
    return iupStrCompareFind(value, findvalue, matchcase, utf8);
}

static int iMatrixExSetFind(Ihandle *ih, const char* value, int dir, int matchcase, int matchcell, int matchmarked, int *lin, int *col)
{
  int utf8 = IupGetInt(NULL, "UTF8MODE");
  int num_lin = IupGetInt(ih, "NUMLIN");
  int num_col = IupGetInt(ih, "NUMCOL");
  int count = (num_lin+1)*(num_col+1);
  int pos = (*lin)*(num_col+1) + *col;
  int start_pos = pos;

  if (dir == 0)
  {
    /* the FOCUS_CELL is always visible and not a title */

    /* only need to check if it is marked */
    if (!matchmarked || iupStrBoolean(iupAttribGetClassObjectId2(ih, "MARK", *lin, *col)))
    {
      if (iMatrixMatch(ih, value, *lin, *col, matchcase, matchcell, utf8))
        return 1;
    }

    dir = 1;
  }

  do 
  {
    pos += dir;

    if (pos % (num_col+1) == 0)  /* col=0 */
      pos += dir;

    if (pos < (num_col+1) + 1) pos = count-1;   /* if at first cell, go to last */
    if (pos > count-1) pos = (num_col+1) + 1;   /* if at last cell, go to first */

    *lin = pos / (num_col+1);
    *col = pos % (num_col+1);

    if (pos == start_pos)
      return 0;

    if (!iupMatrixExIsLineVisible(ih, *lin) || !iupMatrixExIsColumnVisible(ih, *col))
      continue;

    if (matchmarked && !iupStrBoolean(iupAttribGetClassObjectId2(ih, "MARK", *lin, *col)))
      continue;

  } while (!iMatrixMatch(ih, value, *lin, *col, matchcase, matchcell, utf8));

  return 1;
}

static int iMatrixExSetFindAttrib(Ihandle *ih, const char* value)
{
  int lin=1, col=1;
  int dir, matchcase, matchcell, matchmarked;

  if (!value || value[0]==0)
    return 0;

  IupGetIntInt(ih, "FOCUS_CELL", &lin, &col);

  if (iupStrEqualNoCase(iupAttribGet(ih, "FINDDIRECTION"), "PREVIOUS"))
    dir = -1;
  else
    dir = 1;
  matchcase = iupAttribGetInt(ih, "FINDMATCHCASE");
  matchcell = iupAttribGetInt(ih, "FINDMATCHWHOLECELL");
  matchmarked = iupAttribGetInt(ih, "FINDMATCHSELECTION");

  if (dir == 1)
  {
    int last_lin=0, last_col=0;
    IupGetIntInt(ih, "_IUP_LAST_FOUND", &last_lin, &last_col);
    if (last_lin==lin && last_col==col)
      dir = 0;  /* search in the current cell */
  }

  if (iMatrixExSetFind(ih, value, dir, matchcase, matchcell, matchmarked, &lin, &col))
  {
    IupSetfAttribute(ih,"FOCUS_CELL", "%d:%d", lin, col);
//    IupSetfAttribute(ih,"SHOW", "%d:%d", lin, col);
//    IupSetAttribute(ih,"MARKED",NULL);
//    IupSetAttributeId2(ih,"MARK",lin,col,"1");
    IupSetfAttribute(ih,"_IUP_LAST_FOUND", "%d:%d", lin, col);
  }

  return 0;
}

void iupMatrixExRegisterFind(Iclass* ic)
{
  iupClassRegisterAttribute(ic, "FIND", NULL, iMatrixExSetFindAttrib, NULL, NULL, IUPAF_WRITEONLY|IUPAF_NO_INHERIT);

  iupClassRegisterAttribute(ic, "FINDDIRECTION", NULL, NULL, IUPAF_SAMEASSYSTEM, "NEXT", IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "FINDMATCHCASE", NULL, NULL, IUPAF_SAMEASSYSTEM, "Yes", IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "FINDMATCHWHOLECELL", NULL, NULL, IUPAF_SAMEASSYSTEM, "Yes", IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "FINDMATCHSELECTION", NULL, NULL, NULL, NULL, IUPAF_NO_INHERIT);
}
