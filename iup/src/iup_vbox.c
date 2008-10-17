/** \file
 * \brief Vbox Control.
 *
 * See Copyright Notice in iup.ih
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
#include "iup_box.h"


static int iVboxSetRasterSizeAttrib(Ihandle* ih, const char* value)
{
  if (!value)
  {
    ih->userwidth = 0;
    ih->userheight = 0;
  }
  else
  {
    int s = 0;
    iupStrToInt(value, &s);
    if (s > 0) 
    {
      ih->userheight = s;
      ih->userwidth = 0;
    }
  }
  iupAttribSetStr(ih, "SIZE", NULL); /* clear SIZE in hash table */
  return 0;
}

static int iVboxSetSizeAttrib(Ihandle* ih, const char* value)
{
  if (!value)
  {
    ih->userwidth = 0;
    ih->userheight = 0;
  }
  else
  {
    int s = 0;
    iupStrToInt(value, &s);
    if (s > 0) 
    {
      int charwidth, charheight;
      iupdrvFontGetCharSize(ih, &charwidth, &charheight);
      ih->userheight = iupHEIGHT2RASTER(s, charheight);
      ih->userwidth = 0;
    }
  }
  return 1;
}

static int iVboxSetAlignmentAttrib(Ihandle* ih, const char* value)
{
  if (iupStrEqualNoCase(value, "ARIGHT"))
    ih->data->alignment = IUP_ALIGN_ARIGHT;
  else if (iupStrEqualNoCase(value, "ACENTER"))
    ih->data->alignment = IUP_ALIGN_ACENTER;
  else if (iupStrEqualNoCase(value, "ALEFT"))
    ih->data->alignment = IUP_ALIGN_ALEFT;
  return 0;
}

static char* iVboxGetAlignmentAttrib(Ihandle* ih)
{
  char* align2str[3] = {"ALEFT", "ACENTER", "ARIGHT"};
  char *str = iupStrGetMemory(50);
  sprintf(str, "%s", align2str[ih->data->alignment]);
  return str;
}

static void iVboxComputeNaturalSizeMethod(Ihandle* ih)
{
  iupBaseContainerUpdateExpand(ih);

  /* always initialize the natural size using the user size */
  ih->naturalwidth = ih->userwidth;
  ih->naturalheight = ih->userheight;

  if (ih->firstchild)
  {
    Ihandle* child;
    int children_naturalwidth, children_naturalheight;

    /* calculate total children natural size */
    int children_expand = 0;
    int children_count = 0;
    int children_natural_maxwidth = 0;
    int children_natural_maxheight = 0;
    int children_natural_totalheight = 0;
    for (child = ih->firstchild; child; child = child->brother)
    {
      /* update child natural size first */
      iupClassObjectComputeNaturalSize(child);

      if (!child->floating)
      {
        children_expand |= child->expand;
        children_natural_maxwidth = iupMAX(children_natural_maxwidth, child->naturalwidth);
        children_natural_maxheight = iupMAX(children_natural_maxheight, child->naturalheight);
        children_natural_totalheight += child->naturalheight;
        children_count++;
      }
    }

    ih->expand &= children_expand; /* compose but only expand where the box can expand */

    if (iupStrBoolean(iupAttribGetStr(ih, "HOMOGENEOUS")))
      children_natural_totalheight = children_natural_maxheight*children_count;

    /* compute the Vbox contents natural size */
    children_naturalwidth  = children_natural_maxwidth + 2*ih->data->margin_x;
    children_naturalheight = children_natural_totalheight + (children_count-1)*ih->data->gap + 2*ih->data->margin_y;

    ih->naturalwidth = iupMAX(ih->naturalwidth, children_naturalwidth);
    ih->naturalheight = iupMAX(ih->naturalheight, children_naturalheight);
  }
}

static int iVboxCalcEmptyHeight(Ihandle *ih, int expand)
{
  Ihandle* child;
  int children_naturalheight;

  int children_natural_totalheight=0;
  int children_natural_maxheight=0;
  int children_count=0;
  int expand_count=0;
  for (child = ih->firstchild; child; child = child->brother)
  {
    if (!child->floating)
    {
      if (child->naturalheight > children_natural_maxheight)
        children_natural_maxheight = child->naturalheight;
      children_natural_totalheight += child->naturalheight;
      children_count++;

      if (child->expand & expand) 
        expand_count++;
    }
  }
  if (expand_count == 0)
    return 0;

  if (iupStrBoolean(iupAttribGetStr(ih, "HOMOGENEOUS")))
    children_natural_totalheight = children_natural_maxheight*children_count;

  children_naturalheight = children_natural_totalheight + (children_count-1)*ih->data->gap + 2*ih->data->margin_y;
  return (ih->currentheight - children_naturalheight)/expand_count;
}

