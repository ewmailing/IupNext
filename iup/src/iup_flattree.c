/** \file
* \brief FlatTree Control
*
* See Copyright Notice in "iup.h"
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

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
#include "iup_childtree.h"

/* Kinds of node */
#define IFLATTREE_BRANCH        0
#define IFLATTREE_LEAF          1

#define IFLATTREE_UP        0
#define IFLATTREE_DOWN      1

#define IFLATTREE_MARK_SINGLE 0
#define IFLATTREE_MARK_MULTIPLE 1

#define IFLATTREE_TOGGLE_MARGIN 2
#define IFLATTREE_TOGGLE_BORDER 1
#define IFLATTREE_TOGGLE_SPACE  2

/* Orientation */
enum { IFLATTREE_EXPANDED, IFLATTREE_COLLAPSED };

typedef struct _iFlatTreeNode 
{
  /* attributes */
  char* title;
  char* image;
  char* image_expanded;
  char *fgColor;
  char *bgColor;
  char *font;
  int selected;
  int kind;
  int expand;
  int toggle_visible;
  int toggle_value;
  void* userdata;

  /* aux */
  int y, height, width;

  struct _iFlatTreeNode *parent;
  struct _iFlatTreeNode *firstChild;
  struct _iFlatTreeNode *brother;
} iFlatTreeNode;

struct _IcontrolData
{
  iupCanvas canvas;  /* from IupCanvas (must reserve it) */

  iFlatTreeNode *node;  /* tree of nodes */
  Iarray *node_cache;   /* array of nodes */

  /* aux */
  //int dragover_pos, dragged_pos;
  int has_focus, focus_id;
  int level_gap;

  /* attributes */
  int add_expanded;
  int indentation;
  //int expand_all;
  int show_rename;
  int horiz_padding, vert_padding;  /* button margin */
  int spacing, icon_spacing, img_position;        /* used when both text and image are displayed */
  int horiz_alignment, vert_alignment;
  int border_width;
  int mark_mode, mark_start;
  int show_dragdrop;
  //int lastAddNode;
  int show_toggle;
  //int toggle;
  //char* image_leaf;
  //char* image_branch_expanded;
  //char* image_branch_collapsed;
  //int last_clock, min_clock;
  //int toggle_size;
  //int image_plusminus_height;
};


/********************** Additional images **********************/


static Ihandle* load_image_plus(void)
{
  unsigned char imgdata[] = {
    186, 187, 188, 145, 145, 145, 145, 145, 145, 145, 145, 145, 145, 145, 145, 145, 145, 145, 145, 145, 145, 145, 145, 145, 186, 187, 188,
    145, 145, 145, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 145, 145, 145,
    145, 145, 145, 252, 252, 252, 252, 252, 252, 252, 252, 252, 41, 66, 114, 252, 252, 252, 252, 252, 252, 252, 252, 252, 145, 145, 145,
    145, 145, 145, 250, 251, 251, 250, 251, 251, 250, 251, 251, 41, 66, 114, 250, 251, 251, 250, 251, 251, 250, 251, 251, 145, 145, 145,
    145, 145, 145, 250, 251, 251, 75, 99, 167, 75, 99, 167, 75, 99, 167, 75, 99, 167, 75, 99, 167, 250, 251, 251, 145, 145, 145,
    145, 145, 145, 237, 237, 236, 237, 237, 236, 237, 237, 236, 41, 66, 114, 237, 237, 236, 237, 237, 236, 237, 237, 236, 145, 145, 145,
    145, 145, 145, 227, 227, 227, 227, 227, 227, 227, 227, 227, 41, 66, 114, 227, 227, 227, 227, 227, 227, 227, 227, 227, 145, 145, 145,
    145, 145, 145, 227, 227, 227, 227, 227, 227, 227, 227, 227, 227, 227, 227, 227, 227, 227, 227, 227, 227, 227, 227, 227, 145, 145, 145,
    186, 187, 188, 145, 145, 145, 145, 145, 145, 145, 145, 145, 145, 145, 145, 145, 145, 145, 145, 145, 145, 145, 145, 145, 186, 187, 188 };

  Ihandle* image = IupImageRGB(9, 9, imgdata);
  return image;
}

static Ihandle* load_image_minus(void)
{
  unsigned char imgdata[] = {
    186, 187, 188, 145, 145, 145, 145, 145, 145, 145, 145, 145, 145, 145, 145, 145, 145, 145, 145, 145, 145, 145, 145, 145, 186, 187, 188,
    145, 145, 145, 250, 251, 251, 250, 251, 251, 250, 251, 251, 250, 251, 251, 250, 251, 251, 250, 251, 251, 250, 251, 251, 145, 145, 145,
    145, 145, 145, 250, 251, 251, 250, 251, 251, 250, 251, 251, 250, 251, 251, 250, 251, 251, 250, 251, 251, 250, 251, 251, 145, 145, 145,
    145, 145, 145, 250, 251, 251, 250, 251, 251, 250, 251, 251, 250, 251, 251, 250, 251, 251, 250, 251, 251, 250, 251, 251, 145, 145, 145,
    145, 145, 145, 237, 237, 236, 75, 99, 167, 75, 99, 167, 75, 99, 167, 75, 99, 167, 75, 99, 167, 237, 237, 236, 145, 145, 145,
    145, 145, 145, 237, 237, 236, 237, 237, 236, 237, 237, 236, 237, 237, 236, 237, 237, 236, 237, 237, 236, 237, 237, 236, 145, 145, 145,
    145, 145, 145, 227, 227, 227, 227, 227, 227, 227, 227, 227, 227, 227, 227, 227, 227, 227, 227, 227, 227, 227, 227, 227, 145, 145, 145,
    145, 145, 145, 227, 227, 227, 227, 227, 227, 227, 227, 227, 227, 227, 227, 227, 227, 227, 227, 227, 227, 227, 227, 227, 145, 145, 145,
    186, 187, 188, 145, 145, 145, 145, 145, 145, 145, 145, 145, 145, 145, 145, 145, 145, 145, 145, 145, 145, 145, 145, 145, 186, 187, 188 };

  Ihandle* image = IupImageRGB(9, 9, imgdata);
  return image;
}

static void iFlatTreeInitializeImages(void)
{
  IupSetHandle("IMGPLUS", load_image_plus());
  IupSetHandle("IMGMINUS", load_image_minus());
}


/********************** Utilities **********************/


static void iFlatTreeSetNodeDrawFont(Ihandle* ih, const char* font)
{
  if (font)
    iupAttribSetStr(ih, "DRAWFONT", font);
  else
  {
    font = IupGetAttribute(ih, "FONT");
    iupAttribSetStr(ih, "DRAWFONT", font);
  }
}

static char *iFlatTreeGetNodeImage(iFlatTreeNode *node)
{
  char* image;

  if (node->kind == IFLATTREE_LEAF)
  {
    if (node->image)
      image = node->image;
    else
      image = "IMGLEAF";
  }
  else
  {
    if (node->expand == IFLATTREE_COLLAPSED)
    {
      if (node->image)
        image = node->image;
      else
        image = "IMGCOLLAPSED";
    }
    else
      image = "IMGEXPANDED";
  }

  return image;
}


/********************** Node Hierarchy **********************/


static int iFlatTreeNodeIsVisible(iFlatTreeNode *node);

static void iFlatTreeUpdateNodeCacheRec(Ihandle *ih, iFlatTreeNode *node, int *next_posy, int level)
{
  iFlatTreeNode *brother = node;

  while (brother)
  {
    iFlatTreeNode **data = iupArrayInc(ih->data->node_cache);
    char *image = iFlatTreeGetNodeImage(brother);
    int w, h;

    int count = iupArrayCount(ih->data->node_cache);
    data[count - 1] = brother;

    //TODO optimize
    iFlatTreeSetNodeDrawFont(ih, brother->font);    
    iupFlatDrawGetIconSize(ih, ih->data->img_position, ih->data->icon_spacing, ih->data->horiz_padding, ih->data->vert_padding,  image, brother->title, &w, &h, 0);

    //TODO use this values instead of calling iupFlatDrawGetIconSize
    brother->height = h;
    brother->width = w + ((level + 1) * ih->data->level_gap);

    if (iFlatTreeNodeIsVisible(brother))  //TODO optimize
    {
      brother->y = *next_posy;
      *next_posy += (h + ih->data->spacing);
    }
    else
      brother->y = -1;

    if (brother->kind == IFLATTREE_BRANCH && brother->firstChild)
      iFlatTreeUpdateNodeCacheRec(ih, brother->firstChild, next_posy, level + 1);

    brother = brother->brother;
  }
}

static void iFlatTreeRebuildCache(Ihandle *ih)     // TODO: reduce the number of calls to iFlatTreeRebuildCache
{
  int count = iupArrayCount(ih->data->node_cache);
  int next_posy = 0;

  iupArrayRemove(ih->data->node_cache, 0, count);

  iFlatTreeUpdateNodeCacheRec(ih, ih->data->node, &next_posy, 0);
}

static iFlatTreeNode *iFlatTreeGetNode(Ihandle *ih, int id)
{
  iFlatTreeNode **nodes = iupArrayGetData(ih->data->node_cache);
  int count = iupArrayCount(ih->data->node_cache);

  if (id >= 0 && id < count)
    return nodes[id];
  else if (id == IUP_INVALID_ID && count != 0)
    return nodes[ih->data->focus_id];
  else
    return NULL;
}

static iFlatTreeNode *iFlatTreeGetNodeFromString(Ihandle* ih, const char* name_id)
{
  int id = IUP_INVALID_ID;
  iupStrToInt(name_id, &id);
  return iFlatTreeGetNode(ih, id);
}

static int iFlatTreeNodeIsVisible(iFlatTreeNode *node)
{
  iFlatTreeNode *parent = node->parent;

  while (parent)
  {
    if (parent->expand == IFLATTREE_COLLAPSED)
      return 0;
    parent = parent->parent;
  }

  return 1;
}

static int iFlatTreeGetNextVisibleNodeId(Ihandle *ih, int id)
{
  int count = iupArrayCount(ih->data->node_cache);
  iFlatTreeNode **nodes = iupArrayGetData(ih->data->node_cache);
  int i;

  for (i = id + 1; i < count; i++)
  {
    if (iFlatTreeNodeIsVisible(nodes[i]))
      return i;
  }

  return id;
}

static int iFlatTreeGetPreviousVisibleNodeId(Ihandle *ih, int id)
{
  iFlatTreeNode **nodes = iupArrayGetData(ih->data->node_cache);
  int i;

  for (i = id - 1; i >= 0; i--)
  {
    if (iFlatTreeNodeIsVisible(nodes[i]))
      return i;
  }

  return id;
}

static int iFlatTreeGetNodeDepth(iFlatTreeNode *node)
{
  int depth = 0;
  iFlatTreeNode *parent = node;

  while (parent)
  {
    depth++;
    parent = parent->parent;
  }

  return depth;
}

static int iFlatTreeGetVisibleNodesRec(iFlatTreeNode *node)
{
  int count = 0;

  while (node)
  {
    count++;

    if (node->kind == IFLATTREE_BRANCH && node->firstChild && node->expand == IFLATTREE_EXPANDED)
      count += iFlatTreeGetVisibleNodesRec(node->firstChild);

    node = node->brother;
  }

  return count;
}

static int iFlatTreeGetVisibleNodesCount(Ihandle *ih)
{
  return iFlatTreeGetVisibleNodesRec(ih->data->node);
}

static int iFlatTreeGetChildCountRec(iFlatTreeNode *node)
{
  int count = 0;
  iFlatTreeNode *child = node->firstChild;

  while (child)
  {
    if (child->firstChild)
      count += iFlatTreeGetChildCountRec(child->firstChild);
    count++;
    child = child->brother;
  }

  return count;
}

//static iFlatTreeNode *iFlatTreeCloneNode(iFlatTreeNode *node, int copyUserdata)
//{
//  iFlatTreeNode *newNode = (iFlatTreeNode*)malloc(sizeof(iFlatTreeNode));

//  memset(newNode, 0, sizeof(iFlatTreeNode));

//  newNode->title = iupStrDup(node->title);
//  newNode->image = iupStrDup(node->image);
//  newNode->image_expanded = iupStrDup(node->image_expanded);
//  newNode->bgColor = iupStrDup(node->bgColor);
//  newNode->fgColor = iupStrDup(node->fgColor);
//  newNode->font = iupStrDup(node->font);
//  newNode->kind = node->kind;
//  newNode->expand = node->expand;
//  if (copyUserdata)
//    newNode->userdata = node->userdata;
//  newNode->selected = 0;

//  if (node->firstChild)
//  {
//    iFlatTreeNode *brother = node->firstChild;
//    iFlatTreeNode *lastNode = NULL;

//    while (brother)
//    {
//      iFlatTreeNode *newNewNode = iFlatTreeCloneNode(brother, copyUserdata);
//      if (!lastNode)
//        newNode->firstChild = newNewNode;
//      else
//        lastNode->brother = newNewNode;
//      newNewNode->parent = newNode;
//      lastNode = newNewNode;
//      brother = brother->brother;
//    }
//  }

//  return newNode;
//}

//static iFlatTreeNode *iFlatTreeCopyNode(Ihandle *ih, int srcId, int dstId)
//{
//  iFlatTreeNode *srcNode = iFlatTreeGetNode(ih, srcId);
//  iFlatTreeNode *newNode = iFlatTreeCloneNode(srcNode, 0);
//  iFlatTreeNode *dstNode = iFlatTreeGetNode(ih, dstId);

//  if (!dstNode)
//    return NULL;

//  if (dstNode->kind == IFLATTREE_BRANCH && dstNode->expand == IFLATTREE_EXPANDED)
//  {
//    /* copy as first child of expanded branch */
//    newNode->parent = dstNode;
//    newNode->brother = dstNode->firstChild;
//    dstNode->firstChild = newNode;
//  }
//  else
//  {
//    newNode->parent = dstNode->parent;
//    newNode->brother = dstNode->brother;
//    dstNode->brother = newNode;
//  }

//  iFlatTreeRebuildCache(ih);

//  return newNode;
//}

//static void iFlatTreeUnlinkNode(Ihandle *ih, iFlatTreeNode* node, int onlyChildren)
//{
//  iFlatTreeNode *parent = node->parent;

//  if (node->kind == IFLATTREE_LEAF || (node->kind == IFLATTREE_BRANCH && !onlyChildren))
//  {
//    if (node->parent && node == node->parent->firstChild)
//      parent->firstChild = node->brother;
//    else
//    {
//      iFlatTreeNode *brother = NULL;
//      if (node->parent)
//        brother = node->parent->firstChild;
//      else
//        brother = ih->data->node;
//      while (brother->brother && brother->brother != node)
//        brother = brother->brother;
//      brother->brother = node->brother;
//    }
//  }
//  else
//    node->firstChild = NULL;
//}

//static void iFlatTreeDelNode(iFlatTreeNode *node, int onlyChildren)
//{
//  iFlatTreeNode *brother = node->firstChild;

//  while (brother)
//  {
//    iFlatTreeNode *nextNode = brother->brother;
//    iFlatTreeDelNode(brother, 0);
//    brother = nextNode;
//  }

//  if (onlyChildren)
//    return;

//  if (node->title)
//    free(node->title);

//  if (node->image)
//    free(node->image);

//  if (node->fgColor)
//    free(node->fgColor);

//  if (node->bgColor)
//    free(node->bgColor);

//  if (node->font)
//    free(node->font);

//  free(node);

//  node = NULL;
//}

//static void iFlatTreeRemoveNode(Ihandle *ih, iFlatTreeNode* node, int onlyChildren)
//{
//  iFlatTreeUnlinkNode(ih, node, onlyChildren);

//  iFlatTreeRebuildCache(ih);

//  if (node == ih->data->node)
//    ih->data->node = NULL;

//  iFlatTreeDelNode(node, onlyChildren);
//}

//static iFlatTreeNode *iFlatTreeMoveNode(Ihandle *ih, int srcId, int dstId)
//{
//  iFlatTreeNode *srcNode = iFlatTreeGetNode(ih, srcId);
//  iFlatTreeNode *dstNode = iFlatTreeGetNode(ih, dstId);

//  if (!dstNode)
//    return NULL;

//  iFlatTreeUnlinkNode(ih, srcNode, 0);

