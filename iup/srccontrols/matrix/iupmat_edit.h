/** \file
 * \brief iupmatrix. 
 * Functions used to edit a cell in place.
 *
 * See Copyright Notice in iup.h
 * $Id: iupmat_edit.h,v 1.1 2008-11-28 00:13:55 scuri Exp $
 */
 
#ifndef __IMEDIT_H 
#define __IMEDIT_H

#ifdef __cplusplus
extern "C" {
#endif

int   iMatrixEditIsVisible(Ihandle *ih);
int   iMatrixEditShow(Ihandle* ih);
int   iMatrixEditClose(Ihandle* ih);
void  iMatrixEditCheckHidden(Ihandle* ih);
char* iMatrixEditGetValue(Ihandle* ih);
void  iMatrixEditCreate(Ihandle* ih);

#ifdef __cplusplus
}
#endif

#endif
