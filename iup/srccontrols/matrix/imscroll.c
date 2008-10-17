/** \file
 * \brief iupmatrix control
 * scrolling
 *
 * See Copyright Notice in iup.h
 * $Id: imscroll.c,v 1.1 2008-10-17 06:05:36 scuri Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "iup.h"
#include "iupcbs.h"
#include "iupmatrix.h"
#include "iupkey.h"

#include <cd.h>
#include <cdiup.h>
#include <cddbuf.h>

#include "iup_object.h"
#include "iup_attrib.h"
#include "iup_str.h"
#include "iup_drv.h"
#include "iup_stdcontrols.h"
#include "iup_controls.h"
#include "iup_cdutil.h"

#include "matrixdef.h"
#include "matrixcd.h"
#include "imdraw.h"
#include "imscroll.h"
#include "imfocus.h"
#include "imaux.h"
#include "imkey.h"
#include "immark.h"
#include "imgetset.h"
#include "imedit.h"


static void iMatrixCb_Scroll         (Ihandle* ih);
static void iMatrixOpenLineColumn    (Ihandle* ih, int m);
static void iMatrixGetFirstLineColumn(Ihandle* ih, int pos, int m);


/**************************************************************************/
/* Private functions                                                      */
/**************************************************************************/

/* Get columns/lines in the left/top side of the matriz until the last
   column/line is fully visible
   -> m : Define the mode of operation: lines or columns [IMATRIX_MAT_LIN|IMATRIX_MAT_COL]
*/
static void iMatrixOpenLineColumn(Ihandle* ih, int m)
{
  int i = 0;
  int old;
  Tlincol* p;

  if(m == IMATRIX_MAT_LIN)
    p = &(ih->data->lin);
  else
    p = &(ih->data->col);

  old = p->last;  /* Store the last visible column/line */

  /* Increment the first column/line until the last is fully visible */
  do
  {
    i++;
    p->first++;
    iMatrixGetLastWidth(ih, m);
  } while(p->last == old &&
          p->last != (p->num - 1) && /* It is not the last column/line */
          p->lastwh != p->wh[p->last]);

  iMatrixGetPos(ih, m);
  if(m == IMATRIX_MAT_COL)
    iMatrixScroll(ih, IMATRIX_SCROLL_RIGHT, old, i);
  else
    iMatrixScroll(ih, IMATRIX_SCROLL_DOWN,  old, i);
}

/* Compute which must be the first visible column/line, so that the
   sum of widths of the invisible columns/lines, in the left/top side, is
   as near as possible the value.
   -> pos: size of the line
   -> m : Define the mode of operation: lines or columns [IMATRIX_MAT_LIN|IMATRIX_MAT_COL]
*/
static void iMatrixGetFirstLineColumn(Ihandle* ih, int pos, int m)
{
  int i;
  Tlincol* p;

  if(m == IMATRIX_MAT_LIN)
    p = &(ih->data->lin);
  else
    p = &(ih->data->col);

  p->pos = 0;
  for(i = 0; i < p->num; i++)
  {
    p->pos += p->wh[i];
    if(p->pos > pos)
     break;
  }
  if((p->pos - pos) > (pos - p->pos + p->wh[i]) || (p->num == i) )
  {
    p->pos -= p->wh[i];
    i--;
  }
  p->first = i + 1;
}

/* Callback to report to the user which visualization area of
   the matrix changed.
*/
static void iMatrixCb_Scroll(Ihandle* ih)
{
  IFnii cb = (IFnii)IupGetCallback(ih, "SCROLLTOP_CB");

  if(cb) 
  {
    cb(ih, ih->data->lin.first+1, ih->data->col.first+1);
  }
}


/**************************************************************************/
/* Exported functions, used to make scroll using get and put image        */
/**************************************************************************/

