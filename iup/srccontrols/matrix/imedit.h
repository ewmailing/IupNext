/** \file
 * \brief iupmatrix. 
 * Functions used to edit a cell in place.
 *
 * See Copyright Notice in iup.h
 * $Id: imedit.h,v 1.1 2008-10-17 06:05:36 scuri Exp $
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
