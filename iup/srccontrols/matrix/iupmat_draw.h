/** \file
 * \brief iupmatrix control
 * draw functions.
 *
 * See Copyright Notice in "iup.h"
 */
 
#ifndef __IUPMAT_DRAW_H 
#define __IUPMAT_DRAW_H

#ifdef __cplusplus
extern "C" {
#endif

int   iupMatrixDrawLineTitle  (Ihandle* ih, int lin, int lastlin);
int   iupMatrixDrawColumnTitle(Ihandle* ih, int col, int lastcol);
void  iupMatrixDrawTitleCorner(Ihandle* ih);
void  iupMatrixDrawEmptyArea  (Ihandle* ih, int x1, int x2, int y1, int y2);

void  iupMatrixDrawMatrix(Ihandle* ih, int modo);
void  iupMatrixDrawCells (Ihandle* ih, int l1,  int c1,  int l2, int c2);
void  iupMatrixDrawFocus (Ihandle* ih, int lin, int col, int coloca);

char* iupMatrixDrawGetFgColor(Ihandle* ih, int lin, int col);
char* iupMatrixDrawGetBgColor(Ihandle* ih, int lin, int col);
char* iupMatrixDrawGetFont   (Ihandle* ih, int lin, int col);

/* Options to redraw the matrix, used by iupMatrixDrawMatrix */
#define IMAT_DRAW_ALL 0  /* Redraw the title columns and title lines */
#define IMAT_DRAW_COL 1  /* Redraw only the title columns */
#define IMAT_DRAW_LIN 2  /* Redraw only the title lines */


#ifdef __cplusplus
}
#endif

#endif