//  if (dstNode->kind == IFLATTREE_BRANCH && dstNode->expand == IFLATTREE_EXPANDED)
//  {
//    /* copy as first child of expanded branch */
//    srcNode->parent = dstNode;
//    srcNode->brother = dstNode->firstChild;
//    dstNode->firstChild = srcNode;
//  }
//  else
//  {
//    srcNode->parent = dstNode->parent;
//    srcNode->brother = dstNode->brother;
//    dstNode->brother = srcNode;
//  }

//  return srcNode;
//}

//static void iFlatTreeAddNode(Ihandle* ih, int id, int kind, const char* title)
//{
//  iFlatTreeNode *nodeRef = iFlatTreeGetNode(ih, id);
//  char str[1024];

//  iFlatTreeNode *nodeNew;

//  if (!nodeRef && id != -1)
//    return;

//  if (!title || *title == 0)
//    sprintf(str, "TITLE%d", id);
//  else
//    strcpy(str, title);

//  if (id == -1)
//    nodeRef = iFlatTreeGetNode(ih, 0);

//  nodeNew = (iFlatTreeNode *)malloc(sizeof(iFlatTreeNode));

//  memset(nodeNew, 0, sizeof(iFlatTreeNode));

//  nodeNew->title = iupStrDup(str);
//  nodeNew->kind = kind;
//  nodeNew->toggle_visible = 1;

//  if (nodeRef->kind == IFLATTREE_LEAF)
//  {
//    iFlatTreeNode *tmp = nodeRef->brother;
//    nodeRef->brother = nodeNew;
//    nodeNew->brother = tmp;
//    nodeNew->firstChild = NULL;
//    nodeNew->parent = nodeRef->parent;
//    if (nodeNew->kind == IFLATTREE_BRANCH)
//      nodeNew->expand = (iupAttribGetBoolean(ih, "ADDEXPANDED")) ? IFLATTREE_EXPANDED : IFLATTREE_COLLAPSED;
//  }
//  else
//  {
//    iFlatTreeNode *tmp = nodeRef->firstChild;
//    nodeNew->parent = nodeRef;
//    nodeNew->brother = tmp;
//    nodeNew->firstChild = NULL;
//    nodeRef->firstChild = nodeNew;
//    if (nodeNew->kind == IFLATTREE_BRANCH)
//      nodeNew->expand = (iupAttribGetBoolean(ih, "ADDEXPANDED")) ? IFLATTREE_EXPANDED : IFLATTREE_COLLAPSED;
//  }
//}

//static void iFlatTreeInsertNode(Ihandle* ih, int id, int kind, const char* title)
//{
//  iFlatTreeNode *nodeRef = iFlatTreeGetNode(ih, id);
//  iFlatTreeNode *nodeNew, *tmp;
//  char str[1024];

//  if (!nodeRef && id != -1)
//    return;

//  if (!nodeRef)
//    nodeRef = iFlatTreeGetNode(ih, 0);

//  tmp = nodeRef->brother;

//  if (!title || *title == 0)
//    sprintf(str, "TITLE%d", id);
//  else
//    strcpy(str, title);

//  nodeNew = (iFlatTreeNode *)malloc(sizeof(iFlatTreeNode));

//  memset(nodeNew, 0, sizeof(iFlatTreeNode));

//  nodeNew->title = iupStrDup(str);
//  nodeNew->kind = kind;
//  nodeNew->toggle_visible = 1;

//  nodeRef->brother = nodeNew;
//  nodeNew->brother = tmp;
//  nodeNew->parent = nodeRef->parent;
//  if (nodeNew->kind == IFLATTREE_BRANCH)
//    nodeNew->expand = (iupAttribGetBoolean(ih, "ADDEXPANDED")) ? IFLATTREE_EXPANDED : IFLATTREE_COLLAPSED;
//}

//static void iFlatTreeGetTitlePos(Ihandle *ih, int id, const char *image, int *img_w, int *img_h, int *txt_x, int *txt_y);

//static int iFlatTreeRenameNode(Ihandle* ih)
//{
//  if (ih->data->show_rename && ih->data->has_focus)
//  {
//    iFlatTreeNode *nodeFocus = iFlatTreeGetNode(ih, ih->data->focus_id);
//    int pos, img_w, img_h, txt_x, txt_y, width, height;
//    Ihandle* text = NULL;
//    Ihandle* firstChild = ih->firstchild;
//    char title[1024];
//    char *image = iFlatTreeGetNodeImage(nodeFocus);

//    if (!nodeFocus->title || *(nodeFocus->title) == 0)
//      strcpy(title, "XXXXX");
//    else
//    {
//      strcpy(title, nodeFocus->title);
//      strcat(title, "X");

//    }

//    while (firstChild)
//    {
//      if (iupStrEqual(firstChild->iclass->name, "text"))
//      {
//        text = firstChild;
//        break;
//      }
//      firstChild = firstChild->brother;
//    }

//    if (!text)
//      return 1;

//    iFlatTreeGetTitlePos(ih, ih->data->focus_id, image, &img_w, &img_h, &txt_x, &txt_y);
//    iFlatTreeSetNodeDrawFont(ih, nodeFocus->font);
//    iupFlatDrawGetIconSize(ih, ih->data->img_position, ih->data->icon_spacing, ih->data->horiz_padding, ih->data->vert_padding,
//      image, title, &width, &height, 0);

//    if (image && ih->data->img_position == 0)
//      txt_x += ih->data->icon_spacing + ih->data->horiz_padding;

//    text->x = txt_x;
//    text->y = txt_y;

//    if (ih->data->show_toggle && nodeFocus->toggle_visible)
//      text->x += ih->data->toggle_size;

//    text->currentwidth = width - img_w + 1;
//    text->currentheight = height;

//    iupClassObjectLayoutUpdate(text);

//    IupSetAttribute(text, "ALIGMENT", "ALEFT");
//    IupSetStrAttribute(text, "PADDING", iupAttribGetStr(ih, "PADDING"));
//    IupSetAttribute(text, "FONT", nodeFocus->font);
//    IupSetAttribute(text, "VISIBLE", "YES");
//    IupSetAttribute(text, "ACTIVE", "YES");
//    IupSetAttribute(text, "VALUE", nodeFocus->title);
//    IupSetFocus(text);

//    pos = IupConvertXYToPos(text, txt_x, txt_y);
//    IupSetInt(text, "CARETPOS", pos);
//  }
//  return 0;
//}

//static iFlatTreeNode *iFlatTreeGetNode(Ihandle *ih, int id);

//static void iFllatTreeUpdateText(Ihandle *text, int x, int y, int w, int h)
//{
//  if (text->x == x && text->y == y && text->currentwidth > w && text->currentheight > h)
//    return;

//  text->x = x;
//  text->y = y;

//  text->currentwidth = w;
//  text->currentheight = h;

//  iupClassObjectLayoutUpdate(text);
//}

//static int iFlatTreeGetNodeLevel(Ihandle *ih, iFlatTreeNode *node)
//{
//  int level = -1;
//  iFlatTreeNode *parent = node;

//  while (parent)
//  {
//    level++;
//    parent = parent->parent;
//    if (!IupGetInt(ih, "ADDROOT") && parent == ih->data->node)
//      break;
//  }

//  return level;
//}

//static int iFlatTreeConvertIdToY(Ihandle *ih, int id, int *h);

//static void iFlatTreeGetTitlePos(Ihandle *ih, int id, const char *image, int *img_w, int *img_h, int *txt_x, int *txt_y)
//{
//  int total_h = iFlatTreeConvertIdToY(ih, id, NULL);
//  int posx = IupGetInt(ih, "POSX");
//  int posy = IupGetInt(ih, "POSY");
//  iFlatTreeNode *node = iFlatTreeGetNode(ih, id);
//  int level = iFlatTreeGetNodeLevel(ih, node);
//  int border_width = ih->data->border_width;

//  *img_h = 0;
//  *img_w = 0;

//  if (image)
//    iupImageGetInfo(image, img_w, img_h, NULL);

//  *txt_x = -posx + border_width;
//  *txt_y = -posy + border_width;

//  *txt_x += level * ih->data->level_gap + *img_h;
//  *txt_y += total_h;
//}

//static int iFlatTreeNodeIsVisible(iFlatTreeNode *node);
//static int iFlatTreeFindNodeId(Iarray *node_cache, iFlatTreeNode* node);

//static int iFlatTreeConvertIdToPos(Ihandle *ih, int id)
//{
//  int count = iupArrayCount(ih->data->node_cache);
//  iFlatTreeNode **nodes = iupArrayGetData(ih->data->node_cache);
//  int i, pos = 0;

//  if (id > count)
//    return -1;

//  for (i = 0; i < count; i++)
//  {
//    if (!iFlatTreeNodeIsVisible(nodes[i]))
//      continue;
//    if (iFlatTreeFindNodeId(ih->data->node_cache, nodes[i]) == id)
//      break;
//    pos++;
//  }

//  return pos;
//}

//static int iFlatTreeConvertPosToId(Ihandle *ih, int pos)
//{
//  int count = iupArrayCount(ih->data->node_cache);
//  iFlatTreeNode **nodes = iupArrayGetData(ih->data->node_cache);
//  int i, p = 0;

//  if (pos > count)
//    return -1;

//  for (i = 0; i < count; i++)
//  {
//    if (!iFlatTreeNodeIsVisible(nodes[i]))
//      continue;
//    if (p == pos)
//      break;
//    p++;
//  }

//  return i;
//}

//static int iFlatTreePageLastItemId(Ihandle *ih, int py);
//static int iFlatTreePageUpFromFocus(Ihandle *ih);

//static int iFlatTreeFocusPageUp(Ihandle *ih)
//{
//  int node_height;
//  int new_focus_id = ih->data->focus_id;
//  int total_y = iFlatTreeConvertIdToY(ih, ih->data->focus_id, &node_height);

//  int posy = IupGetInt(ih, "POSY");
//  int dy = IupGetInt(ih, "DY");

//  if ((total_y + node_height) > posy && (total_y + node_height) < (posy + dy))
//    new_focus_id = iFlatTreePageLastItemId(ih, posy);

//  if (new_focus_id == ih->data->focus_id)
//    new_focus_id = iFlatTreePageUpFromFocus(ih);

//  ih->data->focus_id = new_focus_id;

//  return new_focus_id;
//}

//static int iFlatTreePageDownFromFocus(Ihandle *ih);

//static int iFlatTreeFocusPageDown(Ihandle *ih)
//{
//  int node_height;
//  int new_focus_id = ih->data->focus_id;
//  int total_y = iFlatTreeConvertIdToY(ih, ih->data->focus_id, &node_height);

//  int posy = IupGetInt(ih, "POSY");
//  int dy = IupGetInt(ih, "DY");

//  if ((total_y + node_height) > posy && (total_y + node_height) < (posy + dy))
//    new_focus_id = iFlatTreePageLastItemId(ih, posy);

//  if (new_focus_id == ih->data->focus_id)
//    new_focus_id = iFlatTreePageDownFromFocus(ih);

//  ih->data->focus_id = new_focus_id;

//  return new_focus_id;
//}

static void iFlatTreeInvertSelection(Ihandle* ih)
{
  int i;
  iFlatTreeNode **nodes = iupArrayGetData(ih->data->node_cache);
  int count = iupArrayCount(ih->data->node_cache);
  for (i = 0; i < count; i++)
    nodes[i]->selected = !(nodes[i]->selected);
}

static void iFlatTreeSelectAll(Ihandle* ih)
{
  int i;
  iFlatTreeNode **nodes = iupArrayGetData(ih->data->node_cache);
  int count = iupArrayCount(ih->data->node_cache);
  for (i = 0; i < count; i++)
    nodes[i]->selected = 1;
}

static void iFlatTreeClearAllSelectionExcept(Ihandle* ih, iFlatTreeNode *nodeExcept)
{
  int i;
  iFlatTreeNode **nodes = iupArrayGetData(ih->data->node_cache);
  int count = iupArrayCount(ih->data->node_cache);
  for (i = 0; i < count; i++)
  {
    if (nodes[i] != nodeExcept)
      nodes[i]->selected = 0;
  }
}

static int iFlatTreeFindNodeId(Iarray *node_cache, iFlatTreeNode* node)
{
  iFlatTreeNode **nodes = iupArrayGetData(node_cache);
  int count = iupArrayCount(node_cache);
  int i;

  for (i = 0; i < count; i++)
  {
    if (nodes[i] == node)
      return i;
  }
  return -1;
}

static void iFlatTreeSelectRange(Ihandle* ih, int id1, int id2)
{
  int i;
  iFlatTreeNode **nodes = iupArrayGetData(ih->data->node_cache);
  int count = iupArrayCount(ih->data->node_cache);

  if (id1 > id2)
  {
    int tmp = id1;
    id1 = id2;
    id2 = tmp;
  }

  for (i = 0; i < count; i++)
  {
    if (i >= id1 && i <= id2)
      nodes[i]->selected = 1;
  }
}

//static void iFlatTreeRemoveMarkedNodes(Ihandle *ih, iFlatTreeNode *curNode)
//{
//  while (curNode)
//  {
//    if (curNode->selected)
//    {
//      iFlatTreeNode *nextNode = curNode->brother;
//      iFlatTreeRemoveNode(ih, curNode, 0);
//      curNode = nextNode;
//    }
//    else if (curNode->kind == IFLATTREE_BRANCH)
//    {
//      iFlatTreeRemoveMarkedNodes(ih, curNode->firstChild);
//      curNode = curNode->brother;
//    }
//    else
//      curNode = curNode->brother;
//  }
//}

static int iFlatTreeGetScrollbar(Ihandle* ih)
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

static int iFlatTreeGetScrollbarSize(Ihandle* ih)
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

//static int iFlatTreeConvertXYToPos(Ihandle* ih, int x, int y)
//{
//  int count = iupArrayCount(ih->data->node_cache);
//  iFlatTreeNode **nodes = iupArrayGetData(ih->data->node_cache);
//  int posy = IupGetInt(ih, "POSY");
//  int pos = 0, py, dy = y + posy, i;

//  if (dy < 0)
//    return -1;

//  for (i = 0; i < count; i++)
//  {
//    if (!iFlatTreeNodeIsVisible(nodes[i]))
//      continue;

//    py = nodes[i]->y;

//    if (dy >= py && dy < (py + nodes[i]->height))
//      return pos;

//    py += nodes[i]->height;
//    pos++;
//  }

//  (void)x;
//  return -1;
//}

//static int iFlatTreeConvertIdToY(Ihandle *ih, int id, int *h)
//{
//  int count = iupArrayCount(ih->data->node_cache);
//  iFlatTreeNode **nodes = iupArrayGetData(ih->data->node_cache);
//  int i;
//  int w, temp_h;
//  int total_h = 0;

//  if (id > count)
//    return -1;

//  for (i = 0; i < count; i++)
//  {
//    const char *image = iFlatTreeGetNodeImage(nodes[i]);
//    if (!iFlatTreeNodeIsVisible(nodes[i]))
//      continue;

//    iFlatTreeSetNodeDrawFont(ih, nodes[i]->font);
//    iupFlatDrawGetIconSize(ih, ih->data->img_position, ih->data->icon_spacing, ih->data->horiz_padding, ih->data->vert_padding,
//      image, nodes[i]->title, &w, &temp_h, 0);

//    if (h)
//      *h = temp_h;

//    if (i == id)
//      break;

//    total_h += temp_h + ih->data->spacing;

//  }

//  return total_h;
//}

//static int iFlatTreePageLastItemId(Ihandle *ih, int py)
//{
//  int count = iupArrayCount(ih->data->node_cache);
//  iFlatTreeNode **nodes = iupArrayGetData(ih->data->node_cache);
//  int posy = IupGetInt(ih, "POSY");
//  int dy = IupGetInt(ih, "DY");
//  int i, id = -1, last_id = -1;
//  int total_y = posy + dy;
//  int total_h, h;

//  for (i = 0; i < count; i++)
//  {
//    if (!iFlatTreeNodeIsVisible(nodes[i]))
//      continue;

//    id = iFlatTreeFindNodeId(ih->data->node_cache, nodes[i]);

//    total_h = iFlatTreeConvertIdToY(ih, id, &h);

//    if (total_h + h > total_y)
//      break;

//    last_id = id;
//  }

//  return last_id;
//}

//static int iFlatTreePageDownFromFocus(Ihandle *ih)
//{
//  int count = iupArrayCount(ih->data->node_cache);
//  iFlatTreeNode **nodes = iupArrayGetData(ih->data->node_cache);
//  int focus_y = iFlatTreeConvertIdToY(ih, ih->data->focus_id, NULL);
//  int dy = IupGetInt(ih, "DY");
//  int i, last_id = -1;
//  int total_h = 0, w, h;

