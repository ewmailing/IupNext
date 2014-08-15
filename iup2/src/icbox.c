/** \file
 * \brief iupcbox control
 *
 * See Copyright Notice in iup.h
 *  */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>

#include "iglobal.h"
#include "idrv.h"
#include "iupkey.h"
#include "icpi.h"
#include "iupcbox.h"

static int moving = 0;
static int start_x = 0;
static int start_y = 0;

static void updateCXY(Ihandle *n, int x, int y)
{
  char str[20];
  sprintf(str, "%d", x);
  iupStoreEnv(n, "CX", str);
  sprintf(str, "%d", y);
  iupStoreEnv(n, "CY", str);
}

static Ihandle* getCBox(Ihandle* self)
{
  Ihandle* parent = IupGetParent(self);
  while (parent && !iupStrEqual("cbox", IupGetClassName(parent)))
    parent = IupGetParent(parent);
  return parent;
}

static int motion_cb(Ihandle *self,int x,int y,char *r)
{
  int dx, dy, posx, posy, w, h;
  int maxwidth = 0, maxheight = 0;

  IupSetAttribute(self, "CURSOR", "ARROW");

  /* move all the controls except the CBOX itself */
  if (iupStrEqual("cbox", IupGetClassName(self)))
    return IUP_DEFAULT;

  IupSetAttribute(self, "CURSOR", "HAND");

  if (!isbutton1(r)) return IUP_DEFAULT;
  if (!moving) return IUP_DEFAULT;

  dx = x - start_x;
  dy = y - start_y;

  posx = iupGetPosX(self);
  posy = iupGetPosY(self);

  posx += dx;
  posy += dy;

  if (posx < 0) posx = 0;
  if (posy < 0) posy = 0;

  iupGetSize(self, &w, &h);
  iupGetSize(getCBox(self), &maxwidth, &maxheight);
  if (posx+w > maxwidth) 
    posx = maxwidth-1 - w;
  if (posy+h > maxheight) 
    posy = maxheight-1 - h;

  updateCXY(self, posx, posy);

  iupCpiSetPosition(self, posx, posy);
  iupdrvResizeObjects(self);
  IupUpdate(IupGetDialog(self));

  return IUP_DEFAULT;
}

static int button_cb(Ihandle *self,int b,int press,int x,int y)
{
  if (b!=IUP_BUTTON1) return IUP_DEFAULT;

  if (press) 
  {
    moving = 1;
    start_x = x;
    start_y = y;
  }
  else
  {
    if (!moving) return IUP_DEFAULT;
    moving = 0;
  }

  return IUP_DEFAULT;
}

/* TEST_CODE
static void convertChar2Pixel(Ihandle *n, int *x, int *y)
{
  int w, h;
  iupdrvGetCharSize(n, &w, &h);
  *x = (*x * w)/4;
  *y = (*y * h)/8;
}
*/

static int boxsetnaturalsize (Ihandle * self)
{
  int maxwidth = 0, maxheight = 0;
  Ihandle *box = (Ihandle*)iupGetEnv(self, "__cbox_box");

  iupGetSize(self, &maxwidth, &maxheight);

  iupCpiSetNaturalSize(box);

  iupSetNaturalWidth (self, maxwidth);
  iupSetNaturalHeight (self, maxheight);
  return 0;
}

static void boxsetcurrentsize (Ihandle * self, int w, int h)
{
  Ihandle *box = (Ihandle*)iupGetEnv(self, "__cbox_box");
  Ihandle *child = IupGetNextChild(box, NULL);

  iupSetCurrentWidth(self, iupGetNaturalWidth(self));
  iupSetCurrentHeight(self, iupGetNaturalHeight(self));

  while(child != NULL)
  {
    iupCpiSetCurrentSize(child, iupGetNaturalWidth(child), iupGetNaturalHeight(child));
    child = IupGetBrother(child);
  }
}

static void boxsetposition (Ihandle * self, int x, int y)
{
  Ihandle *box = (Ihandle*)iupGetEnv(self, "__cbox_box");
  Ihandle *child = NULL;
  int cx = 0, cy = 0;

  iupSetPosX (self, x);
  iupSetPosY (self, y);

  child = IupGetNextChild(box, NULL);

  while(child != NULL)
  {
    cx = IupGetInt(child, "CX");
    cy = IupGetInt(child, "CY");

    iupCpiSetPosition(child, cx, cy);
    iupdrvResizeObjects (child);
   
    child = IupGetBrother(child);
  }
}

