/** \file
* \brief creates controls. but no map to the native control yet
*
* See Copyright Notice in iup.h
* $Id: icreate.c,v 1.1 2008-10-17 06:19:20 scuri Exp $
*/

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "iglobal.h"
#include "idlglist.h"
#include "itree.h"

char* UNKNOWN_ = "unknown";
char* IMAGE_ = "image";
char* BUTTON_ = "button";
char* CANVAS_ = "canvas";
char* DIALOG_ = "dialog";
char* FILL_ = "fill";
char* FRAME_ = "frame";
char* HBOX_ = "hbox";
char* ITEM_ = "item";
char* SEPARATOR_ = "separator";
char* SUBMENU_ = "submenu";
char* KEYACTION_ = "keyaction";
char* LABEL_ = "label";
char* LIST_ = "list";
char* MENU_ = "menu";
char* RADIO_ = "radio";
char* TEXT_ = "text";
char* TOGGLE_ = "toggle";
char* VBOX_ = "vbox";
char* ZBOX_ = "zbox";
char* MULTILINE_ = "multiline";
char* USER_ = "user";
char* CONTROL_ = "control";

Ihandle* IupImage (int width, int height, const unsigned char *pixmap)
{
  Ihandle *n = iupTreeCreateNode(NULL);
  type(n) = IMAGE_;
  IupSetfAttribute(n,IUP_WIDTH,"%u", width);
  IupSetfAttribute(n,IUP_HEIGHT,"%u", height);
  IupSetAttribute(n,"BPP","8");
  IupSetAttribute(n,"CHANNELS","1");
  image_data(n) = (char *) malloc (width*height);
  if (image_data(n))
    memcpy (image_data(n),pixmap,width*height);
  return n;
}

Ihandle* IupButton (const char* label, const char* action)
{
  Ihandle *n = iupTreeCreateNode(NULL);
  type(n) = BUTTON_;
  IupStoreAttribute (n,IUP_TITLE,label);
  if (action) IupStoreAttribute (n,IUP_ACTION,action);
  return n;
}

Ihandle* IupCanvas (const char* action)
{
  Ihandle *n = iupTreeCreateNode(NULL);
  type(n) = CANVAS_;
  if (action) IupStoreAttribute (n,IUP_ACTION,action);
  return n;
}

Ihandle* IupDialog (Ihandle* exp)
{
  Ihandle *n = iupTreeCreateNode(NULL);
  type(n) = DIALOG_;
  child(n) = exp;
  parent(exp)=n;
  iupDlgListAdd(n);
  return n;
}

Ihandle* IupFill (void)
{
  Ihandle *n = iupTreeCreateNode(NULL);
  type(n) = FILL_;
  return n;
}

Ihandle* IupFrame (Ihandle* exp)
{
  Ihandle *n = iupTreeCreateNode(NULL);
  type(n) = FRAME_;
  child(n) = exp;
  parent(exp)=n;
  return n;
}

Ihandle* IupUser (void)
{
  Ihandle *n = iupTreeCreateNode(NULL);
  type(n) = USER_;
  return n;
}

Ihandle* iupMkHbox (Ihandle* box)
{
  Ihandle *n = box;
  type(n) = HBOX_;
  return n;
}

Ihandle *IupHboxv(Ihandle **params)
{
  int i;
  Ihandle *ret;
  Ihandle *box = iupTreeCreateNode (NULL);

  for (i=0; params[i]; i++)
    box = iupTreeAppendNode(box, params[i]);

  ret = iupMkHbox (box);
  IupSetAttribute(ret, IUP_ALIGNMENT, IUP_ATOP);
  return ret;
}

Ihandle *IupHbox (Ihandle *first, ...)
{
  Ihandle *exp;
  Ihandle *ret;
  va_list arg;
  Ihandle *box = iupTreeCreateNode (NULL);
  va_start (arg, first);

  for (exp=first; exp; exp=va_arg(arg,Ihandle *))
    box = iupTreeAppendNode (box, exp);

  va_end (arg);
  ret = iupMkHbox (box);
  IupSetAttribute(ret, IUP_ALIGNMENT, IUP_ATOP);
  return ret;
}


Ihandle* IupItem (const char* label, const char* action)
{
  Ihandle *n = iupTreeCreateNode(NULL);
  type(n) = ITEM_;
  IupStoreAttribute (n,IUP_TITLE,label);
  if (action) IupStoreAttribute (n,IUP_ACTION,action);
  return n;
}

