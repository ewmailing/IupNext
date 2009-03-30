/** \file
 * \brief iupmatrix control
 * cell selection
 *
 * See Copyright Notice in "iup.h"
 */

/**************************************************************************/
/*  Functions to cell selection (mark)
/**************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>

#include "iup.h"
#include "iupcbs.h"
//#include "iupkey.h"

#include <cd.h>

#include "iup_object.h"
#include "iup_attrib.h"
#include "iup_str.h"
//#include "iup_drv.h"
#include "iup_stdcontrols.h"
//#include "iup_controls.h"
//#include "iup_cdutil.h"

#include "iupmat_def.h"
//#include "iupmat_cd.h"
//#include "iupmat_draw.h"
//#include "iupmat_scroll.h"
//#include "iupmat_aux.h"
#include "iupmat_mark.h"
#include "iupmat_getset.h"
#include "iupmat_draw.h"


//
//#define IMAT_MINMAX(a, b, min, max)  ((a > b) ? (min = b, max = a) : (min = a, max = b))
//
/* Mark (select) a full column. Redraw it.
//   -> col : column to be mark.
//   -> redraw: define the redraw or not
*/
//static void iMatrixMarkColumn(Ihandle* ih, int col, int redraw)
//{
//  int lin;
//
//  for(lin = 0; lin < ih->data->lines.num; lin++, ih->data->MarkedCells++)
//    iupMatrixMarkCellSet(ih, lin, col, 1);
//
//  ih->data->columns.marks[col]++;
//
//  if(redraw)
//  {
//    if(ih->data->columns.marks[col] == 1)
//      iupMatrixDrawColumnTitle(ih, col, col);
//    iupMatrixDrawCells(ih, ih->data->lines.first, col, ih->data->lines.last, col);
//  }
//}
//
/* Unchecked (unselected) a full column. Redraw it.
//   -> col : column to be Unchecked.
//   -> redraw: define the redraw or not
*/
//static void iMatrixMarkUncheckedColumn(Ihandle* ih, int col, int redraw)
//{
//  int lin;
//
//  for(lin = 0; lin < ih->data->lines.num; lin++, ih->data->MarkedCells--)
//    iupMatrixMarkCellSet(ih, lin, col, 0);
//
//  ih->data->columns.marks[col]--;
//
//  if(redraw)
//  {
//    if(!ih->data->columns.marks[col])
//      iupMatrixDrawColumnTitle(ih, col, col);
//    iupMatrixDrawCells(ih, ih->data->lines.first, col, ih->data->lines.last, col);
//  }
//}
//
/* Mark (select) a full line. Redraw it.
//   -> lin : line to be mark.
//   -> redraw: define the redraw or not
*/
//static void iMatrixMarkLine(Ihandle* ih, int lin, int redraw)
//{
//  int lin;
//
//  for(lin = 0; lin < ih->data->columns.num; lin++, ih->data->MarkedCells++)
//    iupMatrixMarkCellSet(ih, lin, lin, 1);
//
//  ih->data->lines.marks[lin]++;
//
//  if(redraw)
//  {
//    if(ih->data->lines.marks[lin] == 1)
//      iupMatrixDrawLineTitle(ih, lin, lin);
//    iupMatrixDrawCells(ih, lin, ih->data->columns.first, lin, ih->data->columns.last);
//  }
//}
//
/* Unchecked (unselected) a full line. Redraw it.
//   -> lin : line to be Unchecked.
//   -> redraw: define the redraw or not
*/
//
//static void iMatrixMarkUncheckedLine(Ihandle* ih, int lin, int redraw)
//{
//  int lin;
//
//  for(lin = 0; lin < ih->data->columns.num; lin++, ih->data->MarkedCells--)
//    iupMatrixMarkCellSet(ih, lin, lin, 0);
//  ih->data->lines.marks[lin]--;
//
//  if(redraw)
//  {
//    if(!ih->data->lines.marks[lin])
//      iupMatrixDrawLineTitle(ih, lin, lin);
//    iupMatrixDrawCells(ih, lin, ih->data->columns.first, lin, ih->data->columns.last);
//  }
//}
//
/* Mark (or not) a region of lines using drag.
//   -> lin: final line of selection.
*/
//static void iMatrixMarkRegionLin(Ihandle* ih, int lin)
//{
//  if(lin == ih->data->MarkLin)
//    return;
//
//  if((lin >= ih->data->lines.focus_cell && lin < ih->data->MarkLin) ||
//     (lin <= ih->data->lines.focus_cell && lin > ih->data->MarkLin))     /* Unmarked */
//    iMatrixMarkUncheckedLine(ih, ih->data->MarkLin, 1);
//  else
//    iMatrixMarkLine(ih, lin, 1);
//
//  ih->data->MarkLin = lin;
//  ih->data->MarkCol = ih->data->columns.num - 1;
//}
//
/* Mark (or not) a region of columns (using drag).
//   -> col: final column of selection.
*/
//static void iMatrixMarkRegionCol(Ihandle* ih, int col)
//{
//  if(col == ih->data->MarkCol)
//    return;
//
//  if((col >= ih->data->columns.focus_cell && col < ih->data->MarkCol) ||
//     (col <= ih->data->columns.focus_cell && col > ih->data->MarkCol))     /* unmarked */
//    iMatrixMarkUncheckedColumn(ih, ih->data->MarkCol, 1);
//  else /* Marcando */
//    iMatrixMarkColumn(ih, col, 1);
//
//  ih->data->MarkCol = col;
//  ih->data->MarkLin = ih->data->lines.num - 1;
//}
//
/* Mark (or not) a region of the matrix (using drag).
//   The block to be selected is defined by the cell passed as parameter,
//   and ih->data->MarkLin and ih->data->MarkCol.
//   -> lin, col : final cell of the selection.
*/
//static void iMatrixMarkRegion(Ihandle* ih, int lin, int col)
//{
//  int RedrawLin = -1, RedrawCol = -1;
//  int colstart  =  0, linstart  =  0;
//  int colend    =  0, linend    =  0;
//  int lin;
//
//  if(ih->data->MarkFullCol)  /* Mark/unmarked full columns */
//  {
//    iMatrixMarkRegionCol(ih, col);
//    return;
//  }
//
//  if(ih->data->MarkFullLin)  /* Mark/unmarked full lines */
//  {
//    iMatrixMarkRegionLin(ih, lin);
//    return;
//  }
//
//  if(lin != ih->data->MarkLin)
//  {
//    /* Put the "<" of "ih->data->columns.focus_cell" and "ih->data->MarkCol" in "colstart", and the other in "colend" */
//    IMAT_MINMAX(ih->data->columns.focus_cell, ih->data->MarkCol, colstart, colend);
//
//    if((lin >= ih->data->lines.focus_cell && lin < ih->data->MarkLin) ||
//       (lin <= ih->data->lines.focus_cell && lin > ih->data->MarkLin))     /* unmarked */
//    {
//       /* lin and ih->data->MarkLin differ, in max, of 1 */
//
//       /* Decrement the number of times that the cell was marked... */
//       /* So, cells of other region, already marked and being superpose by */
//       /* current region, are not erased */
//       for(lin = colstart; lin <= colend; lin++)
//         iupMatrixMarkCellSet(ih, ih->data->MarkLin, lin, 0);
//
//       ih->data->MarkedCells -= (colend - colstart + 1);
//       RedrawLin    = ih->data->MarkLin;
//    }
//    else   /* Mark */
//    {
//       /* lin and ih->data->MarkLin differ, in max, of 1 */
//       for(lin = colstart; lin <= colend; lin++)
//         iupMatrixMarkCellSet(ih, lin, lin, 1);
//
//       ih->data->MarkedCells += (colend - colstart + 1);
//       RedrawLin    = lin;
//    }
//    ih->data->MarkLin = lin;
//  }
//
//  if(col != ih->data->MarkCol)
//  {
//    /* Put the "<" of "ih->data->lines.focus_cell" and "ih->data->MarkLin" in "linstart", and the other in "linend" */
//    IMAT_MINMAX(ih->data->lines.focus_cell, ih->data->MarkLin, linstart, linend);
//
//    if((col >= ih->data->columns.focus_cell && col < ih->data->MarkCol) ||
//       (col <= ih->data->columns.focus_cell && col > ih->data->MarkCol))     /* unmarked */
//    {
//       /* col and ih->data->MarkCol differ, in max, of 1 */
//       for(lin = linstart; lin <= linend; lin++)
//         iupMatrixMarkCellSet(ih, lin, ih->data->MarkCol, 0);
//
//       ih->data->MarkedCells -= (linend - linstart + 1);
//       RedrawCol    = ih->data->MarkCol;
//    }
//    else   /* Mark */
//    {
//       /* col and ih->data->MarkCol differ, in max, of 1 */
//       for(lin = linstart; lin <= linend; lin++)
//         iupMatrixMarkCellSet(ih, lin, col, 1);
//
//       ih->data->MarkedCells += (linend - linstart + 1);
//       RedrawCol    = col;
//    }
//    ih->data->MarkCol = col;
//  }
//
//  if(RedrawLin != -1)
//    iupMatrixDrawCells(ih, RedrawLin, colstart, RedrawLin, colend);
//  if(RedrawCol != -1)
//    iupMatrixDrawCells(ih, linstart, RedrawCol, linend, RedrawCol);
//}

