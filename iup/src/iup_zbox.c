/** \file
 * \brief Zbox Control.
 *
 * See Copyright Notice in "iup.h"
 */

#include <stdio.h>
#include <stdlib.h>

#include "iup.h"
#include "iupcbs.h"

#include "iup_object.h"
#include "iup_attrib.h"
#include "iup_str.h"
#include "iup_drv.h"
#include "iup_drvfont.h"
#include "iup_stdcontrols.h"
#include "iup_layout.h"


enum{IZBOX_ALIGN_NORTH, IZBOX_ALIGN_SOUTH, IZBOX_ALIGN_WEST, IZBOX_ALIGN_EAST,
     IZBOX_ALIGN_NE, IZBOX_ALIGN_SE, IZBOX_ALIGN_NW, IZBOX_ALIGN_SW,
     IZBOX_ALIGN_ACENTER};

struct _IcontrolData 
{
  int alignment;
};

static int iZboxCreateMethod(Ihandle* ih, void** params)
{
  ih->data = iupALLOCCTRLDATA();

  ih->data->alignment = IZBOX_ALIGN_NW;

  if (params)
  {
    Ihandle** iparams = (Ihandle**)params;
    while (*iparams)
    {
      IupAppend(ih, *iparams);
      iparams++;
    }
  }

  return IUP_NOERROR;
}

static void iZboxChildAddedMethod(Ihandle* ih, Ihandle* child)
{
  if (!iupAttribGetStr(ih, "VALUE_HANDLE"))
  {
    IupSetAttribute(child, "VISIBLE", "YES");
    iupAttribSetStr(ih, "VALUE_HANDLE", (char*)child);
  }
  else
    IupSetAttribute(child, "VISIBLE", "NO");
}

static void iZboxChildRemovedMethod(Ihandle* ih, Ihandle* child)
{
  if (child == (Ihandle*)iupAttribGetStr(ih, "VALUE_HANDLE"))
  {
    /* reset to the first child, even if it is NULL */
    IupSetAttribute(ih->firstchild, "VISIBLE", "YES");
    iupAttribSetStr(ih, "VALUE_HANDLE", (char*)ih->firstchild);
  }
}

static int iZboxMapMethod(Ihandle* ih)
{
  ih->handle = (InativeHandle*)-1; /* fake value just to indicate that it is already mapped */
  return IUP_NOERROR;
}

static int iZboxSetAlignmentAttrib(Ihandle* ih, const char* value)
{
  if (iupStrEqualNoCase(value, "NORTH") || iupStrEqualNoCase(value, "ATOP"))
    ih->data->alignment = IZBOX_ALIGN_NORTH;
  else if (iupStrEqualNoCase(value, "SOUTH") || iupStrEqualNoCase(value, "ABOTTOM"))
    ih->data->alignment = IZBOX_ALIGN_SOUTH;
  else if (iupStrEqualNoCase(value, "WEST") || iupStrEqualNoCase(value, "ALEFT"))
    ih->data->alignment = IZBOX_ALIGN_WEST;
  else if (iupStrEqualNoCase(value, "EAST") || iupStrEqualNoCase(value, "ARIGHT"))
    ih->data->alignment = IZBOX_ALIGN_EAST;
  else if (iupStrEqualNoCase(value, "NE"))
    ih->data->alignment = IZBOX_ALIGN_NE;
  else if (iupStrEqualNoCase(value, "SE"))
    ih->data->alignment = IZBOX_ALIGN_SE;
  else if (iupStrEqualNoCase(value, "NW"))
    ih->data->alignment = IZBOX_ALIGN_NW;
  else if (iupStrEqualNoCase(value, "SW"))
    ih->data->alignment = IZBOX_ALIGN_SW;
  else if (iupStrEqualNoCase(value, "ACENTER"))
    ih->data->alignment = IZBOX_ALIGN_ACENTER;
  return 0;
}

static char* iZboxGetAlignmentAttrib(Ihandle* ih)
{
  static char* align2str[9] = {"NORTH", "SOUTH", "WEST", "EAST",
                               "NE", "SE", "NW", "SW",
                               "ACENTER"};
  return align2str[ih->data->alignment];
}

static int iZboxSetValueHandleAttrib(Ihandle* ih, const char* value)
{
  Ihandle* old_handle, *new_handle, *child;

  new_handle = (Ihandle*)value;
  if (!iupObjectCheck(new_handle))
    return 0;

  old_handle = (Ihandle*)iupAttribGetStr(ih, "VALUE_HANDLE");
  if (!iupObjectCheck(old_handle))
    old_handle = NULL;

  for (child = ih->firstchild; child; child = child->brother)
  {
    if (child == new_handle) /* found child */
    {
      if (old_handle && old_handle != new_handle)
        IupSetAttribute(old_handle, "VISIBLE", "NO");

      IupSetAttribute(new_handle, "VISIBLE", "YES");
      return 1;
    }
  }
 
  return 0;
}

