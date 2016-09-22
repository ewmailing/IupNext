/** \file
 * \brief FlatFrame Control.
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
#include "iup_register.h"
#include "iup_drvdraw.h"
#include "iup_image.h"


static int iFlatFrameGetAlignment(Ihandle* ih)
{
  int horiz_alignment = IUP_ALIGN_ACENTER;  /* "ACENTER" */
  const char* value = iupAttribGetStr(ih, "TITLEALIGNMENT");
  if (iupStrEqualNoCase(value, "ARIGHT"))
    horiz_alignment = IUP_ALIGN_ARIGHT;
  else if (iupStrEqualNoCase(value, "ALEFT"))
    horiz_alignment = IUP_ALIGN_ALEFT;
  return horiz_alignment;
}

static int iFlatFrameGetImagePosition(Ihandle* ih)
{
  int img_position = IUP_IMGPOS_LEFT; /* "LEFT" */
  const char* value = iupAttribGetStr(ih, "TITLEIMAGEPOSITION");
  if (iupStrEqualNoCase(value, "RIGHT"))
    img_position = IUP_IMGPOS_RIGHT;
  else if (iupStrEqualNoCase(value, "BOTTOM"))
    img_position = IUP_IMGPOS_BOTTOM;
  else if (iupStrEqualNoCase(value, "TOP"))
    img_position = IUP_IMGPOS_TOP;
  return img_position;
}

static void iFlatFrameGetIconSize(Ihandle* ih, int *w, int *h)
{
  char* image = iupAttribGet(ih, "IMAGE");
  char* title = iupAttribGet(ih, "TITLE");
  int horiz_padding = 0, vert_padding = 0;
  IupGetIntInt(ih, "TITLEPADDING", &horiz_padding, &vert_padding);

  *w = 0,
  *h = 0;

  if (image)
  {
    iupImageGetInfo(image, w, h, NULL);

    if (title)
    {
      int img_position = iFlatFrameGetImagePosition(ih);
      int spacing = iupAttribGetInt(ih, "TITLEIMAGESPACING");
      int text_w, text_h;
      iupdrvFontGetMultiLineStringSize(ih, title, &text_w, &text_h);

      if (img_position == IUP_IMGPOS_RIGHT ||
          img_position == IUP_IMGPOS_LEFT)
      {
        *w += text_w + spacing;
        *h = iupMAX(*h, text_h);
      }
      else
      {
        *w = iupMAX(*w, text_w);
        *h += text_h + spacing;
      }
    }
  }
  else if (title)
    iupdrvFontGetMultiLineStringSize(ih, title, w, h);

  *w += 2 * horiz_padding;
  *h += 2 * vert_padding;
}

static int iFlatFrameGetTitleHeight(Ihandle* ih)
{
  int width, height;
  iFlatFrameGetIconSize(ih, &width, &height);

  if (height && iupAttribGetBoolean(ih, "TITLELINE"))
    height += iupAttribGetInt(ih, "TITLELINEWIDTH");

  return height;
}

static void iFlatFrameGetDecorSize(Ihandle* ih, int *width, int *height)
{
  *height = 0;
  *width = 0;

  if (iupAttribGetBoolean(ih, "FRAME"))
  {
    *height = *width = 2 * iupAttribGetInt(ih, "FRAMEWIDTH") + 2 * iupAttribGetInt(ih, "FRAMESPACE");
  }

  (*height) += iFlatFrameGetTitleHeight(ih);
}

static void iFlatFrameGetDecorOffset(Ihandle* ih, int *dx, int *dy)
{
  *dx = 0;
  *dy = 0;

  if (iupAttribGetBoolean(ih, "FRAME"))
  {
    *dx = *dy = iupAttribGetInt(ih, "FRAMEWIDTH") + iupAttribGetInt(ih, "FRAMESPACE");
  }

  dy += iFlatFrameGetTitleHeight(ih);
}

