/** \file
 * \brief iupmatrix control
 * cell selection
 *
 * See Copyright Notice in iup.h
 * $Id: immark.c,v 1.1 2008-10-17 06:05:36 scuri Exp $
 */

/**************************************************************************/
/*  Functions to cell selection (mark)
/**************************************************************************/

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
#include "imaux.h"
#include "immark.h"


#define IMATRIX_MINMAX(a, b, min, max)  ((a > b) ? (min = b, max = a) : (min = a, max = b))


/* MarkLin and MarkCol are used to store the current cell when a block is being selected */
static int MarkLin, MarkCol;

/* FullLin and FullCol indicate if full lines or columns is being selected */
static int MarkFullLin = 0,
           MarkFullCol = 0,
           LastMarkFullCol = 0,
           LastMarkFullLin = 0;

/* MarkedCells indicates the number of selected cells */
static int MarkedCells = 0;


/* Mark (select) a full column. Redraw it.
   -> col : column to be mark.
   -> redraw: define the redraw or not
*/
static void iMatrixMarkColumn(Ihandle* ih, int col, int redraw)
{
  int i;

  for(i = 0; i < ih->data->lin.num; i++, MarkedCells++)
    iMatrixMarkCellSet(ih, i, col, 1);

  ih->data->col.marked[col]++;

  if(redraw)
  {
    if(ih->data->col.marked[col] == 1)
      iMatrixDrawColumnTitle(ih, col, col);
    iMatrixDrawCells(ih, ih->data->lin.first, col, ih->data->lin.last, col);
  }
}

/* Uncheck (unselect) a full column. Redraw it.
   -> col : column to be uncheck.
   -> redraw: define the redraw or not
*/
static void iMatrixMarkUncheckColumn(Ihandle* ih, int col, int redraw)
{
  int i;

  for(i = 0; i < ih->data->lin.num; i++, MarkedCells--)
    iMatrixMarkCellSet(ih, i, col, 0);

  ih->data->col.marked[col]--;

  if(redraw)
  {
    if(!ih->data->col.marked[col])
      iMatrixDrawColumnTitle(ih, col, col);
    iMatrixDrawCells(ih, ih->data->lin.first, col, ih->data->lin.last, col);
  }
}

/* Mark (select) a full line. Redraw it.
   -> lin : line to be mark.
   -> redraw: define the redraw or not
*/
static void iMatrixMarkLine(Ihandle* ih, int lin, int redraw)
{
  int i;

  for(i = 0; i < ih->data->col.num; i++, MarkedCells++)
    iMatrixMarkCellSet(ih, lin, i, 1);

  ih->data->lin.marked[lin]++;

  if(redraw)
  {
    if(ih->data->lin.marked[lin] == 1)
      iMatrixDrawLineTitle(ih, lin, lin);
    iMatrixDrawCells(ih, lin, ih->data->col.first, lin, ih->data->col.last);
  }
}

/* Uncheck (unselect) a full line. Redraw it.
   -> lin : line to be uncheck.
   -> redraw: define the redraw or not
*/

static void iMatrixMarkUncheckLine(Ihandle* ih, int lin, int redraw)
{
  int i;

  for(i = 0; i < ih->data->col.num; i++, MarkedCells--)
    iMatrixMarkCellSet(ih, lin, i, 0);
  ih->data->lin.marked[lin]--;

  if(redraw)
  {
    if(!ih->data->lin.marked[lin])
      iMatrixDrawLineTitle(ih, lin, lin);
    iMatrixDrawCells(ih, lin, ih->data->col.first, lin, ih->data->col.last);
  }
}

/* Mark or unmark a region of lines (using drag).
   -> lin: final line of selection.
*/
static void iMatrixMarkRegionLin(Ihandle* ih, int lin)
{
  if(lin == MarkLin)
    return;

  if((lin >= ih->data->lin.active && lin < MarkLin) ||
     (lin <= ih->data->lin.active && lin > MarkLin))     /* Unmark */
    iMatrixMarkUncheckLine(ih, MarkLin, 1);
  else
    iMatrixMarkLine(ih, lin, 1);

  MarkLin = lin;
  MarkCol = ih->data->col.num - 1;
}

