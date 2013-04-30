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


#define IEXPAND_HANDLE_SIZE 10

enum { IEXPANDER_LEFT, IEXPANDER_RIGHT, IEXPANDER_TOP, IEXPANDER_BOTTOM };
enum { IEXPANDER_CLOSE, IEXPANDER_OPEN };

struct _IcontrolData
{
  /* attributes */
  int position;
  int state;
  int barSize;
};


static void iExpanderOpenCloseChild(Ihandle* ih, int flag)
{
  Ihandle *child = ih->firstchild->brother;

  if (ih->firstchild->handle);
    iupdrvPostRedraw(ih->firstchild);

  if (!child)
    return;

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

  IupRefresh(child); /* this will recompute the layout of the hole dialog */
}

static int iExpanderGetBarSize(Ihandle* ih)
{
  int bar_size;
  if (ih->data->barSize == -1)
  {
    iupdrvFontGetCharSize(ih, NULL, &bar_size); 
    bar_size += 5;

    if (bar_size < IEXPAND_HANDLE_SIZE)
      bar_size = IEXPAND_HANDLE_SIZE;
  }
  else
    bar_size = ih->data->barSize;

  return bar_size;
}

/*****************************************************************************\
|* Callbacks of canvas bar                                                   *|
\*****************************************************************************/

static int iExpanderAction_CB(Ihandle* bar)
{
  Ihandle *ih = bar->parent;
  IdrawCanvas *dc = iupDrawCreateCanvas(bar);
  unsigned char r = 160, g = 160, b = 160, bg_r, bg_g, bg_b;
  char str[50];
  char* title = iupAttribGetStr(ih, "TITLE");
  int len;
  
  iupDrawParentBackground(dc);

  if(ih->data->state == IEXPANDER_CLOSE)
    sprintf(str, "[+] %s", title);
  else
    sprintf(str, "[-] %s", title);

  iupStrToRGB(IupGetAttribute(ih, "FGCOLOR"), &r, &g, &b);
  if (r+g+b > 3*190)
    { bg_r = 100; bg_g = 100; bg_b = 100; }
  else
    { bg_r = 255; bg_g = 255; bg_b = 255; }

  iupStrNextLine(str, &len);  /* get the length of the first line */
  iupDrawText(dc, str, len, 0, 0, r, g, b, IupGetAttribute(ih, "FONT"));

  iupDrawFlush(dc);

  iupDrawKillCanvas(dc);

  return IUP_DEFAULT;
}

static int iExpanderButton_CB(Ihandle* bar, int button, int pressed, int x, int y, char* status)
{
  Ihandle* ih = bar->parent;

  if (button==IUP_BUTTON1 && pressed)
  {
    /* Update the state: OPEN ==> collapsed, CLOSE ==> expanded */
     ih->data->state = (ih->data->state == IEXPANDER_OPEN? IEXPANDER_CLOSE: IEXPANDER_OPEN);

     iExpanderOpenCloseChild(ih, ih->data->state);
  }

  (void)x;
  (void)y;
  (void)status;
  return IUP_DEFAULT;
}


/*****************************************************************************\
|* Attributes                                                                *|
\*****************************************************************************/


static char* iExpanderGetClientSizeAttrib(Ihandle* ih)
{
  char* str = iupStrGetMemory(20);
  int width = ih->currentwidth;
  int height = ih->currentheight;
  int bar_size = iExpanderGetBarSize(ih);

  if (ih->data->position == IEXPANDER_LEFT || ih->data->position == IEXPANDER_RIGHT)
    width -= bar_size;
  else
    height -= bar_size;

  if (width < 0) width = 0;
  if (height < 0) height = 0;
  sprintf(str, "%dx%d", width, height);
  return str;
}

static char* iExpanderGetClientOffsetAttrib(Ihandle* ih)
{
  int dx = 0, dy = 0;
  char* str = iupStrGetMemory(20);
  int bar_size = iExpanderGetBarSize(ih);

  if (ih->data->position == IEXPANDER_LEFT)
    dx += bar_size;
  else if (ih->data->position == IEXPANDER_TOP)
    dy += bar_size;

  sprintf(str, "%dx%d", dx, dy);
  return str;
}

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
  if (!value)
    ih->data->barSize = -1;
  else
    iupStrToInt(value, &ih->data->barSize);  /* must manually update layout */
  return 0; /* do not store value in hash table */
}

static char* iExpanderGetBarSizeAttrib(Ihandle* ih)
{
  char* str = iupStrGetMemory(30);
  int bar_size = iExpanderGetBarSize(ih);
  sprintf(str, "%d", bar_size);
  return str;
}

static int iExpanderSetUpdateAttrib(Ihandle* ih, const char* value)
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

  iExpanderOpenCloseChild(ih, ih->data->state);

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
  int title_size = 0,
      child_expand = 0,
      natural_w, natural_h;
  Ihandle *child = ih->firstchild->brother;
  int bar_size = iExpanderGetBarSize(ih);
  char* title = iupAttribGetStr(ih, "TITLE");
  iupdrvFontGetMultiLineStringSize(ih, title, &title_size, NULL);

  /* bar */
  if (ih->data->position == IEXPANDER_LEFT || ih->data->position == IEXPANDER_RIGHT)
  {
    natural_w = bar_size;
    natural_h = title_size + IEXPAND_HANDLE_SIZE;
  }
  else
  {
    natural_w = title_size + IEXPAND_HANDLE_SIZE;
    natural_h = bar_size;
  }

  if (child)
  {
    /* update child natural bar_size first */
    if (!(child->flags & IUP_FLOATING_IGNORE))
      iupBaseComputeNaturalSize(child);

    if (!(child->flags & IUP_FLOATING))
    {
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

      child_expand = child->expand;
    }
  }

  *expand = child_expand;
  *w = natural_w;
  *h = natural_h;
}

