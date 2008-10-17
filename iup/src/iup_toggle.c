/** \file
 * \brief Toggle Control.
 *
 * See Copyright Notice in iup.ih
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
#include "iup_toggle.h"
#include "iup_image.h"


static char* iToggleGetRadioAttrib(Ihandle* ih)
{
  if (ih->data->radio)
    return "YES";
  else
    return "NO";
}

char* iupToggleGetPaddingAttrib(Ihandle* ih)
{
  char *str = iupStrGetMemory(50);
  sprintf(str, "%dx%d", ih->data->horiz_padding, ih->data->vert_padding);
  return str;
}

static int iToggleCreateMethod(Ihandle* ih, void** params)
{
  if (params)
  {
    if (params[0]) iupAttribStoreStr(ih, "TITLE", (char*)(params[0]));
    if (params[1]) iupAttribStoreStr(ih, "ACTION", (char*)(params[1]));
  }
  ih->data = iupALLOCCTRLDATA();
  return IUP_NOERROR;
}

static void iToggleComputeNaturalSizeMethod(Ihandle* ih)
{
  /* always initialize the natural size using the user size */
  ih->naturalwidth = ih->userwidth;
  ih->naturalheight = ih->userheight;

  /* if user size is not defined, then calculate the natural size */
  if (ih->naturalwidth <= 0 || ih->naturalheight <= 0)
  {
    int natural_w = 0, 
        natural_h = 0,
        type = ih->data->type;

    if (!ih->handle)
    {
      /* if not mapped must initialize the internal values */
      char* value = iupAttribGetStr(ih, "IMAGE");
      if (value)
        type = IUP_TOGGLE_IMAGE;
      else
        type = IUP_TOGGLE_TEXT;
    }

    if (type == IUP_TOGGLE_IMAGE)
    {
      iupImageGetInfo(iupAttribGetStr(ih, "IMAGE"), &natural_w, &natural_h, NULL);

      /* even when IMPRESS is set, must compute the borders space */
      iupdrvButtonAddBorders(&natural_w, &natural_h);

      natural_w += 2*ih->data->horiz_padding;
      natural_h += 2*ih->data->vert_padding;
    }
    else /* IUP_TOGGLE_TEXT */
    {
      /* must use IupGetAttribute to check from the native implementation */
      char* title = IupGetAttribute(ih, "TITLE");
      char* str = iupStrProcessMnemonic(title, NULL, 0);   /* remove & */
      iupdrvFontGetMultiLineStringSize(ih, str, &natural_w, &natural_h);
      if (str!=title) free(str);

      iupdrvToggleAddCheckBox(&natural_w, &natural_h);
    }

    /* only update the natural size if user size is not defined. */
    if (ih->naturalwidth <= 0) ih->naturalwidth = natural_w;
    if (ih->naturalheight <= 0) ih->naturalheight = natural_h;
  }
}


/******************************************************************************/


Ihandle* IupToggle(const char* title, const char* action)
{
  void *params[3];
  params[0] = (void*)title;
  params[1] = (void*)action;
  params[2] = NULL;
  return IupCreatev("toggle", params);
}

Iclass* iupToggleGetClass(void)
{
  Iclass* ic = iupClassNew(NULL);

  ic->name = "toggle";
  ic->format = "SS"; /* two optional strings */
  ic->nativetype = IUP_TYPECONTROL;
  ic->childtype = IUP_CHILDNONE;
  ic->is_interactive = 1;

  /* Class functions */
  ic->Create = iToggleCreateMethod;
  ic->ComputeNaturalSize = iToggleComputeNaturalSizeMethod;

  ic->SetCurrentSize = iupBaseSetCurrentSizeMethod;
  ic->SetPosition = iupBaseSetPositionMethod;

  ic->LayoutUpdate = iupdrvBaseLayoutUpdateMethod;
  ic->UnMap = iupdrvBaseUnMapMethod;

  /* Callbacks */
  iupClassRegisterCallback(ic, "ACTION", "i");

  /* Common */
  iupBaseRegisterCommonAttrib(ic);

  /* Visual */
  iupBaseRegisterVisualAttrib(ic);

  iupClassRegisterAttribute(ic, "RADIO", iToggleGetRadioAttrib, iupBaseNoSetAttrib, NULL, IUP_MAPPED, IUP_NO_INHERIT);
  iupClassRegisterAttribute(ic, "CANFOCUS", NULL, NULL, "YES", IUP_MAPPED, IUP_NO_INHERIT);

  iupdrvToggleInitClass(ic);

  return ic;
}
