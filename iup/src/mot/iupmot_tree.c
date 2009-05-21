#include <Xm/Xm.h>
#include <Xm/Form.h>
#include <Xm/Container.h>
#include <Xm/IconG.h>
#include <Xm/ScrolledW.h>
#include <Xm/XmosP.h>
#include <Xm/Text.h>
#include <Xm/Transfer.h>
#include <X11/keysym.h>

#include <sys/stat.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <stdarg.h>
#include <limits.h>
#include <time.h>

#include "iup.h"
#include "iupcbs.h"

#include "iup_object.h"
#include "iup_childtree.h"
#include "iup_dialog.h"
#include "iup_layout.h"
#include "iup_attrib.h"
#include "iup_str.h"
#include "iup_drv.h"
#include "iup_drvfont.h"
#include "iup_stdcontrols.h"
#include "iup_key.h"
#include "iup_image.h"
#include "iup_array.h"
#include "iup_tree.h"

#include "iupmot_drv.h"
#include "iupmot_color.h"


typedef struct _motTreeItemData
{
  Pixmap image;
  Pixmap image_expanded;
  unsigned char kind;
  void* userdata;
} motTreeItemData;


static int motTreeRenameNode_CB(Ihandle* ih);

/*****************************************************************************/
/* COPYING ITEMS (Branches and its children)                                 */
/*****************************************************************************/
/* Insert the copied item in a new location. Returns the new item. */
static Widget motTreeCopyItem(Ihandle* ih, Widget wItem, Widget wParent)
{
  Widget wNewItem;
  XmString title;
  motTreeItemData *itemData;
  Pixel fgcolor;
  int num_args = 0;
  Arg args[30];
  Pixmap image = XmUNSPECIFIED_PIXMAP;

  // TODO: o que fazer com o motTreeItemData?

  /* Get values to copy */
  XtVaGetValues(wItem, XmNlabelString, &title,
                            XmNuserData, &itemData,
                        XmNforeground, &fgcolor,
                   XmNsmallIconPixmap, &image, NULL);

  iupmotSetArg(args, num_args,  XmNlabelString, title);
  iupmotSetArg(args, num_args,  XmNentryParent, wParent);
  iupmotSetArg(args, num_args,       XmNuserData, itemData);
  iupmotSetArg(args, num_args,   XmNforeground, fgcolor);
  iupmotSetArg(args, num_args, XmNmarginHeight, 0);
  iupmotSetArg(args, num_args,  XmNmarginWidth, 0);
  iupmotSetArg(args, num_args,     XmNviewType, XmSMALL_ICON);
  iupmotSetArg(args, num_args, XmNnavigationType, XmTAB_GROUP);
  iupmotSetArg(args, num_args, XmNtraversalOn, True);
  iupmotSetArg(args, num_args, XmNshadowThickness, 0);
  iupmotSetArg(args, num_args, XmNsmallIconPixmap, image);

  wNewItem = XtCreateManagedWidget("icon", xmIconGadgetClass, ih->handle, args, num_args);
  
  /* Root always expanded */
  XtVaSetValues((Widget)iupAttribGet(ih, "_IUPMOTTREE_ROOTITEM"), XmNoutlineState, XmEXPANDED, NULL);

  XtRealizeWidget(wNewItem);

  return wNewItem;
}

/* Copies all items in a branch to a new location. Returns the new branch node. */
static Widget motTreeCopyBranch(Ihandle* ih, Widget wBranch, Widget wNewParent)
{
  Widget wNewItem = motTreeCopyItem(ih, wBranch, wNewParent);
  WidgetList wItemChildList = NULL;
  int hasItem = 0;
  int numChild = XmContainerGetItemChildren(ih->handle, wBranch, &wItemChildList);

  while(hasItem != numChild)
  {
    /* Recursively transfer all the items */
    motTreeCopyBranch(ih, wItemChildList[hasItem], wNewItem);  

    /* Go to next sibling item */
    hasItem++;
  }

  if (wItemChildList) XtFree((char*)wItemChildList);
  return wNewItem;
}

static void motTreeDragActivate(Widget w)
{
  XButtonEvent ev;
  String params[1];

  params[0] = "Move";

  memset(&ev, 0, sizeof(XButtonEvent));
  ev.type = ButtonPress;
  ev.display = XtDisplay(w);
  ev.send_event = True;
  ev.root = RootWindow(iupmot_display, iupmot_screen);
  ev.time = clock()*CLOCKS_PER_SEC;
  ev.window = XtWindow(w);
  ev.state = Button1Mask | Button2Mask;
  ev.button = Button1 | Button2;
  ev.same_screen = True;

  XtCallActionProc(w, "ContainerStartTransfer", (XEvent*)&ev, (String*)params, 1); 
}

static void motTreeContainerDeselectAll(Ihandle *ih)
{
  XKeyEvent ev;

  memset(&ev, 0, sizeof(XKeyEvent));
  ev.type = KeyPress;
  ev.display = XtDisplay(ih->handle);
  ev.send_event = True;
  ev.root = RootWindow(iupmot_display, iupmot_screen);
  ev.time = clock()*CLOCKS_PER_SEC;
  ev.window = XtWindow(ih->handle);
  ev.state = ControlMask;
  ev.keycode = XK_backslash;
  ev.same_screen = True;

  XtCallActionProc(ih->handle, "ContainerDeselectAll", (XEvent*)&ev, 0, 0);
}

static void motTreeContainerSelectAll(Ihandle *ih)
{
  XKeyEvent ev;

  memset(&ev, 0, sizeof(XKeyEvent));
  ev.type = KeyPress;
  ev.display = XtDisplay(ih->handle);
  ev.send_event = True;
  ev.root = RootWindow(iupmot_display, iupmot_screen);
  ev.time = clock()*CLOCKS_PER_SEC;
  ev.window = XtWindow(ih->handle);
  ev.state = ControlMask;
  ev.keycode = XK_slash;
  ev.same_screen = True;

  XtCallActionProc(ih->handle, "ContainerSelectAll", (XEvent*)&ev, 0, 0);
}

static Widget motTreeGetNextNode(Ihandle* ih, Widget wItem)
{
  Widget wItemParent, wRoot;
  WidgetList wBrothersTree = NULL;
  int numBrothers, i = 0;

  XtVaGetValues(wItem, XmNentryParent, &wItemParent, NULL);
  numBrothers = XmContainerGetItemChildren(ih->handle, wItemParent, &wBrothersTree);
  wRoot = (Widget)iupAttribGet(ih, "_IUPMOTTREE_ROOTITEM");

  while(wBrothersTree[i] != wItem)
    i++;

  if(i >= numBrothers-1)
  {
    if (wItemParent == wRoot)
      wItem = wItemParent;
    else
      wItem = motTreeGetNextNode(ih, wItemParent);
  }
  else
  {
    /* item selected is not the last child */
    wItem = wBrothersTree[i+1];
  }

  if (wBrothersTree) XtFree((char*)wBrothersTree);
  return wItem;
}

static Widget motTreeGetLastPrevNode(Ihandle* ih, Widget wItem)
{
  unsigned char itemState;

  ih->data->id_control++;
  XtVaGetValues(wItem, XmNoutlineState, &itemState, NULL);

  if(itemState == XmEXPANDED)
  {
    WidgetList wChildrenTree = NULL;
    int numChildren = XmContainerGetItemChildren(ih->handle, wItem, &wChildrenTree);
    if(numChildren)
      wItem = motTreeGetLastPrevNode(ih, wChildrenTree[numChildren - 1]);
    if (wChildrenTree) XtFree((char*)wChildrenTree);
  }

  return wItem;
}

static Widget motTreeFindCurrentVisibleNode(Ihandle* ih, WidgetList itemList, int numItems, Widget itemNode)
{
  Widget itemChild;
  WidgetList itemChildList;
  int hasItem = 0;
  int numChild;
  unsigned char itemState;

  while(hasItem != numItems)
  {
    /* ID control to traverse items */
    ih->data->id_control++;

    /* StateID founded! */
    if(itemList[hasItem] == itemNode)
      return itemList[hasItem];

    /* Check whether we have child items */
    itemChildList = NULL;
    numChild = XmContainerGetItemChildren(ih->handle, itemList[hasItem], &itemChildList);
    XtVaGetValues(itemList[hasItem], XmNoutlineState,  &itemState, NULL);

    /* The itemWidget has child and it is expanded */
    if (numChild && itemState == XmEXPANDED)
    {
      /* pass the list of children of this item */
      itemChild = motTreeFindCurrentVisibleNode(ih, itemChildList, numChild, itemNode);

      /* StateID founded! */
      if(itemChild == itemNode)
      {
        if (itemChildList) XtFree((char*)itemChildList);
        return itemChild;
      }
    }

    if (itemChildList) XtFree((char*)itemChildList);
    /* Go to next sibling item */
    hasItem++;
  }

  return itemList[hasItem];
}

static Widget motTreeFindNewVisibleNode(Ihandle* ih, WidgetList itemList, int numItems)
{
  Widget itemChild;
  WidgetList itemChildList;
  int hasItem = 0;
  int numChild;
  unsigned char itemState;

  while(hasItem != numItems)
  {
    /* ID control to traverse items */
    ih->data->id_control--;

    /* StateID founded! */
    if(ih->data->id_control < 0)
      return itemList[hasItem];

    /* Check whether we have child items */
    itemChildList = NULL;
    numChild = XmContainerGetItemChildren(ih->handle, itemList[hasItem], &itemChildList);
    XtVaGetValues(itemList[hasItem], XmNoutlineState,  &itemState, NULL);

    /* The itemWidget has child and it is expanded */
    if(numChild && itemState == XmEXPANDED)
    {
      /* pass the list of children of this item */
      itemChild = motTreeFindNewVisibleNode(ih, itemChildList, numChild);

      /* StateID founded! */
      if(ih->data->id_control < 0)
      {
        if (itemChildList) XtFree((char*)itemChildList);
        return itemChild;
      }
    }

    if (itemChildList) XtFree((char*)itemChildList);
    /* Go to next sibling item */
    hasItem++;
  }

  return itemList[hasItem];
}

static void motTreeUpdateBgColor(Ihandle* ih, WidgetList itemList, int numItems, Pixel bgcolor)
{
  WidgetList itemChildList;
  int hasItem = 0;
  int numChild;

  while(hasItem != numItems)
  {
    XtVaSetValues(itemList[hasItem], XmNbackground, bgcolor, NULL);

    /* Check whether we have child items */
    itemChildList = NULL;
    numChild = XmContainerGetItemChildren(ih->handle, itemList[hasItem], &itemChildList);
    if(numChild)
      motTreeUpdateBgColor(ih, itemChildList, numChild, bgcolor);
    if (itemChildList) XtFree((char*)itemChildList);

    /* Go to next sibling item */
    hasItem++;
  }
}

static void motTreeUpdateImages(Ihandle* ih, WidgetList itemList, int numItems, int mode)
{
  motTreeItemData *itemData;
  int i = 0;

  /* called when one of the default images is changed */

  while(i != numItems)
  {
    /* Get node attributes */
    XtVaGetValues(itemList[i], XmNuserData, &itemData, NULL);
	  
    if (itemData->kind == ITREE_BRANCH)
    {
      unsigned char itemState;
      XtVaGetValues(itemList[i], XmNoutlineState,  &itemState, NULL);
      
      if (itemState == XmEXPANDED)
      {
        if (mode == ITREE_UPDATEIMAGE_EXPANDED)
          XtVaSetValues(itemList[i], XmNsmallIconPixmap, (itemData->image_expanded!=XmUNSPECIFIED_PIXMAP)? itemData->image_expanded: (Pixmap)ih->data->def_image_expanded, NULL);
      }
      else 
      {
        if (mode == ITREE_UPDATEIMAGE_COLLAPSED)
          XtVaSetValues(itemList[i], XmNsmallIconPixmap, (itemData->image!=XmUNSPECIFIED_PIXMAP)? itemData->image: (Pixmap)ih->data->def_image_collapsed, NULL);
      }

      /* Recursively traverse child items */
      {
        WidgetList itemChildList;
        int numChild;
        itemChildList = NULL;
        numChild = XmContainerGetItemChildren(ih->handle, itemList[i], &itemChildList);
        motTreeUpdateImages(ih, itemChildList, numChild, mode);
        if (itemChildList) XtFree((char*)itemChildList);
      }
    }
    else 
    {
      if(mode == ITREE_UPDATEIMAGE_LEAF)
        XtVaSetValues(itemList[i], XmNsmallIconPixmap, (itemData->image!=XmUNSPECIFIED_PIXMAP)? itemData->image: (Pixmap)ih->data->def_image_leaf, NULL);
    }

    /* Go to next sibling node */
    i++;
  }
}

