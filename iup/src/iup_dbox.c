/** \file
 * \brief IupDbox control
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
#include "iup_draw.h"


enum { IDBOX_VERT, IDBOX_HORIZ };
enum { IDBOX_DETACH_START, IDBOX_DETACH_END, IDBOX_DETACH_CANCEL };

struct _IcontrolData
{
  /* aux */
  int is_holding, start_pos, start_size;
  Ihandle *old_parent, *old_brother;

  /* attributes */
  int layoutdrag, barsize, showgrip;
  int orientation;     /* one of the types: IDBOX_VERT, IDBOX_HORIZ */
};


/*****************************************************************************\
|* Attributes                                                                *|
\*****************************************************************************/

static int iDboxSetColorAttrib(Ihandle* ih, const char* value)
{
  (void)value;
  IupUpdate(ih);
  return 1;  /* store value in hash table */
}

static int iDboxSetOrientationAttrib(Ihandle* ih, const char* value)
{
  if (ih->handle) /* only before map */
    return 0;

  if (iupStrEqualNoCase(value, "HORIZONTAL"))
    ih->data->orientation = IDBOX_HORIZ;
  else  /* Default = VERTICAL */
    ih->data->orientation = IDBOX_VERT;

  return 0;  /* do not store value in hash table */
}

static int iDboxSetBarSizeAttrib(Ihandle* ih, const char* value)
{
  if (iupStrToInt(value, &ih->data->barsize) && ih->handle)
    IupRefreshChildren(ih);  

  return 0; /* do not store value in hash table */
}

static char* iDboxGetBarSizeAttrib(Ihandle* ih)
{
  return iupStrReturnInt(ih->data->barsize);
}

static int iDboxSetShowGripAttrib(Ihandle* ih, const char* value)
{
  if (iupStrBoolean(value))
    ih->data->showgrip = 1;
  else
  {
    ih->data->showgrip = 0;

    if (ih->data->barsize == 5)
      iDboxSetBarSizeAttrib(ih, "3");
  }

  return 0; /* do not store value in hash table */
}

static char* iDboxGetShowGripAttrib(Ihandle* ih)
{
  return iupStrReturnBoolean (ih->data->showgrip); 
}

static char* iDboxGetOldParentHandleAttrib(Ihandle* ih)
{
  return (char*)ih->data->old_parent;
}

static char* iDboxGetOldBrotherHandleAttrib(Ihandle* ih)
{
  return (char*)ih->data->old_brother;
}

static char* iDboxGetNewDialogHandleAttrib(Ihandle* ih)
{
  return iupAttribGet(ih, "_IUPDBOX_DIALOG");
}

/*****************************************************************************\
|* Callbacks                                                                 *|
\*****************************************************************************/

static int iDboxClose_CB(Ihandle* dlg)
{
  iupAttribSet(dlg->firstchild, "_IUPDBOX_DIALOG", NULL);
  return IUP_DEFAULT;
}

static int iDboxKeypress_CB(Ihandle* ih, int key, int pressed)
{
  if(ih->data->is_holding && key == K_ESC)  /* DRAG CANCEL */
  {
    IFni detachCB = (IFni)IupGetCallback(ih, "DETACH_CB");

    ih->data->is_holding = 0;

    if(detachCB)
      detachCB(ih, IDBOX_DETACH_CANCEL);
  }

  (void)pressed;
  return IUP_DEFAULT;
}

static int iDboxAction_CB(Ihandle* bar)
{
  Ihandle* ih = bar->parent;
  IdrawCanvas* dc = iupDrawCreateCanvas(bar);

  iupDrawParentBackground(dc);

  if (ih->data->showgrip)
  {
    int i, w, h, x, y, count;
    unsigned char r = 160, g = 160, b = 160, bg_r, bg_g, bg_b;
    iupDrawGetSize(dc, &w, &h);

    iupStrToRGB(IupGetAttribute(ih, "COLOR"), &r, &g, &b);
    if (r+g+b > 3*190)
      { bg_r = 100; bg_g = 100; bg_b = 100; }
    else
      { bg_r = 255; bg_g = 255; bg_b = 255; }

    if (ih->data->orientation == IDBOX_VERT)
    {
      x = ih->data->barsize/2-1;
      y = 2;
      count = (h-2)/5;
    }
    else
    {
      x = 2;
      y = ih->data->barsize/2-1;
      count = (w-2)/5;
    }

    for (i = 0; i < count; i++)
    {
      iupDrawRectangle(dc, x+1, y+1, x+2, y+2, bg_r, bg_g, bg_b, IUP_DRAW_FILL);
      iupDrawRectangle(dc, x, y, x+1, y+1, r, g, b, IUP_DRAW_FILL);
      if (ih->data->orientation == IDBOX_VERT)
        y += 5;
      else
        x += 5;
    }
  }
  
  iupDrawFlush(dc);

  iupDrawKillCanvas(dc);

  return IUP_DEFAULT;
}

