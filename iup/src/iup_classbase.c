/** \file
 * \brief Ihandle Class Base Functions
 *
 * See Copyright Notice in iup.h
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>

#include "iup.h"

#include "iup_object.h"
#include "iup_drv.h"
#include "iup_drvfont.h"
#include "iup_str.h"
#include "iup_attrib.h"
#include "iup_assert.h"



void iupBaseSetCurrentSizeMethod(Ihandle* ih, int w, int h, int shrink)
{
  (void)shrink; /* shrink is only used by containers, usually is 0 */
                /* for non containers is always 1, so they always can be smaller than the natural size */

  /* if expand use the given size, else use the natural size */
  ih->currentwidth = (ih->expand & IUP_EXPAND_WIDTH)? w: ih->naturalwidth;
  ih->currentheight = (ih->expand & IUP_EXPAND_HEIGHT)? h: ih->naturalheight;
}

void iupBaseSetPositionMethod(Ihandle* ih, int x, int y)
{
  ih->x = x;
  ih->y = y;
}

char* iupBaseGetWidAttrib(Ihandle *ih)
{
  return (char*)ih->handle;
}

char* iupBaseNoGetAttrib(Ihandle *ih)
{
  (void)ih;
  return NULL;
}

int iupBaseNoSetAttrib(Ihandle *ih, const char* value)
{
  /* does nothing and do not allow to store in the environment. */
  (void)ih;
  (void)value;
  return 0;
}

void iupBaseUpdateSizeAttrib(Ihandle* ih)
{
  char* value = iupAttribGetStr(ih, "SIZE");
  if (!value)
    return;

  iupBaseSetSizeAttrib(ih, value);
}

int iupBaseSetSizeAttrib(Ihandle* ih, const char* value)
{
  if (!value)
  {
    ih->userwidth = 0;
    ih->userheight = 0;
  }
  else
  {
    /* if not specified, the value is 0 */
    int w = 0, h = 0;
    int charwidth, charheight;
    iupdrvFontGetCharSize(ih, &charwidth, &charheight);
    iupStrToIntInt(value, &w, &h, 'x');
    if (w < 0) w = 0;
    if (h < 0) h = 0;

    ih->userwidth = iupWIDTH2RASTER(w, charwidth);
    ih->userheight = iupHEIGHT2RASTER(h, charheight);
  }
  return 1;  /* always save in the hash table, so when FONT is changed SIZE can be updated */
}

char* iupBaseGetSizeAttrib(Ihandle* ih)
{
  char* str;
  int charwidth, charheight, width, height;

  if (ih->handle)
  {
    width = ih->currentwidth;
    height = ih->currentheight;
  }
  else
  {
    width = ih->userwidth;
    height = ih->userheight;
  }

  iupdrvFontGetCharSize(ih, &charwidth, &charheight);
  if (charwidth == 0 || charheight == 0)
    return NULL;  /* if font failed get from the hash table */

  str = iupStrGetMemory(50);
  sprintf(str, "%dx%d", iupRASTER2WIDTH(width, charwidth), 
                        iupRASTER2HEIGHT(height, charheight));
  return str;
}

int iupBaseSetRasterSizeAttrib(Ihandle* ih, const char* value)
{
  if (!value)
  {
    ih->userwidth = 0;
    ih->userheight = 0;
  }
  else
  {
    /* if not specified, the value is 0 */
    int w = 0, h = 0;
    iupStrToIntInt(value, &w, &h, 'x');
    if (w < 0) w = 0;
    if (h < 0) h = 0;
    ih->userwidth = w;
    ih->userheight = h;
  }
  iupAttribSetStr(ih, "SIZE", NULL); /* clear SIZE in hash table */
  return 0;
}

char* iupBaseGetRasterSizeAttrib(Ihandle* ih)
{
  char* str;
  int width, height;

  if (ih->handle)
  {
    width = ih->currentwidth;
    height = ih->currentheight;
  }
  else
  {
    width = ih->userwidth;
    height = ih->userheight;
  }

  if (!width && !height)
    return NULL;

  str = iupStrGetMemory(50);
  sprintf(str, "%dx%d", width, height);
  return str;
}

