/** \file
 * \brief iupmatrix control
 * auxiliary functions.
 *
 * See Copyright Notice in "iup.h"
 */
 
#ifndef __IUPMAT_AUX_H 
#define __IUPMAT_AUX_H

#ifdef __cplusplus
extern "C" {
#endif

int   iupMatrixAuxIsFullVisibleLast(ImatLinColData *p);
int   iupMatrixAuxIsCellStartVisible(Ihandle* ih, int lin, int col);
int   iupMatrixAuxIsCellVisible(Ihandle* ih, int lin, int col);
void  iupMatrixAuxGetVisibleCellDim   (Ihandle* ih, int lin, int col, int* x, int* y, int* w, int* h);

void  iupMatrixAuxCalcSizes(Ihandle* ih);

void iupMatrixAuxAdjustFirstFromLast(ImatLinColData* p);
void iupMatrixAuxAdjustFirstFromScrollPos(ImatLinColData* p, int scroll_pos);
void  iupMatrixAuxUpdateScrollPos(Ihandle* ih, int m);
void  iupMatrixAuxUpdateLast(ImatLinColData *p);

int   iupMatrixAuxGetColumnWidth(Ihandle* ih, int col, int use_value);
int   iupMatrixAuxGetLineHeight (Ihandle* ih, int lin, int use_value);

int   iupMatrixAuxGetLinColFromXY    (Ihandle* ih, int x, int y, int* l, int* c);

int   iupMatrixAuxCallLeaveCellCb    (Ihandle* ih);
void  iupMatrixAuxCallEnterCellCb    (Ihandle* ih);
int   iupMatrixAuxCallEditionCbLinCol(Ihandle* ih, int lin, int col, int mode, int update);

#ifdef __cplusplus
}
#endif

#endif
