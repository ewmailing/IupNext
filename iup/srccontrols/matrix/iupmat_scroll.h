/** \file
 * \brief iupmatrix control
 * scrolling.
 *
 * See Copyright Notice in "iup.h"
 */
 
#ifndef __IUPMAT_SCROLL_H 
#define __IUPMAT_SCROLL_H

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*iupMatrixScrollMoveF)(Ihandle*, int, float, int);

int  iupMatrixScrollCB  (Ihandle* ih, int action, float x, float y);
void iupMatrixScroll    (Ihandle* ih, int dir, int ref_col, int ncol);
void iupMatrixScrollOpen(Ihandle* ih, int lin, int col);

int  iupMatrixScrollMoveCursor(iupMatrixScrollMoveF func, Ihandle* ih, int modo, float pos, int m);

void iupMatrixScrollHome       (Ihandle* ih, int, float, int);
void iupMatrixScrollEnd        (Ihandle* ih, int, float, int);
void iupMatrixScrollLeftUp     (Ihandle* ih, int, float, int);
void iupMatrixScrollRightDown  (Ihandle* ih, int, float, int);
void iupMatrixScrollPgLeftUp   (Ihandle* ih, int, float, int);
void iupMatrixScrollPgRightDown(Ihandle* ih, int, float, int);
void iupMatrixScrollPos        (Ihandle* ih, int, float, int);
void iupMatrixScrollCr         (Ihandle* ih, int, float, int);

/* Macros used by "dir" parameter of the iMatrixScrollLine and iMatrixScrollColumn functions */
#define IMAT_SCROLL_LEFT    0
#define IMAT_SCROLL_RIGHT   1
#define IMAT_SCROLL_UP      2
#define IMAT_SCROLL_DOWN    3

/* Mode used to "walk" inside the matrix.
   It shows if the movement request was from the scrollbar or from the key.
   Possible values for the "mode" parameter of the iupMatrixScrollMoveCursor function.
 */
#define IMAT_SCROLL    0
#define IMAT_KEY       1


/* Macros to help during the call of iupMatScrollMoveCursor function */

#define ScrollKeyHome(ih)    iupMatrixScrollMoveCursor(iupMatrixScrollHome       , ih, IMAT_KEY, 0, 0)
#define ScrollKeyEnd(ih)     iupMatrixScrollMoveCursor(iupMatrixScrollEnd        , ih, IMAT_KEY, 0, 0)
#define ScrollKeyPgUp(ih)    iupMatrixScrollMoveCursor(iupMatrixScrollPgLeftUp   , ih, IMAT_KEY, 0, IMAT_MAT_LIN)
#define ScrollKeyPgDown(ih)  iupMatrixScrollMoveCursor(iupMatrixScrollPgRightDown, ih, IMAT_KEY, 0, IMAT_MAT_LIN)
#define ScrollKeyUp(ih)      iupMatrixScrollMoveCursor(iupMatrixScrollLeftUp     , ih, IMAT_KEY, 0, IMAT_MAT_LIN)
#define ScrollKeyDown(ih)    iupMatrixScrollMoveCursor(iupMatrixScrollRightDown  , ih, IMAT_KEY, 0, IMAT_MAT_LIN)
#define ScrollKeyLeft(ih)    iupMatrixScrollMoveCursor(iupMatrixScrollLeftUp     , ih, IMAT_KEY, 0, IMAT_MAT_COL)
#define ScrollKeyRight(ih)   iupMatrixScrollMoveCursor(iupMatrixScrollRightDown  , ih, IMAT_KEY, 0, IMAT_MAT_COL)
#define ScrollKeyCr(ih)      iupMatrixScrollMoveCursor(iupMatrixScrollCr         , ih, IMAT_KEY, 0, IMAT_MAT_LIN)

#define ScrollPgUp(ih)       iupMatrixScrollMoveCursor(iupMatrixScrollPgLeftUp   , ih, IMAT_SCROLL, 0, IMAT_MAT_LIN)
#define ScrollPgDown(ih)     iupMatrixScrollMoveCursor(iupMatrixScrollPgRightDown, ih, IMAT_SCROLL, 0, IMAT_MAT_LIN)
#define ScrollUp(ih)         iupMatrixScrollMoveCursor(iupMatrixScrollLeftUp     , ih, IMAT_SCROLL, 0, IMAT_MAT_LIN)
#define ScrollDown(ih)       iupMatrixScrollMoveCursor(iupMatrixScrollRightDown  , ih, IMAT_SCROLL, 0, IMAT_MAT_LIN)
#define ScrollLeft(ih)       iupMatrixScrollMoveCursor(iupMatrixScrollLeftUp     , ih, IMAT_SCROLL, 0, IMAT_MAT_COL)
#define ScrollRight(ih)      iupMatrixScrollMoveCursor(iupMatrixScrollRightDown  , ih, IMAT_SCROLL, 0, IMAT_MAT_COL)
#define ScrollPgLeft(ih)     iupMatrixScrollMoveCursor(iupMatrixScrollPgLeftUp   , ih, IMAT_SCROLL, 0, IMAT_MAT_COL)
#define ScrollPgRight(ih)    iupMatrixScrollMoveCursor(iupMatrixScrollPgRightDown, ih, IMAT_SCROLL, 0, IMAT_MAT_COL)
#define ScrollPosVer(ih, y)  iupMatrixScrollMoveCursor(iupMatrixScrollPos        , ih, IMAT_SCROLL, y, IMAT_MAT_LIN)
#define ScrollPosHor(ih, x)  iupMatrixScrollMoveCursor(iupMatrixScrollPos        , ih, IMAT_SCROLL, x, IMAT_MAT_COL)


#ifdef __cplusplus
}
#endif

#endif
