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


static void iMatrixExCopyColToSetDataSelected(ImatExData* matex_data, int lin, int col, int num_lin, const char* selection, int selection_count, const char* busyname)
{
  int skip_lin = lin;
  char* value = iupStrDup(iupMatrixExGetCell(matex_data, lin, col));

  iupMatrixExInitCellAccess(matex_data);

//  iupMatrixExUndoPushCellBegin(matex_data);

  iupMatrixExBusyStart(matex_data, selection_count, busyname);

  for(lin = 1; lin <= num_lin; ++lin)
  {
    if (lin != skip_lin && selection[lin]=='1' && iupMatrixExIsLineVisible(matex_data->ih, lin))
    {
      //iupMatrixExUndoPushCell(matex_data, lin, col);

      iupMatrixExSetCell(matex_data, lin, col, value);

      if (!iupMatrixExBusyInc(matex_data))
      {
        free(value);
        return;
      }
    }
  }

//  iupMatrixExUndoPushCellEnd(matex_data);

  iupMatrixExBusyEnd(matex_data);

  free(value);
}

static void iMatrixExCopyColToSetData(ImatExData* matex_data, int lin, int col, int lin1, int lin2, const char* busyname)
{
  int skip_lin = lin;
  char* value = iupStrDup(iupMatrixExGetCell(matex_data, lin, col));

  iupMatrixExInitCellAccess(matex_data);

//  iupMatrixExUndoPushBlock(matex_data, lin1, col, lin2-lin1+1, 1);

  iupMatrixExBusyStart(matex_data, lin2-lin1+1, busyname);

  for(lin = lin1; lin <= lin2; ++lin)
  {
    if (lin != skip_lin && iupMatrixExIsLineVisible(matex_data->ih, lin))
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

static int iMatrixExGetMarkedLines(Ihandle *ih, int num_lin, int num_col, int col, char* selection, int *selection_count)
{
  char *marked = IupGetAttribute(ih, "MARKED");
  if (!marked)  /* no marked cells */
    return 0;

  selection[0] = '0';
  *selection_count = 0;

  if (*marked == 'C')
    return 0;
  else if (*marked == 'L')
  {
    int lin;

    marked++;

    for (lin=1; lin<=num_lin; lin++)
    {
      if (iupMatrixExIsLineVisible(ih, lin))
      {
        if (marked[lin-1] == '1')
        {
          selection[lin] = '1';
          (*selection_count)++;
        }
        else
          selection[lin] = '0';
      }
      else
        selection[lin] = '0';
    }
  }
  else
  {
    int lin;

    for(lin = 1; lin <= num_lin; ++lin)
    {
      if (iupMatrixExIsLineVisible(ih, lin))
      {
        int pos = (lin-1) * num_col + (col-1);  /* marked array does not include titles */
        if (marked[pos] == '1')
        {
          selection[lin] = '1';
          (*selection_count)++;
        }
        else
          selection[lin] = '0';
      }
      else
        selection[lin] = '0';
    }
  }

  return 1;
}

static int iMatrixExGetInterval(Ihandle *ih, int num_lin, int num_col, const char* interval, char* selection, int *selection_count)
{
  int lin1, lin2;
  char value[100];

  memset(selection, '0', num_lin+1);
  *selection_count = 0;

//  for (value=strtok((char*)interval,","); value!=NULL; value=strtok(NULL,","))
  {
          const char* next_value = iupStrNextValue(interval, len, &value_len, ',');

          value = iMatrixExStrCopyValue(value, &value_max_size, data, value_len);

          interval = next_value;

    int ret = iupStrToIntInt(value, &lin1, &lin2, '-');
    if (ret == 1)
    {
      if (lin1<=0) lin1 = 1;
      if (lin1>num_lin) lin1 = num_lin;

      selection[lin1] = '1';
      (*selection_count)++;
    }
    else if (ret == 2)
    {
      int lin;

      if (lin1>lin2) {int l=lin1; lin1=lin2; lin2=l;}
      if (lin1<=0) lin1 = 1;
      if (lin2<=0) {lin2 = 1; lin1 = 1;}
      if (lin1>num_lin) lin1 = num_lin;
      if (lin2>num_lin) lin2 = num_lin;

      for(lin = lin1; lin <= lin2; ++lin)
      {
        selection[lin] = '1';
        (*selection_count)++;
      }
    }
    else
      return 0;
  }

  return 1;
}

static int iMatrixExSetCopyColToAttribId2(Ihandle *ih, int lin, int col, const char* value)
{
  ImatExData* matex_data = (ImatExData*)iupAttribGet(ih, "_IUP_MATEX_DATA");
  int lin1, lin2;
  int num_lin, num_col;
  char* busyname;

  num_lin = IupGetInt(ih, "NUMLIN");
  num_col = IupGetInt(ih, "NUMCOL");

  if (col <= 0 ||
      col > num_col ||
      iupMatrixExIsColumnVisible(ih, col))
    return 0;

  if (iupStrEqualNoCase(value, "ALL"))
  {
    busyname = "COPYTOALL";
    lin1 = 1;
    lin2 = num_lin;
  }
  else if (iupStrEqualNoCase(value, "TOP"))
  {
    busyname = "COPYTOTOP";
    lin1 = 1;
    lin2 = lin-1;
  }
  else if (iupStrEqualNoCase(value, "BOTTOM"))
  {
    busyname = "COPYTOBOTTOM";
    lin1 = lin+1;
    lin2 = num_lin;
  }
  else if (iupStrEqualNoCase(value, "MARKED"))
  {
    int selection_count;
    char* selection = (char*)malloc(num_lin+1);

    iupAttribSetStr(ih, "LASTERROR", NULL);

    if (!iMatrixExGetMarkedLines(ih, num_lin, num_col, col, selection, &selection_count))
    {
      iupAttribSetStr(ih, "LASTERROR", "NOMARKED");
      free(selection);
      return 0;
    }

    iMatrixExCopyColToSetDataSelected(matex_data, lin, col, num_lin, selection, selection_count, "COPYTOMARKED");

    free(selection);
    return 0;
  }
  else
  {
    int selection_count;
    char* selection = (char*)malloc(num_lin+1);

    iupAttribSetStr(ih, "LASTERROR", NULL);

    if (!iMatrixExGetInterval(ih, num_lin, num_col, value, selection, &selection_count))
    {
      iupAttribSetStr(ih, "LASTERROR", "INVALIDINTERVAL");
      free(selection);
      return 0;
    }

    iMatrixExCopyColToSetDataSelected(matex_data, lin, col, num_lin, selection, selection_count, "COPYTOINTERVAL");

    free(selection);
    return 0;
  }

  if (lin1>lin2) {int l=lin1; lin1=lin2; lin2=l;}
  if (lin1<=0) lin1 = 1;
  if (lin2<=0) {lin2 = 1; lin1 = 1;}
  if (lin1>num_lin) lin1 = num_lin;
  if (lin2>num_lin) lin2 = num_lin;

  iMatrixExCopyColToSetData(matex_data, lin, col, lin1, lin2, busyname);
  return 0;
}

void iupMatrixExRegisterCopy(Iclass* ic)
{
  iupClassRegisterAttributeId2(ic, "COPYCOLTO", NULL, iMatrixExSetCopyColToAttribId2, IUPAF_NO_INHERIT);
}
