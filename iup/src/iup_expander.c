/** \file
 * \brief iupexpander control
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
#include "iup_drvfont.h"
#include "iup_stdcontrols.h"
#include "iup_layout.h"
#include "iup_childtree.h"
#include "iup_draw.h"


enum { IEXPANDER_LEFT, IEXPANDER_RIGHT, IEXPANDER_TOP, IEXPANDER_BOTTOM };
enum { IEXPANDER_CLOSE, IEXPANDER_OPEN };

struct _IcontrolData
{
  /* attributes */
  int position;
  int state;
  int barHeight, barWidth;
};


static void iExpanderOpenCloseChild(Ihandle* child, int flag)
{
  if (flag == IEXPANDER_CLOSE)
  {
    if (IupGetInt(child, "VISIBLE"))
    {
      IupSetAttribute(child, "FLOATING", "IGNORE");
      IupSetAttribute(child, "VISIBLE", "NO");
    }
  }
  else 
  {
    if (!IupGetInt(child, "VISIBLE"))
    {
      IupSetAttribute(child, "FLOATING", "NO");
      IupSetAttribute(child, "VISIBLE", "YES");
    }
  }
}

static int iExpanderGetWidth(Ihandle* ih)
{
  /* This is the space available for the child,
     It does NOT depends on the child. */
  int width = ih->currentwidth - ih->data->barWidth;
  if (width < 0) width = 0;
  return width;
}

static int iExpanderGetHeight(Ihandle* ih)
{
  /* This is the space available for the child,
     It does NOT depends on the child. */
  int height = ih->currentheight - ih->data->barHeight;
  if (height < 0) height = 0;
  return height;
}

static char* iExpanderGetTitle(Ihandle *ih)
{
  char* value = iupAttribGetStr(ih, "TITLE");
  if(!value) value = "";
  value = iupStrProcessMnemonic(value, NULL, 0);
  return value;
}

static int iExpanderGetTitleWidth(Ihandle *ih)
{
  char* str = iupStrGetMemory(50);  
  sprintf(str, "[+] %s", iExpanderGetTitle(ih));
  return iupdrvFontGetStringWidth(ih, str);
}

/*****************************************************************************\
|* Callbacks of canvas bar                                                   *|
\*****************************************************************************/

static int iExpanderAction_CB(Ihandle* bar)
{
  Ihandle *ih = bar->parent;
  IdrawCanvas *dc = iupDrawCreateCanvas(bar);
  unsigned char r = 160, g = 160, b = 160, bg_r, bg_g, bg_b;
  char* str = iupStrGetMemory(50);
  int len;
  
  iupDrawParentBackground(dc);

  if(ih->data->state == IEXPANDER_CLOSE)
    sprintf(str, "[+] %s", iExpanderGetTitle(ih));
  else
    sprintf(str, "[-] %s", iExpanderGetTitle(ih));

  iupStrToRGB(IupGetAttribute(ih, "COLOR"), &r, &g, &b);
  if (r+g+b > 3*190)
    { bg_r = 100; bg_g = 100; bg_b = 100; }
  else
    { bg_r = 255; bg_g = 255; bg_b = 255; }

  iupStrNextLine(str, &len);
  iupDrawText(dc, str, len, 0, 0, r, g, b, IupGetAttribute(ih, "FONT"));

  iupDrawFlush(dc);

  iupDrawKillCanvas(dc);

  return IUP_DEFAULT;
}

static int iExpanderMotion_CB(Ihandle* bar, int x, int y, char *status)
{
  (void)bar;
  (void)x;
  (void)y;
  (void)status;
  return IUP_DEFAULT;
}

static int iExpanderButton_CB(Ihandle* bar, int button, int pressed, int x, int y, char* status)
{
  Ihandle* ih = bar->parent;

  if (button!=IUP_BUTTON1)
    return IUP_DEFAULT;

  if(pressed)
  {
    /* Update the state: OPEN ==> collapsed, CLOSE ==> expanded */
     ih->data->state = (ih->data->state == IEXPANDER_OPEN ? IEXPANDER_CLOSE : IEXPANDER_OPEN);
     iExpanderOpenCloseChild(ih->firstchild->brother, ih->data->state);
  }
  else
  {
    IupRefreshChildren(ih);  /* Always refresh when releasing the mouse */
    iupdrvPostRedraw(bar);
  }

  (void)x;
  (void)y;
  (void)status;
  return IUP_DEFAULT;
}

static int iExpanderFocus_CB(Ihandle* bar, int focus)
{
  Ihandle* ih = bar->parent;

  if (!ih || focus) /* use only kill focus */
    return IUP_DEFAULT;

  return IUP_DEFAULT;
}


