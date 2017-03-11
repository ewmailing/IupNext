/** \file
 * \brief iupflatscrollbox control
 *
 * See Copyright Notice in "iup.h"
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "iup.h"
#include "iupcbs.h"
#include "iupkey.h"

#include "iup_object.h"
#include "iup_register.h"
#include "iup_attrib.h"
#include "iup_str.h"
#include "iup_stdcontrols.h"
#include "iup_layout.h"
#include "iup_drv.h"
#include "iup_childtree.h"



static Ihandle* iFlatScrollBoxGetVertScrollbar(Ihandle *ih)
{
  return ih->firstchild;
}

static Ihandle* iFlatScrollBoxGetHorizScrollbar(Ihandle *ih)
{
  return ih->firstchild->brother;
}

static Ihandle* iFlatScrollBoxGetChild(Ihandle *ih)
{
  return ih->firstchild->brother->brother;
}

static void iFlatScrollBoxRedrawVertScrollbar(Ihandle* ih)
{
  Ihandle* sb_vert = iFlatScrollBoxGetVertScrollbar(ih);
  IupUpdate(sb_vert);
}

static void iFlatScrollBoxRedrawHorizScrollbar(Ihandle* ih)
{
  Ihandle* sb_horiz = iFlatScrollBoxGetHorizScrollbar(ih);
  IupUpdate(sb_horiz);
}

static void iFlatScrollBoxUpdateChildPos(Ihandle *ih, int drag)
{
  Ihandle* child = iFlatScrollBoxGetChild(ih);
  if (child)
  {
    int x, y;
    int sb_size = iupAttribGetInt(ih, "SCROLLBARSIZE");
    int posx = iupAttribGetInt(ih, "POSX");
    int posy = iupAttribGetInt(ih, "POSY");

    char* offset = iupAttribGet(ih, "CHILDOFFSET");

    if (drag && !iupAttribGetBoolean(ih, "LAYOUTDRAG"))
      return;

    /* Native container, position is reset */
    x = 0;
    y = 0;

    if (offset) iupStrToIntInt(offset, &x, &y, 'x');

    if (iupAttribGetInt(ih, "DX") > iupAttribGetInt(ih, "XMAX") - sb_size)
      posx = 0;
    if (iupAttribGetInt(ih, "DY") > iupAttribGetInt(ih, "YMAX") - sb_size)
      posy = 0;

    x -= posx;
    y -= posy;

    iupBaseSetPosition(child, x, y);
    iupLayoutUpdate(child);
  }
}


/*************************************************************************/


enum {
  SB_NONE,
  SB_INC_X, SB_PAGEINC_X, SB_DRAG_X, SB_PAGEDEC_X, SB_DEC_X,
  SB_INC_Y, SB_PAGEINC_Y, SB_DRAG_Y, SB_PAGEDEC_Y, SB_DEC_Y
};


static int iFlatScrollbarsGetHandler(Ihandle* ih, int x, int y)
{
  int sb_size = iupAttribGetInt(ih->parent, "SCROLLBARSIZE");
  int xmax = iupAttribGetInt(ih->parent, "XMAX");
  int ymax = iupAttribGetInt(ih->parent, "YMAX");
  int dy = iupAttribGetInt(ih->parent, "DY");
  int dx = iupAttribGetInt(ih->parent, "DX");
  int is_vert_scrollbar = 0;
  int range, d, pos;

  if (ih->currentwidth == sb_size)
    is_vert_scrollbar = 1;

  if (is_vert_scrollbar)
  {
    int posy = iupAttribGetInt(ih->parent, "POSY");

    int height = ih->currentheight;
    if (xmax > dx)  /* has horizontal scrollbar */
      height -= sb_size;

    range = height - 1 - 2 * sb_size;
    d = (dy * range) / ymax;
    pos = ((posy)* range) / ymax;
    pos += sb_size;

    if (y < sb_size)
      return SB_DEC_Y;
    else if (y < pos)
      return SB_PAGEDEC_Y;
    else if (y < pos + d)
      return SB_DRAG_Y;
    else if (y < height - sb_size)
      return SB_PAGEINC_Y;
    else if (y < height)
      return SB_INC_Y;
  }
  else
  {
    int posx = iupAttribGetInt(ih->parent, "POSX");

    int width = ih->currentwidth;
    if (ymax > dy)  /* has vertical scrollbar */
      width -= sb_size;

    range = width - 1 - 2 * sb_size;
    d = (dx * range) / xmax;
    pos = ((posx)* range) / xmax;
    pos += sb_size;

    if (x < sb_size)
      return SB_DEC_X;
    else if (x < pos)
      return SB_PAGEDEC_X;
    else if (x < pos + d)
      return SB_DRAG_X;
    else if (x < width - sb_size)
      return SB_PAGEINC_X;
    else if (x < width)
      return SB_INC_X;
  }

  return SB_NONE;
}