char* iupBaseGetCharSizeAttrib(Ihandle* ih)
{
  char* str;
  int charwidth, charheight;

  iupdrvFontGetCharSize(ih, &charwidth, &charheight);
  if (charwidth == 0 || charheight == 0)
    return NULL;

  str = iupStrGetMemory(50);
  sprintf(str, "%dx%d", charwidth, charheight);
  return str;
}

static char* iBaseGetPositionAttrib(Ihandle* ih)
{
  char* str = iupStrGetMemory(50);
  sprintf(str, "%d,%d", ih->x, ih->y);
  return str;
}

static int iBaseSetPositionAttrib(Ihandle* ih, const char* value)
{
  if (ih->floating)
    iupStrToIntInt(value, &ih->x, &ih->y, ',');
  return 0;
}

char* iupBaseGetActiveAttrib(Ihandle *ih)
{
  if (iupdrvIsActive(ih))
    return "YES";
  else
    return "NO";
}

static int iBaseNativeParentIsActive(Ihandle* ih)
{
  if (!ih->parent)
    return 1;

  if (ih->parent->iclass->nativetype == IUP_TYPEVOID)
    return iBaseNativeParentIsActive(ih->parent);
  else 
    return iupdrvIsActive(ih->parent);
}

int iupBaseSetActiveAttrib(Ihandle* ih, const char* value)
{
  if (iupStrBoolean(value))
  {
    if (iBaseNativeParentIsActive(ih))
      iupdrvSetActive(ih, 1);
  }
  else
    iupdrvSetActive(ih, 0);
  return 0;
}

char* iupBaseGetVisibleAttrib(Ihandle* ih)
{
  if (iupdrvIsVisible(ih))
    return "YES";
  else
    return "NO";
}

int iupBaseSetVisibleAttrib(Ihandle* ih, const char* value)
{
  iupdrvSetVisible(ih, iupStrBoolean(value));
  return 0;
}

int iupBaseGetScrollbar(Ihandle* ih)
{
  int sb = IUP_SB_NONE;  /* NO scrollbar by default */
  char* value = IupGetAttribute(ih, "SCROLLBAR"); /* Use IupGetAttribute for inheritance */
  if (value)
  {
    if (iupStrEqualNoCase(value, "YES"))
      sb = IUP_SB_HORIZ | IUP_SB_VERT;
    else if (iupStrEqualNoCase(value, "HORIZONTAL"))
      sb = IUP_SB_HORIZ;
    else if (iupStrEqualNoCase(value, "VERTICAL"))
      sb = IUP_SB_VERT;
  }
  return sb;
}

static Ihandle* iBaseFindChild(Ihandle* ih, const char* name)
{
  Ihandle* child = ih->firstchild;
  while (child)
  {
    char* child_name = iupAttribGetStr(child, "NAME");
    if (child_name && iupStrEqualNoCase(name, child_name))
      return child;

    if (child->firstchild)
    {
      Ihandle* c = iBaseFindChild(child, name);
      if (c) return c;
    }

    child = child->brother;
  }
  return NULL;
}

Ihandle* IupGetDialogChild(Ihandle* ih, const char* name)
{
  Ihandle *child, *dialog;
  char attrib[1024] = "_IUP_DIALOG_CHILD_";

  iupASSERT(iupObjectCheck(ih));
  if (!iupObjectCheck(ih))
    return NULL;

  if (!name)
    return NULL;

  dialog = IupGetDialog(ih);
  if (dialog) ih = dialog;
 
  strcat(attrib, name);
  child = (Ihandle*)iupAttribGetStr(ih, attrib);
  if (child) return child;

  if (ih->firstchild)
  {
    child = iBaseFindChild(ih, name);
    if (child) return child;
  }
  return NULL;
}

