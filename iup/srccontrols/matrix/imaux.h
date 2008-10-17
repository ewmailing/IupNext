/** \file
 * \brief iupmatrix control
 * auxiliary functions.
 *
 * See Copyright Notice in iup.h
 * $Id: imaux.h,v 1.1 2008-10-17 06:05:36 scuri Exp $
 */
 
#ifndef __IMAUX_H 
#define __IMAUX_H

#ifdef __cplusplus
extern "C" {
#endif

int   iMatrixIsCellVisible(Ihandle* ih, int lin, int col);
int   iMatrixGetCellDim   (Ihandle* ih, int lin, int col, int* x, int* y, int* dx, int* dy);

void  iMatrixGetPos            (Ihandle* ih, int m);
void  iMatrixGetLastWidth      (Ihandle* ih, int m);

int   iMatrixGetColumnWidth    (Ihandle* ih, int col);
int   iMatrixGetLineHeight     (Ihandle* ih, int lin);
void  iMatrixFillWidthVec      (Ihandle* ih);
void  iMatrixFillHeightVec     (Ihandle* ih);

int   iMatrixGetTitlelineSize  (Ihandle* ih);
int   iMatrixGetTitlecolumnSize(Ihandle* ih);
int   iMatrixGetLineCol        (Ihandle* ih, int x, int y, int* l, int* c);
void  iMatrixUpdateCellValue   (Ihandle* ih);
char* iMatrixGetCellValue      (Ihandle* ih, int lin, int col);

int   iMatrixCallLeavecellCb   (Ihandle* ih);
void  iMatrixCallEntercellCb   (Ihandle* ih);
int   iMatrixCallEditionCbLinCol(Ihandle* ih, int lin, int col, int modo);

int   iMatrixTextHeight        (Ihandle* ih, char *text, int* totalh, int* lineh, int* spacing);
void  iMatrixTextWidth         (Ihandle* ih, char *text, int* width);

#ifdef __cplusplus
}
#endif

#endif