static void iFlatScrollbarsPressX(Ihandle* ih, int handler)
{
  int xmax = iupAttribGetInt(ih->parent, "XMAX");
  int dx = iupAttribGetInt(ih->parent, "DX");
  int posx = iupAttribGetInt(ih->parent, "POSX");

  if (handler == SB_INC_X)
  {
    int linex = dx / 10;
    char* linex_str = iupAttribGet(ih->parent, "LINEX");
    if (linex_str) iupStrToInt(linex_str, &linex);
    posx += linex;
  }
  else if (handler == SB_PAGEINC_X)
    posx += dx;
  if (handler == SB_DEC_X)
  {
    int linex = dx / 10;
    char* linex_str = iupAttribGet(ih->parent, "LINEX");
    if (linex_str) iupStrToInt(linex_str, &linex);
    posx -= linex;
  }
  else if (handler == SB_PAGEDEC_X)
    posx -= dx;

  if (posx < 0)
    posx = 0;
  if (posx > xmax - dx)
    posx = xmax - dx;

  iupAttribSetInt(ih->parent, "POSX", posx);
}

static void iFlatScrollbarsPressY(Ihandle* ih, int handler)
{
  int ymax = iupAttribGetInt(ih->parent, "YMAX");
  int dy = iupAttribGetInt(ih->parent, "DY");
  int posy = iupAttribGetInt(ih->parent, "POSY");

  if (handler == SB_INC_Y)
  {
    int liney = dy / 10;
    char* liney_str = iupAttribGet(ih->parent, "LINEY");
    if (liney_str) iupStrToInt(liney_str, &liney);
    posy += liney;
  }
  else if (handler == SB_PAGEINC_Y)
    posy += dy;
  if (handler == SB_DEC_Y)
  {
    int liney = dy / 10;
    char* liney_str = iupAttribGet(ih->parent, "LINEY");
    if (liney_str) iupStrToInt(liney_str, &liney);
    posy -= liney;
  }
  else if (handler == SB_PAGEDEC_Y)
    posy -= dy;

  if (posy < 0)
    posy = 0;
  if (posy > ymax - dy)
    posy = ymax - dy;

  iupAttribSetInt(ih->parent, "POSY", posy);
}

static int iFlatScrollbarsMoveX(Ihandle* ih, int diff, int start_pos)
{
  int xmax = iupAttribGetInt(ih->parent, "XMAX");
  int ymax = iupAttribGetInt(ih->parent, "YMAX");
  int dx = iupAttribGetInt(ih->parent, "DX");
  int dy = iupAttribGetInt(ih->parent, "DY");
  int sb_size = iupAttribGetInt(ih->parent, "SCROLLBARSIZE");
  int range;
  int pos, posx;
  int width = ih->currentwidth;

  if (ymax > dy)  /* has vertical scrollbar */
    width -= sb_size;

  range = width - 1 - 2 * sb_size;

  pos = ((start_pos)* range) / xmax;
  pos += sb_size;

  pos += diff;

  pos -= sb_size;
  posx = (pos * xmax) / range;

  if (posx < 0)
    posx = 0;
  if (posx > xmax - dx)
    posx = xmax - dx;

  if (posx != start_pos)
  {
    iupAttribSetInt(ih->parent, "POSX", posx);
    return 1;
  }

  return 0;
}

