/** \file
 * \brief GLButton Control.
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


static int iGLButtonACTION(Ihandle* ih)
{
  char *image = iupAttribGet(ih, "IMAGE");
  char* title = iupAttribGet(ih, "TITLE");
  int active = iupAttribGetInt(ih, "ACTIVE");
  int pressed = iupAttribGetInt(ih, "PRESSED");
  int highlight = iupAttribGetInt(ih, "HIGHLIGHT");
  char* fgcolor = iupAttribGetStr(ih, "FGCOLOR");
  char* bgcolor = iupAttribGetStr(ih, "BGCOLOR");
  char* hlcolor = iupAttribGetStr(ih, "HLCOLOR");
  char* presscolor = iupAttribGetStr(ih, "PRESSCOLOR");
  char* bcolor = iupAttribGetStr(ih, "BORDERCOLOR");
  float bwidth = iupAttribGetFloat(ih, "BORDERWIDTH");
  int border = 0;

  if (pressed)
  {
    bgcolor = presscolor;
    border = 1;
  }
  else if (highlight)
  {
    bgcolor = hlcolor;
    border = 1;
  }

  /* draw border */
  if (border)
    iupGLDrawRect(ih, 0, ih->currentwidth - 1, 0, ih->currentheight - 1, bwidth, bcolor, active);

  /* draw background */
  iupGLDrawBox(ih, 1, ih->currentwidth - 2, 1, ih->currentheight - 2, bgcolor);

  iupGLIconDraw(ih, ih->currentwidth, ih->currentheight, image, title, fgcolor, active);

  return IUP_DEFAULT;
}

static int iGLButtonBUTTON_CB(Ihandle* ih, int button, int pressed, int x, int y, char* status)
{
  if (button == IUP_BUTTON1)
  {
    iupGLSubCanvasRestoreRedraw(ih);

    if (!pressed)
    {
      Icallback cb = IupGetCallback(ih, "ACTION");
      if (cb)
      {
        int ret = cb(ih);
        if (ret == IUP_CLOSE)
          IupExitLoop();
      }
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

static int iGLButtonCreateMethod(Ihandle* ih, void** params)
{
  IupSetCallback(ih, "GL_ACTION", iGLButtonACTION);
  IupSetCallback(ih, "GL_BUTTON_CB", (Icallback)iGLButtonBUTTON_CB);
  IupSetCallback(ih, "GL_LEAVEWINDOW_CB", iupGLSubCanvasRestoreRedraw);
  IupSetCallback(ih, "GL_ENTERWINDOW_CB", iupGLSubCanvasRestoreRedraw);

  (void)params; /* label create already parsed title */
  return IUP_NOERROR;
}

static void iGLButtonComputeNaturalSizeMethod(Ihandle* ih, int *w, int *h, int *children_expand)
{
  float width = iupAttribGetFloat(ih, "BORDERWIDTH");
  if (width == 0)
    return;

  /* add to the label natural size */
  *w += (int)(2*width);
  *h += (int)(2*width);

  (void)children_expand; /* unset if not a container */
}


/******************************************************************************/


Iclass* iupGLButtonNewClass(void)
{
  Iclass* ic = iupClassNew(iupRegisterFindClass("gllabel"));

  ic->name = "glbutton";
  ic->format = "s"; /* one string */
  ic->nativetype = IUP_TYPEVOID;
  ic->childtype = IUP_CHILDNONE;
  ic->is_interactive = 1;

  /* Class functions */
  ic->New = iupGLButtonNewClass;
  ic->Create = iGLButtonCreateMethod;
  ic->ComputeNaturalSize = iGLButtonComputeNaturalSizeMethod;

  return ic;
}

Ihandle* IupGLButton(const char* title)
{
  void *params[2];
  params[0] = (void*)title;
  params[1] = NULL;
  return IupCreatev("glbutton", params);
}

