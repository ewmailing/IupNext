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


typedef struct _IundoData {
  int cell_count;
  const char* name;
  Itable* data_table;
} IundoData;

static void iMatrixExUndoDataInit(IundoData* undo_data, const char* name)
{
  undo_data->cell_count = 0;
  undo_data->name = name;
  undo_data->data_table = iupTableCreate(IUPTABLE_STRINGINDEXED);
}

static void iMatrixExUndoDataClear(IundoData* undo_data)
{
  iupTableDestroy(undo_data->data_table);
  memset(undo_data, 0, sizeof(IundoData));
}

static void iMatrixExUndoDataAddCell(IundoData* undo_data, int lin, int col, const char* value)
{
  char id[40];
  sprintf(id, "%d:%d", lin, col);
  iupTableSet(undo_data->data_table, id, (void*)value, IUPTABLE_STRING);
  undo_data->cell_count++;
}

static int iMatrixExUndoDataSwap(ImatExData* matex_data, IundoData* undo_data)
{
  char* id = iupTableFirst(undo_data->data_table);
  while (id)
  {
    char* value;
    int lin=1, col=1;
    iupStrToIntInt(id, &lin, &col, ':');

    value = (char*)iupTableGetCurr(undo_data->data_table);

    iupMatrixExSetCellValue(matex_data->ih, lin, col, value, 0);

    if (!iupMatrixExBusyInc(matex_data))
      return 0;

    id = iupTableNext(undo_data->data_table);
  }

  return 1;
}

static void iMatrixExUndoStackInit(ImatExData* matex_data)
{
  if (!matex_data->undo_stack) 
    matex_data->undo_stack = iupArrayCreate(40, sizeof(IundoData));
}

static void iMatrixUndoStackAdd(ImatExData* matex_data, const char* name)
{
  int i, undo_stack_count = iupArrayCount(matex_data->undo_stack);
  IundoData* undo_stack_data = (IundoData*)iupArrayGetData(matex_data->undo_stack);

  /* Remove all Redo data */
  for (i=matex_data->undo_stack_pos; i<undo_stack_count; i++)
    iMatrixExUndoDataClear(&(undo_stack_data[i]));

  iMatrixExUndoDataInit(&(undo_stack_data[matex_data->undo_stack_pos]), name);
}

void iupMatrixExUndoPushBegin(ImatExData* matex_data, const char* name)
{
  if (!matex_data->undo_stack_hold)
  {
    iMatrixUndoStackAdd(matex_data, name);
    matex_data->undo_stack_hold = 1;
  }
}

void iupMatrixExUndoPushEnd(ImatExData* matex_data)
{
  if (matex_data->undo_stack_hold)
  {
    matex_data->undo_stack_pos++;
    matex_data->undo_stack_hold = 0;
  }
}

static int iMatrixSetUndoPushCellAttrib(Ihandle* ih, int lin, int col, const char* value)
{
  ImatExData* matex_data = (ImatExData*)iupAttribGet(ih, "_IUP_MATEX_DATA");
  IundoData* undo_stack_data = (IundoData*)iupArrayGetData(matex_data->undo_stack);

  if (matex_data->undo_stack_hold)
    iMatrixExUndoDataAddCell(&(undo_stack_data[matex_data->undo_stack_pos]), lin, col, value);
  else
  {
    iMatrixUndoStackAdd(matex_data, "SETCELL");
    iMatrixExUndoDataAddCell(&(undo_stack_data[matex_data->undo_stack_pos]), lin, col, value);
    matex_data->undo_stack_pos++;
  }

  return 0;
}

static int iMatrixSetUndoClearAttrib(Ihandle* ih, const char* value)
{
  ImatExData* matex_data = (ImatExData*)iupAttribGet(ih, "_IUP_MATEX_DATA");
  if (matex_data->undo_stack && iupArrayCount(matex_data->undo_stack))
  {
    int i, undo_stack_count = iupArrayCount(matex_data->undo_stack);
    IundoData* undo_stack_data = (IundoData*)iupArrayGetData(matex_data->undo_stack);
    for (i=0; i<undo_stack_count; i++)
      iMatrixExUndoDataClear(&(undo_stack_data[i]));
    iupArrayRemove(matex_data->undo_stack, 0, undo_stack_count);
    matex_data->undo_stack_pos = 0;
  }
  (void)value;
  return 0;
}

static char* iMatrixGetUndoAttrib(Ihandle* ih)
{
  ImatExData* matex_data = (ImatExData*)iupAttribGet(ih, "_IUP_MATEX_DATA");
  if (matex_data->undo_stack && iupArrayCount(matex_data->undo_stack))
    return iupStrReturnBoolean(matex_data->undo_stack_pos>0);
  return NULL; 
}

