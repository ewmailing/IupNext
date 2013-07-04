/** \file
 * \brief IupMatrix Expansion Library.
 *
 * See Copyright Notice in "iup.h"
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "iup.h"
#include "iupcbs.h"
#include "iupcontrols.h"
#include "iupmatrixex.h"

#include "iup_object.h"
#include "iup_childtree.h"
#include "iup_register.h"
#include "iup_attrib.h"
#include "iup_str.h"
#include "iup_assert.h"
#include "iup_matrixex.h"


int iupMatrixExIsColumnVisible(Ihandle* ih, int col)
{
  int width = 0;
  char* value;

  if (col==0)
    return (IupGetIntId(ih, "RASTERWIDTH", 0) != 0);

  /* to be invisible must exist the attribute and must be set to 0 (zero), 
     or else is visible */

  value = iupAttribGetId(ih, "WIDTH", col);
  if (!value)
  {
    value = iupAttribGetId(ih, "RASTERWIDTH", col);
    if (!value)
      return 1;
  }

  if (iupStrToInt(value, &width)==1)
  {
    if (width==0)
      return 0;
  }

  return 1;
}

int iupMatrixExIsLineVisible(Ihandle* ih, int lin)
{
  int height = 0;
  char* value;

  if (lin==0)
    return (IupGetIntId(ih, "RASTERHEIGHT", 0) != 0);

  value = iupAttribGetId(ih, "HEIGHT", lin);
  if(!value)
  {
    value = iupAttribGet(ih, "RASTERHEIGHT", lin);
    if(!value)
      return 1;
  }

  if (iupStrToInt(value, &height)==1)
  {
    if (height==0)
      return 0;
  }

  return 1;
}

static char* iMatrixGetVisibleColAttribId(Ihandle *ih, int col)
{
  if (iupMatrixExIsColumnVisible(ih, col))
    return "Yes";
  else
    return "No";
}

static char* iMatrixGetVisibleLinAttribId(Ihandle *ih, int lin)
{
  if (iupMatrixExIsLineVisible(ih, lin))
    return "Yes";
  else
    return "No";
}

static int iMatrixSetVisibleColAttribId(Ihandle *ih, int col, const char* value)
{
  if (iupStrBoolean(value))
    iupAttribSetStrId(ih, "WIDTH", col, "0");    /* this is enough */
  else
  {
    iupAttribSetStrId(ih, "WIDTH", col, NULL);  /* this may be insufficient */
    iupAttribSetStrId(ih, "RASTERWIDTH", col, NULL);
  }
  return 0;
}

static int iMatrixSetVisibleLinAttribId(Ihandle *ih, int lin, const char* value)
{
  if (iupStrBoolean(value))
    iupAttribSetStrId(ih, "HEIGHT", lin, "0");    /* this is enough */
  else
  {
    iupAttribSetStrId(ih, "HEIGHT", lin, NULL);  /* this may be insufficient */
    iupAttribSetStrId(ih, "RASTERHEIGHT", lin, NULL);
  }
  return 0;
}

void iupMatrixExRegisterVisible(Iclass* ic)
{
  iupClassRegisterAttributeId(ic, "VISIBLECOL", iMatrixGetVisibleColAttribId, iMatrixSetVisibleColAttribId, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "VISIBLELIN", iMatrixGetVisibleLinAttribId, iMatrixSetVisibleLinAttribId, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
}

