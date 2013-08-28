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


static int iMatrixMatch(const char* value, int lin, int col, int matchcase, int mathcell)
{
  //if (cell.empty())
  // return false;
  //if (!match_case)
  //{
  // std::string::iterator i(cell.begin());
  // for(;i!=cell.end();++i)
  // {
  //  if (isalpha(static_cast<unsigned char>(*i)))
  //   *i = tolower(*i);
  // }
  //}
  //if (whole_word)
  // return (cell.compare(text)==0);
  //else
  // return (cell.find(text)!=std::string::npos);
  return 0;
}

static int iMatrixExSetFind(Ihandle *ih, const char* value, int dir, int matchcase, int mathcell, int *lin, int *col)
{
  int num_lin = IupGetInt(ih, "NUMLIN");
  int num_col = IupGetInt(ih, "NUMCOL");
  int count = (num_lin+1)*(num_col+1);
  int pos = (*lin)*(num_col+1) + *col;
  int start_pos = pos;

  if (dir == 0)
  {
    if (iMatrixMatch(value, *lin, *col, matchcase, mathcell))
      return 1;

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

  } while (!iMatrixMatch(value, *lin, *col, matchcase, mathcell));

  return 1;
}

static int iMatrixExSetFindAttrib(Ihandle *ih, const char* value)
{
  int lin=1, col=1;
  int dir, matchcase, mathcell;

  if (!value || value[0]==0)
    return 0;

  IupGetIntInt(ih, "FOCUS_CELL", &lin, &col);

  if (iupStrEqualNoCase(iupAttribGet(ih, "FINDDIRECTION"), "PREVIOUS"))
    dir = -1;
  else
    dir = 1;
  matchcase = iupAttribGetInt(ih, "FINDMATCHCASE");
  mathcell = iupAttribGetInt(ih, "FINDMATCHWHOLECELL");

  if (dir == 1)
  {
    int last_lin=0, last_col=0;
    IupGetIntInt(ih, "_IUP_LAST_FOUND", &last_lin, &last_col);
    if (last_lin==lin && last_col==col)
      dir = 0;  /* search in the current cell */
  }

  if (iMatrixExSetFind(ih, value, dir, matchcase, mathcell, &lin, &col))
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
  iupClassRegisterAttribute(ic, "FINDMATCHCASE", NULL, NULL, NULL, NULL, IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "FINDMATCHWHOLECELL", NULL, NULL, NULL, NULL, IUPAF_NO_INHERIT);
}
