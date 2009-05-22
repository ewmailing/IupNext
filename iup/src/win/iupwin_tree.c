/** \file
 * \brief Tree Control
 *
 * See Copyright Notice in iup.h
 */

#undef NOTREEVIEW
#include <windows.h>
#include <commctrl.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <stdarg.h>

#include "iup.h"
#include "iupcbs.h"

#include "iup_object.h"
#include "iup_attrib.h"
#include "iup_str.h"
#include "iup_drv.h"
#include "iup_stdcontrols.h"
#include "iup_tree.h"
#include "iup_image.h"
#include "iup_array.h"

#include "iupwin_drv.h"
#include "iupwin_handle.h"
#include "iupwin_draw.h"
#include "iupwin_info.h"

typedef struct _winTreeItemData
{
  COLORREF color;
  unsigned char kind;
  void* userdata;
  HFONT hFont;
  short image;
  short image_expanded;
} winTreeItemData;


/*****************************************************************************/
/* FINDING ITEMS                                                             */
/*****************************************************************************/
static HTREEITEM winTreeFindNodeFromID(Ihandle* ih, HTREEITEM hItem, HTREEITEM hNode)
{
  TVITEM item;
  winTreeItemData* itemData;

  while(hItem != NULL)
  {
    /* ID control to traverse items */
    ih->data->id_control++;

    /* StateID founded! */
    if(hItem == hNode)
      return hItem;

    /* Get hItem attributes */
    item.hItem = hItem;
    item.mask = TVIF_HANDLE|TVIF_PARAM;
    SendMessage(ih->handle, TVM_GETITEM, 0, (LPARAM)(LPTVITEM)&item);
    itemData = (winTreeItemData*)item.lParam;

    /* Check whether we have child items */
    if (itemData->kind == ITREE_BRANCH)
    {
      /* Recursively traverse child items */
      HTREEITEM hItemChild = (HTREEITEM)SendMessage(ih->handle, TVM_GETNEXTITEM, TVGN_CHILD, (LPARAM)hItem);
      hItemChild = winTreeFindNodeFromID(ih, hItemChild, hNode);

      /* StateID founded! */
      if(hItemChild == hNode)
        return hItemChild;
    }
    /* Go to next sibling item */
    hItem = (HTREEITEM)SendMessage(ih->handle, TVM_GETNEXTITEM, TVGN_NEXT, (LPARAM)hItem);
  }

  return hItem;
}

static HTREEITEM winTreeFindNode(Ihandle* ih, HTREEITEM hItem)
{
  TVITEM item;
  winTreeItemData* itemData;

  while(hItem != NULL)
  {
    /* ID control to traverse items */
    ih->data->id_control--;

    /* StateID founded! */
    if(ih->data->id_control < 0)
      return hItem;

    /* Get hItem attributes */
    item.hItem = hItem;
    item.mask = TVIF_HANDLE|TVIF_PARAM;
    SendMessage(ih->handle, TVM_GETITEM, 0, (LPARAM)(LPTVITEM)&item);
    itemData = (winTreeItemData*)item.lParam;

    /* Check whether we have child items */
    if (itemData->kind == ITREE_BRANCH)
    {
      /* Recursively traverse child items */
      HTREEITEM hItemChild = (HTREEITEM)SendMessage(ih->handle, TVM_GETNEXTITEM, TVGN_CHILD, (LPARAM)hItem);
      hItemChild = winTreeFindNode(ih, hItemChild);

      /* StateID founded! */
      if(ih->data->id_control < 0)
        return hItemChild;
    }

    /* Go to next sibling item */
    hItem = (HTREEITEM)SendMessage(ih->handle, TVM_GETNEXTITEM, TVGN_NEXT, (LPARAM)hItem);
  }

  return hItem;
}

static HTREEITEM winTreeFindNodeFromString(Ihandle* ih, const char* id_string)
{
  if (id_string[0])
  {
    HTREEITEM hRoot = (HTREEITEM)SendMessage(ih->handle, TVM_GETNEXTITEM, TVGN_ROOT, 0);
    iupStrToInt(id_string, &ih->data->id_control);
    return winTreeFindNode(ih, hRoot);
  }
  else
    return (HTREEITEM)SendMessage(ih->handle, TVM_GETNEXTITEM, TVGN_CARET, 0);
}

/* Recursively, find a new brother for the item
   that will have its depth changed. Returns the new brother. */
static HTREEITEM winTreeFindNewBrother(Ihandle* ih, HTREEITEM hBrotherItem)
{
  TVITEM item;
  winTreeItemData* itemData;

  while(hBrotherItem != NULL)
  {
    if(ih->data->id_control < 0)
      return hBrotherItem;

    item.hItem = hBrotherItem;
    item.mask = TVIF_HANDLE|TVIF_PARAM;
    SendMessage(ih->handle, TVM_GETITEM, 0, (LPARAM)(LPTVITEM)&item);
    itemData = (winTreeItemData*)item.lParam;

    if (itemData->kind == ITREE_BRANCH)
    {
      HTREEITEM hItemChild;

      ih->data->id_control--;
      hItemChild = winTreeFindNewBrother(ih, (HTREEITEM)SendMessage(ih->handle, TVM_GETNEXTITEM, TVGN_CHILD, (LPARAM)hBrotherItem));

      if(ih->data->id_control < 0)
        return hItemChild;
    }

    hBrotherItem = (HTREEITEM)SendMessage(ih->handle, TVM_GETNEXTITEM, TVGN_NEXT, (LPARAM)hBrotherItem);
  }

  return hBrotherItem;
}

static HTREEITEM winTreeFindNodePointed(Ihandle* ih)
{
  TVHITTESTINFO tvHitPoint = {0};

  tvHitPoint.pt.x = (short)LOWORD(GetMessagePos());
  tvHitPoint.pt.y = (short)HIWORD(GetMessagePos());

  ScreenToClient(ih->handle, &tvHitPoint.pt);
  
  return (HTREEITEM)SendMessage(ih->handle, TVM_HITTEST, 0, (LPARAM)(LPTVHITTESTINFO)&tvHitPoint);
}

int iupwinGetColor(const char* value, COLORREF *color)
{
  unsigned char r, g, b;
  if (iupStrToRGB(value, &r, &g, &b))
  {
    *color = RGB(r,g,b);
    return 1;
  }
  return 0;
}

/*****************************************************************************/
/* ADDING ITEMS                                                              */
/*****************************************************************************/
void iupdrvTreeAddNode(Ihandle* ih, const char* id_string, int kind, const char* title, int add)
{
  TVITEM item, tviPrevItem;
  TVINSERTSTRUCT tvins;
  HTREEITEM hNewItem, hPrevItem = winTreeFindNodeFromString(ih, id_string);
  int kindPrev;
  winTreeItemData* itemData;

  if (!hPrevItem)
    return;

  itemData = calloc(1, sizeof(winTreeItemData));
  itemData->image = -1;
  itemData->image_expanded = -1;
  itemData->kind = (unsigned char)kind;

  item.mask = TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT; 
  item.pszText = (char*)title;
  item.lParam = (LPARAM)itemData;

  iupwinGetColor(iupAttribGetStr(ih, "FGCOLOR"), &itemData->color);

  if (kind == ITREE_BRANCH)
  {
    item.iSelectedImage = item.iImage = (int)ih->data->def_image_collapsed;

    if (ih->data->add_expanded)
    {
      item.mask |= TVIF_STATE;
      item.state = item.stateMask = TVIS_EXPANDED;
      item.iSelectedImage = item.iImage = (int)ih->data->def_image_expanded;
    }
  }
  else
    item.iSelectedImage = item.iImage = (int)ih->data->def_image_leaf;

  /* Save the heading level in the node's application-defined data area */
  tvins.item = item;

  /* get the KIND attribute of node selected */ 
  tviPrevItem.hItem = hPrevItem;
  tviPrevItem.mask = TVIF_PARAM|TVIF_CHILDREN; 
  SendMessage(ih->handle, TVM_GETITEM, 0, (LPARAM)(LPTVITEM)&tviPrevItem);
  kindPrev = ((winTreeItemData*)tviPrevItem.lParam)->kind;

  /* Define the parent and the position to the new node inside
     the list, using the KIND attribute of node selected */
  if (kindPrev == ITREE_BRANCH && add)
  {
    tvins.hParent = hPrevItem;
    tvins.hInsertAfter = TVI_FIRST;   /* insert the new node after item selected, as first child */
  }
  else
  {
    tvins.hParent = (HTREEITEM)SendMessage(ih->handle, TVM_GETNEXTITEM, TVGN_PARENT, (LPARAM)hPrevItem);
    tvins.hInsertAfter = hPrevItem;   /* insert the new node after item selected */
  }

  /* Add the node to the tree-view control */
  hNewItem = (HTREEITEM)SendMessage(ih->handle, TVM_INSERTITEM, 0, (LPARAM)(LPTVINSERTSTRUCT)&tvins);

  if (kindPrev == ITREE_BRANCH && tviPrevItem.cChildren==0)
  {
    /* this is the first child, redraw to update the '+'/'-' buttons */
    iupdrvDisplayRedraw(ih);
  }
}

static void winTreeAddRootNode(Ihandle* ih)
{
  TVITEM item; 
  TVINSERTSTRUCT tvins;
  HTREEITEM hNewItem;
  winTreeItemData* itemData;

  itemData = calloc(1, sizeof(winTreeItemData));
  itemData->image = -1;
  itemData->image_expanded = -1;
  itemData->kind = ITREE_BRANCH;

  item.mask = TVIF_PARAM | TVIF_STATE | TVIF_IMAGE | TVIF_SELECTEDIMAGE; 
  item.state = item.stateMask = TVIS_EXPANDED;
  item.iSelectedImage = item.iImage = (int)ih->data->def_image_expanded;
  item.lParam = (LPARAM)itemData;

  iupwinGetColor(iupAttribGetStr(ih, "FGCOLOR"), &itemData->color);

  /* Save the heading level in the node's application-defined data area */
  tvins.item = item; 
  tvins.hInsertAfter = TVI_FIRST;
  tvins.hParent = TVI_ROOT;

  /* Add the node to the tree-view control */
  hNewItem = (HTREEITEM)SendMessage(ih->handle, TVM_INSERTITEM, 0, (LPARAM)(LPTVINSERTSTRUCT)&tvins);

  /* Starting node */
  iupAttribSetStr(ih, "_IUPTREE_STARTINGITEM", (char*)hNewItem);
}

/*****************************************************************************/
/* EXPANDING AND STORING ITEMS                                               */
/*****************************************************************************/
static void winTreeExpandedTree(Ihandle* ih, HTREEITEM hItem, int expand)
{
  TVITEM item;
  winTreeItemData* itemData;

  while(hItem != NULL)
  {
    /* Get hItem attributes */
    item.hItem = hItem;
    item.mask = TVIF_HANDLE|TVIF_PARAM;
    SendMessage(ih->handle, TVM_GETITEM, 0, (LPARAM)(LPTVITEM)&item);
    itemData = (winTreeItemData*)item.lParam;

    /* Check whether we have child items */
    if (itemData->kind == ITREE_BRANCH)
    {
      /* Recursively traverse child items */
      HTREEITEM hItemChild = (HTREEITEM)SendMessage(ih->handle, TVM_GETNEXTITEM, TVGN_CHILD, (LPARAM)hItem);

      if (expand)
        SendMessage(ih->handle, TVM_EXPAND, TVE_EXPAND, (LPARAM)hItem);
      else
        SendMessage(ih->handle, TVM_EXPAND, TVE_COLLAPSE, (LPARAM)hItem);

      winTreeExpandedTree(ih, hItemChild, expand);
    }

    /* Go to next sibling item */
    hItem = (HTREEITEM)SendMessage(ih->handle, TVM_GETNEXTITEM, TVGN_NEXT, (LPARAM)hItem);
  }
}