static int iMatrixSetUndoAttrib(Ihandle* ih, const char* value)
{
  ImatExData* matex_data = (ImatExData*)iupAttribGet(ih, "_IUP_MATEX_DATA");
  if (matex_data->undo_stack && iupArrayCount(matex_data->undo_stack) && matex_data->undo_stack_pos>0)
  {
    int i, count = 1, total_cell_count = 0;
    IundoData* undo_stack_data = (IundoData*)iupArrayGetData(matex_data->undo_stack);

    iupStrToInt(value, &count);
    if (count >= matex_data->undo_stack_pos)
      count = matex_data->undo_stack_pos-1;

    for (i=0; i<count; i++)
      total_cell_count += undo_stack_data[matex_data->undo_stack_pos-1 - i].cell_count;

    iupMatrixExBusyStart(matex_data, total_cell_count, "UNDO");

    for (i=0; i<count; i++)
    {
      if (!iMatrixExUndoDataSwap(matex_data, &(undo_stack_data[matex_data->undo_stack_pos-1 - i])))
        break;
    }

    iupMatrixExBusyEnd(matex_data);

    matex_data->undo_stack_pos -= i;
  }
  return 0;
}

static char* iMatrixGetRedoAttrib(Ihandle* ih)
{
  ImatExData* matex_data = (ImatExData*)iupAttribGet(ih, "_IUP_MATEX_DATA");
  if (matex_data->undo_stack && iupArrayCount(matex_data->undo_stack))
    return iupStrReturnBoolean(matex_data->undo_stack_pos<iupArrayCount(matex_data->undo_stack));
  return NULL; 
}

static int iMatrixSetRedoAttrib(Ihandle* ih, const char* value)
{
  ImatExData* matex_data = (ImatExData*)iupAttribGet(ih, "_IUP_MATEX_DATA");
  if (matex_data->undo_stack && iupArrayCount(matex_data->undo_stack) && matex_data->undo_stack_pos<iupArrayCount(matex_data->undo_stack))
  {
    int i, count = 1, total_cell_count = 0, undo_stack_count = iupArrayCount(matex_data->undo_stack);
    IundoData* undo_stack_data = (IundoData*)iupArrayGetData(matex_data->undo_stack);

    iupStrToInt(value, &count);
    if (count > undo_stack_count - matex_data->undo_stack_pos)
      count = undo_stack_count - matex_data->undo_stack_pos;

    for (i=0; i<count; i++)
      total_cell_count += undo_stack_data[matex_data->undo_stack_pos + i].cell_count;

    iupMatrixExBusyStart(matex_data, total_cell_count, "REDO");

    for (i=0; i<count; i++)
    {
      if (!iMatrixExUndoDataSwap(matex_data, &(undo_stack_data[matex_data->undo_stack_pos + i])))
        break;
    }

    iupMatrixExBusyEnd(matex_data);

    matex_data->undo_stack_pos += i;
  }
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

static IattribSetFunc iMatrixSetUndoRedoAttrib = NULL;

static int iMatrixExSetUndoRedoAttrib(Ihandle* ih, const char* value)
{
  ImatExData* matex_data = (ImatExData*)iupAttribGet(ih, "_IUP_MATEX_DATA");
  if (iupStrBoolean(value))
    iMatrixExUndoStackInit(matex_data);
  else
    iMatrixSetUndoClearAttrib(ih, NULL);
  return iMatrixSetUndoRedoAttrib(ih, value);
}

void iupMatrixExRegisterUndo(Iclass* ic)
{
  /* Already defined in IupMatrix */
  iupClassRegisterGetAttribute(ic, "UNDOREDO", NULL, &iMatrixSetUndoRedoAttrib, NULL, NULL, NULL);
  iupClassRegisterReplaceAttribFunc(ic, "UNDOREDO", NULL, iMatrixExSetUndoRedoAttrib);

  iupClassRegisterAttribute(ic, "UNDO", iMatrixGetUndoAttrib, iMatrixSetUndoAttrib, NULL, NULL, IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "REDO", iMatrixGetRedoAttrib, iMatrixSetRedoAttrib, NULL, NULL, IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "UNDOCLEAR", NULL, iMatrixSetUndoClearAttrib, NULL, NULL, IUPAF_WRITEONLY|IUPAF_NO_INHERIT);

  /* Internal attributes */
  iupClassRegisterAttributeId2(ic, "UNDOPUSHCELL", NULL, iMatrixSetUndoPushCellAttrib, IUPAF_WRITEONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "UNDOPUSHBEGIN", NULL, iMatrixSetUndoPushBeginAttrib, NULL, NULL, IUPAF_WRITEONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "UNDOPUSHEND", NULL, iMatrixSetUndoPushEndAttrib, NULL, NULL, IUPAF_WRITEONLY|IUPAF_NO_INHERIT);
}

