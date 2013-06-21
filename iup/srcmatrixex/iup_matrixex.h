/** \file
 * \brief IupMatrix Expansion Library.
 *
 * See Copyright Notice in "iup.h"
 */
 
#ifndef __IUP_MATRIXEX_H 
#define __IUP_MATRIXEX_H 

#ifdef __cplusplus
extern "C" {
#endif


typedef struct _ImatExData
{
  int busy_count;
  IFniis busy_cb;
} ImatExData;

void iupMatrixExBusyStart(Ihandle* ih, int count, const char* busyname);
int iupMatrixExBusyInc(Ihandle* ih);
void iupMatrixExBusyEnd(Ihandle* ih);

int iupMatrixExIsColumnVisible(Ihandle* ih, int col);
int iupMatrixExIsLineVisible(Ihandle* ih, int lin);

char* iupMatrixExGetCell(Ihandle* ih, int lin, int col, sIFnii value_cb);
void iupMatrixExSetCell(Ihandle *ih, int lin, int col, const char* value, IFniiii edition_cb, IFniis value_edit_cb);


void iupMatrixExRegisterClipboard(Iclass* ic);


#ifdef __cplusplus
}
#endif

#endif
