/** \file
 * \brief GLToggle Control.
 *
 * See Copyright Notice in "iup.h"
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "iup.h"
#include "iupcbs.h"

#include "iup_object.h"
#include "iup_attrib.h"
#include "iup_str.h"
#include "iup_register.h"

#include "iup_glcontrols.h"

/* from IupRadio implementation */
Ihandle *iupRadioFindToggleParent(Ihandle* ih_toggle);

static void iGLToggleNotify(Ihandle* ih, int selected)
{
  IFni cb = (IFni)IupGetCallback(ih, "ACTION");
  if (cb)
  {
    int ret = cb(ih, selected);
    if (ret == IUP_CLOSE)
      IupExitLoop();
  }

  if (iupObjectCheck(ih))
    iupBaseCallValueChangedCb(ih);
}

static int iGLToggleBUTTON_CB(Ihandle* ih, int button, int pressed, int x, int y, char* status)
{
  if (button == IUP_BUTTON1)
  {
    Ihandle* radio = iupRadioFindToggleParent(ih);
    int selected = iupAttribGetInt(ih, "VALUE");
    Ihandle* last_tg = NULL;

    if (!pressed)
    {
      if (selected)  /* was ON */
      {
        if (!radio)
        {
          iupAttribSet(ih, "VALUE", "OFF");
          selected = 0;
        }
        else
          last_tg = ih;  /* to avoid the callback call */
      }
      else  /* was OFF */
      {
        if (radio)
        {
          last_tg = (Ihandle*)iupAttribGet(radio, "_IUP_GLTOGGLE_LASTRADIO");
          if (iupObjectCheck(last_tg) && last_tg != ih)
            iupAttribSet(last_tg, "VALUE", "OFF");
          else
            last_tg = NULL;

          iupAttribSet(radio, "_IUP_GLTOGGLE_LASTRADIO", (char*)ih);
        }

        iupAttribSet(ih, "VALUE", "ON");
        selected = 1;
      }
    }

    iupGLSubCanvasRestoreRedraw(ih);

    if (!pressed)
    {
      if (last_tg && ih != last_tg)
        iGLToggleNotify(last_tg, 0);

      if (!radio || ih != last_tg)
        iGLToggleNotify(ih, selected);
    }
  }
  else
  {
    Ihandle* gl_parent = (Ihandle*)iupAttribGet(ih, "GL_CANVAS");
    iupGLSubCanvasRestoreState(gl_parent);
  }

  (void)x;
  (void)y;
  (void)status;
  return IUP_DEFAULT;
}

static int iGLToggleSetValueAttrib(Ihandle* ih, const char* value)
{
  Ihandle* radio = iupRadioFindToggleParent(ih);
  if (radio)
  {
    if (iupStrBoolean(value))
    {
      Ihandle* last_tg = (Ihandle*)iupAttribGet(radio, "_IUP_GLTOGGLE_LASTRADIO");
      if (iupObjectCheck(last_tg) && last_tg != ih)
        iupAttribSet(last_tg, "VALUE", "OFF");

      iupAttribSet(radio, "_IUP_GLTOGGLE_LASTRADIO", (char*)ih);
    }
    else
      return 0;  /* does nothing */
  }

  return 1;
}

static char* iGLToggleGetValueAttrib(Ihandle* ih)
{
  Ihandle* radio = iupRadioFindToggleParent(ih);
  if (radio)
  {
    Ihandle* last_tg = (Ihandle*)iupAttribGet(radio, "_IUP_GLTOGGLE_LASTRADIO");
    if (!last_tg)
    {
      iupAttribSet(ih, "VALUE", "ON");
      iupAttribSet(radio, "_IUP_GLTOGGLE_LASTRADIO", (char*)ih);
    }
  }

  return NULL;
}

static char* iGLToggleGetRadioAttrib(Ihandle* ih)
{
  Ihandle* radio = iupRadioFindToggleParent(ih);
  return iupStrReturnBoolean(radio!=NULL);
}

static int iGLToggleCreateMethod(Ihandle* ih, void** params)
{
  IupSetCallback(ih, "GL_BUTTON_CB", (Icallback)iGLToggleBUTTON_CB);

  (void)params; /* button create already parsed title */
  return IUP_NOERROR;
}



/******************************************************************************/


Iclass* iupGLToggleNewClass(void)
{
  Iclass* ic = iupClassNew(iupRegisterFindClass("glbutton"));

  ic->name = "gltoggle";
  ic->format = "s"; /* one string */
  ic->nativetype = IUP_TYPEVOID;
  ic->childtype = IUP_CHILDNONE;
  ic->is_interactive = 1;

  /* Class functions */
  ic->New = iupGLToggleNewClass;
  ic->Create = iGLToggleCreateMethod;

  iupClassRegisterCallback(ic, "ACTION", "i");
  iupClassRegisterCallback(ic, "VALUECHANGED_CB", "");

  iupClassRegisterAttribute(ic, "VALUE", iGLToggleGetValueAttrib, iGLToggleSetValueAttrib, NULL, NULL, IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "RADIO", iGLToggleGetRadioAttrib, NULL, NULL, NULL, IUPAF_READONLY | IUPAF_NO_INHERIT);

  return ic;
}

Ihandle* IupGLToggle(const char* title)
{
  void *params[2];
  params[0] = (void*)title;
  params[1] = NULL;
  return IupCreatev("gltoggle", params);
}