//void iupMatrixMarkMouseDrag(Ihandle* ih, int x, int y)
//{
//  int lin, col, lin, col, incl, incc;
//
//  if(ih->data->mark_mode == IMAT_MARK_NO)
//    return;
//
//  /* Get the cursor coordinate (lin,col) */
//  if(!iupMatrixAuxGetLinColFromXY(ih, x, y, &lin, &col))
//    return;
//
//  /* There is no drag inside the same cell... */
//  if(lin == ih->data->lines.focus_cell && col == ih->data->columns.focus_cell)
//    return;
//
//  /* Critics according to the mark mode */
//  if(ih->data->mark_mode == IMAT_MARK_LIN)
//  {
//    if(col != -1)
//      return;
//    if(!ih->data->lines.marks[ih->data->lines.focus_cell])
//      return;
//  }
//  else if(ih->data->mark_mode == IMAT_MARK_COL)
//  {
//    if(lin != -1)
//      return;
//    if(!ih->data->columns.marks[ih->data->columns.focus_cell])
//      return;
//  }
//  else if(ih->data->mark_mode == IMAT_MARK_LINCOL)
//  {
//    if((lin != -1) && (col != -1))
//      return;
//    if((lin == -1) && (col == -1))
//    {
//      if((!ih->data->columns.marks[ih->data->columns.focus_cell]) && (!ih->data->lines.marks[ih->data->lines.focus_cell]))
//        return;
//    }
//    else
//    {
//      if((lin == -1) && (!ih->data->columns.marks[ih->data->columns.focus_cell]))
//        return;
//      if((col == -1) && (!ih->data->lines.marks[ih->data->lines.focus_cell]))
//        return;
//    }
//  }
//
//  if(col == -2)      /* Vertical scrollbar area */
//  {
//    if(!ih->data->MarkFullLin)
//      iupMatrixScrollRight(ih);
//
//    col = ih->data->columns.last;
//  }
//  else if(col == -1) /* Column of the line titles */
//  {
//    if(!ih->data->MarkFullLin)
//      iupMatrixScrollLeft(ih);
//
//    col = ih->data->columns.first;
//  }
//
//  if(lin == -2)      /* Horizontal scrollbar area */
//  {
//    if(!ih->data->MarkFullCol)
//      iupMatrixScrollDown(ih);
//
//    lin = ih->data->lines.last;
//  }
//  else if(lin == -1) /* Line of the column titles */
//  {
//    if(!ih->data->MarkFullCol)
//      iupMatrixScrollUp(ih);
//
//    lin = ih->data->lines.first;
//  }
//
//  incl = (lin > ih->data->MarkLin) ? 1 : -1;
//  incc = (col > ih->data->MarkCol) ? 1 : -1;
//
//  /* Call iMatrixMarkRegion more than once, if the mouse was "thrown",
//     and no event has been generated in a line/column */
//  for(lin = ih->data->MarkLin, col = ih->data->MarkCol; lin != lin || col != col;)
//  {
//    if(lin != lin) lin += incl;
//    if(col != col) col += incc;
//    iMatrixMarkRegion(ih, lin, col);
//  }
//
//  iupMatrixDrawUpdate(ih);
//}

