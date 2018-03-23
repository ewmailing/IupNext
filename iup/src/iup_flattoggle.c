/** \file
 * \brief Toggle Control
 *
 * See Copyright Notice in "iup.h"
 */

#include <stdlib.h>

#include "iup.h"
#include "iupcbs.h"

#include "iup_object.h"
#include "iup_attrib.h"
#include "iup_str.h"
#include "iup_drv.h"
#include "iup_drvfont.h"
#include "iup_image.h"
#include "iup_stdcontrols.h"
#include "iup_register.h"
#include "iup_drvdraw.h"
#include "iup_key.h"


#define BUTTON_GAP 2

/* from IupRadio implementation */
Ihandle *iupRadioFindToggleParent(Ihandle* ih_toggle);


struct _IcontrolData
{
  iupCanvas canvas;  /* from IupCanvas (must reserve it) */

  /* attributes */
  int horiz_padding, vert_padding;  /* button margin */
  int spacing, img_position;        /* used when both text and image are displayed */
  int horiz_alignment, vert_alignment;
  int border_width;
  int button_size;

  /* aux */
  int has_focus,
    highlighted,
    pressed;
};


/****************************************************************/


static int iFlatToggleRedraw_CB(Ihandle* ih)
{
  char *image = iupAttribGet(ih, "IMAGE");
  char* title = iupAttribGet(ih, "TITLE");
  int active = IupGetInt(ih, "ACTIVE");  /* native implementation */
  int notdef = iupStrEqualNoCase(iupAttribGet(ih, "VALUE"), "NOTDEF");
  int selected = (notdef) ? -1 : iupAttribGetInt(ih, "VALUE");
  char* fgcolor = iupAttribGetStr(ih, "FGCOLOR");
  char* bgcolor = iupAttribGetStr(ih, "BGCOLOR");
  char* btcolor = "255 255 255";
  char* bgimage = iupAttribGet(ih, "BACKIMAGE");
  char* fgimage = iupAttribGet(ih, "FRONTIMAGE");
  char* text_align = iupAttribGetStr(ih, "TEXTALIGNMENT");
  int right_button = iupAttribGetInt(ih, "RIGHTBUTTON");
  Ihandle* radio = iupRadioFindToggleParent(ih);
  const char* draw_image;
  int border_width = ih->data->border_width;
  int draw_border = 0;
  int image_pressed;
  IdrawCanvas* dc = iupdrvDrawCreateCanvas(ih);
  int make_inactive = 0;
  int button_position = (right_button) ? ih->currentwidth - ih->data->button_size : 0;
  int icon_position = (right_button) ? 0 : ih->data->button_size;

  iupDrawParentBackground(dc, ih);

  if (!bgcolor)
    bgcolor = iupBaseNativeParentGetBgColorAttrib(ih);

  if (selected)
  {
    char* presscolor = iupAttribGetStr(ih, "PSCOLOR");
    if (presscolor)
      btcolor = presscolor;
  }

  if (selected && !ih->data->highlighted)
  {
    char* presscolor = iupAttribGetStr(ih, "PSCOLOR");
    if (presscolor)
      bgcolor = presscolor;

    draw_border = 1;
  }
  else if (ih->data->highlighted)
  {
    char* hlcolor = iupAttribGetStr(ih, "HLCOLOR");
    if (hlcolor)
      bgcolor = hlcolor;

    draw_border = 1;
  }

  /* draw border - can still be disabled setting border_width=0 */
  if (draw_border)
  {
    char* bordercolor = iupAttribGetStr(ih, "BORDERCOLOR");

    if (selected && !ih->data->highlighted)
    {
      char* presscolor = iupAttribGetStr(ih, "BORDERPSCOLOR");
      if (presscolor)
        bordercolor = presscolor;
    }
    else if (ih->data->highlighted)
    {
      char* hlcolor = iupAttribGetStr(ih, "BORDERHLCOLOR");
      if (hlcolor)
        bordercolor = hlcolor;
    }


    iupFlatDrawBorder(dc, 0, ih->currentwidth - 1,
                      0, ih->currentheight - 1,
                      border_width, bordercolor, bgcolor, active);
  }

  /* simulate pressed when selected and has images (but colors and borders are not included) */
  image_pressed = ih->data->pressed;
  if (selected && !ih->data->pressed && (bgimage || image))
    image_pressed = 1;

  draw_image = iupFlatGetImageName(ih, "IMAGE", image, image_pressed, ih->data->highlighted, active, &make_inactive);

  /* draw background */
  if (bgimage)
  {
    draw_image = iupFlatGetImageName(ih, "BACKIMAGE", bgimage, image_pressed, ih->data->highlighted, active, &make_inactive);
    iupdrvDrawImage(dc, draw_image, make_inactive, border_width, border_width);
  }
  else if (draw_image)
    iupFlatDrawBox(dc, border_width, ih->currentwidth - 1 - border_width,
                    border_width, ih->currentheight - 1 - border_width,
                    bgcolor, NULL, 1);  /* background is always active */

  if (draw_image)
    iupFlatDrawIcon(ih, dc, border_width, border_width,
                    ih->currentwidth - 2 * border_width, ih->currentheight - 2 * border_width,
                    ih->data->img_position, ih->data->spacing, ih->data->horiz_alignment, ih->data->vert_alignment, ih->data->horiz_padding, ih->data->vert_padding,
                    draw_image, make_inactive, title, text_align, fgcolor, bgcolor, active);
  else
  {
    iupFlatDrawIcon(ih, dc, border_width + icon_position, border_width,
                    ih->currentwidth - (2 * border_width) - ih->data->button_size, ih->currentheight - 2 * border_width,
                    ih->data->img_position, ih->data->spacing, ih->data->horiz_alignment, ih->data->vert_alignment, ih->data->horiz_padding, ih->data->vert_padding,
                    draw_image, make_inactive, title, text_align, fgcolor, bgcolor, active);

    if (selected)
    {
      if (radio)
        iupFlatDrawDrawCircle(dc, button_position + ih->data->button_size / 2, ih->currentheight / 2, ih->data->button_size / 2 - 2 * BUTTON_GAP, 1, btcolor, btcolor, active);
      else if (selected == -1)
        iupFlatDrawBox(dc, button_position + 2 * BUTTON_GAP, button_position + 2 * BUTTON_GAP + ih->data->button_size - 4 * BUTTON_GAP, (ih->currentheight - (ih->data->button_size - 4 * BUTTON_GAP)) / 2,
                        ((ih->currentheight - (ih->data->button_size - 4*BUTTON_GAP)) / 2) + (ih->data->button_size - 4*BUTTON_GAP),
                        btcolor, btcolor, active);
      else
        iupFlatDrawCheckMark(dc, button_position + BUTTON_GAP, button_position + BUTTON_GAP + ih->data->button_size - 2 * BUTTON_GAP, (ih->currentheight - (ih->data->button_size - 2 * BUTTON_GAP)) / 2,
                              ((ih->currentheight - (ih->data->button_size - 2*BUTTON_GAP)) / 2) + (ih->data->button_size - 2*BUTTON_GAP), fgcolor, bgcolor, active);
    }
    else
    {
      if (radio)
        iupFlatDrawDrawCircle(dc, button_position + ih->data->button_size / 2, ih->currentheight / 2, ih->data->button_size / 2 - BUTTON_GAP, 1, btcolor, btcolor, active);
      else
        iupFlatDrawBox(dc, button_position + BUTTON_GAP, button_position + BUTTON_GAP + ih->data->button_size - 2 * BUTTON_GAP, (ih->currentheight - (ih->data->button_size - 2 * BUTTON_GAP)) / 2,
                        ((ih->currentheight - (ih->data->button_size - 2*BUTTON_GAP)) / 2) + (ih->data->button_size - 2*BUTTON_GAP),
                        btcolor, btcolor, active);
    }

    if (radio)
      iupFlatDrawDrawCircle(dc, button_position + ih->data->button_size / 2, ih->currentheight / 2, ih->data->button_size / 2 - BUTTON_GAP, 0, fgcolor, fgcolor, active);
    else
      iupFlatDrawBorder(dc, button_position + BUTTON_GAP, button_position + BUTTON_GAP + ih->data->button_size - 2 * BUTTON_GAP, (ih->currentheight - (ih->data->button_size - 2 * BUTTON_GAP)) / 2,
                          ((ih->currentheight - (ih->data->button_size - 2*BUTTON_GAP)) / 2) + (ih->data->button_size - 2*BUTTON_GAP), 1,
                          fgcolor, bgcolor, active);
  }

  if (fgimage)
  {
    draw_image = iupFlatGetImageName(ih, "FRONTIMAGE", fgimage, image_pressed, ih->data->highlighted, active, &make_inactive);
    iupdrvDrawImage(dc, draw_image, make_inactive, border_width, border_width);
  }
  else if (!image && !title)
  {
    int space = border_width + 2;
    iupFlatDrawBorder(dc, space, ih->currentwidth - 1 - space,
                      space, ih->currentheight - 1 - space,
                      1, "0 0 0", bgcolor, active);
    space++;
    iupFlatDrawBox(dc, space, ih->currentwidth - 1 - space,
                   space, ih->currentheight - 1 - space,
                   fgcolor, bgcolor, active);
  }


  if (ih->data->has_focus)
    iupdrvDrawFocusRect(dc, border_width, border_width, ih->currentwidth - 1 - border_width, ih->currentheight - 1 - border_width);

  iupdrvDrawFlush(dc);

  iupdrvDrawKillCanvas(dc);

  return IUP_DEFAULT;
}

