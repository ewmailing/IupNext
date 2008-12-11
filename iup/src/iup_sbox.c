/** \file
 * \brief iupsbox control
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


#define ISBOX_THICK 5

enum { ISBOX_NORTH, ISBOX_SOUTH, ISBOX_WEST, ISBOX_EAST };

struct _IcontrolData
{
  int w, h;
  int isholding;
  int start_x, start_y;
  int start_w, start_h;

  int direction;     /* one of the types: ISBOX_NORTH, ISBOX_SOUTH, ISBOX_WEST, ISBOX_EAST */
  char* oldcursor;
};


static int iSboxGetYborder(Ihandle* ih)
{
  if (ih->data->direction == ISBOX_NORTH || ih->data->direction == ISBOX_SOUTH)
    return ISBOX_THICK;
  else
    return 0;
}

static int iSboxGetXborder(Ihandle* ih)
{
  if (ih->data->direction == ISBOX_EAST || ih->data->direction == ISBOX_WEST)
    return ISBOX_THICK;
  else
    return 0;
}

static void iSboxSaveDimension(Ihandle* ih, int w, int h)
{
  ih->data->w = w;
  ih->data->h = h;
}

static void iSboxGetDecorSize(Ihandle* ih, int *w, int *h)
{
  /* size of the area available for the controls inside the sbox */
  *w -= iSboxGetXborder(ih);
  *h -= iSboxGetYborder(ih);
}

static void iSboxGetDecorOffset(Ihandle* ih, int *x, int *y)
{
  /* skip north thumb if there is one */
  if (ih->data->direction == ISBOX_NORTH)
    *y += ISBOX_THICK;

  /* skip west thumb if there is one */
  if (ih->data->direction == ISBOX_WEST)
    *x += ISBOX_THICK;
}

static void iSboxGetFinalSize(Ihandle* ih, int direction, int *w, int *h)
{
  int final_x, final_y;
  int diff_x, diff_y;

  iupStrToIntInt(IupGetGlobal("CURSORPOS"), &final_x, &final_y, 'x');

  diff_x = final_x - ih->data->start_x;
  diff_y = final_y - ih->data->start_y;

  if(direction == ISBOX_WEST)
    diff_x = -diff_x;

  if(direction == ISBOX_NORTH)
    diff_y = -diff_y;

  *w = diff_x + ih->data->start_w;
  *h = diff_y + ih->data->start_h;
}

static void iSboxShakeControls(Ihandle* ih)
{
  int new_w, new_h;

  iSboxGetFinalSize(ih, ih->data->direction, &new_w, &new_h);

  if(ih->data->direction == ISBOX_WEST || ih->data->direction == ISBOX_EAST)
  {
    if (new_w != ih->data->w)
    {
      if (new_w > ih->naturalwidth)
        iSboxSaveDimension(ih, new_w, ih->data->h);
      else 
        iSboxSaveDimension(ih, new_w, ih->naturalwidth);
    }
  }
  else if(ih->data->direction == ISBOX_SOUTH || ih->data->direction == ISBOX_NORTH)
  {
    if(new_h != ih->data->h)
    {
      if (new_h > ih->naturalheight)
        iSboxSaveDimension(ih, ih->data->w, new_h);
      else 
        iSboxSaveDimension(ih, ih->naturalheight, new_h);
    }
  }
  IupRefresh(ih);
}

/*****************************************************************************\
|* Callbacks of canvas bar                                                   *|
\*****************************************************************************/
static int iSboxMotion_CB(Ihandle* bar, int x, int y, char *r)
{
  Ihandle* ih = bar->parent;

  if (ih->data->isholding)
  {
    iSboxShakeControls(ih);
  }
  else if(ih->data->direction == ISBOX_EAST)
  {
    if (!ih->data->oldcursor)
      ih->data->oldcursor = IupGetAttribute(bar, "CURSOR");
    IupSetAttribute(bar, "CURSOR", "RESIZE_E");
  }
  else if(ih->data->direction == ISBOX_WEST)
  {
    if (!ih->data->oldcursor)
      ih->data->oldcursor = IupGetAttribute(bar, "CURSOR");
    IupSetAttribute(bar, "CURSOR", "RESIZE_W");
  }
  else if(ih->data->direction == ISBOX_NORTH)
  {
    if (!ih->data->oldcursor)
      ih->data->oldcursor = IupGetAttribute(bar, "CURSOR");
    IupSetAttribute(bar, "CURSOR", "RESIZE_N");
  }
  else if(ih->data->direction == ISBOX_SOUTH)
  {
    if (!ih->data->oldcursor)
      ih->data->oldcursor = IupGetAttribute(bar, "CURSOR");
    IupSetAttribute(bar, "CURSOR", "RESIZE_S");
  }
  else
  {
    if(ih->data->oldcursor)
    {
      IupSetAttribute(bar, "CURSOR", ih->data->oldcursor);
      ih->data->oldcursor = NULL;
    }
  }

  (void)x;
  (void)y;
  (void)r;
  return IUP_DEFAULT;
}