static void winTreeMarkedItemArray(Ihandle* ih, HTREEITEM hItem)
{
  /* Store the selected/marked items */
  Iarray* markedArray;
  HTREEITEM* hItemArrayData;
  int i;

  markedArray = (Iarray*)iupAttribGet(ih, "_IUPWINTREE_MARKEDITEMARRAY");
  if (!markedArray)
  {
    markedArray = iupArrayCreate(1, sizeof(HTREEITEM));
    iupAttribSetStr(ih, "_IUPWINTREE_MARKEDITEMARRAY", (char*)markedArray);
  }

  hItemArrayData = (HTREEITEM*)iupArrayGetData(markedArray);

  for(i = 0; i < iupArrayCount(markedArray); i++)
  {
    if(hItemArrayData[i] == hItem)
      return;
  }

  hItemArrayData = iupArrayInc(markedArray);
  hItemArrayData[i] = hItem;
}

/*****************************************************************************/
/* SELECTING ITEMS                                                           */
/*****************************************************************************/
/* Selects items form hItemFrom to hItemTo.
   It does not select child items if parent is collapsed.
   Removes selection from all other items */
static void winTreeSelectItems(Ihandle* ih, HTREEITEM hItemFrom, HTREEITEM hItemTo)
{
  HTREEITEM hItem = (HTREEITEM)SendMessage(ih->handle, TVM_GETNEXTITEM, TVGN_ROOT, 0);
  BOOL bSelect = TRUE;
  TVITEM item; 

  /* Clear selection up to the first item */
  while(hItem  &&  hItem != hItemFrom  &&  hItem != hItemTo)
  {
    hItem = (HTREEITEM)SendMessage(ih->handle, TVM_GETNEXTITEM, TVGN_NEXTVISIBLE, (LPARAM)hItem);

    item.mask = TVIF_HANDLE|TVIF_STATE;
    item.hItem = hItem;
    item.stateMask = TVIS_SELECTED;
    item.state = 0;
    SendMessage(ih->handle, TVM_SETITEM, 0, (LPARAM)(const LPTVITEM)&item);
  }

  if(!hItem)
    return;  /* Item is not visible */

  SendMessage(ih->handle, TVM_SELECTITEM, TVGN_CARET, (LPARAM)hItemTo);

  /* Rearrange hItemFrom and hItemTo so that first item is at top */
  if(hItem == hItemTo)
  {
    hItemTo = hItemFrom;
    hItemFrom = hItem;
  }

  while(hItem)
  {
    /* Select or remove selection depending on whether item is still within the range */
    if(bSelect)
    {
      item.mask = TVIF_HANDLE|TVIF_STATE;
      item.hItem = hItem;
      item.stateMask = TVIS_SELECTED;
      item.state = TVIS_SELECTED;
      SendMessage(ih->handle, TVM_SETITEM, 0, (LPARAM)(const LPTVITEM)&item);
    }
    else
    {
      item.mask = TVIF_HANDLE|TVIF_STATE;
      item.hItem = hItem;
      item.stateMask = TVIS_SELECTED;
      item.state = 0;
      SendMessage(ih->handle, TVM_SETITEM, 0, (LPARAM)(const LPTVITEM)&item);
    }

    if((SendMessage(ih->handle, TVM_GETITEMSTATE, (WPARAM)hItem, TVIS_SELECTED)) & TVIS_SELECTED)
      winTreeMarkedItemArray(ih, hItem);

    /* Do we need to start removing items from selection */
    if(hItem == hItemTo)
      bSelect = FALSE;

    hItem = (HTREEITEM)SendMessage(ih->handle, TVM_GETNEXTITEM, TVGN_NEXTVISIBLE, (LPARAM)hItem);
  }
}

static void winTreeAllSelection(Ihandle* ih)
{
  HTREEITEM hItemRoot = (HTREEITEM)SendMessage(ih->handle, TVM_GETNEXTITEM, TVGN_ROOT, 0);
  HTREEITEM hItemLast = (HTREEITEM)SendMessage(ih->handle, TVM_GETNEXTITEM, TVGN_LASTVISIBLE, 0);

  winTreeSelectItems(ih, hItemRoot, hItemLast);
}

static void winTreeClearSelection(Ihandle* ih)
{
  HTREEITEM hItem = (HTREEITEM)SendMessage(ih->handle, TVM_GETNEXTITEM, TVGN_ROOT, 0);
  TVITEM item;
  Iarray* markedArray = (Iarray*)iupAttribGet(ih, "_IUPWINTREE_MARKEDITEMARRAY");

  while(hItem != NULL)
  {
    if((SendMessage(ih->handle, TVM_GETITEMSTATE, (WPARAM)hItem, TVIS_SELECTED)) & TVIS_SELECTED)
    {
      item.mask = TVIF_HANDLE|TVIF_STATE;
      item.hItem = hItem;
      item.stateMask = TVIS_SELECTED;
      item.state = 0;
      SendMessage(ih->handle, TVM_SETITEM, 0, (LPARAM)(const LPTVITEM)&item);
    }

    hItem = (HTREEITEM)SendMessage(ih->handle, TVM_GETNEXTITEM, TVGN_NEXTVISIBLE, (LPARAM)hItem);
  }

  if (markedArray)
  {
    iupArrayDestroy(markedArray);
    iupAttribSetStr(ih, "_IUPWINTREE_MARKEDITEMARRAY", NULL);
  }
}

/*****************************************************************************/
/* COPYING ITEMS (Branches and its children)                                 */
/*****************************************************************************/
/* Insert the copied item in a new location. Returns the new item. */
static HTREEITEM winTreeCopyItem(Ihandle* ih, HTREEITEM hItem, HTREEITEM htiNewParent)
{
  TVITEM item; 
  TVINSERTSTRUCT tvins;
  HTREEITEM hNewItem;
  char* title = iupStrGetMemory(255);

  // TODO: o que fazer com o winTreeItemData?

  item.hItem = hItem;
  item.mask = TVIF_HANDLE | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_STATE | TVIF_PARAM;
  item.pszText = title;
  item.cchTextMax = 255;
  SendMessage(ih->handle, TVM_GETITEM, 0, (LPARAM)(LPTVITEM)&item);

  /* Save the heading level in the node's application-defined data area */
  tvins.item = item; 
  tvins.hInsertAfter = TVI_LAST;
  tvins.hParent = htiNewParent;

  /* Add the node to the tree-view control */
  hNewItem = (HTREEITEM)SendMessage(ih->handle, TVM_INSERTITEM, 0, (LPARAM)(LPTVINSERTSTRUCT)&tvins);

  return hNewItem;
}

/* Copies all items in a branch to a new location. Returns the new branch node. */
static HTREEITEM winTreeCopyBranch(Ihandle* ih, HTREEITEM htiBranch, HTREEITEM htiNewParent)
{
  HTREEITEM hNewItem = winTreeCopyItem(ih, htiBranch, htiNewParent);
  HTREEITEM hChild = (HTREEITEM)SendMessage(ih->handle, TVM_GETNEXTITEM, TVGN_CHILD, (LPARAM)htiBranch);

  while(hChild != NULL)
  {
    /* Recursively transfer all the items */
    winTreeCopyBranch(ih, hChild, hNewItem);  

    /* Go to next sibling item */
    hChild = (HTREEITEM)SendMessage(ih->handle, TVM_GETNEXTITEM, TVGN_NEXT, (LPARAM)hChild);
  }
  return hNewItem;
}

/*****************************************************************************/
/* MANIPULATING IMAGES                                                       */
/*****************************************************************************/
static void winTreeUpdateImages(Ihandle* ih, HTREEITEM hItem, int mode)
{
  HTREEITEM hItemChild;
  TVITEM item;
  winTreeItemData* itemData;

  /* called when one of the default images is changed */

  while(hItem != NULL)
  {
    /* Get node attributes */
    item.hItem = hItem;
    item.mask = TVIF_HANDLE | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_STATE;
    SendMessage(ih->handle, TVM_GETITEM, 0, (LPARAM)(LPTVITEM)&item);
    itemData = (winTreeItemData*)item.lParam;

    if (itemData->kind == ITREE_BRANCH)
    {
      if (item.state & TVIS_EXPANDED)
      {
        if (mode == ITREE_UPDATEIMAGE_EXPANDED)
        {
          item.mask = TVIF_HANDLE | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
          item.iSelectedImage = item.iImage = (itemData->image_expanded!=-1)? itemData->image_expanded: (int)ih->data->def_image_expanded;
          SendMessage(ih->handle, TVM_SETITEM, 0, (LPARAM)(LPTVITEM)&item);
        }
      }
      else
      {
        if (mode == ITREE_UPDATEIMAGE_COLLAPSED)
        {
          item.mask = TVIF_HANDLE | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
          item.iSelectedImage = item.iImage = (itemData->image!=-1)? itemData->image: (int)ih->data->def_image_collapsed;
          SendMessage(ih->handle, TVM_SETITEM, 0, (LPARAM)(LPTVITEM)&item);
        }
      }

      /* Recursively traverse child items */
      hItemChild = (HTREEITEM)SendMessage(ih->handle, TVM_GETNEXTITEM, TVGN_CHILD, (LPARAM)hItem);
      winTreeUpdateImages(ih, hItemChild, mode);
    }
    else
    {
      if (mode == ITREE_UPDATEIMAGE_LEAF)
      {
        item.mask = TVIF_HANDLE | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
        item.iSelectedImage = item.iImage = (itemData->image!=-1)? itemData->image: (int)ih->data->def_image_leaf;
        SendMessage(ih->handle, TVM_SETITEM, 0, (LPARAM)(LPTVITEM)&item);
      }
    }

    /* Go to next sibling node */
    hItem = (HTREEITEM)SendMessage(ih->handle, TVM_GETNEXTITEM, TVGN_NEXT, (LPARAM)hItem);
  }
}

static int winTreeGetImageIndex(Ihandle* ih, const char* value, const char* attrib_name)
{
  HIMAGELIST image_list;
  int count, i;
  Iarray* bmpArray;
  HBITMAP *bmpArrayData;
  HBITMAP bmp = iupImageGetImage(value, ih, 0, attrib_name);
  if (!bmp)
    return -1;

  /* the array is used to avoi adding the same bitmap twice */
  bmpArray = (Iarray*)iupAttribGet(ih, "_IUPWIN_BMPARRAY");
  if (!bmpArray)
  {
    /* create the array if does not exist */
    bmpArray = iupArrayCreate(50, sizeof(HBITMAP));
    iupAttribSetStr(ih, "_IUPWIN_BMPARRAY", (char*)bmpArray);
  }

  bmpArrayData = iupArrayGetData(bmpArray);

  image_list = (HIMAGELIST)SendMessage(ih->handle, TVM_GETIMAGELIST, TVSIL_NORMAL, 0);
  if (!image_list)
  {
    int width, height;

    /* must use this info, since image can be a driver image loaded from resources */
    iupdrvImageGetInfo(bmp, &width, &height, NULL);

    /* create the image list if does not exist */
    image_list = ImageList_Create(width, height, ILC_COLOR32, 0, 50);
    SendMessage(ih->handle, TVM_SETIMAGELIST, 0, (LPARAM)image_list);
  }

  /* check if that bitmap is already added to the list,
     but we can not compare with the actual bitmap at the list since it is a copy */
  count = ImageList_GetImageCount(image_list);
  for (i = 0; i < count; i++)
  {
    if (bmpArrayData[i] == bmp)
      return i;
  }

  bmpArrayData = iupArrayInc(bmpArray);
  bmpArrayData[i] = bmp;
  return ImageList_Add(image_list, bmp, NULL);  /* the bmp is duplicated at the list */
}


