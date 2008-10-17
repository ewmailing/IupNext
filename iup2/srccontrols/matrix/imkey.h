/** \file
 * \brief iupmatrix. keyboard control.
 *
 * See Copyright Notice in iup.h
 * $Id: imkey.h,v 1.1 2008-10-17 06:20:15 scuri Exp $
 */
 
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