static void iFlatToggleNotify(Ihandle* ih)
{
  IFni cb = (IFni)IupGetCallback(ih, "FLAT_ACTION");
  char *value = iupAttribGet(ih, "VALUE");
  int v = iupAttribGetInt(ih, "VALUE");
  if (iupStrEqualNoCase(value, "NOTDEF"))
    v = -1;
  if (cb)
  {
    int ret = cb(ih, v);
    if (ret == IUP_CLOSE)
      IupExitLoop();
  }

  if (iupObjectCheck(ih))
    iupBaseCallValueChangedCb(ih);
}

static int iFlatToggleButton_CB(Ihandle* ih, int button, int pressed, int x, int y, char* status)
{
  IFniiiis cb = (IFniiiis)IupGetCallback(ih, "FLAT_BUTTON_CB");
  if (cb)
  {
    if (cb(ih, button, pressed, x, y, status) == IUP_IGNORE)
      return IUP_DEFAULT;
  }

  if (button == IUP_BUTTON1)
  {
    Ihandle* radio = iupRadioFindToggleParent(ih);
    int tstate = iupAttribGetInt(ih, "3STATE");
    int notdef = iupStrEqualNoCase(iupAttribGet(ih, "VALUE"), "NOTDEF");
    int selected =  (notdef) ? -1 : iupAttribGetInt(ih, "VALUE");
    Ihandle* last_tg = NULL;

    if (!pressed)
    {
      if (selected>0)  /* was ON */
      {
        if (!radio)
        {
          if (tstate)
          {
            iupAttribSet(ih, "VALUE", "NOTDEF");
            selected = -1;
          }
          else
          {
            iupAttribSet(ih, "VALUE", "OFF");
            selected = 0;
          }
        }
        else
          last_tg = ih;  /* to avoid the callback call */

      }
      else if (selected == -1)
      {
        iupAttribSet(ih, "VALUE", "OFF");
        selected = 0;
      }
      else  /* was OFF */
      {
        if (radio)
        {
          last_tg = (Ihandle*)iupAttribGet(radio, "_IUP_FLATBUTTON_LASTRADIO");
          if (iupObjectCheck(last_tg) && last_tg != ih)
          {
            iupAttribSet(last_tg, "VALUE", "OFF");
            iupdrvRedrawNow(last_tg);
          }
          else
            last_tg = NULL;

          iupAttribSet(radio, "_IUP_FLATBUTTON_LASTRADIO", (char*)ih);
        }

        iupAttribSet(ih, "VALUE", "ON");
        selected = 1;
      }
    }

    ih->data->pressed = pressed;
    iupdrvRedrawNow(ih);

    if (!pressed)
    {
      if (last_tg && ih != last_tg)
        iFlatToggleNotify(last_tg);

      if (!radio || ih != last_tg)
        iFlatToggleNotify(ih);
    }
  }

  return IUP_DEFAULT;
}