/* Mark or unmark a region of columns (using drag).
   -> col: final column of selection.
*/
static void iMatrixMarkRegionCol(Ihandle* ih, int col)
{
  if(col == MarkCol)
    return;

  if((col >= ih->data->col.active && col < MarkCol) ||
     (col <= ih->data->col.active && col > MarkCol))     /* Unmark */
    iMatrixMarkUncheckColumn(ih, MarkCol, 1);
  else /* Marcando */
    iMatrixMarkColumn(ih, col, 1);

  MarkCol = col;
  MarkLin = ih->data->lin.num - 1;
}

/* Mark or unmark a region of the matrix (using drag).
   The block to be selected is defined by the cell passed as parameter,
   and MarkLin and MarkCol.
   -> lin, col : final cell of the selection.
*/
static void iMatrixMarkRegion(Ihandle* ih, int lin, int col)
{
  int RedrawLin = -1, RedrawCol = -1;
  int colstart  =  0, linstart  =  0;
  int colend    =  0, linend    =  0;
  int i;

  if(MarkFullCol)  /* Mark/unmark full columns */
  {
    iMatrixMarkRegionCol(ih, col);
    return;
  }

  if(MarkFullLin)  /* Mark/unmark full lines */
  {
    iMatrixMarkRegionLin(ih, lin);
    return;
  }

  if(lin != MarkLin)
  {
    /* Put the "<" of "ih->data->col.active" and "MarkCol" in "colstart", and the other in "colend" */
    IMATRIX_MINMAX(ih->data->col.active, MarkCol, colstart, colend);

    if((lin >= ih->data->lin.active && lin < MarkLin) ||
       (lin <= ih->data->lin.active && lin > MarkLin))     /* Unmark */
    {
       /* lin and MarkLin differ, in max, of 1 */

       /* Decrement the number of times that the cell was marked... */
       /* So, cells of other region, already marked and being superpose by */
       /* current region, are not erased */
       for(i = colstart; i <= colend; i++)
         iMatrixMarkCellSet(ih, MarkLin, i, 0);

       MarkedCells -= (colend - colstart + 1);
       RedrawLin    = MarkLin;
    }
    else   /* Mark */
    {
       /* lin and MarkLin differ, in max, of 1 */
       for(i = colstart; i <= colend; i++)
         iMatrixMarkCellSet(ih, lin, i, 1);

       MarkedCells += (colend - colstart + 1);
       RedrawLin    = lin;
    }
    MarkLin = lin;
  }

  if(col != MarkCol)
  {
    /* Put the "<" of "ih->data->lin.active" and "MarkLin" in "linstart", and the other in "linend" */
    IMATRIX_MINMAX(ih->data->lin.active, MarkLin, linstart, linend);

    if((col >= ih->data->col.active && col < MarkCol) ||
       (col <= ih->data->col.active && col > MarkCol))     /* Unmark */
    {
       /* col and MarkCol differ, in max, of 1 */
       for(i = linstart; i <= linend; i++)
         iMatrixMarkCellSet(ih, i, MarkCol, 0);

       MarkedCells -= (linend - linstart + 1);
       RedrawCol    = MarkCol;
    }
    else   /* Mark */
    {
       /* col and MarkCol differ, in max, of 1 */
       for(i = linstart; i <= linend; i++)
         iMatrixMarkCellSet(ih, i, col, 1);

       MarkedCells += (linend - linstart + 1);
       RedrawCol    = col;
    }
    MarkCol = col;
  }

  if(RedrawLin != -1)
    iMatrixDrawCells(ih, RedrawLin, colstart, RedrawLin, colend);
  if(RedrawCol != -1)
    iMatrixDrawCells(ih, linstart, RedrawCol, linend, RedrawCol);
}

