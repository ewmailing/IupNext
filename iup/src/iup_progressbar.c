/** \file
 * \brief ProgressBar control
 *
 * See Copyright Notice in iup.h
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "iup.h"
#include "iupcbs.h"

#include "iup_object.h"
#include "iup_attrib.h"
#include "iup_str.h"
#include "iup_drv.h"
#include "iup_drvfont.h"
#include "iup_stdcontrols.h"
#include "iup_layout.h"
#include "iup_progressbar.h"


void iProgressBarCropValue(Ihandle* ih)
{
  if(ih->data->value > ih->data->vmax)
    ih->data->value = ih->data->vmax;
  else if(ih->data->value < ih->data->vmin)
    ih->data->value = ih->data->vmin;
}

char* iProgressBarGetValueAttrib(Ihandle* ih)
{
  char* value = iupStrGetMemory(30);
  sprintf(value, "%g", ih->data->value);
  return value;
}

char* iProgressBarGetDashedAttrib(Ihandle* ih)
{
  if(ih->data->dashed)
    return "YES";
  else 
    return "NO";
}

static int iProgressBarSetMinAttrib(Ihandle* ih, const char* value)
{
  ih->data->vmin = atof(value);
  iProgressBarCropValue(ih);
  return 1;
}

static int iProgressBarSetMaxAttrib(Ihandle* ih, const char* value)
{
  ih->data->vmax = atof(value);
  iProgressBarCropValue(ih);
  return 1;
}

static void iProgressBarComputeNaturalSizeMethod(Ihandle* ih)
{
  /* always initialize the natural size using the user size */
  ih->naturalwidth = ih->userwidth;
  ih->naturalheight = ih->userheight;

  /* if user size is not defined, then calculate the natural size */
  if (ih->naturalwidth <= 0 || ih->naturalheight <= 0)
  {
    /* progress bar natural size is 200x30 */

    /* only update the natural size if user size is not defined. */
    if (ih->naturalwidth <= 0) ih->naturalwidth = 200;
    if (ih->naturalheight <= 0) ih->naturalheight = 30;
  }
}

static int iProgressBarCreateMethod(Ihandle* ih, void **params)
{
  (void)params;

  ih->data = iupALLOCCTRLDATA();

  /* default values */
  ih->data->vmax      = 1;
  ih->data->dashed    = 0;

  return IUP_NOERROR;
}

Iclass* iupProgressBarGetClass(void)
{
  Iclass* ic = iupClassNew(NULL);

  ic->name = "progressbar";
  ic->format = NULL; /* no parameters */
  ic->nativetype  = IUP_TYPECONTROL;
  ic->childtype   = IUP_CHILDNONE;
  ic->is_interactive = 0;

  /* Class functions */
  ic->Create = iProgressBarCreateMethod;
  ic->ComputeNaturalSize = iProgressBarComputeNaturalSizeMethod;

  ic->SetCurrentSize = iupBaseSetCurrentSizeMethod;
  ic->SetPosition = iupBaseSetPositionMethod;

  ic->LayoutUpdate = iupdrvBaseLayoutUpdateMethod;
  ic->UnMap = iupdrvBaseUnMapMethod;

  /* Common */
  iupBaseRegisterCommonAttrib(ic);

  /* Visual */
  iupBaseRegisterVisualAttrib(ic);

  /* IupProgressBar only */
  iupClassRegisterAttribute(ic, "MIN", NULL, iProgressBarSetMinAttrib, "0", IUP_NOT_MAPPED, IUP_NO_INHERIT);
  iupClassRegisterAttribute(ic, "MAX", NULL, iProgressBarSetMaxAttrib, "1", IUP_NOT_MAPPED, IUP_NO_INHERIT);

  iupdrvProgressBarInitClass(ic);

  return ic;
}

Ihandle *IupProgressBar(void)
{
  return IupCreate("progressbar");
}
