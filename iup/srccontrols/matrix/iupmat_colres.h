/** \file
 * \brief iupmatrix column resize.
 *
 * See Copyright Notice in iup.h
 * $Id: iupmat_colres.h,v 1.2 2008-11-28 00:19:04 scuri Exp $
 */
 
#ifndef __IUPMAT_COLRES_H 
#define __IUPMAT_COLRES_H

#ifdef __cplusplus
extern "C" {
#endif

int   iMatrixColResTry         (Ihandle* ih, int x, int y);
void  iMatrixColResFinish      (Ihandle* ih, int x);
void  iMatrixColResMove        (Ihandle* ih, int x);
void  iMatrixColResChangeCursor(Ihandle* ih, int x, int y);
int   iMatrixColResResizing    (void);

void  iMatrixColResSet (Ihandle* ih, const char* value, int col, int m, int pixels);
char* iMatrixColResGet (Ihandle* ih, int col, int m, int pixels);

#ifdef __cplusplus
}
#endif

#endif
