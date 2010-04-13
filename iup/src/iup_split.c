/** \file
 * \brief iupsplit control
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
#include "iup_attrib.h"
#include "iup_str.h"
#include "iup_drv.h"
#include "iup_stdcontrols.h"
#include "iup_layout.h"
#include "iup_childtree.h"


#define ISPLIT_THICK 5

enum { ISPLIT_VERT, ISPLIT_HORIZ };

struct _IcontrolData
{
  int is_holding, showdrag;
  int start_pos, start_bar, start_size;

  int direction;  /* one of the types: ISPLIT_VERT, ISPLIT_HORIZ */
  int val;  /* split value: 0-1000, default 500 */
};

static int iSplitGetWidth1(Ihandle* ih)
{
  int width1 = ((ih->currentwidth-ISPLIT_THICK)*ih->data->val)/1000;
  if (width1 < 0) width1 = 0;
  return width1;
}

static int iSplitGetHeight1(Ihandle* ih)
{
  int height1 = ((ih->currentheight-ISPLIT_THICK)*ih->data->val)/1000;
  if (height1 < 0) height1 = 0;
  return height1;
}

static void iSplitSetBarPosition(Ihandle* ih, int cur_x, int cur_y)
{
  if (ih->data->direction == ISPLIT_VERT)
    ih->firstchild->x = ih->data->start_bar + (cur_x - ih->data->start_pos);
  else /* ISPLIT_HORIZ */
    ih->firstchild->y = ih->data->start_bar + (cur_y - ih->data->start_pos);

  IupSetAttribute(ih->firstchild, "ZORDER", "TOP");
  iupClassObjectLayoutUpdate(ih->firstchild);
}


/*****************************************************************************\
|* Callbacks of canvas bar                                                   *|
\*****************************************************************************/


static int iSplitMotion_CB(Ihandle* bar, int x, int y, char *status)
{
  Ihandle* ih = bar->parent;

  if (ih->data->is_holding)
  {
    if (iup_isbutton1(status))
    {
      int cur_x, cur_y;

      iupStrToIntInt(IupGetGlobal("CURSORPOS"), &cur_x, &cur_y, 'x');

      if (ih->data->direction == ISPLIT_VERT)
      {
        int width1 = iSplitGetWidth1(ih);
        width1 = ih->data->start_size + (cur_x - ih->data->start_pos);
        ih->data->val = (width1*1000)/(ih->currentwidth-ISPLIT_THICK);
      }
      else
      {
        int height1 = iSplitGetHeight1(ih);
        height1 = ih->data->start_size + (cur_y - ih->data->start_pos);
        ih->data->val = (height1*1000)/(ih->currentheight-ISPLIT_THICK);
      }

      if (ih->data->val < 0) ih->data->val = 0;
      if (ih->data->val > 1000) ih->data->val = 1000;

      if (ih->data->showdrag)
        IupRefresh(ih);  /* may affect all the elements in the dialog */
      else
        iSplitSetBarPosition(ih, cur_x, cur_y);
    }
    else
      ih->data->is_holding = 0;
  }

  (void)x;
  (void)y;
  return IUP_DEFAULT;
}

static int iSplitButton_CB(Ihandle* bar, int button, int pressed, int x, int y, char* status)
{
  Ihandle* ih = bar->parent;

  if (button!=IUP_BUTTON1)
    return IUP_DEFAULT;

  if (!ih->data->is_holding && pressed)
  {
    int cur_x, cur_y;

    ih->data->is_holding = 1;

    iupStrToIntInt(IupGetGlobal("CURSORPOS"), &cur_x, &cur_y, 'x');

    /* Save the cursor position and size */
    if (ih->data->direction == ISPLIT_VERT)
    {
      ih->data->start_bar = ih->firstchild->x;
      ih->data->start_pos = cur_x;
      ih->data->start_size = iSplitGetWidth1(ih);
    }
    else
    {
      ih->data->start_bar = ih->firstchild->y;
      ih->data->start_pos = cur_y;
      ih->data->start_size = iSplitGetHeight1(ih);
    }
  }
  else if (ih->data->is_holding && !pressed)
  {
    ih->data->is_holding = 0;

    if (!ih->data->showdrag)
      IupRefresh(ih);  /* may affect all the elements in the dialog */
  }

  (void)x;
  (void)y;
  (void)status;
  return IUP_DEFAULT;
}