/* Make the scroll of a number of columns to the left or right side or
   lines on top or down, using the ScrollImage primitive. To the left, just
   move one column - to the right, can move more than one. On top, just
   move only one column - on down, can move more than one.
   -> dir : [IMATRIX_SCROLL_LEFT| IMATRIX_SCROLL_RIGHT | IMATRIX_SCROLL_UP | IMATRIX_SCROLL_DOWN]
   -> ref : First column/line to be redrawn when the move is to right or on down.
   -> num : Number of columns/lines to be moved,  when the move is to right or on down.
*/
void iMatrixScroll(Ihandle* ih, int dir, int ref, int num)
{
  int x1, x2, y1, y2, xt2, yt2;
  int i;

  x1 = 0;
  x2 = ih->data->XmaxC;
  
  y1 = 0;
  y2 = ih->data->YmaxC;

  ih->data->redraw = 1;

  if(dir == IMATRIX_SCROLL_LEFT)
  {
    x1  = ih->data->col.titlewh;      /* skip the line title */
    x2 -= ih->data->col.wh[ih->data->col.first];
    if(x1 >= x2)
    {
     iMatrixDrawMatrix(ih, IMATRIX_DRAW_COL);
     return;
    }

    cdCanvasScrollArea(ih->data->cddbuffer, x1, x2, y1, y2, ih->data->col.wh[ih->data->col.first], 0);

    iMatrixDrawColumnTitle(ih, ih->data->col.first, ih->data->col.first);

    iMatrixSetCdFrameColor(ih);
    cdCanvasLine(ih->data->cddbuffer, ih->data->XmaxC, y1, ih->data->XmaxC, y2);

    iMatrixDrawCells(ih, ih->data->lin.first, ih->data->col.first, ih->data->lin.last, ih->data->col.first);
  }
  else if(dir == IMATRIX_SCROLL_UP)
  {
    y2 -= ih->data->lin.titlewh;      /* skip the column title */
    y1 += ih->data->lin.wh[ih->data->lin.first];
    if(y1 >= y2)
    {
     iMatrixDrawMatrix(ih, IMATRIX_DRAW_LIN);
     return;
    }

    cdCanvasScrollArea(ih->data->cddbuffer, x1, x2, y1, y2, 0, -ih->data->lin.wh[ih->data->lin.first]);
    iMatrixDrawLineTitle(ih, ih->data->lin.first, ih->data->lin.first);
    iMatrixDrawCells(ih, ih->data->lin.first, ih->data->col.first, ih->data->lin.first, ih->data->col.last);
  }
  else if(dir == IMATRIX_SCROLL_RIGHT)
  {
    x1 = ih->data->col.titlewh;      /* skip the line title */
    for(i = 1; i <= num; i++)
      x1 += ih->data->col.wh[ih->data->col.first-i];
    if(x1 >= x2)
    {
     iMatrixDrawMatrix(ih, IMATRIX_DRAW_COL);
     return;
    }

    cdCanvasScrollArea(ih->data->cddbuffer, x1, x2/*-1*/, y1, y2, ih->data->col.titlewh-x1, 0);

    xt2 = iMatrixDrawColumnTitle(ih, ref, ih->data->col.last);

    if(xt2 < x2)          /* clear the right area that doesn't have column */
    {
      iMatrixDrawEmptyArea(ih, xt2, x2, y1, y2);
    }
    else
    {
      iMatrixSetCdFrameColor(ih);
      cdCanvasLine(ih->data->cddbuffer, x2, y1, x2, y2);
    }

    /* draw the cells */
    iMatrixDrawCells(ih, ih->data->lin.first, ref, ih->data->lin.last, ih->data->col.last);
  }
  else if(dir == IMATRIX_SCROLL_DOWN)
  {
    y2 -= ih->data->lin.titlewh;      /* skip the column title */
    for(i = 1; i <= num; i++)
      y2 -= ih->data->lin.wh[ih->data->lin.first-i];
    if(y1 >= y2)
    {
     iMatrixDrawMatrix(ih, IMATRIX_DRAW_LIN);
     return;
    }

    cdCanvasScrollArea(ih->data->cddbuffer, x1, x2/*-1*/, y1, y2, 0, ih->data->YmaxC-ih->data->lin.titlewh-y2);

    yt2 = iMatrixDrawLineTitle(ih, ref, ih->data->lin.last);

    if(yt2 < y1)          /* clear the right area that doesn't have column */
    {
     iMatrixDrawEmptyArea(ih, x1, x2, y1, yt2);
    }
    else
    {
     iMatrixSetCdFrameColor(ih);
     cdCanvasLine(ih->data->cddbuffer, x1, y1, x2, y1);
    }

    /* desenha as celulas */
    iMatrixDrawCells(ih, ref, ih->data->col.first, ih->data->lin.last, ih->data->col.last);
  }
}


/**************************************************************************/
/* Exported functions, used to move into the matrix                       */
/**************************************************************************/

