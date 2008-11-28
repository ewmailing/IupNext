/** \file
 * \brief iupmatrix. cel selection.
 *
 * See Copyright Notice in iup.h
 * $Id: iupmat_mark.h,v 1.1 2008-11-28 00:13:55 scuri Exp $
 */
 
#ifndef __IMMARK_H 
#define __IMMARK_H

#ifdef __cplusplus
extern "C" {
#endif

/* Modo usado para marcar/desmarcar celulas */
#define IMATRIX_MARK_NO      0
#define IMATRIX_MARK_CELL    1
#define IMATRIX_MARK_LIN     2
#define IMATRIX_MARK_COL     3
#define IMATRIX_MARK_LINCOL  4

void iMatrixMarkDrag(Ihandle* ih, int x, int y);

void iMatrixMarkBlock         (Ihandle* ih, int lin, int col);
void iMatrixMarkUncheckBlock (Ihandle* ih);

int  iMatrixMarkCritica (Ihandle* ih, int* lin, int* col, int* shift, int* ctrl,  int* duplo);
void iMatrixMarkShow    (Ihandle* ih, int ctrl, int lin,  int col,    int oldlin, int oldcol);
int  iMatrixMarkHide    (Ihandle* ih, int ctrl);
void iMatrixMarkReset   (void);

int  iMatrixMarkCellGet   (Ihandle* ih, int lin, int col);
void iMatrixMarkCellSet   (Ihandle* ih, int lin, int col, int mark);

int iMatrixMarkColumnMarked (Ihandle* ih, int col);
int iMatrixMarkLineMarked   (Ihandle* ih, int lin);
int iMatrixMarkGetMode      (Ihandle* ih);

int iMatrixMarkFullLin(void);
int iMatrixMarkFullCol(void);

void *iMatrixMarkSet    (Ihandle* ih, const char* v);
char* iMatrixMarkGet    (Ihandle* ih);
void *iMatrixMarkSetMode(Ihandle* ih, const char* v);

#ifdef __cplusplus
}
#endif

#endif
