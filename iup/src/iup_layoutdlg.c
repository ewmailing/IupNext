/** \file
 * \brief IupLayoutDialog pre-defined dialog
 *
 * See Copyright Notice in "iup.h"
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <stdarg.h>
#include <limits.h>

#include "iup.h"

#include "iup_object.h"
#include "iup_attrib.h"
#include "iup_str.h"
#include "iup_dlglist.h"
#include "iup_assert.h"
#include "iup_draw.h"
#include "iup_childtree.h"
#include "iup_drv.h"
#include "iup_func.h"


typedef struct _iLayoutDialog {
  int destroy;  /* destroy the selected dialog, when the layout dialog is destroyed */
  Ihandle *dialog;  /* the selected dialog */
  Ihandle *tree, *status, *timer, *properties;  /* elements from the layout dialog */
} iLayoutDialog;

static int iLayoutDialogShow_CB(Ihandle* dlg, int state)
{
  if (state == IUP_SHOW)
    IupSetAttribute(dlg, "RASTERSIZE", NULL);
  return IUP_DEFAULT;
}

static int iLayoutDialogClose_CB(Ihandle* dlg)
{
  if (IupGetInt(dlg, "DESTROYWHENCLOSED"))
  {
    IupDestroy(dlg);
    return IUP_IGNORE;
  }
  return IUP_DEFAULT;
}

static int iLayoutDialogDestroy_CB(Ihandle* dlg)
{
  iLayoutDialog* layoutdlg = (iLayoutDialog*)iupAttribGet(dlg, "_IUP_LAYOUTDIALOG");
  IupDestroy(layoutdlg->timer);
  if (iupObjectCheck(layoutdlg->properties))
    IupDestroy(layoutdlg->properties);
  if (layoutdlg->destroy && iupObjectCheck(layoutdlg->dialog))
    IupDestroy(layoutdlg->dialog);
  free(layoutdlg);
  return IUP_DEFAULT;
}

static char* iLayoutGetTitle(Ihandle* ih)
{
  char* title = IupGetAttribute(ih, "TITLE");
  char* name = IupGetName(ih);
  char* str = iupStrGetMemory(200);
  if (title)
  {
    char buffer[51];
    int len;
    if (iupStrNextLine(title, &len)!=title ||
        len > 50)
    {
      if (len > 50) len = 50;
      iupStrCopyN(buffer, len+1, title);
      title = &buffer[0];
    }

    if (name)
      sprintf(str, "[%s] %.50s \"%.50s\"", IupGetClassName(ih), title, name);
    else
      sprintf(str, "[%s] %.50s", IupGetClassName(ih), title);
  }
  else
  {
    if (name)
      sprintf(str, "[%s] \"%.50s\"", IupGetClassName(ih), name);
    else
      sprintf(str, "[%s]", IupGetClassName(ih));
  }
  return str;
}

static void iLayoutTreeSetNodeInfo(Ihandle* tree, int id, Ihandle* ih)
{
  IupSetAttributeId(tree, "TITLE", id, iLayoutGetTitle(ih));
  iupAttribSetInt(ih, "_IUP_LAYOUTTREE_ID", id);
  IupTreeSetUserId(tree, id, ih);
}

static void iLayoutTreeAddChildren(Ihandle* tree, int parent_id, Ihandle* parent)
{
  Ihandle *child;
  int last_child_id = parent_id;

  for (child = parent->firstchild; child; child = child->brother)
  {
    if (child->iclass->childtype != IUP_CHILDNONE)
    {
      if (child == parent->firstchild)
      {
        IupSetAttributeId(tree, "ADDBRANCH", last_child_id, "");
        last_child_id++;
      }
      else
      {
        IupSetAttributeId(tree, "INSERTBRANCH", last_child_id, "");
        last_child_id = IupGetInt(tree, "LASTADDNODE");
      }

      iLayoutTreeAddChildren(tree, last_child_id, child);
    }
    else
    {
      if (child == parent->firstchild)
      {
        IupSetAttributeId(tree, "ADDLEAF", last_child_id, "");
        last_child_id++;
      }
      else
      {
        IupSetAttributeId(tree, "INSERTLEAF", last_child_id, "");
        last_child_id = IupGetInt(tree, "LASTADDNODE");
      }
    }

    iLayoutTreeSetNodeInfo(tree, last_child_id, child);
  }
}

static void iLayoutRebuildTree(iLayoutDialog* layoutdlg)
{
  Ihandle* tree = layoutdlg->tree;
  IupSetAttribute(tree, "DELNODE0", "CHILDREN");

  iupAttribSetStr(layoutdlg->dialog, "_IUPLAYOUT_CHANGED", NULL);

  /* must sure the dialog layout is updated */
  IupRefresh(layoutdlg->dialog);

  iLayoutTreeSetNodeInfo(tree, 0, layoutdlg->dialog);
  iLayoutTreeAddChildren(tree, 0, layoutdlg->dialog);

  /* redraw canvas */
  IupUpdate(IupGetBrother(tree));
}

static int iLayoutMenuNew_CB(Ihandle* ih)
{
  Ihandle* dlg = IupGetDialog(ih);
  iLayoutDialog* layoutdlg = (iLayoutDialog*)iupAttribGet(dlg, "_IUP_LAYOUTDIALOG");
  if (layoutdlg->destroy)
    IupDestroy(layoutdlg->dialog);
  layoutdlg->dialog = IupDialog(NULL);
  layoutdlg->destroy = 1;
  iLayoutRebuildTree(layoutdlg);
  return IUP_DEFAULT;
}

static int iLayoutMenuReload_CB(Ihandle* ih)
{
  Ihandle* dlg = IupGetDialog(ih);
  iLayoutDialog* layoutdlg = (iLayoutDialog*)iupAttribGet(dlg, "_IUP_LAYOUTDIALOG");
  iLayoutRebuildTree(layoutdlg);
  return IUP_DEFAULT;
}

static int iLayoutMenuClose_CB(Ihandle* ih)
{
  Ihandle* dlg = IupGetDialog(ih);
  if (IupGetInt(dlg, "DESTROYWHENCLOSED"))
  {
    IupDestroy(dlg);
    return IUP_IGNORE;
  }
  else
  {
    IupHide(dlg);
    return IUP_DEFAULT;
  }
}

static int iLayoutMenuTree_CB(Ihandle* ih)
{
  Ihandle* dlg = IupGetDialog(ih);
  Ihandle* split = IupGetChild(dlg, 0);
  if (IupGetInt(split, "VALUE"))
    IupSetAttribute(split, "VALUE", "0");
  else
    IupSetAttribute(split, "VALUE", "300");
  return IUP_DEFAULT;
}

static int iLayoutMenuRefresh_CB(Ihandle* ih)
{
  Ihandle* dlg = IupGetDialog(ih);
  iLayoutDialog* layoutdlg = (iLayoutDialog*)iupAttribGet(dlg, "_IUP_LAYOUTDIALOG");
  IupRefresh(layoutdlg->dialog);
  /* redraw canvas */
  IupUpdate(IupGetBrother(layoutdlg->tree));
  return IUP_DEFAULT;
}

static int iLayoutAutoUpdate_CB(Ihandle* ih)
{
  iLayoutDialog* layoutdlg = (iLayoutDialog*)iupAttribGet(ih, "_IUP_LAYOUTDIALOG");
  /* redraw canvas */
  IupUpdate(IupGetBrother(layoutdlg->tree));
  return IUP_DEFAULT;
}

static int iLayoutMenuAutoUpdate_CB(Ihandle* ih)
{
  Ihandle* dlg = IupGetDialog(ih);
  iLayoutDialog* layoutdlg = (iLayoutDialog*)iupAttribGet(dlg, "_IUP_LAYOUTDIALOG");
  if (IupGetInt(layoutdlg->timer, "RUN"))
    IupSetAttribute(layoutdlg->timer, "RUN", "No");
  else
    IupSetAttribute(layoutdlg->timer, "RUN", "Yes");
  return IUP_DEFAULT;
}

