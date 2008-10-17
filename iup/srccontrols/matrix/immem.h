/** \file
 * \brief iupmatrix control
 * memory allocation.
 *
 * See Copyright Notice in iup.h
 * $Id: immem.h,v 1.1 2008-10-17 06:05:36 scuri Exp $
 */
 
#ifndef __IMMEM_H 
#define __IMMEM_H

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
