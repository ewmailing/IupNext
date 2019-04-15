/** \file
 * \brief List Control
 *
 * See Copyright Notice in "iup.h"
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "iup.h"
#include "iupcbs.h"

#include "iup_assert.h"
#include "iup_object.h"
#include "iup_attrib.h"
#include "iup_str.h"
#include "iup_drv.h"
#include "iup_drvfont.h"
#include "iup_drvinfo.h"
#include "iup_stdcontrols.h"
#include "iup_layout.h"
#include "iup_image.h"
#include "iup_array.h"
#include "iup_drvdraw.h"
#include "iup_draw.h"
#include "iup_register.h"
#include "iup_flatscrollbar.h"


static void iFlatListCallActionCallback(Ihandle* ih, IFnsii cb, int pos, int state);
static void iFlatListMultipleCallActionCb(Ihandle* ih, IFnsii cb, IFns multi_cb, IFn valuechanged_cb, int* pos, int sel_count);
static void iFlatListSingleCallActionCb(Ihandle* ih, IFnsii cb, IFn valuechanged_cb, int pos);

typedef struct _iFlatListItem {
  char* name;
  char* image;
  char *fgColor;
  char *bgColor;
//  char *fontStyle;
  int selected;
} iFlatListItem;

struct _IcontrolData
{
  iupCanvas canvas;  /* from IupCanvas (must reserve it) */

  Iarray *items_array;

  /* aux */
  int line_height, line_width;
  int dragging_pos;
  int has_focus, focus_pos;

  /* attributes */
  int horiz_padding, vert_padding;  /* button margin */
  int spacing, icon_spacing, img_position;        /* used when both text and image are displayed */
  int horiz_alignment, vert_alignment;
  int border_width;
  int is_multiple;
  int show_dragdrop;
};

/* Color attenuation factor in a marked cell, 20% darker */
#define IMAT_ATENUATION(_x)    ((unsigned char)(((_x)*8)/10))


static int iFlatListGetScrollbar(Ihandle* ih)
{
  int flat = iupFlatScrollBarGet(ih);
  if (flat != IUP_SB_NONE)
    return flat;
  else
  {
    if (!ih->handle)
      ih->data->canvas.sb = iupBaseGetScrollbar(ih);

    return ih->data->canvas.sb;
  }
}

static int iFlatListGetScrollbarSize(Ihandle* ih)
{
  if (iupFlatScrollBarGet(ih) != IUP_SB_NONE)
  {
    if (iupAttribGetBoolean(ih, "SHOWFLOATING"))
      return 0;
    else
      return iupAttribGetInt(ih, "SCROLLBARSIZE");
  }
  else
    return iupdrvGetScrollbarSize();
}

static int iFlatListConvertXYToPos(Ihandle* ih, int x, int y)
{
  int posy = IupGetInt(ih, "POSY");
  int count = iupArrayCount(ih->data->items_array);
  int pos = (int)((y + posy) / (ih->data->line_height + ih->data->spacing)) + 1; /* pos starts at 1 */

  if (pos < 1 || pos > count)
    return -1;

  (void)x;
  return pos;
}

static void iFlatListCopyItem(Ihandle *ih, int from, int to, int selected)
{
  int count = iupArrayCount(ih->data->items_array);
  iFlatListItem* items = (iFlatListItem*)iupArrayGetData(ih->data->items_array);
  iFlatListItem copy = items[from - 1];
  int index;

  if (to <= count)
  {
    items = (iFlatListItem*)iupArrayInsert(ih->data->items_array, to - 1, 1);
    index = to - 1;
  }
  else
  {
    items = (iFlatListItem*)iupArrayInc(ih->data->items_array);
    index = count;
  }

  items[index].name = iupStrDup(copy.name);
  items[index].image = iupStrDup(copy.image);
  items[index].fgColor = iupStrDup(copy.fgColor);
  items[index].bgColor = iupStrDup(copy.bgColor);
  items[index].selected = selected;
}

static void iFlatListRemoveItem(Ihandle *ih, int index, int remove_count)
{
  iFlatListItem* items = (iFlatListItem*)iupArrayGetData(ih->data->items_array);
  int i;
  for (i = index; i < remove_count; i++)
  {
    if (items[i].name)
      free(items[i].name);

    if (items[i].image)
      free(items[i].image);

    if (items[i].fgColor)
      free(items[i].fgColor);

    if (items[i].bgColor)
      free(items[i].bgColor);
  }
  iupArrayRemove(ih->data->items_array, index, remove_count);
}

static void iFlatListUnSelectedAll(Ihandle *ih)
{
  iFlatListItem* items = (iFlatListItem*)iupArrayGetData(ih->data->items_array);
  int count = iupArrayCount(ih->data->items_array);
  int i;
  for (i = 0; i < count; i++)
  {
    if (items[i].selected == 0)
      continue;
    break;
  }
  iupArrayRemove(ih->data->items_array, i, 1);
}

static void iFlatListAddMarkedAttenuation(Ihandle* ih, unsigned char *r, unsigned char *g, unsigned char *b)
{
  char* hlcolor = iupAttribGetStr(ih, "HLCOLOR");
  unsigned char hl_r, hl_g, hl_b;
  if (iupStrToRGB(hlcolor, &hl_r, &hl_g, &hl_b))
  {
    unsigned char a = (unsigned char)iupAttribGetInt(ih, "HLCOLORALPHA");
    *r = iupALPHABLEND(*r, hl_r, a);
    *g = iupALPHABLEND(*g, hl_g, a);
    *b = iupALPHABLEND(*b, hl_b, a);
  }

  *r = IMAT_ATENUATION(*r);
  *g = IMAT_ATENUATION(*g);
  *b = IMAT_ATENUATION(*b);
}

static void iFlatListCalcItemMaxSize(Ihandle *ih, iFlatListItem* items, int count, int *max_w, int *max_h)
{
  int i;

  *max_w = 0;
  *max_h = 0;

  for (i = 0; i < count; i++)
  {
    int item_width, item_height;
    char *text = items[i].name;
    char* imagename = items[i].image;

    iupFlatDrawGetIconSize(ih, ih->data->img_position, ih->data->icon_spacing, ih->data->horiz_padding, ih->data->vert_padding, imagename, text, &item_width, &item_height, 0.0);

    if (item_width > *max_w) *max_w = item_width;
    if (item_height > *max_h) *max_h = item_height;
  }
}