static int iLayoutMenuUpdate_CB(Ihandle* ih)
{
  Ihandle* dlg = IupGetDialog(ih);
  iLayoutDialog* layoutdlg = (iLayoutDialog*)iupAttribGet(dlg, "_IUP_LAYOUTDIALOG");
  /* redraw canvas */
  IupUpdate(IupGetBrother(layoutdlg->tree));
  return IUP_DEFAULT;
}

static int iLayoutMenuRedraw_CB(Ihandle* ih)
{
  Ihandle* dlg = IupGetDialog(ih);
  iLayoutDialog* layoutdlg = (iLayoutDialog*)iupAttribGet(dlg, "_IUP_LAYOUTDIALOG");
  IupRedraw(layoutdlg->dialog, 1);
  return IUP_DEFAULT;
}

static int iLayoutOpacity_CB(Ihandle* dialog, int param_index, void* user_data)
{  
  if (param_index == 0)
  {
    Ihandle* dlg = (Ihandle*)user_data;
    Ihandle* param = (Ihandle*)IupGetAttribute(dialog, "PARAM0");
    int opacity = IupGetInt(param, "VALUE");
    IupSetfAttribute(dlg,"OPACITY", "%d", opacity);
  }
  return 1;
}

static int iLayoutMenuOpacity_CB(Ihandle* ih)
{
  Ihandle* dlg = IupGetDialog(ih);
  int opacity = IupGetInt(dlg, "OPACITY");
  if (opacity == 0)
    opacity = 255;

  IupGetParam("Dialog Layout", iLayoutOpacity_CB, dlg,
              "Opacity: %i[0,255]\n",
              &opacity, NULL);

  if (opacity == 0 || opacity == 255)
    IupSetAttribute(dlg, "OPACITY", NULL);
  else
    IupSetfAttribute(dlg,"OPACITY", "%d", opacity);

  return IUP_DEFAULT;
}

static int iLayoutMenuShow_CB(Ihandle* ih)
{
  Ihandle* dlg = IupGetDialog(ih);
  iLayoutDialog* layoutdlg = (iLayoutDialog*)iupAttribGet(dlg, "_IUP_LAYOUTDIALOG");
  IupShow(layoutdlg->dialog);
  return IUP_DEFAULT;
}

static int iLayoutCompareStr(const void *a, const void *b)
{
  return strcmp( * ( char** ) a, * ( char** ) b );
}

static void iLayoutDialogLoad(Ihandle* parent_dlg, iLayoutDialog* layoutdlg, int only_visible)
{
  int ret, count, i; 	
  Ihandle* *dlg_list;
  char* *dlg_list_str;
  Ihandle *dlg;

  if (only_visible)
    count = iupDlgListVisibleCount();
  else
    count = iupDlgListCount();

  dlg_list = (Ihandle**)malloc(count*sizeof(Ihandle*));
  dlg_list_str = (char**)malloc(count*sizeof(char*));

  for (dlg = iupDlgListFirst(), i=0; dlg && i < count; dlg = iupDlgListNext())
  {
    if (!only_visible ||
        (dlg->handle && IupGetInt(dlg, "VISIBLE")))
    {
      dlg_list[i] = dlg;
      dlg_list_str[i] = iupStrDup(iLayoutGetTitle(dlg));
      i++;
    }
  }

  iupASSERT(i == count);
  if (i != count)
    count = i;

  qsort(dlg_list_str, count, sizeof(char*), iLayoutCompareStr);

  IupStoreGlobal("_IUP_OLD_PARENTDIALOG", IupGetGlobal("PARENTDIALOG"));
  IupSetAttributeHandle(NULL, "PARENTDIALOG", parent_dlg);

  ret = IupListDialog(1,"Dialogs",count,(const char**)dlg_list_str,1,15,count<15? count+1: 15,NULL);

  IupStoreGlobal("PARENTDIALOG", IupGetGlobal("_IUP_OLD_PARENTDIALOG"));
  IupSetGlobal("_IUP_OLD_PARENTDIALOG", NULL);

  if (ret != -1)
  {
    int w = 0, h = 0;

    if (layoutdlg->destroy)
      IupDestroy(layoutdlg->dialog);
    layoutdlg->dialog = dlg_list[ret];
    layoutdlg->destroy = 0;

    IupGetIntInt(layoutdlg->dialog, "CLIENTSIZE", &w, &h);
    if (w && h)
    {
      int pw = 0, ph = 0;
      int prop = IupGetInt(IupGetParent(layoutdlg->tree), "VALUE");
      int status = IupGetInt2(IupGetBrother(IupGetParent(layoutdlg->tree)), "RASTERSIZE");
      w = (w*1000)/(1000-prop);
      IupGetIntInt(parent_dlg, "CLIENTSIZE", &pw, &ph);
      ph -= status;
      if (w > pw || h > ph)
      {
        IupSetfAttribute(parent_dlg, "CLIENTSIZE", "%dx%d", w+10, h+status+10);
        IupShow(parent_dlg);
      }
    }
  }

  for (i=0; i < count; i++)
    free(dlg_list_str[i]);

  free(dlg_list);
  free(dlg_list_str);
}

static int iLayoutMenuLoad_CB(Ihandle* ih)
{
  Ihandle* dlg = IupGetDialog(ih);
  iLayoutDialog* layoutdlg = (iLayoutDialog*)iupAttribGet(dlg, "_IUP_LAYOUTDIALOG");
  iLayoutDialogLoad(dlg, layoutdlg, 0);
  iLayoutRebuildTree(layoutdlg);
  return IUP_DEFAULT;
}

static int iLayoutMenuLoadVisible_CB(Ihandle* ih)
{
  Ihandle* dlg = IupGetDialog(ih);
  iLayoutDialog* layoutdlg = (iLayoutDialog*)iupAttribGet(dlg, "_IUP_LAYOUTDIALOG");
  iLayoutDialogLoad(dlg, layoutdlg, 1);
  iLayoutRebuildTree(layoutdlg);
  return IUP_DEFAULT;
}

