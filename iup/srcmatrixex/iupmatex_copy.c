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


static void iMatrixExCopyToSetData(ImatExData* matex_data, int lin, int col, int lin1, int lin2, const char* busyname)
{
  char* value = iupStrDup(iupMatrixExGetCell(matex_data, lin, col));

//  iupMatrixExPushUndoBlock(ih, lin1, col, lin2-lin1+1, 1);

  iupMatrixExInitCellAccess(matex_data);

  iupMatrixExBusyStart(matex_data, lin2-lin1+1, busyname);

  for(lin = lin1; lin <= lin2; ++lin)
  {
    if (iupMatrixExIsLineVisible(matex_data->ih, lin))
    {
      iupMatrixExSetCell(matex_data, lin, col, value);

      if (!iupMatrixExBusyInc(matex_data))
      {
        free(value);
        return;
      }
    }
  }

  iupMatrixExBusyEnd(matex_data);

  free(value);
}

static int iMatrixExSetCopyToAttribId2(Ihandle *ih, int lin, int col, const char* value)
{
  ImatExData* matex_data = (ImatExData*)iupAttribGet(ih, "_IUP_MATEX_DATA");
  int lin1, lin2;
  char* busyname;

  if (!iupMatrixExIsColumnVisible(ih, col))
    return 0;


  iMatrixExCopyToSetData(matex_data, lin, col, lin1, lin2, busyname);


  IupSetAttribute(ih,"REDRAW","ALL");
  return 0;
}

void iupMatrixExRegisterCopy(Iclass* ic)
{
  iupClassRegisterAttributeId2(ic, "COPYTO", NULL, iMatrixExSetCopyToAttribId2, IUPAF_NO_INHERIT);
}
