/** \file
 * \brief iupmatrix. 
 * Functions used to edit a cell in place.
 *
 * See Copyright Notice in "iup.h"
 */
 
#ifndef __IUPMAT_EDIT_H 
#define __IUPMAT_EDIT_H

#ifdef __cplusplus
extern "C" {
#endif

int   iupMatrixEditIsVisible(Ihandle *ih);
int   iupMatrixEditShow     (Ihandle* ih);
void  iupMatrixEditHide     (Ihandle* ih);
int   iupMatrixEditConfirm  (Ihandle* ih);
void  iupMatrixEditAbort    (Ihandle* ih);
void  iupMatrixEditUpdatePos(Ihandle* ih);
char* iupMatrixEditGetValue (Ihandle* ih);
void  iupMatrixEditCreate   (Ihandle* ih);

#ifdef __cplusplus
}
#endif

#endif
