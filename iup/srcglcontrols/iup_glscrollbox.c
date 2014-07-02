/** \file
 * \brief GLScrollBox control
 *
 * See Copyright Notice in "iup.h"
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "iup.h"
#include "iupcbs.h"
#include "iupglcontrols.h"

#include "iup_object.h"
#include "iup_register.h"
#include "iup_attrib.h"
#include "iup_str.h"
#include "iup_layout.h"

#include "iup_glcontrols.h"

enum {
  SB_NONE, 
  SB_INC_X, SB_PAGEINC_X, SB_DRAG_X, SB_PAGEDEC_X, SB_DEC_X,
  SB_INC_Y, SB_PAGEINC_Y, SB_DRAG_Y, SB_PAGEDEC_Y, SB_DEC_Y
};

int iupGLScrollbarsGetSize(Ihandle* ih)
{
  return iupAttribGetInt(ih, "SCROLLBARSIZE");
}

void iupGLScrollbarsCalcScrollIntPos(double min, double max, double page, double pos,
  int imin, int imax, int *ipage, int *ipos)
{
  double range = max - min;
  int irange = imax - imin;
  double ratio = ((double)irange) / range;

  *ipage = (int)(page*ratio);
  if (*ipage > irange) *ipage = irange;
  if (*ipage < 1) *ipage = 1;

  if (ipos)
  {
    *ipos = (int)((pos - min)*ratio) + imin;
    if (*ipos < imin) *ipos = imin;
    if (*ipos >(imax - *ipage)) *ipos = imax - *ipage;
  }
}

void iupGLScrollbarsCalcScrollRealPos(double min, double max, double *pos,
  int imin, int imax, int ipage, int *ipos)
{
  double range = max - min;
  int irange = imax - imin;
  double ratio = ((double)irange) / range;

  if (*ipos < imin) *ipos = imin;
  if (*ipos >(imax - ipage)) *ipos = imax - ipage;

  *pos = min + ((double)(*ipos - imin)) / ratio;
}

int iupGLScrollbarsIsInsideHandlers(Ihandle* ih, int x, int y)
{
  int has_vert_scroll = 0;
  int has_horiz_scroll = 0;
  int sb_size = iupGLScrollbarsGetSize(ih);
  int width = ih->currentwidth,
    height = ih->currentheight;
  int sb_xmin = iupAttribGetInt(ih, "XMIN");
  int sb_xmax = iupAttribGetInt(ih, "XMAX");
  int sb_ymin = iupAttribGetInt(ih, "YMIN");
  int sb_ymax = iupAttribGetInt(ih, "YMAX");
  int sb_dy = iupAttribGetInt(ih, "DY");
  int sb_dx = iupAttribGetInt(ih, "DX");

  if (sb_xmax - sb_xmin > sb_dx)  /* has horizontal scrollbar */
  {
    has_horiz_scroll = 1;
    height -= sb_size;
  }

  if (sb_ymax - sb_ymin > sb_dy)  /* has vertical scrollbar */
  {
    has_vert_scroll = 1;
    width -= sb_size;
  }

  if (has_vert_scroll)
  {
    if (x >= ih->currentwidth - sb_size && x < ih->currentwidth)
    {
      int sb_posy = iupAttribGetInt(ih, "POSY");
      int ymin = 0;
      int ymax = height - 1;
      int dy   =  (sb_dy              * (ymax - ymin - 2 * sb_size)) / (sb_ymax - sb_ymin);
      int posy = ((sb_posy - sb_ymin) * (ymax - ymin - 2 * sb_size)) / (sb_ymax - sb_ymin);
      posy += ymin + sb_size;

      if (y < sb_size)
        return SB_DEC_Y;
      else if (y < posy)
        return SB_PAGEDEC_Y;
      else if (y < posy + dy)
        return SB_DRAG_Y;
      else if (y < height - sb_size)
        return SB_PAGEINC_Y;
      else if (y < height)
        return SB_INC_Y;

      return SB_NONE;
    }
  }

  if (has_horiz_scroll)
  {
    if (y >= ih->currentheight - sb_size && y < ih->currentheight)
    {
      int sb_posx = iupAttribGetInt(ih, "POSX");
      int xmin = 0;
      int xmax = width - 1;
      int dx   =  (sb_dx              * (xmax - xmin - 2 * sb_size)) / (sb_xmax - sb_xmin);
      int posx = ((sb_posx - sb_xmin) * (xmax - xmin - 2 * sb_size)) / (sb_xmax - sb_xmin);
      posx += xmin + sb_size;

      if (x < sb_size)
        return SB_DEC_X;
      else if (x < posx)
        return SB_PAGEDEC_X;
      else if (x < posx + dx)
        return SB_DRAG_X;
      else if (x < width - sb_size)
        return SB_PAGEINC_X;
      else if (x < width)
        return SB_INC_X;

      return SB_NONE;
    }
  }

  return SB_NONE;
}

