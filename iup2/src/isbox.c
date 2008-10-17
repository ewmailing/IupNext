/** \file
 * \brief iupsbox control
 *
 * See Copyright Notice in iup.h
 * $Id: isbox.c,v 1.1 2008-10-17 06:19:20 scuri Exp $
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "iglobal.h"
#include "idrv.h"
#include "iupkey.h"
#include "icpi.h"
#include "iupsbox.h"


#ifdef WIN32
#include <windows.h>
#endif

#define MAX(a,b) ((a)>(b)?(a):(b))

#define SBOX_WIDTH   5
#define SBOX_DIV     3
#define DRAWTHUMB  1 /* when set draws a thumb for the user to drag sbox */

static Ihandle *isholding = NULL;
static int start_x   = 0;
static int start_y   = 0;
static int start_w   = 0;
static int start_h   = 0;

typedef enum
{
  NORTH=1, 
  SOUTH=2, 
  WEST=4, 
  EAST=8
}
tSboxType;

typedef struct tagSbox
{
  Ihandle *bar;        /* small canvas for the split handler   */
  Ihandle *mainchild;  /* A zbox to contain the split children */
  Ihandle *self;       /* big canvas, covering the background  */

  int w, h; /* user defined and current size */
  long client_w, client_h, client_x, client_y;
  long realnatural_w, realnatural_h;

  int direction; /* one of the types: NORTH, SOUTH, WEST, EAST */
  char *oldcursor;
}
tSbox;

static int iupsbox_button_cb(Ihandle *self, int b, int e, int mx, int my, char *r);
static int iupsbox_motion_cb(Ihandle *self, int x, int y, char *r);
static int iupsbox_focus_cb (Ihandle *self, int focus);

static int getyborder(tSbox *sbox)
{
  int border = 0;
  if(sbox->direction & NORTH)
    border += SBOX_WIDTH;
  if(sbox->direction & SOUTH)
    border += SBOX_WIDTH;
  return border;
}

static int getxborder(tSbox *sbox)
{
  int border = 0;
  if(sbox->direction & EAST)
    border += SBOX_WIDTH;
  if(sbox->direction & WEST)
    border += SBOX_WIDTH;
  return border;
}

static void savedimension(tSbox *sbox, int w, int h)
{
  sbox->w = w;
  sbox->h = h;

  /* size of the area available for the controls inside the sbox */
  sbox->client_w = w - getxborder(sbox);
  sbox->client_h = h - getyborder(sbox);

  /* start position of the controls inside the sbox */
  sbox->client_x = 0;
  sbox->client_y = 0;

  /* skip north thumb if there is one */
  if(sbox->direction & NORTH)
    sbox->client_y += SBOX_WIDTH;

  /* skip west thumb if there is one */
  if(sbox->direction & WEST)
    sbox->client_x += SBOX_WIDTH;
}

static int sboxsetnaturalsize(Ihandle *self)
{
  tSbox *sbox = (tSbox *) iupGetEnv (self, "_IUPSBOX_DATA");
  assert (sbox != NULL);
  if (sbox == NULL)
    return 0;

  iupCpiSetNaturalSize(sbox->mainchild);
  iupSetNaturalWidth(sbox->bar, SBOX_WIDTH);
  iupSetNaturalHeight(sbox->bar, SBOX_WIDTH);
  sbox->realnatural_w = iupGetNaturalWidth(sbox->mainchild) + getxborder(sbox);
  sbox->realnatural_h = iupGetNaturalHeight(sbox->mainchild) + getyborder(sbox);

  if((sbox->direction & EAST) || (sbox->direction & WEST))
    sbox->w = MAX(sbox->realnatural_w, sbox->w);
  else
    sbox->w = sbox->realnatural_w;

  if((sbox->direction & NORTH || sbox->direction & SOUTH))
    sbox->h = MAX(sbox->realnatural_h, sbox->h);
  else
    sbox->h = sbox->realnatural_h;

  if((sbox->direction & EAST) || (sbox->direction & WEST))
  {
    iupSetNaturalWidth(sbox->bar, SBOX_WIDTH);
    iupSetNaturalHeight(sbox->bar, sbox->h);
  }
  else
  {
    iupSetNaturalWidth(sbox->bar, sbox->w);
    iupSetNaturalHeight(sbox->bar, SBOX_WIDTH);
  }

  iupSetNaturalWidth (sbox->mainchild, sbox->w - getxborder(sbox));
  iupSetNaturalHeight(sbox->mainchild, sbox->h - getyborder(sbox));

  iupSetNaturalWidth (self, sbox->w);
  iupSetNaturalHeight(self, sbox->h);

  return 0;
}

