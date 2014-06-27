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

enum { SB_NONE, SB_INC, SB_PAGEINC, SB_DRAG, SB_PAGEDEC, SB_DEC };

int iupGLScrollbarsGetSize(Ihandle* ih)
{
  return iupAttribGetInt(ih, "SCROLLBARSIZE");
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
        return SB_DEC;
      else if (y < posy)
        return SB_PAGEDEC;
      else if (y < posy + dy)
        return SB_DRAG;
      else if (y < height - sb_size)
        return SB_PAGEINC;
      else if (y < height)
        return SB_INC;

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
        return SB_DEC;
      else if (x < posx)
        return SB_PAGEDEC;
      else if (x < posx + dx)
        return SB_DRAG;
      else if (x < width - sb_size)
        return SB_PAGEINC;
      else if (x < width)
        return SB_INC;

      return SB_NONE;
    }
  }

  return SB_NONE;
}

static void iGLScrollbarsDrawVertical(Ihandle* ih, int active, const char* fgcolor, const char* bgcolor)
{
  int sb_size = iupGLScrollbarsGetSize(ih);
  char* bordercolor = iupAttribGetStr(ih, "FRAMECOLOR");
  float bwidth = iupAttribGetFloat(ih, "FRAMEWIDTH");
  int border_width = (int)ceil(bwidth);
  int height = ih->currentheight;
  int xmin, xmax, ymin, ymax, posy, dy;
  int sb_xmin = iupAttribGetInt(ih, "XMIN");
  int sb_xmax = iupAttribGetInt(ih, "XMAX");
  int sb_ymin = iupAttribGetInt(ih, "YMIN");
  int sb_ymax = iupAttribGetInt(ih, "YMAX");
  int sb_dx = iupAttribGetInt(ih, "DX");
  int sb_dy = iupAttribGetInt(ih, "DY");
  int sb_posy = iupAttribGetInt(ih, "POSY");

  if (sb_xmax-sb_xmin > sb_dx)  /* has horizontal scrollbar */
    height -= sb_size;

  xmin = ih->currentwidth - sb_size;
  xmax = ih->currentwidth - 1;
  ymin = 0;
  ymax = height - 1;

  dy   =  (sb_dy              * (ymax - ymin - 2 * sb_size)) / (sb_ymax - sb_ymin);
  posy = ((sb_posy - sb_ymin) * (ymax - ymin - 2 * sb_size)) / (sb_ymax - sb_ymin);
  posy += ymin + sb_size;

  /* draw background */
  iupGLDrawBox(ih, xmin + border_width, xmax - border_width,
                   ymin + border_width, ymax - border_width, bgcolor, 1);

  /* draw border after the background because of the round rect */
  iupGLDrawRect(ih, xmin, xmax,
                    ymin, ymax, bwidth, bordercolor, active, 1);

  /* draw arrows */
  iupGLDrawArrow(ih, xmin, ymin,           fgcolor, active, IUPGL_ARROW_TOP,    sb_size, 1);
  iupGLDrawArrow(ih, xmin, ymax - sb_size, fgcolor, active, IUPGL_ARROW_BOTTOM, sb_size, 1);

  /* draw handler */
  iupGLDrawBox(ih, xmin + border_width, xmax - border_width,
                   posy, posy + dy, fgcolor, active);
}

static void iGLScrollbarsDrawHorizontal(Ihandle* ih, int active, const char* fgcolor, const char* bgcolor)
{
  int sb_size = iupGLScrollbarsGetSize(ih);
  char* bordercolor = iupAttribGetStr(ih, "FRAMECOLOR");
  float bwidth = iupAttribGetFloat(ih, "FRAMEWIDTH");
  int border_width = (int)ceil(bwidth);
  int width = ih->currentwidth;
  int xmin, xmax, ymin, ymax, posx, dx;
  int sb_xmin = iupAttribGetInt(ih, "XMIN");
  int sb_xmax = iupAttribGetInt(ih, "XMAX");
  int sb_ymin = iupAttribGetInt(ih, "YMIN");
  int sb_ymax = iupAttribGetInt(ih, "YMAX");
  int sb_dx = iupAttribGetInt(ih, "DX");
  int sb_dy = iupAttribGetInt(ih, "DY");
  int sb_posx = iupAttribGetInt(ih, "POSX");

  if (sb_ymax - sb_ymin > sb_dy)  /* has vertical scrollbar */
    width -= sb_size;

  xmin = 0;
  xmax = width - 1;
  ymin = ih->currentheight - sb_size;
  ymax = ih->currentheight - 1;

  dx   =  (sb_dx              * (xmax - xmin - 2 * sb_size)) / (sb_xmax - sb_xmin);
  posx = ((sb_posx - sb_xmin) * (xmax - xmin - 2 * sb_size)) / (sb_xmax - sb_xmin);
  posx += xmin + sb_size;

  /* draw background */
  iupGLDrawBox(ih, xmin + border_width, xmax - border_width,
                   ymin + border_width, ymax - border_width, bgcolor, 1);

  /* draw border after the background because of the round rect */
  iupGLDrawRect(ih, xmin, xmax,
                    ymin, ymax, bwidth, bordercolor, active, 1);

  /* draw arrows */
  iupGLDrawArrow(ih, xmin,           ymin, fgcolor, active, IUPGL_ARROW_LEFT,  sb_size, 1);
  iupGLDrawArrow(ih, xmax - sb_size, ymin, fgcolor, active, IUPGL_ARROW_RIGHT, sb_size, 1);

  /* draw handler */
  iupGLDrawBox(ih, posx, posx + dx,
                   ymin + border_width, ymax - border_width, fgcolor, active);
}