static void iGLScrollbarsDrawVertical(Ihandle* ih, int active, const char* fgcolor, const char* bgcolor, int sb_ymin, int sb_ymax, int sb_dy, int has_horiz_scroll)
{
  int sb_size = iupGLScrollbarsGetSize(ih);
  int height = ih->currentheight;
  int xmin, xmax, ymin, ymax, posy, dy;
  int sb_posy = iupAttribGetInt(ih, "POSY");

  if (has_horiz_scroll)
    height -= sb_size;

  xmin = ih->currentwidth - sb_size;
  xmax = ih->currentwidth - 1;
  ymin = 0;
  ymax = height - 1;

  dy   =  (sb_dy              * (ymax - ymin - 2 * sb_size)) / (sb_ymax - sb_ymin);
  posy = ((sb_posy - sb_ymin) * (ymax - ymin - 2 * sb_size)) / (sb_ymax - sb_ymin);
  posy += ymin + sb_size;

  /* draw background */
  iupGLDrawBox(ih, xmin, xmax,
                   ymin, ymax, bgcolor, 1);

  /* draw arrows */
  iupGLDrawArrow(ih, xmin + 2, ymin + 2,           sb_size - 5, fgcolor, active, IUPGL_ARROW_TOP);
  iupGLDrawArrow(ih, xmin + 2, ymax - sb_size + 3, sb_size - 5, fgcolor, active, IUPGL_ARROW_BOTTOM);

  /* draw handler */
  iupGLDrawBox(ih, xmin + 2, xmax - 2,
                   posy, posy + dy, fgcolor, active);
}

static void iGLScrollbarsDrawHorizontal(Ihandle* ih, int active, const char* fgcolor, const char* bgcolor, int sb_xmin, int sb_xmax, int sb_dx, int has_vert_scroll)
{
  int sb_size = iupGLScrollbarsGetSize(ih);
  int width = ih->currentwidth;
  int xmin, xmax, ymin, ymax, posx, dx;
  int sb_posx = iupAttribGetInt(ih, "POSX");

  if (has_vert_scroll)
    width -= sb_size;

  xmin = 0;
  xmax = width - 1;
  ymin = ih->currentheight - sb_size;
  ymax = ih->currentheight - 1;

  dx   =  (sb_dx              * (xmax - xmin - 2 * sb_size)) / (sb_xmax - sb_xmin);
  posx = ((sb_posx - sb_xmin) * (xmax - xmin - 2 * sb_size)) / (sb_xmax - sb_xmin);
  posx += xmin + sb_size;

  /* draw background */
  iupGLDrawBox(ih, xmin, xmax,
                   ymin, ymax, bgcolor, 1);

  /* draw arrows */
  iupGLDrawArrow(ih, xmin + 2,           ymin + 2, sb_size - 5, fgcolor, active, IUPGL_ARROW_LEFT);
  iupGLDrawArrow(ih, xmax - sb_size + 3, ymin + 2, sb_size - 5, fgcolor, active, IUPGL_ARROW_RIGHT);

  /* draw handler */
  iupGLDrawBox(ih, posx, posx + dx,
                   ymin + 2, ymax - 2, fgcolor, active);
}

