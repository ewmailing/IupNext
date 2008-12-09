/** \file
 * \brief iupmatrix. cell selection.
 *
 * See Copyright Notice in "iup.h"
 */
 
#ifndef __IUPMAT_MARK_H 
#define __IUPMAT_MARK_H

#ifdef __cplusplus
extern "C" {
#endif

/* Used to mark (or not mark) cells */
#define IMAT_MARK_NO      0
#define IMAT_MARK_CELL    1
#define IMAT_MARK_LIN     2
#define IMAT_MARK_COL     3
#define IMAT_MARK_LINCOL  4

void iupMatrixMarkDrag(Ihandle* ih, int x, int y);

void iupMatrixMarkBlock         (Ihandle* ih, int lin, int col);
void iupMatrixMarkUncheckedBlock(Ihandle* ih);

int  iupMatrixMarkCritica(Ihandle* ih, int* lin, int* col, int* shift, int* ctrl,  int* duplo);
void iupMatrixMarkShow   (Ihandle* ih, int ctrl, int lin,  int col,    int oldlin, int oldcol);
int  iupMatrixMarkHide   (Ihandle* ih, int ctrl);
void iupMatrixMarkReset  (void);

int  iupMatrixMarkCellGet(Ihandle* ih, int lin, int col);
void iupMatrixMarkCellSet(Ihandle* ih, int lin, int col, int mark);

int iupMatrixMarkColumnMarked(Ihandle* ih, int col);
int iupMatrixMarkLineMarked  (Ihandle* ih, int lin);
int iupMatrixMarkGetMode     (Ihandle* ih);

int iupMatrixMarkFullLin(void);
int iupMatrixMarkFullCol(void);

void *iupMatrixMarkSet    (Ihandle* ih, const char* v);
char* iupMatrixMarkGet    (Ihandle* ih);
void *iupMatrixMarkSetMode(Ihandle* ih, const char* v);

#ifdef __cplusplus
}
#endif

#endif
