/** \file
 * \brief iupsbox control
 *
 * See Copyright Notice in iup.h
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

#ifdef WIN32
  #include <windows.h>
#endif


#define ISBOX_WIDTH 5

static Ihandle *isholding = NULL;
static int start_x   = 0;
static int start_y   = 0;
static int start_w   = 0;
static int start_h   = 0;

typedef enum
{
  NORTH = 1, 
  SOUTH = 2, 
  WEST  = 4, 
  EAST  = 8
}
tSboxType;

struct _IcontrolData
{
  iupCanvas canvas;  /* from IupCanvas (must reserve it) */

  Ihandle* bar;      /* small canvas for the split handler   */
  Ihandle* zbox;     /* A zbox to contain the split children */

  int w, h;          /* user defined and current size */

  int direction;     /* one of the types: NORTH, SOUTH, WEST, EAST */
  char* oldcursor;
};


static int iSboxGetYborder(Ihandle* ih)
{
  int border = 0;
  if(ih->data->direction & NORTH)
    border += ISBOX_WIDTH;
  if(ih->data->direction & SOUTH)
    border += ISBOX_WIDTH;
  return border;
}

static int iSboxGetXborder(Ihandle* ih)
{
  int border = 0;
  if(ih->data->direction & EAST)
    border += ISBOX_WIDTH;
  if(ih->data->direction & WEST)
    border += ISBOX_WIDTH;
  return border;
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
  if(ih->data->direction & NORTH)
    *y += ISBOX_WIDTH;

  /* skip west thumb if there is one */
  if(ih->data->direction & WEST)
    *x += ISBOX_WIDTH;
}

static void iSboxSaveCursorPosition(void)
{
  iupStrToIntInt(IupGetGlobal("CURSORPOS"), &start_x, &start_y, 'x');
}

static void iSboxSaveInitialSize(int w, int h)
{
  start_w = w;
  start_h = h;
}

static void iSboxGetFinalSize(int direction, int *w, int *h)
{
  int final_x, final_y;
  int diff_x, diff_y;

  char *v = NULL;
  v = IupGetGlobal("CURSORPOS");
  iupStrToIntInt(v, &final_x, &final_y, 'x');

  diff_x = final_x - start_x;
  diff_y = final_y - start_y;

  if(direction == WEST)
    diff_x = -diff_x;

  if(direction == NORTH)
    diff_y = -diff_y;

  *w = diff_x + start_w;
  *h = diff_y + start_h;
}

static void iSboxCaptureMouse(Ihandle* ih)
{
#ifdef WIN32
  SetCapture((HWND) IupGetAttribute(ih, "WID"));
#endif
}

