/** \file
 * \brief GLProgressBar control
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

#define IGAUGE_DEFAULTCOLOR "64 96 192"
#define IGAUGE_DEFAULTSIZE  "120x14"

#define IGAUGE_GAP     3
#define IGAUGE_BLOCKS 20


typedef struct _iGLProgressBar
{
  int show_text;
  int horiz_padding, vert_padding;  /* internal margin */

  double value;  /* min<=value<max */
  double vmin;
  double vmax;
} iGLProgressBar;

#if 0
static void iGLProgressBarDrawText(Ihandle* ih, int xmid)
{
  int x, y, xmin, xmax, ymin, ymax;
  char* text = pb->text;
  char buffer[30];

  IupCdSetFont(ih, pb->cddbuffer, IupGetAttribute(ih, "FONT"));
  cdCanvasTextAlignment(pb->cddbuffer, CD_CENTER);
  cdCanvasBackOpacity(pb->cddbuffer, CD_TRANSPARENT);

  x = (int)(0.5 * pb->w);
  y = (int)(0.5 * pb->h);

  if (text == NULL)
  {
    sprintf(buffer, "%.1f%%", 100 * (pb->value - pb->vmin) / (pb->vmax - pb->vmin));
    text = buffer;
  }

  cdCanvasGetTextBox(pb->cddbuffer, x, y, text, &xmin, &xmax, &ymin, &ymax);

  if(xmid < xmin)
  {
    cdCanvasForeground(pb->cddbuffer, pb->fgcolor);
    cdCanvasText(pb->cddbuffer, x, y, text);
  }
  else if(xmid > xmax)
  {
    cdCanvasForeground(pb->cddbuffer, pb->bgcolor);
    cdCanvasText(pb->cddbuffer, x, y, text);
  }
  else
  {
    cdCanvasClip(pb->cddbuffer, CD_CLIPAREA);
    cdCanvasClipArea(pb->cddbuffer, xmin, xmid, ymin, ymax);
    cdCanvasForeground(pb->cddbuffer, pb->bgcolor);
    cdCanvasText(pb->cddbuffer, x, y, text);

    cdCanvasClipArea(pb->cddbuffer, xmid, xmax, ymin, ymax);
    cdCanvasForeground(pb->cddbuffer, pb->fgcolor);
    cdCanvasText(pb->cddbuffer, x, y, text);
    cdCanvasClip(pb->cddbuffer, CD_CLIPOFF);
  }
}

static int iGLProgressBarACTION_CB(Ihandle* ih)
{
  int border = 3;  /* includes the pixel used to draw the 3D border */
  int xstart = pb->horiz_padding+border;
  int ystart = pb->vert_padding+border;
  int xend   = pb->w-1 - (pb->horiz_padding+border);
  int yend   = pb->h-1 - (pb->vert_padding+border);

  cdCanvasBackground(pb->cddbuffer, pb->bgcolor);
  cdCanvasClear(pb->cddbuffer);

  cdIupDrawSunkenRect(pb->cddbuffer, 0, 0, pb->w-1, pb->h-1,
                        pb->light_shadow, pb->mid_shadow, pb->dark_shadow);

  cdCanvasForeground(pb->cddbuffer, pb->fgcolor);

  if (pb->dashed)
  {
    float step = (xend - xstart + 1) / (float)IGAUGE_BLOCKS;
    float boxw = step - IGAUGE_GAP;
    float vx   = (float)((xend-xstart + 1) * (pb->value - pb->vmin) / (pb->vmax - pb->vmin));
    int intvx  = (int)(100 * vx);
    float i = 0;

    if(pb->value == pb->vmin)
      return;

    while(iupRound(100*(i + boxw)) <= intvx)
    {
      cdCanvasBox(pb->cddbuffer, xstart + iupRound(i),
             xstart + iupRound(i + boxw) - 1, ystart, yend);
      i += step;
    }
  }
  else
  {
    int xmid = xstart + iupRound((xend-xstart + 1) * (pb->value - pb->vmin) / (pb->vmax - pb->vmin));

    if(pb->value != pb->vmin)
      cdCanvasBox(pb->cddbuffer, xstart, xmid, ystart, yend );

    if(pb->show_text)
      iGLProgressBarDrawText(ih, xmid);
  }
}
#endif

