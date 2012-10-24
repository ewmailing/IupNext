/** \file
 * \brief iupscrollbox control
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


/*****************************************************************************\
|* Canvas Callbacks                                                          *|
\*****************************************************************************/

static void iScrollBoxUpdatePosition(Ihandle* ih, float posx, float posy)
{
  if (IupGetGlobal("GTKVERSION"))
  {
    /* In GTK, IupCanvas handle is NOT the actual container of the child native handle */
    posx -= (float)ih->x;
    posy -= (float)ih->y;
  }

  iupBaseSetPosition(ih->firstchild, -(int)posx, -(int)posy);
}

static int iScrollBoxScroll_CB(Ihandle *ih, int op, float posx, float posy)
{
  if (ih->firstchild)
  {
    iScrollBoxUpdatePosition(ih, posx, posy);
    iupLayoutUpdate(ih->firstchild);
  }
  (void)op;
  return IUP_DEFAULT;
}



/*****************************************************************************\
|* Methods                                                                   *|
\*****************************************************************************/

static void iScrollBoxLayoutUpdate(Ihandle* ih)
{
  int w, h;
  IupGetIntInt(ih, "DRAWSIZE", &w, &h);
  IupSetfAttribute(ih, "DX", "%d", w);
  IupSetfAttribute(ih, "DY", "%d", h);

  if (ih->firstchild)
  {
    iScrollBoxUpdatePosition(ih, IupGetFloat(ih, "POSX"), IupGetFloat(ih, "POSY"));
    iupLayoutUpdate(ih->firstchild);
  }
}

static void iScrollBoxComputeNaturalSizeMethod(Ihandle* ih, int *w, int *h, int *expand)
{
  if (ih->firstchild)
  {
    /* update child natural size */
    iupBaseComputeNaturalSize(ih->firstchild);

    IupSetfAttribute(ih, "XMAX", "%d", ih->firstchild->currentwidth);
    IupSetfAttribute(ih, "YMAX", "%d", ih->firstchild->currentheight);
  }

  /* ScrollBox size does not depends on the child size */
  (void)w;
  (void)h;
  (void)expand;
}

static void iScrollBoxSetChildrenCurrentSizeMethod(Ihandle* ih, int shrink)
{
  /* update child to its own natural size */
  if (ih->firstchild)
    iupBaseSetCurrentSize(ih->firstchild, ih->firstchild->naturalwidth, ih->firstchild->naturalheight, shrink);
}

static void iScrollBoxSetChildrenPositionMethod(Ihandle* ih, int x, int y)
{
  if (ih->firstchild)
  {
    iScrollBoxUpdatePosition(ih, IupGetFloat(ih, "POSX"), IupGetFloat(ih, "POSY"));

    /* because ScrollBox is a native container, 
       child position is restarted at (0,0) */
    (void)x;
    (void)y;
  }
}

static int iScrollBoxCreateMethod(Ihandle* ih, void** params)
{
  /* Setting callbacks */
  IupSetCallback(ih, "SCROLL_CB",    (Icallback)iScrollBoxScroll_CB);

  if (params)
  {
    Ihandle** iparams = (Ihandle**)params;
    if (iparams[0]) IupAppend(ih, iparams[0]);
  }

  return IUP_NOERROR;
}

Iclass* iupScrollBoxNewClass(void)
{
  Iclass* ic = iupClassNew(iupRegisterFindClass("canvas"));

  ic->name   = "scrollbox";
  ic->format = "h";   /* one ihandle */
  ic->nativetype = IUP_TYPECANVAS;
  ic->childtype  = IUP_CHILDMANY+1;  /* 1 child */
  ic->is_interactive = 1;

  /* Class functions */
  ic->New = iupScrollBoxNewClass;
  ic->Create  = iScrollBoxCreateMethod;

  ic->ComputeNaturalSize = iScrollBoxComputeNaturalSizeMethod;
  ic->SetChildrenCurrentSize = iScrollBoxSetChildrenCurrentSizeMethod;
  ic->SetChildrenPosition = iScrollBoxSetChildrenPositionMethod;
  ic->LayoutUpdate = iScrollBoxLayoutUpdate;

  /* Base Container */
  iupClassRegisterAttribute(ic, "EXPAND", iupBaseContainerGetExpandAttrib, NULL, IUPAF_SAMEASSYSTEM, "YES", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "CLIENTOFFSET", iupBaseGetClientOffsetAttrib, NULL, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_READONLY|IUPAF_NO_INHERIT);
  {
    IattribGetFunc drawsize_get = NULL;
    iupClassRegisterGetAttribute(ic, "DRAWSIZE", &drawsize_get, NULL, NULL, NULL, NULL);
    iupClassRegisterAttribute(ic, "CLIENTSIZE", drawsize_get, NULL, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_READONLY|IUPAF_NO_INHERIT);
  }

  /* replace IupCanvas behavior */
  iupClassRegisterReplaceAttribFunc(ic, "BGCOLOR", iupBaseNativeParentGetBgColorAttrib, NULL);
  iupClassRegisterReplaceAttribDef(ic, "BGCOLOR", "DLGBGCOLOR", NULL);
  iupClassRegisterReplaceAttribDef(ic, "BORDER", "NO", NULL);
  iupClassRegisterReplaceAttribFlags(ic, "BORDER", IUPAF_NO_INHERIT);
  iupClassRegisterReplaceAttribDef(ic, "SCROLLBAR", "YES", NULL);

  return ic;
}

Ihandle* IupScrollBox(Ihandle* child)
{
  void *children[2];
  children[0] = (void*)child;
  children[1] = NULL;
  return IupCreatev("scrollbox", children);
}