static int iSboxButton_CB(Ihandle* bar, int button, int pressed, int x, int y, char* status)
{
  Ihandle* ih = bar->parent;

  if (button!=IUP_BUTTON1)
    return IUP_DEFAULT;

  if (!ih->data->isholding && pressed)
  {
    ih->data->isholding = 1;
    /* Save the cursor position */
    iupStrToIntInt(IupGetGlobal("CURSORPOS"), &ih->data->start_x, &ih->data->start_y, 'x');

    /* Save the initial size */
    ih->data->start_w = ih->data->w;
    ih->data->start_h = ih->data->h;
  }
  else if (ih->data->isholding && !pressed)
    ih->data->isholding = 0;

  (void)x;
  (void)y;
  (void)status;
  return IUP_DEFAULT;
}

static int iSboxFocus_CB(Ihandle* bar, int focus)
{
  Ihandle* ih = bar->parent;

  if (!ih || focus) /* use only kill focus */
    return IUP_DEFAULT;

  if (ih->data->isholding)
    ih->data->isholding = 0;

  return IUP_DEFAULT;
}

/*****************************************************************************\
|* Attributes                                                                *|
\*****************************************************************************/
static int iSboxSetColorAttrib(Ihandle* ih, const char* value)
{
  IupSetAttribute(ih->firstchild, "BGCOLOR", value);
  return 0;  /* do not store value in hash table */
}

static int iSboxSetDirectionAttrib(Ihandle* ih, const char* value)
{
  if (ih->handle) /* only before map */
    return 0;

  if (iupStrEqual(value, "NORTH"))
    ih->data->direction = ISBOX_NORTH;
  else if(iupStrEqual(value, "SOUTH"))
    ih->data->direction = ISBOX_SOUTH;
  else if(iupStrEqual(value, "WEST"))
    ih->data->direction = ISBOX_WEST;
  else  /* Default = EAST */
    ih->data->direction = ISBOX_EAST;

  return 0;  /* do not store value in hash table */
}


/*****************************************************************************\
|* Methods                                                                   *|
\*****************************************************************************/


static void iSboxComputeNaturalSizeMethod(Ihandle* ih)
{
  iupBaseContainerUpdateExpand(ih);

  /* always initialize the natural size using the user size */
  ih->naturalwidth = ih->userwidth;
  ih->naturalheight = ih->userheight;

  if (ih->firstchild->brother)
  {
    Ihandle* child = ih->firstchild->brother;

    /* update child natural size first */
    iupClassObjectComputeNaturalSize(child);

    ih->expand &= child->expand; /* compose but only expand where the box can expand */

    ih->naturalwidth  = iupMAX(ih->naturalwidth,  child->naturalwidth  + iSboxGetXborder(ih));
    ih->naturalheight = iupMAX(ih->naturalheight, child->naturalheight + iSboxGetYborder(ih));

    ih->firstchild->naturalwidth  = ISBOX_THICK;
    ih->firstchild->naturalheight = ISBOX_THICK;

    /* update bar */
    if (ih->data->direction == ISBOX_EAST || ih->data->direction == ISBOX_WEST)
    {
      ih->data->w = iupMAX(ih->naturalwidth, ih->data->w);
      ih->data->h = ih->naturalheight;
      ih->firstchild->naturalwidth  = ISBOX_THICK;
      ih->firstchild->naturalheight = ih->data->h;
    }
    else  /* ISBOX_NORTH || ISBOX_SOUTH */
    {
      ih->data->w = ih->naturalwidth;
      ih->data->h = iupMAX(ih->naturalheight, ih->data->h);
      ih->firstchild->naturalwidth  = ih->data->w;
      ih->firstchild->naturalheight = ISBOX_THICK;
    }

    child->naturalwidth  = ih->data->w - iSboxGetXborder(ih);
    child->naturalheight = ih->data->h - iSboxGetYborder(ih);
    
  }

  ih->naturalwidth  = ih->data->w;
  ih->naturalheight = ih->data->h;
}