/* Move using the cells of matrix.
   Receive as a parameter a pointer to a function that will make the work,
   in fact. This is done to avoid a test to each of the manipulation cursor
   functions, verifying if it is necessary to call or not the scroll
   callback. This is only done here.
   -> func - pointer to the function that will make the movement
   -> modo - parameter passed to func, specify if the movement request is of
             the scrollbar or the keyboard
   -> pos  - parameter passed to func, that will be the handle position function
             of the scrollbar, returning the scrollbar thumb position...
             if func is other function, this parameter will be ignored
   -> m    - parameter passed to func, specify which is the mode of operation:
             lines or columns [IMATRIX_MAT_LIN|IMATRIX_MAT_COL]
*/
int iMatrixScrollMoveCursor(iMatrixScrollMoveF func, Ihandle* ih, int modo, float pos, int m)
{
  int oldfl  = ih->data->lin.first;
  int oldfc  = ih->data->col.first;
  int oldlin = ih->data->lin.active;
  int oldcol = ih->data->col.active;

  iMatrixEditCheckHidden(ih);

  func(ih, modo, pos, m);

  if(ih->data->lin.first != oldfl || ih->data->col.first != oldfc)
    iMatrixCb_Scroll(ih);

  if(ih->data->lin.active != oldlin || ih->data->col.active != oldcol)
    return 1;

  return 0;
}

/* Open a cell does not show fully, boundaries of the matrix.
   Call the scroll callback, if necessary.
   -> lin, col : cell coordinates that will contain the focus.
*/
void iMatrixScrollOpen(Ihandle* ih, int lin, int col)
{
  int oldfl = ih->data->lin.first;
  int oldfc = ih->data->col.first;

  if(col == ih->data->col.last && ih->data->col.lastwh != ih->data->col.wh[ih->data->col.last])
  {
    iMatrixOpenLineColumn(ih, IMATRIX_MAT_COL);
    SetSbH(ih);
  }
  if(lin == ih->data->lin.last && ih->data->lin.lastwh != ih->data->lin.wh[ih->data->lin.last])
  {
    iMatrixOpenLineColumn(ih, IMATRIX_MAT_LIN);
    SetSbV(ih);
  }

  if(ih->data->lin.first != oldfl || ih->data->col.first != oldfc)
    iMatrixCb_Scroll(ih);
}

/* This function is called when the "home" key is pressed.
   In the first time, go to the beginning of the line.
   In the second time, go to the beginning of the page.
   In the third time, go to the beginning of the matrix.
   -> mode and pos : DO NOT USED.
*/
void iMatrixScrollHome(Ihandle* ih, int mode, float pos, int m)
{
  (void)m;
  (void)mode;
  (void)pos;

  if(iMatrixGetHomeKeyCount() == 0)  /* go to the beginning of the line */
  {
    if(ih->data->col.active != 0)
    {
      iMatrixHideFocus(ih);
      if(ih->data->col.first != 0)
      {
        ih->data->col.first = 0;
        while(ih->data->col.first < ih->data->col.num && ih->data->col.wh[ih->data->col.first] == 0)
          ih->data->col.first++;

        iMatrixGetLastWidth(ih, IMATRIX_MAT_COL);
        iMatrixGetPos(ih, IMATRIX_MAT_COL);
        SetSbH(ih);
        iMatrixDrawMatrix(ih, IMATRIX_DRAW_COL);
      }
      iMatrixSetShowFocus(ih, ih->data->lin.active, ih->data->col.first);
    }
  }
  else if(iMatrixGetHomeKeyCount() == 1)   /* go to the beginning of the page */
  {
    if(ih->data->lin.active != ih->data->lin.first)
    {
      iMatrixHideSetShowFocus(ih, ih->data->lin.first, ih->data->col.first);
    }
  }
  else if(iMatrixGetHomeKeyCount() == 2)   /* go to the beginning of the matrix 1:1 */
  {
    if(ih->data->lin.active != 0)
    {
      iMatrixHideFocus(ih);
      ih->data->lin.first = 0;
      while(ih->data->lin.first < ih->data->lin.num && ih->data->lin.wh[ih->data->lin.first] == 0)
        ih->data->lin.first++;
      iMatrixGetLastWidth(ih, IMATRIX_MAT_LIN);
      iMatrixGetPos(ih, IMATRIX_MAT_LIN);
      SetSbV(ih);
      iMatrixDrawMatrix(ih, IMATRIX_DRAW_LIN);
      iMatrixSetShowFocus(ih, 0, ih->data->col.first);
    }
  }
}