//  for (i = ih->data->focus_id; i < count; i++)
//  {
//    const char *image = NULL;

//    if (!iFlatTreeNodeIsVisible(nodes[i]))
//      continue;

//    image = iFlatTreeGetNodeImage(nodes[i]);
//    iFlatTreeSetNodeDrawFont(ih, nodes[i]->font);
//    iupFlatDrawGetIconSize(ih, ih->data->img_position, ih->data->icon_spacing, ih->data->horiz_padding, ih->data->vert_padding,
//      nodes[i]->image, nodes[i]->title, &w, &h, 0);

//    total_h += (h + ih->data->spacing);

//    if (total_h + h > focus_y + dy)
//      break;

//    last_id = i;
//  }

//  return last_id;
//}

//static int iFlatTreePageUpFromFocus(Ihandle *ih)
//{
//  iFlatTreeNode **nodes = iupArrayGetData(ih->data->node_cache);
//  int focus_y = iFlatTreeConvertIdToY(ih, ih->data->focus_id, NULL);
//  int dy = IupGetInt(ih, "DY");
//  int i, last_id = ih->data->focus_id;
//  int total_h = focus_y, w, h;

//  for (i = ih->data->focus_id - 1; i >= 0; i--)
//  {
//    const char *image = NULL;
//    if (!iFlatTreeNodeIsVisible(nodes[i]))
//      continue;

//    image = iFlatTreeGetNodeImage(nodes[i]);
//    iFlatTreeSetNodeDrawFont(ih, nodes[i]->font);
//    iupFlatDrawGetIconSize(ih, ih->data->img_position, ih->data->icon_spacing, ih->data->horiz_padding, ih->data->vert_padding,
//      image, nodes[i]->title, &w, &h, 0);

//    total_h -= (h + ih->data->spacing);

//    if (total_h - h < focus_y - dy)
//      break;

//    last_id = i;
//  }

//  return last_id;
//}

static void iFlatTreeGetViewSize(Ihandle *ih, int *view_width, int *view_height)
{
  int count = iupArrayCount(ih->data->node_cache);
  iFlatTreeNode **nodes = iupArrayGetData(ih->data->node_cache);
  int i;
  int total_h = 0;
  int max_w = 0;

  for (i = 0; i < count; i++)
  {
    if (!iFlatTreeNodeIsVisible(nodes[i]))
      continue;

    total_h += nodes[i]->height + ih->data->spacing;
    max_w = (nodes[i]->width > max_w) ? nodes[i]->width : max_w;
  }

  *view_width = max_w;
  *view_height = total_h;
}

static void iFlatTreeUpdateScrollBar(Ihandle *ih)
{
  int canvas_width = ih->currentwidth;
  int canvas_height = ih->currentheight;
  int sb, view_width, view_height;

  if (iupAttribGetBoolean(ih, "BORDER")) /* native border around scrollbars */
  {
    canvas_width -= 2;
    canvas_height -= 2;
  }

  canvas_width -= 2 * ih->data->border_width;
  canvas_height -= 2 * ih->data->border_width;

  iFlatTreeGetViewSize(ih, &view_width, &view_height);

  if (ih->data->show_dragdrop || iupAttribGetBoolean(ih, "DRAGDROPTREE"))
    view_height += ih->data->level_gap / 2; /* additional space for drop area */

  sb = iFlatTreeGetScrollbar(ih);
  if (sb)
  {
    int sb_size = iFlatTreeGetScrollbarSize(ih);
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

    IupSetfAttribute(ih, "LINEY", "%d", ih->data->level_gap);
  }
  else
  {
    IupSetAttribute(ih, "XMAX", "0");
    IupSetAttribute(ih, "YMAX", "0");

    IupSetAttribute(ih, "DX", "0");
    IupSetAttribute(ih, "DY", "0");
  }
}


/**************************************  Internal Callbacks  *****************************************/


//static void iFlatTreeDrawToggle(Ihandle *ih, IdrawCanvas* dc, iFlatTreeNode *node, int x, int y, int h)
//{
//  char* fgcolor = iupAttribGetStr(ih, "FGCOLOR");
//  char* bgcolor = iupAttribGet(ih, "BGCOLOR");
//  int active = IupGetInt(ih, "ACTIVE");  /* native implementation */

//  if (ih->data->toggle_size)
//  {
//    int check_xmin = x + IFLATTREE_TOGGLE_MARGIN;
//    int check_ymin = y + IFLATTREE_TOGGLE_MARGIN + ((h - ih->data->toggle_size) / 2);
//    int check_size = ih->data->toggle_size - 2 * IFLATTREE_TOGGLE_MARGIN;

//    /* check border */
//    iupFlatDrawBorder(dc, check_xmin, check_xmin + check_size,
//      check_ymin, check_ymin + check_size,
//      IFLATTREE_TOGGLE_BORDER, fgcolor, bgcolor, active);

//    /* check mark */
//    if (node->toggle_value)
//    {
//      if (node->toggle_value == -1)
//        iupFlatDrawBox(dc, check_xmin + IFLATTREE_TOGGLE_SPACE + IFLATTREE_TOGGLE_BORDER, check_xmin + check_size - IFLATTREE_TOGGLE_SPACE - IFLATTREE_TOGGLE_BORDER,
//          check_ymin + IFLATTREE_TOGGLE_SPACE + IFLATTREE_TOGGLE_BORDER, check_ymin + check_size - IFLATTREE_TOGGLE_SPACE - IFLATTREE_TOGGLE_BORDER,
//          fgcolor, bgcolor, active);
//      else
//        iupFlatDrawCheckMark(dc, check_xmin + IFLATTREE_TOGGLE_SPACE + IFLATTREE_TOGGLE_BORDER, check_xmin + check_size - IFLATTREE_TOGGLE_SPACE - IFLATTREE_TOGGLE_BORDER,
//          check_ymin + IFLATTREE_TOGGLE_SPACE + IFLATTREE_TOGGLE_BORDER, check_ymin + check_size - IFLATTREE_TOGGLE_SPACE - IFLATTREE_TOGGLE_BORDER,
//          fgcolor, bgcolor, active);
//    }
//  }

//}

//static int iFlatTreeDrawNodes(Ihandle *ih, IdrawCanvas* dc, iFlatTreeNode *node, int x, int y, char *foreground_color, char *background_color, int make_inactive, int active,
//  int text_flags, int focus_feedback, int width, int border_width, int level, int *pos)
//{
//  iFlatTreeNode *brother = node;
//  int x_pos;
//  int first_y;

//  x_pos = x + (level * ih->data->level_gap);

//  while (brother)
//  {
//    char *fgcolor = (brother->fgColor) ? brother->fgColor : foreground_color;
//    char *bgcolor = (brother->bgColor) ? brother->bgColor : background_color;
//    int w, h, img_w, img_h, txt_x, txt_y, toggle_gap = 0;
//    const char *image = iFlatTreeGetNodeImage(brother);

//    iFlatTreeGetTitlePos(ih, ih->data->focus_id, image, &img_w, &img_h, &txt_x, &txt_y);
//    iFlatTreeSetNodeDrawFont(ih, brother->font);
//    iupFlatDrawGetIconSize(ih, ih->data->img_position, ih->data->icon_spacing, ih->data->horiz_padding, ih->data->vert_padding,
//      image, brother->title, &w, &h, 0);

//    img_w += ih->data->icon_spacing + ih->data->horiz_padding;

//    if (brother->parent && brother == brother->parent->firstChild)
//      first_y = y;

//    if (level != 0)
//    {
//      int px1, py1, px2, py2;
//      px1 = (x_pos - (ih->data->level_gap / 2)) + 1;
//      py1 = y + h / 2;
//      px2 = x_pos;
//      py2 = py1;
//      iupdrvDrawLine(dc, px1, py1, px2, py2, iupDrawColor(0, 0, 0, 255), IUP_DRAW_STROKE_DOT, 1);
//      if (!brother->brother)
//      {
//        px1 = x_pos - (ih->data->level_gap / 2);
//        py1 = first_y;
//        px2 = px1;
//        py2 = (brother->brother) ? y + ih->data->level_gap : y + ih->data->level_gap / 2;
//        iupdrvDrawLine(dc, px1, py1, px2, py2, iupDrawColor(0, 0, 0, 255), IUP_DRAW_STROKE_DOT, 1);
//      }

//    }

//    if (ih->data->show_toggle && brother->toggle_visible)
//    {
//      iFlatTreeDrawToggle(ih, dc, brother, x_pos, y, h);
//      toggle_gap = ih->data->toggle_size;
//    }

//    iupFlatDrawBox(dc, x_pos + img_w + toggle_gap, x_pos + toggle_gap + w - 1, y, y + h - 1, bgcolor, bgcolor, 1);

//    iFlatTreeSetNodeDrawFont(ih, brother->font);

//    iupFlatDrawIcon(ih, dc, x_pos + toggle_gap, y, w, h,
//      ih->data->img_position, ih->data->icon_spacing, ih->data->horiz_alignment, ih->data->vert_alignment, ih->data->horiz_padding, ih->data->vert_padding,
//      image, make_inactive, brother->title, text_flags, 0, fgcolor, bgcolor, active);

//    if (brother->selected || ih->data->dragover_pos == *pos)
//    {
//      unsigned char red, green, blue;
//      char* hlcolor = iupAttribGetStr(ih, "HLCOLOR");
//      unsigned char a = (unsigned char)iupAttribGetInt(ih, "HLCOLORALPHA");
//      long selcolor;

//      if (ih->data->dragover_pos == *pos)
//        a = (2 * a) / 3;

//      iupStrToRGB(hlcolor, &red, &green, &blue);
//      selcolor = iupDrawColor(red, green, blue, a);

//      iupdrvDrawRectangle(dc, x_pos + img_w + toggle_gap, y, x_pos + toggle_gap + w - 1, y + h - 1, selcolor, IUP_DRAW_FILL, 1);
//    }

//    if (ih->data->has_focus && ih->data->focus_id == iFlatTreeFindNodeId(ih->data->node_cache, brother) && focus_feedback)
//      iupdrvDrawFocusRect(dc, x_pos + img_w + toggle_gap, y, x_pos + toggle_gap + w - border_width - 1, y + h - 1);

//    y += h + ih->data->spacing;

//    (*pos)++;

//    if (brother->kind == IFLATTREE_BRANCH && brother->expand == IFLATTREE_EXPANDED)
//      y = iFlatTreeDrawNodes(ih, dc, brother->firstChild, x, y, foreground_color, background_color, make_inactive, active, text_flags, focus_feedback, width, border_width, level + 1, pos);

//    brother = brother->brother;

//  }

//  return y;
//}

//static int iFlatTreeDrawPlusMinus(Ihandle *ih, IdrawCanvas* dc, iFlatTreeNode *node, char *bgcolor, int x, int y, int level)
//{
//  iFlatTreeNode *brother = node;

//  while (brother)
//  {
//    int w, h;
//    const char *image = iFlatTreeGetNodeImage(brother);

//    iFlatTreeSetNodeDrawFont(ih, brother->font);

//    iupFlatDrawGetIconSize(ih, ih->data->img_position, ih->data->icon_spacing, ih->data->horiz_padding, ih->data->vert_padding,
//      image, brother->title, &w, &h, 0);

//    if (brother->kind == IFLATTREE_BRANCH)
//    {
//      int px, py;
//      char *exp = (brother->expand == IFLATTREE_EXPANDED) ? "IMGMINUS" : "IMGPLUS";

//      py = y + ((h - ih->data->image_plusminus_height) / 2);
//      px = x + ((level - 1) * ih->data->level_gap) + 1 + ((ih->data->level_gap - ih->data->image_plusminus_height) / 2);

//      if (level > 0)
//        iupdrvDrawImage(dc, exp, 0, bgcolor, px, py, ih->data->image_plusminus_height, ih->data->image_plusminus_height);
//    }

//    y += h + ih->data->spacing;

//    if (brother->kind == IFLATTREE_BRANCH && brother->expand == IFLATTREE_EXPANDED)
//      y = iFlatTreeDrawPlusMinus(ih, dc, brother->firstChild, bgcolor, x, y, level + 1);

//    brother = brother->brother;

//  }

//  return y;
//}

//static int iFlatTreeRedraw_CB(Ihandle* ih)
//{
//  int text_flags = iupDrawGetTextFlags(ih, "TABSTEXTALIGNMENT", "TABSTEXTWRAP", "TABSTEXTELLIPSIS");
//  char* foreground_color = iupAttribGetStr(ih, "FGCOLOR");
//  char* background_color = iupAttribGetStr(ih, "BGCOLOR");
//  int posx = IupGetInt(ih, "POSX");
//  int posy = IupGetInt(ih, "POSY");
//  char* back_image = iupAttribGet(ih, "BACKIMAGE");
//  int x, y, make_inactive = 0;
//  int border_width = ih->data->border_width;
//  int active = IupGetInt(ih, "ACTIVE");  /* native implementation */
//  int focus_feedback = iupAttribGetBoolean(ih, "FOCUSFEEDBACK");
//  iFlatTreeNode *node;
//  int width, height, pos = 0;

//  IdrawCanvas* dc = iupdrvDrawCreateCanvas(ih);

//  iupdrvDrawGetSize(dc, &width, &height);

//  iupFlatDrawBox(dc, border_width, width - border_width - 1, border_width, height - border_width - 1, background_color, background_color, 1);

//  if (back_image)
//  {
//    int backimage_zoom = iupAttribGetBoolean(ih, "BACKIMAGEZOOM");
//    if (backimage_zoom)
//      iupdrvDrawImage(dc, back_image, 0, background_color, border_width, border_width, width - border_width, height - border_width);
//    else
//      iupdrvDrawImage(dc, back_image, 0, background_color, border_width, border_width, -1, -1);
//  }

//  if (!active)
//    make_inactive = 1;

//  x = -posx + border_width;
//  y = -posy + border_width;

//  node = ih->data->node;

//  iFlatTreeDrawNodes(ih, dc, node, x, y, foreground_color, background_color, make_inactive, active, text_flags, focus_feedback, width, border_width, 0, &pos);

//  iFlatTreeDrawPlusMinus(ih, dc, node, background_color, x, y, 0);

//  if (border_width)
//  {
//    char* bordercolor = iupAttribGetStr(ih, "BORDERCOLOR");
//    iupFlatDrawBorder(dc, 0, width - 1,
//      0, height - 1,
//      border_width, bordercolor, background_color, active);
//  }

//  iupdrvDrawFlush(dc);

//  iupdrvDrawKillCanvas(dc);

//  return IUP_DEFAULT;
//}

//static int iFlatTreeTextEditKILLFOCUS_CB(Ihandle* text)
//{
//  Ihandle* ih = text->parent;
//  IupSetAttribute(text, "VISIBLE", "NO");
//  IupSetAttribute(text, "ACTIVE", "NO");
//  IupUpdate(ih);
//  return IUP_DEFAULT;
//}

//static int iFlatTreeTextEditKANY_CB(Ihandle* text, int c)
//{
//  if (c == K_ESC || c == K_CR)
//  {
//    iFlatTreeTextEditKILLFOCUS_CB(text);
//    return IUP_IGNORE;  /* always ignore to avoid the defaultenter/defaultesc behavior from here */
//  }

//  return IUP_CONTINUE;
//}

//static int iFlatTreeTextEditKCR_CB(Ihandle* text, int c)
//{
//  Ihandle* ih = text->parent;

//  iFlatTreeNode *nodeFocus = iFlatTreeGetNode(ih, ih->data->focus_id);

//  if (nodeFocus->title)
//    free(nodeFocus->title);

//  nodeFocus->title = iupStrDup(IupGetAttribute(text, "VALUE"));

//  iFlatTreeTextEditKILLFOCUS_CB(text);
//  return IUP_DEFAULT;
//}

//static int iFlatTreeTextEditVALUECHANGED_CB(Ihandle* text)
//{
//  Ihandle* ih = text->parent;
//  iFlatTreeNode *nodeFocus = iFlatTreeGetNode(ih, ih->data->focus_id);
//  int img_w, img_h, txt_x, txt_y, width, height;
//  char val[1024];
//  const char *image = iFlatTreeGetNodeImage(nodeFocus);

//  strcpy(val, IupGetAttribute(text, "VALUE"));