/* Unchecked (unselected) the last block marked. The block limits are defined
   by the cell that contains the focus and by the cell stored in ih->data->MarkLin
   and ih->data->MarkCol. DO NOT redraw the matrix. */
//void iupMatrixMarkMouseUncheckedBlock(Ihandle* ih)
//{
//  int lin, col, inc;
//
//  if(!ih->data->MarkedCells)
//    return;
//
//  if(ih->data->LastMarkFullLin)
//  {
//    inc = ih->data->lines.focus_cell < ih->data->MarkLin ? 1 : -1;
//    for(lin = ih->data->lines.focus_cell;  ; lin += inc)
//    {
//      iMatrixMarkUncheckedLine(ih, lin, 0);
//      if(lin == ih->data->MarkLin)
//        break;
//    }
//  }
//  else if(ih->data->LastMarkFullCol)
//  {
//    inc = ih->data->columns.focus_cell < ih->data->MarkCol ? 1 : -1;
//    for(lin = ih->data->columns.focus_cell;  ; lin += inc)
//    {
//      iMatrixMarkUncheckedColumn(ih, lin, 0);
//      if(lin == ih->data->MarkCol)
//        break;
//    }
//  }
//  else
//  {
//    int inclin = (ih->data->lines.focus_cell < ih->data->MarkLin) ? 1 : -1;
//    int inccol = (ih->data->columns.focus_cell < ih->data->MarkCol) ? 1 : -1;
//
//    for(lin = ih->data->lines.focus_cell;  ; lin += inclin)
//    {
//      for(col = ih->data->columns.focus_cell;  ; col += inccol)
//      {
//        iupMatrixMarkCellSet(ih, lin, col, 0);
//        ih->data->MarkedCells--;
//        if(col == ih->data->MarkCol)
//          break;
//      }
//      if(lin == ih->data->MarkLin)
//        break;
//    }
//  }
//}

