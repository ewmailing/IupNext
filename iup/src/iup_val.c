/** \file
 * \brief Valuator Control
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
#include "iupcontrols.h"

#include "iup_object.h"
#include "iup_attrib.h"
#include "iup_str.h"
#include "iup_drv.h"
#include "iup_stdcontrols.h"
#include "iup_layout.h"
#include "iup_val.h"


void iupValCropValue(Ihandle* ih)
{
  if (ih->data->val > ih->data->vmax) 
    ih->data->val = ih->data->vmax;
  else if (ih->data->val < ih->data->vmin) 
    ih->data->val = ih->data->vmin;
}

char* iupValGetShowTicksAttrib(Ihandle* ih)
{
  char* str = iupStrGetMemory(30);
  sprintf(str, "%d", ih->data->show_ticks);
  return str;
}

static int iValSetShowTicksAttrib(Ihandle* ih, const char* value)
{
  ih->data->show_ticks = atoi(value);
  return 0;
}

char* iupValGetValueAttrib(Ihandle* ih)
{
  char* str = iupStrGetMemory(30);
  sprintf(str, "%g", ih->data->val);
  return str;
}

char* iupValGetStepAttrib(Ihandle* ih)
{
  char* str = iupStrGetMemory(30);
  sprintf(str, "%g", ih->data->step);
  return str;
}

char* iupValGetPageStepAttrib(Ihandle* ih)
{
  char* str = iupStrGetMemory(30);
  sprintf(str, "%g", ih->data->pagestep);
  return str;
}

static int iValSetMaxAttrib(Ihandle* ih, const char* value)
{
  ih->data->vmax = atof(value);
  iupValCropValue(ih);
  return 0; /* do not store value in hash table */
}

static char* iValGetMaxAttrib(Ihandle* ih)
{
  char* str = iupStrGetMemory(30);
  sprintf(str, "%g", ih->data->vmax);
  return str;
}

static int iValSetMinAttrib(Ihandle* ih, const char* value)
{
  ih->data->vmin = atof(value);
  iupValCropValue(ih);
  return 0; /* do not store value in hash table */
}

static char* iValGetMinAttrib(Ihandle* ih)
{
  char* str = iupStrGetMemory(30);
  sprintf(str, "%g", ih->data->vmin);
  return str;
}

static int iValSetTypeAttrib(Ihandle* ih, const char *value)
{
  /* valid only before map */
  if (ih->handle)
    return 0;

  if (iupStrEqualNoCase(value, "VERTICAL"))
    ih->data->type = IVAL_VERTICAL;
  else /* "HORIZONTAL" */
    ih->data->type = IVAL_HORIZONTAL;

  return 0; /* do not store value in hash table */
}

static char* iValGetTypeAttrib(Ihandle* ih)
{
  if (ih->data->type == IVAL_HORIZONTAL)
    return "HORIZONTAL";
  else /* (ih->data->type == IVAL_VERTICAL) */
    return "VERTICAL";
}

static int iValSetInvertedAttrib(Ihandle* ih, const char *value)
{
  /* valid only before map */
  if (ih->handle)
    return 0;

  if (iupStrBoolean(value))
    ih->data->inverted = 1;
  else
    ih->data->inverted = 0;

  return 0; /* do not store value in hash table */
}

static char* iValGetInvertedAttrib(Ihandle* ih)
{
  if (ih->data->inverted)
    return "YES";
  else
    return "NO";
}

static void iValComputeNaturalSizeMethod(Ihandle* ih)
{
  /* always initialize the natural size using the user size */
  ih->naturalwidth = ih->userwidth;
  ih->naturalheight = ih->userheight;

  /* if user size is not defined, then calculate the natural size */
  if (ih->naturalwidth <= 0 || ih->naturalheight <= 0)
  {
    int min_w, min_h;
    /* val natural horizontal size is 100xMinH */
    /* val natural vertical size is MinWx100 */

    iupdrvValGetMinSize(ih, &min_w, &min_h);

    if (ih->data->type == IVAL_HORIZONTAL)
    {
      if (ih->naturalwidth <= 0)
        ih->naturalwidth = 100;
      if (ih->naturalheight <= 0)
        ih->naturalheight = min_h;
    }
    else
    {
      if (ih->naturalwidth <= 0)
        ih->naturalwidth = min_w;
      if (ih->naturalheight <= 0)
        ih->naturalheight = 100;
    }
  }
}

static int iValCreateMethod(Ihandle* ih, void **params)
{
  char* type = "HORIZONTAL";
  if (params && params[0])
    type = params[0];

  ih->data = iupALLOCCTRLDATA();

  /* default values */
  iValSetTypeAttrib(ih, type);

  if (ih->data->type == IVAL_VERTICAL)
    ih->data->inverted = 1;  /* default is YES when vertical */

  ih->data->vmax = 1.00;
  ih->data->step = 0.01;
  ih->data->pagestep = 0.10;

  return IUP_NOERROR; 
}

Iclass* iupValGetClass(void)
{
  Iclass* ic = iupClassNew(NULL);

  ic->name = "val";
  ic->format = "S"; /* one optional string */
  ic->nativetype = IUP_TYPECONTROL;
  ic->childtype = IUP_CHILDNONE;
  ic->is_interactive = 1;

  /* Class functions */
  ic->Create  = iValCreateMethod;
  ic->ComputeNaturalSize = iValComputeNaturalSizeMethod;

  ic->SetCurrentSize = iupBaseSetCurrentSizeMethod;
  ic->SetPosition = iupBaseSetPositionMethod;

  ic->LayoutUpdate = iupdrvBaseLayoutUpdateMethod;
  ic->UnMap = iupdrvBaseUnMapMethod;

  /* Common */
  iupBaseRegisterCommonAttrib(ic);

  /* Visual */
  iupBaseRegisterVisualAttrib(ic);

  /* IupVal only */
  iupClassRegisterAttribute(ic, "MAX", iValGetMaxAttrib, iValSetMaxAttrib, "1.0", IUP_NOT_MAPPED, IUP_INHERIT);
  iupClassRegisterAttribute(ic, "MIN", iValGetMinAttrib, iValSetMinAttrib, "0.0", IUP_NOT_MAPPED, IUP_INHERIT);
  iupClassRegisterAttribute(ic, "SHOWTICKS", iupValGetShowTicksAttrib, iValSetShowTicksAttrib, "0", IUP_NOT_MAPPED, IUP_INHERIT);
  iupClassRegisterAttribute(ic, "TYPE", iValGetTypeAttrib, iValSetTypeAttrib, "HORIZONTAL", IUP_NOT_MAPPED, IUP_NO_INHERIT);
  iupClassRegisterAttribute(ic, "INVERTED", iValGetInvertedAttrib, iValSetInvertedAttrib, NULL, IUP_NOT_MAPPED, IUP_NO_INHERIT);
  iupClassRegisterAttribute(ic, "CANFOCUS", NULL, NULL, "YES", IUP_MAPPED, IUP_NO_INHERIT);

  iupdrvValInitClass(ic);

  return ic;
}

Ihandle *IupVal(const char *type)
{
  void *params[2];
  params[0] = (void*)type;
  params[1] = NULL;
  return IupCreatev("val", params);
}