int iupBaseSetNameAttrib(Ihandle* ih, const char* value)
{
  Ihandle* dialog = IupGetDialog(ih);
  if (dialog)
  {
    char attrib[1024] = "_IUP_DIALOG_CHILD_";
    strcat(attrib, value);
    iupAttribSetStr(dialog, attrib, (char*)ih);
  }
  return 1;
}

static int iBaseSetFloatingAttrib(Ihandle* ih, const char* value)
{
  ih->floating = iupStrBoolean(value);
  return 0;
}

static char* iBaseGetFloatingAttrib(Ihandle* ih)
{
  if (ih->floating)
    return "YES";
  else
    return "NO";
}

static int iBaseSetExpandAttrib(Ihandle* ih, const char* value)
{
  if (iupStrEqualNoCase(value, "YES"))
    ih->expand = IUP_EXPAND_BOTH;
  else if (iupStrEqualNoCase(value, "HORIZONTAL"))
    ih->expand = IUP_EXPAND_WIDTH;
  else if (iupStrEqualNoCase(value, "VERTICAL"))
    ih->expand = IUP_EXPAND_HEIGHT;
  else
    ih->expand = IUP_EXPAND_NONE;
  return 0;
}

static char* iBaseGetExpandAttrib(Ihandle* ih)
{
  if (ih->expand & IUP_EXPAND_BOTH)
    return "YES";
  else if (ih->expand & IUP_EXPAND_WIDTH)
    return "HORIZONTAL";
  else if (ih->expand & IUP_EXPAND_HEIGHT)
    return "VERTICAL";
  else
    return "NO";
}

void iupBaseContainerUpdateExpand(Ihandle* ih)
{
  char *expand = iupAttribGetStrInherit(ih, "EXPAND");
  if (!expand)
    ih->expand = IUP_EXPAND_BOTH;  /* default for containers is YES */
  else
  {
    if (iupStrEqualNoCase(expand, "NO"))
      ih->expand = IUP_EXPAND_NONE;
    else if (iupStrEqualNoCase(expand, "HORIZONTAL"))
      ih->expand = IUP_EXPAND_WIDTH;
    else if (iupStrEqualNoCase(expand, "VERTICAL"))
      ih->expand = IUP_EXPAND_HEIGHT;
    else
      ih->expand = IUP_EXPAND_BOTH;  /* default for containers is YES */
  }
}

/* EXPAND is registered with IUP_NO_INHERIT because it is not inheritable,
   but it is inheritable for containers. 
   So if you set at a container it will not affect its children,
   but children that are containers will get the same value. */
char* iupBaseContainerGetExpandAttrib(Ihandle* ih)
{
  return iupAttribGetStrInherit(ih, "EXPAND");
}

