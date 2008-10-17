/** \file
 * \brief iupmatrix. cel selection.
 *
 * See Copyright Notice in iup.h
 * $Id: immark.h,v 1.1 2008-10-17 06:20:15 scuri Exp $
 */
 
#ifndef __IMMARK_H 
#define __IMMARK_H

#ifdef __cplusplus
extern "C" {
#endif

/* Modo usado para marcar/desmarcar celulas */
#define MARK_NO      0
#define MARK_CELL    1
#define MARK_LIN     2
#define MARK_COL     3
#define MARK_LINCOL  4

void iupmatMarkDrag(Ihandle *h, int x, int y);

void iupmatMarkBloco         (Ihandle *h, int lin,int col);
void iupmatMarkDesmarcaBloco (Ihandle *h);

int  iupmatMarkCritica (Ihandle *h, int *lin, int *col, int *shift, int *ctrl,  int *duplo);
void iupmatMarkShow    (Ihandle *h, int ctrl, int lin,  int col,    int oldlin, int oldcol);
int  iupmatMarkHide    (Ihandle *h, int ctrl);
void iupmatMarkReset   (void);

int iupmatMarkCellGet   (Tmat *mat,int lin,int col);
void iupmatMarkCellSet   (Tmat *mat,int lin,int col, int mark);

int iupmatMarkColumnMarked (Ihandle *h, int col);
int iupmatMarkLineMarked   (Ihandle *h, int lin);
int iupmatMarkGetMode      (Ihandle *h);

int iupmatMarkFullLin(void);
int iupmatMarkFullCol(void);

void *iupmatMarkSet    (Ihandle *n, char *v);
char *iupmatMarkGet    (Ihandle *h);
void *iupmatMarkSetMode(Ihandle *n, char *v);

#ifdef __cplusplus
}
#endif

#endif