static void iExpanderSetChildrenCurrentSizeMethod(Ihandle* ih, int shrink)
{
  Ihandle *child = ih->firstchild->brother;
  int width = ih->currentwidth;
  int height = ih->currentheight;
  int bar_size = iExpanderGetBarSize(ih);

  if (ih->data->position == IEXPANDER_LEFT || ih->data->position == IEXPANDER_RIGHT)
  {
    /* bar */
    ih->firstchild->currentwidth  = bar_size;
    ih->firstchild->currentheight = ih->currentheight;

    if (ih->currentwidth < bar_size)
      ih->currentwidth = bar_size;

    width = ih->currentwidth - bar_size;
  }
  else /* IEXPANDER_TOP OR IEXPANDER_BOTTOM */
  {
    /* bar */
    ih->firstchild->currentwidth  = ih->currentwidth;
    ih->firstchild->currentheight = bar_size;

    if (ih->currentheight < bar_size)
      ih->currentheight = bar_size;

    height = ih->currentheight - bar_size;
  }

  if (child)
  {
    if (!(child->flags & IUP_FLOATING) ||
        !(child->flags & IUP_FLOATING_IGNORE))
      iupBaseSetCurrentSize(child, width, height, shrink);
  }
}

static void iExpanderSetChildrenPositionMethod(Ihandle* ih, int x, int y)
{
  Ihandle *child = ih->firstchild->brother;
  int bar_size = iExpanderGetBarSize(ih);

  /* always position bar */
  if (ih->data->position == IEXPANDER_LEFT)
  {
    iupBaseSetPosition(ih->firstchild, x, y);
    x += bar_size;
  }
  else if (ih->data->position == IEXPANDER_RIGHT)
    iupBaseSetPosition(ih->firstchild, x + ih->currentwidth - bar_size, y);
  else if (ih->data->position == IEXPANDER_BOTTOM)
    iupBaseSetPosition(ih->firstchild, x, y + ih->currentheight - bar_size);
  else /* IEXPANDER_TOP */
  {
    iupBaseSetPosition(ih->firstchild, x, y);
    y += bar_size;
  }

  if (child)
  {
    if (!(child->flags & IUP_FLOATING))
      iupBaseSetPosition(child, x, y);
  }
}

static void iExpanderChildAddedMethod(Ihandle* ih, Ihandle* child)
{
  iExpanderOpenCloseChild(ih, ih->data->state);
  (void)child;
}

static int iExpanderCreateMethod(Ihandle* ih, void** params)
{
  Ihandle* bar;

  ih->data = iupALLOCCTRLDATA();

  ih->data->position = IEXPANDER_TOP;
  ih->data->state = IEXPANDER_OPEN;
  ih->data->barSize = -1;

  bar = IupCanvas(NULL);
  iupChildTreeAppend(ih, bar);  /* bar will always be the firstchild */
  bar->flags |= IUP_INTERNAL;

  IupSetAttribute(bar, "CANFOCUS", "NO");
  IupSetAttribute(bar, "BORDER", "NO");
  IupSetAttribute(bar, "EXPAND", "NO");

  /* Setting callbacks */
  IupSetCallback(bar, "BUTTON_CB", (Icallback) iExpanderButton_CB);
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
  ic->ChildAdded = iExpanderChildAddedMethod;

  ic->ComputeNaturalSize     = iExpanderComputeNaturalSizeMethod;
  ic->SetChildrenCurrentSize = iExpanderSetChildrenCurrentSizeMethod;
  ic->SetChildrenPosition    = iExpanderSetChildrenPositionMethod;

  /* Common */
  iupBaseRegisterCommonAttrib(ic);

  /* Base Container */
  iupClassRegisterAttribute(ic, "EXPAND", iupBaseContainerGetExpandAttrib, NULL, IUPAF_SAMEASSYSTEM, "YES", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "CLIENTSIZE", iExpanderGetClientSizeAttrib, NULL, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_READONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "CLIENTOFFSET", iExpanderGetClientOffsetAttrib, NULL, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_READONLY|IUPAF_NO_INHERIT);

  /* IupExpander only */
  iupClassRegisterAttribute(ic, "BARPOSITION", NULL, iExpanderSetPositionAttrib, IUPAF_SAMEASSYSTEM, "TOP", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "BARSIZE", iExpanderGetBarSizeAttrib, iExpanderSetBarSizeAttrib, IUPAF_SAMEASSYSTEM, NULL, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "STATE", iExpanderGetStateAttrib, iExpanderSetStateAttrib, IUPAF_SAMEASSYSTEM, "OPEN", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "FGCOLOR", NULL, iExpanderSetUpdateAttrib, IUPAF_SAMEASSYSTEM, "DLGFGCOLOR", IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "TITLE", NULL, iExpanderSetUpdateAttrib, NULL, NULL, IUPAF_NO_INHERIT);

  return ic;
}

Ihandle* IupExpander(Ihandle* child)
{
  void *children[2];
  children[0] = (void*)child;
  children[1] = NULL;
  return IupCreatev("expander", children);
}

/* TODO:
- desenhar +- ><
- texto na vertical?
- alinhamento?
- atributo para desenhar um triangulo apontando para a esquerda e para baixo, em vez de + e –

- expand automatico com mousemove e timer
- feedback de mouseover
*/