/*****************************************************************************/
/* CALLBACKS                                                                 */
/*****************************************************************************/

static int winTreeShowRename_CB(Ihandle* ih)
{
  IFni cbShowRename = (IFni)IupGetCallback(ih, "SHOWRENAME_CB");

  if(cbShowRename)
  {
    HTREEITEM hItemSelected = (HTREEITEM)SendMessage(ih->handle, TVM_GETNEXTITEM, TVGN_CARET, 0);
   
    cbShowRename(ih, IupGetInt(ih, "VALUE"));
    SendMessage(ih->handle, TVM_EDITLABEL, 0, (LPARAM)hItemSelected);
    return IUP_DEFAULT;
  }

  return IUP_IGNORE;
}

static int winTreeRenameNode_CB(Ihandle* ih)
{
  IFnis cbRenameNode = (IFnis)IupGetCallback(ih, "RENAMENODE_CB");

  if(cbRenameNode)
  {
    HTREEITEM hItemSelected = (HTREEITEM)SendMessage(ih->handle, TVM_GETNEXTITEM, TVGN_CARET, 0);

    cbRenameNode(ih, IupGetInt(ih, "VALUE"), IupGetAttribute(ih, "NAME"));  
    SendMessage(ih->handle, TVM_EDITLABEL, 0, (LPARAM)hItemSelected);

    return IUP_DEFAULT;
  }

  return IUP_IGNORE;
}

static int winTreeRename_CB(Ihandle* ih, NMTVDISPINFO* pTreeItem)
{
  IFnis cbRename = (IFnis)IupGetCallback(ih, "RENAME_CB");

  if (!pTreeItem->item.pszText || pTreeItem->item.pszText[0]==0)
    return IUP_IGNORE;

  if(cbRename)
    cbRename(ih, IupGetInt(ih, "VALUE"), pTreeItem->item.pszText);

  SendMessage(ih->handle, TVM_SETITEM, 0, (LPARAM)(const LPTVITEM)&pTreeItem->item);

  return IUP_DEFAULT;
}

static int winTreeBranchLeaf_CB(Ihandle* ih)
{
  HTREEITEM selected = (HTREEITEM)SendMessage(ih->handle, TVM_GETNEXTITEM, TVGN_CARET, 0);
  TVITEM item;
  winTreeItemData* itemData;

  /* Set Item State */
  item.mask = TVIF_HANDLE|TVIF_STATE;
  item.hItem = selected;
  item.stateMask = TVIS_SELECTED;
  item.state = TVIS_SELECTED;
  SendMessage(ih->handle, TVM_SETITEM, 0, (LPARAM)(const LPTVITEM)&item);

  /* Get Children: branch or leaf */
  item.mask = TVIF_HANDLE|TVIF_PARAM|TVIF_STATE; 
  item.hItem = selected;
  SendMessage(ih->handle, TVM_GETITEM, 0, (LPARAM)(LPTVITEM)&item);
  itemData = (winTreeItemData*)item.lParam;

  if (itemData->kind == ITREE_BRANCH)
  {
    if (item.state & TVIS_EXPANDED)
    {
      IFni cbBranchClose = (IFni)IupGetCallback(ih, "BRANCHCLOSE_CB");
      cbBranchClose(ih, IupGetInt(ih, "VALUE"));
      return IUP_DEFAULT;
    }
    else
    {
      IFni cbBranchOpen  = (IFni)IupGetCallback(ih, "BRANCHOPEN_CB");
      cbBranchOpen(ih, IupGetInt(ih, "VALUE"));
      return IUP_DEFAULT;
    }
  }
  else
  {
    IFni cbExecuteLeaf = (IFni)IupGetCallback(ih, "EXECUTELEAF_CB");
    if(cbExecuteLeaf)
    {
      cbExecuteLeaf(ih, IupGetInt(ih, "VALUE"));
      return IUP_DEFAULT;
    }
  }

  return IUP_IGNORE;
}

static int winTreeMultiSelection_CB(Ihandle* ih)
{
  IFnIi cbMulti = (IFnIi)IupGetCallback(ih, "MULTISELECTION_CB");
  Iarray* markedArray = (Iarray*)iupAttribGet(ih, "_IUPWINTREE_MARKEDITEMARRAY");

  if(!markedArray)
    return IUP_IGNORE;

  if(cbMulti)
  {
    HTREEITEM hItemRoot = (HTREEITEM)SendMessage(ih->handle, TVM_GETNEXTITEM, TVGN_ROOT, 0);
    HTREEITEM* hItemArrayData;
    int* id_hitem;
    int i;

    id_hitem = (int*)malloc(iupArrayCount(markedArray) * sizeof(int));
    hItemArrayData = (HTREEITEM*)iupArrayGetData(markedArray);

    for(i = 0; i < iupArrayCount(markedArray); i++)
    {
      ih->data->id_control = -1;
      winTreeFindNodeFromID(ih, hItemRoot, hItemArrayData[i]);
      id_hitem[i] = ih->data->id_control;
    }

    cbMulti(ih, id_hitem, iupArrayCount(markedArray));
    free(id_hitem);
    return IUP_DEFAULT;
  }

  return IUP_IGNORE;
}

static int winTreeSelection_CB(Ihandle* ih, int status)
{
  IFnii cbSelec = (IFnii)IupGetCallback(ih, "SELECTION_CB");

  if(cbSelec)
  {
    cbSelec(ih, IupGetInt(ih, "VALUE"), status);
    return IUP_DEFAULT;
  }

  return IUP_IGNORE;
}

static int winTreeDragDrop_CB(Ihandle* ih)
{
  IFniiii cbDragDrop = (IFniiii)IupGetCallback(ih, "DRAGDROP_CB");
  int drag_str = iupAttribGetInt(ih, "_IUPTREE_DRAGID");
  int drop_str = iupAttribGetInt(ih, "_IUPTREE_DROPID");

  if(cbDragDrop)
  {
    int isshift = 0;
    int iscontrol = 0;

    if((GetKeyState(VK_SHIFT) & 0x8000))
      isshift = 1;

    if((GetKeyState(VK_CONTROL) & 0x8000))
      iscontrol = 1;

    cbDragDrop(ih, drag_str, drop_str, isshift, iscontrol);

    return IUP_DEFAULT;
  }

  return IUP_IGNORE;
}

static int winTreeRightClick_CB(Ihandle* ih)
{
  IFni cbRightClick  = (IFni)IupGetCallback(ih, "RIGHTCLICK_CB");

  if(cbRightClick)
  {
    HTREEITEM hItemRoot = (HTREEITEM)SendMessage(ih->handle, TVM_GETNEXTITEM, TVGN_ROOT, 0);
    HTREEITEM hItem = (HTREEITEM)SendMessage(ih->handle, TVM_GETNEXTITEM, TVGN_CARET, 0);

    ih->data->id_control = -1;
    winTreeFindNodeFromID(ih, hItemRoot, hItem);
    cbRightClick(ih, ih->data->id_control);

    return IUP_DEFAULT;
  }    

  return IUP_IGNORE;
}


/*****************************************************************************/
/* GET AND SET ATTRIBUTES                                                    */
/*****************************************************************************/


static int winTreeSetImageBranchExpandedAttrib(Ihandle* ih, const char* value)
{
  ih->data->def_image_expanded = (void*)winTreeGetImageIndex(ih, value, "IMAGEBRANCHEXPANDED");

  /* Update all images, starting at root node */
  winTreeUpdateImages(ih, (HTREEITEM)SendMessage(ih->handle, TVM_GETNEXTITEM, TVGN_ROOT, 0), ITREE_UPDATEIMAGE_EXPANDED);

  return 1;
}

static int winTreeSetImageBranchCollapsedAttrib(Ihandle* ih, const char* value)
{
  ih->data->def_image_collapsed = (void*)winTreeGetImageIndex(ih, value, "IMAGEBRANCHCOLLAPSED");

  /* Update all images, starting at root node */
  winTreeUpdateImages(ih, (HTREEITEM)SendMessage(ih->handle, TVM_GETNEXTITEM, TVGN_ROOT, 0), ITREE_UPDATEIMAGE_COLLAPSED);

  return 1;
}

static int winTreeSetImageLeafAttrib(Ihandle* ih, const char* value)
{
  ih->data->def_image_leaf = (void*)winTreeGetImageIndex(ih, value, "IMAGELEAF");

  /* Update all images, starting at root node */
  winTreeUpdateImages(ih, (HTREEITEM)SendMessage(ih->handle, TVM_GETNEXTITEM, TVGN_ROOT, 0), ITREE_UPDATEIMAGE_LEAF);

  return 1;
}

static int winTreeSetImageExpandedAttrib(Ihandle* ih, const char* name_id, const char* value)
{
  TVITEM item;
  winTreeItemData* itemData;
  HTREEITEM hItem = winTreeFindNodeFromString(ih, name_id);
  if (!hItem)
    return 0;

  item.hItem = hItem;
  item.mask = TVIF_HANDLE | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_STATE;
  SendMessage(ih->handle, TVM_GETITEM, 0, (LPARAM)(LPTVITEM)&item);
  itemData = (winTreeItemData*)item.lParam;
  itemData->image_expanded = (short)winTreeGetImageIndex(ih, value, "IMAGEBRANCHEXPANDED");

  if (itemData->kind == ITREE_BRANCH && item.state & TVIS_EXPANDED)
  {
    if (itemData->image_expanded == -1)
      item.iSelectedImage = item.iImage = (int)ih->data->def_image_expanded;
    else
      item.iSelectedImage = item.iImage = itemData->image_expanded;
    item.mask = TVIF_HANDLE | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
    SendMessage(ih->handle, TVM_SETITEM, 0, (LPARAM)(const LPTVITEM)&item);
  }

  return 1;
}

static int winTreeSetImageAttrib(Ihandle* ih, const char* name_id, const char* value)
{
  TVITEM item;
  winTreeItemData* itemData;
  HTREEITEM hItem = winTreeFindNodeFromString(ih, name_id);
  if (!hItem)
    return 0;

  item.hItem = hItem;
  item.mask = TVIF_HANDLE | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_STATE;
  SendMessage(ih->handle, TVM_GETITEM, 0, (LPARAM)(LPTVITEM)&item);
  itemData = (winTreeItemData*)item.lParam;
  itemData->image = (short)winTreeGetImageIndex(ih, value, "IMAGE");

  if (itemData->kind == ITREE_BRANCH)
  {
    if (!(item.state & TVIS_EXPANDED))
    {
      if (itemData->image == -1)
        item.iSelectedImage = item.iImage = (int)ih->data->def_image_collapsed;
      else
        item.iSelectedImage = item.iImage = itemData->image;

      item.mask = TVIF_HANDLE | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
      SendMessage(ih->handle, TVM_SETITEM, 0, (LPARAM)(const LPTVITEM)&item);
    }
  }
  else
  {
    if (itemData->image == -1)
      item.iSelectedImage = item.iImage = (int)ih->data->def_image_leaf;
    else
      item.iSelectedImage = item.iImage = itemData->image;

    item.mask = TVIF_HANDLE | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
    SendMessage(ih->handle, TVM_SETITEM, 0, (LPARAM)(const LPTVITEM)&item);
  }

  return 1;
}