static int iFlatToggleActivate_CB(Ihandle* ih)
{
  char status[IUPKEY_STATUS_SIZE] = IUPKEY_STATUS_INIT;

  iFlatToggleButton_CB(ih, IUP_BUTTON1, 1, 0, 0, status);

  iupdrvSleep(100);

  iFlatToggleButton_CB(ih, IUP_BUTTON1, 0, 0, 0, status);

  return IUP_DEFAULT;
}

static int iFlatToggleFocus_CB(Ihandle* ih, int focus)
{
  IFni cb = (IFni)IupGetCallback(ih, "FLAT_FOCUS_CB");
  if (cb)
  {
    if (cb(ih, focus) == IUP_IGNORE)
      return IUP_DEFAULT;
  }

  ih->data->has_focus = focus;
  iupdrvRedrawNow(ih);

  return IUP_DEFAULT;
}

static int iFlatToggleEnterWindow_CB(Ihandle* ih)
{
  IFn cb = (IFn)IupGetCallback(ih, "FLAT_ENTERWINDOW_CB");
  if (cb)
  {
    if (cb(ih) == IUP_IGNORE)
      return IUP_DEFAULT;
  }

  ih->data->highlighted = 1;
  iupdrvRedrawNow(ih);

  return IUP_DEFAULT;
}