/* This function is called when the "end" key is pressed.
   In the first time, go to the end of the line.
   In the second time, go to the end of the page.
   In the third time, go to the end of the matrix.
   -> mode and pos : DO NOT USED.
*/
void iMatrixScrollEnd(Ihandle* ih, int mode, float pos, int m)
{
  (void)m;
  (void)mode;
  (void)pos;

  if(iMatrixGetEndKeyCount() == 0)  /* go to the end of the line */
  {
    if(ih->data->col.active != ih->data->col.num-1)
    {
      iMatrixHideFocus(ih);
      if(ih->data->col.last != ih->data->col.num-1 ||
         ih->data->col.wh[ih->data->col.last] != ih->data->col.lastwh)
      {
        int i, soma;

        ih->data->col.last = ih->data->col.num-1;
        while(ih->data->col.last > 0 && ih->data->col.wh[ih->data->col.last] == 0)
          ih->data->col.last--;

        ih->data->col.lastwh = ih->data->col.wh[ih->data->col.last];

        for(soma = 0, i = ih->data->col.num-1; i >= 0; i--)
        {
          soma += ih->data->col.wh[i];
          if(soma > ih->data->col.size)
            break;
        }
        ih->data->col.first = i+1;
        if(i >= 0)
          soma -= ih->data->col.wh[i];

        ih->data->col.pos = ih->data->col.totalwh - soma;
        SetSbH(ih);
        iMatrixDrawMatrix(ih, IMATRIX_DRAW_COL);
      }
      iMatrixSetShowFocus(ih, ih->data->lin.active, ih->data->col.last);
    }
  }
  else if(iMatrixGetEndKeyCount() == 1)   /* go to the end of the page */
  {
    if(ih->data->lin.active != ih->data->lin.last ||
       ih->data->lin.wh[ih->data->lin.last] != ih->data->lin.lastwh)
    {
      int old = ih->data->lin.last;
      iMatrixHideFocus(ih);
      if(ih->data->lin.wh[ih->data->lin.last] != ih->data->lin.lastwh)
        iMatrixOpenLineColumn(ih, IMATRIX_MAT_LIN);
      iMatrixSetShowFocus(ih, old, ih->data->col.active);
      SetSbV(ih);
    }
  }
  else if(iMatrixGetEndKeyCount() == 2)   /* go to the end of the matrix */
  {
    if(ih->data->lin.active != ih->data->lin.num-1)
    {
      int i, soma;
      iMatrixHideFocus(ih);

      ih->data->lin.last = ih->data->lin.num-1;
      while(ih->data->lin.last > 0 && ih->data->lin.wh[ih->data->lin.last] == 0)
        ih->data->lin.last--;

      ih->data->lin.lastwh = ih->data->lin.wh[ih->data->lin.last];

      for(soma = 0, i = ih->data->lin.num-1; i >= 0; i--)
      {
        soma += ih->data->lin.wh[i];
        if(soma > ih->data->lin.size)
          break;
      }
      ih->data->lin.first = i+1;
      if(i >= 0)
        soma -= ih->data->lin.wh[i];

      ih->data->lin.pos = ih->data->lin.totalwh - soma;

      SetSbV(ih);
      iMatrixDrawMatrix(ih, IMATRIX_DRAW_LIN);
      iMatrixSetShowFocus(ih, ih->data->lin.num-1, ih->data->col.active);
    }
  }
}

