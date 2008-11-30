/** \file
 * \brief iupmatrix control
 * cell selection
 *
 * See Copyright Notice in iup.h
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

#include "iupmat_def.h"
#include "iupmat_cd.h"
#include "iupmat_draw.h"
#include "iupmat_scroll.h"
#include "iupmat_aux.h"
#include "iupmat_mark.h"


#define IMAT_MINMAX(a, b, min, max)  ((a > b) ? (min = b, max = a) : (min = a, max = b))


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
    iupMatrixMarkCellSet(ih, i, col, 1);

  ih->data->col.marked[col]++;

  if(redraw)
  {
    if(ih->data->col.marked[col] == 1)
      iupMatrixDrawColumnTitle(ih, col, col);
    iupMatrixDrawCells(ih, ih->data->lin.first, col, ih->data->lin.last, col);
  }
}

/* Unchecked (unselected) a full column. Redraw it.
   -> col : column to be Unchecked.
   -> redraw: define the redraw or not
*/
static void iMatrixMarkUncheckedColumn(Ihandle* ih, int col, int redraw)
{
  int i;

  for(i = 0; i < ih->data->lin.num; i++, MarkedCells--)
    iupMatrixMarkCellSet(ih, i, col, 0);

  ih->data->col.marked[col]--;

  if(redraw)
  {
    if(!ih->data->col.marked[col])
      iupMatrixDrawColumnTitle(ih, col, col);
    iupMatrixDrawCells(ih, ih->data->lin.first, col, ih->data->lin.last, col);
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
    iupMatrixMarkCellSet(ih, lin, i, 1);

  ih->data->lin.marked[lin]++;

  if(redraw)
  {
    if(ih->data->lin.marked[lin] == 1)
      iupMatrixDrawLineTitle(ih, lin, lin);
    iupMatrixDrawCells(ih, lin, ih->data->col.first, lin, ih->data->col.last);
  }
}

/* Unchecked (unselected) a full line. Redraw it.
   -> lin : line to be Unchecked.
   -> redraw: define the redraw or not
*/

static void iMatrixMarkUncheckedLine(Ihandle* ih, int lin, int redraw)
{
  int i;

  for(i = 0; i < ih->data->col.num; i++, MarkedCells--)
    iupMatrixMarkCellSet(ih, lin, i, 0);
  ih->data->lin.marked[lin]--;

  if(redraw)
  {
    if(!ih->data->lin.marked[lin])
      iupMatrixDrawLineTitle(ih, lin, lin);
    iupMatrixDrawCells(ih, lin, ih->data->col.first, lin, ih->data->col.last);
  }
}

/* Mark (or not) a region of lines using drag.
   -> lin: final line of selection.
*/
static void iMatrixMarkRegionLin(Ihandle* ih, int lin)
{
  if(lin == MarkLin)
    return;

  if((lin >= ih->data->lin.active && lin < MarkLin) ||
     (lin <= ih->data->lin.active && lin > MarkLin))     /* Unmarked */
    iMatrixMarkUncheckedLine(ih, MarkLin, 1);
  else
    iMatrixMarkLine(ih, lin, 1);

  MarkLin = lin;
  MarkCol = ih->data->col.num - 1;
}

/* Mark (or not) a region of columns (using drag).
   -> col: final column of selection.
*/
static void iMatrixMarkRegionCol(Ihandle* ih, int col)
{
  if(col == MarkCol)
    return;

  if((col >= ih->data->col.active && col < MarkCol) ||
     (col <= ih->data->col.active && col > MarkCol))     /* unmarked */
    iMatrixMarkUncheckedColumn(ih, MarkCol, 1);
  else /* Marcando */
    iMatrixMarkColumn(ih, col, 1);

  MarkCol = col;
  MarkLin = ih->data->lin.num - 1;
}

/* Mark (or not) a region of the matrix (using drag).
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

  if(MarkFullCol)  /* Mark/unmarked full columns */
  {
    iMatrixMarkRegionCol(ih, col);
    return;
  }

  if(MarkFullLin)  /* Mark/unmarked full lines */
  {
    iMatrixMarkRegionLin(ih, lin);
    return;
  }

  if(lin != MarkLin)
  {
    /* Put the "<" of "ih->data->col.active" and "MarkCol" in "colstart", and the other in "colend" */
    IMAT_MINMAX(ih->data->col.active, MarkCol, colstart, colend);

    if((lin >= ih->data->lin.active && lin < MarkLin) ||
       (lin <= ih->data->lin.active && lin > MarkLin))     /* unmarked */
    {
       /* lin and MarkLin differ, in max, of 1 */

       /* Decrement the number of times that the cell was marked... */
       /* So, cells of other region, already marked and being superpose by */
       /* current region, are not erased */
       for(i = colstart; i <= colend; i++)
         iupMatrixMarkCellSet(ih, MarkLin, i, 0);

       MarkedCells -= (colend - colstart + 1);
       RedrawLin    = MarkLin;
    }
    else   /* Mark */
    {
       /* lin and MarkLin differ, in max, of 1 */
       for(i = colstart; i <= colend; i++)
         iupMatrixMarkCellSet(ih, lin, i, 1);

       MarkedCells += (colend - colstart + 1);
       RedrawLin    = lin;
    }
    MarkLin = lin;
  }

  if(col != MarkCol)
  {
    /* Put the "<" of "ih->data->lin.active" and "MarkLin" in "linstart", and the other in "linend" */
    IMAT_MINMAX(ih->data->lin.active, MarkLin, linstart, linend);

    if((col >= ih->data->col.active && col < MarkCol) ||
       (col <= ih->data->col.active && col > MarkCol))     /* unmarked */
    {
       /* col and MarkCol differ, in max, of 1 */
       for(i = linstart; i <= linend; i++)
         iupMatrixMarkCellSet(ih, i, MarkCol, 0);

       MarkedCells -= (linend - linstart + 1);
       RedrawCol    = MarkCol;
    }
    else   /* Mark */
    {
       /* col and MarkCol differ, in max, of 1 */
       for(i = linstart; i <= linend; i++)
         iupMatrixMarkCellSet(ih, i, col, 1);

       MarkedCells += (linend - linstart + 1);
       RedrawCol    = col;
    }
    MarkCol = col;
  }

  if(RedrawLin != -1)
    iupMatrixDrawCells(ih, RedrawLin, colstart, RedrawLin, colend);
  if(RedrawCol != -1)
    iupMatrixDrawCells(ih, linstart, RedrawCol, linend, RedrawCol);
}