void iupGLScrollbarsDraw(Ihandle* ih, int active, const char* fgcolor, const char* bgcolor)
{
  int has_vert_scroll = 0;
  int has_horiz_scroll = 0;
  int sb_xmin = iupAttribGetInt(ih, "XMIN");
  int sb_xmax = iupAttribGetInt(ih, "XMAX");
  int sb_ymin = iupAttribGetInt(ih, "YMIN");
  int sb_ymax = iupAttribGetInt(ih, "YMAX");
  int sb_dx = iupAttribGetInt(ih, "DX");
  int sb_dy = iupAttribGetInt(ih, "DY");

  if (sb_xmax - sb_xmin > sb_dx)  /* has horizontal scrollbar */
    has_horiz_scroll = 1;

  if (sb_ymax - sb_ymin > sb_dy)  /* has vertical scrollbar */
    has_vert_scroll = 1;

  if (has_vert_scroll)
    iGLScrollbarsDrawVertical(ih, active, fgcolor, bgcolor, sb_ymin, sb_ymax, sb_dy, has_horiz_scroll);

  if (has_horiz_scroll)
    iGLScrollbarsDrawHorizontal(ih, active, fgcolor, bgcolor, sb_xmin, sb_xmax, sb_dx, has_vert_scroll);
}

void iupGLScrollbarsLayoutUpdate(Ihandle *ih)
{
  if (ih->firstchild)
  {
    int sb_posx = iupAttribGetInt(ih, "POSX");
    int sb_posy = iupAttribGetInt(ih, "POSY");

    iupBaseSetPosition(ih->firstchild, ih->x - sb_posx, ih->y - sb_posy);
    iupLayoutUpdate(ih->firstchild);
  }
}

void iupGLScrollbarsPressX(Ihandle* ih, int handler)
{
  int sb_xmax = iupAttribGetInt(ih, "XMAX");
  int sb_xmin = iupAttribGetInt(ih, "XMIN");
  int sb_dx = iupAttribGetInt(ih, "DX");
  int sb_posx = iupAttribGetInt(ih, "POSX");

  if (handler == SB_INC_X)
  {
    int sb_linex = sb_dx / 10;
    char* linex = iupAttribGet(ih, "LINEX");
    if (linex) iupStrToInt(linex, &sb_linex);
    sb_posx += sb_linex;
  }
  else if (handler == SB_PAGEINC_X)
    sb_posx += sb_dx;
  if (handler == SB_DEC_X)
  {
    int sb_linex = sb_dx / 10;
    char* linex = iupAttribGet(ih, "LINEX");
    if (linex) iupStrToInt(linex, &sb_linex);
    sb_posx -= sb_linex;
  }
  else if (handler == SB_PAGEDEC_X)
    sb_posx -= sb_dx;

  if (sb_posx < sb_xmin)
    sb_posx = sb_xmin;
  if (sb_posx > sb_xmax - sb_dx)
    sb_posx = sb_xmax - sb_dx;

  iupAttribSetInt(ih, "POSX", sb_posx);
}

void iupGLScrollbarsPressY(Ihandle* ih, int handler)
{
  int sb_ymax = iupAttribGetInt(ih, "YMAX");
  int sb_ymin = iupAttribGetInt(ih, "YMIN");
  int sb_dy = iupAttribGetInt(ih, "DY");
  int sb_posy = iupAttribGetInt(ih, "POSY");

  if (handler == SB_INC_Y)
  {
    int sb_liney = sb_dy / 10;
    char* liney = iupAttribGet(ih, "LINEY");
    if (liney) iupStrToInt(liney, &sb_liney);
    sb_posy += sb_liney;
  }
  else if (handler == SB_PAGEINC_Y)
    sb_posy += sb_dy;
  if (handler == SB_DEC_Y)
  {
    int sb_liney = sb_dy / 10;
    char* liney = iupAttribGet(ih, "LINEY");
    if (liney) iupStrToInt(liney, &sb_liney);
    sb_posy -= sb_liney;
  }
  else if (handler == SB_PAGEDEC_Y)
    sb_posy -= sb_dy;

  if (sb_posy < sb_ymin)
    sb_posy = sb_ymin;
  if (sb_posy > sb_ymax - sb_dy)
    sb_posy = sb_ymax - sb_dy;

  iupAttribSetInt(ih, "POSY", sb_posy);
}