//  iFlatTreeSetNodeDrawFont(ih, nodeFocus->font);
//  iFlatTreeGetTitlePos(ih, ih->data->focus_id, image, &img_w, &img_h, &txt_x, &txt_y);
//  iupFlatDrawGetIconSize(ih, ih->data->img_position, ih->data->icon_spacing, ih->data->horiz_padding, ih->data->vert_padding,
//    image, val, &width, &height, 0);

//  if (text->currentwidth > width)
//    return IUP_DEFAULT;

//  iFllatTreeUpdateText(text, text->x, text->y, width, height);

//  return IUP_DEFAULT;
//}

//static void iFlatTreeCallSelectionCallback(Ihandle* ih, IFnii cb, int id, int state)
//{
//  char *text;

//  if (id < 1 || !cb)
//    return;

//iFlatTreeNode *node = iFlatTreeGetNode(ih, id);
//if (node)
//  text = node->title;

//  if (cb(ih, id, state) == IUP_CLOSE)
//    IupExitLoop();
//}

//static void iFlatTreeSingleCallSelectionCb(Ihandle* ih, IFnsii cb, int id, int old_id)
//{
//  if (old_id != -1)
//  {
//    if (old_id != id)
//    {
//      iFlatTreeCallSelectionCallback(ih, cb, old_id, 0);
//      iFlatTreeCallSelectionCallback(ih, cb, id, 1);
//    }
//  }
//  else
//    iFlatTreeCallSelectionCallback(ih, cb, id, 1);
//}

//static void iFlatTreeMultipleCallActionCb(Ihandle* ih, IFnsii cb, IFns multi_cb, char* str, int count)
//{
//  int i;
//  int unchanged = 1;

//  if (multi_cb)
//  {
//    if (multi_cb(ih, str) == IUP_CLOSE)
//      IupExitLoop();
//  }
//  else
//  {
//    /* must simulate the click on each item */
//    for (i = 0; i < count; i++)
//    {
//      if (str[i] != 'x')
//      {
//        if (str[i] == '+')
//          iFlatTreeCallSelectionCallback(ih, cb, i, 1);
//        else
//          iFlatTreeCallSelectionCallback(ih, cb, i, 0);
//        unchanged = 0;
//      }
//    }
//  }
//}

//static void iFlatTreeSelectNode(Ihandle* ih, int id, int ctrlPressed, int shftPressed)
//{
//  iFlatTreeNode *node;
//  IFns sel_cb = (IFns)IupGetCallback(ih, "SELECTION_CB");
//  IFns multi_cb = (IFns)IupGetCallback(ih, "MULTISELECTION_CB");
//  int count = iupArrayCount(ih->data->node_cache);

//  ih->data->focus_id = id;

//  if (ih->data->mark_mode == IFLATTREE_MARK_MULTIPLE)
//  {
//    int i, start, end;
//    char* str;
//    char *val = iupAttribGet(ih, "_IUPFLATTREE_LASTSELECTED");
//    int last_id = (val) ? atoi(val) : 0;
//    if (id <= last_id)
//    {
//      start = id;
//      end = last_id;
//    }
//    else
//    {
//      start = last_id;
//      end = id;
//    }

//    str = malloc(count + 1);
//    memset(str, 'x', count); /* mark all as unchanged */
//    str[count] = 0;

//    if (!ctrlPressed)
//    {
//      /* un-select all */
//      for (i = 0; i < count; i++)
//      {
//        node = iFlatTreeGetNode(ih, i);
//        if (node->selected)
//        {
//          str[i] = '-';
//          node->selected = 0;
//        }
//      }
//    }

//    if (shftPressed)
//    {
//      /* select interval */
//      for (i = start; i <= end; i++)
//      {
//        node = iFlatTreeGetNode(ih, i);
//        if (!node->selected)
//        {
//          str[i] = '+';
//          node->selected = 1;
//        }
//      }
//    }
//    else
//    {
//      node = iFlatTreeGetNode(ih, id);

//      if (ctrlPressed)
//      {
//        /* toggle selection */
//        if (node->selected)
//        {
//          str[id] = '-';
//          node->selected = 0;
//        }
//        else
//        {
//          str[id] = '+';
//          node->selected = 1;
//        }
//      }
//      else
//      {
//        if (!node->selected)
//        {
//          str[id] = '+';
//          node->selected = 1;
//        }
//      }
//    }

//    if (multi_cb || sel_cb)
//      iFlatTreeMultipleCallActionCb(ih, sel_cb, multi_cb, str, count);

//    free(str);
//  }
//  else
//  {
//    int i, old_id = -1;

//    for (i = 0; i < count; i++)
//    {
//      node = iFlatTreeGetNode(ih, i);
//      if (!node->selected)
//        continue;
//      node->selected = 0;
//      old_id = i;
//      break;
//    }
//    node = iFlatTreeGetNode(ih, id);

//    node->selected = 1;

//    if (sel_cb)
//      iFlatTreeSingleCallSelectionCb(ih, sel_cb, id, old_id);
//  }

//  if (!shftPressed)
//    iupAttribSetInt(ih, "_IUPFLATTREE_LASTSELECTED", id);
//}

//static int iFlatTreeCallDragDropCb(Ihandle* ih, int drag_id, int drop_id, int is_ctrl, int is_shift)
//{
//  IFniiii cbDragDrop = (IFniiii)IupGetCallback(ih, "DRAGDROP_CB");

//  /* ignore a drop that will do nothing */
//  if (is_ctrl == 0 && (drag_id - 1 == drop_id || drag_id == drop_id))
//    return IUP_DEFAULT;
//  if (is_ctrl != 0 && drag_id == drop_id)
//    return IUP_DEFAULT;

//  drag_id++;
//  if (drop_id < 0)
//    drop_id = -1;
//  else
//    drop_id++;

//  if (cbDragDrop)
//    return cbDragDrop(ih, drag_id, drop_id, is_shift, is_ctrl);  /* starts at 1 */

//  return IUP_CONTINUE; /* allow to move/copy by default if callback not defined */
//}

//static int iFlatTreeHitToggle(Ihandle *ih, int x, int y, int id, int level)
//{
//  int h;
//  int py = iFlatTreeConvertIdToY(ih, id, &h) + ((h - ih->data->toggle_size) / 2);
//  int px = (level * ih->data->level_gap) + 1 + ((ih->data->level_gap - ih->data->toggle_size) / 2);
//  int posx = IupGetInt(ih, "POSX");
//  int posy = IupGetInt(ih, "POSY");

//  x += posx - ih->data->border_width;
//  y += posy - ih->data->border_width;

//  if (x > px && x < px + ih->data->toggle_size && y > py && y < py + ih->data->toggle_size)
//    return 1;

//  return 0;
//}

//static int iFlatTreeHitPlusMinus(Ihandle *ih, int x, int y, int id, int level)
//{
//  int h;
//  int py = iFlatTreeConvertIdToY(ih, id, &h) + ((h - ih->data->image_plusminus_height) / 2);
//  int px = ((level - 1) * ih->data->level_gap) + 1 + ((ih->data->level_gap - ih->data->image_plusminus_height) / 2);
//  int posx = IupGetInt(ih, "POSX");
//  int posy = IupGetInt(ih, "POSY");

//  x += posx - ih->data->border_width;
//  y += posy - ih->data->border_width;

//  if (x > px && x < px + ih->data->image_plusminus_height && y > py && y < py + ih->data->image_plusminus_height)
//    return 1;

//  return 0;
//}

//static int iFlatTreeButton_CB(Ihandle* ih, int button, int pressed, int x, int y, char* status)
//{
//  iFlatTreeNode *node;
//  IFniiiis button_cb = (IFniiiis)IupGetCallback(ih, "FLAT_BUTTON_CB");
//  char *image;
//  int pos = iFlatTreeConvertXYToPos(ih, x, y);
//  int id, level, width, height, xmin, xmax;
//  int toggle_gap = (ih->data->show_toggle) ? ih->data->toggle_size : 0;

//  if (button_cb)
//  {
//    if (button_cb(ih, button, pressed, x, y, status) == IUP_IGNORE)
//      return IUP_DEFAULT;
//  }

//  if (button == IUP_BUTTON1 && !pressed && ih->data->dragged_pos > 0)
//  {
//    if (pos == -1)
//    {
//      if (y < 0)
//        pos = 1;
//      else
//      {
//        int count = iFlatTreeGetVisibleNodesCount(ih);
//        pos = count;
//      }
//    }

//    if (iFlatTreeCallDragDropCb(ih, ih->data->dragged_pos, pos, iup_iscontrol(status), iup_isshift(status)) == IUP_CONTINUE)
//    {
//      iFlatTreeNode *droppedNode = NULL;
//      int srcId = iFlatTreeConvertPosToId(ih, ih->data->dragged_pos);
//      int destId = iFlatTreeConvertPosToId(ih, pos);

//      if (!iup_iscontrol(status))
//        droppedNode = iFlatTreeMoveNode(ih, srcId, destId);
//      else
//        droppedNode = iFlatTreeCopyNode(ih, srcId, destId);

//      if (!droppedNode)
//        return IUP_DEFAULT;

//      /* select the dropped item */
//      iFlatTreeSelectNode(ih, iFlatTreeFindNodeId(ih->data->node_cache, droppedNode), 0, 0); /* force no ctrl and no shift for selection */

//      iFlatTreeRebuildCache(ih);
//    }

//    ih->data->dragover_pos = -1;
//    ih->data->dragged_pos = -1;

//    iFlatTreeUpdateScrollBar(ih);
//    IupUpdate(ih);

//    return IUP_DEFAULT;
//  }

//  if (pos == -1)
//    return IUP_DEFAULT;

//  id = iFlatTreeConvertPosToId(ih, pos);
//  node = iFlatTreeGetNode(ih, id);
//  level = iFlatTreeGetNodeLevel(ih, node);
//  image = iFlatTreeGetNodeImage(node);
//  iFlatTreeSetNodeDrawFont(ih, node->font);
//  iupFlatDrawGetIconSize(ih, ih->data->img_position, ih->data->icon_spacing, ih->data->horiz_padding, ih->data->vert_padding,
//    image, node->title, &width, &height, 0);
//  xmin = (level * ih->data->level_gap) + toggle_gap;
//  xmax = xmin + width;

//  if (button == IUP_BUTTON3)
//  {
//    IFni cbRightClick = (IFni)IupGetCallback(ih, "RIGHTCLICK_CB");
//    if (cbRightClick)
//      cbRightClick(ih, id);
//    return IUP_DEFAULT;
//  }

//  if (button == IUP_BUTTON1 && pressed)
//  {
//    if (iup_isdouble(status) && x > xmin && x < xmax)
//    {
//      if (node->kind == IFLATTREE_LEAF)
//      {
//        IFnis dc_cb = (IFnis)IupGetCallback(ih, "EXECUTELEAF_CB");
//        if (dc_cb)
//        {
//          if (dc_cb(ih, id, node->title) == IUP_IGNORE)
//            return IUP_DEFAULT;
//        }
//      }
//      else
//      {
//        if (node->expand == IFLATTREE_EXPANDED)
//          node->expand = IFLATTREE_COLLAPSED;
//        else
//          node->expand = IFLATTREE_EXPANDED;
//      }
//      iFlatTreeRebuildCache(ih);
//    }
//    else if (ih->data->show_rename  && x > xmin && x < xmax)
//    {
//      int current_clock = clock();

//      if (id == ih->data->focus_id && (current_clock - ih->data->last_clock) > ih->data->min_clock)
//      {
//        IFni cb = (IFnnn)IupGetCallback(ih, "SHOWRENAME_CB");
//        if (cb)
//        {
//          if (cb(ih, id) == IUP_IGNORE)
//            return IUP_DEFAULT;
//        }
//        return iFlatTreeRenameNode(ih);
//      }

//      iFlatTreeSelectNode(ih, id, iup_iscontrol(status), iup_isshift(status));

//      if (ih->data->show_dragdrop)
//        ih->data->dragged_pos = pos;

//      ih->data->last_clock = clock();
//    }
//    else if (iFlatTreeHitPlusMinus(ih, x, y, id, level))
//    {
//      if (node->expand == IFLATTREE_EXPANDED)
//        node->expand = IFLATTREE_COLLAPSED;
//      else
//        node->expand = IFLATTREE_EXPANDED;
//    }
//    else if (iFlatTreeHitToggle(ih, x, y, id, level))
//    {
//      IFnis tv_cb = (IFnis)IupGetCallback(ih, "TOGGLEVALUE_CB");
//      int markWhenToggle = IupGetInt(ih, "MARKWHENTOGGLE");

//      if (node->toggle_value > 0)  /* was ON */
//      {
//        if (ih->data->show_toggle == 2)
//          node->toggle_value = -1;
//        else
//          node->toggle_value = 0;
//      }
//      else if (node->toggle_value == -1)
//        node->toggle_value = 0;
//      else  /* was OFF */
//        node->toggle_value = 1;

//      if (markWhenToggle)
//      {
//        if (ih->data->mark_mode == IFLATTREE_MARK_MULTIPLE)
//          node->selected = (node->toggle_value > 0) ? 1 : 0;
//        else
//        {
//          int i, count = iupArrayCount((ih->data->node_cache));

//          for (i = 0; i < count; i++)
//          {
//            iFlatTreeNode *i_node = iFlatTreeGetNode(ih, i);
//            if (!i_node->selected)
//              continue;
//            i_node->selected = 0;
//            break;
//          }
//          node->selected = 1;
//        }
//      }

//      if (tv_cb)
//      {
//        if (tv_cb(ih, id, node->toggle_value) == IUP_IGNORE)
//          return IUP_DEFAULT;
//      }
//    }
//    iFlatTreeRebuildCache(ih);
//  }

//  iFlatTreeUpdateScrollBar(ih);
//  IupUpdate(ih);

//  return IUP_DEFAULT;
//}

//static int iFlatTreeMotion_CB(Ihandle* ih, int x, int y, char* status)
//{
//  IFniis motion_cb = (IFniis)IupGetCallback(ih, "FLAT_MOTION_CB");
//  int pos;

//  iupFlatScrollBarMotionUpdate(ih, x, y);

//  if (motion_cb)
//  {
//    if (motion_cb(ih, x, y, status) == IUP_IGNORE)
//      return IUP_DEFAULT;
//  }

//  if (!iup_isbutton1(status) || ih->data->mark_mode == IFLATTREE_MARK_MULTIPLE || !ih->data->show_dragdrop)
//    return IUP_IGNORE;

//  pos = iFlatTreeConvertXYToPos(ih, x, y);
//  if (pos == -1)
//    return IUP_DEFAULT;

//  if (y < 0 || y > ih->currentheight)
//  {
//    /* scroll if dragging out of canvas */
//    int h;
//    int dy = IupGetInt(ih, "DY");
//    int id = iFlatTreeConvertPosToId(ih, pos);
//    int py = iFlatTreeConvertIdToY(ih, id, &h);
//    int posy = (y < 0) ? py : (py + h) - dy;
//    IupSetInt(ih, "POSY", posy);
//  }

//  if (ih->data->dragged_pos >= 0)
//    ih->data->dragover_pos = pos;

//  IupUpdate(ih);

//  return IUP_DEFAULT;
//}

static int iFlatTreeFocus_CB(Ihandle* ih, int focus)
{
  IFni cb = (IFni)IupGetCallback(ih, "FLAT_FOCUS_CB");
  if (cb)
  {
    if (cb(ih, focus) == IUP_IGNORE)
      return IUP_DEFAULT;
  }

  ih->data->has_focus = focus;

  IupUpdate(ih);

  return IUP_DEFAULT;
}

//static int iFlatTreeResize_CB(Ihandle* ih, int width, int height)
//{
//  (void)width;
//  (void)height;

//  iFlatTreeUpdateScrollBar(ih);

//  return IUP_DEFAULT;
//}

//static void iFlatTreeScrollFocusVisible(Ihandle* ih, int direction)
//{
//  int focus_y = iFlatTreeConvertIdToY(ih, ih->data->focus_id, NULL);
//  iFlatTreeNode *node = iFlatTreeGetNode(ih, ih->data->focus_id);
//  int posy = IupGetInt(ih, "POSY");
//  int dy = IupGetInt(ih, "DY");
//  int ymin = IupGetInt(ih, "YMIN");
//  int ymax = IupGetInt(ih, "YMAX");
//  int line_height, w;

//  if (dy >= (ymax - ymin))
//    return;