void motTreeGetDropWidget(Ihandle* ih, WidgetList itemList, int numItems, int x, int y)
{
  WidgetList itemChildList;
  int numChild;
  Position xItem, yItem;
  Dimension height, width;
  int hasItem = 0;

  while(hasItem != numItems && !iupAttribGet(ih, "_IUPTREE_DROPITEM"))
  {
    XtVaGetValues(itemList[hasItem], XmNheight, &height, NULL);
    XtVaGetValues(itemList[hasItem], XmNwidth, &width, NULL);
    XtVaGetValues(itemList[hasItem], XmNx, &xItem, NULL);
    XtVaGetValues(itemList[hasItem], XmNy, &yItem, NULL);
    
    if(x >= xItem && x <= (xItem + width) &&
       y >= yItem && y <= (yItem + height))
    {
      iupAttribSetStr(ih, "_IUPTREE_DROPITEM", (char*)itemList[hasItem]);
      return;
    }

    /* Check whether we have child items */
    itemChildList = NULL;
    numChild = XmContainerGetItemChildren(ih->handle, itemList[hasItem], &itemChildList);

    if(numChild)
      motTreeGetDropWidget(ih, itemChildList, numChild, x, y);

    if (itemChildList) XtFree((char*)itemChildList);
    /* Go to next sibling item */
    hasItem++;
  }
}

static void motTreeInvertAllNodeMarking(Ihandle* ih, WidgetList itemList, int numItems)
{
  WidgetList itemChildList;
  int numChild;
  int hasItem = 0;
  unsigned char isSelected;

  while(hasItem != numItems)
  {
    XtVaGetValues(itemList[hasItem], XmNvisualEmphasis, &isSelected, NULL);

    if(isSelected == XmSELECTED)
      XtVaSetValues(itemList[hasItem], XmNvisualEmphasis, XmNOT_SELECTED, NULL);
    else
      XtVaSetValues(itemList[hasItem], XmNvisualEmphasis, XmSELECTED, NULL);

    /* Check whether we have child items */
    itemChildList = NULL;
    numChild = XmContainerGetItemChildren(ih->handle, itemList[hasItem], &itemChildList);

    if(numChild)
      motTreeInvertAllNodeMarking(ih, itemChildList, numChild);

    if (itemChildList) XtFree((char*)itemChildList);
    /* Go to next sibling item */
    hasItem++;
  }
}

void motTreeExpandCollapseAllNodes(Ihandle* ih, WidgetList itemList, int numItems, unsigned char itemState)
{
  WidgetList itemChildList;
  int numChild;
  int hasItem = 0;

  while(hasItem != numItems)
  {
    /* Check whether we have child items */
    itemChildList = NULL;
    numChild = XmContainerGetItemChildren(ih->handle, itemList[hasItem], &itemChildList);

    if(numChild)
    {
      XtVaSetValues(itemList[hasItem], XmNoutlineState, itemState, NULL);
      motTreeExpandCollapseAllNodes(ih, itemChildList, numChild, itemState);
    }

    if (itemChildList) XtFree((char*)itemChildList);
    /* Go to next sibling item */
    hasItem++;
  }
}

static void motTreeDestroyItemData(Widget wItem)
{
  motTreeItemData *itemData;
  XtVaGetValues(wItem, XmNuserData, &itemData, NULL);
  free(itemData);
}

static void motTreeRemoveChildren(Ihandle* ih, WidgetList itemList, int numItems)
{
  WidgetList itemChildList;
  int numChild;
  int hasItem = 0;

  while(hasItem != numItems)
  {	  
    /* Check whether we have child items */
    itemChildList = NULL;
    numChild = XmContainerGetItemChildren(ih->handle, itemList[hasItem], &itemChildList);

    if(numChild)
      motTreeRemoveChildren(ih, itemChildList, numChild);

    motTreeDestroyItemData(itemList[hasItem]);
    XtDestroyWidget(itemList[hasItem]);

    if (itemChildList) XtFree((char*)itemChildList);
    /* Go to next sibling item */
    hasItem++;
  }
}

/* Recursively, find a new brother for the item
that will have its depth changed. Returns the new brother. */
static Widget motTreeFindNewBrother(Ihandle* ih, WidgetList itemList, int numItems, int hasItem)
{
  Widget itemChild;
  WidgetList itemChildList;
  int numChild;

  while(hasItem != numItems)
  {
    if(ih->data->id_control < 0)
      return itemList[hasItem];

    /* Check whether we have child items */
    itemChildList = NULL;
    numChild = XmContainerGetItemChildren(ih->handle, itemList[hasItem], &itemChildList);
    if(numChild)
    {
      ih->data->id_control--;

      itemChild = motTreeFindNewBrother(ih, itemChildList, numChild, 0);

      /* StateID founded! */
      if(ih->data->id_control < 0)
      {
        if (itemChildList) XtFree((char*)itemChildList);
        return itemChild;
      }
    }

    if (itemChildList) XtFree((char*)itemChildList);
    /* Go to next sibling item */
    hasItem++;
  }

  return itemList[hasItem];
}

static Widget motTreeFindNodeFromID(Ihandle* ih, WidgetList itemList, int numItems, Widget itemNode)
{
  Widget itemChild;
  WidgetList itemChildList;
  int hasItem = 0;
  int numChild;

  while(hasItem != numItems)
  {
    /* ID control to traverse items */
    ih->data->id_control++;

    /* StateID founded! */
    if(itemList[hasItem] == itemNode)
      return itemList[hasItem];

    /* Check whether we have child items */
    itemChildList = NULL;
    numChild = XmContainerGetItemChildren(ih->handle, itemList[hasItem], &itemChildList);
    if(numChild)
    {
      /* pass the list of children of this item */
      itemChild = motTreeFindNodeFromID(ih, itemChildList, numChild, itemNode);

      /* StateID founded! */
      if(itemChild == itemNode)
      {
        if (itemChildList) XtFree((char*)itemChildList);
        return itemChild;
      }
    }

    if (itemChildList) XtFree((char*)itemChildList);
    /* Go to next sibling item */
    hasItem++;
  }

  return itemList[hasItem];
}

static Widget motTreeFindNode(Ihandle* ih, WidgetList itemList, int numItems)
{
  Widget itemChild;
  WidgetList itemChildList;
  int hasItem = 0;
  int numChild;

  while(hasItem != numItems)
  {
    /* ID control to traverse items */
    ih->data->id_control--;

    /* StateID founded! */
    if(ih->data->id_control < 0)
      return itemList[hasItem];

    /* Check whether we have child items */
    itemChildList = NULL;
    numChild = XmContainerGetItemChildren(ih->handle, itemList[hasItem], &itemChildList);
    if(numChild)
    {
      /* pass the list of children of this item */
      itemChild = motTreeFindNode(ih, itemChildList, numChild);

      /* StateID founded! */
      if(ih->data->id_control < 0)
      {
        if (itemChildList) XtFree((char*)itemChildList);
        return itemChild;
      }
    }

    if (itemChildList) XtFree((char*)itemChildList);
    /* Go to next sibling item */
    hasItem++;
  }

  return itemList[hasItem];
}

static Widget motTreeFindNodeFromString(Ihandle* ih, const char* id_string)
{
  if (id_string[0])
  {
    Widget wListOfItems[1]; 
    wListOfItems[0] = (Widget)iupAttribGet(ih, "_IUPMOTTREE_ROOTITEM");
    iupStrToInt(id_string, &ih->data->id_control);
    return motTreeFindNode(ih, wListOfItems, 1);
  }
  else
  {
    /* Get the first selected object */
    WidgetList wListOfItems = NULL;
    XtVaGetValues(ih->handle, XmNselectedObjects, &wListOfItems, NULL);
    return wListOfItems[0];
  }
}

static void motTreeFocusChangeEvent(Widget w, Ihandle *ih, XEvent *evt, Boolean *cont)
{
  unsigned char selpol;
  Widget wItem = XmGetFocusWidget(w);
  Widget wRoot = (Widget)iupAttribGet(ih, "_IUPMOTTREE_ROOTITEM");
  if (wItem == NULL || wItem == wRoot)
  {
    iupmotFocusChangeEvent(w, ih, evt, cont);
    return;
  }

  XtVaGetValues(w, XmNselectionPolicy, &selpol, NULL);
  if (selpol != XmSINGLE_SELECT)
    return;

  if (evt->type == FocusIn)
  {
    XtVaSetValues(w, XmNselectedObjects,  NULL, NULL);
    XtVaSetValues(w, XmNselectedObjectCount, 0, NULL);
    XtVaSetValues(wItem, XmNvisualEmphasis, XmSELECTED, NULL);
  }
}

void iupdrvTreeAddNode(Ihandle* ih, const char* id_string, int kind, const char* title, int add)
{
  Widget wItemPrev = motTreeFindNodeFromString(ih, id_string);
  Widget wNewItem;
  XmString itemTitle;
  motTreeItemData *itemData, *itemDataPrev;
  Pixel bgcolor, fgcolor;
  int kindPrev, num_args = 0;
  Arg args[30];

  if (!wItemPrev)
    return;

  itemData = calloc(1, sizeof(motTreeItemData));
  itemData->image = XmUNSPECIFIED_PIXMAP;
  itemData->image_expanded = XmUNSPECIFIED_PIXMAP;
  itemData->kind = (unsigned char)kind;

  itemTitle = XmStringCreateLocalized((String)title);

  /* Get default colors */
  XtVaGetValues(ih->handle, XmNforeground, &fgcolor, NULL);
  XtVaGetValues(ih->handle, XmNbackground, &bgcolor, NULL);

  /* Get the kind of previous item */
  XtVaGetValues(wItemPrev, XmNuserData, &itemDataPrev, NULL);
  kindPrev = itemDataPrev->kind;

  if (kindPrev == ITREE_BRANCH && add)
  {
    /* wItemPrev is parent of the new item (firstchild of it) */
    iupmotSetArg(args, num_args,   XmNentryParent, wItemPrev);
    iupmotSetArg(args, num_args, XmNpositionIndex, 0);
  }
  else
  {
    /* wItemPrev is sibling of the new item (set its parent to the new item) */
    Widget wItemParent;
    int pos;

    XtVaGetValues(wItemPrev, XmNentryParent, &wItemParent, NULL);
    XtVaGetValues(wItemPrev, XmNpositionIndex, &pos, NULL);

    iupmotSetArg(args, num_args, XmNentryParent, wItemParent);
    iupmotSetArg(args, num_args, XmNpositionIndex, pos+1);
  }

  iupmotSetArg(args, num_args,       XmNuserData, itemData);
  iupmotSetArg(args, num_args,   XmNforeground, fgcolor);
  iupmotSetArg(args, num_args,   XmNbackground, bgcolor);
  iupmotSetArg(args, num_args, XmNmarginHeight, 0);
  iupmotSetArg(args, num_args,  XmNmarginWidth, 0);
  iupmotSetArg(args, num_args,     XmNviewType, XmSMALL_ICON);
  iupmotSetArg(args, num_args, XmNnavigationType, XmTAB_GROUP);
  iupmotSetArg(args, num_args, XmNtraversalOn, True);
  iupmotSetArg(args, num_args, XmNshadowThickness, 0);
  iupmotSetArg(args, num_args, XmNlabelString, itemTitle);

  if (kind == ITREE_BRANCH)
  {
    if (ih->data->add_expanded)
      iupmotSetArg(args, num_args, XmNsmallIconPixmap, ih->data->def_image_expanded);
    else
      iupmotSetArg(args, num_args, XmNsmallIconPixmap, ih->data->def_image_collapsed);
  }
  else
    iupmotSetArg(args, num_args, XmNsmallIconPixmap, ih->data->def_image_leaf);


  wNewItem = XtCreateManagedWidget("icon", xmIconGadgetClass, ih->handle, args, num_args);

  if (kind == ITREE_BRANCH)
  {
    if (ih->data->add_expanded)
      XtVaSetValues(wNewItem, XmNoutlineState, XmEXPANDED, NULL);
    else
      XtVaSetValues(wNewItem, XmNoutlineState, XmCOLLAPSED, NULL);
  }

  /* Root always expanded */
  XtVaSetValues((Widget)iupAttribGet(ih, "_IUPMOTTREE_ROOTITEM"), XmNoutlineState, XmEXPANDED, NULL);

  XtRealizeWidget(wNewItem);
  XmStringFree(itemTitle);
}