static int iFlatScrollbarsMoveY(Ihandle* ih, int diff, int start_pos)
{
  int xmax = iupAttribGetInt(ih->parent, "XMAX");
  int ymax = iupAttribGetInt(ih->parent, "YMAX");
  int dx = iupAttribGetInt(ih->parent, "DX");
  int dy = iupAttribGetInt(ih->parent, "DY");
  int sb_size = iupAttribGetInt(ih->parent, "SCROLLBARSIZE");
  int range;
  int pos, posy;
  int height = ih->currentheight;

  if (xmax > dx)  /* has horizontal scrollbar */
    height -= sb_size;

  range = height - 1 - 2 * sb_size;

  pos = ((start_pos)* range) / ymax;
  pos += sb_size;

  pos += diff;

  pos -= sb_size;
  posy = (pos * ymax) / range;

  if (posy < 0)
    posy = 0;
  if (posy > ymax - dy)
    posy = ymax - dy;

  if (posy != start_pos)
  {
    iupAttribSetInt(ih->parent, "POSY", posy);
    return 1;
  }

  return 0;
}

static int iFlatScrollBarButton_CB(Ihandle *ih, int button, int pressed, int x, int y)
{
  if (button != IUP_BUTTON1)
    return IUP_DEFAULT;

  if (pressed)
  {
    int handler = iFlatScrollbarsGetHandler(ih, x, y);
    iupAttribSetInt(ih, "_IUP_PRESSED_HANDLER", handler);

    if (handler == SB_DRAG_X || handler == SB_DRAG_Y)
    {
      iupAttribSetInt(ih, "_IUP_START_X", x);
      iupAttribSetInt(ih, "_IUP_START_Y", y);

      if (handler == SB_DRAG_X)
        iupAttribSetStr(ih, "_IUP_START_POS", iupAttribGet(ih->parent, "POSX"));
      else
        iupAttribSetStr(ih, "_IUP_START_POS", iupAttribGet(ih->parent, "POSY"));
    }

    if (handler != SB_NONE)
    {
      IupUpdate(ih);
      return IUP_DEFAULT;
    }
  }
  else
  {
    int press_handler = iupAttribGetInt(ih, "_IUP_PRESSED_HANDLER");
    int handler = iFlatScrollbarsGetHandler(ih, x, y);

    if (handler != SB_NONE && handler != SB_DRAG_X && handler != SB_DRAG_Y &&
        handler == press_handler)
    {
      if (handler == SB_INC_X || handler == SB_PAGEINC_X ||
          handler == SB_DEC_X || handler == SB_PAGEDEC_X)
        iFlatScrollbarsPressX(ih, handler);

      if (handler == SB_INC_Y || handler == SB_PAGEINC_Y ||
          handler == SB_DEC_Y || handler == SB_PAGEDEC_Y)
        iFlatScrollbarsPressY(ih, handler);

      iFlatScrollBoxUpdateChildPos(ih->parent, 0);
    }
    iupAttribSet(ih, "_IUP_PRESSED_HANDLER", NULL);

    if (handler != SB_NONE)
    {
      IupUpdate(ih);
      return IUP_DEFAULT;
    }
  }

  return IUP_DEFAULT;
}

static int iFlatScrollBarMotion_CB(Ihandle *ih, int x, int y)
{
  int redraw = 0;
  int handler = iFlatScrollbarsGetHandler(ih, x, y);

  /* special highlight processing for scrollbar area */
  int old_handler = iupAttribGetInt(ih, "_IUP_HIGHLIGHT_HANDLER");
  if (old_handler != handler)
  {
    redraw = 1;
    iupAttribSetInt(ih, "_IUP_HIGHLIGHT_HANDLER", handler);
  }

  handler = iupAttribGetInt(ih, "_IUP_PRESSED_HANDLER");
  if (handler == SB_DRAG_X)
  {
    int start_x = iupAttribGetInt(ih, "_IUP_START_X");
    int start_pos = iupAttribGetInt(ih, "_IUP_START_POS");

    if (x != start_x && iFlatScrollbarsMoveX(ih, x - start_x, start_pos))
    {
      iFlatScrollBoxUpdateChildPos(ih->parent, 1);
      redraw = 1;
    }
  }
  else if (handler == SB_DRAG_Y)
  {
    int start_y = iupAttribGetInt(ih, "_IUP_START_Y");
    int start_pos = iupAttribGetInt(ih, "_IUP_START_POS");

    if (y != start_y && iFlatScrollbarsMoveY(ih, y - start_y, start_pos))
    {
      iFlatScrollBoxUpdateChildPos(ih->parent, 1);
      redraw = 1;
    }
  }

  if (redraw)
    IupUpdate(ih);

  return IUP_DEFAULT;
}