//  iFlatTreeSetNodeDrawFont(ih, node->font);
//  iupFlatDrawGetIconSize(ih, ih->data->img_position, ih->data->icon_spacing, ih->data->horiz_padding, ih->data->vert_padding,
//    node->image, node->title, &w, &line_height, 0);

//  if (focus_y > posy && (focus_y + line_height) < (posy + dy))
//  {
//    posy = focus_y + line_height - dy;
//    IupSetInt(ih, "POSY", posy);
//  }
//  else if (direction == IFLATTREE_DOWN)
//  {
//    posy += (focus_y - posy - dy + line_height);
//    IupSetInt(ih, "POSY", posy);
//  }
//  else
//  {
//    posy -= (posy - focus_y);
//    IupSetInt(ih, "POSY", posy);
//  }
//}

//static int iFlatTreeKCr_CB(Ihandle* ih)
//{
//  if (ih->data->has_focus)
//  {
//    if (ih->data->focus_id >= 0)
//    {
//      iFlatTreeNode *node = iFlatTreeGetNode(ih, ih->data->focus_id);

//      if (node->kind == IFLATTREE_BRANCH)
//      {
//        if (node->expand == IFLATTREE_EXPANDED)
//          node->expand = IFLATTREE_COLLAPSED;
//        else
//          node->expand = IFLATTREE_EXPANDED;
//      }
//      else
//      {
//        IFnis dc_cb = (IFnis)IupGetCallback(ih, "EXECUTELEAF_CB");
//        if (dc_cb)
//        {
//          if (dc_cb(ih, ih->data->focus_id, node->title) == IUP_IGNORE)
//            return IUP_DEFAULT;
//        }
//      }



//      iFlatTreeScrollFocusVisible(ih, IFLATTREE_DOWN);
//      iFlatTreeRebuildCache(ih);
//      IupUpdate(ih);
//    }
//  }
//  return IUP_DEFAULT;
//}

//static int iFlatTreeKUp_CB(Ihandle* ih)
//{
//  if (ih->data->has_focus)
//  {
//    if (ih->data->focus_id > 0)
//    {
//      int ctrltPressed = IupGetInt(NULL, "CONTROLKEY");
//      int shftPressed = IupGetInt(NULL, "SHIFTKEY");
//      int previousId = iFlatTreeGetPreviousVisibleNodeId(ih, ih->data->focus_id);

//      if (ctrltPressed)
//        ih->data->focus_id = previousId;
//      else
//        iFlatTreeSelectNode(ih, previousId, 0, shftPressed);

//      iFlatTreeScrollFocusVisible(ih, IFLATTREE_UP);
//      IupUpdate(ih);
//    }
//  }
//  return IUP_DEFAULT;
//}

//static int iFlatTreeKDown_CB(Ihandle* ih)
//{
//  int count = iupArrayCount(ih->data->node_cache);
//  if (ih->data->has_focus)
//  {
//    if (ih->data->focus_id < count - 1)
//    {
//      int ctrltPressed = IupGetInt(NULL, "CONTROLKEY");
//      int shftPressed = IupGetInt(NULL, "SHIFTKEY");
//      int nextId = iFlatTreeGetNextVisibleNodeId(ih, ih->data->focus_id);

//      if (ctrltPressed)
//        ih->data->focus_id = nextId;
//      else
//        iFlatTreeSelectNode(ih, nextId, 0, shftPressed);

//      iFlatTreeScrollFocusVisible(ih, IFLATTREE_DOWN);
//      IupUpdate(ih);
//    }
//  }
//  return IUP_DEFAULT;
//}

//static int iFlatTreeKcSpace_CB(Ihandle* ih)
//{
//  iFlatTreeNode *node = iFlatTreeGetNode(ih, ih->data->focus_id);
//  if (!node)
//    return IUP_IGNORE;

//  if (ih->data->mark_mode == IFLATTREE_MARK_SINGLE)
//    iFlatTreeClearAllSelectionExcept(ih, node);

//  node->selected = !node->selected;

//  return IUP_DEFAULT;
//}

//static int iFlatTreeKF2_CB(Ihandle* ih)
//{
//  iFlatTreeNode *node = iFlatTreeGetNode(ih, ih->data->focus_id);
//  if (!node)
//    return IUP_IGNORE;

//  iFlatTreeRenameNode(ih);

//  return IUP_DEFAULT;
//}

//static int iFlatTreeKHome_CB(Ihandle* ih)
//{
//  if (ih->data->has_focus)
//  {
//    int id = iFlatTreeConvertPosToId(ih, 0);

//    iFlatTreeSelectNode(ih, id, 0, 0);

//    iFlatTreeScrollFocusVisible(ih, IFLATTREE_UP);
//    IupUpdate(ih);
//  }
//  return IUP_DEFAULT;
//}

//static int iFlatTreeKEnd_CB(Ihandle* ih)
//{
//  int count = iFlatTreeGetVisibleNodesCount(ih);
//  if (ih->data->has_focus)
//  {
//    int id = iFlatTreeConvertPosToId(ih, count - 1);

//    iFlatTreeSelectNode(ih, id, 0, 0);

//    iFlatTreeScrollFocusVisible(ih, IFLATTREE_DOWN);
//    IupUpdate(ih);
//  }
//  return IUP_DEFAULT;
//}

//static int iFlatTreeKPgUp_CB(Ihandle* ih)
//{
//  if (ih->data->has_focus)
//  {
//    int id = iFlatTreeFocusPageUp(ih);

//    iFlatTreeSelectNode(ih, id, 0, 0);

//    iFlatTreeScrollFocusVisible(ih, IFLATTREE_UP);
//    IupUpdate(ih);
//  }
//  return IUP_DEFAULT;
//}

//static int iFlatTreeKPgDn_CB(Ihandle* ih)
//{
//  if (ih->data->has_focus)
//  {
//    int id = iFlatTreeFocusPageDown(ih);

//    iFlatTreeSelectNode(ih, id, 0, 0);

//    iFlatTreeScrollFocusVisible(ih, IFLATTREE_DOWN);
//    IupUpdate(ih);
//  }
//  return IUP_DEFAULT;
//}

//static int iFlatTreeScroll_CB(Ihandle *ih)
//{
//  IupUpdate(ih);

//  return IUP_DEFAULT;
//}

//static void iFlatTreeDragDropCopyNode(Ihandle* ih_src, Ihandle *ih, iFlatTreeNode *nodeSrc, iFlatTreeNode *nodeDst, int isControl)
//{
//  iFlatTreeNode *nodeNew;
//  int id_dst;

//  id_dst = iFlatTreeFindNodeId(ih->data->node_cache, nodeDst);

//  if (isControl)
//    nodeNew = iFlatTreeCloneNode(nodeSrc, 0);
//  else
//  {
//    iFlatTreeUnlinkNode(ih_src, nodeSrc, 0);
//    nodeNew = nodeSrc;
//  }

//  if (nodeDst->kind == IFLATTREE_BRANCH && nodeDst->expand == IFLATTREE_EXPANDED)
//  {
//    /* copy as first child of expanded branch */
//    nodeNew->parent = nodeDst;
//    nodeNew->brother = nodeDst->firstChild;
//    nodeDst->firstChild = nodeNew;
//  }
//  else
//  {
//    nodeNew->parent = nodeDst->parent;
//    nodeNew->brother = nodeDst->brother;
//    nodeDst->brother = nodeNew;
//  }
//}

//static int iFlatTreeDropData_CB(Ihandle *ih, char* type, void* data, int len, int x, int y)
//{
//  int pos = iFlatTreeConvertXYToPos(ih, x, y);
//  int is_ctrl = 0;
//  char key[5];

//  /* Data is not the pointer, it contains the pointer */
//  Ihandle* ih_source;
//  memcpy((void*)&ih_source, data, len);

//  /* A copy operation is enabled with the CTRL key pressed, or else a move operation will occur.
//     A move operation will be possible only if the attribute DRAGSOURCEMOVE is Yes.
//     When no key is pressed the default operation is copy when DRAGSOURCEMOVE=No and move when DRAGSOURCEMOVE=Yes. */
//  iupdrvGetKeyState(key);
//  if (key[1] == 'C')
//    is_ctrl = 1;

//  /* Here copy/move of multiple selection is not allowed,
//     only a single node and its children. */

//  int srcPos = iupAttribGetInt(ih_source, "_IUP_FLAT_TREE_SOURCEPOS");
//  iFlatTreeNode *itemDst, *itemSrc;

//  itemSrc = iFlatTreeGetNode(ih_source, srcPos);
//  if (!itemSrc)
//    return IUP_DEFAULT;

//  itemDst = iFlatTreeGetNode(ih, pos);
//  if (!itemDst)
//    return IUP_DEFAULT;

//  /* Copy the node and its children to the new position */
//  iFlatTreeDragDropCopyNode(ih_source, ih, itemSrc, itemDst, is_ctrl);

//  if (ih->handle)
//  {
//    iFlatTreeRebuildCache(ih);
//    iFlatTreeUpdateScrollBar(ih);
//    IupUpdate(ih);
//  }

//  if (ih_source->handle)
//  {
//    iFlatTreeRebuildCache(ih_source);
//    iFlatTreeUpdateScrollBar(ih_source);
//    IupUpdate(ih_source);
//  }

//  (void)type;
//  return IUP_DEFAULT;
//}

//static int iFlatTreeDragData_CB(Ihandle *ih, char* type, void *data, int len)
//{
//  int pos = iupAttribGetInt(ih, "_IUP_FLAT_TREE_SOURCEPOS");
//  if (pos < 1)
//    return IUP_DEFAULT;

//  IupSetAttributeId(ih, "MARKED", pos, "YES");

//  /* Copy source handle */
//  memcpy(data, (void*)&ih, len);

//  (void)type;
//  return IUP_DEFAULT;
//}

//static int iFlatTreeDragDataSize_CB(Ihandle* ih, char* type)
//{
//  (void)ih;
//  (void)type;
//  return sizeof(Ihandle*);
//}

//static int iFlatTreeDragBegin_CB(Ihandle* ih, int x, int y)
//{
//  int pos = iFlatTreeConvertXYToPos(ih, x, y);

//  if (ih->data->mark_mode == IFLATTREE_MARK_MULTIPLE)
//    return IUP_IGNORE;

//  iupAttribSetInt(ih, "_IUP_FLAT_TREE_SOURCEPOS", pos);
//  return IUP_DEFAULT;
//}

//static int iFlatTreeDragEnd_CB(Ihandle *ih, int del)
//{
//  iupAttribSetInt(ih, "_IUP_FLAT_TREE_SOURCEPOS", -1);
//  (void)del;
//  return IUP_DEFAULT;
//}


/*********************************  Attributes  ********************************/


static char* iFlatTreeGetAddExpandedAttrib(Ihandle* ih)
{
  return iupStrReturnBoolean(ih->data->add_expanded);
}

static int iFlatTreeSetAddExpandedAttrib(Ihandle* ih, const char* value)
{
  if (iupStrBoolean(value))
    ih->data->add_expanded = 1;
  else
    ih->data->add_expanded = 0;

  return 0;
}

static char* iFlatTreeGetIndentationAttrib(Ihandle* ih)
{
  return iupStrReturnInt(ih->data->indentation);
}

static int iFlatTreeSetIndentationAttrib(Ihandle* ih, const char* value)
{
  iupStrToInt(value, &ih->data->indentation);
  return 0;
}

static char* iFlatTreeGetShowToggleAttrib(Ihandle* ih)
{
  if (ih->data->show_toggle)
  {
    if (ih->data->show_toggle == 2)
      return "3STATE";
    else
      return "YES";
  }
  else
    return "NO";
}

static int iFlatTreeSetShowToggleAttrib(Ihandle* ih, const char* value)
{
  if (iupStrEqualNoCase(value, "3STATE"))
    ih->data->show_toggle = 2;
  else if (iupStrBoolean(value))
    ih->data->show_toggle = 1;
  else
    ih->data->show_toggle = 0;

  return 0;
}

static int iFlatTreeSetAlignmentAttrib(Ihandle* ih, const char* value)
{
  char value1[30], value2[30];

  iupStrToStrStr(value, value1, value2, ':');

  ih->data->horiz_alignment = iupFlatGetHorizontalAlignment(value1);
  ih->data->vert_alignment = iupFlatGetVerticalAlignment(value2);

  if (ih->handle)
    iupdrvRedrawNow(ih);

  return 1;
}

static char* iFlatTreeGetAlignmentAttrib(Ihandle *ih)
{
  char* horiz_align2str[3] = { "ALEFT", "ACENTER", "ARIGHT" };
  char* vert_align2str[3] = { "ATOP", "ACENTER", "ABOTTOM" };
  return iupStrReturnStrf("%s:%s", horiz_align2str[ih->data->horiz_alignment], vert_align2str[ih->data->vert_alignment]);
}

static int iFlatTreeSetSpacingAttrib(Ihandle* ih, const char* value)
{
  iupStrToInt(value, &ih->data->spacing);
  if (ih->handle)
    IupUpdate(ih);
  return 0;
}

static char* iFlatTreeGetSpacingAttrib(Ihandle* ih)
{
  return iupStrReturnInt(ih->data->spacing);
}

static char* iFlatTreeGetHasFocusAttrib(Ihandle* ih)
{
  return iupStrReturnBoolean(ih->data->has_focus);
}

static char* iFlatTreeGetStateAttrib(Ihandle* ih, int id)
{
  iFlatTreeNode *node = iFlatTreeGetNode(ih, id);
  if (!node)
    return NULL;

  if (node->kind == IFLATTREE_LEAF)
    return NULL;

  if (node->expand == IFLATTREE_EXPANDED)
    return "EXPANDED";
  else
    return "COLLAPSED";
}

static int iFlatTreeSetStateAttrib(Ihandle* ih, int id, const char* value)
{
  iFlatTreeNode *node = iFlatTreeGetNode(ih, id);
  if (!node)
    return 0;

  if (node->kind == IFLATTREE_LEAF)
    return 0;

  if (iupStrEqualNoCase(value, "EXPANDED"))
    node->expand = IFLATTREE_EXPANDED;
  else /* "HORIZONTAL" */
    node->expand = IFLATTREE_COLLAPSED;

  if (ih->handle)
  {
    iFlatTreeRebuildCache(ih);
    iFlatTreeUpdateScrollBar(ih);
    IupUpdate(ih);
  }

  return 0;
}

static char* iFlatTreeGetKindAttrib(Ihandle* ih, int id)
{
  iFlatTreeNode *node = iFlatTreeGetNode(ih, id);
  if (!node)
    return NULL;

  if (node->kind == IFLATTREE_BRANCH)
    return "BRANCH";
  else
    return "LEAF";
}

static char* iFlatTreeGetParentAttrib(Ihandle* ih, int id)
{
  iFlatTreeNode *node = iFlatTreeGetNode(ih, id);
  if (!node)
    return NULL;

  return iupStrReturnInt(iFlatTreeFindNodeId(ih->data->node_cache, node->parent));
}

static char* iFlatTreeGetNextAttrib(Ihandle* ih, int id)
{
  iFlatTreeNode *node = iFlatTreeGetNode(ih, id);
  if (!node)
    return NULL;

  return iupStrReturnInt(iFlatTreeFindNodeId(ih->data->node_cache, node->brother));
}

static char* iFlatTreeGetPreviousAttrib(Ihandle* ih, int id)
{
  iFlatTreeNode *node = iFlatTreeGetNode(ih, id);
  iFlatTreeNode *brother;
  if (!node)
    return NULL;

  brother = node->parent->firstChild;

  if (brother == node)
    return NULL;

  while (brother->brother != node)
    brother = brother->brother;

  return iupStrReturnInt(iFlatTreeFindNodeId(ih->data->node_cache, brother));
}

static char* iFlatTreeGetLastAttrib(Ihandle* ih, int id)
{
  iFlatTreeNode *nodeLast = NULL;
  iFlatTreeNode *node = iFlatTreeGetNode(ih, id);
  if (!node)
    return NULL;

  nodeLast = node;
  while (nodeLast->brother)
    nodeLast = nodeLast->brother;

  return iupStrReturnInt(iFlatTreeFindNodeId(ih->data->node_cache, nodeLast));
}

static char* iFlatTreeGetFirstAttrib(Ihandle* ih, int id)
{
  iFlatTreeNode *node = iFlatTreeGetNode(ih, id);
  if (!node)
    return NULL;

  if (!node->parent)
    return "0";

  return iupStrReturnInt(iFlatTreeFindNodeId(ih->data->node_cache, node->parent->firstChild));
}