/* This function is called when the many cells is being selected.
   Make the scroll of the spreadsheet if the draw get out of the visualization area.
   -> x, y : mouse coordinates (canvas coordinates).
*/
void iMatrixMarkDrag(Ihandle* ih, int x, int y)
{
  int lin, col, i, j, incl, incc;
  int err;
  int mark_mode = iMatrixMarkGetMode(ih);

  if(mark_mode == IMATRIX_MARK_NO)
    return;

  /* Get the cursor coordinate (lin,col) */
  if(!iMatrixGetLineCol(ih, x, y, &lin, &col))
    return;

  /* There is no drag inside the same cell... */
  if(lin == ih->data->lin.active && col == ih->data->col.active)
    return;

  /* Critics according to the mark mode */
  if(mark_mode == IMATRIX_MARK_LIN)
  {
    if(col != -1)
      return;
    if(!ih->data->lin.marked[ih->data->lin.active])
      return;
  }
  else if(mark_mode == IMATRIX_MARK_COL)
  {
    if(lin != -1)
      return;
    if(!ih->data->col.marked[ih->data->col.active])
      return;
  }
  else if(mark_mode == IMATRIX_MARK_LINCOL)
  {
    if((lin != -1) && (col != -1))
      return;
    if((lin == -1) && (col == -1))
    {
      if((!ih->data->col.marked[ih->data->col.active]) && (!ih->data->lin.marked[ih->data->lin.active]))
        return;
    }
    else
    {
      if((lin == -1) && (!ih->data->col.marked[ih->data->col.active]))
        return;
      if((col == -1) && (!ih->data->lin.marked[ih->data->lin.active]))
        return;
    }
  }

  IsCanvasSet(ih, err);

  if(col == -2)      /* Vertical scrollbar area */
  {
    if(!MarkFullLin)
      ScrollRight(ih);

    col = ih->data->col.last;
  }
  else if(col == -1) /* Column of the line titles */
  {
    if(!MarkFullLin)
      ScrollLeft(ih);

    col = ih->data->col.first;
  }

  if(lin == -2)      /* Horizontal scrollbar area */
  {
    if(!MarkFullCol)
      ScrollDown(ih);

    lin = ih->data->lin.last;
  }
  else if(lin == -1) /* Line of the column titles */
  {
    if(!MarkFullCol)
      ScrollUp(ih);

    lin = ih->data->lin.first;
  }

  incl = (lin > MarkLin) ? 1 : -1;
  incc = (col > MarkCol) ? 1 : -1;

  /* Call iMatrixMarkRegion more than once, if the mouse was "thrown",
     and no event has been generated in a line/column */
  for(i = MarkLin, j = MarkCol; i != lin || j != col;)
  {
    if(i != lin) i += incl;
    if(j != col) j += incc;
    iMatrixMarkRegion(ih, i, j);
  }

  if(ih->data->redraw)
  {
    cdCanvasFlush(ih->data->cddbuffer);
    ih->data->redraw = 0;
  }
}


/**************************************************************************
***************************************************************************
*
*   Funcoes para Marcar desmarcar blocos de celulas
*
***************************************************************************
***************************************************************************/

/* Uncheck (unselect) the last block marked. The block limits are defined
   by the cell that contains the focus and by the cell stored in MarkLin
   and MarkCol. DO NOT redraw the matrix.
*/
void iMatrixMarkUncheckBlock(Ihandle* ih)
{
  int i, j, inc;

  if(!MarkedCells)
    return;

  if(LastMarkFullLin)
  {
    inc = ih->data->lin.active < MarkLin ? 1 : -1;
    for(i = ih->data->lin.active;  ; i += inc)
    {
      iMatrixMarkUncheckLine(ih, i, 0);
      if(i == MarkLin)
        break;
    }
  }
  else if(LastMarkFullCol)
  {
    inc = ih->data->col.active < MarkCol ? 1 : -1;
    for(i = ih->data->col.active;  ; i += inc)
    {
      iMatrixMarkUncheckColumn(ih, i, 0);
      if(i == MarkCol)
        break;
    }
  }
  else
  {
    int inclin = (ih->data->lin.active < MarkLin) ? 1 : -1;
    int inccol = (ih->data->col.active < MarkCol) ? 1 : -1;

    for(i = ih->data->lin.active;  ; i += inclin)
    {
      for(j = ih->data->col.active;  ; j += inccol)
      {
        iMatrixMarkCellSet(ih, i, j, 0);
        MarkedCells--;
        if(j == MarkCol)
          break;
      }
      if(i == MarkLin)
        break;
    }
  }
}

