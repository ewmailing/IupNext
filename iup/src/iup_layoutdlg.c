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


typedef struct _iLayoutDialog {
  int destroy;
  Ihandle *dialog, *tree, *timer;
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
  if (layoutdlg->destroy)
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
      sprintf(str, "%s: %.50s [%.50s]", IupGetClassName(ih), title, name);
    else
      sprintf(str, "%s: %.50s", IupGetClassName(ih), title);
  }
  else
  {
    if (name)
      sprintf(str, "%s [%.50s]", IupGetClassName(ih), name);
    else
      sprintf(str, "%s", IupGetClassName(ih));
  }
  return str;
}

static void iLayoutTreeSetNodeInfo(Ihandle* tree, int id, Ihandle* ih)
{
  IupTreeSetAttribute(tree, "TITLE", id, iLayoutGetTitle(ih));
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
        IupTreeSetAttribute(tree, "ADDBRANCH", last_child_id, "");
        last_child_id++;
      }
      else
      {
        IupTreeSetAttribute(tree, "INSERTBRANCH", last_child_id, "");
        last_child_id = IupGetInt(tree, "LASTADDNODE");
      }

      iLayoutTreeAddChildren(tree, last_child_id, child);
    }
    else
    {
      if (child == parent->firstchild)
      {
        IupTreeSetAttribute(tree, "ADDLEAF", last_child_id, "");
        last_child_id++;
      }
      else
      {
        IupTreeSetAttribute(tree, "INSERTLEAF", last_child_id, "");
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
    IupDestroy(dlg);
  else
    IupHide(dlg);
  return IUP_DEFAULT;
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

static void iLayoutDialogLoad(Ihandle* lo_dlg, iLayoutDialog* layoutdlg, int only_visible)
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

  IupStoreGlobal("_IUP_OLD_PARENTDIALOG", IupGetGlobal("PARENTDIALOG"));
  IupSetAttributeHandle(NULL, "PARENTDIALOG", lo_dlg);

  ret = IupListDialog(1,"Dialogs",count,(const char**)dlg_list_str,1,30,count+1,NULL);

  IupStoreGlobal("PARENTDIALOG", IupGetGlobal("_IUP_OLD_PARENTDIALOG"));
  IupSetGlobal("_IUP_OLD_PARENTDIALOG", NULL);

  if (ret != -1)
  {
    if (layoutdlg->destroy)
      IupDestroy(layoutdlg->dialog);
    layoutdlg->dialog = dlg_list[ret];
    layoutdlg->destroy = 0;
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

static void iLayoutDrawElement(IdrawCanvas* dc, Ihandle* ih, int native_parent_x, int native_parent_y,
                               unsigned char fr, unsigned char fg, unsigned char fb,
                               unsigned char fvr, unsigned char fvg, unsigned char fvb,
                               unsigned char fmr, unsigned char fmg, unsigned char fmb,
                               unsigned char br, unsigned char bg, unsigned char bb)
{
  int x, y, w, h; 
  char *title, *bgcolor, *image;
  unsigned char r, g, b;

  x = ih->x+native_parent_x;
  y = ih->y+native_parent_y;
  w = ih->currentwidth;
  h = ih->currentheight;
  if (w<=0) w=1;
  if (h<=0) h=1;

  bgcolor = iupAttribGetLocal(ih, "BGCOLOR");
  if (bgcolor)
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

  /* always draw the image first */

  image = iupAttribGetLocal(ih, "IMAGE0");  /* Tree root node title */
  if (!image) 
    image = iupAttribGetLocal(ih, "TABIMAGE0");  /* Tabs first tab image */
  if (image)
  {
    /* returns the image of the active tab */
    int pos = IupGetInt(ih, "VALUEPOS");
    image = IupTreeGetAttribute(ih, "TABIMAGE", pos);
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

    position = iupAttribGetLocal(ih, "IMAGEPOSITION");
    if (position &&
        (iupStrEqualNoCase(position, "BOTTOM") ||
         iupStrEqualNoCase(position, "TOP")))
      y += img_h;
    else
      x += img_w;
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
      title = IupTreeGetAttribute(ih, "TABTITLE", pos);
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

  if (ih->iclass->nativetype!=IUP_TYPEVOID &&
      ih->iclass->childtype==IUP_CHILDNONE &&
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

static void iLayoutDrawElementTree(IdrawCanvas* dc, int showhidden, Ihandle* ih, int native_parent_x, int native_parent_y,
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
    iLayoutDrawElement(dc, ih, native_parent_x, native_parent_y,
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
        if (iupStrEqualNoCase(ih->iclass->name, "tabs"))
        {
          child = (Ihandle*)IupGetAttribute(ih, "VALUE_HANDLE");
          if (child)
            iLayoutDrawElementTree(dc, showhidden, child, native_parent_x, native_parent_y,
                                   fr, fg, fb, fvr, fvg, fvb, fmr, fmg, fmb, br, bg, bb);
          return;
        }
      }
    }

    /* draw its children */
    for (child = ih->firstchild; child; child = child->brother)
    {
      iLayoutDrawElementTree(dc, showhidden, child, native_parent_x, native_parent_y,
                             fr, fg, fb, fvr, fvg, fvb, fmr, fmg, fmb, br, bg, bb);
    }
  }
}

static void iLayoutDrawDialog(iLayoutDialog* layoutdlg, int showhidden, IdrawCanvas* dc, 
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
    iLayoutDrawElementTree(dc, showhidden, layoutdlg->dialog->firstchild, native_parent_x, native_parent_y,
                           fr, fg, fb, fvr, fvg, fvb, fmr, fmg, fmb, br, bg, bb);
  }
}

static int iLayoutCanvas_CB(Ihandle* canvas)
{
  Ihandle* dlg = IupGetDialog(canvas);
  iLayoutDialog* layoutdlg = (iLayoutDialog*)iupAttribGet(dlg, "_IUP_LAYOUTDIALOG");
  IdrawCanvas* dc = iupDrawCreateCanvas(canvas);
  int showhidden = IupGetInt(dlg, "SHOWHIDDEN");
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

  iLayoutDrawDialog(layoutdlg, showhidden, dc, 
                      fr, fg, fb, 
                      fvr, fvg, fvb, 
                      fmr, fmg, fmb, 
                      br, bg, bb);

  iupDrawFlush(dc);

  iupDrawKillCanvas(dc);

  return IUP_DEFAULT;
}

static int iLayoutDialogKAny_CB(Ihandle* dlg, int key)
{
  switch(key)
  {
  case K_F5:
    iLayoutMenuUpdate_CB(dlg);
    break;
  case K_ESC:
    iLayoutMenuClose_CB(dlg);
    break;
  case K_cO:
    iLayoutMenuLoad_CB(dlg);
    break;
  case K_cF5:
    iLayoutMenuReload_CB(dlg);
    break;
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
  Ihandle *tree, *canvas, *dlg, *menu;
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
//BUTTON_CB

  tree = IupTree();
  layoutdlg->tree = tree;
  IupSetAttribute(tree, "RASTERSIZE", NULL);
//  SELECTION_CB
//  RIGHTCLICK_CB
//int function(Ihandle *ih, int id, int status)

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
/*      IupSetCallbacks(IupItem("Expanded", NULL), "ACTION", iLayoutMenuExpanded_CB, NULL), */
      NULL)),
    NULL);

  dlg = IupDialog(IupSetAttributes(IupSplit(tree, canvas), "VALUE=300"));
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
    IupGetIntInt(dlg, "RASTERSIZE", &w, &h);
    if (w && h)
      IupSetfAttribute(dlg, "RASTERSIZE", "%dx%d", (int)(w*1.3), h);
    else
      IupSetAttribute(dlg, "SIZE", "HALFxHALF");
  }

  IupMap(dlg);

  iLayoutRebuildTree(layoutdlg);

  return dlg;
}