static void iLayoutDrawElement(IdrawCanvas* dc, Ihandle* ih, int marked, int native_parent_x, int native_parent_y,
                               unsigned char fr, unsigned char fg, unsigned char fb,
                               unsigned char fvr, unsigned char fvg, unsigned char fvb,
                               unsigned char fmr, unsigned char fmg, unsigned char fmb,
                               unsigned char br, unsigned char bg, unsigned char bb)
{
  int x, y, w, h; 
  char *bgcolor;
  unsigned char r, g, b;

  x = ih->x+native_parent_x;
  y = ih->y+native_parent_y;
  w = ih->currentwidth;
  h = ih->currentheight;
  if (w<=0) w=1;
  if (h<=0) h=1;

  bgcolor = iupAttribGetLocal(ih, "BGCOLOR");
  if (bgcolor && ih->iclass->nativetype!=IUP_TYPEVOID)
  {
    r = br, g = bg, b = bb;
    iupStrToRGB(bgcolor, &r, &g, &b);
    iupDrawRectangle(dc, x, y, x+w-1, y+h-1, r, g, b, IUP_DRAW_FILL);
  }

  if (ih->iclass->nativetype==IUP_TYPEVOID)
    iupDrawRectangle(dc, x, y, x+w-1, y+h-1, fvr, fvg, fvb, IUP_DRAW_STROKE_DASH);
  else
    iupDrawRectangle(dc, x, y, x+w-1, y+h-1, fr, fg, fb, IUP_DRAW_STROKE);

  if (ih->iclass->childtype==IUP_CHILDNONE)
  {
    int pw, ph;
    IupGetIntInt(ih->parent, "CLIENTSIZE", &pw, &ph);

    if (ih->currentwidth == pw && ih->currentwidth == ih->naturalwidth)
    {
      iupDrawLine(dc, x+1, y+1, x+w-2, y+1, fmr, fmg, fmb, IUP_DRAW_STROKE);
      iupDrawLine(dc, x+1, y+h-2, x+w-2, y+h-2, fmr, fmg, fmb, IUP_DRAW_STROKE);
    }

    if (ih->currentheight == ph && ih->currentheight == ih->naturalheight)
    {
      iupDrawLine(dc, x+1, y+1, x+1, y+h-2, fmr, fmg, fmb, IUP_DRAW_STROKE);
      iupDrawLine(dc, x+w-2, y+1, x+w-2, y+h-2, fmr, fmg, fmb, IUP_DRAW_STROKE);
    }
  }
  else if (ih->iclass->nativetype!=IUP_TYPEVOID)
  {
    /* if ih is a Tabs, position the title accordingly */
    if (iupStrEqualNoCase(ih->iclass->name, "tabs"))
    {
      /* TABORIENTATION is ignored */
      char* tabtype = iupAttribGetLocal(ih, "TABTYPE");
      if (iupStrEqualNoCase(tabtype, "BOTTOM"))
      {
        int cw = 0, ch = 0;
        IupGetIntInt(ih, "CLIENTSIZE", &cw, &ch);
        y += ch;  /* position after the client area */
      }
      else if (iupStrEqualNoCase(tabtype, "RIGHT"))
      {
        int cw = 0, ch = 0;
        IupGetIntInt(ih, "CLIENTSIZE", &cw, &ch);
        x += cw;  /* position after the client area */
      }
    }
  }

  /* always draw the image first */
  if (ih->iclass->nativetype!=IUP_TYPEVOID)
  {
    char *title, *image;

    image = iupAttribGetLocal(ih, "IMAGE0");  /* Tree root node title */
    if (!image) 
      image = iupAttribGetLocal(ih, "TABIMAGE0");  /* Tabs first tab image */
    if (image)
    {
      /* returns the image of the active tab */
      int pos = IupGetInt(ih, "VALUEPOS");
      image = IupGetAttributeId(ih, "TABIMAGE", pos);
    }
    if (!image) 
      image = iupAttribGetLocal(ih, "IMAGE");
    if (image)
    {
      char* position;
      int img_w, img_h;

      iupDrawSetClipRect(dc, x+1, y+1, x+w-2, y+h-2);
      iupDrawImage(dc, image, 0, x+1, y+1, &img_w, &img_h);
      iupDrawResetClip(dc);

      position = iupAttribGetLocal(ih, "IMAGEPOSITION");  /* used only for buttons */
      if (position &&
          (iupStrEqualNoCase(position, "BOTTOM") ||
           iupStrEqualNoCase(position, "TOP")))
        y += img_h;
      else
        x += img_w;  /* position text usually at right */
    }

    title = iupAttribGetLocal(ih, "0:0");  /* Matrix title cell */
    if (!title) 
      title = iupAttribGetLocal(ih, "1");  /* List first item */
    if (!title) 
      title = iupAttribGetLocal(ih, "TITLE0");  /* Tree root node title */
    if (!title)
    {
      title = iupAttribGetLocal(ih, "TABTITLE0");  /* Tabs first tab title */
      if (title)
      {
        /* returns the title of the active tab */
        int pos = IupGetInt(ih, "VALUEPOS");
        title = IupGetAttributeId(ih, "TABTITLE", pos);
      }
    }
    if (!title) 
      title = iupAttribGetLocal(ih, "TITLE");
    if (title)
    {
      int len;
      iupStrNextLine(title, &len);
      iupDrawSetClipRect(dc, x+1, y+1, x+w-2, y+h-2);
      r = fr, g = fg, b = fb;
      iupStrToRGB(iupAttribGetLocal(ih, "FGCOLOR"), &r, &g, &b);
      iupDrawText(dc, title, len, x+1, y+1, r, g, b, IupGetAttribute(ih, "FONT"));
      iupDrawResetClip(dc);
    }

    if (ih->iclass->childtype==IUP_CHILDNONE &&
        !title && !image)
    {
      if (iupStrEqualNoCase(ih->iclass->name, "progressbar"))
      {
        float min = IupGetFloat(ih, "MIN");
        float max = IupGetFloat(ih, "MAX");
        float val = IupGetFloat(ih, "VALUE");
        r = fr, g = fg, b = fb;
        iupStrToRGB(iupAttribGetLocal(ih, "FGCOLOR"), &r, &g, &b);
        if (iupStrEqualNoCase(iupAttribGetLocal(ih, "ORIENTATION"), "VERTICAL"))
        {
          int ph = (int)(((max-val)*(h-5))/(max-min));
          iupDrawRectangle(dc, x+2, y+2, x+w-3, y+ph, r, g, b, IUP_DRAW_FILL);
        }
        else
        {
          int pw = (int)(((val-min)*(w-5))/(max-min));
          iupDrawRectangle(dc, x+2, y+2, x+pw, y+h-3, r, g, b, IUP_DRAW_FILL);
        }
      }
      else if (iupStrEqualNoCase(ih->iclass->name, "val"))
      {
        float min = IupGetFloat(ih, "MIN");
        float max = IupGetFloat(ih, "MAX");
        float val = IupGetFloat(ih, "VALUE");
        r = fr, g = fg, b = fb;
        iupStrToRGB(iupAttribGetLocal(ih, "FGCOLOR"), &r, &g, &b);
        if (iupStrEqualNoCase(iupAttribGetLocal(ih, "ORIENTATION"), "VERTICAL"))
        {
          int ph = (int)(((max-val)*(h-5))/(max-min));
          iupDrawRectangle(dc, x+2, y+ph-1, x+w-3, y+ph+1, r, g, b, IUP_DRAW_FILL);
        }
        else
        {
          int pw = (int)(((val-min)*(w-5))/(max-min));
          iupDrawRectangle(dc, x+pw-1, y+2, x+pw+1, y+h-3, r, g, b, IUP_DRAW_FILL);
        }
      }
    }
  }

  if (marked)
  {
    x = ih->x+native_parent_x;
    y = ih->y+native_parent_y;
    w = ih->currentwidth;
    h = ih->currentheight;
    if (w<=0) w=1;
    if (h<=0) h=1;

    iupDrawRectangleInvert(dc, x, y, x+w-1, y+h-1);
  }
}

static void iLayoutDrawElementTree(IdrawCanvas* dc, int showhidden, Ihandle* mark, Ihandle* ih, int native_parent_x, int native_parent_y,
                                   unsigned char fr, unsigned char fg, unsigned char fb,
                                   unsigned char fvr, unsigned char fvg, unsigned char fvb,
                                   unsigned char fmr, unsigned char fmg, unsigned char fmb,
                                   unsigned char br, unsigned char bg, unsigned char bb)
{
  Ihandle *child;
  int dx, dy;

  if (showhidden || iupAttribGetLocal(ih, "VISIBLE"))
  {
    /* draw the element */
    iLayoutDrawElement(dc, ih, ih==mark, native_parent_x, native_parent_y,
                       fr, fg, fb, fvr, fvg, fvb, fmr, fmg, fmb, br, bg, bb);

    if (ih->iclass->childtype != IUP_CHILDNONE)
    {
      /* if ih is a native parent, then update the offset */
      if (ih->iclass->nativetype!=IUP_TYPEVOID)
      {
        dx = 0, dy = 0;
        IupGetIntInt(ih, "CLIENTOFFSET", &dx, &dy);
        native_parent_x += ih->x+dx;
        native_parent_y += ih->y+dy;

        /* if ih is a Tabs, then draw only the active child */
        if (!showhidden && iupStrEqualNoCase(ih->iclass->name, "tabs"))
        {
          child = (Ihandle*)IupGetAttribute(ih, "VALUE_HANDLE");
          if (child)
            iLayoutDrawElementTree(dc, showhidden, mark, child, native_parent_x, native_parent_y,
                                   fr, fg, fb, fvr, fvg, fvb, fmr, fmg, fmb, br, bg, bb);
          return;
        }
      }
    }

    /* draw its children */
    for (child = ih->firstchild; child; child = child->brother)
    {
      iLayoutDrawElementTree(dc, showhidden, mark, child, native_parent_x, native_parent_y,
                             fr, fg, fb, fvr, fvg, fvb, fmr, fmg, fmb, br, bg, bb);
    }
  }
}