static void iFlatListUpdateScrollBar(Ihandle *ih)
{
  int canvas_width = ih->currentwidth;
  int canvas_height = ih->currentheight;
  iFlatListItem* items = (iFlatListItem*)iupArrayGetData(ih->data->items_array);
  int count = iupArrayCount(ih->data->items_array);
  int sb, max_w, max_h, view_width, view_height;

  if (iupAttribGetBoolean(ih, "BORDER")) /* native border around scrollbars */
  {
    canvas_width -= 2;
    canvas_height -= 2;
  }

  canvas_width -= 2 * ih->data->border_width;
  canvas_height -= 2 * ih->data->border_width;

  iFlatListCalcItemMaxSize(ih, items, count, &max_w, &max_h);

  ih->data->line_width = iupMAX(max_w, canvas_width);
  ih->data->line_height = max_h;

  view_width = max_w;
  view_height = max_h * count;
  view_height += (count - 1) * ih->data->spacing;

  sb = iFlatListGetScrollbar(ih);
  if (sb)
  {
    int sb_size = iFlatListGetScrollbarSize(ih);
    int noscroll_width = canvas_width;
    int noscroll_height = canvas_height;

    if (sb & IUP_SB_HORIZ)
    {
      IupSetInt(ih, "XMAX", view_width);

      if (view_height > noscroll_height)  /* affects horizontal size */
        canvas_width -= sb_size;
    }
    else
      IupSetAttribute(ih, "XMAX", "0");

    if (sb & IUP_SB_VERT)
    {
      IupSetInt(ih, "YMAX", view_height);

      if (view_width > noscroll_width)  /* affects vertical size */
        canvas_height -= sb_size;
    }
    else
      IupSetAttribute(ih, "YMAX", "0");

    /* check again, adding a scrollbar may affect the other scrollbar need if not done already */
    if (sb & IUP_SB_HORIZ && view_height <= noscroll_height && view_height > canvas_height)
      canvas_width -= sb_size;
    if (sb & IUP_SB_VERT && view_width <= noscroll_width && view_width > canvas_width)
      canvas_height -= sb_size;

    if (canvas_width < 0) canvas_width = 0;
    if (canvas_height < 0) canvas_height = 0;

    if (sb & IUP_SB_HORIZ)
      IupSetInt(ih, "DX", canvas_width);
    else
      IupSetAttribute(ih, "DX", "0");

    if (sb & IUP_SB_VERT)
      IupSetInt(ih, "DY", canvas_height);
    else
      IupSetAttribute(ih, "DY", "0");

    IupSetfAttribute(ih, "LINEY", "%d", ih->data->line_height + ih->data->spacing);
  }
  else
  {
    IupSetAttribute(ih, "XMAX", "0");
    IupSetAttribute(ih, "YMAX", "0");

    IupSetAttribute(ih, "DX", "0");
    IupSetAttribute(ih, "DY", "0");
  }
}


/*******************************************************************************************************/


static int iFlatListRedraw_CB(Ihandle* ih)
{
  iFlatListItem* items = (iFlatListItem*)iupArrayGetData(ih->data->items_array);
  int count = iupArrayCount(ih->data->items_array);
  int text_flags = iupDrawGetTextFlags(ih, "TABSTEXTALIGNMENT", "TABSTEXTWRAP", "TABSTEXTELLIPSIS");
  char* foreground_color = iupAttribGetStr(ih, "FGCOLOR");
  char* background_color = iupAttribGetStr(ih, "BGCOLOR");
  int posx = IupGetInt(ih, "POSX");
  int posy = IupGetInt(ih, "POSY");
  char* back_image = iupAttribGet(ih, "BACKIMAGE");
  int i, x, y, make_inactive = 0;
  int border_width = ih->data->border_width;
  int active = IupGetInt(ih, "ACTIVE");  /* native implementation */
  int focus_feedback = iupAttribGetBoolean(ih, "FOCUSFEEDBACK");
  int width, height;

  IdrawCanvas* dc = iupdrvDrawCreateCanvas(ih);

  iupdrvDrawGetSize(dc, &width, &height);

  iupFlatDrawBox(dc, border_width, width - border_width - 1, border_width, height - border_width - 1, background_color, background_color, 1);

  if (back_image)
  {
    int backimage_zoom = iupAttribGetBoolean(ih, "BACKIMAGEZOOM");
    if (backimage_zoom)
      iupdrvDrawImage(dc, back_image, 0, background_color, border_width, border_width, width - border_width, height - border_width);
    else
      iupdrvDrawImage(dc, back_image, 0, background_color, border_width, border_width, -1, -1);
  }

  if (!active)
    make_inactive = 1;

  x = -posx + border_width;
  y = -posy + border_width;

  for (i = 0; i < count; i++)
  {
    char *fgcolor = (items[i].fgColor) ? items[i].fgColor : foreground_color;
    char *bgcolor = (items[i].bgColor) ? items[i].bgColor : background_color;
    iupFlatDrawBox(dc, x, x + ih->data->line_width - 1, y, y + ih->data->line_height - 1, bgcolor, bgcolor, 1);
    iupFlatDrawIcon(ih, dc, x, y, ih->data->line_width, ih->data->line_height,
                    ih->data->img_position, ih->data->icon_spacing, ih->data->horiz_alignment, ih->data->vert_alignment, ih->data->horiz_padding, ih->data->vert_padding,
                    items[i].image, make_inactive, items[i].name, text_flags, 0, fgcolor, bgcolor, active);

    if (items[i].selected || ih->data->dragging_pos == i + 1)
    {
      unsigned char red, green, blue;
      char* hlcolor = iupAttribGetStr(ih, "HLCOLOR");
      unsigned char a = (unsigned char)iupAttribGetInt(ih, "HLCOLORALPHA");
      long selcolor;

      iupStrToRGB(hlcolor, &red, &green, &blue);
      selcolor = iupDrawColor(red, green, blue, a);

      iupdrvDrawRectangle(dc, x, y, x + ih->data->line_width - 1, y + ih->data->line_height - 1, selcolor, IUP_DRAW_FILL, 1);
    }

    if (ih->data->has_focus && ih->data->focus_pos == i+1 && focus_feedback)
      iupdrvDrawFocusRect(dc, x, y, x + width - border_width - 1, y + ih->data->line_height - 1);

    y += ih->data->line_height + ih->data->spacing;
  }

  if (border_width)
  {
    char* bordercolor = iupAttribGetStr(ih, "BORDERCOLOR");
    iupFlatDrawBorder(dc, 0, width - 1,
                      0, height - 1,
                      border_width, bordercolor, background_color, active);
  }

  iupdrvDrawFlush(dc);

  iupdrvDrawKillCanvas(dc);

  return IUP_DEFAULT;
}

static void iFlatListSelectItem(Ihandle* ih, int pos, int ctrlPressed, int shftPressed)
{
  IFns multi_cb = (IFns)IupGetCallback(ih, "MULTISELECT_CB");
  IFnsii cb = (IFnsii)IupGetCallback(ih, "FLAT_ACTION");
  IFn vc_cb = (IFn)IupGetCallback(ih, "VALUECHANGED_CB");
  iFlatListItem* items = (iFlatListItem*)iupArrayGetData(ih->data->items_array);
  int count = iupArrayCount(ih->data->items_array);

  ih->data->focus_pos = pos;

  if (ih->data->is_multiple)
  {
    int i, start, end;
    char *val = iupAttribGet(ih, "_IUPLIST_LASTSELECTED");
    int last_pos = (val) ? atoi(val) : 0;
    if (pos <= last_pos)
    {
      start = pos;
      end = last_pos;
    }
    else
    {
      start = last_pos;
      end = pos;
    }

    if (!ctrlPressed)
    {
      for (i = 0; i < count; i++)
        items[i].selected = 0;
    }

    if (shftPressed)
    {
      for (i = start; i <= end; i++)
        items[i - 1].selected = 1;
    }
    else
    {
      if (ctrlPressed)
        items[pos - 1].selected = (items[pos - 1].selected) ? 0 : 1; /* toggle selection */
      else
        items[pos - 1].selected = 1;
    }

    if (multi_cb || cb)
    {
      int i, sel_count = 0;
      int* pos = malloc(sizeof(int)*count);
      for (i = 0; i < count; i++)
      {
        if (items[i].selected)
        {
          pos[sel_count] = i + 1;
          sel_count++;
        }
      }
      iFlatListMultipleCallActionCb(ih, cb, multi_cb, vc_cb, pos, sel_count);
      free(pos);
    }
  }
  else
  {
    int i;

    for (i = 0; i < count; i++)
    {
      if (!items[i].selected)
        continue;
      items[i].selected = 0;
      break;
    }
    items[pos - 1].selected = 1;

    if (cb)
      iFlatListSingleCallActionCb(ih, cb, vc_cb, pos);
  }

  if (!shftPressed)
    iupAttribSetInt(ih, "_IUPLIST_LASTSELECTED", pos);
}

