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
  char* fgcolor = iupAttribGetStr(ih, "FGCOLOR");
  int handler_size = iupAttribGetInt(ih, "HANDLERSIZE");
  int slider_size = iupAttribGetInt(ih, "SLIDERSIZE");
  int border_width = (int)ceil(bwidth);
  int is_horizontal = iupStrEqualNoCase(iupAttribGetStr(ih, "ORIENTATION"), "HORIZONTAL");
  int pressed = iupAttribGetInt(ih, "PRESSED");
  int highlight = iupAttribGetInt(ih, "HIGHLIGHT");
  double percent = (val->value - val->vmin) / (val->vmax - val->vmin);
  int x1, y1, x2, y2;

  if (is_horizontal)
  {
    if (handler_size == 0)
      handler_size = ih->currentheight / 2;
    x1 = handler_size / 2;
    x2 = ih->currentwidth - 1 - handler_size / 2;
    y1 = ih->currentheight / 2 - slider_size;
    y2 = ih->currentheight / 2 + slider_size;
  }
  else
  {
    if (handler_size == 0)
      handler_size = ih->currentwidth / 2;
    y1 = handler_size / 2;
    y2 = ih->currentheight - 1 - handler_size / 2;
    x1 = ih->currentwidth / 2 - slider_size;
    x2 = ih->currentwidth / 2 + slider_size;
  }

  /* draw slider border - can be disabled setting bwidth=0 */
  iupGLDrawRect(ih, x1, x2, y1, y2, bwidth, bcolor, active);

  /* draw slider background */
  iupGLDrawBox(ih, x1 + border_width, x2 - border_width, y1 + border_width, y2 - border_width, bgcolor);


  if (pressed)
  {
    char* presscolor = iupAttribGetStr(ih, "PRESSCOLOR");
    if (presscolor)
      fgcolor = presscolor;
  }
  else if (highlight)
  {
    char* hlcolor = iupAttribGetStr(ih, "HLCOLOR");
    if (hlcolor)
      fgcolor = hlcolor;
  }

  if (is_horizontal)
  {
    int xmid = x1 + iupRound((x2 - x1 + 1) * percent);

    x1 = xmid - handler_size / 2;
    x2 = xmid + handler_size / 2;
    y1 = 0;
    y2 = ih->currentheight-1;
  }
  else
  {
    int ymid = y1 + iupRound((y2 - y1 + 1) * percent);

    y1 = ymid - handler_size / 2;
    y2 = ymid + handler_size / 2;
    x1 = 0;
    x2 = ih->currentwidth - 1;
  }

  /* draw handler border - can still be disabled setting bwidth=0 */
  iupGLDrawRect(ih, x1, x2, y1, y2, bwidth, bcolor, active);

  /* draw handler foreground */
  iupGLDrawBox(ih, x1 + border_width, x2 - border_width, y1 + border_width, y2 - border_width, fgcolor);

  return IUP_DEFAULT;
}

static int iGLValMoveHandler(Ihandle* ih, int dx, int dy)
{
  return 0;
}

static int iGLValIsInsideHandler(Ihandle* ih, int x, int y)
{
  iGLVal* val = (iGLVal*)iupAttribGet(ih, "_IUP_GLVAL");
  int handler_size = iupAttribGetInt(ih, "HANDLERSIZE");
  int is_horizontal = iupStrEqualNoCase(iupAttribGetStr(ih, "ORIENTATION"), "HORIZONTAL");
  double percent = (val->value - val->vmin) / (val->vmax - val->vmin);
  int p, p1, p2, pmid;

  if (is_horizontal)
  {
    if (handler_size == 0)
      handler_size = ih->currentheight / 2;
    p1 = handler_size / 2;
    p2 = ih->currentwidth - 1 - handler_size / 2;
    p = x;
  }
  else
  {
    if (handler_size == 0)
      handler_size = ih->currentwidth / 2;
    p1 = handler_size / 2;
    p2 = ih->currentheight - 1 - handler_size / 2;
    p = y;
  }

  pmid = p1 + iupRound((p2 - p1 + 1) * percent);
  p1 = pmid - handler_size / 2;
  p2 = pmid + handler_size / 2;

  if (p < p1 || p > p2)
    return 0;
  else
    return 1;
}

static int iGLValBUTTON_CB(Ihandle* ih, int button, int pressed, int x, int y, char* status)
{
  if (button == IUP_BUTTON1)
  {
    if (pressed)
    {
      if (!iGLValIsInsideHandler(ih, x, y))
        iupAttribSet(ih, "PRESSED", NULL);
      else
      {
        iupAttribSetInt(ih, "_IUP_START_X", x);
        iupAttribSetInt(ih, "_IUP_START_Y", y);
      }
    }

    iupGLSubCanvasRestoreRedraw(ih);
  }
  else
  {
    Ihandle* gl_parent = (Ihandle*)iupAttribGet(ih, "GL_CANVAS");
    iupGLSubCanvasRestoreState(gl_parent);
  }

  (void)status;
  return IUP_DEFAULT;
}

static int iGLValMOTION_CB(Ihandle* ih, int x, int y, char* status)
{
  int pressed = iupAttribGetInt(ih, "PRESSED");

  Ihandle* gl_parent = (Ihandle*)iupAttribGet(ih, "GL_CANVAS");
  iupGLSubCanvasRestoreState(gl_parent);

  if (pressed && iGLValIsInsideHandler(ih, x, y))
  {
    int start_x = iupAttribGetInt(ih, "_IUP_START_X");
    int start_y = iupAttribGetInt(ih, "_IUP_START_Y");

    if (iGLValMoveHandler(ih, x - start_x, y - start_y))
    {
      Icallback cb = IupGetCallback(ih, "VALUECHANGED_CB");

      IupSetAttribute(gl_parent, "REDRAW", NULL);

      if (cb)
      {
        int ret = cb(ih);
        if (ret == IUP_CLOSE)
          IupExitLoop();
      }
    }
  }

  (void)status;
  return IUP_DEFAULT;
}

static int iGLValENTERWINDOW_CB(Ihandle* ih, int x, int y)
{
  if (!iGLValIsInsideHandler(ih, x, y))
    iupAttribSet(ih, "HIGHLIGHT", NULL);

  return iupGLSubCanvasRestoreRedraw(ih);
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
  IupSetCallback(ih, "GL_BUTTON_CB", (Icallback)iGLValBUTTON_CB);
  IupSetCallback(ih, "GL_MOTION_CB", (Icallback)iGLValMOTION_CB);
  IupSetCallback(ih, "GL_LEAVEWINDOW_CB", iupGLSubCanvasRestoreRedraw);
  IupSetCallback(ih, "GL_ENTERWINDOW_CB", (Icallback)iGLValENTERWINDOW_CB);

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
  iupClassRegisterAttribute(ic, "HLCOLOR", NULL, NULL, IUPAF_SAMEASSYSTEM, "190 210 230", IUPAF_DEFAULT);  /* inheritable */
  iupClassRegisterAttribute(ic, "HANDLERSIZE", NULL, NULL, IUPAF_SAMEASSYSTEM, "0", IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "SLIDERSIZE", NULL, NULL, IUPAF_SAMEASSYSTEM, "5", IUPAF_NO_INHERIT);

  return ic;
}

Ihandle* IupGLVal(void)
{
  return IupCreate("glval");
}