/* Mark a block of cells. If the cell already is marked, increments its counter
   (when two selected areas are overlap. Starts of the cell that contain the
   focus until the cell passed as parameter. Update ih->data->MarkLin and ih->data->MarkCol.
   DO NOT redraw the matrix.
   -> lin, col : cell coordinates that define the block to be marked.
*/
//void iupMatrixMarkMouseBlock(Ihandle* ih, int lin, int col)
//{
//  int lin, col, inc;
//
//  if(ih->data->MarkFullLin)
//  {
//    inc = ih->data->lines.focus_cell < lin ? 1 : -1;
//    for(lin = ih->data->lines.focus_cell;  ; lin += inc)
//    {
//      iMatrixMarkLine(ih, lin, 0);
//      if(lin == lin)
//        break;
//    }
//    ih->data->MarkLin = lin;
//  }
//  else if(ih->data->MarkFullCol)
//  {
//    inc = ih->data->columns.focus_cell < col ? 1 : -1;
//    for(lin = ih->data->columns.focus_cell;  ; lin += inc)
//    {
//      iMatrixMarkColumn(ih, lin, 0);
//      if(lin == col)
//        break;
//    }
//    ih->data->MarkCol = col;
//  }
//  else
//  {
//    int inclin = (ih->data->lines.focus_cell < lin) ? 1 : -1;
//    int inccol = (ih->data->columns.focus_cell < col) ? 1 : -1;
//
//    for(lin = ih->data->lines.focus_cell;  ; lin += inclin)
//    {
//      for(col = ih->data->columns.focus_cell;  ; col += inccol)
//      {
//        iupMatrixMarkCellSet(ih, lin, col, 1);
//        ih->data->MarkedCells++;
//        if(col == col)
//          break;
//      }
//      if(lin == lin)
//        break;
//    }
//    ih->data->MarkLin = lin;
//    ih->data->MarkCol = col;
//  }
//}

static void iMatrixMarkAllLinCol(ImatLinColData *p, int mark)
{
  int i;
  for(i = 1; i < p->num; i++)
  {
    if (mark)
      p->flags[i] |= IUPMAT_MARK;
    else
      p->flags[i] &= ~IUPMAT_MARK;
  }
}

