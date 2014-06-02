/** \file
 * \brief GLVal control
 *
 * See Copyright Notice in "iup.h"
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
#include "iup_register.h"

#include "iup_glcontrols.h"


typedef struct _iGLVal
{
  double value;  /* min<=value<max */
  double vmin;
  double vmax;
} iGLVal;

static int iGLValACTION_CB(Ihandle* ih)
{
  iGLVal* val = (iGLVal*)iupAttribGet(ih, "_IUP_GLVAL");
  float bwidth = iupAttribGetFloat(ih, "BORDERWIDTH");
  char* bcolor = iupAttribGetStr(ih, "BORDERCOLOR");
  int active = iupAttribGetInt(ih, "ACTIVE");
  char* bgcolor = iupAttribGetStr(ih, "BGCOLOR");
  int border_width = (int)ceil(bwidth);
  int is_horizontal = iupStrEqualNoCase(iupAttribGetStr(ih, "ORIENTATION"), "HORIZONTAL");
  int x1, y1, x2, y2;

  if (is_horizontal)
  {
    x1 = 0;
    x2 = ih->currentwidth - 1;
    y1 = y2 = ih->currentheight / 2;
  }
  else
  {
    y1 = 0;
    y2 = ih->currentheight - 1;
    x1 = x2 = ih->currentwidth / 2;
  }

  /* draw center line */
  iupGLDrawLine(ih, x1, y1, x2, y2, bwidth, bcolor, active);

  /* draw handler */
  {
    int xmin = 0;
    int ymin = 0;
    int xmax = ih->currentwidth - 1;
    int ymax = ih->currentheight - 1;
    double percent = (val->value - val->vmin) / (val->vmax - val->vmin);

    {
      char* fgcolor = iupAttribGetStr(ih, "FGCOLOR");
      if (is_horizontal)
      {
        int xmid = xmin + iupRound((xmax - xmin + 1) * percent);
        iupGLDrawBox(ih, xmin, xmid, ymin, ymax, fgcolor);
      }
      else
      {
        int ymid = ymin + iupRound((ymax - ymin + 1) * percent);
        iupGLDrawBox(ih, xmin, xmax, ymid, ymax, fgcolor);
      }
    }
  }

  return IUP_DEFAULT;
}

static void iGLValCropValue(iGLVal* val)
{
  if(val->value>val->vmax)
    val->value = val->vmax;
  else if(val->value<val->vmin)
    val->value = val->vmin;
}

static int iGLValSetValueAttrib(Ihandle* ih, const char* value)
{
  iGLVal* val = (iGLVal*)iupAttribGet(ih, "_IUP_GLVAL");
  if (iupStrToDouble(value, &(val->value)))
    iGLValCropValue(val);
  return 0; /* do not store value in hash table */
}

static char* iGLValGetValueAttrib(Ihandle* ih)
{
  iGLVal* val = (iGLVal*)iupAttribGet(ih, "_IUP_GLVAL");
  return iupStrReturnDouble(val->value);
}

static int iGLValSetMinAttrib(Ihandle* ih, const char* value)
{
  iGLVal* val = (iGLVal*)iupAttribGet(ih, "_IUP_GLVAL");
  if (iupStrToDouble(value, &(val->vmin)))
    iGLValCropValue(val);
  return 0; /* do not store value in hash table */
}

static char* iGLValGetMinAttrib(Ihandle* ih)
{
  iGLVal* val = (iGLVal*)iupAttribGet(ih, "_IUP_GLVAL");
  return iupStrReturnDouble(val->vmin);
}

static int iGLValSetMaxAttrib(Ihandle* ih, const char* value)
{
  iGLVal* val = (iGLVal*)iupAttribGet(ih, "_IUP_GLVAL");
  if (iupStrToDouble(value, &(val->vmax)))
    iGLValCropValue(val);
  return 0; /* do not store value in hash table */
}

static char* iGLValGetMaxAttrib(Ihandle* ih)
{
  iGLVal* val = (iGLVal*)iupAttribGet(ih, "_IUP_GLVAL");
  return iupStrReturnDouble(val->vmax);
}

static void iGLValComputeNaturalSizeMethod(Ihandle* ih, int *w, int *h, int *children_expand)
{
  int natural_w = 0,
      natural_h = 0;
  int charwidth, charheight;
  int is_horizontal = iupStrEqualNoCase(iupAttribGetStr(ih, "ORIENTATION"), "HORIZONTAL");

  iupGLFontGetCharSize(ih, &charwidth, &charheight);

  if (is_horizontal)
  {
    natural_h = charheight;
    if (ih->userwidth <= 0)
      natural_w = 15 * charwidth;
  }
  else
  {
    natural_w = charheight;
    if (ih->userheight <= 0)
      natural_h = 15 * charwidth;
  }

  *w = natural_w;
  *h = natural_h;

  (void)children_expand; /* unset if not a container */
}

static int iGLValCreateMethod(Ihandle* ih, void **params)
{
  iGLVal* val = (iGLVal*)malloc(sizeof(iGLVal));
  iupAttribSet(ih, "_IUP_GLVAL", (char*)val);

  memset(val, 0, sizeof(iGLVal));

  /* default values */
  val->vmax = 1;

  IupSetCallback(ih, "GL_ACTION", (Icallback)iGLValACTION_CB);

  (void)params;
  return IUP_NOERROR;
}

static void iGLValDestroyMethod(Ihandle* ih)
{
  iGLVal* val = (iGLVal*)iupAttribGet(ih, "_IUP_GLVAL");
  free(val);
}

Iclass* iupGLValNewClass(void)
{
  Iclass* ic = iupClassNew(iupRegisterFindClass("glsubcanvas"));

  ic->name = "glval";
  ic->format = NULL; /* no parameters */
  ic->nativetype = IUP_TYPEVOID;
  ic->childtype   = IUP_CHILDNONE;
  ic->is_interactive = 1;

  /* Class functions */
  ic->New = iupGLValNewClass;
  ic->Create  = iGLValCreateMethod;
  ic->Destroy = iGLValDestroyMethod;
  ic->ComputeNaturalSize = iGLValComputeNaturalSizeMethod;

  /* Callbacks */
  iupClassRegisterCallback(ic, "VALUECHANGED_CB", "");

  /* IupGLVal only */
  iupClassRegisterAttribute(ic, "MIN", iGLValGetMinAttrib, iGLValSetMinAttrib, IUPAF_SAMEASSYSTEM, "0", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "MAX", iGLValGetMaxAttrib, iGLValSetMaxAttrib, IUPAF_SAMEASSYSTEM, "1", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "VALUE", iGLValGetValueAttrib, iGLValSetValueAttrib, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "ORIENTATION", NULL, NULL, IUPAF_SAMEASSYSTEM, "HORIZONTAL", IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "FGCOLOR", NULL, NULL, IUPAF_SAMEASSYSTEM, "200 225 245", IUPAF_DEFAULT);  /* inheritable */

  return ic;
}

Ihandle* IupGLVal(void)
{
  return IupCreate("glval");
}
