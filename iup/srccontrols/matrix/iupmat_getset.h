/** \file
 * \brief iupmatrix control
 * attributes set and get.
 *
 * See Copyright Notice in iup.h
 */
 
#ifndef __IUPMAT_GETSET_H 
#define __IUPMAT_GETSET_H

#ifdef __cplusplus
extern "C" {
#endif

void* iupMatrixGSSetCell(Ihandle* ih, int lin, int col, const char* v);
char* iupMatrixGSGetCell(Ihandle* ih, int lin, int col);

void  iupMatrixGSSetFocusPosition (Ihandle* ih, const char* v, int call_cb);
char* iupMatrixGSGetFocusPosition (Ihandle* ih);

void  iupMatrixGSSetColAlign(Ihandle* ih, int col);

void  iupMatrixGSSetTitleLine  (Ihandle* ih, int lin);
void  iupMatrixGSSetTitleColumn(Ihandle* ih, int col);

void  iupMatrixGSSetOrigin(Ihandle* ih, const char* value);
char* iupMatrixGSGetOrigin(Ihandle* ih);

void  iupMatrixGSSetRedraw(Ihandle* ih, const char* value);
void  iupMatrixGSSetRedrawCell (Ihandle* ih, int lin, int col);

void  iupMatrixGSSetActive(Ihandle* ih, int mode, int lincol, const char* val);

#ifdef __cplusplus
}
#endif

#endif
