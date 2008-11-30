/** \file
 * \brief iupmatrix setget control
 * attributes set and get
 *
 * See Copyright Notice in iup.h
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

#include "iupmat_def.h"
#include "iupmat_cd.h"
#include "iupmat_draw.h"
#include "iupmat_scroll.h"
#include "iupmat_aux.h"
#include "iupmat_mem.h"
#include "iupmat_focus.h"
#include "iupmat_mark.h"
#include "iupmat_getset.h"
#include "iupmat_edit.h"


/* Set a cell value of the matrix, redraw a cell if it is visible.
   -> lin, col : cell coordinates to be modify, (1,1) corresponds to the
                 left top corner cell
   -> v : string with the new cell value
*/
void* iupMatrixGSSetCell(Ihandle* ih, int lin ,int col, const char* v)
{
  int cellvisible;
  int visible = IupGetInt(ih, "VISIBLE");

  int err;

  if(v == NULL)
    v = "";

  IsCanvasSet(ih, err);

  /* If the cell doesn't exist, returns NULL */
  if((lin < 1) || (col < 1) || (lin > ih->data->lin.num) || (col > ih->data->col.num))
    return NULL;

  lin--;  /* the left top cell is 1:1 for the user... */
  col--;  /* internally, is 0:0                       */

  cellvisible = iupMatrixAuxIsCellVisible(ih, lin, col);

  if(ih->data->valeditcb)
  {
    ih->data->valeditcb(ih, lin+1, col+1, (char*)v);
  }
  else if(!ih->data->valcb)
  {
    iupMatrixMemAlocCell(ih, lin, col, strlen(v));
    strcpy(ih->data->v[lin][col].value, v);
  }

  if(visible && cellvisible && err == CD_OK)
  {
    iupMatrixDrawCells(ih, lin, col, lin, col);
    iupMatrixFocusShowFocus(ih);
  }

  return (char*)v;
}

/* Return a cell value of the matrix.
   -> lin, col : cell coordinates to be modify, (1,1) corresponds to the
                 left top corner cell

   Used only by getattribute method.
*/
char* iupMatrixGSGetCell (Ihandle* ih, int lin, int col)
{
  /* If the cell doesn't exist, returns NULL */
  if((lin < 0) || (col < 0) || (lin > ih->data->lin.num) || (col > ih->data->col.num))
    return NULL;

  lin--;  /* the left top cell is 1:1 for the user... */ 
  col--;  /* internally, is 0:0                       */ 

  if(lin == ih->data->lin.active && col == ih->data->col.active && IupGetInt(ih->data->datah, "VISIBLE"))
    return iupMatrixEditGetValue(ih);
  else
    return iupMatrixAuxGetCellValue(ih, lin, col);
}