static int iFlatScrollBarLeaveWindow_CB(Ihandle* ih)
{
  iupAttribSet(ih, "_IUP_HIGHLIGHT_HANDLER", NULL);
  iupAttribSet(ih, "_IUP_PRESSED_HANDLER", NULL);
  IupUpdate(ih);
  return IUP_DEFAULT;
}


/*****************************************************************************/


static int iFlatScrollBoxButton_CB(Ihandle *ih, int but, int pressed, int x, int y, char* status)
{
  if (but==IUP_BUTTON1 && pressed)
  {
    iupAttribSetInt(ih, "_IUP_START_X", x);
    iupAttribSetInt(ih, "_IUP_START_Y", y);
    iupAttribSetInt(ih, "_IUP_START_POSX", iupAttribGetInt(ih, "POSX"));
    iupAttribSetInt(ih, "_IUP_START_POSY", iupAttribGetInt(ih, "POSY"));
    iupAttribSet(ih, "_IUP_DRAG_SB", "1");
  }
  if (but==IUP_BUTTON1 && !pressed)
    iupAttribSet(ih, "_IUP_DRAG_SB", NULL);
  (void)status;
  return IUP_DEFAULT;
}

static int iFlatScrollBoxMotion_CB(Ihandle *ih, int x, int y, char* status)
{
  if (iup_isbutton1(status) && iupAttribGet(ih, "_IUP_DRAG_SB"))
  {
    int start_x = iupAttribGetInt(ih, "_IUP_START_X");
    int start_y = iupAttribGetInt(ih, "_IUP_START_Y");
    int dx = x - start_x;
    int dy = y - start_y;
    int posx = iupAttribGetInt(ih, "_IUP_START_POSX");
    int posy = iupAttribGetInt(ih, "_IUP_START_POSY");
    iupAttribSetInt(ih, "POSX", posx - dx);  /* drag direction is opposite to scrollbar */
    iupAttribSetInt(ih, "POSY", posy - dy);

    iFlatScrollBoxRedrawVertScrollbar(ih);
    iFlatScrollBoxRedrawHorizScrollbar(ih);

    iFlatScrollBoxUpdateChildPos(ih, 0);
  }
  return IUP_DEFAULT;
}


/*****************************************************************************/


static int iFlatScrollBoxGetChildPosition(Ihandle* ih, Ihandle* child, int *posx, int *posy)
{
  while (child->parent && child != ih)
  {
    *posx += child->x;
    *posy += child->y;

    child = iupChildTreeGetNativeParent(child);
  }

  if (!child->parent)
    return 0;
  else
    return 1;
}

static int iFlatScrollBoxSetScrollToChildHandleAttrib(Ihandle* ih, const char* value)
{
  Ihandle* child = (Ihandle*)value;
  if (iupObjectCheck(child))
  {
    int posx = 0, posy = 0;
    if (iFlatScrollBoxGetChildPosition(ih, child, &posx, &posy))
    {
      iupAttribSetInt(ih, "POSX", posx);
      iupAttribSetInt(ih, "POSY", posy);

      iFlatScrollBoxRedrawVertScrollbar(ih);
      iFlatScrollBoxRedrawHorizScrollbar(ih);

      iFlatScrollBoxUpdateChildPos(ih, 0);
    }
  }
  return 0;
}

static int iFlatScrollBoxSetScrollToChildAttrib(Ihandle* ih, const char* value)
{
  return iFlatScrollBoxSetScrollToChildHandleAttrib(ih, (char*)IupGetHandle(value));
}

static int iFlatScrollBoxSetScrollToAttrib(Ihandle* ih, const char* value)
{
  if (iupStrEqualNoCase(value, "TOP"))
  {
    iupAttribSetInt(ih, "POSX", 0);
    iupAttribSetInt(ih, "POSY", 0);

    iFlatScrollBoxRedrawVertScrollbar(ih);
    iFlatScrollBoxRedrawHorizScrollbar(ih);

    iFlatScrollBoxUpdateChildPos(ih, 0);
  }
  else if (iupStrEqualNoCase(value, "BOTTOM"))
  {
    iupAttribSetInt(ih, "POSX", 0);
    iupAttribSetInt(ih, "POSY", iupAttribGetInt(ih, "YMAX") - iupAttribGetInt(ih, "DY"));

    iFlatScrollBoxRedrawVertScrollbar(ih);
    iFlatScrollBoxRedrawHorizScrollbar(ih);

    iFlatScrollBoxUpdateChildPos(ih, 0);
  }
  else
  {
    int posx, posy;
    if (iupStrToIntInt(value, &posx, &posy, ',') == 2)
    {
      iupAttribSetInt(ih, "POSX", posx);
      iupAttribSetInt(ih, "POSY", posy);

      iFlatScrollBoxRedrawVertScrollbar(ih);
      iFlatScrollBoxRedrawHorizScrollbar(ih);

      iFlatScrollBoxUpdateChildPos(ih, 0);
    }
  }
  return 0;
}

