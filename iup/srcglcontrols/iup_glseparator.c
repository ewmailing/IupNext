/** \file
 * \brief Separator Control.
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


static int iGLSeparatorIsVertical(Ihandle* ih)
{
  char* value = iupAttribGetStr(ih, "ORIENTATION");
  if (iupStrEqualNoCase(value, "HORIZONTAL"))
    return 0;
  return 1;
}

static int iGLSeparatorAction(Ihandle* ih)
{
  double x1, y1, x2, y2;
  int active = iupAttribGetInt(ih, "ACTIVE");
  char* color = iupAttribGetStr(ih, "BORDERCOLOR");
  float width = iupAttribGetFloat(ih, "BORDERWIDTH");

  if (iGLSeparatorIsVertical(ih))
  {
    y1 = 0;
    y2 = ih->currentheight-1;
    x1 = x2 = ih->currentwidth / 2;
  }
  else
  {
    x1 = 0;
    x2 = ih->currentwidth;
    y1 = y2 = ih->currentheight / 2;
  }

  iupGLDrawLine(ih, x1, y1, x2, y2, width, color, active);

  return IUP_DEFAULT;
}

static int iGLSeparatorCreateMethod(Ihandle* ih, void** params)
{
  IupSetCallback(ih, "GL_ACTION", iGLSeparatorAction);
  
  (void)params;
  return IUP_NOERROR;
}

static void iGLSeparatorComputeNaturalSizeMethod(Ihandle* ih, int *w, int *h, int *children_expand)
{
  int natural_w = 0,
      natural_h = 0;

  if (iGLSeparatorIsVertical(ih))
  {
    natural_w = 2;
    if (ih->userheight <= 0)
      ih->expand = IUP_EXPAND_HFREE;
  }
  else
  {
    natural_h = 2;
    if (ih->userwidth <= 0)
      ih->expand = IUP_EXPAND_WFREE;
  }

  *w = natural_w;
  *h = natural_h;

  (void)children_expand; /* unset if not a container */
}


/******************************************************************************/


Iclass* iupGLSeparatorNewClass(void)
{
  Iclass* ic = iupClassNew(iupRegisterFindClass("glsubcanvas"));

  ic->name = "glseparator";
  ic->format = ""; /* none */
  ic->nativetype = IUP_TYPEVOID;
  ic->childtype = IUP_CHILDNONE;
  ic->is_interactive = 0;

  /* Class functions */
  ic->New = iupGLSeparatorNewClass;
  ic->Create = iGLSeparatorCreateMethod;
  ic->ComputeNaturalSize = iGLSeparatorComputeNaturalSizeMethod;

  /* IupSeparator only */
  iupClassRegisterAttribute(ic, "ORIENTATION", NULL, NULL, IUPAF_SAMEASSYSTEM, "VERTICAL", IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);

  return ic;
}

Ihandle* IupGLSeparator(void)
{
  return IupCreate("glseparator");
}

