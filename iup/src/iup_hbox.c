/** \file
 * \brief Hbox Control.
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


static int iHboxSetRasterSizeAttrib(Ihandle* ih, const char* value)
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
      ih->userheight = 0;
      ih->userwidth = s;
    }
  }
  iupAttribSetStr(ih, "SIZE", NULL); /* clear SIZE in hash table */
  return 0;
}

static int iHboxSetSizeAttrib(Ihandle* ih, const char* value)
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
      ih->userheight = 0;
      ih->userwidth = iupWIDTH2RASTER(s, charwidth);
    }
  }
  return 1;
}

static int iHboxSetAlignmentAttrib(Ihandle* ih, const char* value)
{
  if (iupStrEqualNoCase(value, "ABOTTOM"))
    ih->data->alignment = IUP_ALIGN_ABOTTOM;
  else if (iupStrEqualNoCase(value, "ACENTER"))
    ih->data->alignment = IUP_ALIGN_ACENTER;
  else if (iupStrEqualNoCase(value, "ATOP"))
    ih->data->alignment = IUP_ALIGN_ATOP;
  return 0;
}

static char* iHboxGetAlignmentAttrib(Ihandle* ih)
{
  static char* align2str[3] = {"ATOP", "ACENTER", "ABOTTOM"};
  return align2str[ih->data->alignment];
}

static void iHboxComputeNaturalSizeMethod(Ihandle* ih)
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
    int children_natural_totalwidth = 0;
    int children_natural_maxwidth = 0;
    int children_natural_maxheight = 0;
    for (child = ih->firstchild; child; child = child->brother)
    {
      /* update child natural size first */
      iupClassObjectComputeNaturalSize(child);

      if (!child->floating)
      {
        children_expand |= child->expand;
        children_natural_totalwidth += child->naturalwidth;
        children_natural_maxwidth = iupMAX(children_natural_maxwidth, child->naturalwidth);
        children_natural_maxheight = iupMAX(children_natural_maxheight, child->naturalheight);
        children_count++;
      }
    }

    ih->expand &= children_expand; /* compose but only expand where the box can expand */

    if (iupStrBoolean(iupAttribGetStr(ih, "HOMOGENEOUS")))
      children_natural_totalwidth = children_natural_maxwidth*children_count;

    /* compute the Hbox contents natural size */
    children_naturalwidth  = children_natural_totalwidth + (children_count-1)*ih->data->gap + 2*ih->data->margin_x;
    children_naturalheight = children_natural_maxheight + 2*ih->data->margin_y;

    ih->naturalwidth = iupMAX(ih->naturalwidth, children_naturalwidth);
    ih->naturalheight = iupMAX(ih->naturalheight, children_naturalheight);
  }
}

static int iHboxCalcEmptyWidth(Ihandle *ih, int expand)
{
  Ihandle* child;
  int children_naturalwidth;

  int children_natural_totalwidth=0;
  int children_natural_maxwidth=0;
  int children_count=0;
  int expand_count=0;
  for (child = ih->firstchild; child; child = child->brother)
  {
    if (!child->floating)
    {
      if (child->naturalwidth > children_natural_maxwidth)
        children_natural_maxwidth = child->naturalwidth;
      children_natural_totalwidth += child->naturalwidth;
      children_count++;

      if (child->expand & expand) 
        expand_count++;
    }
  }
  if (expand_count == 0)
    return 0;

  if (iupStrBoolean(iupAttribGetStr(ih, "HOMOGENEOUS")))
    children_natural_totalwidth = children_natural_maxwidth*children_count;

  children_naturalwidth = children_natural_totalwidth + (children_count-1)*ih->data->gap + 2*ih->data->margin_x;
  return (ih->currentwidth - children_naturalwidth)/expand_count;

}

static int iHBoxGetExpandChildren(Ihandle* ih)
{
  if (iupStrBoolean(iupAttribGetStr(ih, "EXPANDCHILDREN")))
    return IUP_EXPAND_HEIGHT;   /* in horiz. box, expand vertically */
  else
    return 0;
}

