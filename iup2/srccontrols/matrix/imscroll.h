/** \file
 * \brief iupmatrix control
 * scrolling.
 *
 * See Copyright Notice in iup.h
 *  */
 
#ifndef __IMSCROLL_H 
#define __IMSCROLL_H

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*iupmatScrollMoveF)(Ihandle*,int,float, int);

int  iupmatScrollCb  (Ihandle *hm,int action, float x, float y);
void iupmatScroll    (Ihandle *h, int dir, int ref_col, int ncol);
void iupmatScrollOpen(Ihandle *h, int lin, int col);

int  iupmatScrollMoveCursor(iupmatScrollMoveF func, Ihandle *h,
                            int modo, float pos, int m);

void iupmatScrollHome        (Ihandle *h,int,float,int);
void iupmatScrollEnd         (Ihandle *h,int,float,int);
void iupmatScrollLeftUp      (Ihandle *h,int,float,int);
void iupmatScrollRightDown   (Ihandle *h,int,float,int);
void iupmatScrollPgLeftUp    (Ihandle *h,int,float,int);
void iupmatScrollPgRightDown (Ihandle *h,int,float,int);
void iupmatScrollPos         (Ihandle *h,int,float,int);
void iupmatScrollCr          (Ihandle *h,int,float,int);

/* Macros usadas no parametro dir de iupmatScrollLine e iupmatScrollColumn */
#define SCROLL_LEFT    0
#define SCROLL_RIGHT   1
#define SCROLL_UP      2
#define SCROLL_DOWN    3

/* Modo usado para "passear" na matriz.
   Indica se o pedido de movimentcao vei da scrollbar ou do teclado.
   Valores possiveis para o parametro modo da funcao iupmatScrollMoveCursor
 */
#define SCROLL    0
#define KEY       1


/* Macros para facilitar a chamada da funcao iupMatScrollMoveCursor */


#define ScrollKeyHome(h)    iupmatScrollMoveCursor(iupmatScrollHome       ,h,KEY,0,0)
#define ScrollKeyEnd(h)     iupmatScrollMoveCursor(iupmatScrollEnd        ,h,KEY,0,0)
#define ScrollKeyPgUp(h)    iupmatScrollMoveCursor(iupmatScrollPgLeftUp   ,h,KEY,0,MAT_LIN)
#define ScrollKeyPgDown(h)  iupmatScrollMoveCursor(iupmatScrollPgRightDown,h,KEY,0,MAT_LIN)
#define ScrollKeyUp(h)      iupmatScrollMoveCursor(iupmatScrollLeftUp     ,h,KEY,0,MAT_LIN)
#define ScrollKeyDown(h)    iupmatScrollMoveCursor(iupmatScrollRightDown  ,h,KEY,0,MAT_LIN)
#define ScrollKeyLeft(h)    iupmatScrollMoveCursor(iupmatScrollLeftUp     ,h,KEY,0,MAT_COL)
#define ScrollKeyRight(h)   iupmatScrollMoveCursor(iupmatScrollRightDown  ,h,KEY,0,MAT_COL)
#define ScrollKeyCr(h)      iupmatScrollMoveCursor(iupmatScrollCr         ,h,KEY,0,MAT_LIN)

#define ScrollPgUp(h)     iupmatScrollMoveCursor(iupmatScrollPgLeftUp   ,h,SCROLL,0,MAT_LIN)
#define ScrollPgDown(h)   iupmatScrollMoveCursor(iupmatScrollPgRightDown,h,SCROLL,0,MAT_LIN)
#define ScrollUp(h)       iupmatScrollMoveCursor(iupmatScrollLeftUp     ,h,SCROLL,0,MAT_LIN)
#define ScrollDown(h)     iupmatScrollMoveCursor(iupmatScrollRightDown  ,h,SCROLL,0,MAT_LIN)
#define ScrollLeft(h)     iupmatScrollMoveCursor(iupmatScrollLeftUp     ,h,SCROLL,0,MAT_COL)
#define ScrollRight(h)    iupmatScrollMoveCursor(iupmatScrollRightDown  ,h,SCROLL,0,MAT_COL)
#define ScrollPgLeft(h)   iupmatScrollMoveCursor(iupmatScrollPgLeftUp   ,h,SCROLL,0,MAT_COL)
#define ScrollPgRight(h)  iupmatScrollMoveCursor(iupmatScrollPgRightDown,h,SCROLL,0,MAT_COL)
#define ScrollPosVer(h,y) iupmatScrollMoveCursor(iupmatScrollPos        ,h,SCROLL,y,MAT_LIN)
#define ScrollPosHor(h,x) iupmatScrollMoveCursor(iupmatScrollPos        ,h,SCROLL,x,MAT_COL)


#ifdef __cplusplus
}
#endif

#endif