static char* iFlatScrollBoxGetExpandAttrib(Ihandle* ih)
{
  if (iupAttribGetBoolean(ih, "CANVASBOX"))
    return iupBaseGetExpandAttrib(ih);
  else
    return iupBaseContainerGetExpandAttrib(ih);
}

static int iFlatScrollBoxSetExpandAttrib(Ihandle* ih, const char* value)
{
  if (iupAttribGetBoolean(ih, "CANVASBOX"))
    return iupBaseSetExpandAttrib(ih, value);
  else
    return 1;  /* store on the hash table */
}

static int iFlatScrollBoxSetDXAttrib(Ihandle* ih, const char *value)
{
  int dx;
  if (iupStrToInt(value, &dx))
  {
    Ihandle* sb_horiz = iFlatScrollBoxGetHorizScrollbar(ih);
    int xmax = iupAttribGetInt(ih, "XMAX");

    if (dx >= xmax)
    {
      IupSetAttribute(sb_horiz, "VISIBLE", "NO");
      IupSetAttribute(ih, "POSX", "0");
    }
    else
    {
      int posx = iupAttribGetInt(ih, "POSX");
      if (posx > xmax - dx)
        iupAttribSetInt(ih, "POSX", xmax - dx);

      IupSetAttribute(sb_horiz, "VISIBLE", "Yes");

      iFlatScrollBoxRedrawHorizScrollbar(ih);  /* force a redraw if it is already visible */
    }

    iFlatScrollBoxRedrawVertScrollbar(ih);  /* force a redraw of the other scrollbar */
  }

  return 1;
}

static int iFlatScrollBoxSetDYAttrib(Ihandle* ih, const char *value)
{
  int dy;
  if (iupStrToInt(value, &dy))
  {
    Ihandle* sb_vert = iFlatScrollBoxGetVertScrollbar(ih);
    int ymax = iupAttribGetInt(ih, "YMAX");

    if (dy >= ymax)
    {
      IupSetAttribute(sb_vert, "VISIBLE", "NO");
      IupSetAttribute(ih, "POSY", "0");
    }
    else
    {
      int posy = iupAttribGetInt(ih, "POSY");
      if (posy > ymax - dy)
        iupAttribSetInt(ih, "POSY", ymax - dy);

      IupSetAttribute(sb_vert, "VISIBLE", "Yes");

      iFlatScrollBoxRedrawVertScrollbar(ih);  /* force a redraw if it is already visible */
    }

    iFlatScrollBoxRedrawHorizScrollbar(ih);  /* force a redraw of the other scrollbar */
  }

  return 1;
}

static int iFlatScrollBoxSetPosXAttrib(Ihandle* ih, const char *value)
{
  int xmax, dx;
  int posx;

  if (!iupStrToInt(value, &posx))
    return 0;

  xmax = iupAttribGetInt(ih, "XMAX");
  dx = iupAttribGetInt(ih, "DX");

  if (posx > xmax - dx) posx = xmax - dx;

  iupAttribSetInt(ih, "POSX", posx);

  iFlatScrollBoxRedrawHorizScrollbar(ih);

  iFlatScrollBoxUpdateChildPos(ih, 0);

  return 0;
}

static int iFlatScrollBoxSetPosYAttrib(Ihandle* ih, const char *value)
{
  int ymax, dy;
  int posy;

  if (!iupStrToInt(value, &posy))
    return 0;

  ymax = iupAttribGetInt(ih, "YMAY");
  dy = iupAttribGetInt(ih, "DY");

  if (posy > ymax - dy) posy = ymax - dy;

  iupAttribSetInt(ih, "POSY", posy);

  iFlatScrollBoxRedrawVertScrollbar(ih);

  iFlatScrollBoxUpdateChildPos(ih, 0);

  return 0;
}