void iupMatrixMarkCellSet2(Ihandle* ih, int lin, int col, int mark, IFniii markedit_cb, char* str)
{
  if (ih->data->callback_mode)
  {
    if (markedit_cb)
      markedit_cb(ih, lin, col, mark);
    else
    {
      sprintf(str, "MARK%d:%d", lin, col);
      if (mark)
        iupAttribSetStr(ih, str, "1");
      else
        iupAttribSetStr(ih, str, NULL);
    }
  }
  else
  {
    if (mark)
      ih->data->cells[lin][col].flags |= IUPMAT_MARK;
    else
      ih->data->cells[lin][col].flags &= ~IUPMAT_MARK;
  }
}

int iupMatrixMarkCellGet(Ihandle* ih, int lin, int col, IFnii mark_cb, char* str)
{
  if (ih->data->callback_mode)
  {
    if (mark_cb)
      return mark_cb(ih, lin, col);
    else
    {
      int mark = 0;
      char* value;
      sprintf(str, "MARK%d:%d", lin, col);
      value = iupAttribGet(ih, str);
      iupStrToInt(value, &mark);
      return mark;
    }
  }
  else
    return ih->data->cells[lin][col].flags & IUPMAT_MARK;
}

void iupMatrixMarkClearAll(Ihandle* ih, int check)
{
  if (ih->data->mark_mode == IMAT_MARK_CELL || !check)
  {
    int lin, col;
    IFniii markedit_cb = NULL;
    char str[100];

    if (check)
      markedit_cb = (IFniii)IupGetCallback(ih, "MARKEDIT_CB");

    for(lin = 1; lin < ih->data->lines.num; lin++)
    {
      for(col = 1; col < ih->data->columns.num; col++)
        iupMatrixMarkCellSet2(ih, lin, col, 0, markedit_cb, str);
    }
  }

  if (ih->data->mark_mode != IMAT_MARK_NO || !check)
  {
    /* clear lines and columns marks */
    iMatrixMarkAllLinCol(&(ih->data->columns), 0);
    iMatrixMarkAllLinCol(&(ih->data->lines), 0);
  }
}

//void iupMatrixMarkMouseReset(void)
//{
//  ih->data->LastMarkFullLin = ih->data->MarkFullLin;
//  ih->data->LastMarkFullCol = ih->data->MarkFullCol;
//
//  ih->data->MarkFullLin = 0;
//  ih->data->MarkFullCol = 0;
//}

int iupMatrixColumnIsMarked(Ihandle* ih, int col)
{
  if (col == 0 ||  /* Line titles are never marked... */
      ih->data->mark_mode == IMAT_MARK_NO)
    return 0;

  return ih->data->columns.flags[col] & IUPMAT_MARK;
}

int iupMatrixLineIsMarked (Ihandle* ih, int lin)
{
  if (lin == 0 || /* Column titles are never marked... */
      ih->data->mark_mode == IMAT_MARK_NO)
    return 0;

  return ih->data->lines.flags[lin] & IUPMAT_MARK;
}