static int iFlatFrameRedraw_CB(Ihandle* ih)
{
  char* backcolor = iupAttribGetStr(ih, "BACKCOLOR");
  int frame_width = iupAttribGetInt(ih, "FRAMEWIDTH");
  int frame = iupAttribGetBoolean(ih, "FRAME");
  IdrawCanvas* dc = iupdrvDrawCreateCanvas(ih);
  int title_height = iFlatFrameGetTitleHeight(ih);

  iupdrvDrawParentBackground(dc);

  if (!backcolor)
    backcolor = iupBaseNativeParentGetBgColorAttrib(ih);

  /* draw border - can still be disabled setting frame_width=0 */
  if (frame)
  {
    char* frame_color = iupAttribGetStr(ih, "FRAMECOLOR");

    iupFlatDrawBorder(dc, 0, ih->currentwidth - 1,
                          0, ih->currentheight - 1,
                          frame_width, frame_color, NULL, 1);
  }

  /* draw background */
  iupFlatDrawBox(dc, frame_width, ih->currentwidth - 1 - frame_width,
                     frame_width + title_height, ih->currentheight - 1 - frame_width, backcolor, NULL, 1);

  if (title_height)
  {
    char *titleimage = iupAttribGet(ih, "TITLEIMAGE");
    char* title = iupAttribGet(ih, "TITLE");
    char* titlecolor = iupAttribGetStr(ih, "TITLECOLOR");
    char* titlebgcolor = iupAttribGetStr(ih, "TITLEBGCOLOR");
    int title_alignment = iFlatFrameGetAlignment(ih);
    int img_position = iFlatFrameGetImagePosition(ih);
    int spacing = iupAttribGetInt(ih, "TITLEIMAGESPACING");
    int horiz_padding, vert_padding;
    IupGetIntInt(ih, "TITLEPADDING", &horiz_padding, &vert_padding);

    int title_line = 0;
    if (iupAttribGetBoolean(ih, "TITLELINE"))
      title_line = iupAttribGetInt(ih, "TITLELINEWIDTH");

    /* draw title background */
    iupFlatDrawBox(dc, frame_width, ih->currentwidth - 1 - frame_width,
                       frame_width, frame_width + title_height - title_line, titlebgcolor, NULL, 1);

    if (iupAttribGetBoolean(ih, "TITLELINE"))
    {
      int i;
      char* title_line_color = iupAttribGetStr(ih, "TITLELINECOLOR");
      unsigned char r = 0, g = 0, b = 0;
      iupStrToRGB(title_line_color, &r, &g, &b);

      for (i = 0; i < title_line; i++)
        iupdrvDrawLine(dc, frame_width, frame_width + title_height - i, 
                           ih->currentwidth - 1 - frame_width, frame_width + title_height - i,
                           r, g, b, IUP_DRAW_STROKE);
    }

    iupFlatDrawIcon(ih, dc, frame_width, frame_width,
                    ih->currentwidth - 2 * frame_width, title_height - title_line,
                    img_position, spacing, title_alignment, IUP_ALIGN_ATOP, horiz_padding, vert_padding,
                    titleimage, 0, title, titlecolor, NULL, 1);
  }

  iupdrvDrawFlush(dc);

  iupdrvDrawKillCanvas(dc);

  return IUP_DEFAULT;
}


/*****************************************************************************************/


static char* iFlatFrameGetDecorSizeAttrib(Ihandle* ih)
{
  int decorwidth, decorheight;
  iFlatFrameGetDecorSize(ih, &decorwidth, &decorheight);
  return iupStrReturnIntInt(decorwidth, decorheight, 'x');
}

static char* iFlatFrameGetDecorOffsetAttrib(Ihandle* ih)
{
  int decor_x, decor_y;
  iFlatFrameGetDecorSize(ih, &decor_x, &decor_y);
  return iupStrReturnIntInt(decor_x, decor_y, 'x');
}