/* This function is called when the many cells is being selected.
   Make the scroll of the spreadsheet if the draw get out of the visualization area.
   -> x, y : mouse coordinates (canvas coordinates).
*/
void iupMatrixMarkDrag(Ihandle* ih, int x, int y)
{
  int lin, col, i, j, incl, incc;
  int err;
  int mark_mode = iupMatrixMarkGetMode(ih);

  if(mark_mode == IMAT_MARK_NO)
    return;

  /* Get the cursor coordinate (lin,col) */
  if(!iupMatrixAuxGetLineCol(ih, x, y, &lin, &col))
    return;

  /* There is no drag inside the same cell... */
  if(lin == ih->data->lin.active && col == ih->data->col.active)
    return;

  /* Critics according to the mark mode */
  if(mark_mode == IMAT_MARK_LIN)
  {
    if(col != -1)
      return;
    if(!ih->data->lin.marked[ih->data->lin.active])
      return;
  }
  else if(mark_mode == IMAT_MARK_COL)
  {
    if(lin != -1)
      return;
    if(!ih->data->col.marked[ih->data->col.active])
      return;
  }
  else if(mark_mode == IMAT_MARK_LINCOL)
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

/* Unchecked (unselected) the last block marked. The block limits are defined
   by the cell that contains the focus and by the cell stored in MarkLin
   and MarkCol. DO NOT redraw the matrix.
*/
void iupMatrixMarkUncheckedBlock(Ihandle* ih)
{
  int i, j, inc;

  if(!MarkedCells)
    return;

  if(LastMarkFullLin)
  {
    inc = ih->data->lin.active < MarkLin ? 1 : -1;
    for(i = ih->data->lin.active;  ; i += inc)
    {
      iMatrixMarkUncheckedLine(ih, i, 0);
      if(i == MarkLin)
        break;
    }
  }
  else if(LastMarkFullCol)
  {
    inc = ih->data->col.active < MarkCol ? 1 : -1;
    for(i = ih->data->col.active;  ; i += inc)
    {
      iMatrixMarkUncheckedColumn(ih, i, 0);
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
        iupMatrixMarkCellSet(ih, i, j, 0);
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
void iupMatrixMarkBlock(Ihandle* ih, int lin, int col)
{
  int i, j, inc;

  if(iupMatrixMarkFullLin())
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
  else if(iupMatrixMarkFullCol())
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
        iupMatrixMarkCellSet(ih, i, j, 1);
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

void iupMatrixMarkCellSet(Ihandle* ih, int lin, int col, int mark)
{
  if(!ih->data->valcb)
    ih->data->v[lin][col].mark = (unsigned char)mark;
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

int iupMatrixMarkCellGet(Ihandle* ih, int lin, int col)
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

/* unmarked (Unchecked) all the marked cells.
   Redraw the cells of this region.
*/
static void iMatrixMarkUncheckedAll(Ihandle* ih)
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
      old = iupMatrixMarkCellGet(ih, i, j);
      iupMatrixMarkCellSet(ih, i, j, 0);
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
      iupMatrixDrawLineTitle(ih, i, i);
  }
  MarkedCells = 0;
  for(j = 0; j < ih->data->col.num; j++)
  {
    old = ih->data->col.marked[j];
    ih->data->col.marked[j] = 0;
    if(old)
      iupMatrixDrawColumnTitle(ih, j, j);
  }

  /* If it is necessary to redraw some cell, the min_lin, max_lin, 
     min_col e max_col values were changed, but only one of them must test
  */
  if(max_lin != -1)
    iupMatrixDrawCells(ih, min_lin, min_col, max_lin, max_col);
}


/**************************************************************************/
/* Consult functions                                                      */
/**************************************************************************/

int iupMatrixMarkFullLin(void)
{
  return MarkFullLin;
}

int iupMatrixMarkFullCol(void)
{
  return MarkFullCol;
}

void iupMatrixMarkReset(void)
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
int iupMatrixMarkColumnMarked(Ihandle* ih, int col)
{
  if(col == 0)  /* Line titles are never marked... */
    return 0;

  return ih->data->col.marked[col-1];
}

/* Verify if the line is or not marked.
   -> lin - number of line. Note that the number is NOT presented in the common way.
      0 represents the line of titles and 1 the first line of the matrix.
*/
int iupMatrixMarkLineMarked (Ihandle* ih, int lin)
{
  if(lin == 0) /* Column titles are never marked... */
    return 0;

  return ih->data->lin.marked[lin-1];
}

/* Return the mark mode set by user. Consults "IMAT_MARK_MODE" attribute and
   returns a constant showing the chosen mode.
*/
int iupMatrixMarkGetMode(Ihandle* ih)
{
  char* mode = IupGetAttribute(ih, "MARK_MODE");

  if (mode && *mode)
  {
    if(iupStrEqualNoCase(mode, "CELL"))
      return IMAT_MARK_CELL;
    if(iupStrEqualNoCase(mode, "LIN"))
      return IMAT_MARK_LIN;
    if(iupStrEqualNoCase(mode, "COL"))
      return IMAT_MARK_COL;
    if(iupStrEqualNoCase(mode, "LINCOL"))
      return IMAT_MARK_LINCOL;
  }
  return IMAT_MARK_NO;
}

int iupMatrixMarkCritica(Ihandle* ih, int* lin, int* col, int* shift, int* ctrl, int* duplo)
{
  int  mark_mode = iupMatrixMarkGetMode(ih);
  int  mult = !iupAttribGetInt(ih, "MULTIPLE") ? 0 : 1;
  int  cont = (iupStrEqualNoCase(IupGetAttribute(ih, "AREA"), "CONTINUOUS")) ? 1 : 0;

  /* If it was pointing for a column title... */
  if(*lin == -1)
  {
    *lin   = ih->data->lin.first;
    *duplo = 0; /* Guarantee that it will be set as a double-click */
    if(mark_mode != IMAT_MARK_NO && ((mark_mode == IMAT_MARK_CELL && mult) || 
                                         mark_mode == IMAT_MARK_COL || 
                                         mark_mode == IMAT_MARK_LINCOL))
      MarkFullCol = 1;
  }
  /* If it was pointing for a line title... */
  else if(*col == -1)
  {
    *col   = ih->data->col.first;
    *duplo = 0;  /* Ensures that will not interpret as a double-click */
    if(mark_mode != IMAT_MARK_NO && ((mark_mode == IMAT_MARK_CELL && mult) || 
                                         mark_mode == IMAT_MARK_LIN || 
                                         mark_mode == IMAT_MARK_LINCOL))
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
  if(mark_mode == IMAT_MARK_LIN && !MarkFullLin)
    mark_mode = IMAT_MARK_NO;
  else if(mark_mode == IMAT_MARK_COL && !MarkFullCol)
    mark_mode = IMAT_MARK_NO;
  else if(mark_mode == IMAT_MARK_LINCOL && !(MarkFullCol || MarkFullLin))
    mark_mode = IMAT_MARK_NO;

  if(mark_mode == IMAT_MARK_LINCOL)
  {
    if((MarkFullCol && (ih->data->MarkLinCol == IMAT_MARK_LIN)) ||
       (MarkFullLin && (ih->data->MarkLinCol == IMAT_MARK_COL)))
    {
      *ctrl = 0;
      *shift = 0;
    }
    ih->data->MarkLinCol = (MarkFullCol) ? IMAT_MARK_COL : IMAT_MARK_LIN;
  }

  /* Consistency tests in reference to multiple attribute */
  if(!mult || (mark_mode == IMAT_MARK_NO))
  {
     *ctrl = 0;
     *shift = 0;
  }

  /* Consistency tests in reference to continuous attribute */
  if(cont)
    *ctrl = 0;

  return mark_mode;
}

int iupMatrixMarkHide(Ihandle* ih, int ctrl)
{
  /* If the CTRL are not pressed, unmarked (unchecked) ALL the marked cells */
  if(!ctrl)
    iMatrixMarkUncheckedAll(ih);
  return 1;
}

void iupMatrixMarkShow(Ihandle* ih, int ctrl, int lin, int col, int oldlin, int oldcol)
{
  if(iupMatrixMarkFullLin())
  {
    iMatrixMarkLine(ih, lin, 1);
    col = ih->data->col.num - 1;
  }
  else if(iupMatrixMarkFullCol())
  {
    iMatrixMarkColumn(ih, col, 1);
    lin = ih->data->lin.num - 1;
  }
  else
  {
    if(iupMatrixMarkCellGet(ih, lin, col))
    {
      iupMatrixMarkCellSet(ih, lin, col, 0);
      MarkedCells--;
    }
    else
    {
      iupMatrixMarkCellSet(ih, lin, col, 1);
      MarkedCells++;
    }
  }

  /* Redraw the previous cell if the CTRL is being pressed
    (so, change the feedback of the previous cell)...
  */
  if(ctrl)
    iupMatrixDrawCells(ih, oldlin, oldcol, oldlin, oldcol);

  /* Redraw the focused cell with the current marked value */
  if(lin >= 0 && col >= 0)
    iupMatrixDrawCells(ih, lin, col, lin, col);

  MarkLin = lin;
  MarkCol = col;
}

/**************************************************************************/
/* Set, Get and Change the mark mode                                      */
/**************************************************************************/

/* Set which cells are marked. unmarked all cells that already were marked.
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
void* iupMatrixMarkSet(Ihandle* ih, const char* v)
{
  int i, j, old;
  const char* p = v;
  int visible = IupGetInt(ih, "VISIBLE");
  int err;

  if(p == NULL)
  {
    iMatrixMarkUncheckedAll(ih);
    return NULL;
  }
  else if(*p == 'C' || *p == 'c')
  {
    p++;
    MarkedCells = 0;
    for(j = 0; j < ih->data->col.num; j++)
    {
      /* Set the counter of cell to 1.  The functions of mark and unmarked lines
         increment and decrement the counter... so, it can not use them.
      */
      old = ih->data->col.marked[j];
      if(*p++ == '1')
      {
        /* Mark all the cells of the j column */
        for(i = 0; i < ih->data->lin.num; i++, MarkedCells++)
          iupMatrixMarkCellSet(ih, i, j, 1);
        ih->data->col.marked[j] = 1;
        iupMatrixDrawColumnTitle(ih, j, j);
      }
      else
      {
        /* unmarked all the cells of the j column */
        for(i = 0; i < ih->data->lin.num; i++)
          iupMatrixMarkCellSet(ih, i, j, 0);
        ih->data->col.marked[j] = 0;
        if(old)
          iupMatrixDrawColumnTitle(ih, j, j);
      }
    }
  }
  else if(*p == 'L' || *p == 'l')
  {
    p++;
    MarkedCells = 0;
    for(i = 0; i < ih->data->lin.num; i++)
    {
      /* Set the counter of cell to 1.  The functions of mark and unmarked lines
         increment and decrement the counter... so, it can not use them.
      */
      old = ih->data->lin.marked[i];
      if(*p++ == '1')
      {
        /* Mark all the cells of the i line */
        for(j = 0; j < ih->data->col.num; j++, MarkedCells++)
          iupMatrixMarkCellSet(ih, i, j, 1);
        ih->data->lin.marked[i] = 1;
        iupMatrixDrawLineTitle(ih, i, i);
      }
      else
      {
        /* unmarked all the cells of the i line */
        for(j = 0; j < ih->data->col.num; j++)
          iupMatrixMarkCellSet(ih, i, j, 0);
        ih->data->lin.marked[i] = 0;
        if(old)
          iupMatrixDrawLineTitle(ih, i, i);
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
          iupMatrixMarkCellSet(ih, i, j, 1);
          MarkedCells++;
        }
        else
          iupMatrixMarkCellSet(ih, i, j, 0);
      }
      old = ih->data->lin.marked[i];
      ih->data->lin.marked[i] = 0;
      if(old)
        iupMatrixDrawLineTitle(ih, i, i);
    }
    for(j = 0; j < ih->data->col.num; j++)
    {
      old = ih->data->col.marked[j];
      ih->data->col.marked[j] = 0;
      if(old)
        iupMatrixDrawColumnTitle(ih, j, j);
    }
  }
  /* Redraw the visible cells */

  IsCanvasSet(ih, err);
  if(visible && err == CD_OK)
    iupMatrixDrawCells(ih, ih->data->lin.first, ih->data->col.first, ih->data->lin.last, ih->data->col.last);
  return (char*)v;
}

/* Return the marked cells */
char* iupMatrixMarkGet(Ihandle* ih)
{
  static int   numbaloc = 0;
  static char* marked   = NULL;

  int i, j, numbneeded;
  char* p;
  int exist_mark = 0;           /* Show if there is someone marked */
  int mark_mode  = iupMatrixMarkGetMode(ih);

  if(mark_mode == IMAT_MARK_NO)
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

  if(mark_mode == IMAT_MARK_LINCOL)
    mark_mode = ih->data->MarkLinCol;

  exist_mark = 0;

  /* Define the format to be returned, based in mark_mode */
  switch(mark_mode)
  {
    case IMAT_MARK_CELL:  for(i = 0; i < ih->data->lin.num; i++)
                               for(j = 0; j < ih->data->col.num; j++)
                                 if(iupMatrixMarkCellGet(ih, i, j))
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

    case IMAT_MARK_LIN:   *p++ = 'L';
                             for(i = 0; i < ih->data->lin.num; i++)
                               if(iupMatrixMarkCellGet(ih,i,0))
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

    case IMAT_MARK_COL:   *p++ = 'C';
                             for(j = 0; j < ih->data->col.num; j++)
                               if(iupMatrixMarkCellGet(ih,0,j))
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

/* This function is called when the mark mode is changed. unmarked all the
   cells that are marked.
   -> v : new mark mode. Currently, it is not used.
*/
void* iupMatrixMarkSetMode(Ihandle* ih, const char* v)
{
  int visible = IupGetInt(ih, "VISIBLE");
  int err;

  IsCanvasSet(ih, err);

  if(visible && err == CD_OK)
    iMatrixMarkUncheckedAll(ih);

  return (char*)v;
}
