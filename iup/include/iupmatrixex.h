/** \file
 * \brief IupMatrix Expansion Library.
 *
 * See Copyright Notice in "iup.h"
 */
 
#ifndef __IUPMATRIXEX_H 
#define __IUPMATRIXEX_H


#ifdef __cplusplus
extern "C" {
#endif


void IupMatrixExOpen(void);

Ihandle* IupMatrixEx(void);

void IupMatrixExInit(Ihandle* ih);

/* available only when linking with "iupluamatrixex" */
void IupMatrixExSetFormula(Ihandle* ih, int col, const char* formula, const char* init);


#ifdef __cplusplus
}
#endif

#endif