//void iupMatrixMarkMouseAdjust(Ihandle* ih, int* lin, int* col, int* shift, int* ctrl, int* duplo)
//{
//  int  mult = !iupAttribGetInt(ih, "MULTIPLE") ? 0 : 1;
//  int  cont = (iupStrEqualNoCase(IupGetAttribute(ih, "AREA"), "CONTINUOUS")) ? 1 : 0;
//
//  /* If it was pointing for a column title... */
//  if(*lin == -1)
//  {
//    *lin   = ih->data->lines.first;
//    *duplo = 0;  /* Make sure that will not be interpreted as a double-click */
//    if(ih->data->mark_mode != IMAT_MARK_NO && ((ih->data->mark_mode == IMAT_MARK_CELL && mult) || 
//                                         ih->data->mark_mode == IMAT_MARK_COL || 
//                                         ih->data->mark_mode == IMAT_MARK_LINCOL))
//      ih->data->MarkFullCol = 1;
//  }
//  /* If it was pointing for a line title... */
//  else if(*col == -1)
//  {
//    *col   = ih->data->columns.first;
//    *duplo = 0;  /* Make sure that will not be interpreted as a double-click */
//    if(ih->data->mark_mode != IMAT_MARK_NO && ((ih->data->mark_mode == IMAT_MARK_CELL && mult) || 
//                                         ih->data->mark_mode == IMAT_MARK_LIN || 
//                                         ih->data->mark_mode == IMAT_MARK_LINCOL))
//      ih->data->MarkFullLin = 1;
//  }
//
//  /* Verify if an line or column title was clicked (selection of full lines or columns)
//     and that full lines and columns were not marked
//  */
//  if(*shift)
//  {
//    if(ih->data->MarkFullLin && !ih->data->LastMarkFullLin)
//      ih->data->MarkFullLin = 0;
//    if(ih->data->MarkFullCol && !ih->data->LastMarkFullCol)
//      ih->data->MarkFullCol = 0;
//  }
//
//  /* Consistency tests between the type of mark requested and
//     the type of mark mode that is being performed
//  */
//  if(ih->data->mark_mode == IMAT_MARK_LIN && !ih->data->MarkFullLin)
//    ih->data->mark_mode = IMAT_MARK_NO;
//  else if(ih->data->mark_mode == IMAT_MARK_COL && !ih->data->MarkFullCol)
//    ih->data->mark_mode = IMAT_MARK_NO;
//  else if(ih->data->mark_mode == IMAT_MARK_LINCOL && !(ih->data->MarkFullCol || ih->data->MarkFullLin))
//    ih->data->mark_mode = IMAT_MARK_NO;
//
//  if(ih->data->mark_mode == IMAT_MARK_LINCOL)
//  {
//    if((ih->data->MarkFullCol && (ih->data->ih->data->MarkLinCol == IMAT_MARK_LIN)) ||
//       (ih->data->MarkFullLin && (ih->data->ih->data->MarkLinCol == IMAT_MARK_COL)))
//    {
//      *ctrl = 0;
//      *shift = 0;
//    }
//    ih->data->ih->data->MarkLinCol = (ih->data->MarkFullCol) ? IMAT_MARK_COL : IMAT_MARK_LIN;
//  }
//
//  /* Consistency tests in reference to multiple attribute */
//  if(!mult || (ih->data->mark_mode == IMAT_MARK_NO))
//  {
//     *ctrl = 0;
//     *shift = 0;
//  }
//
//  /* Consistency tests in reference to continuous attribute */
//  if(cont)
//    *ctrl = 0;
//}

//void iupMatrixMarkMouseShow(Ihandle* ih, int ctrl, int lin, int col, int oldlin, int oldcol)
//{
//  if(ih->data->MarkFullLin)
//  {
//    iMatrixMarkLine(ih, lin, 1);
//    col = ih->data->columns.num - 1;
//  }
//  else if(ih->data->MarkFullCol)
//  {
//    iMatrixMarkColumn(ih, col, 1);
//    lin = ih->data->lines.num - 1;
//  }
//  else
//  {
//    if(iupMatrixMarkCellGet(ih, lin, col))
//    {
//      iupMatrixMarkCellSet(ih, lin, col, 0);
//      ih->data->MarkedCells--;
//    }
//    else
//    {
//      iupMatrixMarkCellSet(ih, lin, col, 1);
//      ih->data->MarkedCells++;
//    }
//  }
//
//  /* Redraw the previous cell if the CTRL is being pressed
//    (so, change the feedback of the previous cell)...
//  */
//  if(ctrl)
//    iupMatrixDrawCells(ih, oldlin, oldcol, oldlin, oldcol);
//
//  /* Redraw the focused cell with the current marked value */
//  if(lin >= 0 && col >= 0)
//    iupMatrixDrawCells(ih, lin, col, lin, col);
//
//  ih->data->MarkLin = lin;
//  ih->data->MarkCol = col;
//}

