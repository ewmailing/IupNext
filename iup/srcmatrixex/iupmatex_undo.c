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


void iupMatrixExUndoPushBegin(ImatExData* matex_data, const char* busyname)
{
}

void iupMatrixExUndoPushEnd(ImatExData* matex_data)
{
}

static int iMatrixSetUndoClearAttrib(Ihandle* ih, const char* value)
{
  ImatExData* matex_data = (ImatExData*)iupAttribGet(ih, "_IUP_MATEX_DATA");

  return 0;
}

static int iMatrixSetUndoPushCellAttrib(Ihandle* ih, int lin, int col, const char* value)
{
  ImatExData* matex_data = (ImatExData*)iupAttribGet(ih, "_IUP_MATEX_DATA");

  return 0;
}

static int iMatrixSetUndoPushBeginAttrib(Ihandle* ih, const char* value)
{
  ImatExData* matex_data = (ImatExData*)iupAttribGet(ih, "_IUP_MATEX_DATA");
  iupMatrixExUndoPushBegin(matex_data, value);
  return 0;
}

static int iMatrixSetUndoPushEndAttrib(Ihandle* ih, const char* value)
{
  ImatExData* matex_data = (ImatExData*)iupAttribGet(ih, "_IUP_MATEX_DATA");
  iupMatrixExUndoPushEnd(matex_data);
  (void)value;
  return 0;
}

void iupMatrixExRegisterUndo(Iclass* ic)
{
  iupClassRegisterAttribute(ic, "UNDOCLEAR", NULL, iMatrixSetUndoClearAttrib, NULL, NULL, IUPAF_WRITEONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId2(ic, "UNDOPUSHCELL", NULL, iMatrixSetUndoPushCellAttrib, IUPAF_WRITEONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "UNDOPUSHBEGIN", NULL, iMatrixSetUndoPushBeginAttrib, NULL, NULL, IUPAF_WRITEONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "UNDOPUSHEND", NULL, iMatrixSetUndoPushEndAttrib, NULL, NULL, IUPAF_WRITEONLY|IUPAF_NO_INHERIT);
}