static int winTreeSetExpandAllAttrib(Ihandle* ih, const char* value)
{
  HTREEITEM hItemRoot = (HTREEITEM)SendMessage(ih->handle, TVM_GETNEXTITEM, TVGN_ROOT, 0);
  HTREEITEM hItem = (HTREEITEM)SendMessage(ih->handle, TVM_GETNEXTITEM, TVGN_CHILD, (LPARAM)hItemRoot); /* skip the root node that is always expanded */
  int expand = iupStrBoolean(value);
  winTreeExpandedTree(ih, hItem, expand);
  return 0;
}

static int winTreeSetBgColorAttrib(Ihandle* ih, const char* value)
{
  unsigned char r, g, b;
  if (iupStrToRGB(value, &r, &g, &b))
  {
    COLORREF cr = RGB(r,g,b);
    SendMessage(ih->handle, TVM_SETBKCOLOR, 0, (LPARAM)cr);

    /* update internal image cache */
    iupTreeUpdateImages(ih);
  }
  return 0;
}

static char* winTreeGetBgColorAttrib(Ihandle* ih)
{
  COLORREF cr = (COLORREF)SendMessage(ih->handle, TVM_GETBKCOLOR, 0, 0);
  if (cr == (COLORREF)-1)
    return IupGetGlobal("TXTBGCOLOR");
  else
  {
    char* str = iupStrGetMemory(20);
    sprintf(str, "%d %d %d", (int)GetRValue(cr), (int)GetGValue(cr), (int)GetBValue(cr));
    return str;
  }
}

static int winTreeSetRenameCaretPos(Ihandle* ih)
{
  HWND editLabel = (HWND)iupAttribGet(ih, "_IUPTREE_EDITNAME");
  int pos = 1;

  sscanf(IupGetAttribute(ih, "RENAMECARET"), "%i", &pos);
  if (pos < 1) pos = 1;
  pos--; /* IUP starts at 1 */

  SendMessage(editLabel, EM_SETSEL, (WPARAM)pos, (LPARAM)pos);

  return 1;
}

static int winTreeSetRenameSelectionPos(Ihandle* ih)
{
  HWND editLabel = (HWND)iupAttribGet(ih, "_IUPTREE_EDITNAME");
  int start = 1, end = 1;

  if (iupStrToIntInt(IupGetAttribute(ih, "RENAMESELECTION"), &start, &end, ':') != 2) 
    return 0;

  if(start < 1 || end < 1) 
    return 0;

  start--; /* IUP starts at 1 */
  end--;

  SendMessage(editLabel, EM_SETSEL, (WPARAM)start, (LPARAM)end);

  return 1;
}

static char* winTreeGetTitleAttrib(Ihandle* ih, const char* name_id)
{
  TVITEM item;
  HTREEITEM hItem = winTreeFindNodeFromString(ih, name_id);
  if (!hItem)
    return NULL;

  item.hItem = hItem;
  item.mask = TVIF_HANDLE | TVIF_TEXT; 
  item.pszText = iupStrGetMemory(255);
  item.cchTextMax = 255;
  SendMessage(ih->handle, TVM_GETITEM, 0, (LPARAM)(LPTVITEM)&item);

  return item.pszText;
}

static int winTreeSetTitleAttrib(Ihandle* ih, const char* name_id, const char* value)
{
  TVITEM item; 
  HTREEITEM hItem = winTreeFindNodeFromString(ih, name_id);
  if (!hItem)
    return 0;

  item.hItem = hItem;
  item.mask = TVIF_HANDLE | TVIF_TEXT; 
  item.pszText = (char*)value;
  SendMessage(ih->handle, TVM_SETITEM, 0, (LPARAM)(const LPTVITEM)&item);
  return 0;
}

static char* winTreeGetUserDataAttrib(Ihandle* ih, const char* name_id)
{
  TVITEM item;
  winTreeItemData* itemData;
  HTREEITEM hItem = winTreeFindNodeFromString(ih, name_id);
  if (!hItem)
    return NULL;

  item.hItem = hItem;
  item.mask = TVIF_HANDLE | TVIF_PARAM;
  SendMessage(ih->handle, TVM_GETITEM, 0, (LPARAM)(LPTVITEM)&item);
  itemData = (winTreeItemData*)item.lParam;

  return itemData->userdata;
}

static int winTreeSetUserDataAttrib(Ihandle* ih, const char* name_id, const char* value)
{
  TVITEM item; 
  winTreeItemData* itemData;
  HTREEITEM hItem = winTreeFindNodeFromString(ih, name_id);
  if (!hItem)
    return 0;

  item.hItem = hItem;
  item.mask = TVIF_HANDLE | TVIF_PARAM;
  SendMessage(ih->handle, TVM_GETITEM, 0, (LPARAM)(LPTVITEM)&item);
  itemData = (winTreeItemData*)item.lParam;

  itemData->userdata = (void*)value;

  return 0;
}

static char* winTreeGetTitleFontAttrib(Ihandle* ih, const char* name_id)
{
  TVITEM item;
  winTreeItemData* itemData;
  HTREEITEM hItem = winTreeFindNodeFromString(ih, name_id);
  if (!hItem)
    return NULL;

  item.hItem = hItem;
  item.mask = TVIF_HANDLE | TVIF_PARAM;
  SendMessage(ih->handle, TVM_GETITEM, 0, (LPARAM)(LPTVITEM)&item);
  itemData = (winTreeItemData*)item.lParam;

  return iupwinFindHFont(itemData->hFont);
}

static int winTreeSetTitleFontAttrib(Ihandle* ih, const char* name_id, const char* value)
{
  TVITEM item; 
  winTreeItemData* itemData;
  HTREEITEM hItem = winTreeFindNodeFromString(ih, name_id);
  if (!hItem)
    return 0;

  item.hItem = hItem;
  item.mask = TVIF_HANDLE | TVIF_PARAM;
  SendMessage(ih->handle, TVM_GETITEM, 0, (LPARAM)(LPTVITEM)&item);
  itemData = (winTreeItemData*)item.lParam;

  if (value)
    itemData->hFont = iupwinGetHFont(value);
  else
    itemData->hFont = NULL;

  iupdrvDisplayUpdate(ih);

  return 0;
}

static char* winTreeGetIndentationAttrib(Ihandle* ih)
{
  char* str = iupStrGetMemory(255);
  int indent = (int)SendMessage(ih->handle, TVM_GETINDENT, 0, 0);
  sprintf(str, "%d", indent);
  return str;
}

static int winTreeSetIndentationAttrib(Ihandle* ih, const char* value)
{
  int indent;
  if (iupStrToInt(value, &indent))
    SendMessage(ih->handle, TVM_SETINDENT, (WPARAM)indent, 0);
  return 0;
}

static char* winTreeGetStateAttrib(Ihandle* ih, const char* name_id)
{
  TVITEM item;
  winTreeItemData* itemData;
  HTREEITEM hItem = winTreeFindNodeFromString(ih, name_id);
  if (!hItem)
    return NULL;

  item.hItem = hItem;
  item.mask = TVIF_HANDLE|TVIF_PARAM; 
  SendMessage(ih->handle, TVM_GETITEM, 0, (LPARAM)(LPTVITEM)&item);
  itemData = (winTreeItemData*)item.lParam;

  if(itemData->kind == ITREE_BRANCH)
  {
    if(SendMessage(ih->handle, TVM_EXPAND, TVE_COLLAPSE, (LPARAM)hItem) == 0)
    {
      return "COLLAPSED";
    }
    else
    {
      SendMessage(ih->handle, TVM_EXPAND, TVE_EXPAND, (LPARAM)hItem);
      return "EXPANDED";
    }
  }

  return NULL;
}

static int winTreeSetStateAttrib(Ihandle* ih, const char* name_id, const char* value)
{
  HTREEITEM hItem = winTreeFindNodeFromString(ih, name_id);
  if (!hItem)
    return 0;

  if(iupStrEqualNoCase(value, "COLLAPSED"))
    SendMessage(ih->handle, TVM_EXPAND, TVE_COLLAPSE, (LPARAM)hItem);
  else if(iupStrEqualNoCase(value, "EXPANDED"))
    SendMessage(ih->handle, TVM_EXPAND, TVE_EXPAND, (LPARAM)hItem);

  return 0;
}

static char* winTreeGetDepthAttrib(Ihandle* ih, const char* name_id)
{
  HTREEITEM hItem = winTreeFindNodeFromString(ih, name_id);
  HTREEITEM hItemRoot = (HTREEITEM)SendMessage(ih->handle, TVM_GETNEXTITEM, TVGN_ROOT, 0);
  int dep = 0;
  char* depth;

  if (!hItem)
    return NULL;

  while((hItemRoot != hItem) && (hItem != NULL))
  {
    hItem = (HTREEITEM)SendMessage(ih->handle, TVM_GETNEXTITEM, TVGN_PARENT, (LPARAM)hItem);
    dep++;
  }

  depth = iupStrGetMemory(10);
  sprintf(depth, "%d", dep);
  return depth;
}

static int winTreeSetMoveNodeAttrib(Ihandle* ih, const char* name_id, const char* value)
{
  int curDepth, newDepth;
  HTREEITEM hBrotherItem, hParent;
  HTREEITEM hItem = winTreeFindNodeFromString(ih, name_id);
  if (!hItem)
    return 0;

  iupStrToInt(winTreeGetDepthAttrib(ih, name_id), &curDepth);
  iupStrToInt(value, &newDepth);

  /* just the root node has depth = 0 */
  if(newDepth <= 0)
    return 0;

  if(curDepth < newDepth)  /* Top -> Down */
  {
    ih->data->id_control = newDepth - curDepth - 1;  /* subtract 1 (one) to reach the level of its new parent */

    /* Firstly, the node will be child of your brother - this avoids circular reference */
    hBrotherItem = winTreeFindNewBrother(ih, (HTREEITEM)SendMessage(ih->handle, TVM_GETNEXTITEM, TVGN_NEXT, (LPARAM)hItem));

    /* Setting the new parent */
    hParent = (HTREEITEM)SendMessage(ih->handle, TVM_GETNEXTITEM, TVGN_PARENT, (LPARAM)hBrotherItem);
  }
  else if (curDepth > newDepth)  /* Bottom -> Up */
  {
    /* When the new depth is less than the current depth, 
       simply define a new parent to the node */
    
    ih->data->id_control = curDepth - newDepth + 1;  /* add 1 (one) to reach the level of its new parent */
    
    /* Starting the search by the parent of the current node */
    hParent = hItem;
    while(ih->data->id_control != 0)
    {
      /* Setting the new parent */
      hParent = (HTREEITEM)SendMessage(ih->handle, TVM_GETNEXTITEM, TVGN_PARENT, (LPARAM)hParent);
      ih->data->id_control--;
    }
  }
  else /* same depth, nothing to do */
    return 0;

  /* without parent, nothing to do */
  if(hParent == NULL)
    return 0;

  /* Copying the node and its children to the new position */
  winTreeCopyBranch(ih, hItem, hParent);

  /* Deleting the node of its old position */
  SendMessage(ih->handle, TVM_DELETEITEM, 0, (LPARAM)hItem);

  return 0;
}