static void motTreeAddRootNode(Ihandle* ih)
{
  Widget wRootItem;
  motTreeItemData *itemData;
  Pixel bgcolor, fgcolor;
  int num_args = 0;
  Arg args[30];

  itemData = calloc(1, sizeof(motTreeItemData));
  itemData->image = XmUNSPECIFIED_PIXMAP;
  itemData->image_expanded = XmUNSPECIFIED_PIXMAP;
  itemData->kind = ITREE_BRANCH;

  /* Get default foreground color */
  XtVaGetValues(ih->handle, XmNforeground, &fgcolor, NULL);
  XtVaGetValues(ih->handle, XmNbackground, &bgcolor, NULL);

  iupmotSetArg(args, num_args,  XmNentryParent, NULL);
  iupmotSetArg(args, num_args,       XmNuserData, itemData);
  iupmotSetArg(args, num_args,   XmNforeground, fgcolor);
  iupmotSetArg(args, num_args,   XmNbackground, bgcolor);
  iupmotSetArg(args, num_args, XmNoutlineState, XmEXPANDED);
  iupmotSetArg(args, num_args, XmNmarginHeight, 0);
  iupmotSetArg(args, num_args,  XmNmarginWidth, 0);
  iupmotSetArg(args, num_args,     XmNviewType, XmSMALL_ICON);
  iupmotSetArg(args, num_args, XmNnavigationType, XmTAB_GROUP);
  iupmotSetArg(args, num_args, XmNtraversalOn, True);
  iupmotSetArg(args, num_args, XmNshadowThickness, 0);
  iupmotSetArg(args, num_args, XmNsmallIconPixmap, ih->data->def_image_expanded);

  wRootItem = XtCreateManagedWidget("icon", xmIconGadgetClass, ih->handle, args, num_args);
  
  /* Select the new item */
  XtVaSetValues(wRootItem, XmNvisualEmphasis, XmSELECTED, NULL);

  XtRealizeWidget(wRootItem);

  /* Save the root node for later use */
  iupAttribSetStr(ih, "_IUPMOTTREE_ROOTITEM", (char*)wRootItem);

  /* Starting node */
  iupAttribSetStr(ih, "_IUPTREE_STARTINGITEM", (char*)wRootItem);
}

/*****************************************************************************/

static int motTreeSetImageExpandedAttrib(Ihandle* ih, const char* name_id, const char* value)
{
  motTreeItemData *itemData;
  unsigned char itemState;
  Widget wItem = motTreeFindNodeFromString(ih, name_id);
  if (!wItem)
    return 0;

  XtVaGetValues(wItem, XmNuserData, &itemData, NULL);
  XtVaGetValues(wItem, XmNoutlineState, &itemState, NULL);
  itemData->image_expanded = (Pixmap)iupImageGetImage(value, ih, 0, "IMAGE");
  if (!itemData->image_expanded) itemData->image_expanded = XmUNSPECIFIED_PIXMAP;

  if (itemData->kind == ITREE_BRANCH && itemState == XmEXPANDED)
  {
    if (itemData->image_expanded == XmUNSPECIFIED_PIXMAP)
      XtVaSetValues(wItem, XmNsmallIconPixmap, (Pixmap)ih->data->def_image_expanded, NULL);
    else
      XtVaSetValues(wItem, XmNsmallIconPixmap, itemData->image_expanded, NULL);
  }

  return 1;
}

static int motTreeSetImageAttrib(Ihandle* ih, const char* name_id, const char* value)
{
  motTreeItemData *itemData;
  Widget wItem = motTreeFindNodeFromString(ih, name_id);
  if (!wItem)  
    return 0;

  XtVaGetValues(wItem, XmNuserData, &itemData, NULL);
  itemData->image = (Pixmap)iupImageGetImage(value, ih, 0, "IMAGE");
  if (!itemData->image) itemData->image = XmUNSPECIFIED_PIXMAP;

  if (itemData->kind == ITREE_BRANCH)
  {
    unsigned char itemState;
    XtVaGetValues(wItem, XmNoutlineState, &itemState, NULL);
    if (itemState == XmCOLLAPSED)
    {
      if (itemData->image == XmUNSPECIFIED_PIXMAP)
        XtVaSetValues(wItem, XmNsmallIconPixmap, (Pixmap)ih->data->def_image_collapsed, NULL);
      else
        XtVaSetValues(wItem, XmNsmallIconPixmap, itemData->image, NULL);
    }
  }
  else
  {
    if (itemData->image == XmUNSPECIFIED_PIXMAP)
      XtVaSetValues(wItem, XmNsmallIconPixmap, (Pixmap)ih->data->def_image_leaf, NULL);
    else
      XtVaSetValues(wItem, XmNsmallIconPixmap, itemData->image, NULL);
  }

  return 1;
}

static int motTreeSetImageBranchExpandedAttrib(Ihandle* ih, const char* value)
{
  Widget wRoot = (Widget)iupAttribGet(ih, "_IUPMOTTREE_ROOTITEM");
  ih->data->def_image_expanded = iupImageGetImage(value, ih, 0, "IMAGEBRANCHEXPANDED");
    
  /* Update all images, starting at root node */
  motTreeUpdateImages(ih, &wRoot, 1, ITREE_UPDATEIMAGE_EXPANDED);

  return 1;
}

static int motTreeSetImageBranchCollapsedAttrib(Ihandle* ih, const char* value)
{
  Widget wRoot = (Widget)iupAttribGet(ih, "_IUPMOTTREE_ROOTITEM");
  ih->data->def_image_collapsed = iupImageGetImage(value, ih, 0, "IMAGEBRANCHCOLLAPSED");
   
  /* Update all images, starting at root node */
  motTreeUpdateImages(ih, &wRoot, 1, ITREE_UPDATEIMAGE_COLLAPSED);

  return 1;
}

static int motTreeSetImageLeafAttrib(Ihandle* ih, const char* value)
{
  Widget wRoot = (Widget)iupAttribGet(ih, "_IUPMOTTREE_ROOTITEM");
  ih->data->def_image_leaf = iupImageGetImage(value, ih, 0, "IMAGELEAF");
    
  /* Update all images, starting at root node */
  motTreeUpdateImages(ih, &wRoot, 1, ITREE_UPDATEIMAGE_LEAF);

  return 1;
}

static char* motTreeGetStateAttrib(Ihandle* ih, const char* name_id)
{
  int hasChildren;
  unsigned char itemState;
  Widget wItem = motTreeFindNodeFromString(ih, name_id);
  if (!wItem)  
    return 0;

  XtVaGetValues(wItem, XmNnumChildren, &hasChildren, NULL);
  XtVaGetValues(wItem, XmNoutlineState, &itemState, NULL);
  
  if(hasChildren)
  {
    if(itemState == XmEXPANDED)
      return "EXPANDED";
    else
      return "COLLAPSED";
  }

  return NULL;
}

static int motTreeSetStateAttrib(Ihandle* ih, const char* name_id, const char* value)
{
  Widget wItem = motTreeFindNodeFromString(ih, name_id);
  if (!wItem)  
    return 0;

  if(iupStrEqualNoCase(value, "COLLAPSED"))
    XtVaSetValues(wItem, XmNoutlineState, XmCOLLAPSED, NULL);
  else if(iupStrEqualNoCase(value, "EXPANDED"))
    XtVaSetValues(wItem, XmNoutlineState, XmEXPANDED, NULL);

  return 0;
}

static char* motTreeGetColorAttrib(Ihandle* ih, const char* name_id)
{
  unsigned char r, g, b;
  Pixel color;
  char* str;
  Widget wItem = motTreeFindNodeFromString(ih, name_id);  
  if (!wItem)  
    return NULL;

  XtVaGetValues(wItem, XmNforeground, &color, NULL); 
  iupmotColorGetRGB(color, &r, &g, &b);

  str = iupStrGetMemory(20);
  sprintf(str, "%d %d %d", (int)r, (int)g, (int)b);
  return str;
}

static int motTreeSetColorAttrib(Ihandle* ih, const char* name_id, const char* value)
{
  Pixel color;
  Widget wItem = motTreeFindNodeFromString(ih, name_id);  
  if (!wItem)  
    return 0;

  color = iupmotColorGetPixelStr(value);
  if (color != (Pixel)-1)
    XtVaSetValues(wItem, XmNforeground, color, NULL);
  return 0; 
}

static char* motTreeGetDepthAttrib(Ihandle* ih, const char* name_id)
{
  Widget wRoot;
  int dep = 0;
  char* depth;
  Widget wItem = motTreeFindNodeFromString(ih, name_id);  
  if (!wItem)  
    return NULL;

  wRoot = (Widget)iupAttribGet(ih, "_IUPMOTTREE_ROOTITEM");

  while((wRoot != wItem) && (wItem != NULL))
  {
    XtVaGetValues(wItem, XmNentryParent, &wItem, NULL);
    dep++;
  }

  depth = iupStrGetMemory(10);
  sprintf(depth, "%d", dep);
  return depth;
}

static int motTreeSetMoveNodeAttrib(Ihandle* ih, const char* name_id, const char* value)
{
  Widget wItemParent;
  Widget wRoot;
  WidgetList wChildList = NULL;
  int numChild;
  int curDepth, newDepth;
  Widget wItem = motTreeFindNodeFromString(ih, name_id);  
  if (!wItem)  
    return 0;

  iupStrToInt(motTreeGetDepthAttrib(ih, name_id), &curDepth);
  iupStrToInt(value, &newDepth);

  /* collapse the node to copy images during the set of depth */
  XtVaSetValues(wItem, XmNoutlineState, XmCOLLAPSED, NULL);

  /* just the root node has depth = 0 */
  if(newDepth <= 0)
    return 0;

  wRoot = (Widget)iupAttribGet(ih, "_IUPMOTTREE_ROOTITEM");

  if(curDepth < newDepth)  /* Top -> Down */
  {
    Widget wItemBrother;
    int pos = 0;

    ih->data->id_control = newDepth - curDepth - 1;  /* subtract 1 (one) to reach the level of its new parent */

    /* Get the current pos of wItem, its current parent and the list of its brothers */
    XtVaGetValues(wItem, XmNpositionIndex, &pos, NULL);
    XtVaGetValues(wItem, XmNentryParent, &wItemParent, NULL);
    numChild = XmContainerGetItemChildren(ih->handle, wItemParent, &wChildList);

    /* The node will be child of your brother - this avoids circular reference */
    wItemBrother = motTreeFindNewBrother(ih, wChildList, numChild, pos+1);
    if (wChildList) XtFree((char*)wChildList);

    /* Setting the new parent */
    XtVaGetValues(wItemBrother, XmNentryParent, &wItemParent, NULL);
  }
  else if (curDepth > newDepth)  /* Bottom -> Up */
  {
    /* When the new depth is less than the current depth, 
    simply define a new parent to the node */
    ih->data->id_control = curDepth - newDepth + 1;  /* add 1 (one) to reach the level of its new parent */

    /* Starting the search by the parent of the current node */
    wItemParent = wItem;
    while(ih->data->id_control != 0)
    {
      /* Setting the new parent */
      XtVaGetValues(wItemParent, XmNentryParent, &wItemParent, NULL);
      ih->data->id_control--;
    }
  }
  else /* same depth, nothing to do */
    return 0;

  /* without parent, nothing to do */
  if(wItemParent == NULL)
    return 0;

  /* Copying the node and its children to the new position */
  motTreeCopyBranch(ih, wItem, wItemParent);

  /* Deleting the node (and its children) inserted into the old position */
  wChildList = NULL;
  numChild = XmContainerGetItemChildren(ih->handle, wItem, &wChildList);

  motTreeRemoveChildren(ih, wChildList, numChild);
  motTreeDestroyItemData(wItem);
  XtDestroyWidget(wItem);
  if (wChildList) XtFree((char*)wChildList);

  return 0;
}

