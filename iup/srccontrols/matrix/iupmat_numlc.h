/** \file
 * \brief iupmatrix. change number of collumns or lines.
 *
 * See Copyright Notice in iup.h
 * $Id: iupmat_numlc.h,v 1.1 2008-11-28 00:13:55 scuri Exp $
 */
 
#ifndef __IMNUMLC_H 
#define __IMNUMLC_H

#ifdef __cplusplus
extern "C" {
#endif

char* iMatrixNlcGetNumLin(Ihandle* ih);
char* iMatrixNlcGetNumCol(Ihandle* ih);

void  iMatrixNlcAddLin(Ihandle* ih, const char* v);
void  iMatrixNlcDelLin(Ihandle* ih, const char* v);
void  iMatrixNlcAddCol(Ihandle* ih, const char* v);
void  iMatrixNlcDelCol(Ihandle* ih, const char* v);

void  iMatrixNlcNumLin(Ihandle* ih, const char* v);
void  iMatrixNlcNumCol(Ihandle* ih, const char* v);


#ifdef __cplusplus
}
#endif

#endif