static void iLayoutDrawDialog(iLayoutDialog* layoutdlg, int showhidden, IdrawCanvas* dc, Ihandle* mark,
                              unsigned char fr, unsigned char fg, unsigned char fb,
                              unsigned char fvr, unsigned char fvg, unsigned char fvb,
                              unsigned char fmr, unsigned char fmg, unsigned char fmb,
                              unsigned char br, unsigned char bg, unsigned char bb)
{
  int w, h;

  iupDrawGetSize(dc, &w, &h);
  iupDrawRectangle(dc, 0, 0, w-1, h-1, br, bg, bb, IUP_DRAW_FILL);

  /* draw the dialog */
  IupGetIntInt(layoutdlg->dialog, "CLIENTSIZE", &w, &h);
  iupDrawRectangle(dc, 0, 0, w-1, h-1, fr, fg, fb, IUP_DRAW_STROKE);

  if (layoutdlg->dialog->firstchild)
  {
    int native_parent_x = 0, native_parent_y = 0;
    IupGetIntInt(layoutdlg->dialog, "CLIENTOFFSET", &native_parent_x, &native_parent_y);
    iLayoutDrawElementTree(dc, showhidden, mark, layoutdlg->dialog->firstchild, native_parent_x, native_parent_y,
                           fr, fg, fb, fvr, fvg, fvb, fmr, fmg, fmb, br, bg, bb);
  }
}

static int iLayoutCanvas_CB(Ihandle* canvas)
{
  Ihandle* dlg = IupGetDialog(canvas);
  iLayoutDialog* layoutdlg = (iLayoutDialog*)iupAttribGet(dlg, "_IUP_LAYOUTDIALOG");
  IdrawCanvas* dc = iupDrawCreateCanvas(canvas);
  int showhidden = IupGetInt(dlg, "SHOWHIDDEN");
  Ihandle* mark;
  unsigned char br, bg, bb, 
                fr, fg, fb,
                fmr, fmg, fmb,
                fvr, fvg, fvb;

  br = 255, bg = 255, bb = 255;
  iupStrToRGB(IupGetAttribute(dlg, "BACKCOLOR"), &br, &bg, &bb);

  fr = 0, fg = 0, fb = 0;
  iupStrToRGB(IupGetAttribute(dlg, "FORECOLOR"), &fr, &fg, &fb);

  fvr = 0, fvg = 0, fvb = 0;
  iupStrToRGB(IupGetAttribute(dlg, "FORECOLORVOID"), &fvr, &fvg, &fvb);

  fmr = 0, fmg = 0, fmb = 0;
  iupStrToRGB(IupGetAttribute(dlg, "FORECOLORMAX"), &fmr, &fmg, &fmb);

  mark = (Ihandle*)iupAttribGet(dlg, "_IUPLAYOUT_MARK");

  iLayoutDrawDialog(layoutdlg, showhidden, dc, mark,
                      fr, fg, fb, 
                      fvr, fvg, fvb, 
                      fmr, fmg, fmb, 
                      br, bg, bb);

  iupDrawFlush(dc);

  iupDrawKillCanvas(dc);

  return IUP_DEFAULT;
}

static void iLayoutUpdateProperties(Ihandle* properties, Ihandle* ih)
{
  int i, j, attr_count, cb_count, total_count = IupGetClassAttributes(ih->iclass->name, NULL, 0);
  char **attr_names = (char **) malloc(total_count * sizeof(char *));
  Ihandle* list1 = (Ihandle*)iupAttribGet(properties, "_IUP_PROPLIST1");
  Ihandle* list2 = (Ihandle*)iupAttribGet(properties, "_IUP_PROPLIST2");
  Ihandle* list3 = (Ihandle*)iupAttribGet(properties, "_IUP_PROPLIST3");

  /* Clear everything */
  IupSetAttribute(list1, "REMOVEITEM", NULL);
  IupSetAttribute(list2, "REMOVEITEM", NULL);
  IupSetAttribute(list3, "REMOVEITEM", NULL);
  IupSetAttribute(IupGetDialogChild(properties, "VALUE1A"), "VALUE", "");
  IupSetAttribute(IupGetDialogChild(properties, "VALUE1B"), "TITLE", "");
  IupSetAttribute(IupGetDialogChild(properties, "VALUE1C"), "TITLE", "");
  IupSetAttribute(IupGetDialogChild(properties, "VALUE2"), "TITLE", "");
  IupSetAttribute(IupGetDialogChild(properties, "VALUE3"), "TITLE", "");
  IupSetAttribute(IupGetDialogChild(properties, "SETBUT"), "ACTIVE", "No");

  attr_count = IupGetClassAttributes(ih->iclass->name, attr_names, total_count);
  for (i=0; i<attr_count; i++)
    IupSetAttributeId(list1,"",i+1, attr_names[i]);

  cb_count = total_count-attr_count;
  IupGetClassCallbacks(ih->iclass->name, attr_names, cb_count);
  for (i=0; i<cb_count; i++)
    IupSetAttributeId(list3,"",i+1, attr_names[i]);

  attr_count = IupGetAllAttributes(ih, NULL, 0);
  if (attr_count > total_count)
    attr_names = (char **) realloc(attr_names, attr_count * sizeof(char *));

  IupGetAllAttributes(ih, attr_names, attr_count);
  for (i=0, j=1; i<attr_count; i++)
  {
    if (!iupClassObjectAttribIsRegistered(ih, attr_names[i]))
    {
      IupSetAttributeId(list2,"",j, attr_names[i]);
      j++;
    }
  }

  iupAttribSetStr(properties, "_IUP_PROPELEMENT", (char*)ih);

  IupSetAttribute(IupGetDialogChild(properties, "ELEMTITLE"), "TITLE", iLayoutGetTitle(ih));

  free(attr_names);
}

static int iLayoutPropertiesClose_CB (Ihandle* ih)
{
  IupHide(IupGetDialog(ih));
  return IUP_DEFAULT;
}

static int iLayoutPropertiesSet_CB(Ihandle* button)
{
  Ihandle* list1 = (Ihandle*)iupAttribGetInherit(button, "_IUP_PROPLIST1");
  char* item = IupGetAttribute(list1, "VALUE");
  if (item)
  {
    Ihandle* elem = (Ihandle*)iupAttribGetInherit(button, "_IUP_PROPELEMENT");
    Ihandle* txt1 = IupGetDialogChild(button, "VALUE1A");
    char* value = IupGetAttribute(txt1, "VALUE");
    char* name = IupGetAttribute(list1, item);
    IupStoreAttribute(elem, name, value);
    iupAttribSetStr(IupGetDialog(elem), "_IUPLAYOUT_CHANGED", "1");
  }
  return IUP_DEFAULT;
}

