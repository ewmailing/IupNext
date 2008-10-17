/** \file
 * \brief iupmatrix control
 * attributes set and get.
 *
 * See Copyright Notice in iup.h
 * $Id: imgetset.h,v 1.1 2008-10-17 06:20:15 scuri Exp $
 */
 
#ifndef __IMGETSET_H 
#define __IMGETSET_H

#ifdef __cplusplus
extern "C" {
#endif

void *iupmatSetCell(Ihandle *n, int lin, int col, char *v);
char *iupmatGetCell(Ihandle *n, int lin, int col);

void  iupmatSetFocusPosition (Ihandle *h, char *v, int call_cb);
char *iupmatGetFocusPosition (Ihandle *h);

void iupmatSetColAlign(Ihandle *h, int col);

void iupmatSetTitleLine  (Ihandle *h, int linha);
void iupmatSetTitleColumn(Ihandle *h, int col);

void  iupmatSetOrigin(Ihandle *h, char *value);
char *iupmatGetOrigin(Ihandle *h);

void  iupmatSetRedraw(Ihandle *h, char *value);
void  iupmatSetRedrawCell (Ihandle* h, int lin, int col);

void  iupmatSetActive(Ihandle *h, int mode, int lincol, char *val);

#ifdef __cplusplus
}
#endif

#endif