static int iSplitFocus_CB(Ihandle* bar, int focus)
{
  Ihandle* ih = bar->parent;

  if (!ih || focus) /* use only kill focus */
    return IUP_DEFAULT;

  if (ih->data->is_holding)
    ih->data->is_holding = 0;

  return IUP_DEFAULT;
}


/*****************************************************************************\
|* Attributes                                                                *|
\*****************************************************************************/


static char* iSplitGetClientSize1Attrib(Ihandle* ih)
{
  int width, height;
  char* str = iupStrGetMemory(20);

  if (ih->data->direction == ISPLIT_VERT)
  {
    width = iSplitGetWidth1(ih);
    height = ih->currentheight;
  }
  else /* ISPLIT_HORIZ */
  {
    height = iSplitGetHeight1(ih);
    width = ih->currentwidth;
  }

  sprintf(str, "%dx%d", width, height);
  return str;
}

static char* iSplitGetClientSize2Attrib(Ihandle* ih)
{
  int width, height;
  char* str = iupStrGetMemory(20);

  if (ih->data->direction == ISPLIT_VERT)
  {
    int width1 = iSplitGetWidth1(ih);

    width = (ih->currentwidth-ISPLIT_THICK)-width1;
    if (width < 0) width = 0;

    height = ih->currentheight;
  }
  else /* ISPLIT_HORIZ */
  {
    int height1 = iSplitGetHeight1(ih);

    height = (ih->currentheight-ISPLIT_THICK)-height1;
    if (height < 0) height = 0;

    width = ih->currentwidth;
  }

  sprintf(str, "%dx%d", width, height);
  return str;
}

static int iSplitSetColorAttrib(Ihandle* ih, const char* value)
{
  IupSetAttribute(ih->firstchild, "BGCOLOR", value);
  return 0;  /* do not store value in hash table */
}

static int iSplitSetDirectionAttrib(Ihandle* ih, const char* value)
{
  if (ih->handle) /* only before map */
    return 0;

  if (iupStrEqual(value, "HORIZONTAL"))
    ih->data->direction = ISPLIT_HORIZ;
  else  /* Default = VERTICAL */
    ih->data->direction = ISPLIT_VERT;

  if (ih->data->direction == ISPLIT_VERT)
    IupSetAttribute(ih->firstchild, "CURSOR", "RESIZE_WE");
  else
    IupSetAttribute(ih->firstchild, "CURSOR", "RESIZE_NS");

  return 0;  /* do not store value in hash table */
}

static int iSplitSetValueAttrib(Ihandle* ih, const char* value)
{
  if (!value)
  {
    ih->data->val = -1;
    IupRefresh(ih);
  }
  else
  {
    int val;
    if (iupStrToInt(value, &val))
    {
      ih->data->val = val;
      IupRefresh(ih);
    }
  }

  return 0; /* do not store value in hash table */
}

static char* iSplitGetValueAttrib(Ihandle* ih)
{
  char* str = iupStrGetMemory(30);
  sprintf(str, "%d", ih->data->val);
  return str;
}

static int iSplitSetShowDragAttrib(Ihandle* ih, const char* value)
{
  if (iupStrBoolean(value))
    ih->data->showdrag = 1;
  else
    ih->data->showdrag = 0;

  return 0; /* do not store value in hash table */
}

static char* iSplitGetShowDragAttrib(Ihandle* ih)
{
  if (ih->data->showdrag)
    return "YES";
  else
    return "NO";
}


/*****************************************************************************\
|* Methods                                                                   *|
\*****************************************************************************/