static int iLayoutPropertiesList1_CB(Ihandle *list1, char *name, int item, int state)
{
  (void)item;
  if (state)
  {
    char* def_value;
    int inherit, not_string, has_id, access;
    Ihandle* elem = (Ihandle*)iupAttribGetInherit(list1, "_IUP_PROPELEMENT");
    char* value = iupAttribGetLocal(elem, name);
    Ihandle* txt1 = IupGetDialogChild(list1, "VALUE1A");
    Ihandle* lbl2 = IupGetDialogChild(list1, "VALUE1B");
    Ihandle* lbl3 = IupGetDialogChild(list1, "VALUE1C");
    Ihandle* setbut = IupGetDialogChild(list1, "SETBUT");

    iupClassObjectGetAttribNameInfo(elem, name, &def_value, &inherit, &not_string, &has_id, &access);

    if (value)
    {
      if (not_string)
        IupSetfAttribute(txt1, "VALUE", "%p", value);
      else
        IupSetAttribute(txt1, "VALUE", value);
    }
    else
      IupSetAttribute(txt1, "VALUE", "NULL");

    if (def_value)
      IupSetAttribute(lbl2, "TITLE", def_value);
    else
      IupSetAttribute(lbl2, "TITLE", "NULL");

    IupSetfAttribute(lbl3, "TITLE", "%s\n%s%s%s", inherit? "Inheritable": "Non Inheritable", 
                                                  not_string? "Not a String\n": "", 
                                                  has_id? "Has ID\n":"", 
                                                  access==1? "Read-Only": (access==2? "Write-Only": (access==3? "NOT SUPPORTED": "")));

    if (def_value && !not_string && access==0 &&
        !iupStrEqualNoCase(def_value, value))
      IupSetAttribute(txt1, "FGCOLOR", "255 0 0");
    else
      IupSetAttribute(txt1, "FGCOLOR", "0 0 0");

    if (access!=1 && !not_string)
      IupSetAttribute(setbut, "ACTIVE", "Yes");
    else
      IupSetAttribute(setbut, "ACTIVE", "No");
  }
  return IUP_DEFAULT;
}

static int iLayoutPropertiesList2_CB(Ihandle *list2, char *name, int item, int state)
{
  (void)item;
  if (state)
  {
    Ihandle* elem = (Ihandle*)iupAttribGetInherit(list2, "_IUP_PROPELEMENT");
    char* value = iupAttribGet(elem, name);
    Ihandle* lbl = IupGetDialogChild(list2, "VALUE2");
    if (value)
      IupSetfAttribute(lbl, "TITLE", "%p", value);
    else
      IupSetAttribute(lbl, "TITLE", "NULL");
  }
  return IUP_DEFAULT;
}

static int iLayoutPropertiesGetAsString_CB(Ihandle *button)
{
  Ihandle* elem = (Ihandle*)iupAttribGetInherit(button, "_IUP_PROPELEMENT");
  Ihandle* list2 = (Ihandle*)iupAttribGetInherit(button, "_IUP_PROPLIST2");
  char* item = IupGetAttribute(list2, "VALUE");
  if (item)
  {
    char* value = iupAttribGet(elem, IupGetAttribute(list2, item));
    Ihandle* lbl = IupGetDialogChild(button, "VALUE2");
    if (value)
      IupSetAttribute(lbl, "TITLE", value);
    else
      IupSetAttribute(lbl, "TITLE", "NULL");
  }
  return IUP_DEFAULT;
}

static int iLayoutPropertiesList3_CB(Ihandle *list3, char *text, int item, int state)
{
  (void)item;
  if (state)
  {
    Ihandle* elem = (Ihandle*)iupAttribGetInherit(list3, "_IUP_PROPELEMENT");
    Icallback cb = IupGetCallback(elem, text);
    Ihandle* lbl = IupGetDialogChild(list3, "VALUE3");
    if (cb)
    {
      char* name = iupGetCallbackName(elem, text);
      if (name)
        IupSetfAttribute(lbl, "TITLE", "%p\n\"%s\"", cb, name);
      else
        IupSetfAttribute(lbl, "TITLE", "%p", cb);
    }
    else
      IupSetAttribute(lbl, "TITLE", "NULL");
  }
  return IUP_DEFAULT;
}

static int iLayoutPropertiesTabChangePos_CB(Ihandle* ih, int new_pos, int old_pos)
{
  (void)old_pos;
  switch (new_pos)
  {
  case 0:
    IupSetAttribute(ih, "TIP", "All attributes that are known by the element.");
    break;
  case 1:
    IupSetAttribute(ih, "TIP", "Custom attributes set by the application.");
    break;
  case 2:
    IupSetAttribute(ih, "TIP", "All callbacks that are known by the element.");
    break;
  }

  /* In GTK the TIP appears for all children */
  if (iupStrEqualNoCase(IupGetGlobal("DRIVER"), "GTK"))
  {
    char* tabtype = iupAttribGetLocal(ih, "TABTYPE");
    int x = 0;
    int y = 0;
    int w = ih->currentwidth;
    int h = ih->currentheight;
    int cw = 0, ch = 0;

    IupGetIntInt(ih, "CLIENTSIZE", &cw, &ch);

    /* TABORIENTATION is ignored */
    if (iupStrEqualNoCase(tabtype, "BOTTOM"))
    {
      y += ch;  /* position after the client area */
      h -= ch;
    }
    else if (iupStrEqualNoCase(tabtype, "RIGHT"))
    {
      x += cw;  /* position after the client area */
      w -= cw;
    }
    else if (iupStrEqualNoCase(tabtype, "LEFT"))
      w -= cw;
    else  /* TOP */
      h -= ch;

    IupSetfAttribute(ih, "TIPRECT", "%d %d %d %d", x, y, x+w, y+h);
  }

  IupSetAttribute(ih, "TIPVISIBLE", "YES");
  return IUP_DEFAULT;
}