/*******************************************************************************************************/


static void iFlatScrollBoxComputeNaturalSizeMethod(Ihandle* ih, int *w, int *h, int *children_expand)
{
  Ihandle* child = iFlatScrollBoxGetChild(ih);
  if (child)
  {
    /* update child natural size first */
    iupBaseComputeNaturalSize(child);
  }

  if (!iupAttribGetBoolean(ih, "CANVASBOX"))
  {
    /* ScrollBox size does not depends on the child size,
     its natural size must be 0 to be free of restrictions. */
    if (ih->currentwidth == 0 && ih->currentheight == 0 && child)
    {
      *w = child->naturalwidth;
      *h = child->naturalheight;
    }
    else
    {
      *w = 0;
      *h = 0;
    }
  }

  /* Also set expand to its own expand so it will not depend on children */
  *children_expand = ih->expand;
}

static void iFlatScrollBoxUpdateVisibleScrollArea(Ihandle* ih, int view_width, int view_height)
{
  /* this is available drawing size not considering the scrollbars (BORDER=NO) */
  int canvas_width = ih->currentwidth,
    canvas_height = ih->currentheight;
  int sb_size = iupAttribGetInt(ih, "SCROLLBARSIZE");

  /* if child is greater than scrollbox in one direction,
  then it has scrollbars
  but this affects the opposite direction */

  if (view_width > ih->currentwidth)  /* check for horizontal scrollbar */
    canvas_height -= sb_size;                /* affect vertical size */
  if (view_height > ih->currentheight)
    canvas_width -= sb_size;

  if (view_width <= ih->currentwidth && view_width > canvas_width)
    canvas_height -= sb_size;
  if (view_height <= ih->currentheight && view_height > canvas_height)
    canvas_width -= sb_size;

  if (canvas_width < 0) canvas_width = 0;
  if (canvas_height < 0) canvas_height = 0;

  IupSetInt(ih, "DX", canvas_width);
  IupSetInt(ih, "DY", canvas_height);
}

static void iFlatScrollBoxSetChildrenCurrentSizeMethod(Ihandle* ih, int shrink)
{
  Ihandle* child = iFlatScrollBoxGetChild(ih);
  Ihandle* sb_vert = iFlatScrollBoxGetVertScrollbar(ih);
  Ihandle* sb_horiz = iFlatScrollBoxGetHorizScrollbar(ih);
  int sb_size = iupAttribGetInt(ih, "SCROLLBARSIZE");

  if (child)
  {
    int w, h, has_sb_horiz=0, has_sb_vert=0;

    /* If child is greater than scrollbox area, use child natural size,
       else use current scrollbox size;
       So this will let the child be greater than the scrollbox,
       or let the child expand to the scrollbox.  */

    if (child->naturalwidth > ih->currentwidth)
    {
      w = child->naturalwidth;
      has_sb_horiz = 1;
    }
    else
      w = ih->currentwidth;  /* expand space */

    if (child->naturalheight > ih->currentheight)
    {
      h = child->naturalheight;
      has_sb_vert = 1;
    }
    else
      h = ih->currentheight; /* expand space */

    if (!has_sb_horiz && has_sb_vert)
      w -= sb_size;  /* reduce expand space */

    if (has_sb_horiz && !has_sb_vert)
      h -= sb_size;  /* reduce expand space */

    /* Now w and h is a possible child size */
    iupBaseSetCurrentSize(child, w, h, shrink);

    /* Now we use the actual child size as the virtual area */
    iupAttribSetInt(ih, "XMAX", child->currentwidth);
    iupAttribSetInt(ih, "YMAX", child->currentheight);

    /* Finally update the visible area */
    iFlatScrollBoxUpdateVisibleScrollArea(ih, child->currentwidth, child->currentheight);
  }
  else
  {
    IupSetAttribute(ih, "XMAX", "0");
    IupSetAttribute(ih, "YMAX", "0");
    IupSetAttribute(ih, "DX", "0");
    IupSetAttribute(ih, "DY", "0");

    IupSetAttribute(sb_vert, "VISIBLE", "NO");
    IupSetAttribute(sb_horiz, "VISIBLE", "NO");
  }

  iupBaseSetCurrentSize(sb_vert, sb_size, ih->currentheight, shrink);
  iupBaseSetCurrentSize(sb_horiz, ih->currentwidth, sb_size, shrink);
}