static char* motTreeGetParentAttrib(Ihandle* ih, const char* name_id)
{
  Widget wItemParent;
  Widget wRoot;
  char* id;
  Widget wItem = motTreeFindNodeFromString(ih, name_id);
  if (!wItem)  
    return NULL;

  /* get the parent item */
  XtVaGetValues(wItem, XmNentryParent, &wItemParent, NULL);
  if (!wItemParent)
    return NULL;

  wRoot = (Widget)iupAttribGet(ih, "_IUPMOTTREE_ROOTITEM");

  id = iupStrGetMemory(10);
  ih->data->id_control = -1;
  motTreeFindNodeFromID(ih, &wRoot, 1, wItemParent);

  sprintf(id, "%d", ih->data->id_control);
  return id;
}

static char* motTreeGetChildCountAttrib(Ihandle* ih, const char* name_id)
{
  char* str;
  WidgetList wList = NULL;
  Widget wItem = motTreeFindNodeFromString(ih, name_id);
  if (!wItem)  
    return NULL;

  str = iupStrGetMemory(10);
  sprintf(str, "%d", XmContainerGetItemChildren(ih->handle, wItem, &wList));
  if (wList) XtFree((char*)wList);
  return str;
}

static int motTreeCount(Ihandle* ih, Widget wItem)
{
  WidgetList wList = NULL;
  int i, count = 0;
  int childCount = XmContainerGetItemChildren(ih->handle, wItem, &wList);
  count++;
  for (i=0; i<childCount; i++)
    count += motTreeCount(ih, wList[i]);
  if (wList) XtFree((char*)wList);
  return count;
}

static char* motTreeGetCountAttrib(Ihandle* ih)
{
  char* str = iupStrGetMemory(10);
  Widget wRoot = (Widget)iupAttribGet(ih, "_IUPMOTTREE_ROOTITEM");
  sprintf(str, "%d", motTreeCount(ih, wRoot));
  return str;
}

static char* motTreeGetKindAttrib(Ihandle* ih, const char* name_id)
{
  motTreeItemData *itemData;
  Widget wItem = motTreeFindNodeFromString(ih, name_id);
  if (!wItem)  
    return NULL;

  XtVaGetValues(wItem, XmNuserData, &itemData, NULL);

  if(itemData->kind == ITREE_BRANCH)
    return "BRANCH";
  else
    return "LEAF";
}

static char* motTreeGetValueAttrib(Ihandle* ih)
{
  Widget wRoot;
  WidgetList wListOfItems = NULL;
  char* id = iupStrGetMemory(10);
  int countItems;
  
  /* get the selected objects */
  XtVaGetValues(ih->handle, XmNselectedObjects, &wListOfItems,
                        XmNselectedObjectCount,   &countItems, NULL);
  if(!countItems)
    return 0;

  wRoot = (Widget)iupAttribGet(ih, "_IUPMOTTREE_ROOTITEM");

  ih->data->id_control = -1;
  motTreeFindNodeFromID(ih, &wRoot, 1, wListOfItems[0]);  /* use only the first element (base) of the selected objects array */
  sprintf(id, "%d", ih->data->id_control);

  return id;
}

/* Changes the selected node, starting from current branch
- value: "ROOT", "LAST", "NEXT", "PREVIOUS", "INVERT",
"BLOCK", "CLEARALL", "MARKALL", "INVERTALL"
or id of the node that will be the current  */
static int motTreeSetValueAttrib(Ihandle* ih, const char* value)
{
  Widget wRoot, wSelectedItem = NULL;
  WidgetList wListOfItems = NULL;
  int countItems;

  /* get the selected objects */
  XtVaGetValues(ih->handle, XmNselectedObjects, &wListOfItems,
                        XmNselectedObjectCount,   &countItems, NULL);

  wRoot = (Widget)iupAttribGet(ih, "_IUPMOTTREE_ROOTITEM");

  if(countItems)
    wSelectedItem = wListOfItems[0];

  if(iupStrEqualNoCase(value, "ROOT"))
  {
    XtVaSetValues(ih->handle, XmNselectedObjects,  NULL, NULL);
    XtVaSetValues(ih->handle, XmNselectedObjectCount, 0, NULL);

    XtVaSetValues(wRoot, XmNvisualEmphasis, XmSELECTED, NULL);
    wSelectedItem = wRoot;
  }
  else if(iupStrEqualNoCase(value, "LAST"))
  {
    Widget wLast = motTreeGetLastPrevNode(ih, wRoot);

    XtVaSetValues(ih->handle, XmNselectedObjects,  NULL, NULL);
    XtVaSetValues(ih->handle, XmNselectedObjectCount, 0, NULL);

    XtVaSetValues(wLast, XmNvisualEmphasis, XmSELECTED, NULL);
    wSelectedItem = wLast;
  }
  else if(iupStrEqualNoCase(value, "PGUP"))
  {
    Widget wPrev;
    ih->data->id_control = -1;

    wPrev = motTreeFindCurrentVisibleNode(ih, &wRoot, 1, wSelectedItem);
    ih->data->id_control -= 10;  /* Up 10 lines */

    if(ih->data->id_control < 0)
      ih->data->id_control = 0;  /* Begin of tree = Root id */

    wPrev = motTreeFindNewVisibleNode(ih, &wRoot, 1);

    XtVaSetValues(ih->handle, XmNselectedObjects,  NULL, NULL);
    XtVaSetValues(ih->handle, XmNselectedObjectCount, 0, NULL);

    XtVaSetValues(wPrev, XmNvisualEmphasis, XmSELECTED, NULL);
    wSelectedItem = wPrev;
  }
  else if(iupStrEqualNoCase(value, "PGDN"))
  {
    Widget wNext;
    ih->data->id_control = -1;

    wNext = motTreeFindCurrentVisibleNode(ih, &wRoot, 1, wSelectedItem);
    ih->data->id_control += 10;  /* Down 10 lines */

    wNext = motTreeFindNewVisibleNode(ih, &wRoot, 1);

    if(ih->data->id_control >= 0)
      wNext = motTreeGetLastPrevNode(ih, wRoot);   /* End of tree = last node */
 
    XtVaSetValues(ih->handle, XmNselectedObjects,  NULL, NULL);
    XtVaSetValues(ih->handle, XmNselectedObjectCount, 0, NULL);

    XtVaSetValues(wNext, XmNvisualEmphasis, XmSELECTED, NULL);
    wSelectedItem = wNext;
  }
  else if(iupStrEqualNoCase(value, "NEXT"))
  {
    Widget wNext;
    WidgetList wChildrenTree = NULL;
    int numChildren;
    unsigned char itemState;

    XtVaGetValues(wSelectedItem, XmNoutlineState, &itemState, NULL);

    numChildren = XmContainerGetItemChildren(ih->handle, wSelectedItem, &wChildrenTree);

    if(itemState == XmEXPANDED && numChildren)
      wNext = wChildrenTree[0];  /* firstchild of item selected */
    else
      wNext = motTreeGetNextNode(ih, wSelectedItem);

    XtVaSetValues(ih->handle, XmNselectedObjects, wNext, NULL);
    XtVaSetValues(ih->handle, XmNselectedObjectCount, 1, NULL);
    
    XtVaSetValues(wNext, XmNvisualEmphasis, XmSELECTED, NULL);
    wSelectedItem = wNext;
    if (wChildrenTree) XtFree((char*)wChildrenTree);
  }
  else if(iupStrEqualNoCase(value, "PREVIOUS"))
  {
    Widget wPrev, wItemParent;
    WidgetList wBrothersTree = NULL;
    int numBrothers, i=0;

    if(wSelectedItem != (Widget)iupAttribGet(ih, "_IUPMOTTREE_ROOTITEM"))
    {
      XtVaGetValues(wSelectedItem, XmNentryParent, &wItemParent, NULL);
      numBrothers = XmContainerGetItemChildren(ih->handle, wItemParent, &wBrothersTree);

      if(numBrothers && wBrothersTree[0] == wSelectedItem)
      {
        /* First child is selected. So, its parent will be the new selected */
        wPrev = wItemParent;
      }
      else
      {
        while(wBrothersTree[i] != wSelectedItem)
          i++;

        wPrev = motTreeGetLastPrevNode(ih, wBrothersTree[i-1]);
      }

      if (wBrothersTree) XtFree((char*)wBrothersTree);
    }
    else
    {
      wPrev = (Widget)iupAttribGet(ih, "_IUPMOTTREE_ROOTITEM");
    }

    XtVaSetValues(ih->handle, XmNselectedObjects, wPrev, NULL);
    XtVaSetValues(ih->handle, XmNselectedObjectCount, 1, NULL);

    XtVaSetValues(wPrev, XmNvisualEmphasis, XmSELECTED, NULL);
    wSelectedItem = wPrev;
  }
  else if(iupStrEqualNoCase(value, "CLEARALL"))
  {
    motTreeContainerDeselectAll(ih);
  }
  else if(iupStrEqualNoCase(value, "MARKALL"))
  {
    motTreeContainerSelectAll(ih);
  }
  else if(iupStrEqualNoCase(value, "INVERTALL"))
  {
    /* INVERTALL *MUST* appear before INVERT, or else INVERTALL will never be called. */
    motTreeInvertAllNodeMarking(ih, &wRoot, 1);
  }
  else if(iupStrEqualPartial(value, "INVERT"))
  {
    /* iupStrEqualPartial allows the use of "INVERTid" form */
    unsigned char isSelected;
    Widget wNewSelItem = motTreeFindNodeFromString(ih, &value[strlen("INVERT")]);
    if (!wNewSelItem)  
      return 0;

    XtVaGetValues(wNewSelItem, XmNvisualEmphasis, &isSelected, NULL);

    if(isSelected == XmSELECTED)
      XtVaSetValues(wNewSelItem, XmNvisualEmphasis, XmNOT_SELECTED, NULL);
    else
      XtVaSetValues(wNewSelItem, XmNvisualEmphasis, XmSELECTED, NULL);
  }
  else
  {
    Widget wNewSelItem = motTreeFindNodeFromString(ih, value);
    if (!wNewSelItem)  
      return 0;

    XtVaSetValues(ih->handle, XmNselectedObjects,  NULL, NULL);
    XtVaSetValues(ih->handle, XmNselectedObjectCount, 0, NULL);

    XtVaSetValues(wNewSelItem, XmNvisualEmphasis, XmSELECTED, NULL);
    wSelectedItem = wNewSelItem;
  }

  /* traverse to the current selected item */
  XmProcessTraversal(wSelectedItem, XmTRAVERSE_CURRENT);

  iupAttribSetInt(ih, "_IUPTREE_OLDVALUE", IupGetInt(ih, "VALUE"));

  return 0;
} 

static char* motTreeGetStartingAttrib(Ihandle* ih)
{
  Widget wItem = (Widget)iupAttribGet(ih, "_IUPTREE_STARTINGITEM");
  Widget wRoot;
  char* id = iupStrGetMemory(10);

  wRoot = (Widget)iupAttribGet(ih, "_IUPMOTTREE_ROOTITEM");

  ih->data->id_control = -1;
  motTreeFindNodeFromID(ih, &wRoot, 1, wItem);

  sprintf(id, "%d", ih->data->id_control);

  return id;
}