Ihandle* IupSubmenu (const char* label, Ihandle* exp)
{
  Ihandle *n = iupTreeCreateNode(NULL);
  type(n) = SUBMENU_;
  child(n) = exp;
  parent(exp)=n;
  IupStoreAttribute (n,IUP_TITLE,label);
  return n;
}

Ihandle* iupMkKeyAction (Itable* list)
{
  Ihandle *n = iupTreeCreateNode(NULL);
  type(n) = KEYACTION_;
  keyaction_list(n) = list;
  return n;
}

Ihandle* IupLabel (const char* label)
{
  Ihandle *n = iupTreeCreateNode(NULL);
  type(n) = LABEL_;
  IupStoreAttribute (n,IUP_TITLE,label);
  IupSetAttribute(n, IUP_ALIGNMENT, IUP_ALEFT);
  return n;
}

Ihandle* IupList (const char* action)
{
  Ihandle *n = iupTreeCreateNode(NULL);
  type(n) = LIST_;
  if (action) IupStoreAttribute (n,IUP_ACTION,action);
  return n;
}

Ihandle* iupMkMenu (Ihandle* menu)
{
  type(menu) = MENU_;
  return menu;
}

Ihandle *IupMenuv(Ihandle **params)
{
  int i;
  Ihandle *menu = iupTreeCreateNode (NULL);

  for (i=0; params[i]; i++)
    menu = iupTreeAppendNode(menu, params[i]);

  return iupMkMenu (menu);
}

Ihandle *IupMenu (Ihandle *first, ...)
{
  Ihandle *exp;
  va_list arg;
  Ihandle *menu = iupTreeCreateNode (NULL);
  va_start (arg, first);

  for (exp=first; exp; exp=va_arg(arg,Ihandle*))
    menu = iupTreeAppendNode (menu, exp);

  va_end (arg);
  return iupMkMenu (menu);
}

Ihandle* IupRadio (Ihandle* exp)
{
  Ihandle *n = iupTreeCreateNode(NULL);
  type(n) = RADIO_;
  child(n) = exp;
  parent(exp)=n;
  return n;
}

Ihandle* IupText (const char* action)
{
  Ihandle *n = iupTreeCreateNode(NULL);
  type(n) = TEXT_;
  if (action) IupStoreAttribute (n,IUP_ACTION,action);
  IupSetAttribute(n, IUP_ALIGNMENT, IUP_ALEFT);
  return n;
}

Ihandle* IupMultiLine (const char* action)
{
  Ihandle *n = iupTreeCreateNode(NULL);
  type(n) = MULTILINE_;
  if (action) IupStoreAttribute (n,IUP_ACTION,action);
  IupSetAttribute(n, IUP_ALIGNMENT, IUP_ALEFT);
  return n;
}

Ihandle* IupToggle (const char *label, const char *action)
{
  Ihandle *n = iupTreeCreateNode(NULL);
  type(n) = TOGGLE_;
  IupStoreAttribute (n,IUP_TITLE,label);
  if (action) IupStoreAttribute (n,IUP_ACTION,action);
  return n;
}

Ihandle* IupSeparator (void)
{
  Ihandle *n = iupTreeCreateNode(NULL);
  type(n) = SEPARATOR_;
  return n;
}

Ihandle* iupMkVbox (Ihandle* box)
{
  Ihandle *n = box;
  type(n) = VBOX_;
  return n;
}

Ihandle *IupVboxv(Ihandle **params)
{
  int i;
  Ihandle *ret;
  Ihandle *box = iupTreeCreateNode (NULL);

  for (i=0; params[i]; i++)
    box = iupTreeAppendNode(box, params[i]);

  ret = iupMkVbox (box);
  IupSetAttribute(ret, IUP_ALIGNMENT, IUP_ALEFT);
  return ret;
}

Ihandle *IupVbox (Ihandle* first, ...)
{
  Ihandle *exp;
  Ihandle *ret;
  va_list arg;
  Ihandle *box = iupTreeCreateNode (NULL);

  va_start (arg, first);

  for (exp=first; exp; exp=va_arg(arg,Ihandle*))
    box = iupTreeAppendNode (box, exp);

  va_end (arg);
  ret = iupMkVbox (box);
  IupSetAttribute(ret, IUP_ALIGNMENT, IUP_ALEFT);
  return ret;
}