static int iGLProgressBarACTION_CB(Ihandle* ih)
{
  float bwidth = iupAttribGetFloat(ih, "BORDERWIDTH");
  char* bcolor = iupAttribGetStr(ih, "BORDERCOLOR");
  int active = iupAttribGetInt(ih, "ACTIVE");

  iupGLDrawRect(ih, 0, ih->currentwidth - 1, 0, ih->currentheight - 1, bwidth, bcolor, active);

  return IUP_DEFAULT;
}

static void iGLProgressBarCropValue(iGLProgressBar* pb)
{
  if(pb->value>pb->vmax)
    pb->value = pb->vmax;
  else if(pb->value<pb->vmin)
    pb->value = pb->vmin;
}

static int iGLProgressBarSetValueAttrib(Ihandle* ih, const char* value)
{
  iGLProgressBar* pb = (iGLProgressBar*)iupAttribGet(ih, "_IUP_GLPROGRESSBAR");
  if (iupStrToDouble(value, &(pb->value)))
    iGLProgressBarCropValue(pb);
  return 0; /* do not store value in hash table */
}

static char* iGLProgressBarGetValueAttrib(Ihandle* ih)
{
  iGLProgressBar* pb = (iGLProgressBar*)iupAttribGet(ih, "_IUP_GLPROGRESSBAR");
  return iupStrReturnDouble(pb->value);
}

static int iGLProgressBarSetMinAttrib(Ihandle* ih, const char* value)
{
  iGLProgressBar* pb = (iGLProgressBar*)iupAttribGet(ih, "_IUP_GLPROGRESSBAR");
  if (iupStrToDouble(value, &(pb->vmin)))
    iGLProgressBarCropValue(pb);
  return 0; /* do not store value in hash table */
}

static char* iGLProgressBarGetMinAttrib(Ihandle* ih)
{
  iGLProgressBar* pb = (iGLProgressBar*)iupAttribGet(ih, "_IUP_GLPROGRESSBAR");
  return iupStrReturnDouble(pb->vmin);
}

static int iGLProgressBarSetMaxAttrib(Ihandle* ih, const char* value)
{
  iGLProgressBar* pb = (iGLProgressBar*)iupAttribGet(ih, "_IUP_GLPROGRESSBAR");
  if (iupStrToDouble(value, &(pb->vmax)))
    iGLProgressBarCropValue(pb);
  return 0; /* do not store value in hash table */
}

static char* iGLProgressBarGetMaxAttrib(Ihandle* ih)
{
  iGLProgressBar* pb = (iGLProgressBar*)iupAttribGet(ih, "_IUP_GLPROGRESSBAR");
  return iupStrReturnDouble(pb->vmax);
}

static int iGLProgressBarSetShowTextAttrib(Ihandle* ih, const char* value)
{
  iGLProgressBar* pb = (iGLProgressBar*)iupAttribGet(ih, "_IUP_GLPROGRESSBAR");
  pb->show_text = iupStrBoolean(value);
  return 0; /* do not store value in hash table */
}

static char* iGLProgressBarGetShowTextAttrib(Ihandle* ih)
{
  iGLProgressBar* pb = (iGLProgressBar*)iupAttribGet(ih, "_IUP_GLPROGRESSBAR");
  return iupStrReturnBoolean(pb->show_text);
}

static int iGLProgressBarSetPaddingAttrib(Ihandle* ih, const char* value)
{
  iGLProgressBar* pb = (iGLProgressBar*)iupAttribGet(ih, "_IUP_GLPROGRESSBAR");
  iupStrToIntInt(value, &pb->horiz_padding, &pb->vert_padding, 'x');
  return 0;
}

