/** \file
 * \brief GLText Control.
 *
 * See Copyright Notice in "iup.h"
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "iup.h"
#include "iupcbs.h"
#include "iupglcontrols.h"

#include "iup_object.h"
#include "iup_attrib.h"
#include "iup_str.h"
#include "iup_register.h"

#include "iup_glcontrols.h"
#include "iup_gldraw.h"
#include "iup_glicon.h"
#include "iup_glsubcanvas.h"


static int iGLTextACTION(Ihandle* ih)
{
  char* value = iupAttribGet(ih, "VALUE");
  int active = iupAttribGetInt(ih, "ACTIVE");
  int highlight = iupAttribGetInt(ih, "HIGHLIGHT");
  char* fgcolor = iupAttribGetStr(ih, "FGCOLOR");
  char* bgcolor = iupAttribGetStr(ih, "BGCOLOR");
  float bwidth = iupAttribGetFloat(ih, "BORDERWIDTH");
  int border_width = (int)ceil(bwidth);
  int draw_border = 0;

  if (highlight)
  {
    char* hlcolor = iupAttribGetStr(ih, "HLCOLOR");
    if (hlcolor)
      bgcolor = hlcolor;
    draw_border = 1;
  }

  /* draw border - can still be disabled setting bwidth=0 */
  if (draw_border)
  {
    char* bordercolor = iupAttribGetStr(ih, "BORDERCOLOR");
    iupGLDrawRect(ih, 0, ih->currentwidth - 1,
                  0, ih->currentheight - 1,
                  bwidth, bordercolor, active, 0);
  }

  /* draw background */
  iupGLDrawBox(ih, border_width, ih->currentwidth - 1 - border_width,
               border_width, ih->currentheight - 1 - border_width,
               bgcolor, 1);  /* always active */

  iupGLIconDraw(ih, border_width, border_width,
                ih->currentwidth - 2 * border_width, ih->currentheight - 2 * border_width,
                NULL, NULL, value, fgcolor, active);

  return IUP_DEFAULT;
}

static int iGLTextBUTTON_CB(Ihandle* ih, int button, int pressed, int x, int y, char* status)
{
  if (button == IUP_BUTTON1)
  {
    /* "PRESSED" was already updated */
    iupGLSubCanvasRedraw(ih);

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

  (void)x;
  (void)y;
  (void)status;
  return IUP_DEFAULT;
}

static int iGLTextCreateMethod(Ihandle* ih, void** params)
{
  IupSetCallback(ih, "GL_ACTION", iGLTextACTION);
  IupSetCallback(ih, "GL_BUTTON_CB", (Icallback)iGLTextBUTTON_CB);
  IupSetCallback(ih, "GL_LEAVEWINDOW_CB", iupGLSubCanvasRedraw);
  IupSetCallback(ih, "GL_ENTERWINDOW_CB", iupGLSubCanvasRedraw);

  (void)params; /* label create already parsed value */
  return IUP_NOERROR;
}

static void iGLTextComputeNaturalSizeMethod(Ihandle* ih, int *w, int *h, int *children_expand)
{
  int natural_w = 0,
    natural_h = 0,
    visiblecolumns = iupAttribGetInt(ih, "VISIBLECOLUMNS");
  float bwidth = iupAttribGetFloat(ih, "BORDERWIDTH");
  int border_width = (int)ceil(bwidth);

  /* Since the contents can be changed by the user, the size can not be dependent on it. */
  iupGLIconGetSize(ih, NULL, "WWWWWWWWWW", &natural_w, &natural_h);  /* one line height */
  natural_w = (visiblecolumns*natural_w) / 10;

  /* compute the borders space */
  *w += 2 * border_width;
  *h += 2 * border_width;

  *w = natural_w;
  *h = natural_h;

  (void)children_expand; /* unset if not a container */
}


/******************************************************************************/


Iclass* iupGLTextNewClass(void)
{
  Iclass* ic = iupClassNew(NULL);

  ic->name = "gltext";
  ic->format = "s"; /* one string */
  ic->nativetype = IUP_TYPEVOID;
  ic->childtype = IUP_CHILDNONE;
  ic->is_interactive = 0;

  /* Class functions */
  ic->New = iupGLTextNewClass;
  ic->Create = iGLTextCreateMethod;
  ic->ComputeNaturalSize = iGLTextComputeNaturalSizeMethod;

  iupClassRegisterCallback(ic, "VALUECHANGED_CB", "");

  iupClassRegisterAttribute(ic, "VALUE", NULL, NULL, NULL, NULL, IUPAF_NO_DEFAULTVALUE | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "FGCOLOR", NULL, NULL, "0 0 0", NULL, IUPAF_DEFAULT);  /* inheritable */
  iupClassRegisterAttribute(ic, "VISIBLECOLUMNS", NULL, NULL, IUPAF_SAMEASSYSTEM, "5", IUPAF_NO_INHERIT);

  return ic;
}

Ihandle* IupGLText(void)
{
  return IupCreate("gltext");
}

