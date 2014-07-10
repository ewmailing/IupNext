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
#include "iup_glscrollbars.h"
#include "iup_glsubcanvas.h"


static int iGLScrollBoxBUTTON_CB(Ihandle *ih, int button, int pressed, int x, int y, char* status)
{
  if (button != IUP_BUTTON1)
    return IUP_DEFAULT;

  if (iupGLScrollbarsButton(ih, pressed, x, y))
    iupGLSubCanvasRedraw(ih);

  (void)status;
  return IUP_DEFAULT;
}

static int iGLScrollBoxMOTION_CB(Ihandle *ih, int x, int y, char* status)
{
  if (iupGLScrollbarsMotion(ih, x, y))
    iupGLSubCanvasRedraw(ih);

  (void)status;
  return IUP_DEFAULT;
}

static int iGLScrollBoxENTERWINDOW_CB(Ihandle* ih, int x, int y)
{
  iupGLScrollbarsEnterWindow(ih, x, y);

  return iupGLSubCanvasRedraw(ih);
}

static int iGLScrollBoxLEAVEWINDOW_CB(Ihandle* ih)
{
  iupGLScrollbarsLeaveWindow(ih);

  return iupGLSubCanvasRedraw(ih);
}

static int iGLScrollBoxACTION_CB(Ihandle* ih)
{
  int active = iupAttribGetInt(ih, "ACTIVE");

  iupGLScrollbarsDraw(ih, active);

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
  IupSetCallback(ih, "GL_LEAVEWINDOW_CB", (Icallback)iGLScrollBoxLEAVEWINDOW_CB);
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
  ic->is_interactive = 0;

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
