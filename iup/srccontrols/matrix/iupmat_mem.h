/** \file
 * \brief iupmatrix control
 * memory allocation.
 *
 * See Copyright Notice in "iup.h"
 */
 
#ifndef __IUPMAT_MEM_H 
#define __IUPMAT_MEM_H

#ifdef __cplusplus
extern "C" {
#endif

void iupMatrixMemAloc         (Ihandle* ih);
void iupMatrixMemRealocLines  (Ihandle* ih, int nlines, int nl);
void iupMatrixMemRealocColumns(Ihandle* ih, int ncols, int nc);
void iupMatrixMemAlocCell     (Ihandle* ih, int lin, int col, int numc);

#ifdef __cplusplus
}
#endif

#endif