static void iSplitComputeNaturalSizeMethod(Ihandle* ih, int *w, int *h, int *expand)
{
  int natural_w = 0, 
      natural_h = 0;
  Ihandle *child1, *child2 = NULL;
  child1 = ih->firstchild->brother;
  if (child1)
    child2 = child1->brother;

  /* always has at least one child, the bar, not necessary to compute its natural size */
  if (ih->data->direction == ISPLIT_VERT)
    natural_w += ISPLIT_THICK;
  else
    natural_h += ISPLIT_THICK;

  if (child1)
  {
    /* update child natural size first */
    iupBaseComputeNaturalSize(child1);

    if (ih->data->direction == ISPLIT_VERT)
    {
      natural_w += child1->naturalwidth;
      natural_h = iupMAX(natural_h, child1->naturalheight);
    }
    else
    {
      natural_w = iupMAX(natural_w, child1->naturalwidth);
      natural_h += child1->naturalheight;
    }

    *expand = child1->expand;

    if (child2)
    {
      /* update child natural size first */
      iupBaseComputeNaturalSize(child2);

      if (ih->data->direction == ISPLIT_VERT)
      {
        natural_w += child2->naturalwidth;
        natural_h = iupMAX(natural_h, child2->naturalheight);
      }
      else
      {
        natural_w = iupMAX(natural_w, child2->naturalwidth);
        natural_h += child2->naturalheight;
      }

      *expand |= child2->expand;
    }
  }

  if (ih->data->val == -1)
  {
    if (child1)
    {
      if (ih->data->direction == ISPLIT_VERT)
        ih->data->val = (child1->naturalwidth*1000)/(natural_w-ISPLIT_THICK);
      else
        ih->data->val = (child1->naturalheight*1000)/(natural_h-ISPLIT_THICK);

      if (ih->data->val < 0) ih->data->val = 0;
      if (ih->data->val > 1000) ih->data->val = 1000;
    }
    else
      ih->data->val = 0;
  }

  *w = natural_w;
  *h = natural_h;
}

static void iSplitSetChildrenCurrentSizeMethod(Ihandle* ih, int shrink)
{
  Ihandle *child1, *child2 = NULL;
  child1 = ih->firstchild->brother;
  if (child1)
    child2 = child1->brother;

  if (ih->data->direction == ISPLIT_VERT)
  {
    int width2 = 0;

    if (child1)
    {
      int width1 = iSplitGetWidth1(ih);
      iupBaseSetCurrentSize(child1, width1, ih->currentheight, shrink);

      width2 = (ih->currentwidth-ISPLIT_THICK)-width1;
      if (width2 < 0) width2 = 0;
    }

    /* bar */
    ih->firstchild->currentwidth  = ISPLIT_THICK;
    ih->firstchild->currentheight = ih->currentheight;

    if (child2)
      iupBaseSetCurrentSize(child2, width2, ih->currentheight, shrink);
  }
  else /* ISPLIT_HORIZ */
  {
    int height2 = 0;

    if (child1)
    {
      int height1 = iSplitGetHeight1(ih);
      iupBaseSetCurrentSize(child1, ih->currentwidth, height1, shrink);

      height2 = (ih->currentheight-ISPLIT_THICK)-height1;
      if (height2 < 0) height2 = 0;
    }

    /* bar */
    ih->firstchild->currentwidth  = ih->currentwidth;
    ih->firstchild->currentheight = ISPLIT_THICK;

    if (child2)
      iupBaseSetCurrentSize(child2, ih->currentwidth, height2, shrink);
  }
}