static int motTreeSetStartingAttrib(Ihandle* ih, const char* name_id)
{
  Widget wItem = motTreeFindNodeFromString(ih, name_id);
  if (!wItem)  
    return 0;

  /* unselect the old value */
  XtVaSetValues((Widget)iupAttribGet(ih, "_IUPTREE_STARTINGITEM"), XmNvisualEmphasis, XmNOT_SELECTED, NULL);

  /* set and select the new value */
  iupAttribSetStr(ih, "_IUPTREE_STARTINGITEM", (char*)wItem);
  XtVaSetValues(wItem, XmNvisualEmphasis, XmSELECTED, NULL);
  XmProcessTraversal(wItem, XmTRAVERSE_CURRENT);

  return 0;
}

static char* motTreeGetMarkedAttrib(Ihandle* ih, const char* name_id)
{
  unsigned char isSelected;
  Widget wItem = motTreeFindNodeFromString(ih, name_id);
  if (!wItem)  
    return NULL;

  XtVaGetValues(wItem, XmNvisualEmphasis, &isSelected, NULL);

  if(isSelected == XmSELECTED)
    return "YES";
  else
    return "NO";
}

static int motTreeSetMarkedAttrib(Ihandle* ih, const char* name_id, const char* value)
{
  Widget wItem = motTreeFindNodeFromString(ih, name_id);
  if (!wItem)  
    return 0;

  if (iupStrBoolean(value))
    XtVaSetValues(wItem, XmNvisualEmphasis, XmSELECTED, NULL);
  else
    XtVaSetValues(wItem, XmNvisualEmphasis, XmNOT_SELECTED, NULL);

  return 0;
}

static char* motTreeGetTitleAttrib(Ihandle* ih, const char* name_id)
{
  char *title;
  XmString itemTitle;
  Widget wItem = motTreeFindNodeFromString(ih, name_id);
  if (!wItem)  
    return NULL;

  XtVaGetValues(wItem, XmNlabelString, &itemTitle, NULL);

  title = iupmotConvertString(itemTitle);
  XmStringFree(itemTitle);
  return title;
}

static int motTreeSetTitleAttrib(Ihandle* ih, const char* name_id, const char* value)
{
  XmString itemTitle;
  Widget wItem = motTreeFindNodeFromString(ih, name_id);
  if (!wItem)  
    return 0;

  itemTitle = XmStringCreateLocalized((String)value);
  XtVaSetValues(wItem, XmNlabelString, itemTitle, NULL);
  XmStringFree(itemTitle);

  return 0;
}

static int motTreeSetTitleFontAttrib(Ihandle* ih, const char* name_id, const char* value)
{
  XmFontList fontlist = NULL;
  Widget wItem = motTreeFindNodeFromString(ih, name_id);
  if (!wItem)  
    return 0;

  if (value)
  {
    char attr[20];
    sprintf(attr, "TITLEFOUNDRY%s", name_id);
    fontlist = iupmotGetFontList(iupAttribGet(ih, attr), value);
  }
  XtVaSetValues(wItem, XmNrenderTable, fontlist, NULL);

  return 0;
}

static char* motTreeGetTitleFontAttrib(Ihandle* ih, const char* name_id)
{
  XmFontList fontlist;
  Widget wItem = motTreeFindNodeFromString(ih, name_id);
  if (!wItem)  
    return NULL;

  XtVaGetValues(wItem, XmNrenderTable, &fontlist, NULL);
  return iupmotFindFontList(fontlist);
}

static char* motTreeGetUserDataAttrib(Ihandle* ih, const char* name_id)
{
  motTreeItemData *itemData;
  Widget wItem = motTreeFindNodeFromString(ih, name_id);
  if (!wItem)  
    return NULL;

  XtVaGetValues(wItem, XmNuserData, &itemData, NULL);

  return itemData->userdata;
}

static int motTreeSetUserDataAttrib(Ihandle* ih, const char* name_id, const char* value)
{
  motTreeItemData *itemData;
  Widget wItem = motTreeFindNodeFromString(ih, name_id);
  if (!wItem)  
    return 0;

  XtVaGetValues(wItem, XmNuserData, &itemData, NULL);
  itemData->userdata = (void*)value;

  return 0;
}

static int motTreeSetRenameAttrib(Ihandle* ih, const char* name_id, const char* value)
{  
  if(IupGetInt(ih, "SHOWRENAME"))
    ;//iupdrvTreeSetTitleAttrib(ih, name_id, value);
  else
    motTreeRenameNode_CB(ih);

  return 1;
}

static int motTreeSetDelNodeAttrib(Ihandle* ih, const char* name_id, const char* value)
{
  if(iupStrEqualNoCase(value, "SELECTED"))  /* selectec here means the specified one */
  {
    Widget wItem = motTreeFindNodeFromString(ih, name_id);
    WidgetList wItemList = NULL;
    Widget wRoot;

    wRoot = (Widget)iupAttribGet(ih, "_IUPMOTTREE_ROOTITEM");

    /* the root node can't be deleted */
    if(!wItem || wItem == wRoot)  /* root is the unique child */
      return 0;

    /* deleting the specified node (and it's children) */
    {
      int numChild = XmContainerGetItemChildren(ih->handle, wItem, &wItemList);
      motTreeRemoveChildren(ih, wItemList, numChild);
      motTreeDestroyItemData(wItem);
      XtDestroyWidget(wItem);    
      if (wItemList) XtFree((char*)wItemList);
    }
  }
  else if(iupStrEqualNoCase(value, "CHILDREN"))  /* children of the specified one */
  {
    Widget wItem = motTreeFindNodeFromString(ih, name_id);
    WidgetList wItemList = NULL;
    Widget wRoot;
    int numChild;

    wRoot = (Widget)iupAttribGet(ih, "_IUPMOTTREE_ROOTITEM");

    /* the root node can't be deleted */
    if(!wItem || wItem == wRoot)  /* root is the unique child */
      return 0;

    numChild = XmContainerGetItemChildren(ih->handle, wItem, &wItemList);

    /* deleting the selected node's children */
    if(numChild)
      motTreeRemoveChildren(ih, wItemList, numChild);
    if (wItemList) XtFree((char*)wItemList);
  }
  else if(iupStrEqualNoCase(value, "MARKED"))  /* Delete the array of marked nodes */
  {
    WidgetList wListOfItems = NULL, wItemList = NULL;
    Widget wRoot;
    int countItems, numChild, i;

    XtVaGetValues(ih->handle, XmNselectedObjects, &wListOfItems,
                              XmNselectedObjectCount, &countItems, NULL);

    wRoot = (Widget)iupAttribGet(ih, "_IUPMOTTREE_ROOTITEM");

    for(i = 0; i < countItems; i++)
    {
      if(wListOfItems[i] != wRoot)  /* the root node can't be deleted */
      {
        wItemList = NULL;
        numChild = XmContainerGetItemChildren(ih->handle, wListOfItems[i], &wItemList);
        motTreeRemoveChildren(ih, wItemList, numChild);
        motTreeDestroyItemData(wListOfItems[i]);
        XtDestroyWidget(wListOfItems[i]);
        if (wItemList) XtFree((char*)wItemList);
      }
    }
  }

  return 0;
}

static char* motTreeGetIndentationAttrib(Ihandle* ih)
{
  char* str = iupStrGetMemory(255);
  Dimension indent;
  XtVaGetValues(ih->handle, XmNoutlineIndentation, &indent, NULL);
  sprintf(str, "%d", (int)indent);
  return str;
}

static int motTreeSetIndentationAttrib(Ihandle* ih, const char* value)
{
  int indent;
  if (iupStrToInt(value, &indent))
    XtVaSetValues(ih->handle, XmNoutlineIndentation, (Dimension)indent, NULL);
  return 0;
}

static int motTreeSetShowDragDropAttrib(Ihandle* ih, const char* value)
{
  if (iupStrBoolean(value))
    motTreeDragActivate(ih->handle);
  else
    iupmotDisableDragSource(ih->handle);

  return 1;
}

static int motTreeSetExpandAllAttrib(Ihandle* ih, const char* value)
{
  Widget wRoot = (Widget)iupAttribGet(ih, "_IUPMOTTREE_ROOTITEM");

  if (iupStrBoolean(value))
    motTreeExpandCollapseAllNodes(ih, &wRoot, 1, XmEXPANDED);
  else
  {
    motTreeExpandCollapseAllNodes(ih, &wRoot, 1, XmCOLLAPSED);

    /* The root node is always expanded */
    XtVaSetValues((Widget)iupAttribGet(ih, "_IUPMOTTREE_ROOTITEM"), XmNoutlineState, XmEXPANDED, NULL);
  }

  return 0;
}

static int motTreeSetBgColorAttrib(Ihandle* ih, const char* value)
{
  Widget sb_win = (Widget)iupAttribGet(ih, "_IUP_EXTRAPARENT");
  Pixel color;

  /* ignore given value for the scrollbars, must use only from parent */
  char* parent_value = iupBaseNativeParentGetBgColor(ih);

  color = iupmotColorGetPixelStr(parent_value);
  if (color != (Pixel)-1)
  {
    Widget sb = NULL;

    iupmotSetBgColor(sb_win, color);

    XtVaGetValues(sb_win, XmNverticalScrollBar, &sb, NULL);
    if (sb) iupmotSetBgColor(sb, color);

    XtVaGetValues(sb_win, XmNhorizontalScrollBar, &sb, NULL);
    if (sb) iupmotSetBgColor(sb, color);
  }

  color = iupmotColorGetPixelStr(value);
  if (color != (Pixel)-1)
  {
    Widget wRoot;
    Widget clipwin = NULL;

    XtVaGetValues(sb_win, XmNclipWindow, &clipwin, NULL);
    if (clipwin) iupmotSetBgColor(clipwin, color);

    wRoot = (Widget)iupAttribGet(ih, "_IUPMOTTREE_ROOTITEM");
    
    /* Update all children, starting at root node */
    motTreeUpdateBgColor(ih, &wRoot, 1, color);
  }

  iupdrvBaseSetBgColorAttrib(ih, value);   /* use given value for contents */

  /* update internal image cache */
  iupTreeUpdateImages(ih);

  return 1;
}

static int motTreeSetFgColorAttrib(Ihandle* ih, const char* value)
{
  Pixel color = iupmotColorGetPixelStr(value);
  if (color != (Pixel)-1)
    XtVaSetValues(ih->handle, XmNforeground, color, NULL);

  return 1;
}


/************************************************************************************************/


static int motTreeSetRenameCaretPos(Ihandle* ih)
{
  Widget cbEdit = (Widget)iupAttribGet(ih, "_IUPTREE_EDITFIELD");
  int pos = 1;

  sscanf(IupGetAttribute(ih, "RENAMECARET"), "%i", &pos);
  if (pos < 1) pos = 1;
  pos--; /* IUP starts at 1 */

  XtVaSetValues(cbEdit, XmNcursorPosition, pos, NULL);

  return 1;
}

static int motTreeSetRenameSelectionPos(Ihandle* ih)
{
  Widget cbEdit = (Widget)iupAttribGet(ih, "_IUPTREE_EDITFIELD");
  int start = 1, end = 1;

  if (iupStrToIntInt(IupGetAttribute(ih, "RENAMESELECTION"), &start, &end, ':') != 2) 
    return 0;

  if(start < 1 || end < 1) 
    return 0;

  start--; /* IUP starts at 1 */
  end--;

  XmTextSetSelection(cbEdit, start, end, CurrentTime);

  return 1;
}

/*****************************************************************************/

static int motTreeBranchClose_CB(Ihandle* ih, int value)
{
  IFni cbBranchClose = (IFni)IupGetCallback(ih, "BRANCHCLOSE_CB");

  if(cbBranchClose)
  {
    cbBranchClose(ih, value);
    return IUP_DEFAULT;
  }

  return IUP_IGNORE;
}

