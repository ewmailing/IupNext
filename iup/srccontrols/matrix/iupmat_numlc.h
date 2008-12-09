/** \file
 * \brief iupmatrix. change number of collumns or lines.
 *
 * See Copyright Notice in "iup.h"
 */
 
#ifndef __IUPMAT_NUMLC_H 
#define __IUPMAT_NUMLC_H

#ifdef __cplusplus
extern "C" {
#endif

char* iupMatrixNumLCGetNumLin(Ihandle* ih);
char* iupMatrixNumLCGetNumCol(Ihandle* ih);

void  iupMatrixNumLCAddLin(Ihandle* ih, const char* v);
void  iupMatrixNumLCDelLin(Ihandle* ih, const char* v);
void  iupMatrixNumLCAddCol(Ihandle* ih, const char* v);
void  iupMatrixNumLCDelCol(Ihandle* ih, const char* v);

void  iupMatrixNumLCNumLin(Ihandle* ih, const char* v);
void  iupMatrixNumLCNumCol(Ihandle* ih, const char* v);


#ifdef __cplusplus
}
#endif

#endif
