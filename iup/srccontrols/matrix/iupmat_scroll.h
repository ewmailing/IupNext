/** \file
 * \brief iupmatrix control
 * scrolling.
 *
 * See Copyright Notice in iup.h
 * $Id: iupmat_scroll.h,v 1.1 2008-11-28 00:13:55 scuri Exp $
 */
 
#ifndef __IMSCROLL_H 
#define __IMSCROLL_H

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*iMatrixScrollMoveF)(Ihandle*, int, float, int);

int  iMatrixScrollCB  (Ihandle* ih, int action, float x, float y);
void iMatrixScroll    (Ihandle* ih, int dir, int ref_col, int ncol);
void iMatrixScrollOpen(Ihandle* ih, int lin, int col);

int  iMatrixScrollMoveCursor(iMatrixScrollMoveF func, Ihandle* ih, int modo, float pos, int m);

void iMatrixScrollHome        (Ihandle* ih, int, float, int);
void iMatrixScrollEnd         (Ihandle* ih, int, float, int);
void iMatrixScrollLeftUp      (Ihandle* ih, int, float, int);
void iMatrixScrollRightDown   (Ihandle* ih, int, float, int);
void iMatrixScrollPgLeftUp    (Ihandle* ih, int, float, int);
void iMatrixScrollPgRightDown (Ihandle* ih, int, float, int);
void iMatrixScrollPos         (Ihandle* ih, int, float, int);
void iMatrixScrollCr          (Ihandle* ih, int, float, int);

/* Macros used by "dir" parameter of the iMatrixScrollLine and iMatrixScrollColumn functions */
#define IMATRIX_SCROLL_LEFT    0
#define IMATRIX_SCROLL_RIGHT   1
#define IMATRIX_SCROLL_UP      2
#define IMATRIX_SCROLL_DOWN    3

/* Mode used to "walk" inside the matrix.
   It shows if the movement request was from the scrollbar or from the key.
   Possible values for the "modo" parameter of the iMatrixScrollMoveCursor function.
 */
#define SCROLL    0
#define KEY       1


/* Macros to help during the call of iupMatScrollMoveCursor function */

#define ScrollKeyHome(ih)    iMatrixScrollMoveCursor(iMatrixScrollHome       , ih, KEY, 0, 0)
#define ScrollKeyEnd(ih)     iMatrixScrollMoveCursor(iMatrixScrollEnd        , ih, KEY, 0, 0)
#define ScrollKeyPgUp(ih)    iMatrixScrollMoveCursor(iMatrixScrollPgLeftUp   , ih, KEY, 0, IMATRIX_MAT_LIN)
#define ScrollKeyPgDown(ih)  iMatrixScrollMoveCursor(iMatrixScrollPgRightDown, ih, KEY, 0, IMATRIX_MAT_LIN)
#define ScrollKeyUp(ih)      iMatrixScrollMoveCursor(iMatrixScrollLeftUp     , ih, KEY, 0, IMATRIX_MAT_LIN)
#define ScrollKeyDown(ih)    iMatrixScrollMoveCursor(iMatrixScrollRightDown  , ih, KEY, 0, IMATRIX_MAT_LIN)
#define ScrollKeyLeft(ih)    iMatrixScrollMoveCursor(iMatrixScrollLeftUp     , ih, KEY, 0, IMATRIX_MAT_COL)
#define ScrollKeyRight(ih)   iMatrixScrollMoveCursor(iMatrixScrollRightDown  , ih, KEY, 0, IMATRIX_MAT_COL)
#define ScrollKeyCr(ih)      iMatrixScrollMoveCursor(iMatrixScrollCr         , ih, KEY, 0, IMATRIX_MAT_LIN)

#define ScrollPgUp(ih)       iMatrixScrollMoveCursor(iMatrixScrollPgLeftUp   , ih, SCROLL, 0, IMATRIX_MAT_LIN)
#define ScrollPgDown(ih)     iMatrixScrollMoveCursor(iMatrixScrollPgRightDown, ih, SCROLL, 0, IMATRIX_MAT_LIN)
#define ScrollUp(ih)         iMatrixScrollMoveCursor(iMatrixScrollLeftUp     , ih, SCROLL, 0, IMATRIX_MAT_LIN)
#define ScrollDown(ih)       iMatrixScrollMoveCursor(iMatrixScrollRightDown  , ih, SCROLL, 0, IMATRIX_MAT_LIN)
#define ScrollLeft(ih)       iMatrixScrollMoveCursor(iMatrixScrollLeftUp     , ih, SCROLL, 0, IMATRIX_MAT_COL)
#define ScrollRight(ih)      iMatrixScrollMoveCursor(iMatrixScrollRightDown  , ih, SCROLL, 0, IMATRIX_MAT_COL)
#define ScrollPgLeft(ih)     iMatrixScrollMoveCursor(iMatrixScrollPgLeftUp   , ih, SCROLL, 0, IMATRIX_MAT_COL)
#define ScrollPgRight(ih)    iMatrixScrollMoveCursor(iMatrixScrollPgRightDown, ih, SCROLL, 0, IMATRIX_MAT_COL)
#define ScrollPosVer(ih, y)  iMatrixScrollMoveCursor(iMatrixScrollPos        , ih, SCROLL, y, IMATRIX_MAT_LIN)
#define ScrollPosHor(ih, x)  iMatrixScrollMoveCursor(iMatrixScrollPos        , ih, SCROLL, x, IMATRIX_MAT_COL)


#ifdef __cplusplus
}
#endif

#endif