static int motTreeBranchOpen_CB(Ihandle* ih, int value)
{
  IFni cbBranchOpen = (IFni)IupGetCallback(ih, "BRANCHOPEN_CB");

  if(cbBranchOpen)
  {
    cbBranchOpen(ih, value);
    return IUP_DEFAULT;
  }

  return IUP_IGNORE;
}

static int motTreeExecuteLeaf_CB(Ihandle* ih, int value)
{
  IFni cbExecuteLeaf = (IFni)IupGetCallback(ih, "EXECUTELEAF_CB");

  if(cbExecuteLeaf)
  {
    cbExecuteLeaf(ih, value);
    return IUP_DEFAULT;
  }

  return IUP_IGNORE;
}

static int motTreeMultiSelection_CB(Ihandle* ih)
{
  IFnIi cbMulti = (IFnIi)IupGetCallback(ih, "MULTISELECTION_CB");
  WidgetList wListOfItems = NULL;
  Widget wRoot;
  int countItems;

  wRoot = (Widget)iupAttribGet(ih, "_IUPMOTTREE_ROOTITEM");

  XtVaGetValues(ih->handle, XmNselectedObjects, &wListOfItems,
                        XmNselectedObjectCount, &countItems, NULL);

  if(cbMulti && countItems > 1)
  {
    int* id_rowItem = malloc(sizeof(int) * countItems);
    int i = 0;

    for(i = 0; i < countItems; i++)
    {
      ih->data->id_control = -1;
      motTreeFindNodeFromID(ih, &wRoot, 1, wListOfItems[i]);
      id_rowItem[i] = ih->data->id_control;
    }

    cbMulti(ih, id_rowItem, countItems);
    free(id_rowItem);

    return IUP_DEFAULT;
  }

  return IUP_IGNORE;
}

static int motTreeRightClick_CB(Ihandle* ih)
{
  IFni cbRightClick  = (IFni)IupGetCallback(ih, "RIGHTCLICK_CB");

  if(cbRightClick)
  {
    cbRightClick(ih, IupGetInt(ih, "VALUE"));
    return IUP_DEFAULT;
  }    

  return IUP_IGNORE;
}

static int motTreeShowRename_CB(Ihandle* ih)
{
  IFni cbShowRename = (IFni)IupGetCallback(ih, "SHOWRENAME_CB");

  if(cbShowRename)
  {
    cbShowRename(ih, IupGetInt(ih, "VALUE"));
    return IUP_DEFAULT;
  }

  return IUP_IGNORE;
}

static int motTreeRenameNode_CB(Ihandle* ih)
{
  IFnis cbRenameNode = (IFnis)IupGetCallback(ih, "RENAMENODE_CB");

  if(cbRenameNode)
  {
    cbRenameNode(ih, IupGetInt(ih, "VALUE"), IupGetAttribute(ih, "NAME"));  

    return IUP_DEFAULT;
  }

  return IUP_IGNORE;
}

static int motTreeRename_CB(Ihandle* ih)
{
  IFnis cbRename = (IFnis)IupGetCallback(ih, "RENAME_CB");
  Widget wRoot;
  String title;

  /* When the focus out occurs because the user selects another item,
     it is necessary to get the correct ID of the item in the editing mode */
  wRoot = (Widget)iupAttribGet(ih, "_IUPMOTTREE_ROOTITEM");
  ih->data->id_control = -1;
  motTreeFindNodeFromID(ih, &wRoot, 1, (Widget)iupAttribGet(ih, "_IUPTREE_SELECTED"));

  XtVaGetValues((Widget)iupAttribGet(ih, "_IUPTREE_EDITFIELD"), XmNvalue, &title, NULL);

  /* No value, editing continues */
  if (title[0]==0)
    return IUP_IGNORE;

  /* Update the item */
  XtVaSetValues((Widget)iupAttribGet(ih, "_IUPTREE_SELECTED"),  XmNlabelString, XmStringCreateLocalized((String)title), NULL);
  XtDestroyWidget((Widget)iupAttribGet(ih, "_IUPTREE_EDITFIELD"));
  XmProcessTraversal((Widget)iupAttribGet(ih, "_IUPTREE_SELECTED"), XmTRAVERSE_CURRENT);

  iupAttribSetStr(ih, "_IUPTREE_EDITFIELD", NULL);
  iupAttribSetStr(ih, "_IUPTREE_SELECTED",  NULL);

  /* RENAME_CB */
  if(cbRename)
    cbRename(ih, ih->data->id_control, title);  

  return IUP_DEFAULT;    
}

static int motTreeDragDrop_CB(Ihandle* ih)
{
  IFniiii cbDragDrop = (IFniiii)IupGetCallback(ih, "DRAGDROP_CB");
  int drag_str = iupAttribGetInt(ih, "_IUPTREE_DRAGID");
  int drop_str = iupAttribGetInt(ih, "_IUPTREE_DROPID");
  int   isshift_str = iupAttribGetInt(ih, "_IUPGTKTREE_ISSHIFT");
  int iscontrol_str = iupAttribGetInt(ih, "_IUPGTKTREE_ISCONTROL");

  if(cbDragDrop)
  {
    cbDragDrop(ih, drag_str, drop_str, isshift_str, iscontrol_str);
    return IUP_DEFAULT;
  }

  return IUP_IGNORE;
}

static void motTreeEditFocusChangeEvent(Widget w, Ihandle *ih, XEvent *evt, Boolean *cont)
{
  if (evt->type == FocusOut)
    motTreeRename_CB(ih);

  iupmotFocusChangeEvent(w, ih, (XEvent*)evt, cont);
}

static void motTreeEditKeyPressEvent(Widget w, Ihandle *ih, XKeyEvent *evt, Boolean *cont)
{
  KeySym motcode = XKeycodeToKeysym(iupmot_display, evt->keycode, 0);

  if(motcode == XK_Return)
    motTreeRename_CB(ih);

  (void)cont;
  (void)w;
}

static void motTreeCreateEditField(Ihandle* ih)
{
  int countItems, num_args = 0;
  Arg args[30];
  WidgetList wListOfItems = NULL;
  Position xIcon, yIcon;
  Dimension wIcon, hIcon;
  char* child_id = iupDialogGetChildIdStr(ih);
  Widget cbEdit;
  XmString title;

  XtVaGetValues(ih->handle, XmNselectedObjects, &wListOfItems, XmNselectedObjectCount, &countItems, NULL);

  if(!countItems)
    return;

  XtVaGetValues(wListOfItems[0], XmNx, &xIcon, XmNy, &yIcon, XmNwidth, &wIcon, XmNheight, &hIcon, XmNlabelString, &title, NULL);

  iupmotSetArg(args, num_args, XmNx, xIcon+21);      /* x-position */
  iupmotSetArg(args, num_args, XmNy, yIcon);         /* y-position */
  iupmotSetArg(args, num_args, XmNwidth, wIcon-21);  /* default width to avoid 0 */
  iupmotSetArg(args, num_args, XmNheight, hIcon);    /* default height to avoid 0 */
  iupmotSetArg(args, num_args, XmNmarginHeight, 0);  /* default padding */
  iupmotSetArg(args, num_args, XmNmarginWidth, 0);
  iupmotSetArg(args, num_args, XmNvalue, iupmotConvertString(title));
  iupmotSetArg(args, num_args, XmNtraversalOn, True);

  cbEdit = XtCreateManagedWidget(
    child_id,       /* child identifier */
    xmTextWidgetClass,   /* widget class */
    (Widget)iupAttribGet(ih, "_IUP_EXTRAPARENT"), /* widget parent */
    args, num_args);

  /* Disable Drag Source */
  iupmotDisableDragSource(cbEdit);

  XtAddEventHandler(cbEdit, EnterWindowMask, False, (XtEventHandler)iupmotEnterLeaveWindowEvent, (XtPointer)ih);
  XtAddEventHandler(cbEdit, LeaveWindowMask, False, (XtEventHandler)iupmotEnterLeaveWindowEvent, (XtPointer)ih);
  XtAddEventHandler(cbEdit, FocusChangeMask, False, (XtEventHandler)motTreeEditFocusChangeEvent, (XtPointer)ih);
  XtAddEventHandler(cbEdit, KeyPressMask,    False, (XtEventHandler)motTreeEditKeyPressEvent, (XtPointer)ih);

  iupAttribSetStr(ih, "_IUPTREE_SELECTED",  (char*)wListOfItems[0]);
  iupAttribSetStr(ih, "_IUPTREE_EDITFIELD", (char*)cbEdit);

  XmProcessTraversal(cbEdit, XmTRAVERSE_CURRENT);

  XmTextSetSelection(cbEdit, (XmTextPosition)0, (XmTextPosition)XmTextGetLastPosition(cbEdit), CurrentTime);

  if(IupGetAttribute(ih, "RENAMECARET"))
    motTreeSetRenameCaretPos(ih);

  if(IupGetAttribute(ih, "RENAMESELECTION"))
    motTreeSetRenameSelectionPos(ih);
}

static void motTreeSelectionCallback(Widget w, Ihandle* ih, XmContainerSelectCallbackStruct *nptr)
{
  IFnii cbSelec = (IFnii)IupGetCallback(ih, "SELECTION_CB");
  int oldpos = iupAttribGetInt(ih, "_IUPTREE_OLDVALUE");
  int curpos = IupGetInt(ih, "VALUE");
  (void)w;
  (void)nptr;

  if (cbSelec)
  {
    if (oldpos != curpos)
    {
      cbSelec(ih, oldpos, 0);  /* unselected */
      cbSelec(ih, curpos, 1);  /*   selected */

      iupAttribSetInt(ih, "_IUPTREE_OLDVALUE", curpos);
    }
  }
}

static void motTreeDefaultActionCallback(Widget w, Ihandle* ih, XmContainerSelectCallbackStruct *nptr)
{
  unsigned char itemState;
  WidgetList wListOfItems = NULL, wChildren = NULL;
  int countItems, numChild;
  motTreeItemData *itemData;
  Widget wItem;
  (void)w;

  wListOfItems = nptr->selected_items;
  countItems = nptr->selected_item_count;

  if (!countItems || (Widget)iupAttribGet(ih, "_IUPTREE_EDITFIELD"))
    return;

  // TODO: check this for multiple selection
  wItem = wListOfItems[0];

  XtVaGetValues(wItem, XmNoutlineState, &itemState,
                       XmNuserData, &itemData, NULL);

  numChild = XmContainerGetItemChildren(ih->handle, wItem, &wChildren);

  if(itemData->kind == ITREE_BRANCH)
  {
    if(itemState == XmEXPANDED)
      XtVaSetValues(wItem, XmNoutlineState,  XmCOLLAPSED, NULL);
    else if(itemState == XmCOLLAPSED && numChild)
      XtVaSetValues(wItem, XmNoutlineState,  XmEXPANDED, NULL);
    else
      motTreeBranchClose_CB(ih, IupGetInt(ih, "VALUE"));
  }
  else
  {
    /* Call EXECUTELEAF_CB callback */
    motTreeExecuteLeaf_CB(ih, IupGetInt(ih, "VALUE"));
  }
  if (wChildren) XtFree((char*)wChildren);
}

static void motTreeOutlineChangedCallback(Widget w, Ihandle* ih, XmContainerOutlineCallbackStruct *nptr)
{
  motTreeItemData *itemData;
  Widget wRoot = (Widget)iupAttribGet(ih, "_IUPMOTTREE_ROOTITEM");

  /* Get the IconGadget value, when it is not selected */
  ih->data->id_control = -1;
  motTreeFindNodeFromID(ih, &wRoot, 1, nptr->item);

  XtVaGetValues(nptr->item, XmNuserData, &itemData, NULL);

  /* Change images */
  if (nptr->reason == XmCR_EXPANDED)
  {
    XtVaSetValues(nptr->item, XmNsmallIconPixmap, (itemData->image_expanded!=XmUNSPECIFIED_PIXMAP)? itemData->image_expanded: (Pixmap)ih->data->def_image_expanded, NULL);

    motTreeBranchOpen_CB(ih, ih->data->id_control);
  }
  else if (nptr->reason == XmCR_COLLAPSED)
  {
    XtVaSetValues(nptr->item, XmNsmallIconPixmap, (itemData->image!=XmUNSPECIFIED_PIXMAP)? itemData->image: (Pixmap)ih->data->def_image_collapsed, NULL);

    motTreeBranchClose_CB(ih, ih->data->id_control);
  }
  
  (void)w;
  (void)ih;
}