int iupMatrixSetMarkedAttrib(Ihandle* ih, const char* value)
{
  int lin, col, mark;
  char str[100];
  IFniii markedit_cb;

  if (ih->data->mark_mode == IMAT_MARK_NO)
    return 0;

  if (!value)
    iupMatrixMarkClearAll(ih, 1);
  else if (*value == 'C' || *value == 'c')  /* columns */
  {
    int all_lines = 1;
    if (ih->data->mark_mode == IMAT_MARK_LIN)
      return 0;

    value++; /* skip C mark */
    if ((int)strlen(value) != ih->data->columns.num-1)
      return 0;

    markedit_cb = (IFniii)IupGetCallback(ih, "MARKEDIT_CB");

    for(col = 1; col < ih->data->columns.num; col++)
    {
      if (*value++ == '1')
        mark = 1;
      else
      {
        mark = 0;
        all_lines = 0; /* if one column is unmarked, then all lines are unmarked */
      }

      /* mark all the cells for that column */
      if (ih->data->mark_mode == IMAT_MARK_CELL)
      {
        for(lin = 1; lin < ih->data->lines.num; lin++)
          iupMatrixMarkCellSet2(ih, lin, col, mark, markedit_cb, str);
      }

      if (mark)
        ih->data->columns.flags[col] |= IUPMAT_MARK;
      else
        ih->data->columns.flags[col] &= ~IUPMAT_MARK;
    }

    iMatrixMarkAllLinCol(&(ih->data->lines), all_lines);
  }
  else if (*value == 'L' || *value == 'l')  /* lines */
  {
    int all_columns = 1;
    if (ih->data->mark_mode == IMAT_MARK_COL)
      return 0;

    value++; /* skip L mark */
    if ((int)strlen(value) != ih->data->lines.num-1)
      return 0;

    markedit_cb = (IFniii)IupGetCallback(ih, "MARKEDIT_CB");

    for(lin = 1; lin < ih->data->lines.num; lin++)
    {
      if (*value++ == '1')
        mark = 1;
      else
      {
        mark = 0;
        all_columns = 0;  /* if one line is unmarked, then all columns are unmarked */
      }

      /* Mark all the cells for that line */
      if (ih->data->mark_mode == IMAT_MARK_CELL)
      {
        for(col = 1; col < ih->data->columns.num; col++)
          iupMatrixMarkCellSet2(ih, lin, col, mark, markedit_cb, str);
      }

      if (mark)
        ih->data->lines.flags[lin] |= IUPMAT_MARK;
      else
        ih->data->lines.flags[lin] &= ~IUPMAT_MARK;
    }

    iMatrixMarkAllLinCol(&(ih->data->columns), all_columns);
  }
  else if (ih->data->mark_mode == IMAT_MARK_CELL)  /* cells */
  {
    if ((int)strlen(value) != (ih->data->lines.num-1)*(ih->data->columns.num-1))
      return 0;

    markedit_cb = (IFniii)IupGetCallback(ih, "MARKEDIT_CB");

    /* mark all columns to unmark during cell processing */
    iMatrixMarkAllLinCol(&(ih->data->columns), 1);

    for(lin = 1; lin < ih->data->lines.num; lin++)
    {
      int mark_line = 1;

      for(col = 1; col < ih->data->columns.num; col++)
      {
        if (*value++ == '1')
          mark = 1;
        else
        {
          mark = 0;
          mark_line = 0;     /* if one cell at the line is unmarked, then the line is unmarked */
          ih->data->columns.flags[col] &= ~IUPMAT_MARK;  /* if one cell at the column is unmarked, then the column is unmarked */
        }

        iupMatrixMarkCellSet2(ih, lin, col, mark, markedit_cb, str);
      }

      if (mark_line)
        ih->data->lines.flags[lin] |= IUPMAT_MARK;
      else
        ih->data->lines.flags[lin] &= ~IUPMAT_MARK;
    }
  }

  if (ih->handle)
    iupMatrixDraw(ih, 1);

  return 0;
}

