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

static int iMatrixExSetFind(Ihandle *ih, const char* value, int inc, int flip, int matchcase, int matchcell, int matchmarked, int *lin, int *col)
{
  int utf8 = IupGetInt(NULL, "UTF8MODE");
  int num_lin = IupGetInt(ih, "NUMLIN");
  int num_col = IupGetInt(ih, "NUMCOL");
  int count = (num_lin+1)*(num_col+1);
  int pos, start_pos;

  if (inc == 0)
  {
    /* the FOCUS_CELL is always visible and not a title */

    /* only need to check if it is marked */
    if (!matchmarked || iupStrBoolean(iupAttribGetClassObjectId2(ih, "MARK", *lin, *col)))
    {
      if (iMatrixMatch(ih, value, *lin, *col, matchcase, matchcell, utf8))
        return 1;
    }

    inc = 1;
  }

  if (flip)
    pos = (*col)*(num_lin+1) + *lin;
  else
    pos = (*lin)*(num_col+1) + *col;

  start_pos = pos;
  do 
  {
    pos += inc;

    if (pos < 0) pos = count-1;   /* if less than first cell, go to last */
    if (pos > count-1) pos = 0;   /* if more than last cell, go to first */

    if (flip)
    {
      *lin = pos % (num_lin+1);
      *col = pos / (num_lin+1);
    }
    else
    {
      *lin = pos / (num_col+1);
      *col = pos % (num_col+1);
    }

    if (*lin == 0 || *col == 0)  /* dont't search on titles */
      continue;

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
  int inc, flip, matchcase, matchcell, matchmarked;
  char* direction;

  if (!value || value[0]==0)
    return 0;

  IupGetIntInt(ih, "FOCUS_CELL", &lin, &col);

  direction = iupAttribGet(ih, "FINDDIRECTION");
  if (iupStrEqualNoCase(direction, "LEFTTOP"))
  {
    flip = 0;
    inc = -1;
  }
  else if (iupStrEqualNoCase(direction, "TOPLEFT"))
  {
    flip = 1;
    inc = -1;
  }
  else if (iupStrEqualNoCase(direction, "BOTTOMRIGHT"))
  {
    flip = 1;
    inc = +1;
  }
  else  /* RIGHTBOTTOM */
  {
    flip = 0;
    inc = +1;
  }
  matchcase = iupAttribGetInt(ih, "FINDMATCHCASE");
  matchcell = iupAttribGetInt(ih, "FINDMATCHWHOLECELL");
  matchmarked = iupAttribGetInt(ih, "FINDMATCHSELECTION");

  if (inc == 1)
  {
    int last_lin=0, last_col=0;
    IupGetIntInt(ih, "_IUP_LAST_FOUND", &last_lin, &last_col);
    if (last_lin==lin && last_col==col)
      inc = 0;  /* search in the current cell */
  }

  if (iMatrixExSetFind(ih, value, inc, flip, matchcase, matchcell, matchmarked, &lin, &col))
  {
    IupSetfAttribute(ih,"FOCUS_CELL", "%d:%d", lin, col);
//    IupSetfAttribute(ih,"SHOW", "%d:%d", lin, col);
    //if (!matchmarked)
    //{
    //  IupSetAttribute(ih,"MARKED",NULL);
    //  IupSetAttributeId2(ih,"MARK",lin,col,"1");
    //}
    IupSetfAttribute(ih,"_IUP_LAST_FOUND", "%d:%d", lin, col);
  }

  return 1;
}

void iupMatrixExRegisterFind(Iclass* ic)
{
  iupClassRegisterAttribute(ic, "FIND", NULL, iMatrixExSetFindAttrib, NULL, NULL, IUPAF_NO_INHERIT);

  iupClassRegisterAttribute(ic, "FINDDIRECTION", NULL, NULL, IUPAF_SAMEASSYSTEM, "RIGHTBOTTOM", IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "FINDMATCHCASE", NULL, NULL, IUPAF_SAMEASSYSTEM, "Yes", IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "FINDMATCHWHOLECELL", NULL, NULL, IUPAF_SAMEASSYSTEM, "Yes", IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "FINDMATCHSELECTION", NULL, NULL, NULL, NULL, IUPAF_NO_INHERIT);
}