static int iDboxButton_CB(Ihandle* bar, int button, int pressed, int x, int y, char* status)
{
  Ihandle* ih = bar->parent;
  Ihandle* mainDlg = IupGetDialog(ih);

  if (button != IUP_BUTTON1)
    return IUP_DEFAULT;

  if (!ih->data->is_holding && pressed)  /* DRAG BEGIN */
  {
    IFni detachCB = (IFni)IupGetCallback(ih, "DETACH_CB");

    ih->data->is_holding = 1;

    if(detachCB)
      detachCB(ih, IDBOX_DETACH_START);
  }
  else if (ih->data->is_holding && !pressed)
  {
    /* DRAG END */
    Ihandle* dlg = (Ihandle*)iupAttribGet(ih, "_IUPDBOX_DIALOG");
    IFni detachCB = (IFni)IupGetCallback(ih, "DETACH_CB");
    int cur_x, cur_y;

    ih->data->is_holding = 0;
    iupStrToIntInt(IupGetGlobal("CURSORPOS"), &cur_x, &cur_y, 'x');

    if(dlg)
      return 0;

    /* Create new dialog */
    dlg = IupDialog(NULL);
    IupSetCallback(dlg, "CLOSE_CB", iDboxClose_CB);
    IupSetAttribute(dlg, "SIZE", "100x100");
    IupSetAttribute(dlg, "TITLE", "IUP");

    /* Set new dialog as child of the current application */
    IupAppend(mainDlg, dlg);

    /* Show new dialog */
    IupShowXY(dlg, cur_x, cur_y);  /* Internally, the element is mapped */

    /* Save new dialog, current parent and current brother */
    iupAttribSet(ih, "_IUPDBOX_DIALOG", (char*)dlg);
    ih->data->old_parent  = IupGetParent(ih);
    ih->data->old_brother = IupGetBrother(ih);

    /* Hide canvas bar */
    iDboxSetBarSizeAttrib(ih, "0");
    IupHide(ih->firstchild);

    /* Set new parent */
    IupReparent(ih, dlg, NULL);
    IupSetAttribute(dlg, "RASTERSIZE", "NULL");

    /* Update the layout of dialogs */
    IupRefreshChildren(dlg);
    IupRefresh(dlg);
    IupRefreshChildren(mainDlg);
    IupRefresh(mainDlg);

    if(detachCB)
      detachCB(ih, IDBOX_DETACH_END);
  }

  (void)x;
  (void)y;
  (void)status;
  return IUP_DEFAULT;
}

static int iDboxFocus_CB(Ihandle* bar, int focus)
{
  Ihandle* ih = bar->parent;

  if (!ih || focus) /* use only kill focus */
    return IUP_DEFAULT;

  if (ih->data->is_holding)
    ih->data->is_holding = 0;

  return IUP_DEFAULT;
}


/*****************************************************************************\
|* Methods                                                                   *|
\*****************************************************************************/

static void iDboxComputeNaturalSizeMethod(Ihandle* ih, int *w, int *h, int *children_expand)
{
  int natural_w = 0, 
      natural_h = 0;

    /* bar */
  if (ih->data->orientation == IDBOX_VERT)
    natural_w += ih->data->barsize;
  else
    natural_h += ih->data->barsize;

  if (ih->firstchild->brother)
  {
    Ihandle* child = ih->firstchild->brother;

    /* update child natural size first */
    iupBaseComputeNaturalSize(child);

    if (ih->data->orientation == IDBOX_VERT)
    {
      natural_w += child->naturalwidth;
      natural_h = iupMAX(natural_h, child->naturalheight);
    }
    else  /* ISPLIT_HORIZ */
    {
      natural_w = iupMAX(natural_w, child->naturalwidth);
      natural_h += child->naturalheight;
    }

    *children_expand |= child->expand;
  }

  *w = natural_w;
  *h = natural_h;
}

static void iDboxSetChildrenCurrentSizeMethod(Ihandle* ih, int shrink)
{
  /* bar */
  if (ih->data->orientation == IDBOX_VERT)
  {
    ih->firstchild->currentwidth  = ih->data->barsize;
    ih->firstchild->currentheight = ih->currentheight;
  }
  else  /* ISPLIT_HORIZ */
  {
    ih->firstchild->currentwidth  = ih->currentwidth;
    ih->firstchild->currentheight = ih->data->barsize;
  }

  /* child */
  if (ih->firstchild->brother)
    iupBaseSetCurrentSize(ih->firstchild->brother, ih->currentwidth, ih->currentheight, shrink);
}