void iupGLScrollbarsDraw(Ihandle* ih, int active, const char* fgcolor, const char* bgcolor)
{
  int has_vert_scroll = 0;
  int has_horiz_scroll = 0;

  if (iupAttribGetInt(ih, "XMAX") - iupAttribGetInt(ih, "XMIN") > iupAttribGetInt(ih, "DX"))  /* has horizontal scrollbar */
    has_horiz_scroll = 1;

  if (iupAttribGetInt(ih, "YMAX") - iupAttribGetInt(ih, "YMIN") > iupAttribGetInt(ih, "DY"))  /* has vertical scrollbar */
    has_vert_scroll = 1;

  if (has_vert_scroll)
    iGLScrollbarsDrawVertical(ih, active, fgcolor, bgcolor);

  if (has_horiz_scroll)
    iGLScrollbarsDrawHorizontal(ih, active, fgcolor, bgcolor);
}


/*********************************************************************/


static void iGLScrollBoxUpdateChildPosition(Ihandle* ih, int posx, int posy)
{
  iupBaseSetPosition(ih->firstchild, -posx, -posy);
}

static int iGLScrollBoxScroll_CB(Ihandle *ih, int posx, int posy)
{
  if (ih->firstchild)
  {
    if (iupAttribGetInt(ih, "DX") > iupAttribGetInt(ih, "XMAX") - iupGLScrollbarsGetSize(ih))
      posx = 0;
    if (iupAttribGetInt(ih, "DY") > iupAttribGetInt(ih, "YMAX") - iupGLScrollbarsGetSize(ih))
      posy = 0;

    iGLScrollBoxUpdateChildPosition(ih, ih->x + posx, ih->y + posy);
    iupLayoutUpdate(ih->firstchild);
  }

  return IUP_DEFAULT;
}

static int iGLScrollBoxBUTTON_CB(Ihandle *ih, int button, int pressed, int x, int y, char* status)
{
  if (button != IUP_BUTTON1)
    return IUP_DEFAULT;

  if (pressed)
  {
    if (!iupGLScrollbarsIsInsideHandlers(ih, x, y))
    {

    }
  }

  (void)status;
  return IUP_DEFAULT;
}

static int iGLScrollBoxMOTION_CB(Ihandle *ih, int x, int y, char* status)
{
  int redraw = 0;

  /* special highlight processing for scrollbar area */
  if (iupGLScrollbarsIsInsideHandlers(ih, x, y))
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

  if (redraw)
    iupGLSubCanvasRedraw(ih);

  (void)status;
  return IUP_DEFAULT;
}

static int iGLScrollBoxENTERWINDOW_CB(Ihandle* ih, int x, int y)
{
  /* special highlight processing for scrollbar area */
  if (iupGLScrollbarsIsInsideHandlers(ih, x, y))
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
    iGLScrollBoxUpdateChildPosition(ih, x + iupAttribGetInt(ih, "POSX"), y + iupAttribGetInt(ih, "POSY"));
  }
}

static void iGLScrollBoxLayoutUpdate(Ihandle* ih)
{
  int width = ih->currentwidth,
     height = ih->currentheight;

  /* already updated the canvas layout, 
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

  if (ih->firstchild)
  {
    iGLScrollBoxUpdateChildPosition(ih, ih->x + iupAttribGetInt(ih, "POSX"), ih->y + iupAttribGetInt(ih, "POSY"));
    iupLayoutUpdate(ih->firstchild);
  }
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
  ic->nativetype = IUP_TYPECANVAS;
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
  iupClassRegisterAttribute(ic, "FORECOLOR", NULL, NULL, IUPAF_SAMEASSYSTEM, "200 225 245", IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "HIGHCOLOR", NULL, NULL, IUPAF_SAMEASSYSTEM, "190 210 230", IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "PRESSCOLOR", NULL, NULL, IUPAF_SAMEASSYSTEM, "150 200 235", IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "BACKCOLOR", NULL, NULL, IUPAF_SAMEASSYSTEM, "90 190 255", IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "FRAMECOLOR", NULL, NULL, IUPAF_SAMEASSYSTEM, "50 150 255", IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "FRAMEWIDTH", NULL, NULL, IUPAF_SAMEASSYSTEM, "1", IUPAF_NO_INHERIT);
  //  iupClassRegisterAttribute(ic, "POSX", NULL, iGLScrollBoxSetPosXAttrib, "0", NULL, IUPAF_NO_INHERIT);
//  iupClassRegisterAttribute(ic, "POSY", NULL, iGLScrollBoxSetPosYAttrib, "0", NULL, IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "SCROLLBARSIZE", NULL, NULL, IUPAF_SAMEASSYSTEM, "10", IUPAF_NO_INHERIT);

  return ic;
}

Ihandle* IupGLScrollBox(Ihandle* child)
{
  void *children[2];
  children[0] = (void*)child;
  children[1] = NULL;
  return IupCreatev("glscrollbox", children);
}
