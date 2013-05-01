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


#define IEXPAND_HANDLE_SIZE 16
#define IEXPAND_HANDLE_SPC   3

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

  if (ih->firstchild->handle)
    iupdrvPostRedraw(ih->firstchild);

  if (!child)
    return;

  if (flag == IEXPANDER_CLOSE)
  {
    if (IupGetInt(child, "VISIBLE"))
      IupSetAttribute(child, "VISIBLE", "NO");
  }
  else 
  {
    if (!IupGetInt(child, "VISIBLE"))
      IupSetAttribute(child, "VISIBLE", "YES");
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

static void iExpanderDrawTriangle(IdrawCanvas *dc, int x, int y, unsigned char r, unsigned char g, unsigned char b, int dir)
{
  int points[6];
  switch(dir)
  {
  case IEXPANDER_LEFT:  /* arrow points left */
    points[0] = x + IEXPAND_HANDLE_SIZE - IEXPAND_HANDLE_SPC;
    points[1] = y + IEXPAND_HANDLE_SPC;
    points[2] = x + IEXPAND_HANDLE_SIZE - IEXPAND_HANDLE_SPC;
    points[3] = y + IEXPAND_HANDLE_SIZE - IEXPAND_HANDLE_SPC;
    points[4] = x + IEXPAND_HANDLE_SPC;
    points[5] = y + IEXPAND_HANDLE_SIZE/2;
    break;
  case IEXPANDER_TOP:    /* arrow points top */
    points[0] = x + IEXPAND_HANDLE_SPC;
    points[1] = y + IEXPAND_HANDLE_SIZE - IEXPAND_HANDLE_SPC;
    points[2] = x + IEXPAND_HANDLE_SIZE - IEXPAND_HANDLE_SPC;
    points[3] = y + IEXPAND_HANDLE_SIZE - IEXPAND_HANDLE_SPC;
    points[4] = x + IEXPAND_HANDLE_SIZE/2;
    points[5] = y + IEXPAND_HANDLE_SPC;
    break;
  case IEXPANDER_RIGHT:  /* arrow points right */
    points[0] = x + IEXPAND_HANDLE_SPC;
    points[1] = y + IEXPAND_HANDLE_SPC;
    points[2] = x + IEXPAND_HANDLE_SPC;
    points[3] = y + IEXPAND_HANDLE_SIZE - IEXPAND_HANDLE_SPC;
    points[4] = x + IEXPAND_HANDLE_SIZE - IEXPAND_HANDLE_SPC;
    points[5] = y + IEXPAND_HANDLE_SIZE/2;
    break;
  case IEXPANDER_BOTTOM:  /* arrow points bottom */
    points[0] = x + IEXPAND_HANDLE_SPC;
    points[1] = y + IEXPAND_HANDLE_SPC;
    points[2] = x + IEXPAND_HANDLE_SIZE - IEXPAND_HANDLE_SPC;
    points[3] = y + IEXPAND_HANDLE_SPC;
    points[4] = x + IEXPAND_HANDLE_SIZE/2;
    points[5] = y + IEXPAND_HANDLE_SIZE - IEXPAND_HANDLE_SPC;
    break;
  }

  iupDrawPolygon(dc, points, 3, r, g, b, IUP_DRAW_FILL);
}

static int iExpanderAction_CB(Ihandle* bar)
{
  Ihandle *ih = bar->parent;
  IdrawCanvas *dc = iupDrawCreateCanvas(bar);
  unsigned char r=0, g=0, b=0;
  char* title = iupAttribGetStr(ih, "TITLE");
  
  iupDrawParentBackground(dc);

  iupStrToRGB(IupGetAttribute(ih, "FGCOLOR"), &r, &g, &b);

  if (ih->data->position == IEXPANDER_TOP && title)
  {
    /* left align everything */
    int len;

    if (ih->data->state == IEXPANDER_CLOSE)
      iExpanderDrawTriangle(dc, 0, 0, r, g, b, IEXPANDER_RIGHT);
    else
      iExpanderDrawTriangle(dc, 0, 0, r, g, b, IEXPANDER_BOTTOM);

    iupStrNextLine(title, &len);  /* get the length of the first line */
    iupDrawText(dc, title, len, IEXPAND_HANDLE_SIZE, 0, r, g, b, IupGetAttribute(ih, "FONT"));
  }
  else
  {
    /* center align the arrow */
    int x, y;
    switch(ih->data->position)
    {
    case IEXPANDER_LEFT:
      x = 0;
      y = (bar->currentheight - IEXPAND_HANDLE_SIZE)/2;
      if (ih->data->state == IEXPANDER_CLOSE)
        iExpanderDrawTriangle(dc, x, y, r, g, b, IEXPANDER_RIGHT);
      else
        iExpanderDrawTriangle(dc, x, y, r, g, b, IEXPANDER_LEFT);
      break;
    case IEXPANDER_TOP:
      x = (bar->currentwidth - IEXPAND_HANDLE_SIZE)/2;
      y = 0;
      if (ih->data->state == IEXPANDER_CLOSE)
        iExpanderDrawTriangle(dc, x, y, r, g, b, IEXPANDER_BOTTOM);
      else
        iExpanderDrawTriangle(dc, x, y, r, g, b, IEXPANDER_TOP);
      break;
    case IEXPANDER_RIGHT:
      x = 0;
      y = (bar->currentheight - IEXPAND_HANDLE_SIZE)/2;
      if (ih->data->state == IEXPANDER_CLOSE)
        iExpanderDrawTriangle(dc, x, y, r, g, b, IEXPANDER_LEFT);
      else
        iExpanderDrawTriangle(dc, x, y, r, g, b, IEXPANDER_RIGHT);
      break;
    case IEXPANDER_BOTTOM:
      x = (bar->currentwidth - IEXPAND_HANDLE_SIZE)/2;
      y = 0;
      if (ih->data->state == IEXPANDER_CLOSE)
        iExpanderDrawTriangle(dc, x, y, r, g, b, IEXPANDER_TOP);
      else
        iExpanderDrawTriangle(dc, x, y, r, g, b, IEXPANDER_BOTTOM);
      break;
    }
  }

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
  iupdrvPostRedraw(ih->firstchild);
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
  int child_expand = 0,
      natural_w, natural_h;
  Ihandle *child = ih->firstchild->brother;
  int bar_size = iExpanderGetBarSize(ih);

  /* bar */
  if (ih->data->position == IEXPANDER_LEFT || ih->data->position == IEXPANDER_RIGHT)
  {
    natural_w = bar_size;
    natural_h = IEXPAND_HANDLE_SIZE;
  }
  else
  {
    natural_w = IEXPAND_HANDLE_SIZE;
    natural_h = bar_size;

    if (ih->data->position == IEXPANDER_TOP)
    {
      char* title = iupAttribGetStr(ih, "TITLE");
      if (title)
      {
        int title_size = 0;
        iupdrvFontGetMultiLineStringSize(ih, title, &title_size, NULL);
        natural_w += title_size;
      }
    }
  }

  if (child)
  {
    /* update child natural bar_size first */
    iupBaseComputeNaturalSize(child);

    if (ih->data->position == IEXPANDER_LEFT || ih->data->position == IEXPANDER_RIGHT)
    {
      if (IupGetInt(child, "VISIBLE"))
        natural_w += child->naturalwidth;
      natural_h = iupMAX(natural_h, child->naturalheight);
    }
    else
    {
      natural_w = iupMAX(natural_w, child->naturalwidth);
      if (IupGetInt(child, "VISIBLE"))
        natural_h += child->naturalheight;
    }

    child_expand = child->expand;
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
//    if (IupGetInt(child, "VISIBLE"))
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
//    if (IupGetInt(child, "VISIBLE"))
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
- melhor aparencia com imagem?
- alinhar o texto verticalmente
- expand automatico com mousemove e timer
- feedback de mouseover
*/