/* This function is called to move a cell to the left or up.
   -> modo : indicate if the command was from the keyboard or the scrollbar. If scrollbar,
             do not change the focus.
   -> pos  : DO NOT USED
   -> m    : define the mode of operation: lines or columns [IMATRIX_MAT_LIN|IMATRIX_MAT_COL]
*/
void iMatrixScrollLeftUp(Ihandle* ih, int modo, float pos, int m)
{
  int nextfirst, nextactive;
  Tlincol* p;
  (void)pos;

  if(m == IMATRIX_MAT_LIN)
    p = &(ih->data->lin);
  else
    p = &(ih->data->col);

  nextfirst = p->first - 1;
  while(nextfirst >= 0 && p->wh[nextfirst] == 0)
    nextfirst--;

  nextactive = p->active - 1;
  while(nextactive >=0 && p->wh[nextactive] == 0)
    nextactive--;

  /* if the command was from the scrollbar or if the focus is on the first column/line */
  if(modo == SCROLL || nextactive < p->first)
  {
    int old = p->first;

    if(nextfirst < 0)
      return;

    iMatrixHideFocus(ih);

    p->first = nextfirst;
    iMatrixGetLastWidth(ih, m);
    iMatrixGetPos(ih, m);

    if(m == IMATRIX_MAT_COL)
      iMatrixScroll(ih, IMATRIX_SCROLL_LEFT, old, old-nextfirst);
    else
      iMatrixScroll(ih, IMATRIX_SCROLL_UP, old, old-nextfirst);

    SetSb(ih, m);

    if(m == IMATRIX_MAT_COL)
      iMatrixSetShowFocus(ih, ih->data->lin.active, modo==SCROLL ? ih->data->col.active : nextfirst);
    else
      iMatrixSetShowFocus(ih, modo == SCROLL ? ih->data->lin.active : nextfirst, ih->data->col.active);
  }
  else
  {
    if(nextactive < 0)
      return;
    if(m == IMATRIX_MAT_COL)
      iMatrixHideSetShowFocus(ih, ih->data->lin.active, nextactive);
    else
      iMatrixHideSetShowFocus(ih, nextactive, ih->data->col.active);
  }
}

/* This function is called to move a cell to the right or down.
   -> modo : indicate if the command from the keyboard or the scrollbar. If scrollbar,
             do not change the focus.
   -> pos  : DO NOT USED
   -> m    : define the mode of operation: lines or columns [IMATRIX_MAT_LIN|IMATRIX_MAT_COL]
*/
void iMatrixScrollRightDown(Ihandle* ih, int modo, float pos, int m)
{
  int old;
  int next;
  Tlincol* p;
  (void)pos;

  if(m == IMATRIX_MAT_LIN)
    p = &(ih->data->lin);
  else
    p = &(ih->data->col);

  old = p->last;

  if(modo == KEY)
  {
    int abre = 0;
    /* In the last line/column, but not fully visible.
       Open, but keeps in the same cell.
    */
    if(p->active == p->last && p->lastwh != p->wh[p->last])
    {
      next = p->last;
      abre = 1;
    }
    else
    {
      next = p->active+1;
      while(next < p->num && p->wh[next]==0)
        next++;

      if(next >= p->num)
        return;

      if((next > p->last) || (next == p->last && p->lastwh != p->wh[p->last]))
        abre = 1;
    }

    /* If it is necessary to open some line/column */
    if(abre)
    {
      iMatrixHideFocus(ih);
      iMatrixOpenLineColumn(ih, m);
      SetSb(ih, m);
      if(m == IMATRIX_MAT_COL)
        iMatrixSetShowFocus(ih, ih->data->lin.active, next);
      else
        iMatrixSetShowFocus(ih, next, ih->data->col.active);
    }
    else
    {
      if(m == IMATRIX_MAT_COL)
        iMatrixHideSetShowFocus(ih, ih->data->lin.active, next);
      else
        iMatrixHideSetShowFocus(ih, next, ih->data->col.active);
    }
  }
  else  /* SCROLL */
  {
    int oldf = p->first;

    /* If the last column/line of the matrix already is fully visible,
       do not have anything to be done... return
    */
    if((p->lastwh == p->wh[p->last]) &&
       (p->last == (p->num - 1)))
      return;

    p->first++;
    while(p->first < p->num && p->wh[p->first] == 0)
      p->first++;

    if(p->first >= p->num)
      return;

    iMatrixGetLastWidth(ih, m);
    iMatrixGetPos(ih, m);

    iMatrixHideFocus(ih);
    if(m == IMATRIX_MAT_COL)
      iMatrixScroll(ih, IMATRIX_SCROLL_RIGHT, old, p->first-oldf);
    else
      iMatrixScroll(ih, IMATRIX_SCROLL_DOWN,  old, p->first-oldf);

    SetSb(ih, m);

    iMatrixShowFocus(ih);
  }
}

