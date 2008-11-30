/** \file
 * \brief iupmatrix column resize.
 *
 * See Copyright Notice in iup.h
 */
 
#ifndef __IUPMAT_COLRES_H 
#define __IUPMAT_COLRES_H

#ifdef __cplusplus
extern "C" {
#endif

int   iupMatrixColResTry         (Ihandle* ih, int x, int y);
void  iupMatrixColResFinish      (Ihandle* ih, int x);
void  iupMatrixColResMove        (Ihandle* ih, int x);
void  iupMatrixColResChangeCursor(Ihandle* ih, int x, int y);
int   iupMatrixColResResizing    (void);

void  iupMatrixColResSet(Ihandle* ih, const char* value, int col, int m, int pixels);
char* iupMatrixColResGet(Ihandle* ih, int col, int m, int pixels);

#ifdef __cplusplus
}
#endif

#endif