static void sboxsetcurrentsize(Ihandle *self, int w, int h)
{
  tSbox *sbox = (tSbox *) iupGetEnv (self, "_IUPSBOX_DATA");
  assert (sbox != NULL);
  if (sbox == NULL)
    return;

  /* Overriding, I only take orders from user */
  if((sbox->direction & EAST || sbox->direction & WEST))
    w = sbox->w; 
  if((sbox->direction & NORTH || sbox->direction & SOUTH))
    h = sbox->h; 

  savedimension(sbox, w, h);

  iupSetCurrentWidth(self, w);
  iupSetCurrentHeight(self, h);

  iupCpiSetCurrentSize(sbox->mainchild, sbox->client_w, sbox->client_h);

  if((sbox->direction & NORTH || sbox->direction & SOUTH))
  {
    iupSetCurrentWidth(sbox->bar, w);
    iupSetCurrentHeight(sbox->bar, SBOX_WIDTH);
  }
  else
  {
    iupSetCurrentWidth(sbox->bar, SBOX_WIDTH);
    iupSetCurrentHeight(sbox->bar, h);
  }
}

static void sboxsetposition(Ihandle * self, int x, int y)
{
  int posx = 0, posy = 0;
  tSbox *sbox = (tSbox *) iupGetEnv (self, "_IUPSBOX_DATA");
  assert (sbox != NULL);
  if (sbox == NULL)
    return;

  iupSetPosX (self, x);
  iupSetPosY (self, y);

  if(sbox->direction==EAST)  posx = (sbox->w) - SBOX_WIDTH;
  if(sbox->direction==SOUTH) posy = (sbox->h) - SBOX_WIDTH;

  iupCpiSetPosition(sbox->bar, posx, posy);
  iupdrvResizeObjects(sbox->bar);

  iupCpiSetPosition(sbox->mainchild, sbox->client_x, sbox->client_y);
  iupdrvResizeObjects(sbox->mainchild);
}

static int iupsbox_focus_cb (Ihandle * self, int focus)
{
  tSbox *sbox = (tSbox *) iupGetEnv (self, "_IUPSBOX_DATA");
  if (sbox == NULL || focus) /* use only kill focus */
    return IUP_DEFAULT;

  /* avoid callback inheritance */
  if (sbox->bar != self)
    return IUP_IGNORE;

  if(isholding)
  {
#ifdef WIN32
    ReleaseCapture();
#endif
    isholding = NULL;
  }
  return IUP_DEFAULT;
}

static Ihandle *sboxcreate(Iclass *sbox_class, Ihandle **params)
{
  tSbox *sbox = (tSbox *) malloc (sizeof (tSbox));
  assert (sbox != NULL);
  if (sbox == NULL)
    return NULL;

  sbox->mainchild = IupZbox(params[0], NULL);
  sbox->self = IupCanvas(NULL);
  sbox->bar = IupCanvas(NULL);
  sbox->direction = EAST;
  sbox->realnatural_h = 0;
  sbox->realnatural_w = 0;
  sbox->h = -1;
  sbox->w = -1;
  sbox->oldcursor = NULL;

  {
    static int count = 1;
    char name[50];
    sprintf(name, "__sbox__%d", count);
    IupSetHandle(name, params[0]);
    IupStoreAttribute(sbox->mainchild, "VALUE", name);
    count++;
  }

  IupAppend(sbox->self, sbox->mainchild);
  IupAppend(sbox->self, sbox->bar);

  IupSetAttribute(sbox->bar, IUP_BORDER, IUP_YES);
  IupSetAttribute(sbox->bar, IUP_EXPAND, IUP_NO);
  IupSetAttribute(sbox->bar, IUP_BGCOLOR, "200 200 200");

  iupSetEnv(sbox->self, "_IUPSBOX_DATA", (char *) sbox);

  /* Setting callbacks */
  IupSetCallback(sbox->bar, "BUTTON_CB", (Icallback) iupsbox_button_cb);
  IupSetCallback(sbox->bar, "FOCUS_CB", (Icallback) iupsbox_focus_cb);
  IupSetCallback(sbox->bar, "MOTION_CB", (Icallback) iupsbox_motion_cb);
  
  return sbox->self;
}