/* This function is called to move a page to the left or up.
   -> modo : indicate if the command was from the keyboard or the scrollbar. If scrollbar,
             do not change the focus.
   -> pos  : DO NOT USED
   -> m    : define the mode of operation: lines (PgLeft) or columns (PgUp) [IMATRIX_MAT_LIN|IMATRIX_MAT_COL]
*/
void iMatrixScrollPgLeftUp(Ihandle* ih, int mode, float pos, int m)
{
  int soma;
  Tlincol* p;
  int old;
  (void)pos;

  if(m == IMATRIX_MAT_LIN)
    p = &(ih->data->lin);
  else
    p = &(ih->data->col);

  if(p->first <= 0)
  {
    /* If it is the first page of the matrix, and the command was from the keyboard,
       then change the focus to the start of page.
    */
    if(mode == KEY)
      iMatrixHideSetShowFocus(ih, 0, ih->data->col.active);
    return;
  }

  old  = p->first;
  soma = p->size;

  /* Search to define the first column/line */
  while((soma > 0) && (p->first!=0))
  {
   soma -= p->wh[p->first-1];
   if(soma >= 0)
     p->first--;
  }

  iMatrixGetLastWidth(ih, m);
  iMatrixGetPos(ih, m);

  iMatrixHideFocus(ih);
  SetSb(ih, m);
  iMatrixDrawMatrix(ih, m);

  if(mode == KEY)
  {
    /* If mode == KEY, m == IMATRIX_MAT_LIN, and there is not PgLeft
       on the key
    */
    int newl = ih->data->lin.active + (p->first - old);
    if(newl <= 0)
      newl = 0;
    iMatrixSetShowFocus(ih, newl, ih->data->col.active);
  }
  else
    iMatrixSetShowFocus(ih, ih->data->lin.active, ih->data->col.active);
}

/* This function is called to move a page to the right or down.
   -> modo : indicate if the command was from the keyboard or the scrollbar. If scrollbar,
             do not change the focus.
   -> pos  : DO NOT USED
   -> m    : define the mode of operation: lines (PgDown) or columns (PgRight) [IMATRIX_MAT_LIN|IMATRIX_MAT_COL]
*/
void iMatrixScrollPgRightDown(Ihandle* ih, int mode, float pos, int m)
{
  Tlincol* p;
  int old;
  (void)pos;

  if(m == IMATRIX_MAT_LIN)
    p = &(ih->data->lin);
  else
    p = &(ih->data->col);

  /* Test hide columns in the right/down side*/
  if((p->last >= p->num) ||
     ((p->last==(p->num-1)) && (p->lastwh == p->wh[p->last])))
  {
    /* If it is the last page of the matrix, and it is fully visible,
       and the command was from the keyboard,
       then change the focus to the last line.
    */
    if(mode == KEY)
      iMatrixHideSetShowFocus(ih, p->num-1, ih->data->col.active);
    return;
  }

  iMatrixHideFocus(ih);

  old = p->first;
  /* Next first column = old last visible.
     If the last column have been show completely (rare), then plus
     one to ih->data->col.first. Same thing if we will be working with lines.
  */
  p->first = p->last;
  if(p->wh[p->last] == p->lastwh)
    p->first++;
  iMatrixGetLastWidth(ih, m);
  iMatrixGetPos(ih, m);
  SetSb(ih, m);
  iMatrixDrawMatrix(ih, m);

  if(mode == KEY)
  {
    /* If mode == KEY, m == IMATRIX_MAT_LIN, and there is not PgRight
       on the key
    */
    int newl = ih->data->lin.active + (p->first - old);
    if(newl > p->num-1)
      newl = p->num-1;
    iMatrixSetShowFocus(ih, newl, ih->data->col.active);
  }
  else
    iMatrixSetShowFocus(ih, ih->data->lin.active, ih->data->col.active);
}

void iMatrixScrollCr(Ihandle* ih, int mode, float pos, int m)
{
  int redraw = 0;
  int lin = 0, col = 0; /* random initialization */
  int width;
  int oldlin = ih->data->lin.active;
  int oldcol = ih->data->col.active;

  iMatrixScrollRightDown(ih, mode, pos, m);

  if(ih->data->lin.active == oldlin && ih->data->col.active == oldcol)
  {
    /* Do not change the focus, because it was in the last line of the column.
       Go to the same line of the next column, if it exists.
    */
    col = ih->data->col.active + 1;
    while(col <= ih->data->col.num-1 && ih->data->col.wh[col]==0)
      col++;
    if(col <= ih->data->col.num-1)
    {
      if(col > ih->data->col.last || (col == ih->data->col.last && ih->data->col.wh[col] != ih->data->col.lastwh))
      {
        width = ih->data->col.wh[col];
        while(width > 0)
        {
          width -= ih->data->col.wh[ih->data->col.first];
          ih->data->col.first++;
        }
        iMatrixGetLastWidth(ih, IMATRIX_MAT_COL);
        iMatrixGetPos(ih, IMATRIX_MAT_COL);
        redraw = 1;
      }

      lin = ih->data->lin.active;
      while(lin <= ih->data->lin.num-1 && ih->data->lin.wh[lin] == 0)
        lin++;
      if(ih->data->lin.first != lin)
      {
        ih->data->lin.first = lin;
        iMatrixGetLastWidth(ih, IMATRIX_MAT_LIN);
        iMatrixGetPos(ih, IMATRIX_MAT_LIN);
        redraw = 1;
      }

      iMatrixHideFocus(ih);
      iMatrixSetFocusPos(ih, lin, col);
      if(redraw)
      {
        SetSbV(ih);
        SetSbH(ih);
        iMatrixDrawMatrix(ih, IMATRIX_DRAW_ALL);
      }
    }
  }
}