/* Mark a block of cells. If the cell already is marked, increments its counter
   (when two selected areas are overlap. Starts of the cell that contain the
   focus until the cell passed as parameter. Update MarkLin and MarkCol.
   DO NOT redraw the matrix.
   -> lin, col : cell coordinates that define the block to be marked.
*/
void iMatrixMarkBlock(Ihandle* ih, int lin, int col)
{
  int i, j, inc;

  if(iMatrixMarkFullLin())
  {
    inc = ih->data->lin.active < lin ? 1 : -1;
    for(i = ih->data->lin.active;  ; i += inc)
    {
      iMatrixMarkLine(ih, i, 0);
      if(i == lin)
        break;
    }
    MarkLin = lin;
  }
  else if(iMatrixMarkFullCol())
  {
    inc = ih->data->col.active < col ? 1 : -1;
    for(i = ih->data->col.active;  ; i += inc)
    {
      iMatrixMarkColumn(ih, i, 0);
      if(i == col)
        break;
    }
    MarkCol = col;
  }
  else
  {
    int inclin = (ih->data->lin.active < lin) ? 1 : -1;
    int inccol = (ih->data->col.active < col) ? 1 : -1;

    for(i = ih->data->lin.active;  ; i += inclin)
    {
      for(j = ih->data->col.active;  ; j += inccol)
      {
        iMatrixMarkCellSet(ih, i, j, 1);
        MarkedCells++;
        if(j == col)
          break;
      }
      if(i == lin)
        break;
    }
    MarkLin = lin;
    MarkCol = col;
  }
}

void iMatrixMarkCellSet(Ihandle* ih, int lin, int col, int mark)
{
  if(!ih->data->valcb)
    ih->data->v[lin][col].mark = mark;
  else if(ih->data->markeditcb)
    ih->data->markeditcb(ih, lin + 1, col + 1, mark);
  else
  {
    char* str = iupStrGetMemory(100);

    sprintf(str, "MARK%d:%d", lin, col);
    if(mark)
      IupSetAttribute(ih, str, "1");
    else
      IupSetAttribute(ih, str, NULL);
  }
}

int iMatrixMarkCellGet(Ihandle* ih, int lin, int col)
{
  if(!ih->data->valcb)
    return ih->data->v[lin][col].mark;
  else
  {
    if(ih->data->markcb)
      return ih->data->markcb(ih, lin + 1, col + 1);
    else
    {
      char* str = iupStrGetMemory(100);

      sprintf(str, "MARK%d:%d", lin, col);
      return iupAttribGetInt(ih, str);
    }
  }
}

/* Unmark (uncheck) all the marked cells.
   Redraw the cells of this region.
*/
static void iMatrixMarkUncheckAll(Ihandle* ih)
{
  int i, j, old;
  int min_col = ih->data->col.num + 1,
      max_col = -1,
      min_lin = ih->data->lin.num + 1,
      max_lin = -1;

  for(i = 0; i < ih->data->lin.num; i++)
  {
    for(j = 0; j < ih->data->col.num; j++)
    {
      old = iMatrixMarkCellGet(ih, i, j);
      iMatrixMarkCellSet(ih, i, j, 0);
      if(old)
      {
        if(i < min_lin)
          min_lin = i;
        if(i > max_lin)
          max_lin = i;
        if(j < min_col)
          min_col = j;
        if(j > max_col)
          max_col = j;
      }
    }
    old = ih->data->lin.marked[i];
    ih->data->lin.marked[i] = 0;
    if(old)
      iMatrixDrawLineTitle(ih, i, i);
  }
  MarkedCells = 0;
  for(j = 0; j < ih->data->col.num; j++)
  {
    old = ih->data->col.marked[j];
    ih->data->col.marked[j] = 0;
    if(old)
      iMatrixDrawColumnTitle(ih, j, j);
  }

  /* If it is necessary to redraw some cell, the min_lin, max_lin, 
     min_col e max_col values were changed, but only one of them must test
  */
  if(max_lin != -1)
    iMatrixDrawCells(ih, min_lin, min_col, max_lin, max_col);
}


/**************************************************************************/
/* Consult functions                                                      */
/**************************************************************************/

int iMatrixMarkFullLin(void)
{
  return MarkFullLin;
}

int iMatrixMarkFullCol(void)
{
  return MarkFullCol;
}

void iMatrixMarkReset(void)
{
  LastMarkFullLin = MarkFullLin;
  LastMarkFullCol = MarkFullCol;

  MarkFullLin = 0;
  MarkFullCol = 0;
}

