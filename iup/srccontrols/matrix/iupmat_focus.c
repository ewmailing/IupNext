/** \file
 * \brief iupmatrix focus control
 *
 * See Copyright Notice in "iup.h"
 */
 
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

#include "iupmat_cd.h"
#include "iupmat_def.h"
#include "iupmat_aux.h"
#include "iupmat_draw.h"
#include "iupmat_edit.h"
#include "iupmat_focus.h"
#include "iupmat_mark.h"
#include "iupmat_scroll.h"
#include "iupmat_getset.h"

/* Verify if the cell that will contain the focus
   is in some visible area of the matrix.
*/
int iupMatrixFocusIsFocusVisible(Ihandle* ih)
{
  if (!iupMatrixAuxIsCellVisible(ih, ih->data->lin.active, ih->data->col.active))
    return 0;

  return 1;
}

/* Show the focus representation in the cell that contain (ih->data->lin.active and
   ih->data->col.active).
   This function just put the focus representation, DO NOT redraw the cell,
   (who called the function must do it...)
   Set the IUP focus to the matrix or to the edit field
*/
void iupMatrixFocusShowFocus(Ihandle* ih)
{
  if(!iupMatrixFocusIsFocusVisible(ih) || !ih->data->hasiupfocus)
    return;

  iupMatrixDrawFocus(ih, ih->data->lin.active, ih->data->col.active, 1);
}

/* Hide the focus representation */
void iupMatrixFocusHideFocus(Ihandle* ih)
{
  if(iupMatrixFocusIsFocusVisible(ih)) 
    iupMatrixDrawFocus(ih, ih->data->lin.active, ih->data->col.active, 0);
}

/* Set which cell contains the focus. DO NOT show the focus
   representation... iupMatrixFocusShowFocus is responsible for it.
   MUST NOT be call when the focus representation is visible
   in the screen
   -> lin, col - cell coordinates to receive the focus
*/
void iupMatrixFocusSetFocusPos(Ihandle* ih, int lin, int col)
{
  ih->data->lin.active = lin;
  ih->data->col.active = col;
}

/* Change the focus position... just a compact way to the
   many cases where hide/set/show should be called in sequence
   -> lin, col - cell coordinates to receive the focus
*/
void iupMatrixFocusHideSetShowFocus(Ihandle* ih, int lin, int col)
{
  iupMatrixFocusHideFocus(ih);
  iupMatrixFocusSetFocusPos(ih, lin, col);
  iupMatrixFocusShowFocus(ih);
}

/* Set the focus position and show it... just a compact way to the
   many cases where set/show should be called in sequence
   -> lin, col - cell coordinates to receive the focus
*/
void iupMatrixFocusSetShowFocus(Ihandle* ih, int lin, int col)
{
  iupMatrixFocusSetFocusPos(ih, lin, col);
  iupMatrixFocusShowFocus(ih);
}

int iupMatrixFocusFocusCB(Ihandle* ih, int focus)
{
  int err;
  int rc = IUP_DEFAULT;

  /* avoid callback inheritance */
  if (!iupStrEqual(IupGetClassType(ih), "matrix"))
    return IUP_IGNORE;

#ifdef _MOTIF_
  if(focus && IupGetInt(ih, "_IUPMATRIX_DOUBLE_CLICK"))
  {
    IupSetAttribute(ih, "_IUPMATRIX_DOUBLE_CLICK", NULL);
    return IUP_DEFAULT;
  }
#endif

  IsCanvasSet(ih, err);
  if(err == CD_OK)
  {
    if(focus)
    {
      iupMatrixSetSbV(ih);
      iupMatrixSetSbH(ih);

      ih->data->hasiupfocus = 1;
      iupMatrixFocusShowFocus(ih);
    }
    else
    {
      ih->data->hasiupfocus = 0;
      iupMatrixFocusHideFocus(ih);
    }

    if (ih->data->redraw)
    {
      cdCanvasFlush(ih->data->cddbuffer);
      ih->data->redraw = 0;
    }
  }

  if(focus)
    iupMatrixAuxCallEntercellCb(ih);
  else
    iupMatrixAuxCallLeavecellCb(ih);

  return rc;
}