int iupGLScrollbarsMove(Ihandle* ih, int diff_x, int diff_y, int start_pos, int handler)
{
  int has_vert_scroll = 0;
  int has_horiz_scroll = 0;
  int sb_xmin = iupAttribGetInt(ih, "XMIN");
  int sb_xmax = iupAttribGetInt(ih, "XMAX");
  int sb_ymin = iupAttribGetInt(ih, "YMIN");
  int sb_ymax = iupAttribGetInt(ih, "YMAX");
  int sb_dx = iupAttribGetInt(ih, "DX");
  int sb_dy = iupAttribGetInt(ih, "DY");
  int sb_size = iupGLScrollbarsGetSize(ih);

  if (sb_xmax - sb_xmin > sb_dx)  /* has horizontal scrollbar */
    has_horiz_scroll = 1;

  if (sb_ymax - sb_ymin > sb_dy)  /* has vertical scrollbar */
    has_vert_scroll = 1;

  if (handler == SB_DRAG_Y)
  {
    int ymin, ymax, posy, sb_posy;
    int height = ih->currentheight;
    if (has_horiz_scroll)
      height -= sb_size;

    if (diff_y == 0)
      return 0;

    ymin = 0;
    ymax = height - 1;

    posy = ((start_pos - sb_ymin) * (ymax - ymin - 2 * sb_size)) / (sb_ymax - sb_ymin);
    posy += ymin + sb_size;

    posy += diff_y;

    posy -= ymin + sb_size;
    sb_posy = (posy * (sb_ymax - sb_ymin)) / (ymax - ymin - 2 * sb_size) + sb_ymin;

    if (sb_posy < sb_ymin)
      sb_posy = sb_ymin;
    if (sb_posy > sb_ymax - sb_dy)
      sb_posy = sb_ymax - sb_dy;

    if (sb_posy != start_pos)
    {
      iupAttribSetInt(ih, "POSY", sb_posy);
      return 1;
    }
  }
  else if (handler == SB_DRAG_X)
  {
    int xmin, xmax, posx, sb_posx;
    int width = ih->currentwidth;
    if (has_vert_scroll)
      width -= sb_size;

    if (diff_x == 0)
      return 0;

    xmin = 0;
    xmax = width - 1;

    posx = ((start_pos - sb_xmin) * (xmax - xmin - 2 * sb_size)) / (sb_xmax - sb_xmin);
    posx += xmin + sb_size;

    posx += diff_x;

    posx -= xmin + sb_size;
    sb_posx = (posx * (sb_xmax - sb_xmin)) / (xmax - xmin - 2 * sb_size) + sb_xmin;

    if (sb_posx < sb_xmin)
      sb_posx = sb_xmin;
    if (sb_posx > sb_xmax - sb_dx)
      sb_posx = sb_xmax - sb_dx;

    if (sb_posx != start_pos)
    {
      iupAttribSetInt(ih, "POSX", sb_posx);
      return 1;
    }
  }

  return 0;
}

static int iGLScrollbarsSetPosXAttrib(Ihandle *ih, const char *value)
{
  int xmin, xmax, dx;
  int posx;

  if (!iupStrToInt(value, &posx))
    return 0;

  xmin = iupAttribGetInt(ih, "XMIN");
  xmax = iupAttribGetInt(ih, "XMAX");
  dx = iupAttribGetInt(ih, "DX");

  if (posx < xmin) posx = xmin;
  if (posx > xmax - dx) posx = xmax - dx;

  iupAttribSetInt(ih, "POSX", posx);

  iupGLScrollbarsLayoutUpdate(ih);
  return 0;
}

static int iGLScrollbarsSetPosYAttrib(Ihandle *ih, const char *value)
{
  int ymin, ymax, dy;
  int posy;

  if (!iupStrToInt(value, &posy))
    return 0;

  ymin = iupAttribGetInt(ih, "YMIN");
  ymax = iupAttribGetInt(ih, "YMAY");
  dy = iupAttribGetInt(ih, "DY");

  if (posy < ymin) posy = ymin;
  if (posy > ymax - dy) posy = ymax - dy;

  iupAttribSetInt(ih, "POSY", posy);

  iupGLScrollbarsLayoutUpdate(ih);
  return 0;
}