static int iFlatListButton_CB(Ihandle* ih, int button, int pressed, int x, int y, char* status)
{
  IFniiiis button_cb = (IFniiiis)IupGetCallback(ih, "FLAT_BUTTON_CB");
  int itemDragged = iupAttribGetInt(ih, "_IUPLIST_ITEMDRAGGED");
  int pos = iFlatListConvertXYToPos(ih, x, y);

  if (button_cb)
  {
    if (button_cb(ih, button, pressed, x, y, status) == IUP_IGNORE)
      return IUP_DEFAULT;
  }

  if (pos == -1)
    return IUP_DEFAULT;

  if (button == IUP_BUTTON1 && !pressed && itemDragged > 0)
  {
    iFlatListItem* items = (iFlatListItem*)iupArrayGetData(ih->data->items_array);
    iFlatListCopyItem(ih, itemDragged, pos, 0);
    iFlatListUnSelectedAll(ih);
    items[pos - 1].selected = 1;
    ih->data->dragging_pos = 0;
    iupAttribSetInt(ih, "_IUPLIST_ITEMDRAGGED", 0);
  }

  if (button == IUP_BUTTON1 && pressed)
    iFlatListSelectItem(ih, pos, iup_iscontrol(status), iup_isshift(status));

  if (iup_isdouble(status))
  {
    IFnis dc_cb = (IFnis)IupGetCallback(ih, "DBLCLICK_CB");
    if (dc_cb)
    {
      iFlatListItem* items = (iFlatListItem*)iupArrayGetData(ih->data->items_array);
      if (dc_cb(ih, pos, items[pos - 1].name) == IUP_IGNORE)
        return IUP_DEFAULT;
    }
  }

  IupUpdate(ih);

  return IUP_DEFAULT;
}

static int iFlatListMotion_CB(Ihandle* ih, int x, int y, char* status)
{
  IFniis motion_cb = (IFniis)IupGetCallback(ih, "FLAT_MOTION_CB");
  int pos, itemDragged;

  iupFlatScrollBarMotionUpdate(ih, x, y);

  if (motion_cb)
  {
    if (motion_cb(ih, x, y, status) == IUP_IGNORE)
      return IUP_DEFAULT;
  }

  if (!iup_isbutton1(status) || ih->data->is_multiple || !ih->data->show_dragdrop)
    return IUP_IGNORE;

  pos = iFlatListConvertXYToPos(ih, x, y);

  if (pos == -1)
    return IUP_DEFAULT;

  itemDragged = iupAttribGetInt(ih, "_IUPLIST_ITEMDRAGGED");
  if (itemDragged == 0)
    iupAttribSetInt(ih, "_IUPLIST_ITEMDRAGGED", pos);

  //if (y < 0 || y > ih->currentheight)
  //{
  //  int posy = (pos - 1) * (ih->data->line_height + ih->data->spacing);
  //  IupSetInt(ih, "POSY", posy);
  //}

  ih->data->dragging_pos = pos;

  IupUpdate(ih);

  return IUP_DEFAULT;
}

static int iFlatListFocus_CB(Ihandle* ih, int focus)
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

static int iFlatListResize_CB(Ihandle* ih, int width, int height)
{
  (void)width;
  (void)height;

  iFlatListUpdateScrollBar(ih);

  return IUP_DEFAULT;
}

static void iFlatListSingleCallDblClickCb(Ihandle* ih, IFnis cb, int pos)
{
  char *text;

  if (pos < 1)
    return;

  text = IupGetAttributeId(ih, "", pos);

  if (cb(ih, pos, text) == IUP_CLOSE)
    IupExitLoop();
}

static void iFlatListCallActionCallback(Ihandle* ih, IFnsii cb, int pos, int state)
{
  char *text;

  if (pos < 1)
    return;

  text = IupGetAttributeId(ih, "", pos);

  if (cb(ih, text, pos, state) == IUP_CLOSE)
    IupExitLoop();
}

static void iFlatListUpdateOldValue(Ihandle* ih, int pos, int removed)
{
  char* old_value = iupAttribGet(ih, "_IUPLIST_OLDVALUE");
  if (old_value)
  {
    int old_pos = atoi(old_value) - 1; /* was in IUP reference, starting at 1 */
    if (!ih->data->is_multiple)
    {
      if (old_pos >= pos)
      {
        if (removed && old_pos == pos)
        {
          /* when the current item is removed nothing remains selected */
          iupAttribSet(ih, "_IUPLIST_OLDVALUE", NULL);
        }
        else
          iupAttribSetInt(ih, "_IUPLIST_OLDVALUE", removed ? old_pos - 1 : old_pos + 1);
      }
    }
    else
    {
      /* multiple selection on a non drop-down list. */
      char* value = IupGetAttribute(ih, "VALUE");
      iupAttribSetStr(ih, "_IUPLIST_OLDVALUE", value);
    }
  }
}

static void iFlatListSingleCallActionCb(Ihandle* ih, IFnsii cb, IFn valuechanged_cb, int pos)
{
  char* old_str = iupAttribGet(ih, "_IUPLIST_OLDVALUE");
  int unchanged = 1;

  if (old_str)
  {
    int oldpos = atoi(old_str);
    if (oldpos != pos)
    {
      iFlatListCallActionCallback(ih, cb, oldpos, 0);
      iupAttribSetInt(ih, "_IUPLIST_OLDVALUE", pos);
      iFlatListCallActionCallback(ih, cb, pos, 1);
      unchanged = 0;
    }
  }
  else
  {
    iFlatListItem* items = (iFlatListItem*)iupArrayGetData(ih->data->items_array);
    iupAttribSetInt(ih, "_IUPLIST_OLDVALUE", pos);
    iFlatListCallActionCallback(ih, cb, pos, 1);
    if (items[pos - 1].selected)
      unchanged = 0;
  }

  if (!unchanged && valuechanged_cb)
    valuechanged_cb(ih);
}