static void iSboxSetCurrentSizeMethod(Ihandle* ih, int w, int h, int shrink)
{
  /* Overriding, I only take orders from user */
  if (ih->data->direction == ISBOX_EAST || ih->data->direction == ISBOX_WEST)
    w = ih->data->w; 
  else
    h = ih->data->h; 

  iupBaseContainerSetCurrentSizeMethod(ih, w, h, shrink);

  if (ih->firstchild->brother)
  {
    iSboxGetDecorSize(ih, &w, &h);
    iupClassObjectSetCurrentSize(ih->firstchild->brother, w, h, shrink);
  }

  /* update bar */
  if((ih->data->direction == ISBOX_NORTH || ih->data->direction == ISBOX_SOUTH))
  {
    ih->firstchild->currentwidth  = w;
    ih->firstchild->currentheight = ISBOX_THICK;
  }
  else
  {
    ih->firstchild->currentwidth  = ISBOX_THICK;
    ih->firstchild->currentheight = h;
  }
}

static void iSboxSetPositionMethod(Ihandle* ih, int x, int y)
{
  int posx = 0, posy = 0;

  iupBaseSetPositionMethod(ih, x, y);

  if(ih->data->direction == ISBOX_EAST)
    posx = ih->data->w - ISBOX_THICK;
  if(ih->data->direction == ISBOX_SOUTH)
    posy = ih->data->h - ISBOX_THICK;

  iupClassObjectSetPosition(ih->firstchild, x+posx, y+posy);

  if (ih->firstchild->brother)
  {  
    iSboxGetDecorOffset(ih, &x, &y);
    iupClassObjectSetPosition(ih->firstchild->brother, x, y);
  } 
}

static int iSboxMapMethod(Ihandle* ih)
{
  ih->handle = (InativeHandle*)-1; /* fake value just to indicate that it is already mapped */
  return IUP_NOERROR;
}

static int iSboxCreateMethod(Ihandle* ih, void** params)
{
  Ihandle* bar;

  ih->data = iupALLOCCTRLDATA();

  ih->data->direction = ISBOX_EAST;
  ih->data->h = -1;
  ih->data->w = -1;

  bar = IupCanvas(NULL);
  iupChildTreeAppend(ih, bar);  /* bar will always be the firstchild */

  IupSetAttribute(bar, "BORDER", "YES");
  IupSetAttribute(bar, "EXPAND", "NO");
  IupSetAttribute(bar, "BGCOLOR", "200 200 200");

  /* Setting callbacks */
  IupSetCallback(bar, "BUTTON_CB", (Icallback) iSboxButton_CB);
  IupSetCallback(bar, "FOCUS_CB",  (Icallback) iSboxFocus_CB);
  IupSetCallback(bar, "MOTION_CB", (Icallback) iSboxMotion_CB);

  if (params)
  {
    Ihandle** iparams = (Ihandle**)params;
    if (*iparams)
      IupAppend(ih, *iparams);
  }

  return IUP_NOERROR;
}

Iclass* iupSboxGetClass(void)
{
  Iclass* ic = iupClassNew(NULL);

  ic->name   = "sbox";
  ic->format = "H";   /* one optional ihandle */
  ic->nativetype = IUP_TYPEVOID;
  ic->childtype  = IUP_CHILDMANY;  /* should be IUP_CHILDONE but has the IupCanvas */
  ic->is_interactive = 0;

  /* Class functions */
  ic->Create  = iSboxCreateMethod;
  ic->Map     = iSboxMapMethod;

  ic->ComputeNaturalSize = iSboxComputeNaturalSizeMethod;
  ic->SetCurrentSize     = iSboxSetCurrentSizeMethod;
  ic->SetPosition        = iSboxSetPositionMethod;

  /* Common */
  iupBaseRegisterCommonAttrib(ic);

  /* Base Container */
  iupClassRegisterAttribute(ic, "CLIENTSIZE", iupBaseGetRasterSizeAttrib, iupBaseNoSetAttrib, NULL, IUP_MAPPED, IUP_NO_INHERIT);
  iupClassRegisterAttribute(ic, "EXPAND", iupBaseContainerGetExpandAttrib, NULL, "YES", IUP_NOT_MAPPED, IUP_NO_INHERIT);

  /* IupSbox only */
  iupClassRegisterAttribute(ic, "COLOR",     NULL, iSboxSetColorAttrib,     NULL,   IUP_NOT_MAPPED, IUP_NO_INHERIT);
  iupClassRegisterAttribute(ic, "DIRECTION", NULL, iSboxSetDirectionAttrib, "EAST", IUP_NOT_MAPPED, IUP_NO_INHERIT);

  return ic;
}

Ihandle* IupSbox(Ihandle* child)
{
  void *params[2];
  params[0] = (void*)child;
  params[1] = NULL;
  return IupCreatev("sbox", params);
}