/* Verify if the column is or not marked.
   -> col - number of column. Note that the number is NOT presented in the common way.
      0 represents the column of titles and 1 the first column of the matrix.
*/
int iMatrixMarkColumnMarked(Ihandle* ih, int col)
{
  if(col == 0)  /* Line titles are never marked... */
    return 0;

  return ih->data->col.marked[col-1];
}

/* Verify if the line is or not marked.
   -> lin - number of line. Note that the number is NOT presented in the common way.
      0 represents the line of titles and 1 the first line of the matrix.
*/
int iMatrixMarkLineMarked (Ihandle* ih, int lin)
{
  if(lin == 0) /* Column titles are never marked... */
    return 0;

  return ih->data->lin.marked[lin-1];
}

/* Return the mark mode set by user. Consults "IMATRIX_MARK_MODE" attribute and
   returns a constant showing the chosen mode.
*/
int iMatrixMarkGetMode(Ihandle* ih)
{
  char* mode = iupAttribGetStr(ih, "MARK_MODE");

  if (mode && *mode)
  {
    if(iupStrEqualNoCase(mode, "CELL"))
      return IMATRIX_MARK_CELL;
    if(iupStrEqualNoCase(mode, "LIN"))
      return IMATRIX_MARK_LIN;
    if(iupStrEqualNoCase(mode, "COL"))
      return IMATRIX_MARK_COL;
    if(iupStrEqualNoCase(mode, "LINCOL"))
      return IMATRIX_MARK_LINCOL;
  }
  return IMATRIX_MARK_NO;
}

int iMatrixMarkCritica(Ihandle* ih, int* lin, int* col, int* shift, int* ctrl, int* duplo)
{
  int  mark_mode = iMatrixMarkGetMode(ih);
  int  mult = !iupAttribGetInt(ih, "MULTIPLE") ? 0 : 1;
  int  cont = (iupStrEqualNoCase(iupAttribGetStrDefault(ih, "AREA"), "CONTINUOUS")) ? 1 : 0;

  /* If it was pointing for a column title... */
  if(*lin == -1)
  {
    *lin   = ih->data->lin.first;
    *duplo = 0; /* Garante que nao vai interpretar como um double-click */
    if(mark_mode != IMATRIX_MARK_NO && ((mark_mode == IMATRIX_MARK_CELL && mult) || 
                                         mark_mode == IMATRIX_MARK_COL || 
                                         mark_mode == IMATRIX_MARK_LINCOL))
      MarkFullCol = 1;
  }
  /* If it was pointing for a line title... */
  else if(*col == -1)
  {
    *col   = ih->data->col.first;
    *duplo = 0;  /* Ensures that will not interpret as a double-click */
    if(mark_mode != IMATRIX_MARK_NO && ((mark_mode == IMATRIX_MARK_CELL && mult) || 
                                         mark_mode == IMATRIX_MARK_LIN || 
                                         mark_mode == IMATRIX_MARK_LINCOL))
      MarkFullLin = 1;
  }

  /* Verify if an line or column title was clicked (selection of full lines or columns)
     and that full lines and columns were not marked
  */
  if(*shift)
  {
    if(MarkFullLin && !LastMarkFullLin)
      MarkFullLin = 0;
    if(MarkFullCol && !LastMarkFullCol)
      MarkFullCol = 0;
  }

  /* Consistency tests between the type of mark requested and
     the type of mark mode that is being performed
  */
  if(mark_mode == IMATRIX_MARK_LIN && !MarkFullLin)
    mark_mode = IMATRIX_MARK_NO;
  else if(mark_mode == IMATRIX_MARK_COL && !MarkFullCol)
    mark_mode = IMATRIX_MARK_NO;
  else if(mark_mode == IMATRIX_MARK_LINCOL && !(MarkFullCol || MarkFullLin))
    mark_mode = IMATRIX_MARK_NO;

  if(mark_mode == IMATRIX_MARK_LINCOL)
  {
    if((MarkFullCol && (ih->data->MarkLinCol == IMATRIX_MARK_LIN)) ||
       (MarkFullLin && (ih->data->MarkLinCol == IMATRIX_MARK_COL)))
    {
      *ctrl = 0;
      *shift = 0;
    }
    ih->data->MarkLinCol = (MarkFullCol) ? IMATRIX_MARK_COL : IMATRIX_MARK_LIN;
  }

  /* Consistency tests in reference to multiple attribute */
  if(!mult || (mark_mode == IMATRIX_MARK_NO))
  {
     *ctrl = 0;
     *shift = 0;
  }

  /* Consistency tests in reference to continuous attribute */
  if(cont)
    *ctrl = 0;

  return mark_mode;
}