static char* iFlatTreeGetTitleAttrib(Ihandle* ih, int id)
{
  iFlatTreeNode *node = iFlatTreeGetNode(ih, id);
  if (!node)
    return NULL;
  return iupStrReturnStr(node->title);
}

static int iFlatTreeSetTitleAttrib(Ihandle* ih, int id, const char* value)
{
  iFlatTreeNode *node = iFlatTreeGetNode(ih, id);
  if (!node)
    return 0;

  node->title = iupStrDup(value);

  if (ih->handle)
    IupUpdate(ih);

  return 0;
}

static char* iFlatTreeGetTitleFontAttrib(Ihandle* ih, int id)
{
  iFlatTreeNode *node = iFlatTreeGetNode(ih, id);
  if (!node)
    return NULL;

  return node->font;
}

static int iFlatTreeSetTitleFontAttrib(Ihandle* ih, int id, const char* value)
{
  iFlatTreeNode *node = iFlatTreeGetNode(ih, id);
  if (!node)
    return 0;

  if (node->font)
    free(node->font);
  node->font = iupStrDup(value);

  if (ih->handle)
  {
    IupUpdate(ih);
    iFlatTreeRebuildCache(ih);
    iFlatTreeUpdateScrollBar(ih);
  }

  return 0;
}

static char* iFlatTreeGetTitleFontStyleAttrib(Ihandle* ih, int id)
{
  int size = 0;
  int is_bold = 0,
    is_italic = 0,
    is_underline = 0,
    is_strikeout = 0;
  char typeface[1024];

  char* font = IupGetAttributeId(ih, "TITLEFONT", id);
  if (!font)
    font = IupGetAttribute(ih, "FONT");

  if (!iupGetFontInfo(font, typeface, &size, &is_bold, &is_italic, &is_underline, &is_strikeout))
    return NULL;

  return iupStrReturnStrf("%s%s%s%s", is_bold ? "Bold " : "", is_italic ? "Italic " : "", is_underline ? "Underline " : "", is_strikeout ? "Strikeout " : "");
}

static int iFlatTreeSetTitleFontSizeAttrib(Ihandle* ih, int id, const char* value)
{
  int size = 0;
  int is_bold = 0,
    is_italic = 0,
    is_underline = 0,
    is_strikeout = 0;
  char typeface[1024];
  char* font;

  if (!value)
    return 0;

  font = IupGetAttributeId(ih, "TITLEFONT", id);
  if (!font)
    font = IupGetAttribute(ih, "FONT");

  if (!iupGetFontInfo(font, typeface, &size, &is_bold, &is_italic, &is_underline, &is_strikeout))
    return 0;

  IupSetfAttributeId(ih, "TITLEFONT", id, "%s, %s%s%s%s %s", typeface, is_bold ? "Bold " : "", is_italic ? "Italic " : "", is_underline ? "Underline " : "", is_strikeout ? "Strikeout " : "", value);

  return 0;
}

static char* iFlatTreeGetTitleFontSizeAttrib(Ihandle* ih, int id)
{
  int size = 0;
  int is_bold = 0,
    is_italic = 0,
    is_underline = 0,
    is_strikeout = 0;
  char typeface[1024];

  char* font = IupGetAttributeId(ih, "TITLEFONT", id);
  if (!font)
    font = IupGetAttribute(ih, "FONT");

  if (!iupGetFontInfo(font, typeface, &size, &is_bold, &is_italic, &is_underline, &is_strikeout))
    return NULL;

  return iupStrReturnInt(size);
}

static int iFlatTreeSetTitleFontStyleAttrib(Ihandle* ih, int id, const char* value)
{
  int size = 0;
  int is_bold = 0,
    is_italic = 0,
    is_underline = 0,
    is_strikeout = 0;
  char typeface[1024];
  char* font;

  if (!value)
    return 0;

  font = IupGetAttributeId(ih, "TITLEFONT", id);
  if (!font)
    font = IupGetAttribute(ih, "FONT");

  if (!iupGetFontInfo(font, typeface, &size, &is_bold, &is_italic, &is_underline, &is_strikeout))
    return 0;

  IupSetfAttributeId(ih, "TITLEFONT", id, "%s, %s %d", typeface, value, size);

  return 0;
}

static char* iFlatTreeGetToggleValueAttrib(Ihandle* ih, int id)
{
  iFlatTreeNode *node;

  if (!ih->data->show_toggle)
    return NULL;

  node = iFlatTreeGetNode(ih, id);
  if (!node)
    return NULL;

  if (!node->toggle_visible)
    return NULL;

  return iupStrReturnChecked(node->toggle_value);
}

static int iFlatTreeSetToggleValueAttrib(Ihandle* ih, int id, const char* value)
{
  iFlatTreeNode *node;

  if (!ih->data->show_toggle)
    return 0;

  node = iFlatTreeGetNode(ih, id);
  if (!node)
    return 0;

  if (!node->toggle_visible)
    return 0;

  if (ih->data->show_toggle == 2 && iupStrEqualNoCase(value, "NOTDEF"))
    node->toggle_value = -1;  /* indeterminate, inconsistent */
  else if (iupStrEqualNoCase(value, "ON"))
    node->toggle_value = 1;
  else
    node->toggle_value = 0;

  if (ih->handle)
    IupUpdate(ih);

  return 0;
}

static char* iFlatTreeGetToggleVisibleAttrib(Ihandle* ih, int id)
{
  iFlatTreeNode *node;

  if (!ih->data->show_toggle)
    return NULL;

  node = iFlatTreeGetNode(ih, id);
  if (!node)
    return NULL;

  return iupStrReturnBoolean(node->toggle_visible);
}

static int iFlatTreeSetToggleVisibleAttrib(Ihandle* ih, int id, const char* value)
{
  iFlatTreeNode *node;

  if (!ih->data->show_toggle)
    return 0;

  node = iFlatTreeGetNode(ih, id);
  if (!node)
    return 0;

  node->toggle_visible = iupStrBoolean(value);

  if (ih->handle)
    IupUpdate(ih);

  return 0;
}

static char* iFlatTreeGetUserDataAttrib(Ihandle* ih, int id)
{
  iFlatTreeNode *node = (iFlatTreeNode *)iFlatTreeGetNode(ih, id);
  if (!node)
    return NULL;

  return node->userdata;
}

static int iFlatTreeSetUserDataAttrib(Ihandle* ih, int id, const char* value)
{
  iFlatTreeNode *node = (iFlatTreeNode *)iFlatTreeGetNode(ih, id);
  if (!node)
    return 0;

  node->userdata = (void*)value;
  return 0;
}

//static int iFlatTreeSetRenameAttrib(Ihandle* ih, const char* value)
//{
//  iFlatTreeRenameNode(ih);
//  (void)value;
//  return 0;
//}

//static int iFlatTreeSetAddLeafAttrib(Ihandle* ih, int pos, const char* value)
//{
//  int count = iupArrayCount(ih->data->node_cache);

//  if (pos > count)
//    return 0;

//  if (value)
//  {
//    iFlatTreeAddNode(ih, pos, IFLATTREE_LEAF, value);

//    iFlatTreeRebuildCache(ih);
//  }

//  ih->data->lastAddNode = pos + 1;

//  if (ih->handle)
//  {
//    iFlatTreeUpdateScrollBar(ih);
//    iFlatTreeRebuildCache(ih);
//    IupUpdate(ih);
//  }

//  return 0;
//}

//static int iFlatTreeSetInsertLeafAttrib(Ihandle* ih, int pos, const char* value)
//{
//  int count = iupArrayCount(ih->data->node_cache);

//  if (pos > count)
//    return 0;

//  if (value)
//  {
//    iFlatTreeInsertNode(ih, pos, IFLATTREE_LEAF, value);

//    iFlatTreeRebuildCache(ih);
//  }

//  ih->data->lastAddNode = pos + 1;

//  if (ih->handle)
//  {
//    iFlatTreeUpdateScrollBar(ih);
//    iFlatTreeRebuildCache(ih);
//    IupUpdate(ih);
//  }

//  return 0;
//}

//static int iFlatTreeSetAddBranchAttrib(Ihandle* ih, int pos, const char* value)
//{
//  int count = iupArrayCount(ih->data->node_cache);

//  if (pos > count)
//    return 0;

//  if (value)
//  {
//    iFlatTreeAddNode(ih, pos, IFLATTREE_BRANCH, value);

//    iFlatTreeRebuildCache(ih);
//  }

//  ih->data->lastAddNode = pos + 1;

//  if (ih->handle)
//  {
//    iFlatTreeUpdateScrollBar(ih);
//    iFlatTreeRebuildCache(ih);
//    IupUpdate(ih);
//  }

//  return 0;
//}

//static int iFlatTreeSetInsertBranchAttrib(Ihandle* ih, int pos, const char* value)
//{
//  int count = iupArrayCount(ih->data->node_cache);

//  if (pos > count)
//    return 0;

//  if (value)
//  {
//    iFlatTreeInsertNode(ih, pos, IFLATTREE_BRANCH, value);

//    iFlatTreeRebuildCache(ih);
//  }

//  ih->data->lastAddNode = pos + 1;

//  if (ih->handle)
//  {
//    iFlatTreeUpdateScrollBar(ih);
//    iFlatTreeRebuildCache(ih);
//    IupUpdate(ih);
//  }

//  return 0;
//}

//static int iFlatTreeSetDelNodeAttrib(Ihandle* ih, int pos, const char* value)
//{
//  iFlatTreeNode *node = iFlatTreeGetNode(ih, pos);

//  if (!ih->handle)  /* do not do the action before map */
//    return 0;

//  if (iupStrEqualNoCase(value, "ALL"))
//  {
//    iFlatTreeRemoveNode(ih, ih->data->node, 0);
//    return 0;
//  }

//  if (iupStrEqualNoCase(value, "SELECTED")) /* selected here means the reference one */
//  {
//    iFlatTreeRemoveNode(ih, node, 0);
//    return 0;
//  }
//  else if (iupStrEqualNoCase(value, "CHILDREN"))  /* children of the reference node */
//  {
//    iFlatTreeRemoveNode(ih, node, 1);
//    return 0;
//  }
//  else if (iupStrEqualNoCase(value, "MARKED"))
//  {
//    iFlatTreeRemoveMarkedNodes(ih, ih->data->node);
//    return 0;
//  }

//  if (ih->handle)
//  {
//    iFlatTreeUpdateScrollBar(ih);
//    iFlatTreeRebuildCache(ih);
//    IupUpdate(ih);
//  }

//  return 0;
//}

//static int iFlatTreeSetExpandAllAttrib(Ihandle* ih, const char* value)
//{
//  int count = iupArrayCount(ih->data->node_cache);
//  iFlatTreeNode **nodes = iupArrayGetData(ih->data->node_cache);
//  int i, all = iupStrFalse(value);

//  for (i = 0; i < count; i++)
//  {
//    if (nodes[i]->kind == IFLATTREE_LEAF)
//      continue;

//    nodes[i]->expand = (all) ? 1 : 0;
//  }

//  if (ih->handle)
//  {
//    iFlatTreeUpdateScrollBar(ih);
//    iFlatTreeRebuildCache(ih);
//    IupUpdate(ih);
//  }

//  return 0;
//}

//static int iFlatTreeSetMoveNodeAttrib(Ihandle* ih, int id, const char* value)
//{
//  iFlatTreeNode *nodeDst, *hParent, *nodeSrc;
//  int dstId = IUP_INVALID_ID;

//  if (!ih->handle)  /* do not do the action before map */
//    return 0;

//  iupStrToInt(value, &dstId);

//  /* If Drag item is an ancestor of Drop item then return */
//  hParent = nodeDst;
//  while (hParent)
//  {
//    hParent = hParent->parent;
//    if (hParent == nodeSrc)
//      return 0;
//  }

//  if (ih->handle)
//  {
//    iFlatTreeUpdateScrollBar(ih);
//    iFlatTreeRebuildCache(ih);
//    IupUpdate(ih);
//  }

//  return 0;
//}

//static int iFlatTreeSetCopyNodeAttrib(Ihandle* ih, int id, const char* value)
//{
//  iFlatTreeNode *nodeDst, *hParent, *nodeSrc;
//  int dstId = IUP_INVALID_ID;

//  if (!ih->handle)  /* do not do the action before map */
//    return 0;

//  iupStrToInt(value, &dstId);

//  /* If Drag item is an ancestor of Drop item then return */
//  hParent = nodeDst;
//  while (hParent)
//  {
//    hParent = hParent->parent;
//    if (hParent == nodeSrc)
//      return 0;
//  }

//  iFlatTreeCopyNode(ih, id, dstId);

//  return 0;
//}

//static int iFlatTreeSetValueAttrib(Ihandle* ih, const char* value)
//{
//  int old_focus_id = ih->data->focus_id;
//
//  if (iupStrEqualNoCase(value, "ROOT") || iupStrEqualNoCase(value, "FIRST"))
//    ih->data->focus_id = 0;
//  else if (iupStrEqualNoCase(value, "LAST"))
//  {
//    int last_pos = iFlatTreeGetVisibleNodesCount(ih) - 1;
//    ih->data->focus_id = iFlatTreeConvertPosToId(ih, last_pos);
//  }
//  else if (iupStrEqualNoCase(value, "PGUP"))
//    ih->data->focus_id = iFlatTreeFocusPageUp(ih);
//  else if (iupStrEqualNoCase(value, "PGDN"))
//    ih->data->focus_id = iFlatTreeFocusPageDown(ih);
//  else if (iupStrEqualNoCase(value, "NEXT"))
//    ih->data->focus_id = iFlatTreeGetNextVisibleNodeId(ih, ih->data->focus_id);
//  else if (iupStrEqualNoCase(value, "PREVIOUS"))
//    ih->data->focus_id = iFlatTreeGetPreviousVisibleNodeId(ih, ih->data->focus_id);
//  else if (iupStrEqualNoCase(value, "CLEAR"))
//    ih->data->focus_id = -1;
//  else
//  {
//    int id = IUP_INVALID_ID;
//    if (iupStrToInt(value, &id))
//    {
//      int count = iupArrayCount(ih->data->node_cache);
//      if (id >= 0 && id < count)
//        ih->data->focus_id = id;
//    }
//  }
//
//  if (ih->data->focus_id != old_focus_id)
//  {
//    int direction = (old_focus_id < ih->data->focus_id) ? IFLATTREE_DOWN : IFLATTREE_UP;
//    if (ih->data->mark_mode == IFLATTREE_MARK_SINGLE)
//      iFlatTreeSelectNode(ih, ih->data->focus_id, 0, 0);
//    iFlatTreeScrollFocusVisible(ih, direction);
//  }
//
//  IupUpdate(ih);
//
//  return 0;
//}

static char* iFlatTreeGetValueAttrib(Ihandle* ih)
{
  int count = iupArrayCount(ih->data->node_cache);
  if (ih->data->focus_id < 0 || ih->data->focus_id >= count)
  {
    if (count == 0)
      return "-1";
    else
      return "0";
  }
  else
    return iupStrReturnInt(ih->data->focus_id);
}

static int iFlatTreeSetMarkAttrib(Ihandle* ih, const char* value)
{
  if (ih->data->mark_mode == IFLATTREE_MARK_SINGLE)
    return 0;

  if (iupStrEqualNoCase(value, "BLOCK"))
    iFlatTreeSelectRange(ih, ih->data->mark_start, ih->data->focus_id);
  else if (iupStrEqualNoCase(value, "CLEARALL"))
    iFlatTreeClearAllSelectionExcept(ih, NULL);
  else if (iupStrEqualNoCase(value, "MARKALL"))
    iFlatTreeSelectAll(ih);
  else if (iupStrEqualNoCase(value, "INVERTALL")) /* INVERTALL *MUST* appear before INVERT, or else INVERTALL will never be called. */
    iFlatTreeInvertSelection(ih);
  else if (iupStrEqualPartial(value, "INVERT")) /* iupStrEqualPartial allows the use of "INVERTid" form */
  {
    iFlatTreeNode *node = iFlatTreeGetNodeFromString(ih, &value[strlen("INVERT")]);
    if (!node)
      return 0;

    node->selected = !(node->selected); /* toggle */
  }
  else
  {
    int id1, id2;

    if (iupStrToIntInt(value, &id1, &id2, '-') != 2)
      return 0;

    iFlatTreeSelectRange(ih, id1, id2);
  }

  IupUpdate(ih);

  return 1;
}