static void iFlatListMultipleCallActionCb(Ihandle* ih, IFnsii cb, IFns multi_cb, IFn valuechanged_cb, int* pos, int sel_count)
{
  int i, count = iupArrayCount(ih->data->items_array);

  char* old_str = iupAttribGet(ih, "_IUPLIST_OLDVALUE");
  int old_count = old_str ? (int)strlen(old_str) : 0;
  int unchanged = 1;

  char* str = malloc(count + 1);
  memset(str, '-', count);
  str[count] = 0;
  for (i = 0; i < sel_count; i++)
    str[pos[i] - 1] = '+';

  if (old_count != count)
  {
    old_count = 0;
    old_str = NULL;
  }

  if (multi_cb)
  {
    unchanged = 1;

    for (i = 0; i < count && old_str; i++)
    {
      if (str[i] == old_str[i])
        str[i] = 'x';    /* mark unchanged values */
      else
        unchanged = 0;
    }

    if (old_str && unchanged)
    {
      free(str);
      return;
    }

    if (multi_cb(ih, str) == IUP_CLOSE)
      IupExitLoop();

    for (i = 0; i < count && old_str; i++)
    {
      if (str[i] == 'x')
        str[i] = old_str[i];    /* restore unchanged values */
    }
  }
  else
  {
    /* must simulate the click on each item */
    for (i = 0; i < count; i++)
    {
      unchanged = 1;
      if (i >= old_count)  /* new items, if selected then call the callback */
      {
        if (str[i] == '+')
          iFlatListCallActionCallback(ih, cb, i + 1, 1);
        unchanged = 0;
      }
      else if (str[i] != old_str[i])
      {
        if (str[i] == '+')
          iFlatListCallActionCallback(ih, cb, i + 1, 1);
        else
          iFlatListCallActionCallback(ih, cb, i + 1, 0);
        unchanged = 0;
      }
    }
  }

  if (!unchanged && valuechanged_cb)
    valuechanged_cb(ih);

  iupAttribSetStr(ih, "_IUPLIST_OLDVALUE", str);
  free(str);
}

static int iFlatListKUp_CB(Ihandle* ih)
{
  if (ih->data->has_focus)
  {
    if (ih->data->focus_pos > 1)
    {
      int ctrlPressed = 0; /* behave as no ctrl key pressed when using arrow keys */
      int shftPressed = IupGetInt(NULL, "SHIFTKEY");

      iFlatListSelectItem(ih, ih->data->focus_pos - 1, ctrlPressed, shftPressed);

      IupUpdate(ih);
    }
  }
  return IUP_DEFAULT;
}

static int iFlatListKDown_CB(Ihandle* ih)
{
  if (ih->data->has_focus)
  {
    int count = iupArrayCount(ih->data->items_array);
    if (ih->data->focus_pos < count)
    {
      int ctrlPressed = 0; /* behave as no ctrl key pressed when using arrow keys */
      int shftPressed = IupGetInt(NULL, "SHIFTKEY");

      iFlatListSelectItem(ih, ih->data->focus_pos + 1, ctrlPressed, shftPressed);

      IupUpdate(ih);
    }
  }
  return IUP_DEFAULT;
}



/******************************************************************************************/


static int iFlatListSetAlignmentAttrib(Ihandle* ih, const char* value)
{
  char value1[30], value2[30];

  iupStrToStrStr(value, value1, value2, ':');

  ih->data->horiz_alignment = iupFlatGetHorizontalAlignment(value1);
  ih->data->vert_alignment = iupFlatGetVerticalAlignment(value2);

  if (ih->handle)
    iupdrvRedrawNow(ih);

  return 1;
}

static char* iFlatListGetAlignmentAttrib(Ihandle *ih)
{
  char* horiz_align2str[3] = { "ALEFT", "ACENTER", "ARIGHT" };
  char* vert_align2str[3] = { "ATOP", "ACENTER", "ABOTTOM" };
  return iupStrReturnStrf("%s:%s", horiz_align2str[ih->data->horiz_alignment], vert_align2str[ih->data->vert_alignment]);
}

static char* iFlatListGetSpacingAttrib(Ihandle* ih)
{
  return iupStrReturnInt(ih->data->spacing);
}

static char* iFlatListGetHasFocusAttrib(Ihandle* ih)
{
  return iupStrReturnBoolean(ih->data->has_focus);
}

static char* iFlatListGetIdValueAttrib(Ihandle* ih, int pos)
{
  iFlatListItem* items = (iFlatListItem*)iupArrayGetData(ih->data->items_array);
  int count = iupArrayCount(ih->data->items_array);

  if (pos < 1 || pos > count)
    return 0;

  if (items)
    return items[pos - 1].name;
  return NULL;
}

static int iFlatListSetIdValueAttrib(Ihandle* ih, int pos, const char* value)
{
  int count = iupArrayCount(ih->data->items_array);

  if (pos < 1)
    return 0;

  if (!value)
  {
    iFlatListRemoveItem(ih, 0, count - pos - 1);
    iupAttribSet(ih, "_IUPLIST_OLDVALUE", NULL);
  }
  else if (pos <= count)
  {
    iFlatListItem* items = (iFlatListItem*)iupArrayInsert(ih->data->items_array, pos - 1, 1);
    if (items)
      items[pos - 1].name = iupStrDup(value);
  }
  else
  {
    iFlatListItem* items = (iFlatListItem*)iupArrayInsert(ih->data->items_array, count, pos - 1 - count + 1);
    if (items)
      items[pos - 1].name = iupStrDup(value);
  }

  iFlatListUpdateOldValue(ih, pos, 0);

  if (ih->handle)
  {
    iFlatListUpdateScrollBar(ih);
    IupUpdate(ih);
  }

  return 0;
}

static int iFlatListSetAppendItemAttrib(Ihandle* ih, const char* value)
{
  if (value)
  {
    iFlatListItem* items = (iFlatListItem*)iupArrayInc(ih->data->items_array);
    if (items)
    {
      int index = iupArrayCount(ih->data->items_array) - 1;
      items[index].name = iupStrDup(value);
    }
  }

  if (ih->handle)
  {
    iFlatListUpdateScrollBar(ih);
    IupUpdate(ih);
  }
  return 0;
}

static int iFlatListSetInsertItemAttrib(Ihandle* ih, int pos, const char* value)
{
  int count = iupArrayCount(ih->data->items_array);

  if (pos < 1 || pos > count)
    return 0;

  if (value)
  {
    iFlatListItem* items = (iFlatListItem*)iupArrayInsert(ih->data->items_array, pos-1, 1);
    if (items)
      items[pos - 1].name = iupStrDup(value);
  }

  iFlatListUpdateOldValue(ih, pos, 0);

  if (ih->handle)
  {
    iFlatListUpdateScrollBar(ih);
    IupUpdate(ih);
  }

  return 0;
}

static int iFlatListSetRemoveItemAttrib(Ihandle* ih, const char* value)
{
  if (!value || iupStrEqualNoCase(value, "ALL"))
  {
    iFlatListRemoveItem(ih, 0, iupArrayCount(ih->data->items_array));
    iupAttribSet(ih, "_IUPLIST_OLDVALUE", NULL);
  }
  else
  {
    int id;
    if (iupStrToInt(value, &id))
    {
      iFlatListRemoveItem(ih, id - 1, 1);
      iFlatListUpdateOldValue(ih, id - 1, 1);
    }
  }

  if (ih->handle)
  {
    iFlatListUpdateScrollBar(ih);
    IupUpdate(ih);
  }

  return 0;
}

static int iFlatListSetImageAttrib(Ihandle* ih, int pos, const char* value)
{
  int count = iupArrayCount(ih->data->items_array);

  if (pos < 1 || pos > count)
    return 0;

  iFlatListItem *items = (iFlatListItem *)iupArrayGetData(ih->data->items_array);
  if (items)
  {
    if (items[pos - 1].image)
      free(items[pos - 1].image);

    items[pos-1].image = iupStrDup(value);
  }

  if (ih->handle)
  {
    iFlatListUpdateScrollBar(ih);
    IupUpdate(ih);
  }

  return 0;
}

static int iFlatListSetImagePositionAttrib(Ihandle* ih, const char* value)
{
  ih->data->img_position = iupFlatGetImagePosition(value);

  if (ih->handle)
    IupUpdate(ih);

  return 0;
}

