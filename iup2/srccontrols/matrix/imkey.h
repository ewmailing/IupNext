/** \file
 * \brief iupmatrix. keyboard control.
 *
 * See Copyright Notice in iup.h
 *  */
 
#ifndef __IMKEY_H 
#define __IMKEY_H

#ifdef __cplusplus
extern "C" {
#endif

int  iupmatKeyPressCb    (Ihandle *hm, int c, int press);
int iupmatKey(Ihandle *h, int c);

void iupmatResetKeyCount  (void);
int  iupmatGetHomeKeyCount(void);
int  iupmatGetEndKeyCount (void);

#ifdef __cplusplus
}
#endif

#endif
