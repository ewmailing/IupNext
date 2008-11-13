/** \file
 * \brief Abstract Layout Management
 *
 * See Copyright Notice in iup.h
 */

#include <stdlib.h>  
#include <stdio.h>  
#include <stdarg.h>  

#include "iup.h"

#include "iup_object.h"
#include "iup_drv.h"
#include "iup_attrib.h"
#include "iup_str.h"
#include "iup_layout.h"
#include "iup_assert.h" 

 
void IupNormalizeSizev(const char* value, Ihandle** ih_list)
{
  Ihandle* ih;
  int i, natural_maxwidth = 0, natural_maxheight = 0;
  enum{NORMALIZE_NONE, NORMALIZE_WIDTH, NORMALIZE_HEIGHT};
  int normalize = NORMALIZE_NONE;

  if (iupStrEqualNoCase(value, "HORIZONTAL"))
    normalize = NORMALIZE_WIDTH;
  else if (iupStrEqualNoCase(value, "VERTICAL"))
    normalize = NORMALIZE_HEIGHT;
  else if (iupStrEqualNoCase(value, "BOTH"))
    normalize = NORMALIZE_WIDTH|NORMALIZE_HEIGHT;
  else 
    return;

  for (i = 0; ih_list[i]; i++)
  {
    ih = ih_list[i];
    iupClassObjectComputeNaturalSize(ih);
    natural_maxwidth = iupMAX(natural_maxwidth, ih->naturalwidth);
    natural_maxheight = iupMAX(natural_maxheight, ih->naturalheight);
  }

  for (i = 0; ih_list[i]; i++)
  {
    ih = ih_list[i];
    if (normalize & NORMALIZE_WIDTH)
      ih->userwidth = natural_maxwidth;
    if (normalize & NORMALIZE_HEIGHT)
      ih->userheight = natural_maxheight;
  }
}

void IupNormalizeSize(const char* value, Ihandle* ih_first, ...)
{
  va_list arglist;
  Ihandle **ih_list;

  va_start(arglist, ih_first);
  ih_list = (Ihandle **)iupObjectGetParamList(ih_first, arglist);
  va_end(arglist);

  IupNormalizeSizev(value, ih_list);

  free(ih_list);
}

void IupRefresh(Ihandle* ih)
{
  Ihandle* dialog;

  iupASSERT(iupObjectCheck(ih));
  if (!iupObjectCheck(ih))
    return;

  dialog = IupGetDialog(ih);
  if (dialog)
  {
    iupLayoutCompute(dialog);
    iupLayoutUpdate(dialog);
  }
}

static void iLayoutDisplayUpdateChildren(Ihandle *ih)
{
  Ihandle* child;
  for (child = ih->firstchild; child; child = child->brother)
  {
    iLayoutDisplayUpdateChildren(child);

    if (child->handle && child->iclass->nativetype != IUP_TYPEVOID)
      iupdrvDisplayUpdate(child);
  }
}

void IupUpdate(Ihandle* ih)
{
  iupASSERT(iupObjectCheck(ih));
  if (!iupObjectCheck(ih))
    return;

  if (ih->handle && ih->iclass->nativetype != IUP_TYPEVOID)
    iupdrvDisplayUpdate(ih);
}

void IupUpdateChildren(Ihandle* ih)
{
  iupASSERT(iupObjectCheck(ih));
  if (!iupObjectCheck(ih))
    return;

  iLayoutDisplayUpdateChildren(ih);
}

void iupLayoutUpdate(Ihandle* ih)
{
  if (ih->handle)
  {
    Ihandle* child;

    /* update size and position of the native control */
    iupClassObjectLayoutUpdate(ih);

    /* update its children */
    for (child = ih->firstchild; child; child = child->brother)
      iupLayoutUpdate(child);
  }
}

void iupLayoutCompute(Ihandle* ih)
{
  int shrink = iupAttribGetInt(ih, "SHRINK");

  /* Compute the natural size for all elements in the dialog,   
     using the minimum visible size and the defined user size.
     The minimum visible size is the size where all the controls can display
     all their contents.
     The defined user size is used to increase the value of the minimum visible size for containers,
     for standard controls will replace the minimum visible size.
     So the native size will be the maximum value between 
     minimum visible size and defined user size.
     First calculate the native size for the children, then for the element.
     Also calculates the expand configuration for each element. */
  iupClassObjectComputeNaturalSize(ih);

  /* Set the current size (not reflected in the native element yet) based on
     the natural size and the expand configuration. 
     If shrink is 0 (default) the current size of containers can be only larger than the natural size,
     the result will depend on the EXPAND attribute.
     If shrink is 1 the containers can be resized to sizes smaller than the natural size.
     Now, will first calculate the current size of the element, then for the children. */
  iupClassObjectSetCurrentSize(ih, 0, 0, shrink);

  /* Now that the current size is known, set the position of the elements 
     relative to the parent. */ 
  iupClassObjectSetPosition(ih, 0, 0);
}