static void motTreeTraverseObscuredCallback(Widget widget, Ihandle* ih, XmTraverseObscuredCallbackStruct *cbs)
{
  (void)ih;
  /* allow to do automatic scroll when navigating in the tree */
  XmScrollVisible(widget, cbs->traversal_destination, 10, 10);
}

static void motTreeConvertCallback(Widget w, Ihandle* ih, XtPointer call_data)
{
  XmConvertCallbackStruct *cptr = (XmConvertCallbackStruct *) call_data;
  Atom TARGETS, EXPORTS, CB_TARGETS, ITEM_TARGET;
  Atom *targets;

  TARGETS = XInternAtom (iupmot_display, "TARGETS", False);
  EXPORTS = XInternAtom (iupmot_display, "_MOTIF_EXPORT_TARGETS", False);
  CB_TARGETS = XInternAtom (iupmot_display, "_MOTIF_CLIPBOARD_TARGETS", False);
  ITEM_TARGET = XInternAtom (iupmot_display, "ITEM_TARGET", False);

  /* If the destination has requested the list of targets, we return this as the convert data */
  if ((cptr->target == TARGETS) || (cptr->target == CB_TARGETS) || (cptr->target == EXPORTS))
  {
      /* A request from the destination for the supported data types we are willing to handle */
      targets = (Atom *) XtMalloc ((unsigned) sizeof (Atom));
      targets[0] = ITEM_TARGET;
      cptr->type = XA_ATOM;
      cptr->value = (XtPointer) targets;
      cptr->length = 1;
      cptr->format = 32;
      cptr->status = XmCONVERT_MERGE;
  }
  else
  {
    /* A request from the destination for a specific data type */
    if (cptr->target == ITEM_TARGET)
    {
      WidgetList wList = NULL;

      XtVaGetValues(w, XmNselectedObjects, &wList, NULL);
      iupAttribSetInt(ih, "_IUPTREE_DRAGID", IupGetInt(ih, "VALUE"));
      iupAttribSetStr(ih, "_IUPTREE_DRAGITEM", (char*)wList[0]);

      cptr->value = (XtPointer) wList[0];
      cptr->length = 1;
      cptr->type = cptr->target;
      cptr->format = 8;
      cptr->status = XmCONVERT_DONE;
    }
    else
    {
      /* Presumably toolkit built-in type - Let the Traits take over */
      cptr->value = (XtPointer) 0;
      cptr->length = 0;
      cptr->format = 0;
      cptr->type = cptr->target;
      cptr->status = XmCONVERT_MERGE;
    }
  }
}

static void motTreeTransferCallback(Widget w, XtPointer client_data, XtPointer call_data)
{
  XmSelectionCallbackStruct *sptr = (XmSelectionCallbackStruct *) call_data;
  Atom EXPORTS, TARGETS, CB_TARGETS, ITEM_TARGET;
  Atom *targets, choice;
  Ihandle* ih = IupGetHandle("tree");
  int  i;
  (void)w;
  (void)client_data;

  choice = (Atom) 0;
  TARGETS = XInternAtom (iupmot_display, "TARGETS", False);
  EXPORTS = XInternAtom (iupmot_display, "_MOTIF_EXPORT_TARGETS", False);
  CB_TARGETS = XInternAtom (iupmot_display, "_MOTIF_CLIPBOARD_TARGETS", False);
  ITEM_TARGET = XInternAtom (iupmot_display, "ITEM_TARGET", False);

  if ((sptr->type == XA_ATOM) && ((sptr->target == TARGETS) ||
      (sptr->target == CB_TARGETS) || (sptr->target == EXPORTS)))
  {
      /* The source has sent us a list of available data formats in which it is prepared to export the data */
      /* We get to choose one (IconGadget Selected) */
      targets = (Atom *) sptr->value;

      for (i = 0; i < (int)sptr->length; i++)
      {
        if (targets[i] == ITEM_TARGET)  /* its our own preferred format */
          choice = targets[i];
      }

      /* Lets assume we are only interested in our own data transfer */
      if (choice == (Atom) 0)
      {
        XmTransferDone (sptr->transfer_id, XmTRANSFER_DONE_FAIL);
        return;
      }

      /* Go back to the source asking for own target */
      XmTransferValue (sptr->transfer_id, choice, motTreeTransferCallback, NULL, XtLastTimestampProcessed (iupmot_display));
  }
  else
  {
    if (sptr->target == ITEM_TARGET)  /* The source has sent us a specific data format (IconGadget) */
    {
      Widget wItemDrag = (Widget)iupAttribGet(ih, "_IUPTREE_DRAGITEM");
      Widget wItemDrop = (Widget)iupAttribGet(ih, "_IUPTREE_DROPITEM");
      Widget wNewItem, wRoot;
      WidgetList wItemList = NULL;
      int numChild;
      motTreeItemData *itemData;

      wRoot = (Widget)iupAttribGet(ih, "_IUPMOTTREE_ROOTITEM");

      /* Get the destination kind */
      XtVaGetValues(wItemDrop, XmNuserData, &itemData, NULL);

      /* Source and destination must be different - Root can't be moved - Destination is a branch*/
      if(wItemDrag != wItemDrop && wItemDrag != wRoot && itemData->kind == ITREE_BRANCH)  /* Transfer is valid */
      {
        /* Copy the dragged item to the new position. After, remove it */
        wNewItem = motTreeCopyBranch(ih, wItemDrag, wItemDrop);

        numChild = XmContainerGetItemChildren(ih->handle, wItemDrag, &wItemList);
        if(numChild)
          motTreeRemoveChildren(ih, wItemList, numChild);
        if (wItemList) XtFree((char*)wItemList);

        motTreeDestroyItemData(wItemDrag);
        XtDestroyWidget(wItemDrag);   

        /* DragDrop Callback */
        motTreeDragDrop_CB(ih);

        /* expand the new parent and set the item dropped as the new item selected */
        XtVaSetValues(wItemDrop, XmNoutlineState, XmEXPANDED, NULL);

        /* Select the dragged item */
        XtVaSetValues(ih->handle, XmNselectedObjects,  NULL, NULL);
        XtVaSetValues(ih->handle, XmNselectedObjectCount, 0, NULL);

        XtVaSetValues(wNewItem, XmNvisualEmphasis, XmSELECTED, NULL);

        XmProcessTraversal(wNewItem, XmTRAVERSE_CURRENT);
      }

      iupAttribSetInt(ih, "_IUPTREE_DRAGID", 0);
      iupAttribSetInt(ih, "_IUPTREE_DROPID", 0);
      iupAttribSetStr(ih, "_IUPTREE_DRAGITEM", NULL);
      iupAttribSetStr(ih, "_IUPTREE_DROPITEM", NULL);

      XmTransferDone(sptr->transfer_id, XmTRANSFER_DONE_SUCCEED);
    }
    else
    {
      /* Transfer Failed */
      XmTransferDone (sptr->transfer_id, XmTRANSFER_DONE_FAIL);
    }
  }
}

static void motTreeDestinationCallback(Widget w, Ihandle* ih, XtPointer call_data)
{
  XmDestinationCallbackStruct *dptr = (XmDestinationCallbackStruct *) call_data;
  Atom TARGETS = XInternAtom (iupmot_display, "TARGETS", False);
  XPoint* point = (XPoint*)dptr->location_data;
  Widget wRoot;
  (void)w;

  /* Get the destination widget */
  wRoot = (Widget)iupAttribGet(ih, "_IUPMOTTREE_ROOTITEM");
  motTreeGetDropWidget(ih, &wRoot, 1, point->x, point->y);

  ih->data->id_control = -1;
  motTreeFindNodeFromID(ih, &wRoot, 1, (Widget)iupAttribGet(ih, "_IUPTREE_DROPITEM"));

  iupAttribSetInt(ih, "_IUPTREE_DROPID", ih->data->id_control);

  /* Start the transfer */
  XmTransferValue (dptr->transfer_id, TARGETS, motTreeTransferCallback, NULL, XtLastTimestampProcessed (iupmot_display));
}

static void motTreeKeyReleaseEvent(Widget w, Ihandle *ih, XKeyEvent *evt, Boolean *cont)
{
  KeySym motcode = XKeycodeToKeysym(iupmot_display, evt->keycode, 0);
  (void)w;
  (void)cont;

  if((motcode == XK_Shift_L || motcode == XK_Shift_R) && ih->data->tree_shift)
  {
    /* Multi Selection Callback */
    motTreeMultiSelection_CB(ih);
    iupAttribSetInt(ih, "_IUPMOTTREE_ISSHIFT", 0);
  }
  else if((motcode == XK_Control_L || motcode == XK_Control_R) && ih->data->tree_ctrl)
  {
    iupAttribSetInt(ih, "_IUPMOTTREE_ISCONTROL", 0);
  }
}

static void motTreeKeyPressEvent(Widget w, Ihandle *ih, XKeyEvent *evt, Boolean *cont)
{
  KeySym motcode = XKeycodeToKeysym(iupmot_display, evt->keycode, 0);

  if(iupAttribGet(ih, "_IUPTREE_EDITFIELD"))
    return;

  if (motcode == XK_Tab || motcode == XK_KP_Tab)
  {
    if (evt->state & ShiftMask)
      IupPreviousField(ih);
    else
      IupNextField(ih);
    *cont = False;
    return;
  }
  else if(motcode == XK_F2)
  {
    if(IupGetInt(ih, "SHOWRENAME"))
    {
      motTreeCreateEditField(ih);
      motTreeShowRename_CB(ih);
    }
    else
    {
      motTreeRenameNode_CB(ih);
    }

    return;
  }
  else if((motcode == XK_Shift_L || motcode == XK_Shift_R) && ih->data->tree_shift)
  {
    iupAttribSetInt(ih, "_IUPMOTTREE_ISSHIFT", 1);
  }
  else if((motcode == XK_Control_L || motcode == XK_Control_R) && ih->data->tree_ctrl)
  {
    iupAttribSetInt(ih, "_IUPMOTTREE_ISCONTROL", 1);
  }

  iupmotKeyPressEvent(w, ih, (XEvent*)evt, cont);
}

static void motTreeButtonPressEvent(Widget w, Ihandle* ih, XButtonEvent* evt, Boolean* cont)
{
  (void)w;
  (void)cont;

  if (evt->type==ButtonPress && evt->button==Button3)
  {   
    /* select the pointed item */
    motTreeRightClick_CB(ih);
  }
  else if (evt->type==ButtonPress)
  {
    unsigned long elapsed;
    static Time last = 0;

    elapsed = evt->time - last;
    last = evt->time;

    if ((int)elapsed <= XtGetMultiClickTime(iupmot_display)) /* Double Click */
    {
      WidgetList wListOfItems = NULL;
      Position xIcon;
      int countItems;

      XtVaGetValues(ih->handle, XmNselectedObjects, &wListOfItems, XmNselectedObjectCount, &countItems, NULL);
      if(!countItems)
        return;

      XtVaGetValues(wListOfItems[0], XmNx, &xIcon, NULL);

      /* Double click over the Icon Gadget label text area */
      if(evt->x > xIcon+21)
      {
        if(IupGetInt(ih, "SHOWRENAME"))
        {
          motTreeCreateEditField(ih);
          motTreeShowRename_CB(ih);
        }
        else
        {
          motTreeRenameNode_CB(ih);
        }
      }
    }
  }
}