static void iLayoutCreatePropertiesDialog(iLayoutDialog* layoutdlg, Ihandle* parent)
{
  Ihandle *list1, *list2, *list3, *close, *dlg, *dlg_box, *button_box,
          *tabs, *box1, *box11, *box2, *box22, *box3, *box33, *set;

  close = IupButton("Close", NULL);
  IupSetAttribute(close,"PADDING" ,"20x5");
  IupSetCallback(close, "ACTION", (Icallback)iLayoutPropertiesClose_CB);

  button_box = IupHbox(
    IupFill(), 
    close,
    NULL);
  IupSetAttribute(button_box,"MARGIN","0x0");

  list1 = IupList(NULL);
  IupSetCallback(list1, "ACTION", (Icallback)iLayoutPropertiesList1_CB);
  IupSetAttribute(list1, "VISIBLELINES", "15");
  IupSetAttribute(list1, "VISIBLECOLUMNS", "11");
  IupSetAttribute(list1, "SORT", "Yes");
  IupSetAttribute(list1, "EXPAND", "VERTICAL");

  list2 = IupList(NULL);
  IupSetCallback(list2, "ACTION", (Icallback)iLayoutPropertiesList2_CB);
  IupSetAttribute(list2, "VISIBLELINES", "15");
  IupSetAttribute(list2, "VISIBLECOLUMNS", "11");
  IupSetAttribute(list2, "SORT", "Yes");
  IupSetAttribute(list2, "EXPAND", "VERTICAL");

  list3 = IupList(NULL);
  IupSetCallback(list3, "ACTION", (Icallback)iLayoutPropertiesList3_CB);
  IupSetAttribute(list3, "VISIBLELINES", "15");
  IupSetAttribute(list3, "VISIBLECOLUMNS", "14");
  IupSetAttribute(list3, "SORT", "Yes");
  IupSetAttribute(list3, "EXPAND", "VERTICAL");

  set = IupButton("Set", NULL);
  IupSetCallback(set, "ACTION", iLayoutPropertiesSet_CB);
  IupSetAttribute(set, "PADDING", "5x5");
  IupSetAttribute(set, "NAME", "SETBUT");

  box11 = IupVbox(
            IupLabel("Value:"),
            IupHbox(IupSetAttributes(IupText(NULL), "ALIGNMENT=ALEFT:ATOP, EXPAND=YES, NAME=VALUE1A"), set, NULL),
            IupSetAttributes(IupFill(), "RASTERSIZE=10"), 
            IupLabel("Default Value:"),
            IupFrame(IupSetAttributes(IupLabel(NULL), "ALIGNMENT=ALEFT:ATOP, EXPAND=HORIZONTAL, NAME=VALUE1B")),
            IupSetAttributes(IupFill(), "RASTERSIZE=10"), 
            IupLabel("Other Info:"),
            IupFrame(IupSetAttributes(IupLabel(NULL), "SIZE=90x60, ALIGNMENT=ALEFT:ATOP, NAME=VALUE1C")),
            NULL);
  IupSetAttribute(box11,"MARGIN","0x0");
  IupSetAttribute(box11,"GAP","0");

  box22 = IupVbox(
            IupLabel("Value:"),
            IupFrame(IupSetAttributes(IupLabel(NULL), "ALIGNMENT=ALEFT:ATOP, EXPAND=YES, NAME=VALUE2")),
            IupSetAttributes(IupFill(), "RASTERSIZE=10"), 
            IupSetCallbacks(IupSetAttributes(IupButton("Get as String", NULL), "PADDING=3x3"), "ACTION", iLayoutPropertiesGetAsString_CB, NULL),
            IupLabel("IMPORTANT: if the attribute is not a string\nthis can crash the application."),
            IupSetAttributes(IupFill(), "SIZE=60"), 
            NULL);
  IupSetAttribute(box22,"MARGIN","0x0");
  IupSetAttribute(box22,"GAP","0");

  box33 = IupVbox(
            IupLabel("Value:"),
            IupFrame(IupSetAttributes(IupLabel(""), "SIZE=x20, ALIGNMENT=ALEFT:ATOP, EXPAND=HORIZONTAL, NAME=VALUE3")),
            NULL);
  IupSetAttribute(box33,"MARGIN","0x0");
  IupSetAttribute(box33,"GAP","0");

  box1 = IupHbox(IupSetAttributes(IupVbox(IupLabel("Name:"), list1, NULL), "MARGIN=0x0, GAP=0"), box11, NULL);
  box2 = IupHbox(IupSetAttributes(IupVbox(IupLabel("Name:"), list2, NULL), "MARGIN=0x0, GAP=0"), box22, NULL);
  box3 = IupHbox(IupSetAttributes(IupVbox(IupLabel("Name:"), list3, NULL), "MARGIN=0x0, GAP=0"), box33, NULL);

  tabs = IupTabs(box1, box2, box3, NULL);
  IupSetAttribute(tabs, "TABTITLE0", "Registered Attributes");
  IupSetAttribute(tabs, "TABTITLE1", "Hash Table Only");
  IupSetAttribute(tabs, "TABTITLE2", "Callbacks");
  IupSetCallback(tabs, "TABCHANGEPOS_CB", (Icallback)iLayoutPropertiesTabChangePos_CB);
  iLayoutPropertiesTabChangePos_CB(tabs, 0, 0);

  dlg_box = IupVbox(
    IupSetAttributes(IupLabel(""), "EXPAND=HORIZONTAL, NAME=ELEMTITLE"),
    tabs,
    button_box,
    NULL);

  IupSetAttribute(dlg_box,"MARGIN","10x10");
  IupSetAttribute(dlg_box,"GAP","10");

  dlg = IupDialog(dlg_box);
  IupSetAttribute(dlg,"TITLE", "Element Properties");
  IupSetAttribute(dlg,"MINBOX","NO");
  IupSetAttribute(dlg,"MAXBOX","NO");
  IupSetAttributeHandle(dlg,"DEFAULTENTER", close);
  IupSetAttributeHandle(dlg,"DEFAULTESC", close);
  IupSetAttributeHandle(dlg,"PARENTDIALOG", parent);
  IupSetAttribute(dlg,"ICON", IupGetGlobal("ICON"));
  iupAttribSetStr(dlg, "_IUP_PROPLIST1", (char*)list1);
  iupAttribSetStr(dlg, "_IUP_PROPLIST2", (char*)list2);
  iupAttribSetStr(dlg, "_IUP_PROPLIST3", (char*)list3);

  layoutdlg->properties = dlg;
}

static int iLayoutMenuProperties_CB(Ihandle* menu)
{
  iLayoutDialog* layoutdlg = (iLayoutDialog*)iupAttribGetInherit(menu, "_IUP_LAYOUTDIALOG");
  Ihandle* elem = (Ihandle*)iupAttribGetInherit(menu, "_IUP_LAYOUTELEMENT");
  Ihandle* dlg = (Ihandle*)iupAttribGetInherit(menu, "_IUP_LAYOUTDLG");

  if (!layoutdlg->properties)
    iLayoutCreatePropertiesDialog(layoutdlg, dlg);

  iLayoutUpdateProperties(layoutdlg->properties, elem);

  IupShow(layoutdlg->properties);

  return IUP_DEFAULT;
}

static int iLayoutMenuAdd_CB(Ihandle* menu)
{
  iLayoutDialog* layoutdlg = (iLayoutDialog*)iupAttribGetInherit(menu, "_IUP_LAYOUTDIALOG");
  Ihandle* ref_elem = (Ihandle*)iupAttribGetInherit(menu, "_IUP_LAYOUTELEMENT");
  Ihandle* dlg = (Ihandle*)iupAttribGetInherit(menu, "_IUP_LAYOUTDLG");
  int ret, count; 	
  char** class_list_str;

  count = IupGetAllClasses(NULL, 0);
  class_list_str = (char**)malloc(count*sizeof(char*));

  IupGetAllClasses(class_list_str, count);
  qsort(class_list_str, count, sizeof(char*), iLayoutCompareStr);

  IupStoreGlobal("_IUP_OLD_PARENTDIALOG", IupGetGlobal("PARENTDIALOG"));
  IupSetAttributeHandle(NULL, "PARENTDIALOG", dlg);

  ret = IupListDialog(1,"Available Classes",count,(const char**)class_list_str,1,10,count<15? count+1: 15,NULL);

  IupStoreGlobal("PARENTDIALOG", IupGetGlobal("_IUP_OLD_PARENTDIALOG"));
  IupSetGlobal("_IUP_OLD_PARENTDIALOG", NULL);

  if (ret != -1)
  {
    int add_brother = IupGetInt(menu, "_IUP_ADBROTHER");
    Ihandle* elem = IupCreate(class_list_str[ret]);
    int id = IupTreeGetId(layoutdlg->tree, ref_elem);

//    IupAppend()
//    IupInsert()

    iupAttribSetStr(layoutdlg->dialog, "_IUPLAYOUT_CHANGED", "1");

    /* add to the tree */
//    iLayoutAddToTree(layoutdlg, id, elem);
//    IupSetAttributeId(layoutdlg->tree, "ADDBRANCH", id, "SELECTED");

    /* must sure the dialog layout is updated */
    IupRefresh(layoutdlg->dialog);

    /* redraw canvas */
    IupUpdate(IupGetBrother(layoutdlg->tree));
  }

  free(class_list_str);

  return IUP_DEFAULT;
}