/*****************************************************************************\
|* Attributes                                                                *|
\*****************************************************************************/

static int iExpanderSetPositionAttrib(Ihandle* ih, const char* value)
{
  if (ih->handle) /* only before map */
    return 0;

  if (iupStrEqualNoCase(value, "LEFT"))
    ih->data->position = IEXPANDER_LEFT;
  else if (iupStrEqualNoCase(value, "RIGHT"))
    ih->data->position = IEXPANDER_RIGHT;
  else if (iupStrEqualNoCase(value, "BOTTOM"))
    ih->data->position = IEXPANDER_BOTTOM;
  else  /* Default = TOP */
    ih->data->position = IEXPANDER_TOP;

  return 0;  /* do not store value in hash table */
}

static int iExpanderSetBarSizeAttrib(Ihandle* ih, const char* value)
{
  if(iupStrToIntInt(value, &ih->data->barWidth, &ih->data->barHeight, 'x') && ih->handle)
    IupRefreshChildren(ih);

  return 0; /* do not store value in hash table */
}

static char* iExpanderGetBarSizeAttrib(Ihandle* ih)
{
  char* str = iupStrGetMemory(30);
  sprintf(str, "%dx%d", ih->data->barWidth, ih->data->barHeight);
  return str;
}

static int iExpanderSetColorAttrib(Ihandle* ih, const char* value)
{
  (void)value;
  iupdrvPostRedraw(ih);
  return 1;  /* store value in hash table */
}

static int iExpanderSetStateAttrib(Ihandle* ih, const char* value)
{
  if (iupStrEqualNoCase(value, "OPEN"))
    ih->data->state = IEXPANDER_OPEN;
  else
    ih->data->state = IEXPANDER_CLOSE;

  return 0; /* do not store value in hash table */
}

static char* iExpanderGetStateAttrib(Ihandle* ih)
{
  if (ih->data->state)
    return "OPEN";
  else
    return "CLOSE";
}

/*****************************************************************************\
|* Methods                                                                   *|
\*****************************************************************************/


static void iExpanderComputeNaturalSizeMethod(Ihandle* ih, int *w, int *h, int *expand)
{
  int natural_w = 0, 
      natural_h = 0;
  Ihandle *child = ih->firstchild->brother;

  /* update bar size */
  iupdrvFontGetCharSize(ih, NULL, &ih->data->barHeight);
  ih->data->barWidth = iExpanderGetTitleWidth(ih);

  /* bar */
  natural_w += ih->data->barWidth;
  natural_h += ih->data->barHeight;

  if (child)
  {
    /* update child natural size first */
    iupBaseComputeNaturalSize(child);

    if (ih->data->position == IEXPANDER_LEFT || ih->data->position == IEXPANDER_RIGHT)
    {
      natural_w += child->naturalwidth;
      natural_h = iupMAX(natural_h, child->naturalheight);
    }
    else
    {
      natural_w = iupMAX(natural_w, child->naturalwidth);
      natural_h += child->naturalheight;
    }

    *expand = child->expand;
  }

  *w = natural_w;
  *h = natural_h;
}

static void iExpanderSetChildrenCurrentSizeMethod(Ihandle* ih, int shrink)
{
  Ihandle *child = ih->firstchild->brother;

  if (ih->data->position == IEXPANDER_LEFT || ih->data->position == IEXPANDER_RIGHT)
  {
    int width = iExpanderGetWidth(ih);

    if (child)
    {
      iupBaseSetCurrentSize(child, width, ih->currentheight, shrink);

      if (child->currentwidth > width)
      {
        /* has a minimum size, must fix value */
        width = child->currentwidth;
      }
    }

    /* bar */
    ih->firstchild->currentwidth  = ih->data->barWidth;
    ih->firstchild->currentheight = ih->currentheight;
  }
  else /* IEXPANDER_TOP OR IEXPANDER_BOTTOM */
  {
    int height = iExpanderGetHeight(ih);

    if (child)
    {
      iupBaseSetCurrentSize(child, ih->currentwidth, height, shrink);

      if (child->currentheight > height)
      {
        /* has a minimum size, must fix value */
        height = child->currentheight;
      }
    }

    /* bar */
    ih->firstchild->currentwidth  = ih->currentwidth;
    ih->firstchild->currentheight = ih->data->barHeight;
  }
}