/* Set the cell that contains the focus. If the cell is not visible,
   change the visible part of the matrix to contain the cell as the focus.
   -> v : string that stores the cell will receive the focus.
          Its format is "%d:%d", when the first number indicates the line,
          and the second, the column. "1:1" corresponds the left top
          corner cell.
*/
void iupMatrixGSSetFocusPosition(Ihandle* ih, const char* v, int call_cb)
{
  int lin, col;
  int redraw = 0;
  int visible = IupGetInt(ih, "VISIBLE");
  int err;

  IsCanvasSet(ih, err);

  if(!v)
    return;

  if(iupStrToIntInt(v, &lin, &col, ':') != 2)
    return;

  /* If the cell doesn't exist, returns */
  if((lin < 1) || (col < 1) || (lin > ih->data->lin.num) || (col > ih->data->col.num))
    return;

  lin--;  /* the left top cell is 1:1 for the user... */
  col--;  /* internally, is 0:0                       */

  if(ih->data->col.wh[col] == 0)
    return;

  /* if the matrix is focused, it ask permission for the application to
     leave the cell. When the matrix is not focused, the application already
     has been notified of the loss of focus
   */
  if(ih->data->hasiupfocus && call_cb)
  {
    if(iupMatrixAuxCallLeavecellCb(ih) == IUP_IGNORE)
      return;
  }

  /* If the line is invisible... */
  if(lin < ih->data->lin.first || lin > ih->data->lin.last)
  {
    /* the first line will contain the focus */
    ih->data->lin.first = lin;
    iupMatrixAuxGetLastWidth(ih, IMAT_MAT_LIN);
    redraw = 1;
  }

  /* If the column is invisible...  */
  if(col < ih->data->col.first || col > ih->data->col.last)
  {
    /* the first column will contain the focus */
    ih->data->col.first = col;
    iupMatrixAuxGetLastWidth(ih, IMAT_MAT_COL);
    redraw = 1;
  }

  if(visible && err == CD_OK)
  {
    iupMatrixFocusHideFocus(ih);
    iupMatrixFocusSetFocusPos(ih, lin, col);

    if(redraw)
      iupMatrixDrawMatrix(ih, IMAT_DRAW_ALL);
    iupMatrixFocusShowFocus(ih);
  }

  /* If the matrix is focused, notify the application
     about the new cell that contain the focus. If not,
     when the matrix receives the focus, then the
     application will be notified.
  */
  if(ih->data->hasiupfocus && call_cb)
   iupMatrixAuxCallEntercellCb(ih);
}

/* Get the cell that contain the focus.
   The string format that stores the cell is "%d:%d",
   when the first number indicates the line, and the second,
   the column. "1:1" corresponds the left top corner cell.
*/
char* iupMatrixGSGetFocusPosition(Ihandle* ih)
{
  char* cell = iupStrGetMemory(100);
  
  if(ih)
   sprintf(cell, "%d:%d", ih->data->lin.active+1, ih->data->col.active+1);
  else
   return NULL;

  return cell;
}

/* Change the text alignment in a specific column. Redraw the column, if it
   is visible.
   -> col : column that will have changed its alignment
            col = 1 corresponds to the first column of the matrix
*/
void iupMatrixGSSetColAlign (Ihandle* ih, int col)
{
  int visible = IupGetInt(ih, "VISIBLE");
  int err;

  IsCanvasSet(ih, err);

  if(col > ih->data->col.num || col < 0)
    return;

  if(!visible || err != CD_OK)
    return;

  if(col == 0) /* Title column alignment */
  {
    iupMatrixDrawLineTitle(ih, ih->data->lin.first, ih->data->lin.last);
  }
  else
  {
    col--;  /* the left top cell is 1:1 to the user, internally is 0:0 */
    /* If the column is in the visible part, redraw the matrix */
    if((col >= ih->data->col.first) && (col <= ih->data->col.last))
    {
      iupMatrixDrawCells(ih, ih->data->lin.first, col, ih->data->lin.last, col);
      iupMatrixFocusShowFocus(ih);
    }
  }
}

/* Redraw the title if it is visible, when the line title is changed.
   lin : line that had its title changed. lin = 1 indicates that the
         first line title was changed. In other hand, lin = 0 indicates
         that the cell title (localized in the corner between line and
         column titles) was modified.
*/
void iupMatrixGSSetTitleLine(Ihandle* ih, int lin)
{
 int visible = IupGetInt(ih, "VISIBLE");
 int err;

 if(lin > ih->data->lin.num || lin < 0)
   return;

 IsCanvasSet(ih, err);
 if(visible && err == CD_OK)
 {
   /* If it is the corner title, and the corner exists... */
   if(lin == 0)
   {
     /* Redraw the corner between line and column titles */
     iupMatrixDrawTitleCorner(ih);
   }
   else
   {
     lin--;
     /* Redraw the line title, if it is visible */
     iupMatrixDrawLineTitle(ih, lin, lin);
   }
 }
}