static void iHboxSetCurrentSizeMethod(Ihandle* ih, int w, int h, int shrink)
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
    int d, d0 = 0, d1 = 0, client_height, expand_children;

    expand_children = iHBoxGetExpandChildren(ih);
    if (expand_children)
      ih->expand |= expand_children;

    /* must calculate the space left for each control to grow inside the container */
    /* W1 means there is an EXPAND enabled inside */
    if (ih->expand & IUP_EXPAND_W1)
      d1 = iHboxCalcEmptyWidth(ih, IUP_EXPAND_W1);
    /* Not W1 and W0 means that EXPAND is not enabled but there are IupFill(s) inside */
    else if (ih->expand & IUP_EXPAND_W0)
      d0 = iHboxCalcEmptyWidth(ih, IUP_EXPAND_W0);

    client_height = ih->currentheight - 2*ih->data->margin_y;
    if (client_height<0) client_height=0;

    for (child = ih->firstchild; child; child = child->brother)
    {
      if (!child->floating)
      {
        if (expand_children)
          child->expand |= expand_children;

        d = (child->expand & IUP_EXPAND_W1)? d1: ((child->expand & IUP_EXPAND_W0)? d0: 0);
        iupClassObjectSetCurrentSize(child, child->naturalwidth+d, client_height, shrink);
      }
      else
      {
        /* update children to their own natural size */
        iupClassObjectSetCurrentSize(child, child->naturalwidth, child->naturalheight, shrink);
      }
    }
  }
}

static void iHboxSetPositionMethod(Ihandle* ih, int x, int y)
{
  int dy, client_height, child_maxwidth = 0;
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
        if (child->currentwidth > child_maxwidth)
          child_maxwidth = child->currentwidth;
      }
    }
  }

  client_height = ih->currentheight - 2*ih->data->margin_y;
  if (client_height<0) client_height=0;

  for (child = ih->firstchild; child; child = child->brother)
  {
    if (!child->floating)
    {
      if (ih->data->alignment == IUP_ALIGN_ACENTER)
        dy = (client_height - child->currentheight)/2;
      else if (ih->data->alignment == IUP_ALIGN_ABOTTOM)
        dy = client_height - child->currentheight;
      else  /* IUP_ALIGN_ATOP */
        dy = 0;
                      
      /* update child */
      iupClassObjectSetPosition(child, x, y+dy);

      /* calculate next */
      if (child_maxwidth)
        x += child_maxwidth + ih->data->gap;
      else
        x += child->currentwidth + ih->data->gap;
    }
  }
}


/******************************************************************************/


Ihandle *IupHboxv(Ihandle **children)
{
  return IupCreatev("hbox", (void**)children);
}

Ihandle *IupHbox(Ihandle* child, ...)
{
  Ihandle **children;
  Ihandle *ih;

  va_list arglist;
  va_start(arglist, child);
  children = (Ihandle **)iupObjectGetParamList(child, arglist);
  va_end(arglist);

  ih = IupCreatev("hbox", (void**)children);
  free(children);

  return ih;
}

Iclass* iupHboxGetClass(void)
{
  Iclass* ic = iupBoxClassBase();

  ic->name = "hbox";

  /* Class functions */
  ic->ComputeNaturalSize = iHboxComputeNaturalSizeMethod;
  ic->SetCurrentSize = iHboxSetCurrentSizeMethod;
  ic->SetPosition = iHboxSetPositionMethod;

  /* Overwrite Common */
  iupClassRegisterAttribute(ic, "SIZE", iupBaseGetSizeAttrib, iHboxSetSizeAttrib, NULL, IUP_NOT_MAPPED, IUP_NO_INHERIT);
  iupClassRegisterAttribute(ic, "RASTERSIZE", iupBaseGetRasterSizeAttrib, iHboxSetRasterSizeAttrib, NULL, IUP_NOT_MAPPED, IUP_NO_INHERIT);

  /* Hbox only */
  iupClassRegisterAttribute(ic, "ALIGNMENT", iHboxGetAlignmentAttrib, iHboxSetAlignmentAttrib, "ATOP", IUP_NOT_MAPPED, IUP_NO_INHERIT);

  return ic;
}
