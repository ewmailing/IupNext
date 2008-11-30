/** \file
 * \brief iupmatrix. keyboard control.
 *
 * See Copyright Notice in iup.h
 */
 
#ifndef __IUPMAT_KEY_H 
#define __IUPMAT_KEY_H

#ifdef __cplusplus
extern "C" {
#endif

int  iupMatrixKeyKeyPressCB(Ihandle* ih, int c, int press);
int  iupMatrixKey(Ihandle* ih, int c);

void iupMatrixKeyResetKeyCount  (void);
int  iupMatrixKeyGetHomeKeyCount(void);
int  iupMatrixKeyGetEndKeyCount (void);

#ifdef __cplusplus
}
#endif

#endif