void iupGLScrollbarsRegisterAttrib(Iclass* ic)
{
  iupClassRegisterAttribute(ic, "POSX", NULL, iGLScrollbarsSetPosXAttrib, "0", NULL, IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "POSY", NULL, iGLScrollbarsSetPosYAttrib, "0", NULL, IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "XMIN", NULL, NULL, IUPAF_SAMEASSYSTEM, "0", IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "XMAX", NULL, NULL, IUPAF_SAMEASSYSTEM, "1", IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "YMIN", NULL, NULL, IUPAF_SAMEASSYSTEM, "0", IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "YMAX", NULL, NULL, IUPAF_SAMEASSYSTEM, "1", IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "LINEX", NULL, NULL, NULL, NULL, IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "LINEY", NULL, NULL, NULL, NULL, IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "DX", NULL, NULL, "0.1", NULL, IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "DY", NULL, NULL, "0.1", NULL, IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "SCROLLBARSIZE", NULL, NULL, IUPAF_SAMEASSYSTEM, "11", IUPAF_NO_INHERIT);
}


/*********************************************************************/


static int iGLScrollBoxBUTTON_CB(Ihandle *ih, int button, int pressed, int x, int y, char* status)
{
  if (button != IUP_BUTTON1)
    return IUP_DEFAULT;

  if (pressed)
  {
    int handler = iupGLScrollbarsIsInsideHandlers(ih, x, y);
    if (handler != SB_DRAG_X && handler != SB_DRAG_Y)
    {
      iupAttribSet(ih, "PRESSED", NULL);  /* preserved only during DRAG */
      iupAttribSet(ih, "_IUP_DRAG_HANDLER", NULL);
      iupAttribSetInt(ih, "_IUP_PRESS_HANDLER", handler);
    }
    else
    {
      if (handler == SB_DRAG_X)
        iupAttribSetStr(ih, "_IUP_START_POS", iupAttribGet(ih, "POSX"));
      else
        iupAttribSetStr(ih, "_IUP_START_POS", iupAttribGet(ih, "POSY"));
      iupAttribSetInt(ih, "_IUP_DRAG_HANDLER", handler);
      iupAttribSetInt(ih, "_IUP_START_X", x);
      iupAttribSetInt(ih, "_IUP_START_Y", y);
    }
  }
  else if (!iupAttribGet(ih, "_IUP_DRAG_HANDLER"))
  {
    int press_handler = iupAttribGetInt(ih, "_IUP_PRESS_HANDLER");
    int handler = iupGLScrollbarsIsInsideHandlers(ih, x, y);
    if (handler != SB_NONE && press_handler == handler)
    {
      if (handler == SB_INC_X || handler == SB_PAGEINC_X ||
        handler == SB_DEC_X || handler == SB_PAGEDEC_X)
        iupGLScrollbarsPressX(ih, handler);

      if (handler == SB_INC_Y || handler == SB_PAGEINC_Y ||
        handler == SB_DEC_Y || handler == SB_PAGEDEC_Y)
        iupGLScrollbarsPressY(ih, handler);

      iupGLScrollbarsLayoutUpdate(ih);
    }
  }

  iupGLSubCanvasRedraw(ih);

  (void)status;
  return IUP_DEFAULT;
}

static int iGLScrollBoxMOTION_CB(Ihandle *ih, int x, int y, char* status)
{
  int redraw = 0;
  int pressed = iupAttribGetInt(ih, "PRESSED");

  /* special highlight processing for scrollbar area */
  if (iupGLScrollbarsIsInsideHandlers(ih, x, y) != SB_NONE)
  {
    if (!iupAttribGet(ih, "HIGHLIGHT"))
    {
      redraw = 1;
      iupAttribSet(ih, "HIGHLIGHT", "1");
    }
  }
  else
  {
    if (iupAttribGet(ih, "HIGHLIGHT"))
    {
      redraw = 1;
      iupAttribSet(ih, "HIGHLIGHT", NULL);
    }
  }

  if (pressed)  /* means handler = SB_DRAG_* */
  {
    int handler = iupAttribGetInt(ih, "_IUP_DRAG_HANDLER");
    int start_x = iupAttribGetInt(ih, "_IUP_START_X");
    int start_y = iupAttribGetInt(ih, "_IUP_START_Y");
    int start_pos = iupAttribGetInt(ih, "_IUP_START_POS");

    if (iupGLScrollbarsMove(ih, x - start_x, y - start_y, start_pos, handler))
    {
      iupGLScrollbarsLayoutUpdate(ih);

      iupGLSubCanvasRedraw(ih);
      redraw = 0;
    }
  }

  if (redraw)
    iupGLSubCanvasRedraw(ih);

  (void)status;
  return IUP_DEFAULT;
}

