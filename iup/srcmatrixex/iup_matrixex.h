/** \file
 * \brief IupMatrix Expansion Library.
 *
 * See Copyright Notice in "iup.h"
 */
 
#ifndef __IUP_MATRIXEX_H 
#define __IUP_MATRIXEX_H 

#include"iup_array.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef struct _ImatExData
{
  Ihandle* ih;  /* self reference */

  int busy, busy_count, busy_undo_block,
      busy_progress_abort;
  IFniis busy_cb;
  Ihandle* busy_progress_dlg;

  Ihandle* find_dlg;

  Iarray* undo_stack;
  int undo_stack_pos;   /* points to the position where a new undo will be inserted, starts at 0 */
  int undo_stack_hold;
} ImatExData;

void iupMatrixExBusyStart(ImatExData* matex_data, int count, const char* busyname);
int iupMatrixExBusyInc(ImatExData* matex_data);
void iupMatrixExBusyEnd(ImatExData* matex_data);

void iupMatrixExUndoPushBegin(ImatExData* matex_data, const char* busyname);
void iupMatrixExUndoPushEnd(ImatExData* matex_data);

void iupMatrixExSortShowDialog(ImatExData* matex_data);

void iupMatrixExFindShowDialog(ImatExData* matex_data);

int iupMatrixExIsColumnVisible(Ihandle* ih, int col);
int iupMatrixExIsLineVisible(Ihandle* ih, int lin);

void iupMatrixExCheckLimitsOrder(int *v1, int *v2, int min, int max);

void iupMatrixExRegisterClipboard(Iclass* ic);
void iupMatrixExRegisterBusy(Iclass* ic);
void iupMatrixExRegisterVisible(Iclass* ic);
void iupMatrixExRegisterExport(Iclass* ic);
void iupMatrixExRegisterCopy(Iclass* ic);
void iupMatrixExRegisterUnits(Iclass* ic);
void iupMatrixExRegisterUndo(Iclass* ic);
void iupMatrixExRegisterFind(Iclass* ic);
void iupMatrixExRegisterSort(Iclass* ic);

/* Implemented in IupMatrix */
char* iupMatrixExGetCellValue(Ihandle* ih, int lin, int col, int convert);
void  iupMatrixExSetCellValue(Ihandle* ih, int lin, int col, const char* value, int edition);  /* NO numeric conversion */


#ifdef __cplusplus
}
#endif

#endif
