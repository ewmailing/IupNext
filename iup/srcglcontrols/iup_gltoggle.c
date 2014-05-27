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


static int iGLToggleBUTTON_CB(Ihandle* ih, int button, int pressed, int x, int y, char* status)
{
  if (button == IUP_BUTTON1)
  {
    int selected = iupAttribGetInt(ih, "VALUE");

    if (!pressed)
    {
      if (selected)
      {
        iupAttribSet(ih, "VALUE", "OFF");
        selected = 0;
      }
      else
      {
        iupAttribSet(ih, "VALUE", "ON");
        selected = 1;
      }
    }

    iupGLSubCanvasRestoreRedraw(ih);

    if (!pressed)
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

  iupClassRegisterAttribute(ic, "VALUE", NULL, NULL, NULL, NULL, IUPAF_NO_INHERIT);
//  iupClassRegisterAttribute(ic, "RADIO", iToggleGetRadioAttrib, NULL, NULL, NULL, IUPAF_READONLY | IUPAF_NO_INHERIT);

  return ic;
}

Ihandle* IupGLToggle(const char* title)
{
  void *params[2];
  params[0] = (void*)title;
  params[1] = NULL;
  return IupCreatev("gltoggle", params);
}