/* This function is called when a drag is performed in the scrollbar.
   -> x    : scrollbar thumb position, value between 0 and 1
   -> mode : DO NOT USED
   -> m    : define the mode of operation: lines or columns [IMATRIX_MAT_LIN|IMATRIX_MAT_COL]
*/
void iMatrixScrollPos(Ihandle* ih, int mode, float x, int m)
{
  int   pos;
  float dx;
  Tlincol* p;
  (void)mode;

  if(m == IMATRIX_MAT_LIN)
  {
    p = &(ih->data->lin);
    dx = IupGetFloat(ih, "DY");
  }
  else
  {
    p = &(ih->data->col);
    dx = IupGetFloat(ih, "DX");
  }

  pos = (int)(x * p->totalwh + 0.5);

  iMatrixHideFocus(ih);
  iMatrixGetFirstLineColumn(ih, pos, m);
  iMatrixGetLastWidth(ih, m);

  /* The count executed by iMatrixGetFirstLineColumn function finds the column
     with the position most closely of the scrollbar. But, when the scrollbar is
     nearly in the end of rigth side, this count can not solve the problem,
     because the last column will not be fully visible...
     So, the following code identifies that the scrollbar is nearly in the
     right side, plus its position with its size, and compare with 1.0
     (tolerance of 0.0005).
  */
  if((x + dx >= 0.9995) && (p->last   != (p->num-1) ||
                            p->lastwh != p->wh[p->num-1]))
  {
    /* Increment the first col until that the last is fully visible */
    do
    {
      p->first++;
      iMatrixGetLastWidth(ih, m);
    } while(p->lastwh != p->wh[p->num-1] ||
            p->last   != (p->num-1));

    iMatrixGetPos(ih, m);
  }

  SetSb(ih, m);
  iMatrixDrawMatrix(ih, m);
  iMatrixSetShowFocus(ih, ih->data->lin.active, ih->data->col.active);
}

/* This callback is called when some action is performed on the
   scrollbar.
   -> action : type of action that call the event.
   -> x,y    : scrollbar thumb positions, value between 0 and 1
*/
int iMatrixScrollCB(Ihandle* ih, int action, float x, float y)
{
  int err;

  x = IupGetFloat(ih, "POSX");
  y = IupGetFloat(ih, "POSY");

  IsCanvasSet(ih, err);
  if(err == CD_OK)
  {
    switch(action)
    {
      case IUP_SBUP      : ScrollUp(ih);       break;
      case IUP_SBDN      : ScrollDown(ih);     break;
      case IUP_SBPGUP    : ScrollPgUp(ih);     break;
      case IUP_SBPGDN    : ScrollPgDown(ih);   break;
      case IUP_SBRIGHT   : ScrollRight(ih);    break;
      case IUP_SBLEFT    : ScrollLeft(ih);     break;
      case IUP_SBPGRIGHT : ScrollPgRight(ih);  break;
      case IUP_SBPGLEFT  : ScrollPgLeft(ih);   break;
      case IUP_SBPOSV    : ScrollPosVer(ih,y); break;
      case IUP_SBPOSH    : ScrollPosHor(ih,x); break;
      case IUP_SBDRAGV   : ScrollPosVer(ih,y); break;
      case IUP_SBDRAGH   : ScrollPosHor(ih,x); break;
    }
  }

  {
    cdCanvasFlush(ih->data->cddbuffer);
    ih->data->redraw = 0;
  } /* always update */

  return IUP_DEFAULT;
}