static char* winTreeGetColorAttrib(Ihandle* ih, const char* name_id)
{
  unsigned char r, g, b;
  char* str;
  TVITEM item;
  winTreeItemData* itemData;
  HTREEITEM hItem = winTreeFindNodeFromString(ih, name_id);
  if (!hItem)
    return NULL;

  item.hItem = hItem;
  item.mask = TVIF_HANDLE | TVIF_PARAM;
  SendMessage(ih->handle, TVM_GETITEM, 0, (LPARAM)(LPTVITEM)&item);
  itemData = (winTreeItemData*)item.lParam;

  r = GetRValue(itemData->color);
  g = GetGValue(itemData->color);
  b = GetBValue(itemData->color);

  str = iupStrGetMemory(12);
  sprintf(str, "%d %d %d", r, g, b);
  return str;
}
 
static int winTreeSetColorAttrib(Ihandle* ih, const char* name_id, const char* value)
{
  unsigned char r, g, b;
  TVITEM item;
  winTreeItemData* itemData;
  HTREEITEM hItem = winTreeFindNodeFromString(ih, name_id);
  if (!hItem)
    return 0;

  item.hItem = hItem;
  item.mask = TVIF_HANDLE | TVIF_PARAM;
  SendMessage(ih->handle, TVM_GETITEM, 0, (LPARAM)(LPTVITEM)&item);
  itemData = (winTreeItemData*)item.lParam;

  if (iupStrToRGB(value, &r, &g, &b))
  {
    itemData->color = RGB(r,g,b);
    iupdrvDisplayUpdate(ih);
  }

 return 0;
}

static int winTreeSetFgColorAttrib(Ihandle* ih, const char* value)
{
  unsigned char r, g, b;

  if (iupStrToRGB(value, &r, &g, &b))
  {
    COLORREF color = RGB(r,g,b);
    SendMessage(ih->handle, TVM_SETTEXTCOLOR, 0, (LPARAM)color);
  }
  else
    SendMessage(ih->handle, TVM_SETTEXTCOLOR, 0, (LPARAM)CLR_DEFAULT);

  return 0;
}

static char* winTreeGetChildCountAttrib(Ihandle* ih, const char* name_id)
{
  int count;
  char* str;
  HTREEITEM hItem = winTreeFindNodeFromString(ih, name_id);
  if (!hItem)
    return NULL;

  count = 0;
  hItem = (HTREEITEM)SendMessage(ih->handle, TVM_GETNEXTITEM, TVGN_CHILD, (LPARAM)hItem);
  while(hItem != NULL)
  {
    count++;

    /* Go to next sibling item */
    hItem = (HTREEITEM)SendMessage(ih->handle, TVM_GETNEXTITEM, TVGN_NEXT, (LPARAM)hItem);
  }

  str = iupStrGetMemory(10);
  sprintf(str, "%d", count);
  return str;
}

static char* winTreeGetCountAttrib(Ihandle* ih)
{
  char* str = iupStrGetMemory(10);
  sprintf(str, "%d", (int)SendMessage(ih->handle, TVM_GETCOUNT, 0, 0));
  return str;
}

static char* winTreeGetKindAttrib(Ihandle* ih, const char* name_id)
{
  TVITEM item; 
  winTreeItemData* itemData;
  HTREEITEM hItem = winTreeFindNodeFromString(ih, name_id);
  if (!hItem)
    return NULL;

  item.hItem = hItem;
  item.mask = TVIF_HANDLE|TVIF_PARAM; 
  SendMessage(ih->handle, TVM_GETITEM, 0, (LPARAM)(LPTVITEM)&item);
  itemData = (winTreeItemData*)item.lParam;

  if(itemData->kind == ITREE_BRANCH)
    return "BRANCH";
  else
    return "LEAF";
}

static char* winTreeGetParentAttrib(Ihandle* ih, const char* name_id)
{
  char* id;
  HTREEITEM hItemRoot = (HTREEITEM)SendMessage(ih->handle, TVM_GETNEXTITEM, TVGN_ROOT, 0);
  HTREEITEM hItem = winTreeFindNodeFromString(ih, name_id);
  if (!hItem)
    return NULL;

  hItem = (HTREEITEM)SendMessage(ih->handle, TVM_GETNEXTITEM, TVGN_PARENT, (LPARAM)hItem);
  if (!hItem)
    return NULL;

  ih->data->id_control = -1;
  winTreeFindNodeFromID(ih, hItemRoot, hItem);

  id = iupStrGetMemory(10);
  sprintf(id, "%d", ih->data->id_control);
  return id;
}

static void winTreeDelNodeData(Ihandle* ih, HTREEITEM hItem)
{
  TVITEM item; 
  winTreeItemData* itemData;
  HTREEITEM hChildItem;

  item.hItem = hItem;
  item.mask = TVIF_HANDLE|TVIF_PARAM; 
  SendMessage(ih->handle, TVM_GETITEM, 0, (LPARAM)(LPTVITEM)&item);
  itemData = (winTreeItemData*)item.lParam;
  free(itemData);

  hChildItem = (HTREEITEM)SendMessage(ih->handle, TVM_GETNEXTITEM, TVGN_CHILD, (LPARAM)hItem);
  while (hChildItem)
  {
    winTreeDelNodeData(ih, hChildItem);
    hChildItem = (HTREEITEM)SendMessage(ih->handle, TVM_GETNEXTITEM, TVGN_NEXT, (LPARAM)hChildItem);
  }
}

static int winTreeSetDelNodeAttrib(Ihandle* ih, const char* name_id, const char* value)
{
  if(iupStrEqualNoCase(value, "SELECTED")) /* selectec here means the specified one */
  {
    HTREEITEM hItem = winTreeFindNodeFromString(ih, name_id);
    HTREEITEM hItemRoot  = (HTREEITEM)SendMessage(ih->handle, TVM_GETNEXTITEM, TVGN_ROOT, 0);

    /* the root node can't be deleted */
    if(!hItem || hItem == hItemRoot)
      return 0;

    /* deleting the specified node (and it's children) */
    winTreeDelNodeData(ih, hItem);
    SendMessage(ih->handle, TVM_DELETEITEM, 0, (LPARAM)hItem);
    
    return 0;
  }
  else if(iupStrEqualNoCase(value, "CHILDREN"))  /* children of the specified one */
  {
    HTREEITEM hItem = winTreeFindNodeFromString(ih, name_id);
    HTREEITEM hItemRoot  = (HTREEITEM)SendMessage(ih->handle, TVM_GETNEXTITEM, TVGN_ROOT, 0);
    HTREEITEM hChildItem = (HTREEITEM)SendMessage(ih->handle, TVM_GETNEXTITEM, TVGN_CHILD, (LPARAM)hItem);

    /* the root node can't be deleted */
    if(!hItem || hItem == hItemRoot)
      return 0;

    /* deleting the selected node's children */
    while(hChildItem && SendMessage(ih->handle, TVM_GETITEMSTATE, (WPARAM)hItem, TVIS_SELECTED))
    {
      winTreeDelNodeData(ih, hChildItem);
      SendMessage(ih->handle, TVM_DELETEITEM, 0, (LPARAM)hChildItem);
      hChildItem = (HTREEITEM)SendMessage(ih->handle, TVM_GETNEXTITEM, TVGN_CHILD, (LPARAM)hItem);
    }

    return 0;
  }
  else if(iupStrEqualNoCase(value, "MARKED"))
  {
    int i;
    Iarray* markedArray;
    HTREEITEM* hItemArrayData;
    HTREEITEM  hItemRoot = (HTREEITEM)SendMessage(ih->handle, TVM_GETNEXTITEM, TVGN_ROOT, 0);

    /* Delete the array of marked nodes */
    markedArray = (Iarray*)iupAttribGet(ih, "_IUPWINTREE_MARKEDITEMARRAY");
    if(!markedArray)
      return 0;

    hItemArrayData = (HTREEITEM*)iupArrayGetData(markedArray);

    for(i = 0; i < iupArrayCount(markedArray); i++)
    {
      if (hItemArrayData[i] != hItemRoot)  /* the root node can't be deleted */
      {
        winTreeDelNodeData(ih, hItemArrayData[i]);
        SendMessage(ih->handle, TVM_DELETEITEM, 0, (LPARAM)hItemArrayData[i]);
      }
    }

    iupArrayDestroy(markedArray);
    iupAttribSetStr(ih, "_IUPWINTREE_MARKEDITEMARRAY", NULL);

    return 0;
  }

  return 0;
}

// static char* winTreeGetShowRenameAttrib(Ihandle* ih)
// {
//   DWORD dwStyle = (DWORD)GetWindowLongPtr(ih->handle, GWL_STYLE);
// 
//   if(dwStyle & TVS_EDITLABELS)
//     return "YES";
//   else
//     return "NO";
// }
// 
// static int winTreeSetShowRenameAttrib(Ihandle* ih, const char* value)
// {
//   if(iupStrEqualNoCase(value, "YES"))
//     iupwinSetStyle(ih, TVS_EDITLABELS, 1);
//   else
//     iupwinSetStyle(ih, TVS_EDITLABELS, 0);
// 
//   return 1;
// }

static int winTreeSetRenameAttrib(Ihandle* ih, const char* name_id, const char* value)
{  
  if(IupGetInt(ih, "SHOWRENAME"))
    ;//iupdrvTreeSetTitleAttrib(ih, name_id, value);
  else
    winTreeRenameNode_CB(ih);
  
  return 0;
}

static char* winTreeGetMarkedAttrib(Ihandle* ih, const char* name_id)
{
  HTREEITEM hItem = winTreeFindNodeFromString(ih, name_id);
  int state = SendMessage(ih->handle, TVM_GETITEMSTATE, (WPARAM)hItem, TVIS_SELECTED);
  if (!hItem)
    return NULL;

  if(state & TVIS_SELECTED)
    return "YES";
  else if(state & ~TVIS_SELECTED)
    return "NO";

  return NULL;
}

static int winTreeSetMarkedAttrib(Ihandle* ih, const char* name_id, const char* value)
{
  TVITEM item;
  HTREEITEM hItem = winTreeFindNodeFromString(ih, name_id);
  if (!hItem)
    return 0;

  item.hItem = hItem;
  item.mask = TVIF_HANDLE|TVIF_STATE;

  SendMessage(ih->handle, TVM_GETITEM, 0, (LPARAM)(LPTVITEM)&item);

  if (iupStrBoolean(value))
  {
    item.state     |= TVIS_SELECTED;
    item.stateMask |= TVIS_SELECTED;
  }
  else
  {
    item.state     &= ~TVIS_SELECTED;
    item.stateMask &= ~TVIS_SELECTED;
  }

  SendMessage(ih->handle, TVM_SETITEM, 0, (LPARAM)(const LPTVITEM)&item);

  return 0;
}

static int winTreeSetStartingAttrib(Ihandle* ih, const char* name_id)
{
  HTREEITEM hItem = winTreeFindNodeFromString(ih, name_id);
  if (!hItem)
    return 0;

  iupAttribSetStr(ih, "_IUPTREE_STARTINGITEM", (char*)hItem);

  return 1;
}

static char* winTreeGetValueAttrib(Ihandle* ih)
{
  HTREEITEM selected = (HTREEITEM)SendMessage(ih->handle, TVM_GETNEXTITEM, TVGN_CARET, 0);
  HTREEITEM hItemRoot = (HTREEITEM)SendMessage(ih->handle, TVM_GETNEXTITEM, TVGN_ROOT, 0);
  char* id = iupStrGetMemory(16);

  if(selected == NULL)
    return 0;

  ih->data->id_control = -1;
  winTreeFindNodeFromID(ih, hItemRoot, selected);
  sprintf(id, "%d", ih->data->id_control);

  return id;
}