static int iFlatToggleLeaveWindow_CB(Ihandle* ih)
{
  IFn cb = (IFn)IupGetCallback(ih, "FLAT_LEAVEWINDOW_CB");
  if (cb)
  {
    if (cb(ih) == IUP_IGNORE)
      return IUP_DEFAULT;
  }

  ih->data->highlighted = 0;
  iupdrvRedrawNow(ih);

  return IUP_DEFAULT;
}


/***********************************************************************************************/


static int iFlatToggleSetActiveAttrib(Ihandle* ih, const char* value)
{
  iupBaseSetActiveAttrib(ih, value);
  iupdrvRedrawNow(ih);
  return 0;
}

static int iFlatToggleSetAlignmentAttrib(Ihandle* ih, const char* value)
{
  char value1[30], value2[30];

  iupStrToStrStr(value, value1, value2, ':');

  ih->data->horiz_alignment = iupFlatGetHorizontalAlignment(value1);
  ih->data->vert_alignment = iupFlatGetVerticalAlignment(value2);

  if (ih->handle)
    iupdrvRedrawNow(ih);

  return 1;
}

static char* iFlatToggleGetAlignmentAttrib(Ihandle *ih)
{
  char* horiz_align2str[3] = { "ALEFT", "ACENTER", "ARIGHT" };
  char* vert_align2str[3] = { "ATOP", "ACENTER", "ABOTTOM" };
  return iupStrReturnStrf("%s:%s", horiz_align2str[ih->data->horiz_alignment], vert_align2str[ih->data->vert_alignment]);
}

static int iFlatToggleSetPaddingAttrib(Ihandle* ih, const char* value)
{
  iupStrToIntInt(value, &ih->data->horiz_padding, &ih->data->vert_padding, 'x');
  if (ih->handle)
    iupdrvRedrawNow(ih);
  return 0;
}

static int iFlatToggleSetButtonSizeAttrib(Ihandle* ih, const char* value)
{
  iupStrToInt(value, &ih->data->button_size);
  if (ih->handle)
    iupdrvRedrawNow(ih);
  return 0;
}

