/** \file
 * \brief iupmatrix control
 * draw functions.
 *
 * See Copyright Notice in iup.h
 * $Id: imdraw.h,v 1.1 2008-10-17 06:20:15 scuri Exp $
 */
 
#ifndef __IMDRAW_H 
#define __IMDRAW_H

#ifdef __cplusplus
extern "C" {
#endif

int  iupmatDrawLineTitle  (Ihandle *h, int lin, int lastlin);
int  iupmatDrawColumnTitle(Ihandle *h, int col, int lastcol);
void iupmatDrawTitleCorner(Ihandle *h);
void iupmatDrawEmptyArea  (Ihandle *h, int x1, int x2, int y1, int y2);


void iupmatDrawMatrix(Ihandle *h, int modo);
void iupmatDrawCells (Ihandle *h, int l1,  int c1,  int l2, int c2);
void iupmatDrawFocus (Ihandle *h, int lin, int col, int coloca);

char* iupmatDrawGetFgColor(Ihandle *h, int lin, int col);
char* iupmatDrawGetBgColor(Ihandle *h, int lin, int col);
char* iupmatDrawGetFont   (Ihandle *h, int lin, int col);


/* Formas de redesenhar a matriz, usadas por iupmatDrawMatrix */
#define DRAW_ALL 0  /* Redesenha as colunas e linhas de titulos */
#define DRAW_COL 1  /* Redesenha somente as colunas de titulos */
#define DRAW_LIN 2  /* Redesenha somente as linas de titulos */


#ifdef __cplusplus
}
#endif

#endif