static void iDboxSetChildrenPositionMethod(Ihandle* ih, int x, int y)
{
  /* bar */
  iupBaseSetPosition(ih->firstchild, x, y);

  /* child */
  if (ih->data->orientation == IDBOX_VERT)
  {
    x += ih->data->barsize;
    iupBaseSetPosition(ih->firstchild->brother, x, y);
  }
  else  /* ISPLIT_HORIZ */
  {
    y += ih->data->barsize;
    iupBaseSetPosition(ih->firstchild->brother, x, y);
  }
}

static int iDboxCreateMethod(Ihandle* ih, void** params)
{
  Ihandle* bar;

  ih->data = iupALLOCCTRLDATA();

  ih->data->orientation = IDBOX_VERT;
  ih->data->barsize = 5;
  ih->data->showgrip = 1;

  bar = IupCanvas(NULL);
  bar->flags |= IUP_INTERNAL;
  iupChildTreeAppend(ih, bar);  /* bar will always be the firstchild */

  IupSetAttribute(bar, "CANFOCUS", "NO");
  IupSetAttribute(bar, "BORDER", "NO");
  IupSetAttribute(bar, "EXPAND", "NO");
  IupSetAttribute(bar, "CURSOR", "MOVE");
  IupSetAttribute(bar, "BGCOLOR", "192 192 192");

  /* Setting canvas bar callbacks */
  IupSetCallback(bar, "BUTTON_CB", (Icallback) iDboxButton_CB);
  IupSetCallback(bar, "FOCUS_CB",  (Icallback) iDboxFocus_CB);
  IupSetCallback(bar, "ACTION", (Icallback) iDboxAction_CB);

  /* Setting callbacks */
  IupSetCallback(ih, "K_ANY",  (Icallback) iDboxKeypress_CB);

  if (params)
  {
    Ihandle** iparams = (Ihandle**)params;
    if (*iparams)
      IupAppend(ih, *iparams);
  }

  return IUP_NOERROR;
}

Iclass* iupDboxNewClass(void)
{
  Iclass* ic = iupClassNew(NULL);

  ic->name   = "dbox";
  ic->format = "h";   /* one ihandle */
  ic->nativetype = IUP_TYPEVOID;
  ic->childtype  = IUP_CHILDMANY+2; /* canvas+child */
  ic->is_interactive = 0;

  /* Class functions */
  ic->New    = iupDboxNewClass;
  ic->Create = iDboxCreateMethod;
  ic->Map    = iupBaseTypeVoidMapMethod;

  ic->ComputeNaturalSize     = iDboxComputeNaturalSizeMethod;
  ic->SetChildrenCurrentSize = iDboxSetChildrenCurrentSizeMethod;
  ic->SetChildrenPosition    = iDboxSetChildrenPositionMethod;

  iupClassRegisterCallback(ic, "DETACH_CB", "i");

  /* Common */
  iupBaseRegisterCommonAttrib(ic);

  /* Base Container */
  iupClassRegisterAttribute(ic, "CLIENTSIZE", iupBaseGetRasterSizeAttrib, NULL, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_READONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "CLIENTOFFSET", iupBaseGetClientOffsetAttrib, NULL, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_READONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "EXPAND", iupBaseContainerGetExpandAttrib, NULL, IUPAF_SAMEASSYSTEM, "YES", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);

  /* IupDbox only */
  iupClassRegisterAttribute(ic, "COLOR", NULL, iDboxSetColorAttrib, IUPAF_SAMEASSYSTEM, "160 160 160", IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "ORIENTATION", NULL, iDboxSetOrientationAttrib, IUPAF_SAMEASSYSTEM, "VERTICAL", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "BARSIZE", iDboxGetBarSizeAttrib, iDboxSetBarSizeAttrib, IUPAF_SAMEASSYSTEM, "5", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "SHOWGRIP", iDboxGetShowGripAttrib, iDboxSetShowGripAttrib, IUPAF_SAMEASSYSTEM, "YES", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "OLDPARENT_HANDLE", iDboxGetOldParentHandleAttrib, NULL, NULL, NULL, IUPAF_READONLY|IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT|IUPAF_NO_STRING);
  iupClassRegisterAttribute(ic, "OLDBROTHER_HANDLE", iDboxGetOldBrotherHandleAttrib, NULL, NULL, NULL, IUPAF_READONLY|IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT|IUPAF_NO_STRING);
  iupClassRegisterAttribute(ic, "NEWDIALOG_HANDLE", iDboxGetNewDialogHandleAttrib, NULL, NULL, NULL, IUPAF_READONLY|IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT|IUPAF_NO_STRING);

  return ic;
}

Ihandle* IupDbox(Ihandle* child)
{
  void *children[2];
  children[0] = (void*)child;
  children[1] = NULL;
  return IupCreatev("dbox", children);
}