static int iLayoutMenuRemove_CB(Ihandle* menu)
{
  iLayoutDialog* layoutdlg = (iLayoutDialog*)iupAttribGetInherit(menu, "_IUP_LAYOUTDIALOG");
  Ihandle* elem = (Ihandle*)iupAttribGetInherit(menu, "_IUP_LAYOUTELEMENT");

  Ihandle* msg = IupMessageDlg();
  IupSetAttribute(msg,"DIALOGTYPE", "QUESTION");
  IupSetAttribute(msg,"BUTTONS", "OKCANCEL");
  IupSetAttribute(msg,"TITLE", "Element Remove");
  IupSetAttribute(msg,"VALUE", "Remove the selected element?");

  IupPopup(msg, IUP_MOUSEPOS, IUP_MOUSEPOS);

  if (IupGetInt(msg, "BUTTONRESPONSE")==1)
  {
    int id = IupTreeGetId(layoutdlg->tree, elem);

    iupAttribSetStr(layoutdlg->dialog, "_IUPLAYOUT_CHANGED", "1");

    /* remove from the tree */
    IupSetAttributeId(layoutdlg->tree, "DELNODE", id, "SELECTED");

    /* update properties if necessary */
    if (layoutdlg->properties && IupGetInt(layoutdlg->properties, "VISIBLE"))
    {
      Ihandle* propelem = (Ihandle*)iupAttribGetInherit(layoutdlg->properties, "_IUP_PROPELEMENT");
      if (iupChildTreeIsChild(elem, propelem))
      {
        /* if current element will be removed, then use the previous element on the tree |*/
        iLayoutUpdateProperties(layoutdlg->properties, (Ihandle*)IupTreeGetUserId(layoutdlg->tree, id-1));
      }
    }

    IupDestroy(elem);

    /* must sure the dialog layout is updated */
    IupRefresh(layoutdlg->dialog);

    /* redraw canvas */
    IupUpdate(IupGetBrother(layoutdlg->tree));
  }
  return IUP_DEFAULT;
}

static void iLayoutContextMenu(iLayoutDialog* layoutdlg, Ihandle* ih, Ihandle* dlg)
{
  Ihandle* menu;

  menu = IupMenu(
    IupSetCallbacks(IupItem("Properties...", NULL), "ACTION", iLayoutMenuProperties_CB, NULL),
    IupSeparator(),
    IupSetCallbacks(IupSetAttributes(IupItem("Add Brother...", NULL), "_IUP_ADBROTHER=1"), "ACTION", iLayoutMenuAdd_CB, NULL),
    IupSetCallbacks(IupItem("Add Child...", NULL), "ACTION", iLayoutMenuAdd_CB, NULL),
    IupSetCallbacks(IupItem("Remove...", NULL), "ACTION", iLayoutMenuRemove_CB, NULL),
    NULL);

  iupAttribSetStr(menu, "_IUP_LAYOUTELEMENT", (char*)ih);
  iupAttribSetStr(menu, "_IUP_LAYOUTDIALOG", (char*)layoutdlg);
  iupAttribSetStr(menu, "_IUP_LAYOUTDLG", (char*)dlg);

  IupPopup(menu, IUP_MOUSEPOS, IUP_MOUSEPOS);
}

static void iLayoutBlink(Ihandle* ih)
{
  if (ih->iclass->nativetype!=IUP_TYPEVOID && IupGetInt(ih, "VISIBLE"))
  {
    int i;
    for (i=0; i<3; i++)
    {
      IupSetAttribute(ih, "VISIBLE", "NO");
      IupFlush();
      iupdrvSleep(100);
      IupSetAttribute(ih, "VISIBLE", "Yes");
      IupFlush();
      iupdrvSleep(100);
    }
  }
}

static void iLayoutUpdateMark(iLayoutDialog* layoutdlg, Ihandle* ih, int id)
{
  IupSetfAttribute(layoutdlg->status, "TITLE", "[SZ] User:%4d,%4d | Natural:%4d,%4d | Current:%4d,%4d", ih->userwidth, ih->userheight, ih->naturalwidth, ih->naturalheight, ih->currentwidth, ih->currentheight);

  IupSetAttributeId(layoutdlg->tree, "COLOR", id, "255 0 0");
  
  iupAttribSetStr(IupGetDialog(layoutdlg->tree), "_IUPLAYOUT_MARK", (char*)ih);
  IupUpdate(IupGetBrother(layoutdlg->tree));

  if (layoutdlg->properties && IupGetInt(layoutdlg->properties, "VISIBLE"))
    iLayoutUpdateProperties(layoutdlg->properties, ih);
}

static Ihandle* iLayoutFindElement(Ihandle* ih, int native_parent_x, int native_parent_y, int x, int y)
{
  Ihandle *child, *elem;
  int dx, dy;

  if (iupAttribGetLocal(ih, "VISIBLE"))
  {
    /* check the element */
    if (x >= ih->x+native_parent_x &&
        y >= ih->y+native_parent_y && 
        x < ih->x+native_parent_x+ih->currentwidth &&
        y < ih->y+native_parent_y+ih->currentheight)
    {
      if (ih->iclass->childtype != IUP_CHILDNONE)
      {
        /* if ih is a native parent, then update the offset */
        if (ih->iclass->nativetype!=IUP_TYPEVOID)
        {
          dx = 0, dy = 0;
          IupGetIntInt(ih, "CLIENTOFFSET", &dx, &dy);
          native_parent_x += ih->x+dx;
          native_parent_y += ih->y+dy;

          /* if ih is a Tabs, then find only the active child */
          if (iupStrEqualNoCase(ih->iclass->name, "tabs"))
          {
            child = (Ihandle*)IupGetAttribute(ih, "VALUE_HANDLE");
            if (child)
              return iLayoutFindElement(child, native_parent_x, native_parent_y, x, y);
            return NULL;
          }
        }
      }

      /* check its children */
      for (child = ih->firstchild; child; child = child->brother)
      {
        elem = iLayoutFindElement(child, native_parent_x, native_parent_y, x, y);
        if (elem)
          return elem;
      }

      return ih;
    }
  }
  return NULL;
}

static Ihandle* iLayoutFindDialogElement(iLayoutDialog* layoutdlg, int x, int y)
{                                            
  int w, h;

  /* check the dialog */
  IupGetIntInt(layoutdlg->dialog, "CLIENTSIZE", &w, &h);
  if (layoutdlg->dialog->firstchild &&
      x >= 0 && y >= 0 &&
      x < w && y < h)
  {
    Ihandle* elem;
    int native_parent_x = 0, native_parent_y = 0;
    IupGetIntInt(layoutdlg->dialog, "CLIENTOFFSET", &native_parent_x, &native_parent_y);
    elem = iLayoutFindElement(layoutdlg->dialog->firstchild, native_parent_x, native_parent_y, x, y);
    if (elem)
      return elem;
    return layoutdlg->dialog;
  }
  return NULL;
}

static int iLayoutCanvasButton_CB(Ihandle* canvas, int but, int pressed, int x, int y, char* status)
{
  (void)status;
  if (but==IUP_BUTTON1 && pressed)
  {
    Ihandle* dlg = IupGetDialog(canvas);
    iLayoutDialog* layoutdlg = (iLayoutDialog*)iupAttribGet(dlg, "_IUP_LAYOUTDIALOG");
    Ihandle* elem = iLayoutFindDialogElement(layoutdlg, x, y);
    if (elem && elem != layoutdlg->dialog)
    {
      if (iup_isdouble(status))
      {
        iLayoutBlink(elem);
      }
      else
      {
        int id = IupTreeGetId(layoutdlg->tree, elem);
        IupSetAttributeId(layoutdlg->tree, "COLOR", IupGetInt(layoutdlg->tree, "VALUE"), "0 0 0");
        IupSetfAttribute(layoutdlg->tree, "VALUE", "%d", id);
        iLayoutUpdateMark(layoutdlg, elem, id);
      }
    }
    else if (iupAttribGet(dlg, "_IUPLAYOUT_MARK"))
    {
      iupAttribSetStr(dlg, "_IUPLAYOUT_MARK", NULL);
      IupUpdate(canvas);
    }
  }
  else if (but==IUP_BUTTON3 && pressed)
  {
    Ihandle* dlg = IupGetDialog(canvas);
    iLayoutDialog* layoutdlg = (iLayoutDialog*)iupAttribGet(dlg, "_IUP_LAYOUTDIALOG");
    Ihandle* elem = iLayoutFindDialogElement(layoutdlg, x, y);
    if (elem && elem != layoutdlg->dialog)
      iLayoutContextMenu(layoutdlg, elem, dlg);
  }
  return IUP_DEFAULT;
}