static int iFlatToggleSetBgColorAttrib(Ihandle* ih, const char* value)
{
  (void)value;
  iupdrvPostRedraw(ih);
  return 1;
}

static char* iFlatToggleGetBgColorAttrib(Ihandle* ih)
{
  char* value = iupAttribGet(ih, "BGCOLOR");
  if (!value)
    return iupBaseNativeParentGetBgColorAttrib(ih);
  else
    return value;
}

static char* iFlatToggleGetPaddingAttrib(Ihandle* ih)
{
  return iupStrReturnIntInt(ih->data->horiz_padding, ih->data->vert_padding, 'x');
}

static char* iFlatToggleGetButtonSizeAttrib(Ihandle* ih)
{
  return iupStrReturnIntInt(ih->data->button_size, ih->data->button_size, 'x');
}

static int iFlatToggleSetImagePositionAttrib(Ihandle* ih, const char* value)
{
  ih->data->img_position = iupFlatGetImagePosition(value);

  if (ih->handle)
    iupdrvRedrawNow(ih);

  return 0;
}

static char* iFlatToggleGetImagePositionAttrib(Ihandle *ih)
{
  char* img_pos2str[4] = { "LEFT", "RIGHT", "TOP", "BOTTOM" };
  return iupStrReturnStr(img_pos2str[ih->data->img_position]);
}

static int iFlatToggleSetSpacingAttrib(Ihandle* ih, const char* value)
{
  iupStrToInt(value, &ih->data->spacing);
  if (ih->handle)
    iupdrvRedrawNow(ih);
  return 0;
}

static char* iFlatToggleGetSpacingAttrib(Ihandle *ih)
{
  return iupStrReturnInt(ih->data->spacing);
}

static int iFlatToggleSetBorderWidthAttrib(Ihandle* ih, const char* value)
{
  iupStrToInt(value, &ih->data->border_width);
  if (ih->handle)
    iupdrvRedrawNow(ih);
  return 0;
}

static char* iFlatToggleGetBorderWidthAttrib(Ihandle *ih)
{
  return iupStrReturnInt(ih->data->border_width);
}

static int iFlatToggleSetValueAttrib(Ihandle* ih, const char* value)
{
  Ihandle* radio = iupRadioFindToggleParent(ih);
  if (radio)
  {
    /* can only set Radio to ON */
    if (iupStrBoolean(value))
    {
      Ihandle* last_tg = (Ihandle*)iupAttribGet(radio, "_IUP_FLATBUTTON_LASTRADIO");
      if (iupObjectCheck(last_tg) && last_tg != ih)
      {
        iupAttribSet(last_tg, "VALUE", "OFF");
        if (last_tg->handle)
          iupdrvRedrawNow(last_tg);
      }

      iupAttribSet(radio, "_IUP_FLATBUTTON_LASTRADIO", (char*)ih);
    }
    else
      return 0;
  }
  else
  {
    int oldcheck = iupAttribGetBoolean(ih, "VALUE");
    if (oldcheck)
      iupAttribSet(ih, "VALUE", "OFF");
    else
      iupAttribSet(ih, "VALUE", "ON");

    if (ih->handle)
      iupdrvRedrawNow(ih);

    return 0;
  }

  if (ih->handle)
    iupdrvPostRedraw(ih);

  return 1;
}

static char* iFlatToggleGetRadioAttrib(Ihandle* ih)
{
  Ihandle* radio = iupRadioFindToggleParent(ih);
  return iupStrReturnBoolean(radio != NULL);
}

static char* iFlatToggleGetHighlightedAttrib(Ihandle* ih)
{
  return iupStrReturnBoolean(ih->data->highlighted);
}

static char* iFlatToggleGetPressedAttrib(Ihandle* ih)
{
  return iupStrReturnBoolean(ih->data->pressed);
}

static char* iFlatToggleGetHasFocusAttrib(Ihandle* ih)
{
  return iupStrReturnBoolean(ih->data->has_focus);
}


/*****************************************************************************************/


