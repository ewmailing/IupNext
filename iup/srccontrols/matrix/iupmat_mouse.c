/** \file
 * \brief iupmatrix control
 * mouse events
 *
 * See Copyright Notice in "iup.h"
 */

/**************************************************************************/
/* Functions to handle mouse events                                       */
/**************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "iup.h"
#include "iupcbs.h"
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
#include "iupmat_colres.h"
#include "iupmat_aux.h"
#include "iupmat_focus.h"
#include "iupmat_mouse.h"
#include "iupmat_key.h"
#include "iupmat_mark.h"
#include "iupmat_getset.h"
#include "iupmat_edit.h"


/* Is the left button pressed? */
static int LeftPressed  = 0;

/* This variables store the line and column positions where
   the right button was pressed
*/
static int RightClickLin;
static int RightClickCol;

/**************************************************************************/
/* Private functions                                                      */
/**************************************************************************/

/* Call the user callback to the mouse move event on the matrix.
   -> lin, col : cell coordinates
*/
static void iMatrixMouseCallMoveCb(Ihandle* ih, int lin, int col)
{
   IFnii cb;

   if(lin < -3 || col < -3)
     return;

   cb = (IFnii)IupGetCallback(ih, "MOUSEMOVE_CB");

   if(cb)
     cb(ih, lin + 1, col + 1);
}

/* Call the user callback to the mouse click event on a cell.
   -> lin, col : cell coordinates
*/
static int iMatrixMouseCallClickCb(Ihandle* ih, int press, int x, int y, char* r)
{
  IFniis cb;

  if(press)
    cb = (IFniis)IupGetCallback(ih, "CLICK_CB");
  else
    cb = (IFniis)IupGetCallback(ih, "RELEASE_CB");

  if(cb)
  { 
    int lin, col;
    iupMatrixAuxGetLineCol(ih, x, y, &lin, &col);
    return cb(ih, lin + 1, col + 1, r);  /* internally : lin = -1 -> title line   */
  }                                      /*              col = -1 -> column title */
                       
  return IUP_DEFAULT;
}

/* Action when the mouse is moved on the matrix.
   Verify if there is a resize is being done on a column.
   Change the cursor format if it is passed on a join involving two
   column titles (when RESIZEMATRIX = YES)
   Make the cell selection, when necessary.
   -> x, y : mouse coordinates (canvas coordinates)
*/
static void iMatrixMouseMouseMove(Ihandle* ih, int x, int y)
{
  int mult = !iupAttribGetInt(ih, "MULTIPLE") ? 0 : 1;

  if(LeftPressed && mult)
  {
    /* the right canvas is activated inside iupMatrixMarkDrag */
    iupMatrixMarkDrag(ih, x, y);
  }
  else if(iupMatrixColResResizing()) /* Make a resize in a column size */
  {
    /* the right canvas is activated inside iupMatrixColResMove */
    iupMatrixColResMove(ih, x);
  }
  else /* Change cursor when it is passed on a join involving column titles */
  {
    iupMatrixColResChangeCursor(ih, x, y);
  }
}

/* Function calls by iMatrixMouseLeftPress to handle selections with the shift key pressed
   Unchecked (unmarked) the previous block, and mark a new block, starting in the
   clicked position until the focus position. If there is more than one block,
   just unchecked that block was being marked last.
   DO NOT change the focus position...
   -> lin, col : cell coordinates
*/
static void iMatrixMouseLeftClickShift(Ihandle* ih, int lin, int col)
{
  iupMatrixMarkUncheckedBlock(ih);
  iupMatrixMarkBlock(ih, lin, col);

  /* Redraw all the visible area */
  iupMatrixDrawMatrix(ih, IMAT_DRAW_ALL);

  /* Redraw erases the focus border */
  iupMatrixDrawFocus(ih, ih->data->lin.active, ih->data->col.active, 1);
}

/* This function handles the pressing of the button. Called by iMatrixMouseLeftPress.
   -> lin, col : cell coordinates
   -> ctrl : status of CTRL key (1 is pressed, 0 is not)
   -> dclick: show if it was or not double click.
   -> mark_mode : mark (selection) mode.
*/
static void iMatrixMouseLeftClick(Ihandle* ih, int lin, int col, int ctrl, int dclick, int mark_mode)
{
  if(dclick)
  {
    LeftPressed = 0;
    iupMatrixMarkReset();

    /* if a double click NOT in the current cell */
    if(lin != ih->data->lin.active || col != ih->data->col.active)
    {
      /* leave the previous cell if the matrix previously had the focus */
      if(ih->data->hasiupfocus && iupMatrixAuxCallLeavecellCb(ih) == IUP_IGNORE)
        return;

      iupMatrixFocusSetFocusPos(ih, lin, col);
      iupMatrixAuxCallEntercellCb(ih);
    }
    
    if(iupMatrixEditShow(ih))
    {
      if(ih->data->datah == ih->data->droph) 
        IupSetAttribute(ih->data->datah, "SHOWDROPDOWN", "YES");

#ifdef _MOTIF_
      if(atoi(IupGetGlobal("MOTIFNUMBER")) < 2203) /* since OpenMotif version 2.2.3 this is not necessary */
        IupSetAttribute(ih, "_IUPMATRIX_DOUBLE_CLICK", "1");
#endif
    }
  }
  else /* single click */
  {
    int mark_show;
    int oldlin = ih->data->lin.active;
    int oldcol = ih->data->col.active;

    /* leave the previous cell if the matrix previously had the focus */
    if(ih->data->hasiupfocus && iupMatrixAuxCallLeavecellCb(ih) == IUP_IGNORE)
      return;

    iupMatrixFocusHideFocus(ih);

    /* hide the cell selection, if appropriated... */
    mark_show = iupMatrixMarkHide(ih, ctrl);

    iupMatrixFocusSetFocusPos(ih, lin, col);

    /* show the cell selection, if appropriated... */
    if((mark_mode != IMAT_MARK_NO) && mark_show)
      iupMatrixMarkShow(ih, ctrl, lin, col, oldlin, oldcol);

    iupMatrixAuxCallEntercellCb(ih);
    iupMatrixFocusShowFocus(ih);
  }
}