static void iExpanderSetChildrenPositionMethod(Ihandle* ih, int x, int y)
{
  Ihandle *child = ih->firstchild->brother;

  if (ih->data->position == IEXPANDER_LEFT)
  {
    /* bar */
    iupBaseSetPosition(ih->firstchild, x, y);
    x += ih->data->barWidth;

    if (child)
      iupBaseSetPosition(child, x, y);
  }
  else if (ih->data->position == IEXPANDER_RIGHT)
  {
    if (child)
      iupBaseSetPosition(child, x, y);

    /* bar */
    x +=  iExpanderGetWidth(ih);
    iupBaseSetPosition(ih->firstchild, x, y);
  }
  else if (ih->data->position == IEXPANDER_BOTTOM)
  {
    if (child)
      iupBaseSetPosition(child, x, y);

    /* bar */
    y += iExpanderGetHeight(ih);
    iupBaseSetPosition(ih->firstchild, x, y);
  }
  else /* IEXPANDER_TOP */
  {
    /* bar */
    iupBaseSetPosition(ih->firstchild, x, y);
    y += ih->data->barHeight;

    if (child)
      iupBaseSetPosition(child, x, y);
  }
}

static int iExpanderCreateMethod(Ihandle* ih, void** params)
{
  Ihandle* bar;

  ih->data = iupALLOCCTRLDATA();

  ih->data->position = IEXPANDER_TOP;
  ih->data->state = IEXPANDER_OPEN;
  ih->data->barHeight = 5;
  ih->data->barWidth  = 5;

  bar = IupCanvas(NULL);
  iupChildTreeAppend(ih, bar);  /* bar will always be the firstchild */
  bar->flags |= IUP_INTERNAL;

  IupSetAttribute(bar, "CANFOCUS", "NO");
  IupSetAttribute(bar, "BORDER", "NO");
  IupSetAttribute(bar, "EXPAND", "NO");

  /* Setting callbacks */
  IupSetCallback(bar, "BUTTON_CB", (Icallback) iExpanderButton_CB);
  IupSetCallback(bar, "FOCUS_CB",  (Icallback) iExpanderFocus_CB);
  IupSetCallback(bar, "MOTION_CB", (Icallback) iExpanderMotion_CB);
  IupSetCallback(bar, "ACTION",    (Icallback) iExpanderAction_CB);

  if (params)
  {
    Ihandle** iparams = (Ihandle**)params;
    if (*iparams)
      IupAppend(ih, *iparams);
  }

  return IUP_NOERROR;
}

Iclass* iupExpanderNewClass(void)
{
  Iclass* ic = iupClassNew(NULL);

  ic->name   = "expander";
  ic->format = "h";   /* one ihandle */
  ic->nativetype = IUP_TYPEVOID;
  ic->childtype  = IUP_CHILDMANY+2;  /* canvas+child */
  ic->is_interactive = 0;

  /* Class functions */
  ic->New     = iupExpanderNewClass;
  ic->Create  = iExpanderCreateMethod;
  ic->Map     = iupBaseTypeVoidMapMethod;

  ic->ComputeNaturalSize     = iExpanderComputeNaturalSizeMethod;
  ic->SetChildrenCurrentSize = iExpanderSetChildrenCurrentSizeMethod;
  ic->SetChildrenPosition    = iExpanderSetChildrenPositionMethod;

  /* Common */
  iupBaseRegisterCommonAttrib(ic);

  /* Base Container */
  iupClassRegisterAttribute(ic, "EXPAND", iupBaseContainerGetExpandAttrib, NULL, IUPAF_SAMEASSYSTEM, "YES", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "CLIENTSIZE", iupBaseGetRasterSizeAttrib, NULL, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_READONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "CLIENTOFFSET", iupBaseGetClientOffsetAttrib, NULL, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_READONLY|IUPAF_NO_INHERIT);

  /* IupExpander only */
  iupClassRegisterAttribute(ic, "POSITION", NULL, iExpanderSetPositionAttrib, IUPAF_SAMEASSYSTEM, "TOP", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "BARSIZE", iExpanderGetBarSizeAttrib, iExpanderSetBarSizeAttrib, IUPAF_SAMEASSYSTEM, "5x5", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "STATE", iExpanderGetStateAttrib, iExpanderSetStateAttrib, IUPAF_SAMEASSYSTEM, "OPEN", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "COLOR", NULL, iExpanderSetColorAttrib, IUPAF_SAMEASSYSTEM, "160 160 160", IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "TITLE", NULL, NULL, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);

  return ic;
}

Ihandle* IupExpander(Ihandle* child)
{
  void *children[2];
  children[0] = (void*)child;
  children[1] = NULL;
  return IupCreatev("expander", children);
}
