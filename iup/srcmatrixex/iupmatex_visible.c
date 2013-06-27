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
  char str[100];
  char* value;

  if (col==0)
    return (IupGetIntId(ih, "RASTERWIDTH", 0) != 0);

  /* to be invisible must exist the attribute and must be set to 0 (zero), 
     or else is visible */

  sprintf(str, "WIDTH%d", col);
  value = iupAttribGet(ih, str);
  if (!value)
  {
    sprintf(str, "RASTERWIDTH%d", col);
    value = iupAttribGet(ih, str);
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
  char str[100];
  char* value;

  if (lin==0)
    return (IupGetIntId(ih, "RASTERHEIGHT", 0) != 0);

  sprintf(str, "HEIGHT%d", lin);
  value = iupAttribGet(ih, str);
  if(!value)
  {
    sprintf(str, "RASTERHEIGHT%d", lin);
    value = iupAttribGet(ih, str);
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

static char* iMatrixGetVisibleColAttribId(Ihandle *ih, int id)
{
  if (iupMatrixExIsColumnVisible(ih, id))
    return "Yes";
  else
    return "No";
}

static char* iMatrixGetVisibleLinAttribId(Ihandle *ih, int id)
{
  if (iupMatrixExIsLineVisible(ih, id))
    return "Yes";
  else
    return "No";
}

static int iMatrixSetVisibleColAttribId(Ihandle *ih, int id, const char* value)
{
  char str[100];
  sprintf(str, "WIDTH%d", id);  /* this is enough */
  if (iupStrBoolean(value))
    iupAttribSetStr(ih, str, "0");
  else
  {
    iupAttribSetStr(ih, str, NULL);  /* this may be insufficient */
    sprintf(str, "RASTERWIDTH%d", id);
    iupAttribSetStr(ih, str, NULL);
  }
  return 0;
}

static int iMatrixSetVisibleLinAttribId(Ihandle *ih, int id, const char* value)
{
  char str[100];
  sprintf(str, "HEIGHT%d", id);  /* this is enough */
  if (iupStrBoolean(value))
    iupAttribSetStr(ih, str, "0");
  else
  {
    iupAttribSetStr(ih, str, NULL);  /* this may be insufficient */
    sprintf(str, "RASTERHEIGHT%d", id);
    iupAttribSetStr(ih, str, NULL);
  }
  return 0;
}

void iupMatrixExRegisterVisible(Iclass* ic)
{
  iupClassRegisterAttributeId(ic, "VISIBLECOL", iMatrixGetVisibleColAttribId, iMatrixSetVisibleColAttribId, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "VISIBLELIN", iMatrixGetVisibleLinAttribId, iMatrixSetVisibleLinAttribId, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
}