static int iFlatTreeSetMarkStartAttrib(Ihandle* ih, const char* value)
{
  int id;
  if (iupStrToInt(value, &id))
  {
    int count = iupArrayCount(ih->data->node_cache);

    if (id >= 0 && id < count)
      ih->data->mark_start = id;
  }

  return 0;
}

static char* iFlatTreeGetMarkStartAttrib(Ihandle* ih)
{
  return iupStrReturnInt(ih->data->mark_start);
}

static char* iFlatTreeGetMarkedAttrib(Ihandle* ih, int id)
{
  iFlatTreeNode *node = iFlatTreeGetNode(ih, id);
  if (!node)
    return NULL;

  return iupStrReturnBoolean(node->selected);
}

static int iFlatTreeSetMarkedAttrib(Ihandle* ih, int id, const char* value)
{
  iFlatTreeNode *node = iFlatTreeGetNode(ih, id);
  if (!node)
    return 0;

  node->selected = iupStrBoolean(value);

  if (ih->data->mark_mode == IFLATTREE_MARK_SINGLE && node->selected)
  {
    iFlatTreeNode *nodeFocus = iFlatTreeGetNode(ih, ih->data->focus_id);
    if (nodeFocus != node)
    {
      nodeFocus->selected = 0;
      ih->data->focus_id = id;
    }
  }

  return 0;
}

static char* iFlatTreeGetMarkedNodesAttrib(Ihandle* ih)
{
  iFlatTreeNode **nodes = iupArrayGetData(ih->data->node_cache);
  int count = iupArrayCount(ih->data->node_cache);
  char* str = iupStrGetMemory(count + 1);
  int i;

  for (i = 0; i < count; i++)
  {
    if (nodes[i]->selected)
      str[i] = '+';
    else
      str[i] = '-';
  }

  str[count] = 0;
  return str;
}

static int iFlatTreeSetMarkedNodesAttrib(Ihandle* ih, const char* value)
{
  int count, i, len;
  iFlatTreeNode **nodes;

  if (ih->data->mark_mode == IFLATTREE_MARK_SINGLE || !value)
    return 0;

  count = iupArrayCount(ih->data->node_cache);
  nodes = iupArrayGetData(ih->data->node_cache);

  len = (int)strlen(value);
  if (len < count)
    count = len;

  for (i = 0; i < count; i++)
  {
    if (value[i] == '+')
      nodes[i]->selected = 1;
    else
      nodes[i]->selected = 1;
  }

  return 0;
}

static char* iFlatTreeGetMarkModeAttrib(Ihandle* ih)
{
  if (ih->data->mark_mode == IFLATTREE_MARK_SINGLE)
    return "SINGLE";
  else
    return "MULTIPLE";
}

static int iFlatTreeSetMarkModeAttrib(Ihandle* ih, const char* value)
{
  if (iupStrEqualNoCase(value, "MULTIPLE"))
    ih->data->mark_mode = IFLATTREE_MARK_MULTIPLE;
  else
    ih->data->mark_mode = IFLATTREE_MARK_SINGLE;

  if (ih->handle)
    IupUpdate(ih);

  return 0;
}

static int iFlatTreeSetImageAttrib(Ihandle* ih, int id, const char* value)
{
  iFlatTreeNode *node = iFlatTreeGetNode(ih, id);
  if (!node)
    return 0;

  if (node->image)
    free(node->image);
  node->image = iupStrDup(value);

  if (ih->handle)
  {
    iFlatTreeUpdateScrollBar(ih);
    iFlatTreeRebuildCache(ih);
    IupUpdate(ih);
  }

  return 0;
}

static char* iFlatTreeGetImageAttrib(Ihandle* ih, int id)
{
  iFlatTreeNode *node = iFlatTreeGetNode(ih, id);
  if (!node)
    return NULL;

  return node->image;
}

static int iFlatTreeSetImageExpandedAttrib(Ihandle* ih, int id, const char* value)
{
  iFlatTreeNode *node = iFlatTreeGetNode(ih, id);
  if (!node)
    return 0;

  if (node->image_expanded)
    free(node->image_expanded);
  node->image_expanded = iupStrDup(value);

  if (ih->handle)
  {
    iFlatTreeUpdateScrollBar(ih);
    iFlatTreeRebuildCache(ih);
    IupUpdate(ih);
  }

  return 0;
}

static char* iFlatTreeGetImageExpandedAttrib(Ihandle* ih, int id)
{
  iFlatTreeNode *node = iFlatTreeGetNode(ih, id);
  if (!node)
    return NULL;

  return node->image_expanded;
}

static char* iFlatTreeGetImageNativeHandleAttribId(Ihandle* ih, int id)
{
  iFlatTreeNode *node = iFlatTreeGetNode(ih, id);
  if (!node)
    return NULL;

  return iupImageGetImage(node->image, ih, 0, NULL);
}

static int iFlatTreeSetImagePositionAttrib(Ihandle* ih, const char* value)
{
  ih->data->img_position = iupFlatGetImagePosition(value);

  if (ih->handle)
    IupUpdate(ih);

  return 0;
}

static char* iFlatTreeGetImagePositionAttrib(Ihandle *ih)
{
  char* img_pos2str[4] = { "LEFT", "RIGHT", "TOP", "BOTTOM" };
  return img_pos2str[ih->data->img_position];
}

static char* iFlatTreeGetShowDragDropAttrib(Ihandle* ih)
{
  return iupStrReturnBoolean(ih->data->show_dragdrop);
}

static int iFlatTreeSetShowDragDropAttrib(Ihandle* ih, const char* value)
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

