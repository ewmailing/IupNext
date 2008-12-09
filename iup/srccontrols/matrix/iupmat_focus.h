/** \file
 * \brief iupmatrix focus control.
 *
 * See Copyright Notice in "iup.h"
 */
 
#ifndef __IUPMAT_FOCUS_H 
#define __IUPMAT_FOCUS_H

#ifdef __cplusplus
extern "C" {
#endif

int  iupMatrixFocusFocusCB(Ihandle* ih, int focus);

void iupMatrixFocusShowFocus  (Ihandle* ih);
void iupMatrixFocusHideFocus  (Ihandle* ih);
void iupMatrixFocusSetFocusPos(Ihandle* ih, int lin, int col);

void iupMatrixFocusHideSetShowFocus(Ihandle* ih, int lin, int col);
void iupMatrixFocusSetShowFocus    (Ihandle* ih, int lin, int col);

int  iupMatrixFocusIsFocusVisible(Ihandle* ih);

#ifdef __cplusplus
}
#endif

#endif