void iupBaseRegisterCommonAttrib(Iclass* ic)
{
  iupClassRegisterAttribute(ic, "WID", iupBaseGetWidAttrib, iupBaseNoSetAttrib, NULL, IUP_MAPPED, IUP_NO_INHERIT);
  iupClassRegisterAttribute(ic, "NAME", NULL, iupBaseSetNameAttrib, NULL, IUP_NOT_MAPPED, IUP_NO_INHERIT);
  iupClassRegisterAttribute(ic, "FLOATING", iBaseGetFloatingAttrib, iBaseSetFloatingAttrib, NULL, IUP_NOT_MAPPED, IUP_NO_INHERIT);
  iupClassRegisterAttribute(ic, "EXPAND", iBaseGetExpandAttrib, iBaseSetExpandAttrib, NULL, IUP_NOT_MAPPED, IUP_NO_INHERIT);

  /* make sure everyone has the correct default value */
  iupClassRegisterAttribute(ic, "VISIBLE", NULL, NULL, "YES", IUP_NOT_MAPPED, IUP_INHERIT);
  iupClassRegisterAttribute(ic, "ACTIVE", NULL, NULL, "YES", IUP_NOT_MAPPED, IUP_INHERIT);

  iupClassRegisterAttribute(ic, "SIZE", iupBaseGetSizeAttrib, iupBaseSetSizeAttrib, NULL, IUP_NOT_MAPPED, IUP_NO_INHERIT);
  iupClassRegisterAttribute(ic, "RASTERSIZE", iupBaseGetRasterSizeAttrib, iupBaseSetRasterSizeAttrib, NULL, IUP_NOT_MAPPED, IUP_NO_INHERIT);
  iupClassRegisterAttribute(ic, "CHARSIZE", iupBaseGetCharSizeAttrib, iupBaseNoSetAttrib, NULL, IUP_NOT_MAPPED, IUP_NO_INHERIT);

  iupClassRegisterAttribute(ic, "POSITION", iBaseGetPositionAttrib, iBaseSetPositionAttrib, NULL, IUP_NOT_MAPPED, IUP_NO_INHERIT);

  iupClassRegisterAttribute(ic, "STANDARDFONT", NULL, iupdrvSetStandardFontAttrib, IupGetGlobal("DEFAULTFONT"), IUP_NOT_MAPPED, IUP_INHERIT);  /* use inheritance to retrieve standard fonts */
  iupClassRegisterAttribute(ic, "FONT", iupGetFontAttrib, iupSetFontAttrib, IupGetGlobal("DEFAULTFONT"), IUP_NOT_MAPPED, IUP_NO_INHERIT);

  iupClassRegisterAttribute(ic, "FONTSTYLE", iupGetFontStyleAttrib, iupSetFontStyleAttrib, NULL, IUP_NOT_MAPPED, IUP_NO_INHERIT);
  iupClassRegisterAttribute(ic, "FONTSIZE", iupGetFontSizeAttrib, iupSetFontSizeAttrib, NULL, IUP_NOT_MAPPED, IUP_NO_INHERIT);
  iupClassRegisterAttribute(ic, "FONTFACE", iupGetFontFaceAttrib, iupBaseNoSetAttrib, NULL, IUP_NOT_MAPPED, IUP_NO_INHERIT);

  iupdrvBaseRegisterCommonAttrib(ic);
}

void iupBaseRegisterVisualAttrib(Iclass* ic)
{
  /* VISIBLE inheritance comes from the native system */
  iupClassRegisterAttribute(ic, "VISIBLE", iupBaseGetVisibleAttrib, iupBaseSetVisibleAttrib, "YES", IUP_MAPPED, IUP_NO_INHERIT);
  iupClassRegisterAttribute(ic, "ACTIVE", iupBaseGetActiveAttrib, iupBaseSetActiveAttrib, "YES", IUP_MAPPED, IUP_INHERIT);

  iupClassRegisterAttribute(ic, "ZORDER", NULL, iupdrvBaseSetZorderAttrib, NULL, IUP_MAPPED, IUP_NO_INHERIT);
  iupClassRegisterAttribute(ic, "X", iupdrvBaseGetXAttrib, iupBaseNoSetAttrib, "0", IUP_MAPPED, IUP_NO_INHERIT);
  iupClassRegisterAttribute(ic, "Y", iupdrvBaseGetYAttrib, iupBaseNoSetAttrib, "0", IUP_MAPPED, IUP_NO_INHERIT);

  iupClassRegisterAttribute(ic, "TIP", NULL, iupdrvBaseSetTipAttrib, NULL, IUP_MAPPED, IUP_NO_INHERIT);
  iupClassRegisterAttribute(ic, "TIPVISIBLE", NULL, iupdrvBaseSetTipVisibleAttrib, NULL, IUP_MAPPED, IUP_NO_INHERIT);
  iupClassRegisterAttribute(ic, "TIPDELAY", NULL, NULL, "5000", IUP_NOT_MAPPED, IUP_INHERIT);          /* 5 seconds */
  iupClassRegisterAttribute(ic, "TIPBGCOLOR", NULL, NULL, "255 255 225", IUP_NOT_MAPPED, IUP_INHERIT); /* Light Yellow */
  iupClassRegisterAttribute(ic, "TIPFGCOLOR", NULL, NULL, "0 0 0", IUP_NOT_MAPPED, IUP_INHERIT);       /* black */
}
