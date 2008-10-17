/** \file
 * \brief iupmatrix control
 * draw functions.
 *
 * See Copyright Notice in iup.h
 * $Id: imdraw.h,v 1.1 2008-10-17 06:05:36 scuri Exp $
 */
 
#ifndef __IMDRAW_H 
#define __IMDRAW_H

#ifdef __cplusplus
extern "C" {
#endif

int   iMatrixDrawLineTitle  (Ihandle* ih, int lin, int lastlin);
int   iMatrixDrawColumnTitle(Ihandle* ih, int col, int lastcol);
void  iMatrixDrawTitleCorner(Ihandle* ih);
void  iMatrixDrawEmptyArea  (Ihandle* ih, int x1, int x2, int y1, int y2);

void  iMatrixDrawMatrix(Ihandle* ih, int modo);
void  iMatrixDrawCells (Ihandle* ih, int l1,  int c1,  int l2, int c2);
void  iMatrixDrawFocus (Ihandle* ih, int lin, int col, int coloca);

char* iMatrixDrawGetFgColor(Ihandle* ih, int lin, int col);
char* iMatrixDrawGetBgColor(Ihandle* ih, int lin, int col);
char* iMatrixDrawGetFont   (Ihandle* ih, int lin, int col);


/* Formas de redesenhar a matriz, usadas por iMatrixDrawMatrix */
#define IMATRIX_DRAW_ALL 0  /* Redesenha as colunas e linhas de titulos */
#define IMATRIX_DRAW_COL 1  /* Redesenha somente as colunas de titulos */
#define IMATRIX_DRAW_LIN 2  /* Redesenha somente as linas de titulos */


#ifdef __cplusplus
}
#endif

#endif