static char* iFlatListGetImagePositionAttrib(Ihandle *ih)
{
  char* img_pos2str[4] = { "LEFT", "RIGHT", "TOP", "BOTTOM" };
  return img_pos2str[ih->data->img_position];
}

static char* iFlatListGetShowDragDropAttrib(Ihandle* ih)
{
  return iupStrReturnBoolean(ih->data->show_dragdrop);
}

static int iFlatListSetShowDragDropAttrib(Ihandle* ih, const char* value)
{
  /* valid only before map */
  if (ih->handle)
    return 0;

  if (iupStrBoolean(value))
    ih->data->show_dragdrop = 1;
  else
    ih->data->show_dragdrop = 0;

  return 0;
}

static int iFlatListDropData_CB(Ihandle *ih, char* type, void* data, int len, int x, int y)
{
  int pos = IupConvertXYToPos(ih, x, y);
  int is_ctrl = 0;
  char key[5];

  /* Data is not the pointer, it contains the pointer */
  Ihandle* ih_source;
  memcpy((void*)&ih_source, data, len);

  /* A copy operation is enabled with the CTRL key pressed, or else a move operation will occour.
  A move operation will be possible only if the attribute DRAGSOURCEMOVE is Yes.
  When no key is pressed the default operation is copy when DRAGSOURCEMOVE=No and move when DRAGSOURCEMOVE=Yes. */
  iupdrvGetKeyState(key);
  if (key[1] == 'C')
    is_ctrl = 1;

  if (ih_source->data->is_multiple)
  {
    char *buffer = IupGetAttribute(ih_source, "VALUE");

    /* Copy all selected items */
    int i = 1;  /* IUP starts at 1 */
    while (buffer[i - 1] != '\0')
    {
      if (buffer[i - 1] == '+')
      {
        iFlatListItem* srcItems = (iFlatListItem*)iupArrayGetData(ih_source->data->items_array);
        iFlatListItem* items = (iFlatListItem*)iupArrayInsert(ih->data->items_array, pos - 1, 1);
        if (srcItems && items)
        {
          items[pos - 1].name = iupStrDup(srcItems[i].name);
          items[pos - 1].image = iupStrDup(srcItems[i].image);
          items[pos - 1].fgColor = iupStrDup(srcItems[i].fgColor);
          items[pos - 1].bgColor = iupStrDup(srcItems[i].bgColor);
          items[pos - 1].selected = 0;
        }
      }

      i++;
    }

    if (IupGetInt(ih_source, "DRAGSOURCEMOVE") && !is_ctrl)
    {
      /* Remove all item from source if MOVE */
      i = 1;  /* IUP starts at 1 */
      while (*buffer != '\0')
      {
        if (*buffer == '+')
        {
          iFlatListRemoveItem(ih_source, i - 1, 1);
        }

        i++;
        buffer++;
      }
    }
  }
  else
  {
    iFlatListItem* srcItems = (iFlatListItem*)iupArrayGetData(ih_source->data->items_array);
    iFlatListItem* items = (iFlatListItem*)iupArrayInsert(ih->data->items_array, pos - 1, 1);
    if (srcItems && items)
    {
      int srcPos = IupGetInt(ih_source, "VALUE");
      items[pos - 1].name = iupStrDup(srcItems[srcPos - 1].name);
      items[pos - 1].image = iupStrDup(srcItems[srcPos - 1].image);
      items[pos - 1].fgColor = iupStrDup(srcItems[srcPos - 1].fgColor);
      items[pos - 1].bgColor = iupStrDup(srcItems[srcPos - 1].bgColor);
      items[pos - 1].selected = 0;
    }

    if (IupGetInt(ih_source, "DRAGSOURCEMOVE") && !is_ctrl)
    {
      int srcPos = iupAttribGetInt(ih_source, "_IUP_LIST_SOURCEPOS");
      iFlatListRemoveItem(ih_source, srcPos - 1, 1);
    }
  }

  IupUpdate(ih);

  (void)type;
  return IUP_DEFAULT;
}

static int iFlatListDragData_CB(Ihandle *ih, char* type, void *data, int len)
{
  int pos = iupAttribGetInt(ih, "_IUP_LIST_SOURCEPOS");
  if (pos < 1)
    return IUP_DEFAULT;

  if (ih->data->is_multiple)
  {
    char *buffer = IupGetAttribute(ih, "VALUE");

    /* It will not drag all selected items only
    when the user begins to drag an item not selected.
    In this case, unmark all and mark only this item.  */
    if (buffer[pos - 1] == '-')
    {
      int len = (int)strlen(buffer);
      IupSetAttribute(ih, "SELECTION", "NONE");
      memset(buffer, '-', len);
      buffer[pos - 1] = '+';
      IupSetAttribute(ih, "VALUE", buffer);
    }
  }
  else
  {
    /* Single selection */
    IupSetInt(ih, "VALUE", pos);
  }

  /* Copy source handle */
  memcpy(data, (void*)&ih, len);

  (void)type;
  return IUP_DEFAULT;
}


static int iFlatListDragDataSize_CB(Ihandle* ih, char* type)
{
  (void)ih;
  (void)type;
  return sizeof(Ihandle*);
}

static int iFlatListDragBegin_CB(Ihandle* ih, int x, int y)
{
  int pos = IupConvertXYToPos(ih, x, y);
  iupAttribSetInt(ih, "_IUP_LIST_SOURCEPOS", pos);
  return IUP_DEFAULT;
}

static int iFlatListDragEnd_CB(Ihandle *ih, int del)
{
  iupAttribSetInt(ih, "_IUP_LIST_SOURCEPOS", 0);
  (void)del;
  return IUP_DEFAULT;
}

static int iFlatListSetDragDropListAttrib(Ihandle* ih, const char* value)
{
  if (iupStrBoolean(value))
  {
    /* Register callbacks to enable drag and drop between lists */
    IupSetCallback(ih, "DRAGBEGIN_CB", (Icallback)iFlatListDragBegin_CB);
    IupSetCallback(ih, "DRAGDATASIZE_CB", (Icallback)iFlatListDragDataSize_CB);
    IupSetCallback(ih, "DRAGDATA_CB", (Icallback)iFlatListDragData_CB);
    IupSetCallback(ih, "DRAGEND_CB", (Icallback)iFlatListDragEnd_CB);
    IupSetCallback(ih, "DROPDATA_CB", (Icallback)iFlatListDropData_CB);
  }
  else
  {
    /* Unregister callbacks */
    IupSetCallback(ih, "DRAGBEGIN_CB", NULL);
    IupSetCallback(ih, "DRAGDATASIZE_CB", NULL);
    IupSetCallback(ih, "DRAGDATA_CB", NULL);
    IupSetCallback(ih, "DRAGEND_CB", NULL);
    IupSetCallback(ih, "DROPDATA_CB", NULL);
  }

  return 1;
}

static int iFlatListSetIconSpacingAttrib(Ihandle* ih, const char* value)
{
  iupStrToInt(value, &ih->data->icon_spacing);
  if (ih->handle)
    IupUpdate(ih);
  return 0;
}

static char* iFlatListGetIconSpacingAttrib(Ihandle *ih)
{
  return iupStrReturnInt(ih->data->icon_spacing);
}

static char* iFlatListGetCountAttrib(Ihandle* ih)
{
  return iupStrReturnInt(iupArrayCount(ih->data->items_array));
}

