/** \file
 * \brief iupmatrix control
 * auxiliary functions.
 *
 * See Copyright Notice in iup.h
 */
 
#ifndef __IUPMAT_AUX_H 
#define __IUPMAT_AUX_H

#ifdef __cplusplus
extern "C" {
#endif

int   iupMatrixAuxIsCellVisible(Ihandle* ih, int lin, int col);
int   iupMatrixAuxGetCellDim   (Ihandle* ih, int lin, int col, int* x, int* y, int* dx, int* dy);

void  iupMatrixAuxGetPos      (Ihandle* ih, int m);
void  iupMatrixAuxGetLastWidth(Ihandle* ih, int m);

int   iupMatrixAuxGetColumnWidth(Ihandle* ih, int col);
int   iupMatrixAuxGetLineHeight (Ihandle* ih, int lin);
void  iupMatrixAuxFillWidthVec  (Ihandle* ih);
void  iupMatrixAuxFillHeightVec (Ihandle* ih);

int   iupMatrixAuxGetTitlelineSize  (Ihandle* ih);
int   iupMatrixAuxGetTitlecolumnSize(Ihandle* ih);
int   iupMatrixAuxGetLineCol        (Ihandle* ih, int x, int y, int* l, int* c);
void  iupMatrixAuxUpdateCellValue   (Ihandle* ih);
char* iupMatrixAuxGetCellValue      (Ihandle* ih, int lin, int col);

int   iupMatrixAuxCallLeavecellCb    (Ihandle* ih);
void  iupMatrixAuxCallEntercellCb    (Ihandle* ih);
int   iupMatrixAuxCallEditionCbLinCol(Ihandle* ih, int lin, int col, int modo);

int   iupMatrixAuxTextHeight(Ihandle* ih, char *text, int* totalh, int* lineh, int* spacing);
void  iupMatrixAuxTextWidth (Ihandle* ih, char *text, int* width);

#ifdef __cplusplus
}
#endif

#endif