static void iFlatScrollBoxSetChildrenPositionMethod(Ihandle* ih, int x, int y)
{
  Ihandle* child = iFlatScrollBoxGetChild(ih);
  Ihandle* sb_vert = iFlatScrollBoxGetVertScrollbar(ih);
  Ihandle* sb_horiz = iFlatScrollBoxGetHorizScrollbar(ih);
  int sb_size = iupAttribGetInt(ih, "SCROLLBARSIZE");

  if (child)
  {
    char* offset = iupAttribGet(ih, "CHILDOFFSET");

    /* Native container, position is reset */
    x = 0;
    y = 0;

    if (offset) iupStrToIntInt(offset, &x, &y, 'x');

    x -= iupAttribGetInt(ih, "POSX");
    y -= iupAttribGetInt(ih, "POSY");

    /* Child coordinates are relative to client left-top corner. */
    iupBaseSetPosition(child, x, y);
  }

  iupBaseSetPosition(sb_vert, ih->currentwidth-1 - sb_size, 0);
  iupBaseSetPosition(sb_horiz, 0, ih->currentheight - 1 - sb_size);

  IupSetAttribute(sb_vert, "ZORDER", "TOP");
  IupSetAttribute(sb_horiz, "ZORDER", "TOP");
}


/******************************************************************************/


static int iFlatScrollBoxCreateMethod(Ihandle* ih, void** params)
{
  Ihandle* sb_vert, *sb_horiz;

  sb_vert = IupCanvas(NULL);
  IupSetAttribute(sb_vert, "BORDER", "NO");
  IupSetAttribute(sb_vert, "ZORDER", "TOP");
  iupChildTreeAppend(ih, sb_vert);  /* sb_vert will always be the firstchild */
  sb_vert->flags |= IUP_INTERNAL;
  //  IupSetCallback(sb_vert, "ACTION", (Icallback)iFlatScrollBarAction_CB);
  IupSetCallback(sb_vert, "BUTTON_CB", (Icallback)iFlatScrollBarButton_CB);
  IupSetCallback(sb_vert, "MOTION_CB", (Icallback)iFlatScrollBarMotion_CB);
  IupSetCallback(sb_vert, "LEAVEWINDOW_CB", (Icallback)iFlatScrollBarLeaveWindow_CB);
  //TODO WHEEL_CB

  sb_horiz = IupCanvas(NULL);
  IupSetAttribute(sb_horiz, "BORDER", "NO");
  IupSetAttribute(sb_horiz, "ZORDER", "TOP");
  iupChildTreeAppend(ih, sb_horiz);  /* sb_vert will always be the firstchild->brother */
  sb_horiz->flags |= IUP_INTERNAL;
//  IupSetCallback(sb_horiz, "ACTION", (Icallback)iFlatScrollBarAction_CB);
  IupSetCallback(sb_horiz, "BUTTON_CB", (Icallback)iFlatScrollBarButton_CB);
  IupSetCallback(sb_horiz, "MOTION_CB", (Icallback)iFlatScrollBarMotion_CB);
  IupSetCallback(sb_horiz, "LEAVEWINDOW_CB", (Icallback)iFlatScrollBarLeaveWindow_CB);
  //TODO WHEEL_CB

  /* Setting callbacks */
  IupSetCallback(ih, "BUTTON_CB", (Icallback)iFlatScrollBoxButton_CB);
  IupSetCallback(ih, "MOTION_CB", (Icallback)iFlatScrollBoxMotion_CB);
  //TODO WHEEL_CB

  IupSetAttribute(ih, "CANFOCUS", "NO");

  if (params)
  {
    Ihandle** iparams = (Ihandle**)params;
    if (iparams[0]) IupAppend(ih, iparams[0]);  /* firstchild->brother->brother */
  }

  return IUP_NOERROR;
}