static void sboxmap(Ihandle *self, Ihandle *parent)
{
  tSbox *sbox = (tSbox *) iupGetEnv (self, "_IUPSBOX_DATA");
  assert (sbox != NULL);
  if (sbox == NULL)
    return;

  iupSetEnv (self, IUP_BORDER, IUP_NO);

  iupCpiDefaultMap(self, parent);

  iupSetEnv (self, IUP_BORDER, NULL); /* to avoid propagating to the children */

  iupCpiMap(sbox->mainchild, self);
  iupCpiMap(sbox->bar, self);
}

static void sboxsetattr(Ihandle *self, char *attr, char *value)
{
  tSbox *sbox = (tSbox *) iupGetEnv (self, "_IUPSBOX_DATA");

  assert (sbox != NULL);
  if (sbox == NULL)
  {
    iupCpiDefaultSetAttr (self, attr, value);
    return;
  }
  else if (iupStrEqual(attr, IUP_COLOR))
  {
    IupSetAttribute(sbox->bar, IUP_BGCOLOR, value);
  }
  else if(iupStrEqual(attr, IUP_RASTERSIZE))
  {
    if(value && IupGetAttribute(self, IUP_WID))
    {
      int w=-1, h=-1;
      iupStrToIntInt(value, &w, &h, 'x');
      sbox->w = w;
      sbox->h = h;
      IupRefresh(self);
    }
    else
      sbox->w = sbox->h = -1;
  }
  else if(iupStrEqual(attr, IUP_SIZE))
  {
    if(IupGetAttribute(self, IUP_WID))
    {
      iupGetSize(self, &sbox->w, &sbox->h);
      IupRefresh(self);
    }
  }
  else if(iupStrEqual(attr, "DIRECTION"))
  {
    if(iupStrEqual(value, IUP_NORTH))
      sbox->direction = NORTH;
    else if(iupStrEqual(value, IUP_SOUTH))
      sbox->direction = SOUTH;
    else if(iupStrEqual(value, IUP_WEST))
      sbox->direction = WEST;
    else if(iupStrEqual(value, IUP_EAST))
      sbox->direction = EAST;
    else
    {
      assert(0); /* Wrong parameters */
    }
  }
  else
  {
    iupCpiDefaultSetAttr (self, attr, value);        /* chama metodo default */
    return;
  }
}

static char *sboxgetattr (Ihandle * self, char *attr)
{
  if(iupStrEqual(attr, "EXPAND"))
    return IUP_NO;
  else if(iupStrEqual(attr, "SCROLLBAR"))
    return IUP_NO;
  else if(iupStrEqual(attr, "GAP"))
    return "0";
  else if(iupStrEqual(attr, "BGCOLOR"))
  {
    char* color = iupGetEnv(self, IUP_BGCOLOR);
    if(color == NULL)
      color = iupGetParentBgColor(self);
    return color;
  }
  else if(iupStrEqual(attr, "MARGIN"))
    return "0x0";
  else
    return iupCpiDefaultGetAttr (self, attr);
}

Ihandle* IupSbox(Ihandle *child)
{
	return IupCreatep("sbox", child, NULL);
}

static void savecursorposition(void)
{
  iupStrToIntInt(IupGetGlobal("CURSORPOS"), &start_x, &start_y, 'x');
}

static void saveinitialsize(int w, int h)
{
  start_w = w;
  start_h = h;
}

static void getfinalsize(int direction, int *w, int *h)
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

static void capturemouse(Ihandle* self)
{
#ifdef WIN32
  SetCapture((HWND) IupGetAttribute(self, IUP_WID));
#endif
}

static int iupsbox_button_cb(Ihandle *self, int b, int e, int mx, int my, char *r)
{
  tSbox *sbox = (tSbox *) iupGetEnv (self, "_IUPSBOX_DATA");

  /* avoid callback inheritance */
  if (sbox->bar != self)
    return IUP_IGNORE;

  if(isholding == NULL && e && sbox->direction == EAST)
  {
    isholding = self;
    capturemouse(self);
    savecursorposition();
    saveinitialsize(sbox->w, sbox->h);
  }
  else if(isholding == NULL && e && sbox->direction == WEST)
  {
    isholding = self;
    capturemouse(self);
    savecursorposition();
    saveinitialsize(sbox->w, sbox->h);
  }
  else if(isholding == NULL && e && sbox->direction == NORTH)
  {
    isholding = self;
    capturemouse(self);
    savecursorposition();
    saveinitialsize(sbox->w, sbox->h);
  }
  else if(isholding == NULL && e && sbox->direction == SOUTH)
  {
    isholding = self;
    capturemouse(self);
    savecursorposition();
    saveinitialsize(sbox->w, sbox->h);
  }
  else if(e == 0 && isholding)
  {
    isholding = NULL;
  }
  return IUP_DEFAULT;
}