static void iSplitSetChildrenPositionMethod(Ihandle* ih, int x, int y)
{
  Ihandle *child1, *child2 = NULL;
  child1 = ih->firstchild->brother;
  if (child1)
    child2 = child1->brother;

  if (ih->data->direction == ISPLIT_VERT)
  {
    if (child1)
    {
      iupBaseSetPosition(child1, x, y);
      x += iSplitGetWidth1(ih);
    }

    /* bar */
    iupBaseSetPosition(ih->firstchild, x, y);
    x += ISPLIT_THICK;

    if (child2)
      iupBaseSetPosition(child2, x, y);
  }
  else /* ISPLIT_HORIZ */
  {
    if (child1)
    {
      iupBaseSetPosition(child1, x, y);
      y += iSplitGetHeight1(ih);
    }

    /* bar */
    iupBaseSetPosition(ih->firstchild, x, y);
    y += ISPLIT_THICK;

    if (child2)
      iupBaseSetPosition(child2, x, y);
  }
}

static int iSplitCreateMethod(Ihandle* ih, void** params)
{
  Ihandle* bar;

  ih->data = iupALLOCCTRLDATA();

  ih->data->direction = ISPLIT_VERT;
  ih->data->val = -1;
  ih->data->showdrag = 1;

  bar = IupCanvas(NULL);
  iupChildTreeAppend(ih, bar);  /* bar will always be the firstchild */

  IupSetAttribute(bar, "BORDER", "YES");
  IupSetAttribute(bar, "EXPAND", "NO");
  IupSetAttribute(bar, "BGCOLOR", "192 192 192");

  /* Setting callbacks */
  IupSetCallback(bar, "BUTTON_CB", (Icallback) iSplitButton_CB);
  IupSetCallback(bar, "FOCUS_CB",  (Icallback) iSplitFocus_CB);
  IupSetCallback(bar, "MOTION_CB", (Icallback) iSplitMotion_CB);

  if (params)
  {
    Ihandle** iparams = (Ihandle**)params;
    if (iparams[0]) IupAppend(ih, iparams[0]);
    if (iparams[1]) IupAppend(ih, iparams[1]);
  }

  return IUP_NOERROR;
}

Iclass* iupSplitGetClass(void)
{
  Iclass* ic = iupClassNew(NULL);

  ic->name   = "split";
  ic->format = "HH";   /* two optional ihandle */
  ic->nativetype = IUP_TYPEVOID;
  ic->childtype  = IUP_CHILDMANY;
  ic->is_interactive = 0;

  /* Class functions */
  ic->Create  = iSplitCreateMethod;
  ic->Map     = iupBaseTypeVoidMapMethod;

  ic->ComputeNaturalSize = iSplitComputeNaturalSizeMethod;
  ic->SetChildrenCurrentSize = iSplitSetChildrenCurrentSizeMethod;
  ic->SetChildrenPosition    = iSplitSetChildrenPositionMethod;

  /* Common */
  iupBaseRegisterCommonAttrib(ic);

  /* Base Container */
  iupClassRegisterAttribute(ic, "CLIENTSIZE1", iSplitGetClientSize1Attrib, NULL, NULL, NULL, IUPAF_READONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "CLIENTSIZE2", iSplitGetClientSize2Attrib, NULL, NULL, NULL, IUPAF_READONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "EXPAND", iupBaseContainerGetExpandAttrib, NULL, IUPAF_SAMEASSYSTEM, "YES", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);

  /* IupSplit only */
  iupClassRegisterAttribute(ic, "COLOR",     NULL, iSplitSetColorAttrib,     IUPAF_SAMEASSYSTEM, "192 192 192", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "DIRECTION", NULL, iSplitSetDirectionAttrib, IUPAF_SAMEASSYSTEM, "VERTICAL", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "VALUE", iSplitGetValueAttrib, iSplitSetValueAttrib, IUPAF_SAMEASSYSTEM, "500", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "SHOWDRAG", iSplitGetShowDragAttrib, iSplitSetShowDragAttrib, IUPAF_SAMEASSYSTEM, "YES", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);

  return ic;
}

Ihandle* IupSplit(Ihandle* child1, Ihandle* child2)
{
  void *params[3];
  params[0] = (void*)child1;
  params[1] = (void*)child2;
  params[2] = NULL;
  return IupCreatev("split", params);
}