Iclass* iupFlatScrollBoxNewClass(void)
{
  Iclass* ic = iupClassNew(iupRegisterFindClass("canvas"));

  ic->name   = "flatscrollbox";
  ic->format = "h";   /* one Ihandle* */
  ic->nativetype = IUP_TYPECANVAS;
  ic->childtype  = IUP_CHILDMANY+3;  /* 1 child + 2 scrollbars */
  ic->is_interactive = 1;

  /* Class functions */
  ic->New = iupFlatScrollBoxNewClass;
  ic->Create  = iFlatScrollBoxCreateMethod;

  ic->ComputeNaturalSize = iFlatScrollBoxComputeNaturalSizeMethod;
  ic->SetChildrenCurrentSize = iFlatScrollBoxSetChildrenCurrentSizeMethod;
  ic->SetChildrenPosition = iFlatScrollBoxSetChildrenPositionMethod;

  /* Base Container */
  iupClassRegisterAttribute(ic, "EXPAND", iFlatScrollBoxGetExpandAttrib, iFlatScrollBoxSetExpandAttrib, IUPAF_SAMEASSYSTEM, "YES", IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "CLIENTOFFSET", iupBaseGetClientOffsetAttrib, NULL, NULL, NULL, IUPAF_NOT_MAPPED | IUPAF_READONLY | IUPAF_NO_INHERIT);
  {
    IattribGetFunc drawsize_get = NULL;
    iupClassRegisterGetAttribute(ic, "DRAWSIZE", &drawsize_get, NULL, NULL, NULL, NULL);
    iupClassRegisterAttribute(ic, "CLIENTSIZE", drawsize_get, NULL, NULL, NULL, IUPAF_READONLY|IUPAF_NO_INHERIT);
  }

  /* Native Container */
  iupClassRegisterAttribute(ic, "CHILDOFFSET", NULL, NULL, NULL, NULL, IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);

  /* replace IupCanvas behavior */
  iupClassRegisterReplaceAttribFunc(ic, "BGCOLOR", iupBaseNativeParentGetBgColorAttrib, NULL);
  iupClassRegisterReplaceAttribDef(ic, "BGCOLOR", "DLGBGCOLOR", NULL);
  iupClassRegisterReplaceAttribDef(ic, "BORDER", "NO", NULL);
  iupClassRegisterReplaceAttribFlags(ic, "BORDER", IUPAF_READONLY | IUPAF_NO_INHERIT);

  iupClassRegisterReplaceAttribFlags(ic, "SCROLLBAR", IUPAF_READONLY | IUPAF_NO_INHERIT);  /* will be always NO, but it will behave as always Yes */
  iupClassRegisterAttribute(ic, "YAUTOHIDE", NULL, NULL, IUPAF_SAMEASSYSTEM, "YES", IUPAF_READONLY | IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);  /* will be always Yes */
  iupClassRegisterAttribute(ic, "XAUTOHIDE", NULL, NULL, IUPAF_SAMEASSYSTEM, "YES", IUPAF_READONLY | IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);  /* will be always Yes */

  /* Scrollbox */
  iupClassRegisterAttribute(ic, "SCROLLTO", NULL, iFlatScrollBoxSetScrollToAttrib, NULL, NULL, IUPAF_WRITEONLY | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "SCROLLTOCHILD", NULL, iFlatScrollBoxSetScrollToChildAttrib, NULL, NULL, IUPAF_IHANDLENAME | IUPAF_WRITEONLY | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "SCROLLTOCHILD_HANDLE", NULL, iFlatScrollBoxSetScrollToChildHandleAttrib, NULL, NULL, IUPAF_IHANDLE | IUPAF_WRITEONLY | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "LAYOUTDRAG", NULL, NULL, IUPAF_SAMEASSYSTEM, "YES", IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);

  iupClassRegisterAttribute(ic, "CANVASBOX", NULL, NULL, NULL, NULL, IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);

  iupClassRegisterAttribute(ic, "DX", NULL, iFlatScrollBoxSetDXAttrib, NULL, NULL, IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "DY", NULL, iFlatScrollBoxSetDYAttrib, NULL, NULL, IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "POSX", NULL, iFlatScrollBoxSetPosXAttrib, "0", NULL, IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "POSY", NULL, iFlatScrollBoxSetPosYAttrib, "0", NULL, IUPAF_NO_INHERIT);

  iupClassRegisterAttribute(ic, "SCROLLBARSIZE", NULL, NULL, IUPAF_SAMEASSYSTEM, "11", IUPAF_NO_INHERIT);

  return ic;
}

Ihandle* IupFlatScrollBox(Ihandle* child)
{
  void *children[2];
  children[0] = (void*)child;
  children[1] = NULL;
  return IupCreatev("flatscrollbox", children);
}
