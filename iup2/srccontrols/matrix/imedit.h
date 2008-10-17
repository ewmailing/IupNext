/** \file
 * \brief iupmatrix. 
 * Functions used to edit a cell in place.
 *
 * See Copyright Notice in iup.h
 * $Id: imedit.h,v 1.1 2008-10-17 06:20:15 scuri Exp $
 */
 
#ifndef __IMEDIT_H 
#define __IMEDIT_H

#ifdef __cplusplus
extern "C" {
#endif

int iupmatEditIsVisible(Ihandle *self);
int iupmatEditShow(Ihandle *h);
int iupmatEditClose(Ihandle *h);
void iupmatEditCheckHidden(Ihandle* h);
char* iupmatEditGetValue(Tmat *mat);
void iupmatEditCreate(Tmat *mat);
void iupmatEditMap(Tmat *mat, Ihandle* parent);

#ifdef __cplusplus
}
#endif

#endif