static int iFlatListSetValueAttrib(Ihandle* ih, const char* value)
{
  iFlatListItem* items = (iFlatListItem*)iupArrayGetData(ih->data->items_array);
  if (ih->data->is_multiple)
  {
    int i;
    int count = iupArrayCount(ih->data->items_array);

    if (!value)
    {
      iupAttribSet(ih, "_IUPLIST_OLDVALUE", NULL);
      return 0;
    }

    int len = (int)strlen(value);
    if (len != count)
      return 1;
    for (i = 0; i < count; i++)
    {
      if (value[i] == '+')
      {
        items[i].selected = 1;
        iupAttribSetInt(ih, "_IUPLIST_LASTSELECTED", i);
      }
      else if (value[i] == '-')
        items[i].selected = 0;
      else
        return 1;
    }

    iupAttribSetStr(ih, "_IUPLIST_OLDVALUE", value);
  }
  else
  {
    int pos;
    int count = iupArrayCount(ih->data->items_array);
    if (iupStrToInt(value, &pos) == 1 && pos > 0 && pos <= count)
    {
      items[pos - 1].selected = 1;
      iupAttribSetInt(ih, "_IUPLIST_OLDVALUE", pos);
    }
    else
      iupAttribSet(ih, "_IUPLIST_OLDVALUE", NULL);

    iupAttribSetInt(ih, "_IUPLIST_LASTSELECTED", pos);
  }

  if (ih->handle)
    IupUpdate(ih);

  return 0;
}

static char* iFlatListGetValueStringAttrib(Ihandle* ih)
{
  if (!ih->data->is_multiple)
  {
    int i = IupGetInt(ih, "VALUE");
    return IupGetAttributeId(ih, "", i);
  }
  return NULL;
}

static int iFlatListSetValueStringAttrib(Ihandle* ih, const char* value)
{
  if (!ih->data->is_multiple)
  {
    int i;
    int count = iupArrayCount(ih->data->items_array);

    for (i = 1; i <= count; i++)
    {
      char* item = IupGetAttributeId(ih, "", i);
      if (iupStrEqual(value, item))
      {
        IupSetInt(ih, "VALUE", i);
        return 0;
      }
    }
  }

  return 0;
}

static char* iFlatListGetValueAttrib(Ihandle* ih)
{
  iFlatListItem* items = (iFlatListItem*)iupArrayGetData(ih->data->items_array);
  int count = iupArrayCount(ih->data->items_array);
  int i;
  char *retval = NULL;

  if (ih->data->is_multiple)
  {
    char *val = (char *)malloc((count + 1)*sizeof(char));
    for (i = 0; i < count; i++)
      val[i] = (items[i].selected) ? '+' : '-';
    val[i] = '\0';
    retval = iupStrReturnStr(val);
    free(val);
    return retval;
  }
  else
  {
    for (i = 0; i < count; i++)
    {
      if (items[i].selected == 0)
        continue;
      retval = iupStrReturnInt(i + 1);
      break;
    }
  }

  return retval;
}

static int iFlatListSetSpacingAttrib(Ihandle* ih, const char* value)
{
  iupStrToInt(value, &ih->data->spacing);
  if (ih->handle)
    IupUpdate(ih);
  return 0;
}

static char* iFlatListGetFGColorAttrib(Ihandle* ih, int id)
{
  iFlatListItem* items = (iFlatListItem*)iupArrayGetData(ih->data->items_array);
  if (items)
  {
    id--;
    return items[id].fgColor;
  }
  return NULL;
}

static int iFlatListSetFGColorAttrib(Ihandle* ih, int id, const char* value)
{
  iFlatListItem *items = (iFlatListItem *)iupArrayGetData(ih->data->items_array);
  if (items)
  {
    id--;

    if (items[id].fgColor)
      free(items[id].fgColor);

    items[id].fgColor = iupStrDup(value);
  }

  if (ih->handle)
    IupUpdate(ih);

  return 0;
}

static char* iFlatListGetBGColorAttrib(Ihandle* ih, int id)
{
  iFlatListItem* items = (iFlatListItem*)iupArrayGetData(ih->data->items_array);
  if (items)
  {
    id--;
    return items[id].bgColor;
  }
  return NULL;
}

static int iFlatListSetBGColorAttrib(Ihandle* ih, int id, const char* value)
{
  iFlatListItem *items = (iFlatListItem *)iupArrayGetData(ih->data->items_array);
  if (items)
  {
    id--;

    if (items[id].bgColor)
      free(items[id].bgColor);

    items[id].bgColor = iupStrDup(value);
  }

  if (ih->handle)
    IupUpdate(ih);

  return 0;
}

static int iFlatListSetPaddingAttrib(Ihandle* ih, const char* value)
{
  iupStrToIntInt(value, &ih->data->horiz_padding, &ih->data->vert_padding, 'x');
  if (ih->handle)
    IupUpdate(ih);
  return 0;
}

static char* iFlatListGetPaddingAttrib(Ihandle* ih)
{
  return iupStrReturnIntInt(ih->data->horiz_padding, ih->data->vert_padding, 'x');
}

static int iFlatListSetMultipleAttrib(Ihandle* ih, const char* value)
{
  /* valid only before map */
  if (ih->handle)
    return 0;

  if (iupStrBoolean(value))
    ih->data->is_multiple = 1;
  else
    ih->data->is_multiple = 0;

  return 0;
}

static char* iFlatListGetMultipleAttrib(Ihandle* ih)
{
  return iupStrReturnBoolean(ih->data->is_multiple);
}

static int iFlatListSetTopItemAttrib(Ihandle* ih, const char* value)
{
  int pos = 1;
  if (iupStrToInt(value, &pos))
  {
    int count = iupArrayCount(ih->data->items_array);
    int posy;

    if (pos < 1 || pos > count)
      return 0;

    posy = (pos - 1) * (ih->data->line_height + ih->data->spacing);
    IupSetInt(ih, "POSY", posy);

    IupUpdate(ih);
  }
  return 0;
}

static int iFlatListCallDragDropCb(Ihandle* ih, int drag_id, int drop_id, int *is_ctrl)
{
  IFniiii cbDragDrop = (IFniiii)IupGetCallback(ih, "DRAGDROP_CB");
  int is_shift = 0;
  char key[5];
  iupdrvGetKeyState(key);
  if (key[0] == 'S')
    is_shift = 1;
  if (key[1] == 'C')
    *is_ctrl = 1;
  else
    *is_ctrl = 0;

  /* ignore a drop that will do nothing */
  if ((*is_ctrl) == 0 && (drag_id + 1 == drop_id || drag_id == drop_id))
    return IUP_DEFAULT;
  if ((*is_ctrl) != 0 && drag_id == drop_id)
    return IUP_DEFAULT;

  drag_id++;
  if (drop_id < 0)
    drop_id = -1;
  else
    drop_id++;

  if (cbDragDrop)
    return cbDragDrop(ih, drag_id, drop_id, is_shift, *is_ctrl);  /* starts at 1 */

  return IUP_CONTINUE; /* allow to move/copy by default if callback not defined */
}

static char* iListGetShowDragDropAttrib(Ihandle* ih)
{
  return iupStrReturnBoolean(ih->data->show_dragdrop);
}

static int iListSetShowDragDropAttrib(Ihandle* ih, const char* value)
{
  /* valid only before map */
  if (ih->handle)
    return 0;

  if (iupStrBoolean(value))
    ih->data->show_dragdrop = 1;
  else
    ih->data->show_dragdrop = 0;

  return 0;
}