static int iFlatToggleCreateMethod(Ihandle* ih, void** params)
{
  if (params && params[0])
  {
    iupAttribSetStr(ih, "TITLE", (char*)(params[0]));
  }

  /* free the data allocated by IupCanvas */
  free(ih->data);
  ih->data = iupALLOCCTRLDATA();

  /* change the IupCanvas default values */
  iupAttribSet(ih, "BORDER", "NO");
  ih->expand = IUP_EXPAND_NONE;

  /* non zero default values */
  ih->data->spacing = 2;
  ih->data->border_width = 1;
  ih->data->horiz_alignment = IUP_ALIGN_ACENTER;
  ih->data->vert_alignment = IUP_ALIGN_ACENTER;

  /* initial values - don't use default so they can be set to NULL */
  iupAttribSet(ih, "HLCOLOR", "200 225 245");
  iupAttribSet(ih, "PSCOLOR", "150 200 235");

  /* internal callbacks */
  IupSetCallback(ih, "ACTION", (Icallback)iFlatToggleRedraw_CB);
  IupSetCallback(ih, "BUTTON_CB", (Icallback)iFlatToggleButton_CB);
  IupSetCallback(ih, "FOCUS_CB", (Icallback)iFlatToggleFocus_CB);
  IupSetCallback(ih, "LEAVEWINDOW_CB", iFlatToggleLeaveWindow_CB);
  IupSetCallback(ih, "ENTERWINDOW_CB", iFlatToggleEnterWindow_CB);
  IupSetCallback(ih, "K_CR", (Icallback)iFlatToggleActivate_CB);
  IupSetCallback(ih, "K_SP", (Icallback)iFlatToggleActivate_CB);

  return IUP_NOERROR;
}

static int iFlatToggleMapMethod(Ihandle* ih)
{
  Ihandle* radio = iupRadioFindToggleParent(ih);
  if (radio)
  {
    if (!iupAttribGet(radio, "_IUP_FLATBUTTON_LASTRADIO"))
    {
      /* this is the first toggle in the radio, and then set it with VALUE=ON */
      iupAttribSet(ih, "VALUE", "ON");
    }

    /* make sure it has at least one name */
    if (!iupAttribGetHandleName(ih))
      iupAttribSetHandleName(ih);
  }
  return IUP_NOERROR;
}

static void iFlatToggleComputeNaturalSizeMethod(Ihandle* ih, int *w, int *h, int *children_expand)
{
  int fit2backimage = iupAttribGetBoolean(ih, "FITTOBACKIMAGE");
  char* bgimage = iupAttribGet(ih, "BACKIMAGE");
  if (fit2backimage && bgimage)
    iupImageGetInfo(bgimage, w, h, NULL);
  else
  {
    char* image = iupAttribGet(ih, "IMAGE");
    char* title = iupAttribGet(ih, "TITLE");

    *w = 0,
      *h = 0;

    if (image)
    {
      iupImageGetInfo(image, w, h, NULL);

      if (title)
      {
        int text_w, text_h;
        iupdrvFontGetMultiLineStringSize(ih, title, &text_w, &text_h);

        if (ih->data->img_position == IUP_IMGPOS_RIGHT ||
            ih->data->img_position == IUP_IMGPOS_LEFT)
        {
          *w += text_w + ih->data->spacing;
          *h = iupMAX(*h, text_h);
        }
        else
        {
          *w = iupMAX(*w, text_w);
          *h += text_h + ih->data->spacing;
        }
      }
    }
    else if (title)
      iupdrvFontGetMultiLineStringSize(ih, title, w, h);

    *w += 2 * ih->data->horiz_padding;
    *h += 2 * ih->data->vert_padding;

    *w += 2 * ih->data->border_width;
    *h += 2 * ih->data->border_width;

    *w += ih->data->button_size;
    if (ih->data->button_size > *h)
      *h = ih->data->button_size;
  }

  (void)children_expand; /* unset if not a container */
}


/******************************************************************************/