static int winTreeSetMarkAttrib(Ihandle* ih, const char* value)
{
  if(iupStrEqualNoCase(value, "BLOCK"))
  {
    HTREEITEM hItemSelected = (HTREEITEM)SendMessage(ih->handle, TVM_GETNEXTITEM, TVGN_CARET, 0);
    winTreeSelectItems(ih, (HTREEITEM)iupAttribGet(ih, "_IUPTREE_STARTINGITEM"), hItemSelected);
  }
  else if(iupStrEqualNoCase(value, "CLEARALL"))
    winTreeClearSelection(ih);
  else if(iupStrEqualNoCase(value, "MARKALL"))
    winTreeAllSelection(ih);
  else if(iupStrEqualNoCase(value, "INVERTALL"))
  {
    /* INVERTALL *MUST* appear before INVERT, or else INVERTALL will never be called. */
    HTREEITEM hItem = (HTREEITEM)SendMessage(ih->handle, TVM_GETNEXTITEM, TVGN_ROOT, 0);
    TVITEM item;

    while(hItem)
    {
      if((SendMessage(ih->handle, TVM_GETITEMSTATE, (WPARAM)hItem, TVIS_SELECTED)) & TVIS_SELECTED)
      {
        item.mask = TVIF_HANDLE|TVIF_STATE;
        item.hItem = hItem;
        item.stateMask = TVIS_SELECTED;
        item.state = 0;
        SendMessage(ih->handle, TVM_SETITEM, 0, (LPARAM)(const LPTVITEM)&item);

        iupAttribSetStr(ih, "_IUPWINTREE_MARKEDITEMARRAY", NULL);
      }
      else
      {
        item.mask = TVIF_HANDLE|TVIF_STATE;
        item.hItem = hItem;
        item.stateMask = TVIS_SELECTED;
        item.state = TVIS_SELECTED;
        SendMessage(ih->handle, TVM_SETITEM, 0, (LPARAM)(const LPTVITEM)&item);

        winTreeMarkedItemArray(ih, hItem);
      }

      hItem = (HTREEITEM)SendMessage(ih->handle, TVM_GETNEXTITEM, TVGN_NEXTVISIBLE, (LPARAM)hItem);
    }
  }
  else if(iupStrEqualPartial(value, "INVERT"))
  {
    /* iupStrEqualPartial allows the use of "INVERTid" form */
    TVITEM item;
    HTREEITEM hItem = winTreeFindNodeFromString(ih, &value[strlen("INVERT")]);
    if (!hItem)
      return 0;

    if((SendMessage(ih->handle, TVM_GETITEMSTATE, (WPARAM)hItem, TVIS_SELECTED)) & TVIS_SELECTED)
    {
      item.mask = TVIF_HANDLE|TVIF_STATE;
      item.hItem = hItem;
      item.stateMask = TVIS_SELECTED;
      item.state = 0;
      SendMessage(ih->handle, TVM_SETITEM, 0, (LPARAM)(const LPTVITEM)&item);

      iupAttribSetStr(ih, "_IUPWINTREE_MARKEDITEMARRAY", NULL);
    }
    else
    {
      item.mask = TVIF_HANDLE|TVIF_STATE;
      item.hItem = hItem;
      item.stateMask = TVIS_SELECTED;
      item.state = TVIS_SELECTED;
      SendMessage(ih->handle, TVM_SETITEM, 0, (LPARAM)(const LPTVITEM)&item);

      winTreeMarkedItemArray(ih, hItem);
    }
  }
  else
  {
    HTREEITEM hItem1, hItem2;

    char str1[50], str2[50];
    if (iupStrToStrStr(value, str1, str2, '-')!=2)
      return 0;

    hItem1 = winTreeFindNodeFromString(ih, str1);
    if (!hItem1)  
      return 0;
    hItem2 = winTreeFindNodeFromString(ih, str2);
    if (!hItem2)  
      return 0;

    winTreeSelectItems(ih, hItem1, hItem2);
  }

  return 1;
} 

static int winTreeSetValueAttrib(Ihandle* ih, const char* value)
{
  HTREEITEM hItemNewSelected = NULL;
  HTREEITEM hItemSelected;

  if (winTreeSetMarkAttrib(ih, value))
    return 0;

  hItemSelected = (HTREEITEM)SendMessage(ih->handle, TVM_GETNEXTITEM, TVGN_CARET, 0);

  if(iupStrEqualNoCase(value, "ROOT"))
    hItemNewSelected = (HTREEITEM)SendMessage(ih->handle, TVM_GETNEXTITEM, TVGN_ROOT, 0);
  else if(iupStrEqualNoCase(value, "LAST"))
    hItemNewSelected = (HTREEITEM)SendMessage(ih->handle, TVM_GETNEXTITEM, TVGN_LASTVISIBLE, 0);
  else if(iupStrEqualNoCase(value, "PGUP"))
  {
    int i;
    HTREEITEM hItemPrev = hItemSelected;
    HTREEITEM hItemNext = hItemSelected;
    for(i = 0; i < 10; i++)
    {
      hItemNext = hItemPrev;
      hItemPrev = (HTREEITEM)SendMessage(ih->handle, TVM_GETNEXTITEM, TVGN_PREVIOUSVISIBLE, (LPARAM)hItemPrev);
      if(hItemPrev == NULL)
      {
        hItemPrev = hItemNext;
        break;
      }
    }
    
    hItemNewSelected = hItemPrev;
  }
  else if(iupStrEqualNoCase(value, "PGDN"))
  {
    int i;
    HTREEITEM hItemPrev = hItemSelected;
    HTREEITEM hItemNext = hItemSelected;
    
    for(i = 0; i < 10; i++)
    {
      hItemPrev = hItemNext;
      hItemNext = (HTREEITEM)SendMessage(ih->handle, TVM_GETNEXTITEM, TVGN_NEXTVISIBLE, (LPARAM)hItemNext);
      if(hItemNext == NULL)
      {
        hItemNext = hItemPrev;
        break;
      }
    }
    
    hItemNewSelected = hItemNext;
  }
  else if(iupStrEqualNoCase(value, "NEXT"))
    hItemNewSelected = (HTREEITEM)SendMessage(ih->handle, TVM_GETNEXTITEM, TVGN_NEXTVISIBLE, (LPARAM)hItemSelected);
  else if(iupStrEqualNoCase(value, "PREVIOUS"))
    hItemNewSelected = (HTREEITEM)SendMessage(ih->handle, TVM_GETNEXTITEM, TVGN_PREVIOUSVISIBLE, (LPARAM)hItemSelected);
  else
    hItemNewSelected = winTreeFindNodeFromString(ih, value);

  if (hItemNewSelected)
    SendMessage(ih->handle, TVM_SELECTITEM, TVGN_CARET, (LPARAM)hItemNewSelected);

  return 0;
} 


/*********************************************************************************************************/


static int winTreeEditProc(Ihandle* ih, HWND cbedit, UINT msg, WPARAM wp, LPARAM lp, LRESULT *result)
{
  switch (msg)
  {
    case WM_GETDLGCODE:
    {
      MSG* pMsg = (MSG*)lp;

      if (pMsg && (pMsg->message == WM_KEYDOWN || pMsg->message == WM_SYSKEYDOWN))
      {
        if (pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_RETURN)
        {
          *result = DLGC_WANTALLKEYS;
          return 1;
        }
      }
    }
  }
  
  (void)wp;
  (void)cbedit;
  (void)ih;
  return 0;
}

static LRESULT CALLBACK winTreeEditWinProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{   
  int ret = 0;
  LRESULT result = 0;
  WNDPROC oldProc;
  Ihandle *ih;

  ih = iupwinHandleGet(hwnd); 
  if (!ih)
    return DefWindowProc(hwnd, msg, wp, lp);  /* should never happen */

  /* retrieve the control previous procedure for subclassing */
  oldProc = (WNDPROC)IupGetCallback(ih, "_IUPWIN_EDITOLDPROC_CB");

  ret = winTreeEditProc(ih, hwnd, msg, wp, lp, &result);

  if (ret)
    return result;
  else
    return CallWindowProc(oldProc, hwnd, msg, wp, lp);
}

static void winTreeDragStart(Ihandle* ih, LPARAM lp)
{
  HTREEITEM	hItemDrop;
  HIMAGELIST dragImageList = (HIMAGELIST)iupAttribGet(ih, "_IUPWINTREE_DRAGIMAGELIST");

  if (dragImageList)
  {
    POINT pnt;
    
    pnt.x = LOWORD(lp);
    pnt.y = HIWORD(lp);

    GetCursorPos(&pnt);
    ClientToScreen(GetDesktopWindow(), &pnt) ;
    ImageList_DragMove(pnt.x, pnt.y);
  }

  if ((hItemDrop = winTreeFindNodePointed(ih)) != NULL)
  {
    HTREEITEM hItemRoot = (HTREEITEM)SendMessage(ih->handle, TVM_GETNEXTITEM, TVGN_ROOT, 0);

    if(dragImageList)
      ImageList_DragShowNolock(FALSE);

    SendMessage(ih->handle, TVM_SELECTITEM, TVGN_DROPHILITE, (LPARAM)hItemDrop);

    /* store the drop item to be executed */
    iupAttribSetStr(ih, "_IUPTREE_DROPITEM", (char*)hItemDrop);

    if(dragImageList)
      ImageList_DragShowNolock(TRUE);

    ih->data->id_control = -1;
    winTreeFindNodeFromID(ih, hItemRoot, hItemDrop);
    iupAttribSetInt(ih, "_IUPTREE_DROPID", ih->data->id_control);
  }
}

static void winTreeDropEnd(Ihandle* ih)
{
  HTREEITEM  hItemParent;
  HTREEITEM  hItemNew;
  HTREEITEM	 hItemDrag     =  (HTREEITEM)iupAttribGet(ih, "_IUPTREE_DRAGITEM");
  HTREEITEM	 hItemDrop     =  (HTREEITEM)iupAttribGet(ih, "_IUPTREE_DROPITEM");
  HIMAGELIST dragImageList = (HIMAGELIST)iupAttribGet(ih, "_IUPWINTREE_DRAGIMAGELIST");

  if(dragImageList)
  {
    ImageList_DragLeave(ih->handle);
    ImageList_EndDrag();
    ReleaseCapture();
    ShowCursor(TRUE);
    ImageList_Destroy(dragImageList);
  }

  /* Remove drop target highlighting */
  SendMessage(ih->handle, TVM_SELECTITEM, TVGN_DROPHILITE, (LPARAM)NULL);

  iupAttribSetStr(ih, "_IUPTREE_DRAGITEM", NULL);

  if (hItemDrag == hItemDrop)
    return;

  /* DragDrop Callback */
  winTreeDragDrop_CB(ih);

  /* If Drag item is an ancestor of Drop item then return */
  hItemParent = hItemDrop;

  while((hItemParent = (HTREEITEM)SendMessage(ih->handle, TVM_GETNEXTITEM, TVGN_PARENT, (LPARAM)hItemParent)) != NULL)
  {
    if( hItemParent == hItemDrag )
      return;
  }

  SendMessage(ih->handle, TVM_EXPAND, TVE_EXPAND, (LPARAM)hItemDrop);

  hItemNew = winTreeCopyBranch(ih, hItemDrag, hItemDrop);
  SendMessage(ih->handle, TVM_DELETEITEM, 0, (LPARAM)hItemDrag);
  SendMessage(ih->handle, TVM_SELECTITEM, TVGN_CARET, (LPARAM)hItemNew);
}  

