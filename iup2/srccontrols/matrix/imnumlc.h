/** \file
 * \brief iupmatrix. change number of collumns or lines.
 *
 * See Copyright Notice in iup.h
 *  */
 
#ifndef __IMNUMLC_H 
#define __IMNUMLC_H

#ifdef __cplusplus
extern "C" {
#endif

char *iupmatNlcGetNumLin(Ihandle *h);
char *iupmatNlcGetNumCol(Ihandle *h);

void iupmatNlcAddLin(Ihandle *h, char *v);
void iupmatNlcDelLin(Ihandle *h, char *v);
void iupmatNlcAddCol(Ihandle *h, char *v);
void iupmatNlcDelCol(Ihandle *h, char *v);

void iupmatNlcNumLin(Ihandle *h, char *v);
void iupmatNlcNumCol(Ihandle *h, char *v);


#ifdef __cplusplus
}
#endif

#endif