char* iupMatrixGetMarkedAttrib(Ihandle* ih)
{
  int lin, col, size;
  IFnii mark_cb;
  char str[100];
  char* p, *value = NULL;
  int exist_mark = 0;           /* Show if there is someone marked */

  if (ih->data->mark_mode == IMAT_MARK_NO)
    return NULL;

  mark_cb = (IFnii)IupGetCallback(ih, "MARK_CB");

  if (ih->data->mark_mode == IMAT_MARK_CELL)
  {
    size = (ih->data->lines.num-1) * (ih->data->columns.num-1) + 1;
    value = iupStrGetMemory(size);
    p = value;

    for(lin = 1; lin < ih->data->lines.num; lin++)
    {
      for(col = 1; col < ih->data->columns.num; col++)
      {
         if (iupMatrixMarkCellGet(ih, lin, col, mark_cb, str))
         {
           exist_mark = 1;
           *p++ = '1';
         }
         else
           *p++ = '0';
      }
    }
    *p = 0;
  }
  else
  {
    int marked_lines = 0, marked_cols = 0;

    if (ih->data->mark_mode == IMAT_MARK_LINCOL) /* must find which format to return */
    {
      /* look for a marked column */
      for(col = 1; col < ih->data->columns.num; col++)
      {
        if (ih->data->columns.flags[col] & IUPMAT_MARK)
        {
          marked_cols = 1; /* at least one column is marked */
          break;
        }
      }

      if (!marked_cols)
        marked_lines = 1;
    }
    else if (ih->data->mark_mode == IMAT_MARK_LIN)
      marked_lines = 1;
    else if (ih->data->mark_mode == IMAT_MARK_COL)
      marked_cols = 1;

    if (marked_lines)
    {
      size = 1 + (ih->data->lines.num-1) + 1;
      value = iupStrGetMemory(size);
      p = value;

      *p++ = 'L';

      for(lin = 1; lin < ih->data->lines.num; lin++)
      {
        if (ih->data->lines.flags[lin] & IUPMAT_MARK)
        {
          exist_mark = 1;
          *p++ = '1';
        }
        else
         *p++ = '0';
      }
      *p = 0;
    }
    else if (marked_cols)
    {
      size = 1 + (ih->data->columns.num-1) + 1;
      value = iupStrGetMemory(size);
      p = value;

      *p++ = 'C';

      for(col = 1; col < ih->data->columns.num; col++)
      {
        if (ih->data->columns.flags[col] & IUPMAT_MARK)
        {
          exist_mark = 1;
          *p++ = '1';
        }
        else
         *p++ = '0';
      }
      *p = 0;
    }
  }

  return exist_mark? value: NULL;
}

int iupMatrixSetMarkAttrib(Ihandle* ih, const char* name_id, const char* value)
{
  int lin = 0, col = 0;

  if (ih->data->mark_mode != IMAT_MARK_CELL)
    return 0;

  if (iupStrToIntInt(name_id, &lin, &col, ':') == 2)
  {
    int mark;

    if (!iupMatrixCheckCellPos(ih, lin, col))
      return 0;

    if (lin == 0 || col == 0) /* title can NOT have a mark */
      return 0;

    mark = iupStrBoolean(value);

    if (ih->data->callback_mode)
    {
      IFniii markedit_cb = (IFniii)IupGetCallback(ih, "MARKEDIT_CB");
      if (markedit_cb)
        markedit_cb(ih, lin, col, mark);
      else if (mark)
        return 1;  /* store the attribute */
    }
    else
    {
      if (mark)
        ih->data->cells[lin][col].flags |= IUPMAT_MARK;
      else
        ih->data->cells[lin][col].flags &= ~IUPMAT_MARK;
    }

    if (!mark)
    {
      ih->data->lines.flags[lin] &= ~IUPMAT_MARK;
      ih->data->columns.flags[col] &= ~IUPMAT_MARK;
    }

    if (ih->handle)
    {
      /* This assumes that the matrix has been draw completely previously */
      iupMatrixStoreGlobalAttrib(ih);
      iupMatrixDrawCell(ih, lin, col);
    }
  }

  return 0;
}

char* iupMatrixGetMarkAttrib(Ihandle* ih, const char* name_id)
{
  int lin = 0, col = 0;

  if (ih->data->mark_mode == IMAT_MARK_NO)
    return "0";

  if (iupStrToIntInt(name_id, &lin, &col, ':') == 2)
  {
    if (!iupMatrixCheckCellPos(ih, lin, col))
      return NULL;

    if (lin == 0 || col == 0) /* title can NOT have the focus */
      return NULL;

    if (ih->data->mark_mode == IMAT_MARK_CELL)
    {
      if (ih->data->callback_mode)
      {
        IFnii mark_cb = (IFnii)IupGetCallback(ih, "MARK_CB");
        if (mark_cb)
        {
          if (mark_cb(ih, lin, col))
            return "1";
          else
            return "0";
        }
        else
          return NULL;  /* let check the hash table */
      }
      else
      {
        if (ih->data->cells[lin][col].flags & IUPMAT_MARK)
          return "1";
        else
          return "0";
      }
    }
    else
    {
      if (ih->data->lines.flags[lin] & IUPMAT_MARK || ih->data->columns.flags[col] & IUPMAT_MARK)
        return "1";
      else
        return "0";
    }
  }

  return NULL;
}