static int iZboxSetValuePosAttrib(Ihandle* ih, const char* value)
{
  Ihandle* child;
  int pos, i;

  if (!iupStrToInt(value, &pos))
    return 0;

  for (i=0, child=ih->firstchild; child; child = child->brother, i++)
  {
    if (i == pos) /* found child */
    {
      if (iZboxSetValueHandleAttrib(ih, (char*)child))
        iupAttribSetStr(ih, "VALUE_HANDLE", (char*)child);
      return 0;
    }
  }
 
  return 0;
}

static char* iZboxGetValuePosAttrib(Ihandle* ih)
{
  Ihandle* child;
  int pos;

  Ihandle* curr_handle = (Ihandle*)iupAttribGetStr(ih, "VALUE_HANDLE");
  if (!iupObjectCheck(curr_handle))
    return NULL;

  for (pos=0, child = ih->firstchild; child; child = child->brother, pos++)
  {
    if (child == curr_handle) /* found child */
    {
      char *str = iupStrGetMemory(50);
      sprintf(str, "%d", pos);
      return str;
    }
  }

  return NULL;
}

static int iZboxSetValueAttrib(Ihandle* ih, const char* value)
{
  Ihandle *new_handle;

  if (!value)
    return 0;

  new_handle = IupGetHandle(value);
  if (!new_handle)
    return 0;

  if (iZboxSetValueHandleAttrib(ih, (char*)new_handle))
    iupAttribSetStr(ih, "VALUE_HANDLE", (char*)new_handle);

  return 0;
}

static char* iZboxGetValueAttrib(Ihandle* ih)
{
  Ihandle* child;
  int pos;

  Ihandle* curr_handle = (Ihandle*)iupAttribGetStr(ih, "VALUE_HANDLE");
  if (!iupObjectCheck(curr_handle))
    return NULL;

  for (pos=0, child = ih->firstchild; child; child = child->brother, pos++)
  {
    if (child == curr_handle) /* found child, just cheking */
      return IupGetName(curr_handle);
  }

  return NULL;
}

static int iZboxSetVisibleAttrib(Ihandle* ih, const char* value)
{
  Ihandle* child = (Ihandle*)iupAttribGetStr(ih, "VALUE_HANDLE");
  if (child)
    IupSetAttribute(child, "VISIBLE", (char*)value);
  return 0;
}

static void iZboxComputeNaturalSizeMethod(Ihandle* ih)
{
  iupBaseContainerUpdateExpand(ih);

  /* always initialize the natural size using the user size */
  ih->naturalwidth = ih->userwidth;
  ih->naturalheight = ih->userheight;

  if (ih->firstchild)
  {
    Ihandle* child;
    int children_expand, 
        children_naturalwidth, children_naturalheight;

    /* calculate total children natural size (even for hidden children) */
    children_expand = 0;
    children_naturalwidth = 0;
    children_naturalheight = 0;
    for (child = ih->firstchild; child; child = child->brother)
    {
      /* update child natural size first */
      iupClassObjectComputeNaturalSize(child);

      if (!child->floating)
      {
        children_expand |= child->expand;
        children_naturalwidth = iupMAX(children_naturalwidth, child->naturalwidth);
        children_naturalheight = iupMAX(children_naturalheight, child->naturalheight);
      }
    }

    ih->expand &= children_expand; /* compose but only expand where the box can expand */
    ih->naturalwidth = iupMAX(ih->naturalwidth, children_naturalwidth);
    ih->naturalheight = iupMAX(ih->naturalheight, children_naturalheight);
  }
}

static void iZboxSetCurrentSizeMethod(Ihandle* ih, int w, int h, int shrink)
{
  Ihandle* child;

  iupBaseContainerSetCurrentSizeMethod(ih, w, h, shrink);

  if (!ih->firstchild)
    return;

  /* update children */

  for (child = ih->firstchild; child; child = child->brother)
  {
    if (!child->floating)
      iupClassObjectSetCurrentSize(child, ih->currentwidth, ih->currentheight, shrink);
  }
}

