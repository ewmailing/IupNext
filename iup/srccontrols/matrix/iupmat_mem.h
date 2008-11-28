/** \file
 * \brief iupmatrix control
 * memory allocation.
 *
 * See Copyright Notice in iup.h
 * $Id: iupmat_mem.h,v 1.2 2008-11-28 00:19:04 scuri Exp $
 */
 
#ifndef __IUPMAT_MEM_H 
#define __IUPMAT_MEM_H

#ifdef __cplusplus
extern "C" {
#endif

void iMatrixMemAloc         (Ihandle* ih);
void iMatrixMemRealocLines  (Ihandle* ih, int nlines, int nl);
void iMatrixMemRealocColumns(Ihandle* ih, int ncols, int nc);
void iMatrixMemAlocCell     (Ihandle* ih, int lin, int col, int numc);

#ifdef __cplusplus
}
#endif

#endif