static void iSboxShakeControls(Ihandle* ih)
{
  int new_w, new_h;

  iSboxGetFinalSize(ih->data->direction, &new_w, &new_h);

  if(ih->data->direction == WEST || ih->data->direction == EAST)
  {
    if (new_w != ih->data->w)
    {
      if (new_w > ih->naturalwidth)
        iSboxSaveDimension(ih, new_w, ih->data->h);
      else 
        iSboxSaveDimension(ih, new_w, ih->naturalwidth);
    }
  }
  else if(ih->data->direction == SOUTH || ih->data->direction == NORTH)
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

  /* avoids unnecessary repaints (doesn't happen in Windows because of SetCapture) */
  if(isholding && isholding != bar)
    return IUP_DEFAULT;

  if(isholding)
  {
    iSboxShakeControls(ih);
  }
  else if(ih->data->direction == EAST)
  {
    if(ih->data->oldcursor == NULL)
      ih->data->oldcursor = IupGetAttribute(bar, "CURSOR");
    IupSetAttribute(bar, "CURSOR", "RESIZE_E");
  }
  else if(ih->data->direction == WEST)
  {
    if(ih->data->oldcursor == NULL)
      ih->data->oldcursor = IupGetAttribute(bar, "CURSOR");
    IupSetAttribute(bar, "CURSOR", "RESIZE_W");
  }
  else if(ih->data->direction == NORTH)
  {
    if(ih->data->oldcursor == NULL)
      ih->data->oldcursor = IupGetAttribute(bar, "CURSOR");
    IupSetAttribute(bar, "CURSOR", "RESIZE_N");
  }
  else if(ih->data->direction == SOUTH)
  {
    if(ih->data->oldcursor == NULL)
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

static int iSboxButton_CB(Ihandle* bar, int b, int e, int mx, int my, char* r)
{
  Ihandle* ih = bar->parent;

  if(isholding == NULL && e)
  {
    isholding = bar;
    iSboxCaptureMouse(bar);
    iSboxSaveCursorPosition();
    iSboxSaveInitialSize(ih->data->w, ih->data->h);
  }
  else if(e == 0 && isholding)
  {
    isholding = NULL;
  }

  (void)b;
  (void)mx;
  (void)my;
  (void)r;
  return IUP_DEFAULT;
}

static int iSboxFocus_CB(Ihandle* bar, int focus)
{
  Ihandle* ih = bar->parent;

  if (!ih || focus) /* use only kill focus */
    return IUP_DEFAULT;

  if(isholding)
  {
#ifdef WIN32
    ReleaseCapture();
#endif
    isholding = NULL;
  }
  return IUP_DEFAULT;
}

/*****************************************************************************\
|* Attributes                                                                *|
\*****************************************************************************/
static int iSboxSetColorAttrib(Ihandle* ih, const char* value)
{
  IupSetAttribute(ih->data->bar, "BGCOLOR", value);
  return 0;  /* do not store value in hash table */
}

static int iSboxSetDirectionAttrib(Ihandle* ih, const char* value)
{
  if(iupStrEqual(value, "NORTH"))
    ih->data->direction = NORTH;
  else if(iupStrEqual(value, "SOUTH"))
    ih->data->direction = SOUTH;
  else if(iupStrEqual(value, "WEST"))
    ih->data->direction = WEST;
  else  /* Default = EAST */
    ih->data->direction = EAST;

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

  if (ih->firstchild)
  {
    Ihandle* child = ih->firstchild;

    /* update child natural size first */
    iupClassObjectComputeNaturalSize(child);

    ih->expand &= child->expand; /* compose but only expand where the box can expand */

    ih->naturalwidth  = iupMAX(ih->naturalwidth,  child->naturalwidth  + iSboxGetXborder(ih));
    ih->naturalheight = iupMAX(ih->naturalheight, child->naturalheight + iSboxGetYborder(ih));

    ih->data->bar->naturalwidth  = ISBOX_WIDTH;
    ih->data->bar->naturalheight = ISBOX_WIDTH;

    /* update bar */
    if((ih->data->direction & EAST) || (ih->data->direction & WEST))
      ih->data->w = iupMAX(ih->naturalwidth, ih->data->w);
    else
      ih->data->w = ih->naturalwidth;

    if((ih->data->direction & NORTH || ih->data->direction & SOUTH))
      ih->data->h = iupMAX(ih->naturalheight, ih->data->h);
    else
      ih->data->h = ih->naturalheight;

    if((ih->data->direction & EAST) || (ih->data->direction & WEST))
    {
      ih->data->bar->naturalwidth  = ISBOX_WIDTH;
      ih->data->bar->naturalheight = ih->data->h;
    }
    else  /* NORTH || SOUTH */
    {
      ih->data->bar->naturalwidth  = ih->data->w;
      ih->data->bar->naturalheight = ISBOX_WIDTH;
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
  if((ih->data->direction & EAST || ih->data->direction & WEST))
    w = ih->data->w; 
  if((ih->data->direction & NORTH || ih->data->direction & SOUTH))
    h = ih->data->h; 

  if (shrink)
  {
    /* if expand use the given size, else use the natural size */
    ih->currentwidth  = (ih->expand & IUP_EXPAND_WIDTH) ? w: ih->naturalwidth;
    ih->currentheight = (ih->expand & IUP_EXPAND_HEIGHT)? h: ih->naturalheight;
  }
  else
  {
    /* if expand use the given size (if greater than natural size), else use the natural size */
    ih->currentwidth  = (ih->expand & IUP_EXPAND_WIDTH) ? iupMAX(ih->naturalwidth,  w): ih->naturalwidth;
    ih->currentheight = (ih->expand & IUP_EXPAND_HEIGHT)? iupMAX(ih->naturalheight, h): ih->naturalheight;
  }

  /* always has a zbox child */
  if(ih->firstchild)
  {
    iSboxGetDecorSize(ih, &w, &h);
    iupClassObjectSetCurrentSize(ih->firstchild, w, h, shrink);
  }

  /* update bar */
  {
    if((ih->data->direction & NORTH || ih->data->direction & SOUTH))
    {
      ih->data->bar->currentwidth  = w;
      ih->data->bar->currentheight = ISBOX_WIDTH;
    }
    else
    {
      ih->data->bar->currentwidth  = ISBOX_WIDTH;
      ih->data->bar->currentheight = h;
    }
  }
}

static void iSboxSetPositionMethod(Ihandle* ih, int x, int y)
{
  int posx = 0, posy = 0;

  iupBaseSetPositionMethod(ih, x, y);

  if(ih->data->direction == EAST)
    posx = ih->data->w - ISBOX_WIDTH;
  if(ih->data->direction == SOUTH)
    posy = ih->data->h - ISBOX_WIDTH;

  iupClassObjectSetPosition(ih->data->bar, x+posx, y+posy);

  if (ih->firstchild)
  {  
    iSboxGetDecorOffset(ih, &x, &y);
    iupClassObjectSetPosition(ih->firstchild, x, y);
  } 
}

static int iSboxMapMethod(Ihandle* ih)
{
  ih->handle = (InativeHandle*)-1; /* fake value just to indicate that it is already mapped */
  return IUP_NOERROR;
}

static int iSboxCreateMethod(Ihandle* ih, void** params)
{
  if (params)
  {
    Ihandle** iparams = (Ihandle**)params;
    if (*iparams)
      IupAppend(ih, *iparams);
  }

  /* free the data allocated by IupCanvas */
  if (ih->data)
    free(ih->data);
  ih->data = iupALLOCCTRLDATA();

  ih->data->direction = EAST;
  ih->data->h = -1;
  ih->data->w = -1;
  ih->data->oldcursor = NULL;

  ih->data->zbox = IupZbox(params[0], NULL);
  ih->firstchild = ih->data->zbox;
  ih->data->zbox->parent = ih;

  ih->data->bar = IupCanvas(NULL);
  ih->data->zbox->brother = ih->data->bar;
  ih->data->bar->parent = ih;

  IupSetAttribute(ih->data->bar, "BORDER", "YES");
  IupSetAttribute(ih->data->bar, "EXPAND", "NO");
  IupSetAttribute(ih->data->bar, "BGCOLOR", "200 200 200");

  /* Setting callbacks */
  IupSetCallback(ih->data->bar, "BUTTON_CB", (Icallback) iSboxButton_CB);
  IupSetCallback(ih->data->bar, "FOCUS_CB",  (Icallback) iSboxFocus_CB);
  IupSetCallback(ih->data->bar, "MOTION_CB", (Icallback) iSboxMotion_CB);

  return IUP_NOERROR;
}

Iclass* iupSboxGetClass(void)
{
  Iclass* ic = iupClassNew(NULL);

  ic->name   = "sbox";
  ic->format = "H";
  ic->nativetype = IUP_TYPEVOID;
  ic->childtype  = IUP_CHILD_ONE;
  ic->is_interactive = 0;

  /* Class functions */
  ic->Create  = iSboxCreateMethod;
  ic->Map     = iSboxMapMethod;

  ic->ComputeNaturalSize = iSboxComputeNaturalSizeMethod;
  ic->SetCurrentSize     = iSboxSetCurrentSizeMethod;
  ic->SetPosition        = iSboxSetPositionMethod;

 /* IupSbox Callbacks */
  iupClassRegisterCallback(ic, "BUTTON_CB", "iiiis");
  iupClassRegisterCallback(ic, "MOTION_CB", "iis");
  iupClassRegisterCallback(ic, "FOCUS_CB",  "i");

  /* Common */
  iupBaseRegisterCommonAttrib(ic);

  /* Visual */
  iupBaseRegisterVisualAttrib(ic);

  /* Base Container */
  iupClassRegisterAttribute(ic, "CLIENTSIZE", iupBaseGetRasterSizeAttrib, iupBaseNoSetAttrib, NULL, IUP_MAPPED, IUP_NO_INHERIT);
  iupClassRegisterAttribute(ic, "EXPAND", iupBaseContainerGetExpandAttrib, NULL, "YES", IUP_NOT_MAPPED, IUP_NO_INHERIT);

  /* IupSbox only */
  iupClassRegisterAttribute(ic, "COLOR",     NULL, iSboxSetColorAttrib,     NULL,   IUP_MAPPED, IUP_NO_INHERIT);
  iupClassRegisterAttribute(ic, "DIRECTION", NULL, iSboxSetDirectionAttrib, "EAST", IUP_MAPPED, IUP_NO_INHERIT);

  return ic;
}

Ihandle* IupSbox(Ihandle* child)
{
  void *params[2];
  params[0] = (void*)child;
  params[1] = NULL;
  return IupCreatev("sbox", params);
}
