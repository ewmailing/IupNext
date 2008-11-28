/** \file
 * \brief iupmatrix control
 * draw functions.
 *
 * See Copyright Notice in iup.h
 * $Id: imdraw.h,v 1.2 2008-11-28 00:02:49 scuri Exp $
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


/* Options to redraw the matrix, used by iMatrixDrawMatrix */
#define IMATRIX_DRAW_ALL 0  /* Redraw the title columns and title lines */
#define IMATRIX_DRAW_COL 1  /* Redraw only the title columns */
#define IMATRIX_DRAW_LIN 2  /* Redraw only the title lines */


#ifdef __cplusplus
}
#endif

#endif