static void iZboxSetPositionMethod(Ihandle* ih, int x, int y)
{
  int dx = 0, dy = 0;
  Ihandle* child;

  iupBaseSetPositionMethod(ih, x, y);

  for (child = ih->firstchild; child; child = child->brother)
  {
    if (!child->floating)
    {
      switch (ih->data->alignment)
      {
      case IZBOX_ALIGN_ACENTER:
        dx=(ih->currentwidth-child->currentwidth)/2;
        dy=(ih->currentheight-child->currentheight)/2;
        break;
      case IZBOX_ALIGN_NORTH:
        dx=(ih->currentwidth-child->currentwidth)/2;
        dy=0;
        break;
      case IZBOX_ALIGN_SOUTH:
        dx=(ih->currentwidth-child->currentwidth)/2;
        dy=ih->currentheight-child->currentheight;
        break;
      case IZBOX_ALIGN_WEST:
        dx=0;
        dy=(ih->currentheight-child->currentheight)/2;
        break;
      case IZBOX_ALIGN_EAST:
        dx=ih->currentwidth-child->currentwidth;
        dy=(ih->currentheight-child->currentheight)/2;
        break;
      case IZBOX_ALIGN_NE:
        dx=ih->currentwidth-child->currentwidth;
        dy=0;
        break;
      case IZBOX_ALIGN_SE:
        dx=ih->currentwidth-child->currentwidth;
        dy=ih->currentheight-child->currentheight;
        break;
      case IZBOX_ALIGN_NW:
        dx=0;
        dy=0;
        break;
      case IZBOX_ALIGN_SW:
        dx=0;
        dy=ih->currentheight-child->currentheight;
        break;
      }
                     
      /* update child */
      iupClassObjectSetPosition(child, x+dx, y+dy);
    }
  }
}


/******************************************************************************/


Ihandle *IupZboxv(Ihandle **children)
{
  return IupCreatev("zbox", (void**)children);
}

Ihandle *IupZbox(Ihandle* child, ...)
{
  Ihandle **children;
  Ihandle *ih;

  va_list arglist;
  va_start(arglist, child);
  children = (Ihandle **)iupObjectGetParamList(child, arglist);
  va_end(arglist);

  ih = IupCreatev("zbox", (void**)children);
  free(children);

  return ih;
}

Iclass* iupZboxGetClass(void)
{
  Iclass* ic = iupClassNew(NULL);

  ic->name = "zbox";
  ic->format = "g"; /* array of Ihandle */
  ic->nativetype = IUP_TYPEVOID;
  ic->childtype = IUP_CHILDMANY;
  ic->is_interactive = 0;

  /* Class functions */
  ic->Create = iZboxCreateMethod;
  ic->Map = iZboxMapMethod;
  ic->ChildAdded = iZboxChildAddedMethod;
  ic->ChildRemoved = iZboxChildRemovedMethod;

  ic->ComputeNaturalSize = iZboxComputeNaturalSizeMethod;
  ic->SetCurrentSize = iZboxSetCurrentSizeMethod;
  ic->SetPosition = iZboxSetPositionMethod;

  /* Common */
  iupBaseRegisterCommonAttrib(ic);

  /* Base Container */
  iupClassRegisterAttribute(ic, "EXPAND", iupBaseContainerGetExpandAttrib, NULL, "YES", IUP_NOT_MAPPED, IUP_NO_INHERIT);
  iupClassRegisterAttribute(ic, "CLIENTSIZE", iupBaseGetRasterSizeAttrib, iupBaseNoSetAttrib, NULL, IUP_NOT_MAPPED, IUP_NO_INHERIT);

  /* Zbox only */
  iupClassRegisterAttribute(ic, "ALIGNMENT", iZboxGetAlignmentAttrib, iZboxSetAlignmentAttrib, "NW", IUP_NOT_MAPPED, IUP_NO_INHERIT);
  iupClassRegisterAttribute(ic, "VALUE", iZboxGetValueAttrib, iZboxSetValueAttrib, NULL, IUP_NOT_MAPPED, IUP_NO_INHERIT);
  iupClassRegisterAttribute(ic, "VALUEPOS", iZboxGetValuePosAttrib, iZboxSetValuePosAttrib, NULL, IUP_NOT_MAPPED, IUP_NO_INHERIT);
  iupClassRegisterAttribute(ic, "VALUE_HANDLE", NULL, iZboxSetValueHandleAttrib, NULL, IUP_NOT_MAPPED, IUP_NO_INHERIT);

  /* Intercept VISIBLE since ZBOX works showing and hidding its children */
  iupClassRegisterAttribute(ic, "VISIBLE", NULL, iZboxSetVisibleAttrib, NULL, IUP_NOT_MAPPED, IUP_NO_INHERIT);

  return ic;
}