Iclass* iupFlatToggleNewClass(void)
{
  Iclass* ic = iupClassNew(iupRegisterFindClass("canvas"));

  ic->name = "flattoggle";
  ic->format = "s"; /* one string */
  ic->nativetype = IUP_TYPECANVAS;
  ic->childtype = IUP_CHILDNONE;
  ic->is_interactive = 1;

  /* Class functions */
  ic->New = iupFlatToggleNewClass;
  ic->Create = iFlatToggleCreateMethod;
  ic->ComputeNaturalSize = iFlatToggleComputeNaturalSizeMethod;
  ic->Map = iFlatToggleMapMethod;

  /* Callbacks */
  iupClassRegisterCallback(ic, "FLAT_ACTION", "i");
  iupClassRegisterCallback(ic, "FLAT_BUTTON_CB", "iiiis");
  iupClassRegisterCallback(ic, "FLAT_FOCUS_CB", "i");
  iupClassRegisterCallback(ic, "FLAT_ENTERWINDOW_CB", "ii");
  iupClassRegisterCallback(ic, "FLAT_LEAVEWINDOW_CB", "");
  iupClassRegisterCallback(ic, "VALUECHANGED_CB", "");

  /* Overwrite Visual */
  iupClassRegisterAttribute(ic, "ACTIVE", iupBaseGetActiveAttrib, iFlatToggleSetActiveAttrib, IUPAF_SAMEASSYSTEM, "YES", IUPAF_DEFAULT);

  /* Special */
  iupClassRegisterAttribute(ic, "TITLE", NULL, NULL, NULL, NULL, IUPAF_NO_DEFAULTVALUE | IUPAF_NO_INHERIT);

  /* IupButton only */
  iupClassRegisterAttribute(ic, "VALUE", NULL, iFlatToggleSetValueAttrib, NULL, NULL, IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "RADIO", iFlatToggleGetRadioAttrib, NULL, NULL, NULL, IUPAF_READONLY | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "ALIGNMENT", iFlatToggleGetAlignmentAttrib, iFlatToggleSetAlignmentAttrib, "ACENTER:ACENTER", NULL, IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "PADDING", iFlatToggleGetPaddingAttrib, iFlatToggleSetPaddingAttrib, IUPAF_SAMEASSYSTEM, "10x10", IUPAF_NOT_MAPPED);
  iupClassRegisterAttribute(ic, "SPACING", iFlatToggleGetSpacingAttrib, iFlatToggleSetSpacingAttrib, IUPAF_SAMEASSYSTEM, "2", IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "BUTTONSIZE", iFlatToggleGetButtonSizeAttrib, iFlatToggleSetButtonSizeAttrib, "15x15", NULL, IUPAF_NOT_MAPPED);
  iupClassRegisterAttribute(ic, "IGNORERADIO", NULL, NULL, NULL, NULL, IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "HIGHLIGHTED", iFlatToggleGetHighlightedAttrib, NULL, NULL, NULL, IUPAF_READONLY | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "PRESSED", iFlatToggleGetPressedAttrib, NULL, NULL, NULL, IUPAF_READONLY | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "HASFOCUS", iFlatToggleGetHasFocusAttrib, NULL, NULL, NULL, IUPAF_READONLY | IUPAF_NO_INHERIT);

  iupClassRegisterAttribute(ic, "BORDERCOLOR", NULL, NULL, IUPAF_SAMEASSYSTEM, "50 150 255", IUPAF_DEFAULT);  /* inheritable */
  iupClassRegisterAttribute(ic, "BORDERPSCOLOR", NULL, NULL, NULL, NULL, IUPAF_DEFAULT);  /* inheritable */
  iupClassRegisterAttribute(ic, "BORDERHLCOLOR", NULL, NULL, NULL, NULL, IUPAF_DEFAULT);  /* inheritable */
  iupClassRegisterAttribute(ic, "BORDERWIDTH", iFlatToggleGetBorderWidthAttrib, iFlatToggleSetBorderWidthAttrib, IUPAF_SAMEASSYSTEM, "1", IUPAF_DEFAULT);  /* inheritable */
  iupClassRegisterAttribute(ic, "FGCOLOR", NULL, NULL, "DLGFGCOLOR", NULL, IUPAF_NOT_MAPPED);  /* force the new default value */
  iupClassRegisterAttribute(ic, "BGCOLOR", iFlatToggleGetBgColorAttrib, iFlatToggleSetBgColorAttrib, IUPAF_SAMEASSYSTEM, "DLGBGCOLOR", IUPAF_NO_SAVE | IUPAF_DEFAULT);
  iupClassRegisterAttribute(ic, "HLCOLOR", NULL, NULL, NULL, NULL, IUPAF_DEFAULT);  /* inheritable */
  iupClassRegisterAttribute(ic, "PSCOLOR", NULL, NULL, NULL, NULL, IUPAF_DEFAULT);  /* inheritable */

  iupClassRegisterAttribute(ic, "IMAGE", NULL, NULL, NULL, NULL, IUPAF_IHANDLENAME | IUPAF_NO_DEFAULTVALUE | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "IMAGEPRESS", NULL, NULL, NULL, NULL, IUPAF_IHANDLENAME | IUPAF_NO_DEFAULTVALUE | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "IMAGEHIGHLIGHT", NULL, NULL, NULL, NULL, IUPAF_IHANDLENAME | IUPAF_NO_DEFAULTVALUE | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "IMAGEINACTIVE", NULL, NULL, NULL, NULL, IUPAF_IHANDLENAME | IUPAF_NO_DEFAULTVALUE | IUPAF_NO_INHERIT);

  iupClassRegisterAttribute(ic, "IMAGEPOSITION", iFlatToggleGetImagePositionAttrib, iFlatToggleSetImagePositionAttrib, IUPAF_SAMEASSYSTEM, "LEFT", IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "TEXTALIGNMENT", NULL, NULL, IUPAF_SAMEASSYSTEM, "ALEFT", IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);

  iupClassRegisterAttribute(ic, "BACKIMAGE", NULL, NULL, NULL, NULL, IUPAF_IHANDLENAME | IUPAF_NO_DEFAULTVALUE | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "BACKIMAGEPRESS", NULL, NULL, NULL, NULL, IUPAF_IHANDLENAME | IUPAF_NO_DEFAULTVALUE | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "BACKIMAGEHIGHLIGHT", NULL, NULL, NULL, NULL, IUPAF_IHANDLENAME | IUPAF_NO_DEFAULTVALUE | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "BACKIMAGEINACTIVE", NULL, NULL, NULL, NULL, IUPAF_IHANDLENAME | IUPAF_NO_DEFAULTVALUE | IUPAF_NO_INHERIT);

  iupClassRegisterAttribute(ic, "FITTOBACKIMAGE", NULL, NULL, NULL, NULL, IUPAF_NO_INHERIT);

  iupClassRegisterAttribute(ic, "FRONTIMAGE", NULL, NULL, NULL, NULL, IUPAF_IHANDLENAME | IUPAF_NO_DEFAULTVALUE | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "FRONTIMAGEPRESS", NULL, NULL, NULL, NULL, IUPAF_IHANDLENAME | IUPAF_NO_DEFAULTVALUE | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "FRONTIMAGEHIGHLIGHT", NULL, NULL, NULL, NULL, IUPAF_IHANDLENAME | IUPAF_NO_DEFAULTVALUE | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "FRONTIMAGEINACTIVE", NULL, NULL, NULL, NULL, IUPAF_IHANDLENAME | IUPAF_NO_DEFAULTVALUE | IUPAF_NO_INHERIT);

  iupClassRegisterAttribute(ic, "RIGHTBUTTON", NULL, NULL, NULL, NULL, IUPAF_NO_INHERIT);

  return ic;
}

Ihandle* IupFlatToggle(const char* title)
{
  void *params[2];
  params[0] = (void*)title;
  params[1] = NULL;
  return IupCreatev("flattoggle", params);
}
