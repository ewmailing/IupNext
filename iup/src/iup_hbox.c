/** \file
 * \brief Hbox Control.
 *
 * See Copyright Notice in "iup.h"
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
        children_natural_maxwidth = iupMAX(children_natural_maxwidth, child->naturalwidth);
        children_natural_maxheight = iupMAX(children_natural_maxheight, child->naturalheight);
        children_count++;
      }
    }

    /* reset to max natural width and/or height if NORMALIZESIZE is defined */
    iupNormalizeSizeBoxChild(ih, children_natural_maxwidth, children_natural_maxheight);

    for (child = ih->firstchild; child; child = child->brother)
    {
      if (!child->floating)
        children_natural_totalwidth += child->naturalwidth;
    }

    ih->expand &= children_expand; /* compose but only expand where the box can expand */

    /* leave room at the element for the maximum natural size of the children when homogeneous */
    if (iupStrBoolean(iupAttribGetStr(ih, "HOMOGENEOUS")))
      children_natural_totalwidth = children_natural_maxwidth*children_count;

    /* compute the Hbox contents natural size */
    children_naturalwidth  = children_natural_totalwidth + (children_count-1)*ih->data->gap + 2*ih->data->margin_x;
    children_naturalheight = children_natural_maxheight + 2*ih->data->margin_y;

    ih->naturalwidth = iupMAX(ih->naturalwidth, children_naturalwidth);
    ih->naturalheight = iupMAX(ih->naturalheight, children_naturalheight);

    /* Store to be used in iHboxCalcEmptyWidth */
    ih->data->children_naturalsize = children_naturalwidth;
  }
}

static int iHboxCalcHomogeneousWidth(Ihandle *ih)
{
  Ihandle* child;
  int homogeneous_width;

  int children_count=0;
  for (child = ih->firstchild; child; child = child->brother)
  {
    if (!child->floating)
      children_count++;
  }
  if (children_count == 0)
    return 0;

  /* equal spaces for all elements */
  homogeneous_width = (ih->currentwidth - (children_count-1)*ih->data->gap - 2*ih->data->margin_x)/children_count;
  if (homogeneous_width<0) homogeneous_width = 0;
  return homogeneous_width;
}

static int iHboxCalcEmptyWidth(Ihandle *ih, int expand)
{
  /* This is the space that the child can be expanded. */
  Ihandle* child;
  int empty_width;

  int expand_count=0;
  for (child = ih->firstchild; child; child = child->brother)
  {
    if (!child->floating)
    {
      if (child->expand & expand) 
        expand_count++;
    }
  }
  if (expand_count == 0)
    return 0;

  /* equal spaces for all expandable elements */
  empty_width = (ih->currentwidth - ih->data->children_naturalsize)/expand_count;  
  if (empty_width<0) empty_width = 0;
  return empty_width;
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
  iupBaseContainerSetCurrentSizeMethod(ih, w, h, shrink);

  if (ih->firstchild)
  {
    /* update children */
    Ihandle* child;
    int empty_w0 = 0, empty_w1 = 0, client_height, expand_children, homogeneous_width = 0;

    expand_children = iHBoxGetExpandChildren(ih);
    if (expand_children)
      ih->expand |= expand_children;

    if (iupStrBoolean(iupAttribGetStr(ih, "HOMOGENEOUS")))
    {
      homogeneous_width = iHboxCalcHomogeneousWidth(ih);
      ih->data->homogeneous_size = homogeneous_width;
    }
    else
    {
      /* must calculate the space left for each control to grow inside the container */
      /* W1 means there is an EXPAND enabled inside */
      if (ih->expand & IUP_EXPAND_W1)
        empty_w1 = iHboxCalcEmptyWidth(ih, IUP_EXPAND_W1);
      /* Not W1 and W0 means that EXPAND is not enabled but there are IupFill(s) inside */
      else if (ih->expand & IUP_EXPAND_W0)
        empty_w0 = iHboxCalcEmptyWidth(ih, IUP_EXPAND_W0);
    }

    client_height = ih->currentheight - 2*ih->data->margin_y;
    if (client_height<0) client_height=0;

    for (child = ih->firstchild; child; child = child->brother)
    {
      if (!child->floating)
      {
        if (expand_children)
          child->expand |= expand_children;

        if (homogeneous_width)
          iupClassObjectSetCurrentSize(child, homogeneous_width, client_height, shrink);
        else
        {
          int empty = (child->expand & IUP_EXPAND_W1)? empty_w1: ((child->expand & IUP_EXPAND_W0)? empty_w0: 0);
          iupClassObjectSetCurrentSize(child, child->naturalwidth+empty, client_height, shrink);
        }
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
  int dy, client_height;
  Ihandle* child;

  iupBaseSetPositionMethod(ih, x, y);

  x += ih->data->margin_x;
  y += ih->data->margin_y;

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
      if (ih->data->homogeneous_size)
        x += ih->data->homogeneous_size + ih->data->gap;
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
  iupClassRegisterAttribute(ic, "SIZE", iupBaseGetSizeAttrib, iHboxSetSizeAttrib, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "RASTERSIZE", iupBaseGetRasterSizeAttrib, iHboxSetRasterSizeAttrib, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);

  /* Hbox only */
  iupClassRegisterAttribute(ic, "ALIGNMENT", iHboxGetAlignmentAttrib, iHboxSetAlignmentAttrib, "ATOP", NULL, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);

  return ic;
}