static int iVBoxGetExpandChildren(Ihandle* ih)
{
  if (iupStrBoolean(iupAttribGetStr(ih, "EXPANDCHILDREN")))
    return IUP_EXPAND_WIDTH;    /* in vert. box, expand horizontally */
  else
    return 0;
}

static void iVboxSetCurrentSizeMethod(Ihandle* ih, int w, int h, int shrink)
{
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

  if (ih->firstchild)
  {
    /* update children */
    Ihandle* child;
    int d, d0 = 0, d1 = 0, client_width, expand_children;

    expand_children = iVBoxGetExpandChildren(ih);
    if (expand_children)
      ih->expand |= expand_children;

    /* must calculate the space left for each control to grow inside the container */
    /* H1 means there is an EXPAND enabled inside */
    if (ih->expand & IUP_EXPAND_H1)
      d1 = iVboxCalcEmptyHeight(ih, IUP_EXPAND_H1);
    /* Not H1 and H0 means that EXPAND is not enabled, but there are IupFill(s) inside */
    else if (ih->expand & IUP_EXPAND_H0)
      d0 = iVboxCalcEmptyHeight(ih, IUP_EXPAND_H0);

    client_width = ih->currentwidth - 2*ih->data->margin_x;
    if (client_width<0) client_width=0;

    for (child = ih->firstchild; child; child = child->brother)
    {
      if (!child->floating)
      {
        if (expand_children)
          child->expand |= expand_children;

        d = (child->expand & IUP_EXPAND_H1)? d1: ((child->expand & IUP_EXPAND_H0)? d0: 0);
        iupClassObjectSetCurrentSize(child, client_width, child->naturalheight+d, shrink);
      }
      else
      {
        /* update children to their own natural size */
        iupClassObjectSetCurrentSize(child, child->naturalwidth, child->naturalheight, shrink);
      }
    }
  }
}

static void iVboxSetPositionMethod(Ihandle* ih, int x, int y)
{
  int dx, client_width, child_maxheight = 0;
  Ihandle* child;

  iupBaseSetPositionMethod(ih, x, y);

  x += ih->data->margin_x;
  y += ih->data->margin_y;

  if (iupStrBoolean(iupAttribGetStr(ih, "HOMOGENEOUS")))
  {
    for (child = ih->firstchild; child; child = child->brother)
    {
      if (!child->floating)
      {
        if (child->currentheight > child_maxheight)
          child_maxheight = child->currentheight;
      }
    }
  }

  client_width = ih->currentwidth - 2*ih->data->margin_x;
  if (client_width<0) client_width=0;

  for (child = ih->firstchild; child; child = child->brother)
  {
    if (!child->floating)
    {
      if (ih->data->alignment == IUP_ALIGN_ACENTER)
        dx = (client_width - child->currentwidth)/2;
      else if (ih->data->alignment == IUP_ALIGN_ARIGHT)
        dx = client_width - child->currentwidth;
      else  /* IUP_ALIGN_ALEFT */
        dx = 0;
                      
      /* update child */
      iupClassObjectSetPosition(child, x+dx, y);

      /* calculate next */
      if (child_maxheight)
        y += child_maxheight + ih->data->gap;
      else
        y += child->currentheight + ih->data->gap;
    }
  }
}


/******************************************************************************/


Ihandle *IupVboxv(Ihandle **children)
{
  return IupCreatev("vbox", (void**)children);
}

Ihandle *IupVbox(Ihandle* child, ...)
{
  Ihandle **children;
  Ihandle *ih;

  va_list arglist;
  va_start(arglist, child);
  children = (Ihandle **)iupObjectGetParamList(child, arglist);
  va_end(arglist);

  ih = IupCreatev("vbox", (void**)children);
  free(children);

  return ih;
}

Iclass* iupVboxGetClass(void)
{
  Iclass* ic = iupBoxClassBase();

  ic->name = "vbox";

  /* Class functions */
  ic->ComputeNaturalSize = iVboxComputeNaturalSizeMethod;
  ic->SetCurrentSize = iVboxSetCurrentSizeMethod;
  ic->SetPosition = iVboxSetPositionMethod;

  /* Overwrite Common */
  iupClassRegisterAttribute(ic, "SIZE", iupBaseGetSizeAttrib, iVboxSetSizeAttrib, NULL, IUP_NOT_MAPPED, IUP_NO_INHERIT);
  iupClassRegisterAttribute(ic, "RASTERSIZE", iupBaseGetRasterSizeAttrib, iVboxSetRasterSizeAttrib, NULL, IUP_NOT_MAPPED, IUP_NO_INHERIT);

  /* Vbox only */
  iupClassRegisterAttribute(ic, "ALIGNMENT", iVboxGetAlignmentAttrib, iVboxSetAlignmentAttrib, "ALEFT", IUP_NOT_MAPPED, IUP_NO_INHERIT);

  return ic;
}
