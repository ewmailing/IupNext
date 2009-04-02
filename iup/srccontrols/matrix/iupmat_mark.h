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
#define IMAT_MARK_LIN     1
#define IMAT_MARK_COL     2
#define IMAT_MARK_LINCOL  3
#define IMAT_MARK_CELL    4

int iupMatrixSetMarkedAttrib(Ihandle* ih, const char* value);
char* iupMatrixGetMarkedAttrib(Ihandle* ih);
char* iupMatrixGetMarkAttrib(Ihandle* ih, const char* name_id);
int iupMatrixSetMarkAttrib(Ihandle* ih, const char* name_id, const char* value);

void iupMatrixMarkClearAll(Ihandle* ih, int check);

int iupMatrixMarkCellGet(Ihandle* ih, int lin, int col, IFnii mark_cb, char* str);
void iupMatrixMarkCellSet(Ihandle* ih, int lin, int col, int mark, IFniii markedit_cb, char* str);

int iupMatrixColumnIsMarked(Ihandle* ih, int col);
int iupMatrixLineIsMarked  (Ihandle* ih, int lin);

void iupMatrixMarkMouseDrag(Ihandle* ih, int x, int y);
void iupMatrixMarkMouseBlock(Ihandle* ih, int lin, int col);
void iupMatrixMarkMouseItem(Ihandle* ih, int ctrl, int lin, int col);
void iupMatrixMarkMouseReset(Ihandle* ih);


#ifdef __cplusplus
}
#endif

#endif