Ihandle* iupMkZbox (Ihandle* box)
{
  Ihandle *n = box;
  type(n) = ZBOX_;
  return n;
}

Ihandle *IupZboxv(Ihandle **params)
{
  int i;
  Ihandle *ret;
  Ihandle *box = iupTreeCreateNode (NULL);

  for (i=0; params[i]; i++)
    box = iupTreeAppendNode(box, params[i]);

  ret = iupMkZbox (box);
  IupSetAttribute(ret, IUP_ALIGNMENT, IUP_NE);
  return ret;
}

Ihandle *IupZbox (Ihandle* first, ...)
{
  Ihandle *exp;
  Ihandle *ret;
  va_list arg;
  Ihandle *box = iupTreeCreateNode (NULL);
  va_start (arg, first);

  for (exp=first; exp; exp=va_arg(arg,Ihandle*))
    box = iupTreeAppendNode (box, exp);

  va_end (arg);
  ret = iupMkZbox (box);
  IupSetAttribute(ret, IUP_ALIGNMENT, IUP_NE);
  return ret;
}

Ihandle* IupCreatev(const char *name, void **params)
{
  Iclass *ic = NULL;
  char lower[30];
  int size = 0;
  assert(name);
  if(name == NULL)
    return NULL;

  size = (int)strlen(name);
  size++;
  assert(size < 30 && size > 0); /* invalid name */
  if(size > 30 || size <= 0)
    return NULL;

  while(size--)
    lower[size] = (char)tolower(name[size]);

  if(iupStrEqual(lower, "dialog"))
    return IupDialog(params? params[0] : NULL);
  else if(iupStrEqual(lower, "canvas"))
    return IupCanvas(NULL);
  else if(iupStrEqual(lower, "button"))
    return IupButton(NULL, NULL);
  else if(iupStrEqual(lower, "toggle"))
    return IupToggle(NULL, NULL);
  else if(iupStrEqual(lower, "label"))
    return IupLabel(NULL);
  else if(iupStrEqual(lower, "frame"))
    return IupFrame(NULL);
  else if(iupStrEqual(lower, "list"))
    return IupList(NULL);
  else if(iupStrEqual(lower, "multiline"))
    return IupMultiLine(NULL);
  else if(iupStrEqual(lower, "text"))
    return IupText(NULL);
  else if(iupStrEqual(lower, "user"))
    return IupUser();
  else if(iupStrEqual(lower, "fill"))
    return IupFill();
  else if(iupStrEqual(lower, "separator"))
    return IupSeparator();
  else if(iupStrEqual(lower, "item"))
    return IupItem(NULL, NULL);
  else if(iupStrEqual(lower, "hbox"))
    return IupHboxv((Ihandle**)params);
  else if(iupStrEqual(lower, "zbox"))
    return IupZboxv((Ihandle**)params);
  else if(iupStrEqual(lower, "vbox"))
    return IupVboxv((Ihandle**)params);
  else if(iupStrEqual(lower, "menu"))
    return IupMenuv((Ihandle**)params);
  else if(iupStrEqual(lower, "radio"))
    return IupRadio(params[0]);
  else if(iupStrEqual(lower, "submenu"))
    return IupSubmenu(NULL, params? params[0] : NULL);

  /*  
  Not accepted:
  image
  color
  IupGetFile
  IupListDialog
  IupMessage
  IupScanf
  IupAlarm
  */

  ic = iupCpiGetClass(lower);
  return iupCpiCreate(ic, params);
}

Ihandle* IupCreate(const char *name)
{
  return IupCreatev(name, NULL);
}

Ihandle *IupCreatep(const char *name, void* first, ...)
{
  void **params = NULL;
  Ihandle *elem = NULL;
  unsigned int i = 0;
  va_list arg;

  /* finding number of elements */
  if(first)
  {
    va_start (arg, first); 
    i = 1;
    while (va_arg(arg, void *))
      i++;
    va_end (arg);

    /* allocating memory for array */
    params = (void**) malloc (sizeof (void*) * (i+1));
    assert(params != NULL);
    if(params == NULL) 
      return NULL;

    /* really reading them */
    i = 0;
    va_start (arg, first); 
    elem = first;
    while (elem != NULL)
    {
      params[i++] = elem;
      elem = va_arg(arg, void *);
    }
    params[i] = NULL;
    va_end (arg);
  }

  /* creating for real */
  elem = IupCreatev(name, params);

  /* no leak */
  free(params);

  return elem;
}