/* Redraw the title if it is visible, when the column title is changed.
   col : column that had its title changed. col = 1 indicates that the
         first line title was changed
*/
void iupMatrixGSSetTitleColumn(Ihandle* ih, int col)
{
 int visible = IupGetInt(ih, "VISIBLE");
 int err;

 if(col > ih->data->col.num || col < 1)
   return;

 col--;

 IsCanvasSet(ih, err);
 if(visible && err == CD_OK)
 {
   /* Redraw the column title, if it is visible */
   iupMatrixDrawColumnTitle(ih, col, col);
 }
}

/* Set which the visible area of the matrix. Uses the information about the
   cell must be placed at the left top corner of the matrix.
   -> value - information about the cell must be placed at the left top
              corner of the matrix, codified in lin:col format
*/
void iupMatrixGSSetOrigin(Ihandle* ih, const char* value)
{
 int visible = IupGetInt(ih, "VISIBLE");
 int err;
 int lin = -1, col = -1, oldlin, oldcol, num = 0;

 if(value == NULL)
   return;

 /* Get the parameters. The '*' indicates that want to keep the table in
    the same line or column
 */
 if(iupStrToIntInt(value, &lin, &col, ':') != 2)
 {
   if(lin != -1)
     col = ih->data->col.first + 1;
   else if(col != -1)
     lin = ih->data->lin.first + 1;
   else
     return;
 }

  /* If the cell doesn't exist, returns NULL */
 if((lin < 1) || (col < 1) || (lin > ih->data->lin.num) || (col > ih->data->col.num))
   return;

 /* TODO: allow position based on the title cells also */

 lin--;  /* the left top cell is 1:1 for the user... */
 col--;  /* internally, is 0:0                       */

 IsCanvasSet(ih, err);

 oldcol = ih->data->col.first;
 oldlin = ih->data->lin.first;

 ih->data->col.first = col;
 iupMatrixAuxGetLastWidth(ih, IMAT_MAT_COL);
 iupMatrixAuxGetPos(ih, IMAT_MAT_COL);

 ih->data->lin.first = lin;
 iupMatrixAuxGetLastWidth(ih, IMAT_MAT_LIN);
 iupMatrixAuxGetPos(ih, IMAT_MAT_LIN);

 num = abs(oldcol - ih->data->col.first) + abs(oldlin - ih->data->lin.first);

 /* If the matrix is visible, and there is changes in same visible area,
    then redraw...
 */
 if(visible && (err == CD_OK) && num)
 {
  iupMatrixFocusHideFocus(ih);

  if(num == 1)
  {
    if(oldlin != ih->data->lin.first)
      iupMatrixScroll(ih, oldlin < ih->data->lin.first ? IMAT_SCROLL_DOWN  : IMAT_SCROLL_UP,   oldlin, 1);
    else
      iupMatrixScroll(ih, oldcol < ih->data->col.first ? IMAT_SCROLL_RIGHT : IMAT_SCROLL_LEFT, oldcol, 1);

    iupMatrixSetSbV(ih);
    iupMatrixSetSbH(ih);
  }
  else
  {
    iupMatrixSetSbV(ih);
    iupMatrixSetSbH(ih);

    iupMatrixDrawMatrix(ih, IMAT_DRAW_ALL);
  }

  iupMatrixFocusShowFocus(ih);
  ih->data->redraw = 1;
 }
}

/* Return the left top cell coordinate of the current visible part of
   the matrix.
*/
char* iupMatrixGSGetOrigin(Ihandle* ih)
{
  char* val = iupStrGetMemory(100);

  sprintf(val, "%d:%d", ih->data->lin.first + 1, ih->data->col.first + 1);
  return val;
}

