/** \file
 * \brief iupmatrix focus control.
 *
 * See Copyright Notice in iup.h
 * $Id: imfocus.h,v 1.1 2008-10-17 06:20:15 scuri Exp $
 */
 
#ifndef __IMFOCUS_H 
#define __IMFOCUS_H

#ifdef __cplusplus
extern "C" {
#endif

int iupmatFocusCb(Ihandle *hm, int focus);

void iupmatShowFocus  (Ihandle *h);
void iupmatHideFocus  (Ihandle *h);
void iupmatSetFocusPos(Ihandle *h, int lin, int col);

void iupmatHideSetShowFocus(Ihandle *h, int lin, int col);
void iupmatSetShowFocus    (Ihandle *h, int lin, int col);

int iupmatIsFocusVisible(Ihandle *h);

#ifdef __cplusplus
}
#endif

#endif
