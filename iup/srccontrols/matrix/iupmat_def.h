/** \file
 * \brief iupmatrix. definitions.
 *
 * See Copyright Notice in iup.h
 */
 
#ifndef __IUPMAT_DEF_H 
#define __IUPMAT_DEF_H

#include "iupmat_draw.h"

#ifdef __cplusplus
extern "C" {
#endif


/***************************************************************************/
/* Possible state transitions for a cell                                   */
/***************************************************************************/
#define IMAT_EDIT    1
#define IMAT_NOEDIT  0

#define IMAT_MAT_LIN   IMAT_DRAW_LIN
#define IMAT_MAT_COL   IMAT_DRAW_COL


/***************************************************************************/
/* Decoration size in pixels                                               */
/***************************************************************************/
#define IMAT_DECOR_X   6

#ifdef WIN32
#define IMAT_DECOR_Y   6
#else
#define IMAT_DECOR_Y  10
#endif


/***************************************************************************/
/* Macros to help the access for line and column title attributes          */
/***************************************************************************/
#define IMAT_TITLE_LIN   "%d:0"
#define IMAT_TITLE_COL   "0:%d"


/***************************************************************************/
/* Structures stored in each matrix                                        */
/***************************************************************************/
typedef struct _ImatCell
{
  char *value;         /* Cell value                              */
  int nba;             /* Number of bytes allocated to this value */
  unsigned char mark;  /* Is this cell marked?                    */
} ImatCell;

typedef struct _ImatLinColData
{
  int *wh;         /* Width/height of the columns/lines     */
  int lastwh;      /* Width/height of the last column/line  */
  int titlewh;     /* Width/height of the column/line title */

  char *marked;    /* Shows whether the columns/lines are or not marked   */
  char *inactive;  /* Shows whether the columns/lines are or not inactive */

  int num;         /* Number of columns/lines in the matrix */
  int numaloc;     /* Number of columns/lines allocated */

  int first;       /* First visible column/line */
  int last;        /* Last visible column/line  */

  int totalwh;     /* Sum of the widths/heights of the columns/lines */
  int pos;         /* Sum of the widths/heights of the invisible part left/ above */
  int size;        /* Width/height of the visible window */

  int active;      /* Width/height of the active cell */
} ImatLinColData;

struct _IcontrolData
{
  iupCanvas canvas;  /* from IupCanvas (must reserve it) */

  ImatCell** v;              /* Cell value */

  sIFnii  valcb;       /* Callback to get a value of a cell */
  IFniis  valeditcb;
  IFnii   markcb;
  IFniii  markeditcb;

  Ihandle* texth;     /* Text handle                    */
  Ihandle* droph;     /* Dropdown handle                */
  Ihandle* datah;     /* Current active element edition */
  /* may be equal to texth or droph */

  cdCanvas* cddbuffer;
  cdCanvas* cdcanvas;
  int redraw;
  int checkframecolor;

  ImatLinColData lin;
  ImatLinColData col;

  short int hasiupfocus;  /* IUP focus? */

  int MarkLinCol;         /* Is it marking lines or columns?     */

  char sb_posicaox[10], sb_tamanhox[10];  /* used to store values */
  char sb_posicaoy[10], sb_tamanhoy[10];  /* passed to the IUP    */

  int YmaxC;  /* Current size canvas variables */
  int XmaxC;  /* set when a callback is called */
};

/* Prototypes to set the scrollbar */
void iupMatrixSetSbH(Ihandle*);
void iupMatrixSetSbV(Ihandle*);
void iupMatrixSetSb(Ihandle*, int);


#ifdef __cplusplus
}
#endif

#endif