static int winTreeProc(Ihandle* ih, UINT msg, WPARAM wp, LPARAM lp, LRESULT *result)
{
  switch (msg)
  {
  case WM_GETDLGCODE:
    {
      *result = DLGC_WANTALLKEYS;
      return 1;
    }
  case WM_KEYDOWN:
  case WM_SYSKEYDOWN:
    {
      if (wp == VK_RETURN)
      {
        HTREEITEM selected = (HTREEITEM)SendMessage(ih->handle, TVM_GETNEXTITEM, TVGN_CARET, 0);
        TVITEM item;
        item.hItem = selected;
        item.mask = TVIF_HANDLE | TVIF_STATE;
        SendMessage(ih->handle, TVM_GETITEM, 0, (LPARAM)(LPTVITEM)&item);
        
        winTreeBranchLeaf_CB(ih);

        if (item.state & TVIS_EXPANDED)
          SendMessage(ih->handle, TVM_EXPAND, TVE_COLLAPSE, (LPARAM)selected);
        else
          SendMessage(ih->handle, TVM_EXPAND, TVE_EXPAND, (LPARAM)selected);

        return 1;
      }      
      else if (wp == VK_F2)
      {
        if(IupGetInt(ih, "SHOWRENAME"))
        {
          winTreeShowRename_CB(ih);
          return 0;
        }
        else
        {
          winTreeRenameNode_CB(ih);
          return 0;
        }
      }
      else if (wp == VK_TAB)
      {
        if (GetKeyState(VK_SHIFT) & 0x8000)
          IupPreviousField(ih);
        else
          IupNextField(ih);
        return 0;
      }
      break;
    }
  case WM_LBUTTONDBLCLK:
  case WM_MBUTTONDBLCLK:
  case WM_RBUTTONDBLCLK:
  case WM_LBUTTONDOWN:
  case WM_MBUTTONDOWN:
  case WM_RBUTTONDOWN:
    if (iupwinButtonDown(ih, msg, wp, lp)==-1)
    {
      *result = 0;
      return 1;
    }
    break;
  case WM_MBUTTONUP:
  case WM_RBUTTONUP:
  case WM_LBUTTONUP:
    if (IupGetInt(ih, "SHOWDRAGDROP") && (HTREEITEM)iupAttribGet(ih, "_IUPTREE_DRAGITEM") != NULL)
      winTreeDropEnd(ih);
    else if (iupwinButtonUp(ih, msg, wp, lp)==-1)
    {
      *result = 0;
      return 1;
    }
    break;
  case WM_MOUSEMOVE:
    if (IupGetInt(ih, "SHOWDRAGDROP") && (HTREEITEM)iupAttribGet(ih, "_IUPTREE_DRAGITEM") != NULL)
      winTreeDragStart(ih, lp);
    else
      iupwinMouseMove(ih, msg, wp, lp);
    break;
  }

  return iupwinBaseProc(ih, msg, wp, lp, result);
}

static int winTreeWmNotify(Ihandle* ih, NMHDR* msg_info, int *result)
{
  if(msg_info->code == TVN_SELCHANGING)
  {
    winTreeSelection_CB(ih, 0);  /* node was unselected */
  }
  else if(msg_info->code == TVN_SELCHANGED)
  {
    winTreeSelection_CB(ih, 1);  /* node was unselected */
  }
  else if(msg_info->code == TVN_BEGINLABELEDIT)
  {
    HWND editLabel = (HWND)SendMessage(ih->handle, TVM_GETEDITCONTROL, 0, 0);

    iupwinHandleAdd(ih, editLabel);
    iupAttribSetStr(ih, "_IUPTREE_EDITNAME", (char*)editLabel);

    /* subclass the edit box. */
    IupSetCallback(ih, "_IUPWIN_EDITOLDPROC_CB", (Icallback)GetWindowLongPtr(editLabel, GWLP_WNDPROC));
    SetWindowLongPtr(editLabel, GWLP_WNDPROC, (LONG_PTR)winTreeEditWinProc);

    if(IupGetAttribute(ih, "RENAMECARET"))
      winTreeSetRenameCaretPos(ih);

    if(IupGetAttribute(ih, "RENAMESELECTION"))
      winTreeSetRenameSelectionPos(ih);
  }
  else if(msg_info->code == TVN_ENDLABELEDIT)
  {
    HWND editLabel = (HWND)iupAttribGet(ih, "_IUPTREE_EDITNAME");

    winTreeRename_CB(ih, (NMTVDISPINFO*)msg_info);

    if(editLabel)
      DestroyWindow((HWND)iupAttribGet(ih, "_IUPTREE_EDITNAME"));

    iupAttribSetStr(ih, "_IUPTREE_EDITNAME", NULL);
  }
  else if(msg_info->code == TVN_BEGINDRAG)
  {
    if(IupGetInt(ih, "SHOWDRAGDROP"))
    {
      NMTREEVIEW* pNMTreeView = (NMTREEVIEW*)msg_info;
      HTREEITEM   hItemRoot   = (HTREEITEM)SendMessage(ih->handle, TVM_GETNEXTITEM, TVGN_ROOT, 0);
      HTREEITEM   hItemDrag   = pNMTreeView->itemNew.hItem;
      HIMAGELIST  dragImageList;

      /* store the drag-and-drop items */
      iupAttribSetStr(ih, "_IUPTREE_DRAGITEM", (char*)hItemDrag);
      iupAttribSetStr(ih, "_IUPTREE_DROPITEM", NULL);

      /* set the drag id */
      ih->data->id_control = -1;
      winTreeFindNodeFromID(ih, hItemRoot, hItemDrag);
      iupAttribSetInt(ih, "_IUPTREE_DRAGID", ih->data->id_control);

      /* get the image list for dragging */
      dragImageList = (HIMAGELIST)SendMessage(ih->handle, TVM_CREATEDRAGIMAGE, 0, (LPARAM)hItemDrag);

      if(dragImageList)
      {
        POINT pt = pNMTreeView->ptDrag;
        iupAttribSetStr(ih, "_IUPWINTREE_DRAGIMAGELIST", (char*)dragImageList);
        ImageList_BeginDrag(dragImageList, 0, 0, 0);
        ShowCursor(FALSE);
        SetCapture(GetDesktopWindow());
        ClientToScreen(ih->handle, &pt);
        ImageList_DragEnter(NULL, pt.x, pt.y);
      }
    }
  }
  else if(msg_info->code == NM_DBLCLK)
  {
    HTREEITEM selected = (HTREEITEM)SendMessage(ih->handle, TVM_GETNEXTITEM, TVGN_CARET, 0);
    TVITEM item;
    winTreeItemData* itemData;

    /* Get Children: branch or leaf */
    item.mask = TVIF_HANDLE|TVIF_PARAM; 
    item.hItem = selected;
    SendMessage(ih->handle, TVM_GETITEM, 0, (LPARAM)(LPTVITEM)&item);
    itemData = (winTreeItemData*)item.lParam;

    if (itemData->kind == ITREE_LEAF)
    {
      IFni cbExecuteLeaf = (IFni)IupGetCallback(ih, "EXECUTELEAF_CB");
      if(cbExecuteLeaf)
      {
        cbExecuteLeaf(ih, IupGetInt(ih, "VALUE"));
        return IUP_DEFAULT;
      }
    }
  }
  else if(msg_info->code == NM_CLICK)
  {
    if((GetKeyState(VK_CONTROL) & 0x8000) && ih->data->tree_ctrl)
    {
      /* Control key is down */
      HTREEITEM hItem = winTreeFindNodePointed(ih);
      TVITEM item;
 
      if(hItem)
      {
        UINT  uNewSelState;
        UINT  uOldSelState;
        HTREEITEM hItemOld;
 
        /* Toggle selection state */
        if((SendMessage(ih->handle, TVM_GETITEMSTATE, (WPARAM)hItem, TVIS_SELECTED)) & TVIS_SELECTED)
          uNewSelState = 0;
        else
          uNewSelState = TVIS_SELECTED;
 
        /* Get old selected (focus) item and state */
        hItemOld = (HTREEITEM)SendMessage(ih->handle, TVM_GETNEXTITEM, TVGN_CARET, 0);     
 
        if(hItemOld)
          uOldSelState = SendMessage(ih->handle, TVM_GETITEMSTATE, (WPARAM)hItemOld, TVIS_SELECTED);
        else
          uOldSelState = 0;
 
        /* Select new item */
         if((HTREEITEM)SendMessage(ih->handle, TVM_GETNEXTITEM, TVGN_CARET, 0) == hItem)
           SendMessage(ih->handle, TVM_SELECTITEM, TVGN_CARET, (LPARAM)NULL);	/* to prevent edit */
 
        /* Set proper selection (highlight) state for new item */
        item.mask = TVIF_HANDLE|TVIF_STATE;
        item.hItem = hItem;
        item.stateMask = TVIS_SELECTED;
        item.state = uNewSelState;
        SendMessage(ih->handle, TVM_SETITEM, 0, (LPARAM)(const LPTVITEM)&item);
 
        /* Restore state of old selected item */
        if (hItemOld && hItemOld != hItem)
        {
          item.mask = TVIF_HANDLE|TVIF_STATE;
          item.hItem = hItemOld;
          item.stateMask = TVIS_SELECTED;
          item.state = uOldSelState;
          SendMessage(ih->handle, TVM_SETITEM, 0, (LPARAM)(const LPTVITEM)&item);
        }
 
        /* Update the array of marked items */
        winTreeMarkedItemArray(ih, hItemOld);
        winTreeMarkedItemArray(ih, hItem);

        return 1;
      }
    }
    else if((GetKeyState(VK_SHIFT) & 0x8000) && ih->data->tree_shift)
    {
      /* Shift key is down */
      HTREEITEM hItem = winTreeFindNodePointed(ih);
      HTREEITEM hItemFirstSel = (HTREEITEM)iupAttribGet(ih, "_IUPWINTREE_FIRSTSELITEM");

      /* Initialize the reference item if this is the first shift selection */
      if(!hItemFirstSel)
      {
        iupAttribSetStr(ih, "_IUPWINTREE_FIRSTSELITEM", (char*)(HTREEITEM)SendMessage(ih->handle, TVM_GETNEXTITEM, TVGN_CARET, 0));
        hItemFirstSel = (HTREEITEM)iupAttribGet(ih, "_IUPWINTREE_FIRSTSELITEM");
      }

      /* Select new item */
      if((HTREEITEM)SendMessage(ih->handle, TVM_GETNEXTITEM, TVGN_CARET, 0) == hItem)
        SendMessage(ih->handle, TVM_SELECTITEM, TVGN_CARET, (LPARAM)NULL);	/* to prevent edit */

      if(hItemFirstSel)
      {
        winTreeSelectItems(ih, hItemFirstSel, hItem);
        winTreeMultiSelection_CB(ih);
      }
      return 1;
    }
    else
    {
      // TODO: this is leaving no selection
      winTreeClearSelection(ih);
      iupAttribSetStr(ih, "_IUPWINTREE_FIRSTSELITEM", NULL);
      /* continue to process the new single selection ... */
    }
  }
  else if(msg_info->code == TVN_ITEMEXPANDING)
  {
    /* mouse click by user: click on the "+" sign or double click on the selected node */
    TVITEM item;
    winTreeItemData* itemData;
    NMTREEVIEW* tree_info = (NMTREEVIEW*)msg_info;
    HTREEITEM hItem = winTreeFindNodePointed(ih);
    SendMessage(ih->handle, TVM_SELECTITEM, TVGN_CARET, (LPARAM)hItem);

    item.hItem = hItem;
    item.mask = TVIF_HANDLE | TVIF_PARAM;
    SendMessage(ih->handle, TVM_GETITEM, 0, (LPARAM)(LPTVITEM)&item);
    itemData = (winTreeItemData*)item.lParam;

    if (tree_info->action == TVE_EXPAND)
      item.iSelectedImage = item.iImage = (itemData->image_expanded!=-1)? itemData->image_expanded: (int)ih->data->def_image_expanded;
    else
      item.iSelectedImage = item.iImage = (itemData->image!=-1)? itemData->image: (int)ih->data->def_image_collapsed;

    item.mask = TVIF_HANDLE | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
    SendMessage(ih->handle, TVM_SETITEM, 0, (LPARAM)(LPTVITEM)&item);

    winTreeBranchLeaf_CB(ih);
    /* continue to process the expanding ... */
  }
  else if(msg_info->code == NM_RCLICK)
  {
    HTREEITEM hItem = winTreeFindNodePointed(ih);
    SendMessage(ih->handle, TVM_SELECTITEM, TVGN_CARET, (LPARAM)hItem);

    winTreeRightClick_CB(ih);
    return 1;
  }
  else if (msg_info->code == NM_CUSTOMDRAW)
  {
    NMTVCUSTOMDRAW *customdraw = (NMTVCUSTOMDRAW*)msg_info;

    if (customdraw->nmcd.dwDrawStage == CDDS_PREPAINT)
    {
      *result = CDRF_NOTIFYITEMDRAW;
      return 1;
    }
 
    if (customdraw->nmcd.dwDrawStage == CDDS_ITEMPREPAINT)
    {
      TVITEM item;
      winTreeItemData* itemData;
      HTREEITEM hItem = (HTREEITEM) customdraw->nmcd.dwItemSpec;

      item.hItem = hItem;
      item.mask = TVIF_HANDLE | TVIF_PARAM;
      SendMessage(ih->handle, TVM_GETITEM, 0, (LPARAM)(LPTVITEM)&item);
      itemData = (winTreeItemData*)item.lParam;

      customdraw->clrText = itemData->color;
      *result = CDRF_NOTIFYSUBITEMDRAW;

      if (itemData->hFont)
      {
        SelectObject(customdraw->nmcd.hdc, itemData->hFont);
        *result |= CDRF_NEWFONT;
      }

      return 1;

    }
  }

  (void)result;
  return 0;  /* allow the default processsing */
}