static int iGLScrollBoxENTERWINDOW_CB(Ihandle* ih, int x, int y)
{
  /* special highlight processing for scrollbar area */
  if (iupGLScrollbarsIsInsideHandlers(ih, x, y) != SB_NONE)
    iupAttribSet(ih, "HIGHLIGHT", "1");
  else
    iupAttribSet(ih, "HIGHLIGHT", NULL);

  return iupGLSubCanvasRedraw(ih);
}

static int iGLScrollBoxACTION_CB(Ihandle* ih)
{
  int active = iupAttribGetInt(ih, "ACTIVE");
  char* fgcolor = iupAttribGetStr(ih, "FORECOLOR");
  char* bgcolor = iupAttribGetStr(ih, "BACKCOLOR");
  int highlight = iupAttribGetInt(ih, "HIGHLIGHT");
  int pressed = iupAttribGetInt(ih, "PRESSED");

  if (pressed)
  {
    char* presscolor = iupAttribGetStr(ih, "PRESSCOLOR");
    if (presscolor)
      fgcolor = presscolor;
  }
  else if (highlight)
  {
    char* hlcolor = iupAttribGetStr(ih, "HIGHCOLOR");
    if (hlcolor)
      fgcolor = hlcolor;
  }

  iupGLScrollbarsDraw(ih, active, fgcolor, bgcolor);

  return IUP_DEFAULT;
}

static char* iGLScrollBoxGetClientSizeAttrib(Ihandle* ih)
{
  int dx = IupGetInt(ih, "DX");
  int dy = IupGetInt(ih, "DY");
  return iupStrReturnIntInt(dx, dy, 'x');
}


/*****************************************************************************\
|* Methods                                                                   *|
\*****************************************************************************/


static void iGLScrollBoxComputeNaturalSizeMethod(Ihandle* ih, int *w, int *h, int *children_expand)
{
  if (ih->firstchild)
  {
    /* update child natural size first */
    iupBaseComputeNaturalSize(ih->firstchild);
  }

  /* ScrollBox size does not depends on the child size,
     its natural size must be 0 to be free of restrictions. */
  (void)w;
  (void)h;

  /* Also set expand to its own expand so it will not depend on children */
  *children_expand = ih->expand;
}

static void iGLScrollBoxSetChildrenCurrentSizeMethod(Ihandle* ih, int shrink)
{
  if (ih->firstchild)
  {
    int w, h, has_sb_horiz=0, has_sb_vert=0;

    /* If child is greater than scrollbox area, use child natural size,
       else use current scrollbox size;
       So this will let the child be greater than the scrollbox,
       or let the child expand to the scrollbox. */

    if (ih->firstchild->naturalwidth > ih->currentwidth)
    {
      w = ih->firstchild->naturalwidth;
      has_sb_horiz = 1;
    }
    else
      w = ih->currentwidth;  /* expand space */

    if (ih->firstchild->naturalheight > ih->currentheight)
    {
      h = ih->firstchild->naturalheight;
      has_sb_vert = 1;
    }
    else
      h = ih->currentheight; /* expand space */

    if (!has_sb_horiz && has_sb_vert)
      w -= iupGLScrollbarsGetSize(ih);  /* reduce expand space */
    if (has_sb_horiz && !has_sb_vert)
      h -= iupGLScrollbarsGetSize(ih);  /* reduce expand space */

    iupBaseSetCurrentSize(ih->firstchild, w, h, shrink);

    IupSetInt(ih, "XMAX", ih->firstchild->currentwidth);
    IupSetInt(ih, "YMAX", ih->firstchild->currentheight);
  }
  else
  {
    IupSetAttribute(ih, "XMAX", "0");
    IupSetAttribute(ih, "YMAX", "0");
  }
}

static void iGLScrollBoxSetChildrenPositionMethod(Ihandle* ih, int x, int y)
{
  if (ih->firstchild)
  {
    iupBaseSetPosition(ih->firstchild, x - iupAttribGetInt(ih, "POSX"),
                                       y - iupAttribGetInt(ih, "POSY"));
  }
}