static int iFlatFrameCreateMethod(Ihandle* ih, void** params)
{
  (void)params;
  IupSetCallback(ih, "ACTION", (Icallback)iFlatFrameRedraw_CB);
  return IUP_NOERROR;
}


/******************************************************************************/


Ihandle* IupFlatFrame(Ihandle* child)
{
  void *children[2];
  children[0] = (void*)child;
  children[1] = NULL;
  return IupCreatev("flatframe", children);
}

Iclass* iupFlatFrameNewClass(void)
{
  Iclass* ic = iupClassNew(iupRegisterFindClass("backgroundbox"));

  ic->name = "flatframe";
  ic->format = "h"; /* one Ihandle* */
  ic->nativetype = IUP_TYPECONTROL;
  ic->childtype = IUP_CHILDMANY+1;   /* one child */
  ic->is_interactive = 0;

  /* Class functions */
  ic->New = iupFlatFrameNewClass;
  ic->Create = iFlatFrameCreateMethod;

  /* replace IupCanvas behavior */
  iupClassRegisterReplaceAttribFlags(ic, "BORDER", IUPAF_READONLY);

  /* replace IupBackgroundBox behavior */
  iupClassRegisterAttribute(ic, "DECORATION", NULL, NULL, IUPAF_SAMEASSYSTEM, "Yes", IUPAF_NOT_MAPPED | IUPAF_READONLY | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "DECORSIZE", iFlatFrameGetDecorSizeAttrib, NULL, NULL, NULL, IUPAF_NOT_MAPPED | IUPAF_READONLY | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "DECOROFFSET", iFlatFrameGetDecorOffsetAttrib, NULL, NULL, NULL, IUPAF_NOT_MAPPED | IUPAF_READONLY | IUPAF_NO_INHERIT);

  /* Special */
  iupClassRegisterAttribute(ic, "BACKCOLOR", NULL, NULL, IUPAF_SAMEASSYSTEM, "DLGBGCOLOR", IUPAF_NO_INHERIT);

  iupClassRegisterAttribute(ic, "TITLE", NULL, NULL, NULL, NULL, IUPAF_NO_DEFAULTVALUE | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "TITLECOLOR", NULL, NULL, IUPAF_SAMEASSYSTEM, "DLGFGCOLOR", IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "TITLEBGCOLOR", NULL, NULL, IUPAF_SAMEASSYSTEM, "DLGBGCOLOR", IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "TITLELINE", NULL, NULL, IUPAF_SAMEASSYSTEM, "YES", IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "TITLELINECOLOR", NULL, NULL, IUPAF_SAMEASSYSTEM, "DLGFGCOLOR", IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "TITLELINEWIDTH", NULL, NULL, IUPAF_SAMEASSYSTEM, "1", IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "TITLEIMAGE", NULL, NULL, NULL, NULL, IUPAF_IHANDLENAME | IUPAF_NO_DEFAULTVALUE | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "TITLEIMAGEPOSITION", NULL, NULL, IUPAF_SAMEASSYSTEM, "LEFT", IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "TITLEIMAGESPACING", NULL, NULL, IUPAF_SAMEASSYSTEM, "2", IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "TITLEALIGNMENT", NULL, NULL, "ACENTER", NULL, IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "TITLEPADDING", NULL, NULL, IUPAF_SAMEASSYSTEM, "0x0", IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);

  iupClassRegisterAttribute(ic, "FRAME", NULL, NULL, IUPAF_SAMEASSYSTEM, "YES", IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "FRAMECOLOR", NULL, NULL, IUPAF_SAMEASSYSTEM, "DLGFGCOLOR", IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "FRAMEWIDTH", NULL, NULL, IUPAF_SAMEASSYSTEM, "1", IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "FRAMESPACE", NULL, NULL, IUPAF_SAMEASSYSTEM, "2", IUPAF_NO_INHERIT);

  return ic;
}