static int winTreeConvertXYToPos(Ihandle* ih, int x, int y)
{
  TVHITTESTINFO info;
  HTREEITEM hItem;
  info.pt.x = x;
  info.pt.y = y;
  hItem = (HTREEITEM)SendMessage(ih->handle, TVM_HITTEST, 0, (LPARAM)&info);
  if (hItem)
  {
    HTREEITEM hItemRoot = (HTREEITEM)SendMessage(ih->handle, TVM_GETNEXTITEM, TVGN_ROOT, 0);
    ih->data->id_control = -1;
    winTreeFindNodeFromID(ih, hItemRoot, hItem);
    return ih->data->id_control;
  }
  return -1;
}


/*******************************************************************************************/

static void winTreeUnMapMethod(Ihandle* ih)
{
  Iarray* bmp_array;
  HIMAGELIST image_list;

  HTREEITEM itemRoot = (HTREEITEM)SendMessage(ih->handle, TVM_GETNEXTITEM, TVGN_ROOT, 0);
  winTreeDelNodeData(ih, itemRoot);

  image_list = (HIMAGELIST)SendMessage(ih->handle, TVM_GETIMAGELIST, TVSIL_NORMAL, 0);
  if (image_list)
    ImageList_Destroy(image_list);

  bmp_array = (Iarray*)iupAttribGet(ih, "_IUPWIN_BMPARRAY");
  if (bmp_array)
    iupArrayDestroy(bmp_array);

  iupdrvBaseUnMapMethod(ih);
}

static int winTreeMapMethod(Ihandle* ih)
{
  DWORD dwStyle = WS_CHILD | WS_BORDER | TVS_SHOWSELALWAYS;

  /* TVS_EDITLABELS can be set only on the Tree View creation */
  if (ih->data->show_rename)
    dwStyle |= TVS_EDITLABELS;

  if (!iupAttribGetInt(ih, "HIDELINES"))
    dwStyle |= TVS_HASLINES;

  if (!iupAttribGetInt(ih, "HIDEBUTTONS"))
    dwStyle |= TVS_HASBUTTONS;

  if (iupStrBoolean(iupAttribGetStr(ih, "CANFOCUS")))
    dwStyle |= WS_TABSTOP;

  if (!ih->parent)
    return IUP_ERROR;

  if (!iupwinCreateWindowEx(ih, WC_TREEVIEW, 0, dwStyle))
    return IUP_ERROR;

  IupSetCallback(ih, "_IUPWIN_CTRLPROC_CB", (Icallback)winTreeProc);
  IupSetCallback(ih, "_IUPWIN_NOTIFY_CB",   (Icallback)winTreeWmNotify);

  /* Force background update before setting the images */
  {
    char* value = iupAttribGet(ih, "BGCOLOR");
    if (value)
    {
      winTreeSetBgColorAttrib(ih, value);
      iupAttribSetStr(ih, "BGCOLOR", NULL);
    }
  }

  /* Initialize the default images */
  ih->data->def_image_leaf = (void*)winTreeGetImageIndex(ih, "IMGLEAF", "IMAGELEAF");
  ih->data->def_image_collapsed = (void*)winTreeGetImageIndex(ih, "IMGCOLLAPSED", "IMAGEBRANCHCOLLAPSED");
  ih->data->def_image_expanded = (void*)winTreeGetImageIndex(ih, "IMGEXPANDED", "IMAGEBRANCHEXPANDED");

  /* Add the Root Node */
  winTreeAddRootNode(ih);

  /* configure for DRAG&DROP of files */
  if (IupGetCallback(ih, "DROPFILES_CB"))
    iupAttribSetStr(ih, "DRAGDROP", "YES");

  IupSetCallback(ih, "_IUP_XY2POS_CB", (Icallback)winTreeConvertXYToPos);

  return IUP_NOERROR;
}

void iupdrvTreeInitClass(Iclass* ic)
{
  /* Driver Dependent Class functions */
  ic->Map = winTreeMapMethod;
  ic->UnMap = winTreeUnMapMethod;

  /* Visual */
  iupClassRegisterAttribute(ic, "BGCOLOR", winTreeGetBgColorAttrib, winTreeSetBgColorAttrib, IUPAF_SAMEASSYSTEM, "TXTBGCOLOR", IUPAF_DEFAULT);
  iupClassRegisterAttribute(ic, "FGCOLOR", NULL, winTreeSetFgColorAttrib, IUPAF_SAMEASSYSTEM, "TXTFGCOLOR", IUPAF_DEFAULT);

  /* IupTree Attributes - GENERAL */
  iupClassRegisterAttribute(ic, "EXPANDALL",  NULL, winTreeSetExpandAllAttrib, NULL, "NO", IUPAF_WRITEONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "SHOWDRAGDROP", NULL, NULL, NULL, "NO", IUPAF_DEFAULT);
  iupClassRegisterAttribute(ic, "INDENTATION", winTreeGetIndentationAttrib, winTreeSetIndentationAttrib, NULL, NULL, IUPAF_DEFAULT);
  iupClassRegisterAttribute(ic, "COUNT", winTreeGetCountAttrib, NULL, NULL, NULL, IUPAF_NO_DEFAULTVALUE|IUPAF_READONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "DRAGDROP", NULL, iupwinSetDragDropAttrib, NULL, NULL, IUPAF_NO_INHERIT);

  /* IupTree Attributes - IMAGES */
  iupClassRegisterAttributeId(ic, "IMAGE", NULL, winTreeSetImageAttrib, IUPAF_WRITEONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "IMAGEBRANCHEXPANDED", NULL, winTreeSetImageExpandedAttrib, IUPAF_WRITEONLY|IUPAF_NO_INHERIT);
  
  iupClassRegisterAttribute(ic, "IMAGELEAF",            NULL, winTreeSetImageLeafAttrib, IUPAF_SAMEASSYSTEM, "IMGLEAF", IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "IMAGEBRANCHCOLLAPSED", NULL, winTreeSetImageBranchCollapsedAttrib, IUPAF_SAMEASSYSTEM, "IMGCOLLAPSED", IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "IMAGEBRANCHEXPANDED",  NULL, winTreeSetImageBranchExpandedAttrib, IUPAF_SAMEASSYSTEM, "IMGEXPANDED", IUPAF_NO_INHERIT);

  /* IupTree Attributes - NODES */
  iupClassRegisterAttributeId(ic, "STATE",  winTreeGetStateAttrib,  winTreeSetStateAttrib, IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "DEPTH",  winTreeGetDepthAttrib,  NULL, IUPAF_READONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "KIND",   winTreeGetKindAttrib,   NULL, IUPAF_READONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "PARENT", winTreeGetParentAttrib, NULL, IUPAF_READONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "NAME",   winTreeGetTitleAttrib,   winTreeSetTitleAttrib, IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "TITLE",   winTreeGetTitleAttrib,   winTreeSetTitleAttrib, IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "CHILDCOUNT",   winTreeGetChildCountAttrib,   NULL, IUPAF_READONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "USERDATA",   winTreeGetUserDataAttrib,   winTreeSetUserDataAttrib, IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "COLOR",  winTreeGetColorAttrib,  winTreeSetColorAttrib, IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "TITLEFONT", winTreeGetTitleFontAttrib, winTreeSetTitleFontAttrib, IUPAF_NO_INHERIT);

  /* IupTree Attributes - MARKS */
  iupClassRegisterAttributeId(ic, "MARKED",   winTreeGetMarkedAttrib,   winTreeSetMarkedAttrib,   IUPAF_NO_INHERIT);
  iupClassRegisterAttribute  (ic, "MARK",    NULL,    winTreeSetMarkAttrib,    NULL, NULL, IUPAF_WRITEONLY|IUPAF_NO_INHERIT);
  
  iupClassRegisterAttribute  (ic, "VALUE",    winTreeGetValueAttrib,    winTreeSetValueAttrib,    NULL, NULL, IUPAF_NO_DEFAULTVALUE|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute  (ic, "STARTING", NULL, winTreeSetStartingAttrib, NULL, NULL, IUPAF_NO_DEFAULTVALUE|IUPAF_NO_INHERIT);

  /* IupTree Attributes - ACTION */
  iupClassRegisterAttributeId(ic, "DELNODE", NULL, winTreeSetDelNodeAttrib, IUPAF_WRITEONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "RENAME",  NULL, winTreeSetRenameAttrib,  IUPAF_NO_INHERIT);

  if (!iupwin_comctl32ver6)  /* Used by iupdrvImageCreateImage */
    iupClassRegisterAttribute(ic, "FLAT_ALPHA", NULL, NULL, IUPAF_SAMEASSYSTEM, "YES", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
}
