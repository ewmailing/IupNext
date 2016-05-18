/** \file
 * \brief Canvas Control.
 *
 * See Copyright Notice in "iup.h"
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
#include "iup_canvas.h"

//iupdrvFontGetMultiLineStringSize(ih, title, &txt_width, &txt_height);
//iupImageGetInfo(imagename, &img_width, &img_height, NULL);
#if 0
IdrawCanvas* iupDrawCreateCanvas(Ihandle* ih);

void iupDrawKillCanvas(IdrawCanvas* dc);
void iupDrawFlush(IdrawCanvas* dc);

void iupDrawParentBackground(IdrawCanvas* dc);

void iupDrawLine(IdrawCanvas* dc, int x1, int y1, int x2, int y2, unsigned char r, unsigned char g, unsigned char b, int style);

void iupDrawRectangle(IdrawCanvas* dc, int x1, int y1, int x2, int y2, unsigned char r, unsigned char g, unsigned char b, int style);

void iupDrawArc(IdrawCanvas* dc, int x1, int y1, int x2, int y2, double a1, double a2, unsigned char r, unsigned char g, unsigned char b, int style);

void iupDrawPolygon(IdrawCanvas* dc, int* points, int count, unsigned char r, unsigned char g, unsigned char b, int style);

void iupDrawText(IdrawCanvas* dc, const char* text, int len, int x, int y, unsigned char r, unsigned char g, unsigned char b, const char* font);

void iupDrawImage(IdrawCanvas* dc, const char* name, int make_inactive, int x, int y);

void iupDrawSetClipRect(IdrawCanvas* dc, int x1, int y1, int x2, int y2);

void iupDrawResetClip(IdrawCanvas* dc);

void iupDrawSelectRect(IdrawCanvas* dc, int x, int y, int w, int h);

void iupDrawFocusRect(IdrawCanvas* dc, int x, int y, int w, int h);
#endif

void iupCanvasCalcScrollIntPos(double min, double max, double page, double pos, 
                                 int imin,   int imax,  int *ipage,  int *ipos)
{
  double range = max-min;
  int irange = imax-imin;
  double ratio = ((double)irange)/range;

  *ipage = (int)(page*ratio);
  if (*ipage > irange) *ipage = irange;
  if (*ipage < 1) *ipage = 1;

  if (ipos)
  {
    *ipos = (int)((pos-min)*ratio) + imin;
    if (*ipos < imin) *ipos = imin;
    if (*ipos > (imax - *ipage)) *ipos = imax - *ipage;
  }
}

void iupCanvasCalcScrollRealPos(double min, double max, double *pos, 
                                 int imin,   int imax,  int ipage,  int *ipos)
{
  double range = max-min;
  int irange = imax-imin;
  double ratio = ((double)irange)/range;

  if (*ipos < imin) *ipos = imin;
  if (*ipos > (imax - ipage)) *ipos = imax - ipage;

  *pos = min + ((double)(*ipos-imin))/ratio;
}

char* iupCanvasGetPosXAttrib(Ihandle* ih)
{
  return iupStrReturnFloat(ih->data->posx);
}

char* iupCanvasGetPosYAttrib(Ihandle* ih)
{
  return iupStrReturnFloat(ih->data->posy);
}

static int iCanvasCreateMethod(Ihandle* ih, void** params)
{
  if (params && params[0])
  {
    char* action = (char*)params[0];
    iupAttribSetStr(ih, "ACTION", action);
  }

  ih->data = iupALLOCCTRLDATA();

  /* default EXPAND is YES */
  ih->expand = IUP_EXPAND_BOTH;
  
  return IUP_NOERROR;
}

static void iCanvasComputeNaturalSizeMethod(Ihandle* ih, int *w, int *h, int *children_expand)
{
  int natural_w = 0, natural_h = 0;
  (void)children_expand; /* unset if not a container */

  /* canvas natural size is 1 character */
  iupdrvFontGetCharSize(ih, &natural_w, &natural_h);

  *w = natural_w;
  *h = natural_h;
}


/******************************************************************************/


Ihandle* IupCanvas(const char* action)
{
  void *params[2];
  params[0] = (void*)action;
  params[1] = NULL;
  return IupCreatev("canvas", params);
}

Iclass* iupCanvasNewClass(void)
{
  Iclass* ic = iupClassNew(NULL);

  ic->name = "canvas";
  ic->format = "a"; /* one ACTION callback name */
  ic->nativetype = IUP_TYPECANVAS;
  ic->childtype = IUP_CHILDNONE;
  ic->is_interactive = 1;

  /* Class functions */
  ic->New = iupCanvasNewClass;
  ic->Create = iCanvasCreateMethod;
  ic->ComputeNaturalSize = iCanvasComputeNaturalSizeMethod;

  ic->LayoutUpdate = iupdrvBaseLayoutUpdateMethod;
  ic->UnMap = iupdrvBaseUnMapMethod;

  /* Callbacks */
  iupClassRegisterCallback(ic, "RESIZE_CB", "ii");
  iupClassRegisterCallback(ic, "FOCUS_CB", "i");
  iupClassRegisterCallback(ic, "WOM_CB", "i");
  iupClassRegisterCallback(ic, "BUTTON_CB", "iiiis");
  iupClassRegisterCallback(ic, "MOTION_CB", "iis");
  iupClassRegisterCallback(ic, "KEYPRESS_CB", "ii");
  iupClassRegisterCallback(ic, "ACTION", "ff");
  iupClassRegisterCallback(ic, "SCROLL_CB", "iff");
  iupClassRegisterCallback(ic, "WHEEL_CB", "fiis");

  /* Common Callbacks */
  iupBaseRegisterCommonCallbacks(ic);

  /* Common */
  iupBaseRegisterCommonAttrib(ic);

  /* Change the default to YES */
  iupClassRegisterReplaceAttribDef(ic, "EXPAND", IUPAF_SAMEASSYSTEM, "YES");

  /* Visual */
  iupBaseRegisterVisualAttrib(ic);

  /* Drag&Drop */
  iupdrvRegisterDragDropAttrib(ic);

  /* IupCanvas only */
  iupClassRegisterAttribute(ic, "CURSOR", NULL, iupdrvBaseSetCursorAttrib, IUPAF_SAMEASSYSTEM, "ARROW", IUPAF_IHANDLENAME|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "XMIN", NULL, NULL, IUPAF_SAMEASSYSTEM, "0", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "XMAX", NULL, NULL, IUPAF_SAMEASSYSTEM, "1", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "YMIN", NULL, NULL, IUPAF_SAMEASSYSTEM, "0", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "YMAX", NULL, NULL, IUPAF_SAMEASSYSTEM, "1", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "LINEX", NULL, NULL, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "LINEY", NULL, NULL, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "YAUTOHIDE", NULL, NULL, IUPAF_SAMEASSYSTEM, "YES", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "XAUTOHIDE", NULL, NULL, IUPAF_SAMEASSYSTEM, "YES", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "XHIDDEN", NULL, NULL, NULL, NULL, IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "YHIDDEN", NULL, NULL, NULL, NULL, IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "BORDER", NULL, NULL, IUPAF_SAMEASSYSTEM, "YES", IUPAF_DEFAULT);
  iupClassRegisterAttribute(ic, "SCROLLBAR", NULL, NULL, NULL, NULL, IUPAF_NO_INHERIT);

  iupdrvCanvasInitClass(ic);

  return ic;
}