static int iFlatTreeSetDragDropTreeAttrib(Ihandle* ih, const char* value)
{
  if (iupStrBoolean(value))
  {
    /* Register callbacks to enable drag and drop between lists */
//    IupSetCallback(ih, "DRAGBEGIN_CB", (Icallback)iFlatTreeDragBegin_CB);
//    IupSetCallback(ih, "DRAGDATASIZE_CB", (Icallback)iFlatTreeDragDataSize_CB);
//    IupSetCallback(ih, "DRAGDATA_CB", (Icallback)iFlatTreeDragData_CB);
//    IupSetCallback(ih, "DRAGEND_CB", (Icallback)iFlatTreeDragEnd_CB);
//    IupSetCallback(ih, "DROPDATA_CB", (Icallback)iFlatTreeDropData_CB);
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

static int iFlatTreeSetIconSpacingAttrib(Ihandle* ih, const char* value)
{
  iupStrToInt(value, &ih->data->icon_spacing);
  if (ih->handle)
    IupUpdate(ih);
  return 0;
}

static char* iFlatTreeGetIconSpacingAttrib(Ihandle *ih)
{
  return iupStrReturnInt(ih->data->icon_spacing);
}

static char* iFlatTreeGetCountAttrib(Ihandle* ih)
{
  return iupStrReturnInt(iupArrayCount(ih->data->node_cache));
}

static char* iFlatTreeGetChildCountAttrib(Ihandle* ih, int id)
{
  int count;
  iFlatTreeNode *child;
  iFlatTreeNode *node = iFlatTreeGetNode(ih, id);
  if (!node)
    return NULL;

  count = 0;
  child = node->firstChild;
  while (child != NULL)
  {
    count++;
    child = child->brother;
  }

  return iupStrReturnInt(count);
}

static char* iFlatTreeGetTotalChildCountAttrib(Ihandle* ih, int id)
{
  int count;
  iFlatTreeNode *node = iFlatTreeGetNode(ih, id);
  if (!node)
    return NULL;

  count = iFlatTreeGetChildCountRec(node);

  return iupStrReturnInt(count);
}

static char* iFlatTreeGetRootCountAttrib(Ihandle* ih)
{
  int count;
  iFlatTreeNode *brother;
  iFlatTreeNode *node = iFlatTreeGetNode(ih, 0);
  if (!node)
    return "0";

  brother = node->brother;

  count = 1;
  while (brother)
  {
    count++;
    brother = brother->brother;
  }

  return iupStrReturnInt(count);
}

static char* iFlatTreeGetDepthAttrib(Ihandle* ih, int id)
{
  int depth;
  iFlatTreeNode *node = iFlatTreeGetNode(ih, id);
  if (!node)
    return NULL;

  depth = iFlatTreeGetNodeDepth(node);

  return iupStrReturnInt(depth);
}

static char* iFlatTreeGetColorAttrib(Ihandle* ih, int id)
{
  iFlatTreeNode *node = iFlatTreeGetNode(ih, id);
  if (!node)
    return NULL;

  return node->fgColor;
}

static int iFlatTreeSetColorAttrib(Ihandle* ih, int id, const char* value)
{
  iFlatTreeNode *node = iFlatTreeGetNode(ih, id);
  if (!node)
    return 0;

  if (node->fgColor)
    free(node->fgColor);
  node->fgColor = iupStrDup(value);

  if (ih->handle)    // TODO: option for multiple update without redraw
    IupUpdate(ih);

  return 0;
}

static char* iFlatTreeGetBackColorAttrib(Ihandle* ih, int id)
{
  iFlatTreeNode *node = iFlatTreeGetNode(ih, id);
  if (!node)
    return NULL;

  return node->bgColor;
}

static int iFlatTreeSetBackColorAttrib(Ihandle* ih, int pos, const char* value)
{
  iFlatTreeNode *node = iFlatTreeGetNode(ih, pos);
  if (!node)
    return 0;

  if (node->bgColor)
    free(node->bgColor);
  node->bgColor = iupStrDup(value);

  if (ih->handle)
    IupUpdate(ih);

  return 0;
}

static int iFlatTreeSetPaddingAttrib(Ihandle* ih, const char* value)
{
  iupStrToIntInt(value, &ih->data->horiz_padding, &ih->data->vert_padding, 'x');
  if (ih->handle)
    IupUpdate(ih);
  return 0;
}

static char* iFlatTreeGetPaddingAttrib(Ihandle* ih)
{
  return iupStrReturnIntInt(ih->data->horiz_padding, ih->data->vert_padding, 'x');
}

static int iFlatTreeSetTopItemAttrib(Ihandle* ih, const char* value)
{
  int id = 0;
  if (iupStrToInt(value, &id))
  {
    int count = iupArrayCount(ih->data->node_cache);
    int posy = 0;

    if (id < 0 || id > count-1)
      return 0;

//    posy = iFlatTreeConvertIdToY(ih, id, NULL);
    IupSetInt(ih, "POSY", posy);

    IupUpdate(ih);
  }
  return 0;
}

static char* iFlatTreeGetShowRenameAttrib(Ihandle* ih)
{
  return iupStrReturnBoolean(ih->data->show_rename);
}

static int iFlatTreeSetShowRenameAttrib(Ihandle* ih, const char* value)
{
  if (iupStrBoolean(value))
    ih->data->show_rename = 1;
  else
    ih->data->show_rename = 0;

  return 0;
}

static int iFlatTreeWheel_CB(Ihandle* ih, float delta)
{
  if (iupFlatScrollBarGet(ih) != IUP_SB_NONE)
    iupFlatScrollBarWheelUpdate(ih, delta);
  return IUP_DEFAULT;
}

static int iFlatTreeSetFlatScrollbarAttrib(Ihandle* ih, const char* value)
{
  /* can only be set before map */
  if (ih->handle)
    return IUP_DEFAULT;

  if (value && !iupStrEqualNoCase(value, "NO"))
  {
    if (iupFlatScrollBarCreate(ih))
    {
      IupSetAttribute(ih, "SCROLLBAR", "NO");
      IupSetCallback(ih, "WHEEL_CB", (Icallback)iFlatTreeWheel_CB);
    }
    return 1;
  }
  else
    return 0;
}

static int iFlatTreeSetBorderWidthAttrib(Ihandle* ih, const char* value)
{
  iupStrToInt(value, &ih->data->border_width);
  if (ih->handle)
    iupdrvRedrawNow(ih);
  return 0;
}

static char* iFlatTreeGetBorderWidthAttrib(Ihandle *ih)
{
  return iupStrReturnInt(ih->data->border_width);
}

static int iFlatTreeSetAttribPostRedraw(Ihandle* ih, const char* value)
{
  (void)value;
  if (ih->handle)
    iupdrvPostRedraw(ih);
  return 1;
}


/*********************************  Methods  ************************************/


static void iFlatTreeSetChildrenCurrentSizeMethod(Ihandle* ih, int shrink)
{
  if (iupFlatScrollBarGet(ih) != IUP_SB_NONE)
    iupFlatScrollBarSetChildrenCurrentSize(ih, shrink);
}

static void iFlatTreeSetChildrenPositionMethod(Ihandle* ih, int x, int y)
{
  if (iupFlatScrollBarGet(ih) != IUP_SB_NONE)
    iupFlatScrollBarSetChildrenPosition(ih);

  (void)x;
  (void)y;
}

static void iFlatTreeDestroyMethod(Ihandle* ih)
{
  if (ih->data->node)
  {
//    iFlatTreeDelNode(ih->data->node, 0);
    ih->data->node = NULL;
  }

  iupArrayDestroy(ih->data->node_cache);
}

static int iFlatTreeCreateMethod(Ihandle* ih, void** params)
{
  Ihandle* text;
  (void)params;

  /* free the data allocated by IupCanvas, and reallocate */
  free(ih->data);
  ih->data = iupALLOCCTRLDATA();

  text = IupText(NULL);

  text->currentwidth = 20;  /* just to avoid initial size 0x0 */
  text->currentheight = 10;
  text->flags |= IUP_INTERNAL;
  iupChildTreeAppend(ih, text);

  //IupSetCallback(text, "VALUECHANGED_CB", (Icallback)iFlatTreeTextEditVALUECHANGED_CB);
  //IupSetCallback(text, "KILLFOCUS_CB", (Icallback)iFlatTreeTextEditKILLFOCUS_CB);
  //IupSetCallback(text, "K_ANY", (Icallback)iFlatTreeTextEditKANY_CB);
  //IupSetCallback(text, "K_CR", (Icallback)iFlatTreeTextEditKCR_CB);
  IupSetAttribute(text, "FLOATING", "IGNORE");
  IupSetAttribute(text, "VISIBLE", "NO");
  IupSetAttribute(text, "ACTIVE", "NO");

  /* non zero default values TODO: check this */
  ih->data->img_position = IUP_IMGPOS_LEFT;
  ih->data->horiz_alignment = IUP_ALIGN_ALEFT;
  ih->data->vert_alignment = IUP_ALIGN_ACENTER;
  ih->data->horiz_padding = 2;
  ih->data->vert_padding = 2;
  ih->data->icon_spacing = 2;
  ih->data->add_expanded = 1;
  //ih->data->dragover_pos = -1;
  //ih->data->toggle_size = (iupRound(iupdrvGetScreenDpi()) > 120) ? 24 : 16;
 // ih->data->level_gap = ih->data->toggle_size;
  //ih->data->image_plusminus_height = 9;
 // ih->data->min_clock = 500;

  ih->data->node = (iFlatTreeNode*)malloc(sizeof(iFlatTreeNode));
  memset(ih->data->node, 0, sizeof(iFlatTreeNode));

  ih->data->node_cache = iupArrayCreate(10, sizeof(iFlatTreeNode*));

  //if (iupAttribGetInt(ih, "ADDROOT"))
  //{
  //  /* add root TODO: move this to insert/add */
  //  iFlatTreeNode ** nodes = iupArrayInc(ih->data->node_cache);
  //  nodes[0] = ih->data->node;
  //  nodes[0]->kind = IFLATTREE_BRANCH;
  //  nodes[0]->toggle_visible = 1;
  //}

  /* internal callbacks */
  //IupSetCallback(ih, "ACTION", (Icallback)iFlatTreeRedraw_CB);
  //IupSetCallback(ih, "BUTTON_CB", (Icallback)iFlatTreeButton_CB);
  //IupSetCallback(ih, "MOTION_CB", (Icallback)iFlatTreeMotion_CB);
  //IupSetCallback(ih, "RESIZE_CB", (Icallback)iFlatTreeResize_CB);
  IupSetCallback(ih, "FOCUS_CB", (Icallback)iFlatTreeFocus_CB);
  //IupSetCallback(ih, "K_CR", (Icallback)iFlatTreeKCr_CB);
  //IupSetCallback(ih, "K_UP", (Icallback)iFlatTreeKUp_CB);
  //IupSetCallback(ih, "K_DOWN", (Icallback)iFlatTreeKDown_CB);
  //IupSetCallback(ih, "K_HOME", (Icallback)iFlatTreeKHome_CB);
  //IupSetCallback(ih, "K_END", (Icallback)iFlatTreeKEnd_CB);
  //IupSetCallback(ih, "K_PGUP", (Icallback)iFlatTreeKPgUp_CB);
  //IupSetCallback(ih, "K_PGDN", (Icallback)iFlatTreeKPgDn_CB);
  //IupSetCallback(ih, "K_sUP", (Icallback)iFlatTreeKUp_CB);
  //IupSetCallback(ih, "K_sDOWN", (Icallback)iFlatTreeKDown_CB);
  //IupSetCallback(ih, "K_cUP", (Icallback)iFlatTreeKUp_CB);
  //IupSetCallback(ih, "K_cDOWN", (Icallback)iFlatTreeKDown_CB);
  //IupSetCallback(ih, "K_cSP", (Icallback)iFlatTreeKcSpace_CB);
  //IupSetCallback(ih, "K_F2", (Icallback)iFlatTreeKF2_CB);
  //IupSetCallback(ih, "SCROLL_CB", (Icallback)iFlatTreeScroll_CB);

  return IUP_NOERROR;
}


/******************************************************************************/


IUP_API Ihandle* IupFlatTree(void)
{
  return IupCreate("flattree");
}

Iclass* iupFlatTreeNewClass(void)
{
  Iclass* ic = iupClassNew(iupRegisterFindClass("canvas"));

  ic->name = "flattree";
  ic->format = NULL;  /* no parameters */
  ic->nativetype = IUP_TYPECANVAS;
  ic->childtype = IUP_CHILDNONE;
  ic->is_interactive = 1;
  ic->has_attrib_id = 1;

  /* Class functions */
  ic->New = iupFlatTreeNewClass;
  ic->Create = iFlatTreeCreateMethod;
  ic->Destroy = iFlatTreeDestroyMethod;
  ic->SetChildrenCurrentSize = iFlatTreeSetChildrenCurrentSizeMethod;
  ic->SetChildrenPosition = iFlatTreeSetChildrenPositionMethod;

  /* Callbacks */
  iupClassRegisterCallback(ic, "TOGGLEVALUE_CB", "ii");
  iupClassRegisterCallback(ic, "SELECTION_CB", "ii");
  iupClassRegisterCallback(ic, "MULTISELECTION_CB", "Ii");
  iupClassRegisterCallback(ic, "MULTIUNSELECTION_CB", "Ii");
  iupClassRegisterCallback(ic, "BRANCHOPEN_CB", "i");
  iupClassRegisterCallback(ic, "BRANCHCLOSE_CB", "i");
  iupClassRegisterCallback(ic, "EXECUTELEAF_CB", "i");
  iupClassRegisterCallback(ic, "SHOWRENAME_CB", "i");
  iupClassRegisterCallback(ic, "RENAME_CB", "is");
  iupClassRegisterCallback(ic, "DRAGDROP_CB", "iiii");
  iupClassRegisterCallback(ic, "RIGHTCLICK_CB", "i");
  iupClassRegisterCallback(ic, "FLAT_BUTTON_CB", "iiiis");
  iupClassRegisterCallback(ic, "FLAT_MOTION_CB", "iis");
  iupClassRegisterCallback(ic, "FLAT_FOCUS_CB", "i");

  iupClassRegisterAttribute(ic, "ACTIVE", iupBaseGetActiveAttrib, iupFlatSetActiveAttrib, IUPAF_SAMEASSYSTEM, "YES", IUPAF_DEFAULT);

  /* General Attributes */

  iupClassRegisterAttribute(ic, "ADDEXPANDED", iFlatTreeGetAddExpandedAttrib, iFlatTreeSetAddExpandedAttrib, "YES", NULL, IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "ADDROOT", NULL, NULL, IUPAF_SAMEASSYSTEM, "YES", IUPAF_NO_INHERIT | IUPAF_NOT_MAPPED);
  iupClassRegisterAttribute(ic, "FGCOLOR", NULL, iFlatTreeSetAttribPostRedraw, IUP_FLAT_FORECOLOR, NULL, IUPAF_NOT_MAPPED);  /* force the new default value */
  iupClassRegisterAttribute(ic, "BGCOLOR", NULL, iFlatTreeSetAttribPostRedraw, IUP_FLAT_BACKCOLOR, NULL, IUPAF_NOT_MAPPED);  /* force the new default value */
  iupClassRegisterAttribute(ic, "EMPTYAS3STATE", NULL, NULL, NULL, NULL, IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "HLCOLOR", NULL, NULL, IUPAF_SAMEASSYSTEM, "TXTHLCOLOR", IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "HLCOLORALPHA", NULL, NULL, IUPAF_SAMEASSYSTEM, "128", IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "INDENTATION", iFlatTreeGetIndentationAttrib, iFlatTreeSetIndentationAttrib, NULL, NULL, IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "SHOWTOGGLE", iFlatTreeGetShowToggleAttrib, iFlatTreeSetShowToggleAttrib, NULL, NULL, IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "SPACING", iFlatTreeGetSpacingAttrib, iFlatTreeSetSpacingAttrib, IUPAF_SAMEASSYSTEM, "0", IUPAF_NO_INHERIT | IUPAF_NOT_MAPPED);
  iupClassRegisterAttribute(ic, "TOPITEM", NULL, iFlatTreeSetTopItemAttrib, NULL, NULL, IUPAF_WRITEONLY | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "SHOWRENAME", iFlatTreeGetShowRenameAttrib, iFlatTreeSetShowRenameAttrib, NULL, NULL, IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);

  /* IupFlatTree Attributes - NODES */

  iupClassRegisterAttribute(ic, "COUNT", iFlatTreeGetCountAttrib, NULL, NULL, NULL, IUPAF_READONLY | IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "CHILDCOUNT", iFlatTreeGetChildCountAttrib, NULL, IUPAF_READONLY | IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "TOTALCHILDCOUNT", iFlatTreeGetTotalChildCountAttrib, NULL, IUPAF_READONLY | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "ROOTCOUNT", iFlatTreeGetRootCountAttrib, NULL, NULL, NULL, IUPAF_READONLY | IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "COLOR", iFlatTreeGetColorAttrib, iFlatTreeSetColorAttrib, IUPAF_NO_INHERIT | IUPAF_NOT_MAPPED);
  iupClassRegisterAttributeId(ic, "BACKCOLOR", iFlatTreeGetBackColorAttrib, iFlatTreeSetBackColorAttrib, IUPAF_NO_INHERIT | IUPAF_NOT_MAPPED);
  iupClassRegisterAttributeId(ic, "DEPTH", iFlatTreeGetDepthAttrib, NULL, IUPAF_READONLY | IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "KIND", iFlatTreeGetKindAttrib, NULL, IUPAF_READONLY | IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "PARENT", iFlatTreeGetParentAttrib, NULL, IUPAF_READONLY | IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "NEXT", iFlatTreeGetNextAttrib, NULL, IUPAF_READONLY | IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "PREVIOUS", iFlatTreeGetPreviousAttrib, NULL, IUPAF_READONLY | IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "LAST", iFlatTreeGetLastAttrib, NULL, IUPAF_READONLY | IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "FIRST", iFlatTreeGetFirstAttrib, NULL, IUPAF_READONLY | IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "STATE", iFlatTreeGetStateAttrib, iFlatTreeSetStateAttrib, IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "TITLE", iFlatTreeGetTitleAttrib, iFlatTreeSetTitleAttrib, IUPAF_NO_INHERIT | IUPAF_NOT_MAPPED);
  iupClassRegisterAttributeId(ic, "TITLEFONT", iFlatTreeGetTitleFontAttrib, iFlatTreeSetTitleFontAttrib, IUPAF_NO_INHERIT | IUPAF_NOT_MAPPED);
  iupClassRegisterAttributeId(ic, "TITLEFONTSTYLE", iFlatTreeGetTitleFontStyleAttrib, iFlatTreeSetTitleFontStyleAttrib, IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "TITLEFONTSIZE", iFlatTreeGetTitleFontSizeAttrib, iFlatTreeSetTitleFontSizeAttrib, IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "TOGGLEVALUE", iFlatTreeGetToggleValueAttrib, iFlatTreeSetToggleValueAttrib, IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "TOGGLEVISIBLE", iFlatTreeGetToggleVisibleAttrib, iFlatTreeSetToggleVisibleAttrib, IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "USERDATA", iFlatTreeGetUserDataAttrib, iFlatTreeSetUserDataAttrib, IUPAF_NO_STRING | IUPAF_NO_INHERIT);
  //iupClassRegisterAttribute(ic, "RENAME", NULL, iFlatTreeSetRenameAttrib, NULL, NULL, IUPAF_WRITEONLY | IUPAF_NO_INHERIT);

  /* IupFlatTree Attributes - IMAGES */

  iupClassRegisterAttributeId(ic, "IMAGE", iFlatTreeGetImageAttrib, iFlatTreeSetImageAttrib, IUPAF_IHANDLENAME | IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "IMAGEEXPANDED", iFlatTreeGetImageExpandedAttrib, iFlatTreeSetImageExpandedAttrib, IUPAF_IHANDLENAME | IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "IMAGELEAF", NULL, iFlatTreeSetAttribPostRedraw, IUPAF_SAMEASSYSTEM, "IMGLEAF", IUPAF_IHANDLENAME | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "IMAGEBRANCHCOLLAPSED", NULL, iFlatTreeSetAttribPostRedraw, IUPAF_SAMEASSYSTEM, "IMGCOLLAPSED", IUPAF_IHANDLENAME | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "IMAGEBRANCHEXPANDED", NULL, iFlatTreeSetAttribPostRedraw, IUPAF_SAMEASSYSTEM, "IMGEXPANDED", IUPAF_IHANDLENAME | IUPAF_NO_INHERIT);

  /* IupFlatTree Attributes - FOCUS NODE */

//  iupClassRegisterAttribute(ic, "VALUE", iFlatTreeGetValueAttrib, iFlatTreeSetValueAttrib, NULL, NULL, IUPAF_NO_SAVE | IUPAF_NO_DEFAULTVALUE | IUPAF_NO_INHERIT);

  /* IupFlatTree Attributes - MARKS */

  iupClassRegisterAttribute(ic, "MARK", NULL, iFlatTreeSetMarkAttrib, NULL, NULL, IUPAF_WRITEONLY | IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "MARKED", iFlatTreeGetMarkedAttrib, iFlatTreeSetMarkedAttrib, IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "MARKEDNODES", iFlatTreeGetMarkedNodesAttrib, iFlatTreeSetMarkedNodesAttrib, NULL, NULL, IUPAF_NO_SAVE | IUPAF_NO_DEFAULTVALUE | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "MARKMODE", iFlatTreeGetMarkModeAttrib, iFlatTreeSetMarkModeAttrib, IUPAF_SAMEASSYSTEM, "SINGLE", IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "MARKSTART", iFlatTreeGetMarkStartAttrib, iFlatTreeSetMarkStartAttrib, NULL, NULL, IUPAF_NO_DEFAULTVALUE | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "MARKWHENTOGGLE", NULL, NULL, NULL, NULL, IUPAF_NO_INHERIT);

  /* IupFlatTree Attributes - HIERARCHY */

  //iupClassRegisterAttributeId(ic, "ADDLEAF", NULL, iFlatTreeSetAddLeafAttrib, IUPAF_WRITEONLY | IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
  //iupClassRegisterAttributeId(ic, "ADDBRANCH", NULL, iFlatTreeSetAddBranchAttrib, IUPAF_WRITEONLY | IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
  //iupClassRegisterAttributeId(ic, "COPYNODE", NULL, iFlatTreeSetCopyNodeAttrib, IUPAF_NOT_MAPPED | IUPAF_WRITEONLY | IUPAF_NO_INHERIT);
  //iupClassRegisterAttributeId(ic, "DELNODE", NULL, iFlatTreeSetDelNodeAttrib, IUPAF_WRITEONLY | IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
  //iupClassRegisterAttribute(ic, "EXPANDALL", NULL, iFlatTreeSetExpandAllAttrib, NULL, NULL, IUPAF_WRITEONLY | IUPAF_NO_INHERIT);
  //iupClassRegisterAttributeId(ic, "INSERTLEAF", NULL, iFlatTreeSetInsertLeafAttrib, IUPAF_WRITEONLY | IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
  //iupClassRegisterAttributeId(ic, "INSERTBRANCH", NULL, iFlatTreeSetInsertBranchAttrib, IUPAF_WRITEONLY | IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
  //iupClassRegisterAttributeId(ic, "MOVENODE", NULL, iFlatTreeSetMoveNodeAttrib, IUPAF_NOT_MAPPED | IUPAF_WRITEONLY | IUPAF_NO_INHERIT);

  iupClassRegisterAttribute(ic, "BORDERCOLOR", NULL, NULL, IUPAF_SAMEASSYSTEM, IUP_FLAT_BORDERCOLOR, IUPAF_DEFAULT);  /* inheritable */
  iupClassRegisterAttribute(ic, "BORDERWIDTH", iFlatTreeGetBorderWidthAttrib, iFlatTreeSetBorderWidthAttrib, IUPAF_SAMEASSYSTEM, "0", IUPAF_NOT_MAPPED);  /* inheritable */
  iupClassRegisterAttribute(ic, "PADDING", iFlatTreeGetPaddingAttrib, iFlatTreeSetPaddingAttrib, IUPAF_SAMEASSYSTEM, "2x2", IUPAF_NOT_MAPPED);
  iupClassRegisterAttribute(ic, "HASFOCUS", iFlatTreeGetHasFocusAttrib, NULL, NULL, NULL, IUPAF_READONLY | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "ALIGNMENT", iFlatTreeGetAlignmentAttrib, iFlatTreeSetAlignmentAttrib, "ALEFT:ACENTER", NULL, IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "FOCUSFEEDBACK", NULL, NULL, IUPAF_SAMEASSYSTEM, "YES", IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);

  iupClassRegisterAttributeId(ic, "IMAGENATIVEHANDLE", iFlatTreeGetImageNativeHandleAttribId, NULL, IUPAF_NO_STRING | IUPAF_READONLY | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "IMAGEPOSITION", iFlatTreeGetImagePositionAttrib, iFlatTreeSetImagePositionAttrib, IUPAF_SAMEASSYSTEM, "LEFT", IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "ICONSPACING", iFlatTreeGetIconSpacingAttrib, iFlatTreeSetIconSpacingAttrib, IUPAF_SAMEASSYSTEM, "2", IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "TEXTALIGNMENT", NULL, NULL, IUPAF_SAMEASSYSTEM, "ALEFT", IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "TEXTWRAP", NULL, NULL, NULL, NULL, IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "TEXTELLIPSIS", NULL, NULL, NULL, NULL, IUPAF_NO_INHERIT);

  iupClassRegisterAttribute(ic, "BACKIMAGE", NULL, NULL, NULL, NULL, IUPAF_NOT_MAPPED | IUPAF_IHANDLENAME | IUPAF_NO_DEFAULTVALUE | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "BACKIMAGEZOOM", NULL, NULL, NULL, NULL, IUPAF_NO_INHERIT);

  iupClassRegisterAttribute(ic, "SHOWDRAGDROP", iFlatTreeGetShowDragDropAttrib, iFlatTreeSetShowDragDropAttrib, NULL, NULL, IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "DRAGDROPTREE", NULL, iFlatTreeSetDragDropTreeAttrib, NULL, NULL, IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);

  iupClassRegisterAttribute(ic, "VISIBLECOLUMNS", NULL, NULL, NULL, NULL, IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "VISIBLELINES", NULL, NULL, "5", NULL, IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);

  iupClassRegisterReplaceAttribDef(ic, "SCROLLBAR", "YES", NULL);  /* change the default to Yes */
  iupClassRegisterAttribute(ic, "YAUTOHIDE", NULL, NULL, IUPAF_SAMEASSYSTEM, "YES", IUPAF_READONLY | IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);  /* will be always Yes */
  iupClassRegisterAttribute(ic, "XAUTOHIDE", NULL, NULL, IUPAF_SAMEASSYSTEM, "YES", IUPAF_READONLY | IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);  /* will be always Yes */

  iFlatTreeInitializeImages();

  /* Flat Scrollbar */
  iupFlatScrollBarRegister(ic);

  iupClassRegisterAttribute(ic, "FLATSCROLLBAR", NULL, iFlatTreeSetFlatScrollbarAttrib, NULL, NULL, IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);

  return ic;
}