static int iFlatListWheel_CB(Ihandle* ih, float delta)
{
  iupFlatScrollBarWheelUpdate(ih, delta);
  return IUP_DEFAULT;
}

static int iFlatListSetFlatScrollbarAttrib(Ihandle* ih, const char* value)
{
  /* can only be set before map */
  if (ih->handle)
    return IUP_DEFAULT;

  if (value && !iupStrEqualNoCase(value, "NO"))
  {
    if (iupFlatScrollBarCreate(ih))
    {
      IupSetAttribute(ih, "SCROLLBAR", "NO");
      IupSetCallback(ih, "WHEEL_CB", (Icallback)iFlatListWheel_CB);
    }
    return 1;
  }
  else
    return 0;
}

static int iFlatListSetBorderWidthAttrib(Ihandle* ih, const char* value)
{
  iupStrToInt(value, &ih->data->border_width);
  if (ih->handle)
    iupdrvRedrawNow(ih);
  return 0;
}

static char* iFlatListGetBorderWidthAttrib(Ihandle *ih)
{
  return iupStrReturnInt(ih->data->border_width);
}

static int iFlatListSetAttribPostRedraw(Ihandle* ih, const char* value)
{
  (void)value;
  if (ih->handle)
    iupdrvPostRedraw(ih);
  return 1;
}

/*****************************************************************************************/

static void iFlatListComputeNaturalSizeMethod(Ihandle* ih, int *w, int *h, int *children_expand)
{
  int num_lines;
  int fit2backimage = iupAttribGetBoolean(ih, "FITTOBACKIMAGE");
  int visiblecolumns = iupAttribGetInt(ih, "VISIBLECOLUMNS");
  int visiblelines = iupAttribGetInt(ih, "VISIBLELINES");
  char* back_image = iupAttribGet(ih, "BACKIMAGE");
  iFlatListItem* items = (iFlatListItem*)iupArrayGetData(ih->data->items_array);
  int count = iupArrayCount(ih->data->items_array);
  int sb, max_h, max_w;

  (void)children_expand; /* unset if not a container */

  if (fit2backimage && back_image)
  {
    iupImageGetInfo(back_image, w, h, NULL);
    *w += 2 * ih->data->border_width;
    *h += 2 * ih->data->border_width;

    if (iupAttribGetBoolean(ih, "BORDER")) /* native border around scrollbars */
    {
      *w += 2;
      *h += 2;
    }
    return;
  }

  iFlatListCalcItemMaxSize(ih, items, count, &max_w, &max_h);

  if (visiblecolumns)
  {
    *w = iupdrvFontGetStringWidth(ih, "WWWWWWWWWW");
    *w = (visiblecolumns*(*w)) / 10;
  }
  else
    *w = max_w;

  if (visiblelines)
    num_lines = visiblelines;
  else
    num_lines = count;

  *h = max_h * num_lines;
  *h += (num_lines - 1) * ih->data->spacing;

  sb = iFlatListGetScrollbar(ih);
  if (sb)
  {
    int sb_size = iFlatListGetScrollbarSize(ih);

    if (sb & IUP_SB_VERT && visiblelines && visiblelines < count)
      *w += sb_size;  /* affects horizontal size (width) */

    if (sb & IUP_SB_HORIZ && visiblecolumns && visiblecolumns < max_w)
      *h += sb_size;  /* affects vertical size (height) */
  }

  *w += 2 * ih->data->border_width;
  *h += 2 * ih->data->border_width;

  if (iupAttribGetBoolean(ih, "BORDER")) /* native border around scrollbars */
  {
    *w += 2;
    *h += 2;
  }
}

static void iFlatListSetChildrenCurrentSizeMethod(Ihandle* ih, int shrink)
{
  if (iupFlatScrollBarGet(ih) != IUP_SB_NONE)
    iupFlatScrollBarSetChildrenCurrentSize(ih, shrink);
}

static void iFlatListSetChildrenPositionMethod(Ihandle* ih, int x, int y)
{
  if (iupFlatScrollBarGet(ih) != IUP_SB_NONE)
    iupFlatScrollBarSetChildrenPosition(ih);

  (void)x;
  (void)y;
}

static void iFlatListDestroyMethod(Ihandle* ih)
{
  if (ih->data->items_array)
  {
    int i;
    iFlatListItem* items = iupArrayGetData(ih->data->items_array);
    for (i = 0; i < iupArrayCount(ih->data->items_array); i++)
    {
      if (items[i].name)
        free(items[i].name);

      if (items[i].image)
        free(items[i].image);

      if (items[i].fgColor)
        free(items[i].fgColor);

      if (items[i].bgColor)
        free(items[i].bgColor);
    }
    iupArrayDestroy(ih->data->items_array);
  }
}

static int iFlatListCreateMethod(Ihandle* ih, void** params)
{
  (void)params;

  /* free the data allocated by IupCanvas */
  free(ih->data);
  ih->data = iupALLOCCTRLDATA();

  /* non zero default values */
  ih->data->border_width = 0;
  ih->data->spacing = 0;
  ih->data->horiz_alignment = IUP_ALIGN_ALEFT;
  ih->data->vert_alignment = IUP_ALIGN_ACENTER;
  ih->data->horiz_padding = 2;
  ih->data->vert_padding = 2;

  ih->data->items_array = iupArrayCreate(20, sizeof(iFlatListItem));

  IupSetCallback(ih, "_IUP_XY2POS_CB", (Icallback)iFlatListConvertXYToPos);

  /* internal callbacks */
  IupSetCallback(ih, "ACTION", (Icallback)iFlatListRedraw_CB);
  IupSetCallback(ih, "BUTTON_CB", (Icallback)iFlatListButton_CB);
  IupSetCallback(ih, "MOTION_CB", (Icallback)iFlatListMotion_CB);
  IupSetCallback(ih, "RESIZE_CB", (Icallback)iFlatListResize_CB);
  IupSetCallback(ih, "FOCUS_CB", (Icallback)iFlatListFocus_CB);
  IupSetCallback(ih, "K_UP", (Icallback)iFlatListKUp_CB);
  IupSetCallback(ih, "K_DOWN", (Icallback)iFlatListKDown_CB);
  IupSetCallback(ih, "K_sUP", (Icallback)iFlatListKUp_CB);
  IupSetCallback(ih, "K_sDOWN", (Icallback)iFlatListKDown_CB);
  IupSetCallback(ih, "K_cUP", (Icallback)iFlatListKUp_CB);
  IupSetCallback(ih, "K_cDOWN", (Icallback)iFlatListKDown_CB);

  return IUP_NOERROR;
}


/******************************************************************************/


Ihandle* IupFlatList(void)
{
  return IupCreate("flatlist");
}