static void shakecontrols(tSbox *sbox)
{
  int new_w, new_h;
  getfinalsize(sbox->direction, &new_w, &new_h);

  if(sbox->direction == WEST || sbox->direction == EAST)
  {
    if (new_w != sbox->w)
    {
      if (new_w > sbox->realnatural_w)
      {
        savedimension(sbox, new_w, sbox->h);
        IupRefresh(sbox->self);
      }
      else 
      {
        savedimension(sbox, sbox->realnatural_w, sbox->h);
        IupRefresh(sbox->self);
      }
    }
  }
  else if(sbox->direction == SOUTH || sbox->direction == NORTH)
  {
    if(new_h != sbox->h)
    {
      if (new_h > sbox->realnatural_h)
      {
        savedimension(sbox, sbox->w, new_h);
        IupRefresh(sbox->self);
      }
      else 
      {
        savedimension(sbox, sbox->realnatural_h, new_h);
        IupRefresh(sbox->self);
      }
    }
  }
}

static int iupsbox_motion_cb(Ihandle *self, int x, int y, char *r)
{
  tSbox *sbox = (tSbox *) iupGetEnv (self, "_IUPSBOX_DATA");

  /* avoid callback inheritance */
  if (sbox->bar != self)
    return IUP_IGNORE;

  /* avoids unnecessary repaints (doesn't happen in Windows because of SetCapture) */
  if(isholding && isholding != self)
    return IUP_DEFAULT;

  if(isholding)
  {
    shakecontrols(sbox);
  }
  else if(sbox->direction == EAST)
  {
    if(sbox->oldcursor == NULL)
      sbox->oldcursor = IupGetAttribute(self, IUP_CURSOR);
    IupSetAttribute(self, IUP_CURSOR, IUP_RESIZE_E);
  }
  else if(sbox->direction == WEST)
  {
    if(sbox->oldcursor == NULL)
      sbox->oldcursor = IupGetAttribute(self, IUP_CURSOR);
    IupSetAttribute(self, IUP_CURSOR, IUP_RESIZE_W);
  }
  else if(sbox->direction == NORTH)
  {
    if(sbox->oldcursor == NULL)
      sbox->oldcursor = IupGetAttribute(self, IUP_CURSOR);
    IupSetAttribute(self, IUP_CURSOR, IUP_RESIZE_N);
  }
  else if(sbox->direction == SOUTH)
  {
    if(sbox->oldcursor == NULL)
      sbox->oldcursor = IupGetAttribute(self, IUP_CURSOR);
    IupSetAttribute(self, IUP_CURSOR, IUP_RESIZE_S);
  }
  else
  {
    if(sbox->oldcursor)
    {
      IupSetAttribute(self, IUP_CURSOR, sbox->oldcursor);
      sbox->oldcursor = NULL;
    }
  }
  return IUP_DEFAULT;
}

static void sboxdestroy(Ihandle *c)
{
  tSbox *sbox = (tSbox *) iupGetEnv (c, "_IUPSBOX_DATA");
  assert (sbox);
  if (sbox == NULL)
    return;
  free (sbox);
  iupSetEnv(c, "_IUPSBOX_DATA", NULL);
}

void IupSboxOpen (void)
{
  Iclass* ICsbox = iupCpiCreateNewClass("sbox" , "C");

  iupCpiSetClassMethod(ICsbox, ICPI_SETNATURALSIZE, (Imethod) sboxsetnaturalsize);
  iupCpiSetClassMethod(ICsbox, ICPI_SETCURRENTSIZE, (Imethod) sboxsetcurrentsize);
  iupCpiSetClassMethod(ICsbox, ICPI_SETPOSITION,    (Imethod) sboxsetposition);
  iupCpiSetClassMethod(ICsbox, ICPI_CREATE,         (Imethod) sboxcreate);
  iupCpiSetClassMethod(ICsbox, ICPI_DESTROY,        (Imethod) sboxdestroy);
  iupCpiSetClassMethod(ICsbox, ICPI_MAP,            (Imethod) sboxmap);
  iupCpiSetClassMethod(ICsbox, ICPI_SETATTR,        (Imethod) sboxsetattr);
  iupCpiSetClassMethod(ICsbox, ICPI_GETATTR,        (Imethod) sboxgetattr);
}
