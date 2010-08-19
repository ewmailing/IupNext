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
  Ihandle *dialog, *tree;
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
  if (layoutdlg->destroy)
    IupDestroy(layoutdlg->dialog);
  free(layoutdlg);
  return IUP_DEFAULT;
}

static char* iLayoutGetTitle(Ihandle* ih)
{
  char* title = IupGetAttribute(ih, "TITLE");
  char* name = IupGetName(ih);
  char* str = iupStrGetMemory(1000);
  if (title)
  {
    if (name)
      sprintf(str, "%s: %s [%s]", IupGetClassName(ih), title, name);
    else
      sprintf(str, "%s: %s", IupGetClassName(ih), title);
  }
  else
  {
    if (name)
      sprintf(str, "%s [%s]", IupGetClassName(ih), name);
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

static void iLayoutRebuildTree(Ihandle* dlg, iLayoutDialog* layoutdlg)
{
  Ihandle* tree = layoutdlg->tree;
  IupSetAttribute(tree, "DELNODE0", "CHILDREN");

  iLayoutTreeSetNodeInfo(tree, 0, layoutdlg->dialog);
  iLayoutTreeAddChildren(tree, 0, layoutdlg->dialog);

  /* redraw canvas */
  IupUpdate(IupGetNextChild(dlg, tree));
}

static int iLayoutMenuNew_CB(Ihandle* ih)
{
  Ihandle* dlg = IupGetDialog(ih);
  iLayoutDialog* layoutdlg = (iLayoutDialog*)iupAttribGet(dlg, "_IUP_LAYOUTDIALOG");
  if (layoutdlg->destroy)
    IupDestroy(layoutdlg->dialog);
  layoutdlg->dialog = IupDialog(NULL);
  layoutdlg->destroy = 1;
  iLayoutRebuildTree(dlg, layoutdlg);
  return IUP_DEFAULT;
}

static int iLayoutMenuReload_CB(Ihandle* ih)
{
  Ihandle* dlg = IupGetDialog(ih);
  iLayoutDialog* layoutdlg = (iLayoutDialog*)iupAttribGet(dlg, "_IUP_LAYOUTDIALOG");
  iLayoutRebuildTree(dlg, layoutdlg);
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
  return IUP_DEFAULT;
}

static int iLayoutMenuUpdate_CB(Ihandle* ih)
{
  Ihandle* dlg = IupGetDialog(ih);
  iLayoutDialog* layoutdlg = (iLayoutDialog*)iupAttribGet(dlg, "_IUP_LAYOUTDIALOG");
  /* redraw canvas */
  IupUpdate(IupGetNextChild(dlg, layoutdlg->tree));
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

static void iLayoutDialogLoad(iLayoutDialog* layoutdlg, int only_visible)
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

  ret = IupListDialog(1,"Dialogs",count,dlg_list_str,1,30,count+1,NULL);
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
  iLayoutDialogLoad(layoutdlg, 0);
  iLayoutRebuildTree(dlg, layoutdlg);
  return IUP_DEFAULT;
}

static int iLayoutMenuLoadVisible_CB(Ihandle* ih)
{
  Ihandle* dlg = IupGetDialog(ih);
  iLayoutDialog* layoutdlg = (iLayoutDialog*)iupAttribGet(dlg, "_IUP_LAYOUTDIALOG");
  iLayoutDialogLoad(layoutdlg, 1);
  iLayoutRebuildTree(dlg, layoutdlg);
  return IUP_DEFAULT;
}

static void iLayoutGetNativeParentOffset(Ihandle* ih, int *x, int *y)
{
  int dx = 0, dy = 0;
  Ihandle* native_parent = iupChildTreeGetNativeParent(ih);
  while(native_parent)
  {
    IupGetIntInt(native_parent, "CLIENTOFFSET", &dx, &dy);
    *x += native_parent->x+dx;
    *y += native_parent->y+dy;
    native_parent = iupChildTreeGetNativeParent(native_parent);
  }
}

static void iLayoutDrawElements(iLayoutDialog* layoutdlg, int showhidden, Ihandle* canvas, IdrawCanvas* dc, 
                                unsigned char fr, unsigned char fg, unsigned char fb,
                                unsigned char br, unsigned char bg, unsigned char bb)
{
  Ihandle* tree = layoutdlg->tree;
  int x, y, w, h, id, 
      count = IupGetInt(tree, "COUNT");
  for (id=0; id<count; id++)
  {
    Ihandle* ih = (Ihandle*)IupTreeGetUserId(tree, id);
    if (id == 0) /* exclude dialog decorations */
    {
      IupGetIntInt(ih, "CLIENTSIZE", &w, &h);
      iupDrawRectangle(dc, 0, 0, w-1, h-1, fr, fg, fb, IUP_DRAW_STROKE);
    }
    else if (showhidden || iupAttribGetLocal(ih, "VISIBLE"))
    {
      char *title, *bgcolor;
      unsigned char r, g, b;

      x = ih->x;
      y = ih->y;
      w = ih->currentwidth;
      h = ih->currentheight;

      iLayoutGetNativeParentOffset(ih, &x, &y);

      bgcolor = iupAttribGetLocal(ih, "BGCOLOR");
      if (bgcolor)
      {
        r = br, g = bg, b = bb;
        iupStrToRGB(bgcolor, &r, &g, &b);
        iupDrawRectangle(dc, x, y, x+w-1, y+h-1, r, g, b, IUP_DRAW_FILL);
      }

      iupDrawRectangle(dc, x, y, x+w-1, y+h-1, fr, fg, fb, ih->iclass->nativetype==IUP_TYPEVOID? IUP_DRAW_STROKE_DASH: IUP_DRAW_STROKE);

      title = iupAttribGetLocal(ih, "TITLE");
      if (title)
      {
        char* line = strstr(title, "\n");
        int len;
        if (line) len = line-title;
        else len = strlen(title);
        IupSetAttribute(canvas, "FONT", IupGetAttribute(ih, "FONT"));
        iupDrawSetClipRect(dc, x+1, y+1, x+w-2, y+h-2);
        r = fr, g = fg, b = fb;
        iupStrToRGB(iupAttribGetLocal(ih, "FGCOLOR"), &r, &g, &b);
        iupDrawText(dc, title, len, x+1, y+1, r, g, b);
        iupDrawResetClip(dc);
      }
    }
  }
}

static int iLayoutCanvas_CB(Ihandle* canvas)
{
  Ihandle* dlg = IupGetDialog(canvas);
  iLayoutDialog* layoutdlg = (iLayoutDialog*)iupAttribGet(dlg, "_IUP_LAYOUTDIALOG");
  IdrawCanvas* dc = iupDrawCreateCanvas(canvas);
  int showhidden = IupGetInt(dlg, "SHOWHIDDEN");
  int w, h;
  unsigned char br, bg, bb, fr, fg, fb;

  br = 255, bg = 255, bb = 255;
  iupStrToRGB(IupGetAttribute(dlg, "BGCOLOR"), &br, &bg, &bb);
  iupDrawGetSize(dc, &w, &h);
  iupDrawRectangle(dc, 0, 0, w-1, h-1, br, bg, bb, IUP_DRAW_FILL);

  br = 0, bg = 0, bb = 0;
  iupStrToRGB(IupGetAttribute(dlg, "FGCOLOR"), &fr, &fg, &fb);
  iLayoutDrawElements(layoutdlg, showhidden, canvas, dc, fr, fg, fb, br, bg, bb);

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
    IupSubmenu("Dialog", IupMenu(
      IupSetCallbacks(IupItem("New", NULL), "ACTION", iLayoutMenuNew_CB, NULL),
      IupSetCallbacks(IupItem("Load...", NULL), "ACTION", iLayoutMenuLoad_CB, NULL),
      IupSetCallbacks(IupItem("Load Visible...", NULL), "ACTION", iLayoutMenuLoadVisible_CB, NULL),
      IupSetCallbacks(IupItem("Reload\tCtrl+F5", NULL), "ACTION", iLayoutMenuReload_CB, NULL),
      IupSeparator(),
      IupSetCallbacks(IupItem("Refresh", NULL), "ACTION", iLayoutMenuRefresh_CB, NULL),
      IupSetCallbacks(IupItem("Redraw", NULL), "ACTION", iLayoutMenuRedraw_CB, NULL),
      IupSetCallbacks(IupItem("Show", NULL), "ACTION", iLayoutMenuShow_CB, NULL),
      IupSeparator(),
      IupSetCallbacks(IupItem("Close\tEsc", NULL), "ACTION", iLayoutMenuClose_CB, NULL),
      NULL)),
    IupSubmenu("Layout", IupMenu(
      IupSetCallbacks(IupSetAttributes(IupItem("Hierarchy", NULL), "AUTOTOGGLE=YES, VALUE=ON"), "ACTION", iLayoutMenuTree_CB, NULL),
      IupSetCallbacks(IupItem("Update\tF5", NULL), "ACTION", iLayoutMenuUpdate_CB, NULL),
      IupSetCallbacks(IupItem("Opacity\tCtrl+/Ctrl-", NULL), "ACTION", iLayoutMenuOpacity_CB, NULL),
//      IupSetCallbacks(IupItem("Expanded", NULL), "ACTION", iLayoutMenuExpanded_CB, NULL),
      NULL)),
    NULL);

  dlg = IupDialog(IupSetAttributes(IupSplit(tree, canvas), "VALUE=300"));
  IupSetAttribute(dlg,"TITLE", "Dialog Layout");
  IupSetAttribute(dlg,"MINBOX","NO");
  IupSetAttribute(dlg,"MAXBOX","NO");
  IupSetAttribute(dlg,"PARENTDIALOG", IupGetGlobal("PARENTDIALOG"));
  IupSetAttribute(dlg,"ICON", IupGetGlobal("ICON"));
  IupSetCallback(dlg, "DESTROY_CB", iLayoutDialogDestroy_CB);
  IupSetCallback(dlg, "SHOW_CB", (Icallback)iLayoutDialogShow_CB);
  IupSetCallback(dlg, "K_ANY", (Icallback)iLayoutDialogKAny_CB);
  IupSetCallback(dlg, "CLOSE_CB", iLayoutDialogClose_CB);
  iupAttribSetStr(dlg, "_IUP_LAYOUTDIALOG", (char*)layoutdlg);
  IupSetAttributeHandle(dlg, "MENU", menu);

  IupSetAttribute(dlg,"BGCOLOR", "255 255 255");
  IupSetAttribute(dlg,"FGCOLOR", "0 0 0");
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

  iLayoutRebuildTree(dlg, layoutdlg);

  return dlg;
}