static void iMatrixMouseLeftPress(Ihandle* ih, int x, int y, int shift, int ctrl, int dclick)
{
  int lin, col;
  int mark_mode;

  iupMatrixKeyResetKeyCount();

  if(iupMatrixColResTry(ih, x, y))
    return;                      /* Resize of the width a of a column was started */

  /* If x, y are not of a cell of the matrix, return.
     If the cell are -1,-1 (left top corner), return.
  */
  if(!iupMatrixAuxGetLineCol(ih, x, y, &lin, &col) || lin == -2 || col == -2 ||
     (lin == -1 && col == -1))
    return;

  /* Apply a set of relevant critics when the selection (mark) is on. The critics works
     changing the value of the control variables: shift, ctrl, ...
  */
  mark_mode = iupMatrixMarkCritica(ih, &lin, &col, &shift, &ctrl, &dclick);

  LeftPressed = 1;

  if(shift)
    iMatrixMouseLeftClickShift(ih, lin, col);
  else
    iMatrixMouseLeftClick(ih, lin, col, ctrl, dclick, mark_mode);
}

/* This function is called when the left button is released.
   -> x, y : mouse coordinates (canvas coordinates)
*/
static void iMatrixMouseLeftRelease(Ihandle* ih, int x)
{
  LeftPressed = 0;   /* The left button is released */

  iupMatrixMarkReset();

  if(iupMatrixColResResizing())  /* If it was made a column resize, finish it */
    iupMatrixColResFinish(ih, x);
}

/* This function is called when the mouse button is pressed.
   Stores the cell that was pressed, comparing when the button is released.
   -> x, y : mouse coordinates (canvas coordinates)
*/
static void iMatrixMouseRightPress(Ihandle* ih, int x, int y)
{
  int lin, col;

  if(iupMatrixAuxGetLineCol(ih, x, y, &lin, &col) && (lin!= -2) && (col != -2))
  {
    RightClickLin = lin;
    RightClickCol = col;
  }
  else
  {
    RightClickLin = -9; /* this value never be return by iupMatrixAuxGetLineCol */
    RightClickCol = -9;
  }
}

/* This function is called when the right mouse button is released.
   Calls a user callback if the button was released in the same cell in which
   it was pressed. 
   -> x, y : mouse coordinates (canvas coordinates)
*/
static void iMatrixMouseRightRelease(Ihandle* ih, int x, int y)
{
  int lin, col;

  if(iupMatrixAuxGetLineCol(ih, x, y, &lin, &col) &&
     (lin == RightClickLin) && (col == RightClickCol));
}

/**************************************************************************/
/* Exported functions                                                     */
/**************************************************************************/

/* Callback calls when a mouse button is pressed or released.
   -> b : identify the mouse button [IUP_BUTTON1, IUP_BUTTON2 or IUP_BUTTON3],
   -> press : 1 for the pressing event, 0 for releasing event
   -> x, y : mouse position
   -> r : string that contain which keys are pressed [SHIFT,CTRL and ALT]
*/
int iupMatrixMouseButtonCB (Ihandle* ih, int b, int press, int x, int y, char* r)
{
  int ret = IUP_DEFAULT;

  if(press)
  {
    /* The edit Kill Focus is not called when the user clicks in the parent canvas. 
       so we have to compensate that. */
    iupMatrixEditCheckHidden(ih);

    ih->data->hasiupfocus = 1;
  }

  if(b == IUP_BUTTON1)
  {
    if(press)
    {
      int dclick = isdouble(r);
      iMatrixMouseLeftPress(ih, x, y, isshift(r), iscontrol(r), dclick);
    }
    else
      iMatrixMouseLeftRelease(ih, x);
  }
  else if(b == IUP_BUTTON3)
  {
    if(press)
      iMatrixMouseRightPress(ih, x, y);
    else
      iMatrixMouseRightRelease(ih, x, y);
  }

  ret = iMatrixMouseCallClickCb(ih, press, x, y, r);
  if(ret == IUP_IGNORE)
    return ret;

  if(ih->data->redraw)
  {
    cdCanvasFlush(ih->data->cddbuffer); 
    ih->data->redraw = 0; 
  }
  return ret;
}

/* Callback calls when a mouse is moved.
   -> x, y : mouse position.
%o Retorna IUP_DEFAULT.
*/
int iupMatrixMouseMoveCB(Ihandle* ih, int x, int y)
{
  int lin, col;

  iMatrixMouseMouseMove(ih, x, y);

  iupMatrixAuxGetLineCol(ih, x, y, &lin, &col);
  iMatrixMouseCallMoveCb(ih, lin, col);

  return IUP_DEFAULT;
}