int iMatrixMarkHide(Ihandle* ih, int ctrl)
{
  /* If the CTRL are not pressed, unmark (uncheck) ALL the marked cells */
  if(!ctrl)
    iMatrixMarkUncheckAll(ih);
  return 1;
}

void iMatrixMarkShow(Ihandle* ih, int ctrl, int lin, int col, int oldlin, int oldcol)
{
  if(iMatrixMarkFullLin())
  {
    iMatrixMarkLine(ih, lin, 1);
    col = ih->data->col.num - 1;
  }
  else if(iMatrixMarkFullCol())
  {
    iMatrixMarkColumn(ih, col, 1);
    lin = ih->data->lin.num - 1;
  }
  else
  {
    if(iMatrixMarkCellGet(ih, lin, col))
    {
      iMatrixMarkCellSet(ih, lin, col, 0);
      MarkedCells--;
    }
    else
    {
      iMatrixMarkCellSet(ih, lin, col, 1);
      MarkedCells++;
    }
  }

  /* Redraw the previous cell if the CTRL is being pressed
    (so, change the feedback of the previous cell)...
  */
  if(ctrl)
    iMatrixDrawCells(ih, oldlin, oldcol, oldlin, oldcol);

  /* Redraw the focused cell with the current marked value */
  if(lin >= 0 && col >= 0)
    iMatrixDrawCells(ih, lin, col, lin, col);

  MarkLin = lin;
  MarkCol = col;
}

/**************************************************************************/
/* Set, Get and Change the mark mode                                      */
/**************************************************************************/

/* Set which cells are marked. Unmark all cells that already were marked.
   -> v : this string contains the representation of cells to be marked.
          There are three possible representation:
       a) "C...." when .... consists a number of 0s and 1s equal to the
          number of columns of the matrix, representing each column marked
          or not. The first number is the column 1, the second is
          the column 2, and so on.
       b) "L...." in the same way that previous definition, but used for the
          lines of the matrix.
       c) "...." when .... consists consists a number of 0s and 1s equal to the
          number of cells of the matrix, representing each cell marked or not.
          In any matrix composes of L lines and C columns, the C first numbers
          are the columns of the line number 1, the next C are the columns of
          the line number 2, and so on.
*/
void* iMatrixMarkSet(Ihandle* ih, char* v)
{
  int i, j, old;
  char* p = v;
  int visible = IupGetInt(ih, "VISIBLE");
  int err;

  if(p == NULL)
  {
    iMatrixMarkUncheckAll(ih);
    return NULL;
  }
  else if(*p == 'C' || *p == 'c')
  {
    p++;
    MarkedCells = 0;
    for(j = 0; j < ih->data->col.num; j++)
    {
      /* Set the counter of cell to 1.  The functions of mark and unmark lines
         increment and decrement the counter... so, it can not use them.
      */
      old = ih->data->col.marked[j];
      if(*p++ == '1')
      {
        /* Mark all the cells of the j column */
        for(i = 0; i < ih->data->lin.num; i++, MarkedCells++)
          iMatrixMarkCellSet(ih, i, j, 1);
        ih->data->col.marked[j] = 1;
        iMatrixDrawColumnTitle(ih, j, j);
      }
      else
      {
        /* Unmark all the cells of the j column */
        for(i = 0; i < ih->data->lin.num; i++)
          iMatrixMarkCellSet(ih, i, j, 0);
        ih->data->col.marked[j] = 0;
        if(old)
          iMatrixDrawColumnTitle(ih, j, j);
      }
    }
  }
  else if(*p == 'L' || *p == 'l')
  {
    p++;
    MarkedCells = 0;
    for(i = 0; i < ih->data->lin.num; i++)
    {
      /* Set the counter of cell to 1.  The functions of mark and unmark lines
         increment and decrement the counter... so, it can not use them.
      */
      old = ih->data->lin.marked[i];
      if(*p++ == '1')
      {
        /* Mark all the cells of the i line */
        for(j = 0; j < ih->data->col.num; j++, MarkedCells++)
          iMatrixMarkCellSet(ih, i, j, 1);
        ih->data->lin.marked[i] = 1;
        iMatrixDrawLineTitle(ih, i, i);
      }
      else
      {
        /* Unmark all the cells of the i line */
        for(j = 0; j < ih->data->col.num; j++)
          iMatrixMarkCellSet(ih, i, j, 0);
        ih->data->lin.marked[i] = 0;
        if(old)
          iMatrixDrawLineTitle(ih, i, i);
      }
    }
  }
  else
  {
    MarkedCells = 0;
    for(i = 0; i < ih->data->lin.num; i++)
    {
      for(j = 0; j < ih->data->col.num; j++)
      {
        if(*p++ == '1')
        {
          iMatrixMarkCellSet(ih, i, j, 1);
          MarkedCells++;
        }
        else
          iMatrixMarkCellSet(ih, i, j, 0);
      }
      old = ih->data->lin.marked[i];
      ih->data->lin.marked[i] = 0;
      if(old)
        iMatrixDrawLineTitle(ih, i, i);
    }
    for(j = 0; j < ih->data->col.num; j++)
    {
      old = ih->data->col.marked[j];
      ih->data->col.marked[j] = 0;
      if(old)
        iMatrixDrawColumnTitle(ih, j, j);
    }
  }
  /* Redraw the visible cells */

  IsCanvasSet(ih, err);
  if(visible && err == CD_OK)
    iMatrixDrawCells(ih, ih->data->lin.first, ih->data->col.first, ih->data->lin.last, ih->data->col.last);
  return v;
}