static char *boxgetattr (Ihandle * self, char *attr)
{
  if(iupStrEqual(attr, "EXPAND"))
    return IUP_NO;
  else if(iupStrEqual(attr, "SCROLLBAR"))
    return IUP_NO;
  else if(iupStrEqual(attr, "BGCOLOR"))
    return iupGetBgColor(self);
  else
    return iupCpiDefaultGetAttr (self, attr);
}

static void boxmap(Ihandle *self, Ihandle *parent)
{
  Ihandle *box = (Ihandle*)iupGetEnv(self, "__cbox_box");

  iupSetEnv (self, IUP_BORDER, IUP_NO);

  iupCpiDefaultMap(self, parent);

  iupSetEnv (self, IUP_BORDER, NULL);  /* to avoid propagating to the children */

  iupCpiMap(box, self);
}

static Ihandle *boxcreate (Iclass *box_class, void **params)
{
  Ihandle *self = IupCanvas(NULL);
  Ihandle *box = IupHbox(NULL);
  Ihandle *elem;
  
  while ((elem = *((Ihandle **) params)) != NULL)
  {
    IupAppend(box, elem);
    params = ((void **) params) + 1;
  }

  IupAppend(self, box);
  iupSetEnv(self, "__cbox_box", (char *)box);

  return self;
}

Ihandle *IupCboxv(Ihandle** params)
{
  return IupCreatev("cbox", (void**)params);
}

Ihandle *IupCbox (Ihandle * first,...)
{
  Ihandle **params = NULL;
  Ihandle *elem = NULL;

  va_list arglist;
  va_start(arglist, first);
  params = iupGetParamList(first, arglist);
  va_end(arglist);

  elem = IupCreatev("cbox", (void**)params);
  free(params);
  return elem;
}

Ihandle *IupCboxBuilderv(Ihandle** params)
{
  int i = 0;
  Ihandle *cbox;

  cbox = IupCreatev("cbox", (void**)params);

  while (params[i])
  {
    IupSetAttribute(params[i], "CURSOR", "HAND");
    i++;
  }

  iupSetEnv(cbox, "CURSOR", "ARROW");

  IupSetCallback(cbox,"CBUTTON_CB",(Icallback)button_cb);
  IupSetCallback(cbox,"CMOTION_CB",(Icallback)motion_cb);

  IupSetGlobal("MOUSEHOOK", "ON");

  free(params);
  return cbox;
}

Ihandle *IupCboxBuilder (Ihandle * first,...)
{
  Ihandle **params = NULL;

  va_list arglist;
  va_start(arglist, first);
  params = iupGetParamList(first, arglist);
  va_end(arglist);

  return IupCboxBuilderv(params);
}

void IupCboxOpen(void)
{
  Iclass* ICBox = iupCpiCreateNewClass("cbox" ,"C");

  iupCpiSetClassMethod(ICBox, ICPI_SETNATURALSIZE, (Imethod) boxsetnaturalsize);
  iupCpiSetClassMethod(ICBox, ICPI_SETCURRENTSIZE, (Imethod) boxsetcurrentsize);
  iupCpiSetClassMethod(ICBox, ICPI_SETPOSITION, (Imethod) boxsetposition);
  iupCpiSetClassMethod(ICBox, ICPI_MAP,    (Imethod) boxmap);
  iupCpiSetClassMethod(ICBox, ICPI_CREATE, (Imethod) boxcreate);
  iupCpiSetClassMethod(ICBox, ICPI_GETATTR,        (Imethod) boxgetattr);
}

/* TO DO 

For the IupCboxBuilder:

- display handlers on mouse over
- resize (conflict: the mouse callbacks are called only inside the controls,
                    handlers are usually drawn outsize the control.)
- right click could show context menu 
    - on the background: shows list of controls to add
                         option to save the configuration in LED, Lua and C.
    - on a control shows: remove, attributes, bring to top/send to bottom (zorder)
- select more than one control and move or align a group of controls.
- test a cbox inside another cbox
- how to manage IupRadio, IupTabs and IupSbox
- use WM_GETMINMAXINFO to avoid limits?
- change the example to position controls inside IupFrame using also concrete layout
  The IupFrame should start empty?
- Implement the global mouse callbacks in Motif
- Lua binding
- Should use Detach/Destroy to remove
         and Append/Map to add
         But IupMap is not well structured.
         Must fix dynamic IupMap for new controls added to the dialog.

Other:

- create a IupBoxBuilder for abstract layout positioning

*/