/* Active/Inactive a line or column of the matrix
   -> mode   - Flag indicates if it is operating in lines or columns
               [IMAT_MAT_LIN|IMAT_MAT_COL]
   -> lincol - Number of line or column (IUP format)
   -> val    - Attribute value
*/
void iupMatrixGSSetActive(Ihandle* ih, int mode, int lincol, const char* val)
{
  int on = iupStrBoolean(val);
  int visible = IupGetInt(ih, "VISIBLE");
  int err;

  lincol--;

  if(mode == IMAT_MAT_LIN)
    ih->data->lin.inactive[lincol] = !on;
  else
    ih->data->col.inactive[lincol] = !on;

  IsCanvasSet(ih,err);
  if(visible && (err == CD_OK))
  {
    if(mode == IMAT_MAT_LIN)
    {
      iupMatrixDrawLineTitle(ih, lincol, lincol);
      iupMatrixDrawCells(ih, lincol, ih->data->col.first, lincol, ih->data->col.last);
    }
    else
    {
      iupMatrixDrawColumnTitle(ih, lincol, lincol);
      iupMatrixDrawCells(ih, ih->data->lin.first, lincol, ih->data->lin.last, lincol);
    }
  }
}

void iupMatrixGSSetRedraw(Ihandle* ih, const char* value)
{
  int visible = IupGetInt(ih, "VISIBLE");
  int err;
  int type = IMAT_DRAW_ALL;

  if(value == NULL)
    return;

  IsCanvasSet(ih, err);

  if(visible && (err == CD_OK))
  {
    if(value[0] == 'L' || value[0] == 'l')
      type = IMAT_DRAW_LIN;
    else if(value[0] == 'C' || value[0] == 'c')
      type = IMAT_DRAW_COL;

    if(type != IMAT_DRAW_ALL)
    {
      int min = 0, max = 0;
      value++;

      if(iupStrToIntInt(value, &min, &max, ':') != 2)
        max = min;

      if(type == IMAT_DRAW_LIN)
        iupMatrixDrawCells(ih, min-1, ih->data->col.first, max-1, ih->data->col.last);
      else
        iupMatrixDrawCells(ih, ih->data->lin.first, min-1, ih->data->lin.last, max-1);
    }
    else
    {
      iupMatrixDrawMatrix(ih, IMAT_DRAW_ALL);
    }

    iupMatrixFocusShowFocus(ih);
  }
}

/* Functions to redraw of cells. */
void iupMatrixGSSetRedrawCell(Ihandle* ih, int lin, int col)
{
  int visible = IupGetInt(ih, "VISIBLE");
  int err;

  IsCanvasSet(ih, err);
  if(!visible || err != CD_OK)
    return;

  if(lin == -1) /* redraw a column */
  {
    iupMatrixFocusHideFocus(ih);
    iupMatrixDrawCells(ih, ih->data->lin.first, col-1, ih->data->lin.last, col-1);
    iupMatrixFocusShowFocus(ih);
  }
  else if(col == -1) /* redraw a line */
  {
    iupMatrixFocusHideFocus(ih);
    iupMatrixDrawCells(ih, lin-1, ih->data->col.first, lin-1, ih->data->col.last);
    iupMatrixFocusShowFocus(ih);
  }
  else /* redraw just a cell */
  {
     /* If the cell doesn't exist, returns NULL */
    if((lin < 0) || (col < 0) || (lin > ih->data->lin.num) || (col > ih->data->col.num))
      return;

    lin--;  /* the left top cell is 1:1 for the user... */ 
    col--;  /* internally, is 0:0                       */ 

    if(col == -1 && lin == -1) /* Title corner */
     iupMatrixDrawTitleCorner(ih);
    else if(col == -1) /* Line title */
     iupMatrixDrawLineTitle(ih, lin, lin);
    else if(lin == -1) /* Column title */
      iupMatrixDrawColumnTitle(ih, col, col);
    else if(iupMatrixAuxIsCellVisible(ih, lin, col))
    {
      iupMatrixDrawCells(ih, lin, col, lin, col);
      iupMatrixFocusShowFocus(ih);
    }
  }
}