static void iGLScrollBoxLayoutUpdate(Ihandle* ih)
{
  int width = ih->currentwidth,
     height = ih->currentheight;

  /* already updated the subcanvas layout, 
     so just have to update the scrollbars and child. */

  if (ih->firstchild)
  {
    /* if child is greater than scrollbox, has scrollbars
       but this affects the opposite direction */

    if (ih->firstchild->currentwidth > ih->currentwidth)
      height -= iupGLScrollbarsGetSize(ih);

    if (ih->firstchild->currentheight > ih->currentheight)
      width -= iupGLScrollbarsGetSize(ih);
  }

  if (width < 0) width = 0;
  if (height < 0) height = 0;

  IupSetInt(ih, "DX", width);
  IupSetInt(ih, "DY", height);

  iupGLScrollbarsLayoutUpdate(ih);
}

static int iGLScrollBoxCreateMethod(Ihandle* ih, void** params)
{
  IupSetCallback(ih, "GL_ACTION", (Icallback)iGLScrollBoxACTION_CB);
  IupSetCallback(ih, "GL_BUTTON_CB", (Icallback)iGLScrollBoxBUTTON_CB);
  IupSetCallback(ih, "GL_MOTION_CB", (Icallback)iGLScrollBoxMOTION_CB);
  IupSetCallback(ih, "GL_LEAVEWINDOW_CB", iupGLSubCanvasRedraw);
  IupSetCallback(ih, "GL_ENTERWINDOW_CB", (Icallback)iGLScrollBoxENTERWINDOW_CB);

  if (params)
  {
    Ihandle** iparams = (Ihandle**)params;
    if (iparams[0]) IupAppend(ih, iparams[0]);
  }

  return IUP_NOERROR;
}

Iclass* iupGLScrollBoxNewClass(void)
{
  Iclass* ic = iupClassNew(iupRegisterFindClass("glsubcanvas"));

  ic->name   = "glscrollbox";
  ic->format = "h";   /* one ihandle */
  ic->nativetype = IUP_TYPEVOID;
  ic->childtype  = IUP_CHILDMANY+1;  /* 1 child */
  ic->is_interactive = 1;

  /* Class functions */
  ic->New = iupGLScrollBoxNewClass;
  ic->Create  = iGLScrollBoxCreateMethod;

  ic->ComputeNaturalSize = iGLScrollBoxComputeNaturalSizeMethod;
  ic->SetChildrenCurrentSize = iGLScrollBoxSetChildrenCurrentSizeMethod;
  ic->SetChildrenPosition = iGLScrollBoxSetChildrenPositionMethod;
  ic->LayoutUpdate = iGLScrollBoxLayoutUpdate;

  /* Base Container */
  iupClassRegisterAttribute(ic, "EXPAND", iupBaseContainerGetExpandAttrib, NULL, IUPAF_SAMEASSYSTEM, "YES", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "CLIENTOFFSET", iupBaseGetClientOffsetAttrib, NULL, NULL, NULL, IUPAF_NOT_MAPPED | IUPAF_READONLY | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "CLIENTSIZE", iGLScrollBoxGetClientSizeAttrib, NULL, NULL, NULL, IUPAF_NOT_MAPPED | IUPAF_READONLY | IUPAF_NO_INHERIT);

  /* ScrollBox */
  iupClassRegisterAttribute(ic, "HIGHCOLOR", NULL, NULL, IUPAF_SAMEASSYSTEM, "90 190 255", IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "FORECOLOR", NULL, NULL, IUPAF_SAMEASSYSTEM, "110 210 230", IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "PRESSCOLOR", NULL, NULL, IUPAF_SAMEASSYSTEM, "50 150 255", IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "BACKCOLOR", NULL, NULL, IUPAF_SAMEASSYSTEM, "200 225 245", IUPAF_NO_INHERIT);

  iupGLScrollbarsRegisterAttrib(ic);

  return ic;
}

Ihandle* IupGLScrollBox(Ihandle* child)
{
  void *children[2];
  children[0] = (void*)child;
  children[1] = NULL;
  return IupCreatev("glscrollbox", children);
}
