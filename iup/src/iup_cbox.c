/** \file
 * \brief cbox control
 *
 * See Copyright Notice in iup.h
 */

#include <stdio.h>
#include <stdlib.h>

#include "iup.h"
#include "iupcbs.h"

#include "iup_object.h"
#include "iup_attrib.h"
#include "iup_str.h"
#include "iup_drv.h"
#include "iup_drvfont.h"
#include "iup_stdcontrols.h"
#include "iup_layout.h"


static int iCboxCreateMethod(Ihandle* ih, void** params)
{
  if (params)
  {
    Ihandle** iparams = (Ihandle**)params;
    while (*iparams) 
    {
      IupAppend(ih, *iparams);
      iparams++;
    }
  }

  return IUP_NOERROR;
}

static int iCboxMapMethod(Ihandle* ih)
{
  ih->handle = (InativeHandle*)-1; /* fake value just to indicate that it is already mapped */
  return IUP_NOERROR;
}

static void iCboxComputeNaturalSizeMethod(Ihandle* ih)
{
  iupBaseContainerUpdateExpand(ih);

  /* always initialize the natural size using the user size */
  ih->naturalwidth = ih->userwidth;
  ih->naturalheight = ih->userheight;

  if (ih->firstchild)
  {
    Ihandle* child;
    int children_expand, 
        children_naturalwidth, children_naturalheight;
    int cx, cy;

    /* calculate total children natural size (even for hidden children) */
    children_expand = 0;
    children_naturalwidth = 0;
    children_naturalheight = 0;
    for (child = ih->firstchild; child; child = child->brother)
    {
      /* update child natural size first */
      iupClassObjectComputeNaturalSize(child);

      cx = iupAttribGetInt(child, "CX");
      cy = iupAttribGetInt(child, "CY");

      children_expand |= child->expand;
      children_naturalwidth = iupMAX(children_naturalwidth, cx+child->naturalwidth);
      children_naturalheight = iupMAX(children_naturalheight, cy+child->naturalheight);
    }

    ih->expand &= children_expand; /* compose but only expand where the box can expand */

    ih->naturalwidth = iupMAX(ih->naturalwidth, children_naturalwidth);
    ih->naturalheight = iupMAX(ih->naturalheight, children_naturalheight);
  }
}

static void iCboxSetCurrentSizeMethod(Ihandle* ih, int w, int h, int shrink)
{
  Ihandle* child;

  if (shrink)
  {
    /* if expand use the given size, else use the natural size */
    ih->currentwidth  = (ih->expand & IUP_EXPAND_WIDTH)? w: ih->naturalwidth;
    ih->currentheight = (ih->expand & IUP_EXPAND_HEIGHT)? h: ih->naturalheight;
  }
  else
  {
    /* if expand use the given size (if greater than natural size), else use the natural size */
    ih->currentwidth  = (ih->expand & IUP_EXPAND_WIDTH)? iupMAX(ih->naturalwidth, w): ih->naturalwidth;
    ih->currentheight = (ih->expand & IUP_EXPAND_HEIGHT)? iupMAX(ih->naturalheight, h) : ih->naturalheight;
  }

  for (child = ih->firstchild; child; child = child->brother)
  {
    /* update children to their own natural size */
    iupClassObjectSetCurrentSize(child, child->naturalwidth, child->naturalheight, shrink);
  }
}

static void iCboxSetPositionMethod(Ihandle* ih, int x, int y)
{
  int cx, cy;
  Ihandle* child;

  iupBaseSetPositionMethod(ih, x, y);

  for (child = ih->firstchild; child; child = child->brother)
  {
    cx = iupAttribGetInt(child, "CX");
    cy = iupAttribGetInt(child, "CY");

    /* update child */
    iupClassObjectSetPosition(child, x+cx, y+cy);
  }
}


/******************************************************************************/


Ihandle *IupCboxv(Ihandle** children)
{
  return IupCreatev("cbox", (void**)children);
}

Ihandle *IupCbox (Ihandle * child,...)
{
  Ihandle **children;
  Ihandle *ih;

  va_list arglist;
  va_start(arglist, child);
  children = (Ihandle **)iupObjectGetParamList(child, arglist);
  va_end(arglist);

  ih = IupCreatev("cbox", (void**)children);
  free(children);

  return ih;
}

Iclass* iupCboxGetClass(void)
{
  Iclass* ic = iupClassNew(NULL);

  ic->name = "cbox";
  ic->format = "g"; /* array of Ihandle */
  ic->nativetype = IUP_TYPEVOID;
  ic->childtype = IUP_CHILDMANY;
  ic->is_interactive = 0;

  /* Class functions */
  ic->Create = iCboxCreateMethod;
  ic->Map = iCboxMapMethod;

  ic->ComputeNaturalSize = iCboxComputeNaturalSizeMethod;
  ic->SetCurrentSize = iCboxSetCurrentSizeMethod;
  ic->SetPosition = iCboxSetPositionMethod;

  /* Common */
  iupBaseRegisterCommonAttrib(ic);

  /* Base Container */
  iupClassRegisterAttribute(ic, "EXPAND", iupBaseContainerGetExpandAttrib, NULL, "YES", IUP_NOT_MAPPED, IUP_NO_INHERIT);
  iupClassRegisterAttribute(ic, "CLIENTSIZE", iupBaseGetRasterSizeAttrib, iupBaseNoSetAttrib, NULL, IUP_NOT_MAPPED, IUP_NO_INHERIT);

  return ic;
}
