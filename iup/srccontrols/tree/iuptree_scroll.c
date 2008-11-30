/** \file
 * \brief iuptree control
 * Functions used to scroll the tree
 *
 * See Copyright Notice in iup.h
 */

#include <stdio.h>
#include <stdlib.h>

#include "iup.h"
#include "iupcbs.h"
#include "iuptree.h"
#include "iupkey.h"

#include "iup_object.h"
#include "iup_attrib.h"
#include "iup_str.h"
#include "iup_drv.h"
#include "iup_drvfont.h"
#include "iup_stdcontrols.h"
#include "iup_controls.h"

#include <cd.h>

#include "iuptree_scroll.h"
#include "iuptree_draw.h"
#include "iuptree_def.h"
#include "iuptree_getset.h"
#include "iuptree_find.h"


int iupTreeScrollDown(Ihandle* ih)
{
  if(ih->data->selected_y < ITREE_NODE_Y + ITREE_NODE_Y / 2)
    IupSetfAttribute(ih, "POSY", "%f", IupGetFloat(ih, "POSY") + IupGetFloat(ih, "DY") / iupTreeFindNumNodesInCanvas(ih));

  return IUP_DEFAULT;
}

int iupTreeScrollUp(Ihandle* ih)
{
  if(ih->data->selected_y > ih->data->YmaxC - ITREE_TREE_TOP_MARGIN - ITREE_NODE_Y - ITREE_NODE_Y)
    IupSetfAttribute(ih, "POSY", "%f", IupGetFloat(ih, "POSY") - IupGetFloat(ih, "DY") / iupTreeFindNumNodesInCanvas(ih));

  return IUP_DEFAULT;
}

int iupTreeScrollPgUp(Ihandle* ih)
{
  float temp = IupGetFloat(ih, "POSY") - IupGetFloat(ih, "DY");

  if(temp < 0.0)
    temp = 0.0;
  IupSetfAttribute(ih, "POSY", "%f", temp);

  return IUP_DEFAULT;
}

int iupTreeScrollPgDn(Ihandle* ih)
{
  double temp = IupGetFloat(ih, "POSY") + IupGetFloat(ih, "DY");

  if(temp > 1.0 - IupGetFloat(ih, "DY"))
    temp  = 1.0 - IupGetFloat(ih, "DY");
  IupSetfAttribute(ih, "POSY", "%f", temp);

  return IUP_DEFAULT;
}

int iupTreeScrollEnd(Ihandle* ih)
{
  IupSetfAttribute(ih, "POSY", "%f", 1.0 - IupGetFloat(ih, "DY"));

  return IUP_DEFAULT;
}

int iupTreeScrollBegin(Ihandle* ih)
{
  IupSetAttribute(ih, "POSY", "0.0");

  return IUP_DEFAULT;
}

int iupTreeScrollShow(Ihandle* ih)
{
  /* If node is above the canvas, scrolls down to make it visible */
  if(ih->data->selected_y > ih->data->YmaxC - ITREE_TREE_TOP_MARGIN)
    IupSetfAttribute(ih, "POSY", "%f", atof(iupTreeGSGetValue(ih)) / (float)iupTreeFindNumNodes(ih));
  else
  /* If node is below the canvas, scrolls up to make it visible */
  if(ih->data->selected_y < 0)
    IupSetfAttribute(ih, "POSY", "%f", (atof(iupTreeGSGetValue(ih)) - (iupTreeFindNumNodesInCanvas(ih) - 1)) / (float)iupTreeFindNumNodes(ih));

  return IUP_DEFAULT;
}