static int motTreeConvertXYToPos(Ihandle* ih, int x, int y)
{
  Widget wItem = XmObjectAtPoint(ih->handle, (Position)x, (Position)y);
  if (wItem)
  {
    Widget wRoot = (Widget)iupAttribGet(ih, "_IUPMOTTREE_ROOTITEM");
    ih->data->id_control = -1;
    motTreeFindNodeFromID(ih, &wRoot, 1, wItem);
    return ih->data->id_control;
  }
  return -1;
}

static int motTreeMapMethod(Ihandle* ih)
{
  int num_args = 0;
  Arg args[40];
  Widget parent = iupChildTreeGetNativeParentHandle(ih);
  char* child_id = iupDialogGetChildIdStr(ih);
  Widget sb_win;

  /******************************/
  /* Create the scrolled window */
  /******************************/
  iupmotSetArg(args, num_args, XmNmappedWhenManaged, False);  /* not visible when managed */
  iupmotSetArg(args, num_args, XmNscrollingPolicy, XmAUTOMATIC);
  iupmotSetArg(args, num_args, XmNvisualPolicy, XmVARIABLE); 
  iupmotSetArg(args, num_args, XmNscrollBarDisplayPolicy, XmAS_NEEDED);
  iupmotSetArg(args, num_args, XmNspacing, 0); /* no space between scrollbars and text */
  iupmotSetArg(args, num_args, XmNborderWidth, 0);
  iupmotSetArg(args, num_args, XmNshadowThickness, 2);

  sb_win = XtCreateManagedWidget(
    child_id,  /* child identifier */
    xmScrolledWindowWidgetClass, /* widget class */
    parent,                      /* widget parent */
    args, num_args);

  if (!sb_win)
     return IUP_ERROR;

  XtAddCallback (sb_win, XmNtraverseObscuredCallback, (XtCallbackProc)motTreeTraverseObscuredCallback, (XtPointer)ih);

  parent = sb_win;
  child_id = "container";

  num_args = 0;
 
  iupmotSetArg(args, num_args, XmNx, 0);  /* x-position */
  iupmotSetArg(args, num_args, XmNy, 0);  /* y-position */
  iupmotSetArg(args, num_args, XmNwidth, 10);  /* default width to avoid 0 */
  iupmotSetArg(args, num_args, XmNheight, 10); /* default height to avoid 0 */

  iupmotSetArg(args, num_args, XmNmarginHeight, 0);  /* default padding */
  iupmotSetArg(args, num_args, XmNmarginWidth, 0);

  if (iupStrBoolean(iupAttribGetStr(ih, "CANFOCUS")))
    iupmotSetArg(args, num_args, XmNtraversalOn, True);
  else
    iupmotSetArg(args, num_args, XmNtraversalOn, False);

  iupmotSetArg(args, num_args, XmNnavigationType, XmTAB_GROUP);
  iupmotSetArg(args, num_args, XmNhighlightThickness, 2);
  iupmotSetArg(args, num_args, XmNshadowThickness, 0);

  iupmotSetArg(args, num_args, XmNlayoutType, XmOUTLINE);
  iupmotSetArg(args, num_args, XmNentryViewType, XmSMALL_ICON);
//  iupmotSetArg(args, num_args, XmNselectionPolicy, XmEXTENDED_SELECT);
  iupmotSetArg(args, num_args, XmNselectionPolicy, XmSINGLE_SELECT);
  iupmotSetArg(args, num_args, XmNprimaryOwnership, XmOWN_NEVER);//POSSIBLE_MULTIPLE);
  iupmotSetArg(args, num_args, XmNoutlineIndentation, 20);

  if (iupAttribGetInt(ih, "HIDELINES"))
    iupmotSetArg(args, num_args, XmNoutlineLineStyle,  XmNO_LINE);
  else
    iupmotSetArg(args, num_args, XmNoutlineLineStyle, XmSINGLE);

  if (iupAttribGetInt(ih, "HIDEBUTTONS"))
    iupmotSetArg(args, num_args, XmNoutlineButtonPolicy,  XmOUTLINE_BUTTON_ABSENT);
  else
    iupmotSetArg(args, num_args, XmNoutlineButtonPolicy, XmOUTLINE_BUTTON_PRESENT);

  ih->handle = XtCreateManagedWidget(
    child_id,       /* child identifier */
    xmContainerWidgetClass,   /* widget class */
    parent,         /* widget parent */
    args, num_args);

  if (!ih->handle)
    return IUP_ERROR;

  ih->serial = iupDialogGetChildId(ih); /* must be after using the string */

  iupAttribSetStr(ih, "_IUP_EXTRAPARENT", (char*)parent);

  XtAddEventHandler(ih->handle, EnterWindowMask, False, (XtEventHandler)iupmotEnterLeaveWindowEvent, (XtPointer)ih);
  XtAddEventHandler(ih->handle, LeaveWindowMask, False, (XtEventHandler)iupmotEnterLeaveWindowEvent, (XtPointer)ih);
  XtAddEventHandler(ih->handle, FocusChangeMask, False, (XtEventHandler)motTreeFocusChangeEvent,  (XtPointer)ih);
  XtAddEventHandler(ih->handle, KeyPressMask,    False, (XtEventHandler)motTreeKeyPressEvent,    (XtPointer)ih);
  XtAddEventHandler(ih->handle, KeyReleaseMask,  False, (XtEventHandler)motTreeKeyReleaseEvent,  (XtPointer)ih);
  XtAddEventHandler(ih->handle, ButtonPressMask, False, (XtEventHandler)motTreeButtonPressEvent, (XtPointer)ih);
  XtAddEventHandler(ih->handle, PointerMotionMask, False, (XtEventHandler)iupmotPointerMotionEvent, (XtPointer)ih);

  /* Callbacks */
  XtAddCallback(ih->handle, XmNhelpCallback, (XtCallbackProc)iupmotHelpCallback, (XtPointer)ih);
  XtAddCallback(ih->handle, XmNoutlineChangedCallback, (XtCallbackProc)motTreeOutlineChangedCallback, (XtPointer)ih);
  XtAddCallback(ih->handle, XmNdefaultActionCallback,  (XtCallbackProc)motTreeDefaultActionCallback,  (XtPointer)ih);
  XtAddCallback(ih->handle, XmNselectionCallback,      (XtCallbackProc)motTreeSelectionCallback,      (XtPointer)ih);
  //XtAddCallback(ih->handle, XmNconvertCallback,        (XtCallbackProc)motTreeConvertCallback,        (XtPointer)ih);
  //XtAddCallback(ih->handle, XmNdestinationCallback,    (XtCallbackProc)motTreeDestinationCallback,    (XtPointer)ih);

  XtRealizeWidget(parent);

  /* Force background update before setting the images */
  {
    char* value = iupAttribGet(ih, "BGCOLOR");
    if (value)
    {
      motTreeSetBgColorAttrib(ih, value);
      iupAttribSetStr(ih, "BGCOLOR", NULL);
    }
  }

  /* Initialize the default images */
  ih->data->def_image_leaf = iupImageGetImage("IMGLEAF", ih, 0, "IMAGELEAF");
  ih->data->def_image_collapsed = iupImageGetImage("IMGCOLLAPSED", ih, 0, "IMAGEBRANCHCOLLAPSED");
  ih->data->def_image_expanded = iupImageGetImage("IMGEXPANDED", ih, 0, "IMAGEBRANCHEXPANDED");

  motTreeAddRootNode(ih);

  IupSetCallback(ih, "_IUP_XY2POS_CB", (Icallback)motTreeConvertXYToPos);

  return IUP_NOERROR;
}

void iupdrvTreeInitClass(Iclass* ic)
{
  /* Driver Dependent Class functions */
  ic->Map = motTreeMapMethod;

  /* Visual */
  iupClassRegisterAttribute(ic, "BGCOLOR", NULL, motTreeSetBgColorAttrib, "TXTBGCOLOR", NULL, IUPAF_DEFAULT);
  iupClassRegisterAttribute(ic, "FGCOLOR", NULL, motTreeSetFgColorAttrib, IUPAF_SAMEASSYSTEM, "TXTFGCOLOR", IUPAF_DEFAULT);

  /* IupTree Attributes - GENERAL */
  iupClassRegisterAttribute(ic, "EXPANDALL",  NULL, motTreeSetExpandAllAttrib,  NULL, "NO", IUPAF_WRITEONLY||IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "SHOWDRAGDROP", NULL, motTreeSetShowDragDropAttrib, NULL, "NO", IUPAF_DEFAULT);
  iupClassRegisterAttribute(ic, "INDENTATION",  motTreeGetIndentationAttrib, motTreeSetIndentationAttrib, NULL, NULL, IUPAF_DEFAULT);
  iupClassRegisterAttribute(ic, "COUNT", motTreeGetCountAttrib, NULL, NULL, NULL, IUPAF_NO_DEFAULTVALUE|IUPAF_READONLY|IUPAF_NO_INHERIT);

  /* IupTree Attributes - IMAGES */
  iupClassRegisterAttributeId(ic, "IMAGE", NULL, motTreeSetImageAttrib, IUPAF_WRITEONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "IMAGEBRANCHEXPANDED", NULL, motTreeSetImageExpandedAttrib, IUPAF_WRITEONLY|IUPAF_NO_INHERIT);

  iupClassRegisterAttribute(ic, "IMAGELEAF",            NULL, motTreeSetImageLeafAttrib, IUPAF_SAMEASSYSTEM, "IMGLEAF", IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "IMAGEBRANCHCOLLAPSED", NULL, motTreeSetImageBranchCollapsedAttrib, IUPAF_SAMEASSYSTEM, "IMGCOLLAPSED", IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "IMAGEBRANCHEXPANDED",  NULL, motTreeSetImageBranchExpandedAttrib, IUPAF_SAMEASSYSTEM, "IMGEXPANDED", IUPAF_NO_INHERIT);

  /* IupTree Attributes - NODES */
  iupClassRegisterAttributeId(ic, "STATE",  motTreeGetStateAttrib,  motTreeSetStateAttrib, IUPAF_NO_DEFAULTVALUE|IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "DEPTH",  motTreeGetDepthAttrib,  NULL, IUPAF_READONLY|IUPAF_NO_DEFAULTVALUE|IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "KIND",   motTreeGetKindAttrib,   NULL, IUPAF_READONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "PARENT", motTreeGetParentAttrib, NULL, IUPAF_READONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "COLOR",  motTreeGetColorAttrib,  motTreeSetColorAttrib, IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "NAME",   motTreeGetTitleAttrib,   motTreeSetTitleAttrib, IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "TITLE",   motTreeGetTitleAttrib,   motTreeSetTitleAttrib, IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "USERDATA",   motTreeGetUserDataAttrib,   motTreeSetUserDataAttrib, IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "CHILDCOUNT",   motTreeGetChildCountAttrib,   NULL, IUPAF_READONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "TITLEFONT", motTreeGetTitleFontAttrib, motTreeSetTitleFontAttrib, IUPAF_NO_INHERIT);

  /* IupTree Attributes - MARKS */
  iupClassRegisterAttributeId(ic, "MARKED",   motTreeGetMarkedAttrib,   motTreeSetMarkedAttrib,   IUPAF_NO_INHERIT);

  iupClassRegisterAttribute  (ic, "VALUE",    motTreeGetValueAttrib,    motTreeSetValueAttrib,    NULL, NULL, IUPAF_NO_DEFAULTVALUE|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute  (ic, "STARTING", motTreeGetStartingAttrib, motTreeSetStartingAttrib, NULL, NULL, IUPAF_NO_DEFAULTVALUE|IUPAF_NO_INHERIT);

  /* IupTree Attributes - ACTION */
  iupClassRegisterAttributeId(ic, "DELNODE", NULL, motTreeSetDelNodeAttrib, IUPAF_WRITEONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "RENAME",  NULL, motTreeSetRenameAttrib,  IUPAF_NO_INHERIT);
}

// review motTreeCreateEditField
// optimize IupGetInt(ih, "VALUE")