Iclass* iupFlatListNewClass(void)
{
  Iclass* ic = iupClassNew(iupRegisterFindClass("canvas"));

  ic->name = "flatlist";
  ic->format = NULL;  /* no parameters */
  ic->nativetype = IUP_TYPECANVAS;
  ic->childtype = IUP_CHILDNONE;
  ic->is_interactive = 1;
  ic->has_attrib_id = 1;

  /* Class functions */
  ic->New = iupFlatListNewClass;
  ic->Create = iFlatListCreateMethod;
  ic->Destroy = iFlatListDestroyMethod;
  ic->ComputeNaturalSize = iFlatListComputeNaturalSizeMethod;
  ic->SetChildrenCurrentSize = iFlatListSetChildrenCurrentSizeMethod;
  ic->SetChildrenPosition = iFlatListSetChildrenPositionMethod;

  /* Callbacks */
  iupClassRegisterCallback(ic, "FLAT_ACTION", "sii");
  iupClassRegisterCallback(ic, "MULTISELECT_CB", "s");
  iupClassRegisterCallback(ic, "DBLCLICK_CB", "is");
  iupClassRegisterCallback(ic, "VALUECHANGED_CB", "");
  iupClassRegisterCallback(ic, "DRAGDROP_CB", "iiii");
  iupClassRegisterCallback(ic, "FLAT_BUTTON_CB", "iiiis");
  iupClassRegisterCallback(ic, "FLAT_MOTION_CB", "iis");
  iupClassRegisterCallback(ic, "FLAT_FOCUS_CB", "i");

  iupClassRegisterAttributeId(ic, "IDVALUE", iFlatListGetIdValueAttrib, iFlatListSetIdValueAttrib, IUPAF_NOT_MAPPED);
  iupClassRegisterAttribute(ic, "MULTIPLE", iFlatListGetMultipleAttrib, iFlatListSetMultipleAttrib, NULL, NULL, IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "COUNT", iFlatListGetCountAttrib, NULL, NULL, NULL, IUPAF_READONLY | IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "VALUE", iFlatListGetValueAttrib, iFlatListSetValueAttrib, NULL, NULL, IUPAF_NO_SAVE | IUPAF_NO_DEFAULTVALUE | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "VALUESTRING", iFlatListGetValueStringAttrib, iFlatListSetValueStringAttrib, NULL, NULL, IUPAF_NO_DEFAULTVALUE | IUPAF_NO_INHERIT);

  iupClassRegisterAttribute(ic, "BORDERCOLOR", NULL, NULL, IUPAF_SAMEASSYSTEM, "50 150 255", IUPAF_DEFAULT);  /* inheritable */
  iupClassRegisterAttribute(ic, "BORDERWIDTH", iFlatListGetBorderWidthAttrib, iFlatListSetBorderWidthAttrib, IUPAF_SAMEASSYSTEM, "0", IUPAF_NOT_MAPPED);  /* inheritable */
  iupClassRegisterAttribute(ic, "FGCOLOR", NULL, iFlatListSetAttribPostRedraw, "0 0 0", NULL, IUPAF_NOT_MAPPED);  /* force the new default value */
  iupClassRegisterAttribute(ic, "BGCOLOR", NULL, iFlatListSetAttribPostRedraw, "255 255 255", NULL, IUPAF_NOT_MAPPED);  /* force the new default value */
  iupClassRegisterAttributeId(ic, "ITEMFGCOLOR", iFlatListGetFGColorAttrib, iFlatListSetFGColorAttrib, IUPAF_NO_INHERIT | IUPAF_NOT_MAPPED);
  iupClassRegisterAttributeId(ic, "ITEMBGCOLOR", iFlatListGetBGColorAttrib, iFlatListSetBGColorAttrib, IUPAF_NO_INHERIT | IUPAF_NOT_MAPPED);
  iupClassRegisterAttribute(ic, "HLCOLOR", NULL, NULL, IUPAF_SAMEASSYSTEM, "TXTHLCOLOR", IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "HLCOLORALPHA", NULL, NULL, IUPAF_SAMEASSYSTEM, "128", IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "SPACING", iFlatListGetSpacingAttrib, iFlatListSetSpacingAttrib, IUPAF_SAMEASSYSTEM, "0", IUPAF_NO_INHERIT | IUPAF_NOT_MAPPED);
  iupClassRegisterAttribute(ic, "PADDING", iFlatListGetPaddingAttrib, iFlatListSetPaddingAttrib, IUPAF_SAMEASSYSTEM, "2x2", IUPAF_NOT_MAPPED);
  iupClassRegisterAttribute(ic, "HASFOCUS", iFlatListGetHasFocusAttrib, NULL, NULL, NULL, IUPAF_READONLY | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "ALIGNMENT", iFlatListGetAlignmentAttrib, iFlatListSetAlignmentAttrib, "ALEFT:ACENTER", NULL, IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "FOCUSFEEDBACK", NULL, NULL, IUPAF_SAMEASSYSTEM, "YES", IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);

  iupClassRegisterAttributeId(ic, "INSERTITEM", NULL, iFlatListSetInsertItemAttrib, IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "APPENDITEM", NULL, iFlatListSetAppendItemAttrib, NULL, NULL, IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "REMOVEITEM", NULL, iFlatListSetRemoveItemAttrib, NULL, NULL, IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);

  iupClassRegisterAttributeId(ic, "IMAGE", NULL, iFlatListSetImageAttrib, IUPAF_IHANDLENAME | IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "IMAGEPOSITION", iFlatListGetImagePositionAttrib, iFlatListSetImagePositionAttrib, IUPAF_SAMEASSYSTEM, "LEFT", IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "ICONSPACING", iFlatListGetIconSpacingAttrib, iFlatListSetIconSpacingAttrib, IUPAF_SAMEASSYSTEM, "2", IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "TEXTALIGNMENT", NULL, NULL, IUPAF_SAMEASSYSTEM, "ALEFT", IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "TEXTWRAP", NULL, NULL, NULL, NULL, IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "TEXTELLIPSIS", NULL, NULL, NULL, NULL, IUPAF_NO_INHERIT);

  iupClassRegisterAttribute(ic, "BACKIMAGE", NULL, NULL, NULL, NULL, IUPAF_NOT_MAPPED | IUPAF_IHANDLENAME | IUPAF_NO_DEFAULTVALUE | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "BACKIMAGEZOOM", NULL, NULL, NULL, NULL, IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "FITTOBACKIMAGE", NULL, NULL, NULL, NULL, IUPAF_NO_INHERIT);

  iupClassRegisterAttribute(ic, "SHOWDRAGDROP", iFlatListGetShowDragDropAttrib, iFlatListSetShowDragDropAttrib, NULL, NULL, IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "DRAGDROPLIST", NULL, iFlatListSetDragDropListAttrib, NULL, NULL, IUPAF_NO_INHERIT);

  iupClassRegisterAttribute(ic, "VISIBLECOLUMNS", NULL, NULL, NULL, NULL, IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "VISIBLELINES", NULL, NULL, "5", NULL, IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);

  iupClassRegisterAttribute(ic, "TOPITEM", NULL, iFlatListSetTopItemAttrib, NULL, NULL, IUPAF_WRITEONLY | IUPAF_NO_INHERIT);
  iupClassRegisterReplaceAttribDef(ic, "SCROLLBAR", "YES", NULL);  /* change the default to Yes */
  iupClassRegisterAttribute(ic, "YAUTOHIDE", NULL, NULL, IUPAF_SAMEASSYSTEM, "YES", IUPAF_READONLY | IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);  /* will be always Yes */
  iupClassRegisterAttribute(ic, "XAUTOHIDE", NULL, NULL, IUPAF_SAMEASSYSTEM, "YES", IUPAF_READONLY | IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);  /* will be always Yes */

  /* Flat Scrollbar */
  iupFlatScrollBarRegister(ic);

  iupClassRegisterAttribute(ic, "FLATSCROLLBAR", NULL, iFlatListSetFlatScrollbarAttrib, NULL, NULL, IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);

  return ic;
}