static int iLayoutTreeExecuteLeaf_CB(Ihandle *tree, int id)
{
  Ihandle* elem = (Ihandle*)IupTreeGetUserId(tree, id);
  iLayoutBlink(elem);
  return IUP_DEFAULT;
}

static int iLayoutTreeRightClick_CB(Ihandle *tree, int id)
{
  Ihandle* dlg = IupGetDialog(tree);
  iLayoutDialog* layoutdlg = (iLayoutDialog*)iupAttribGet(dlg, "_IUP_LAYOUTDIALOG");
  Ihandle* elem = (Ihandle*)IupTreeGetUserId(tree, id);
  iLayoutContextMenu(layoutdlg, elem, dlg);
  return IUP_DEFAULT;
}

static int iLayoutTreeSelection_CB(Ihandle* tree, int id, int status)
{
  if (status == 1)
  {
    Ihandle* dlg = IupGetDialog(tree);
    Ihandle* elem = (Ihandle*)IupTreeGetUserId(tree, id);
    iLayoutDialog* layoutdlg = (iLayoutDialog*)iupAttribGet(dlg, "_IUP_LAYOUTDIALOG");
    iLayoutUpdateMark(layoutdlg, elem, id);
  }
  else
    IupSetAttributeId(tree, "COLOR", id, "0 0 0");
  return IUP_DEFAULT;
}

static int iLayoutDialogKAny_CB(Ihandle* dlg, int key)
{
  switch(key)
  {
  case K_F5:
    return iLayoutMenuUpdate_CB(dlg);
  case K_ESC:
    return iLayoutMenuClose_CB(dlg);
  case K_cO:
    return iLayoutMenuLoad_CB(dlg);
  case K_cF5:
    return iLayoutMenuReload_CB(dlg);
  case K_cMinus:
  case K_cPlus:
    {
      int opacity = IupGetInt(dlg, "OPACITY");
      if (opacity == 0)
        opacity = 255;
      if (key == K_cPlus)
        opacity++;
      else
        opacity--;
      if (opacity == 0 || opacity == 255)
        IupSetAttribute(dlg, "OPACITY", NULL);
      else
        IupSetfAttribute(dlg,"OPACITY", "%d", opacity);
      break;
    }
  }

  return IUP_DEFAULT;
}

Ihandle* IupLayoutDialog(Ihandle* dialog)
{
  Ihandle *tree, *canvas, *dlg, *menu, *status, *split;
  iLayoutDialog* layoutdlg;

  layoutdlg = calloc(1, sizeof(iLayoutDialog));
  if (dialog)
    layoutdlg->dialog = dialog;
  else
  {
    layoutdlg->dialog = IupDialog(NULL);
    layoutdlg->destroy = 1;
  }

  layoutdlg->timer = IupTimer();
  IupSetCallback(layoutdlg->timer, "ACTION_CB", iLayoutAutoUpdate_CB);
  IupSetAttribute(layoutdlg->timer, "TIME", "300");
  IupSetAttribute(layoutdlg->timer, "_IUP_LAYOUTDIALOG", (char*)layoutdlg);

  canvas = IupCanvas(NULL);
  IupSetCallback(canvas, "ACTION", iLayoutCanvas_CB);
  IupSetCallback(canvas, "BUTTON_CB", (Icallback)iLayoutCanvasButton_CB);

  tree = IupTree();
  layoutdlg->tree = tree;
  IupSetAttribute(tree, "RASTERSIZE", NULL);
  IupSetCallback(tree, "SELECTION_CB", (Icallback)iLayoutTreeSelection_CB);
  IupSetCallback(tree, "EXECUTELEAF_CB", (Icallback)iLayoutTreeExecuteLeaf_CB);
  IupSetCallback(tree, "RIGHTCLICK_CB", (Icallback)iLayoutTreeRightClick_CB);

  status = IupLabel(NULL);
  IupSetAttribute(status, "EXPAND", "HORIZONTAL");
  IupSetAttribute(status, "FONT", "Courier, 11");
  IupSetAttribute(status, "SIZE", "x12");
  layoutdlg->status = status;

  split = IupSplit(tree, canvas);
  IupSetAttribute(split, "VALUE", "300");

  menu = IupMenu(
    IupSubmenu("&Dialog", IupMenu(
      IupSetCallbacks(IupItem("New", NULL), "ACTION", iLayoutMenuNew_CB, NULL),
      IupSetCallbacks(IupItem("Load...\tCtrl+O", NULL), "ACTION", iLayoutMenuLoad_CB, NULL),
      IupSetCallbacks(IupItem("Load Visible...", NULL), "ACTION", iLayoutMenuLoadVisible_CB, NULL),
      IupSetCallbacks(IupItem("Reload\tCtrl+F5", NULL), "ACTION", iLayoutMenuReload_CB, NULL),
      IupSeparator(),
      IupSetCallbacks(IupItem("Refresh", NULL), "ACTION", iLayoutMenuRefresh_CB, NULL),
      IupSetCallbacks(IupItem("Redraw", NULL), "ACTION", iLayoutMenuRedraw_CB, NULL),
      IupSetCallbacks(IupItem("Show", NULL), "ACTION", iLayoutMenuShow_CB, NULL),
      IupSeparator(),
      IupSetCallbacks(IupItem("&Close\tEsc", NULL), "ACTION", iLayoutMenuClose_CB, NULL),
      NULL)),
    IupSubmenu("&Layout", IupMenu(
      IupSetCallbacks(IupSetAttributes(IupItem("&Hierarchy", NULL), "AUTOTOGGLE=YES, VALUE=ON"), "ACTION", iLayoutMenuTree_CB, NULL),
      IupSetCallbacks(IupItem("Update\tF5", NULL), "ACTION", iLayoutMenuUpdate_CB, NULL),
      IupSetCallbacks(IupSetAttributes(IupItem("Auto Update", NULL), "AUTOTOGGLE=YES, VALUE=OFF"), "ACTION", iLayoutMenuAutoUpdate_CB, NULL),
      IupSetCallbacks(IupItem("Opacity\tCtrl+/Ctrl-", NULL), "ACTION", iLayoutMenuOpacity_CB, NULL),
      NULL)),
    NULL);

  dlg = IupDialog(IupVbox(split, status, NULL));
  IupSetAttribute(dlg,"TITLE", "Dialog Layout");
  IupSetAttribute(dlg,"PARENTDIALOG", IupGetGlobal("PARENTDIALOG"));
  IupSetAttribute(dlg,"ICON", IupGetGlobal("ICON"));
  IupSetCallback(dlg, "DESTROY_CB", iLayoutDialogDestroy_CB);
  IupSetCallback(dlg, "SHOW_CB", (Icallback)iLayoutDialogShow_CB);
  IupSetCallback(dlg, "K_ANY", (Icallback)iLayoutDialogKAny_CB);
  IupSetCallback(dlg, "CLOSE_CB", iLayoutDialogClose_CB);
  iupAttribSetStr(dlg, "_IUP_LAYOUTDIALOG", (char*)layoutdlg);
  IupSetAttributeHandle(dlg, "MENU", menu);

  IupSetAttribute(dlg,"BACKCOLOR", "255 255 255");
  IupSetAttribute(dlg,"FORECOLOR", "0 0 0");
  IupSetAttribute(dlg,"FORECOLORVOID", "164 164 164");
  IupSetAttribute(dlg,"FORECOLORMAX", "255 0 0");
  IupSetAttribute(dlg,"DESTROYWHENCLOSED", "Yes");

  {
    int w = 0, h = 0;
    IupGetIntInt(layoutdlg->dialog, "RASTERSIZE", &w, &h);
    if (w && h)
      IupSetfAttribute(dlg, "RASTERSIZE", "%dx%d", (int)(w*1.3), h);
    else
      IupSetAttribute(dlg, "SIZE", "HALFxHALF");
  }

  IupMap(dlg);

  iLayoutRebuildTree(layoutdlg);

  return dlg;
}