static char* iGLProgressBarGetPaddingAttrib(Ihandle* ih)
{
  iGLProgressBar* pb = (iGLProgressBar*)iupAttribGet(ih, "_IUP_GLPROGRESSBAR");
  return iupStrReturnIntInt(pb->horiz_padding, pb->vert_padding, 'x');
}

static int iGLProgressBarMapMethod(Ihandle* ih)
{
  if (iupStrEqualNoCase(iupAttribGetStr(ih, "ORIENTATION"), "VERTICAL"))
  {
    if (ih->currentheight < ih->currentwidth)
    {
      int tmp = ih->currentheight;
      ih->currentheight = ih->currentwidth;
      ih->currentwidth = tmp;
    }
  }
  return IUP_NOERROR;
}

static int iGLProgressBarCreateMethod(Ihandle* ih, void **params)
{
  iGLProgressBar* pb = (iGLProgressBar*)malloc(sizeof(iGLProgressBar));
  iupAttribSet(ih, "_IUP_GLPROGRESSBAR", (char*)pb);

  /* default values */
  pb->vmax      = 1;
  pb->show_text = 1;

  /* progress bar default size is 200x30 */
  IupSetAttribute(ih, "RASTERSIZE", "200x30");

  IupSetCallback(ih, "GL_ACTION", (Icallback)iGLProgressBarACTION_CB);

  (void)params;
  return IUP_NOERROR;
}

static void iGLProgressBarDestroyMethod(Ihandle* ih)
{
  iGLProgressBar* pb = (iGLProgressBar*)iupAttribGet(ih, "_IUP_GLPROGRESSBAR");
  free(pb);
}

Iclass* iupGLProgressBarNewClass(void)
{
  Iclass* ic = iupClassNew(iupRegisterFindClass("glsubcanvas"));

  ic->name = "glprogressbar";
  ic->format = NULL; /* no parameters */
  ic->nativetype = IUP_TYPEVOID;
  ic->childtype   = IUP_CHILDNONE;
  ic->is_interactive = 0;

  /* Class functions */
  ic->New = iupGLProgressBarNewClass;
  ic->Create  = iGLProgressBarCreateMethod;
  ic->Destroy = iGLProgressBarDestroyMethod;
  ic->Map = iGLProgressBarMapMethod;

  /* IupGLProgressBar only */
  iupClassRegisterAttribute(ic, "MIN", iGLProgressBarGetMinAttrib, iGLProgressBarSetMinAttrib, IUPAF_SAMEASSYSTEM, "0", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "MAX", iGLProgressBarGetMaxAttrib, iGLProgressBarSetMaxAttrib, IUPAF_SAMEASSYSTEM, "1", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "VALUE", iGLProgressBarGetValueAttrib, iGLProgressBarSetValueAttrib, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "ORIENTATION", NULL, NULL, IUPAF_SAMEASSYSTEM, "HORIZONTAL", IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "PADDING", iGLProgressBarGetPaddingAttrib, iGLProgressBarSetPaddingAttrib, IUPAF_SAMEASSYSTEM, "0x0", IUPAF_NOT_MAPPED);
  iupClassRegisterAttribute(ic, "TEXT", NULL, NULL, NULL, NULL, IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "SHOW_TEXT", iGLProgressBarGetShowTextAttrib, iGLProgressBarSetShowTextAttrib, IUPAF_SAMEASSYSTEM, "YES", IUPAF_NOT_MAPPED);
  iupClassRegisterAttribute(ic, "FGCOLOR", NULL, NULL, IUPAF_SAMEASSYSTEM, "200 225 245", IUPAF_NOT_MAPPED);

  return ic;
}

Ihandle *IupGLProgressBar(void)
{
  return IupCreate("glprogressbar");
}