/* Return the marked cells */
char* iMatrixMarkGet(Ihandle* ih)
{
  static int   numbaloc = 0;
  static char* marked   = NULL;

  int i, j, numbneeded;
  char* p;
  int exist_mark = 0;           /* Show if there is someone marked */
  int mark_mode  = iMatrixMarkGetMode(ih);

  if(mark_mode == IMATRIX_MARK_NO)
   return NULL;

  numbneeded = ih->data->lin.num * ih->data->col.num + 2; /* plus 2 because of L or C and the '\0'*/

  if(numbneeded > numbaloc)  /* Allocate space */
  {
    if(numbaloc)
      marked = realloc (marked, numbneeded * sizeof(char));
    else
      marked = malloc(numbneeded * sizeof(char));
    numbaloc = numbneeded ;
  }

  p = marked;

  if(mark_mode == IMATRIX_MARK_LINCOL)
    mark_mode = ih->data->MarkLinCol;

  exist_mark = 0;

  /* Define the format to be returned, based in mark_mode */
  switch(mark_mode)
  {
    case IMATRIX_MARK_CELL:  for(i = 0; i < ih->data->lin.num; i++)
                               for(j = 0; j < ih->data->col.num; j++)
                                 if(iMatrixMarkCellGet(ih, i, j))
                                 {
                                   exist_mark = 1;
                                   *p++ = '1';
                                 }
                                 else
                                 {
                                   *p++ = '0';
                                 }
                             *p = 0;
                             break;

    case IMATRIX_MARK_LIN:   *p++ = 'L';
                             for(i = 0; i < ih->data->lin.num; i++)
                               if(iMatrixMarkCellGet(ih,i,0))
                               {
                                 exist_mark = 1;
                                 *p++ = '1';
                               }
                               else
                               {
                                 *p++ = '0';
                               }
                             *p = 0;
                             break;

    case IMATRIX_MARK_COL:   *p++ = 'C';
                             for(j = 0; j < ih->data->col.num; j++)
                               if(iMatrixMarkCellGet(ih,0,j))
                               {
                                 exist_mark = 1;
                                 *p++ = '1';
                               }
                               else
                               {
                                 *p++ = '0';
                               }
                             *p = 0;
                             break;
  }

  return exist_mark ? marked : NULL;
}

/* This function is called when the mark mode is changed. Unmark all the
   cells that are marked.
   -> v : new mark mode. Currently, it is not used.
*/
void* iMatrixMarkSetMode(Ihandle* ih, char* v)
{
  int visible = IupGetInt(ih, "VISIBLE");
  int err;

  IsCanvasSet(ih, err);

  if(visible && err == CD_OK)
    iMatrixMarkUncheckAll(ih);

  return v;
}
