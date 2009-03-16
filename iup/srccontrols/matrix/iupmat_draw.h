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

/* Render one cell */
void iupMatrixDrawCell(Ihandle* ih, int lin, int col);

/* Render the visible cells and update display */
void iupMatrixDraw(Ihandle* ih, int update);

/* Update the display only */
void iupMatrixDrawUpdate(Ihandle* ih);

int iupMatrixDrawSetRedrawAttrib(Ihandle* ih, const char* value);

#ifdef __cplusplus
}
#endif

#endif
