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

#define IFLATTREE_UP        0
#define IFLATTREE_DOWN      1

#define IFLATTREE_TOGGLE_MARGIN 2
#define IFLATTREE_TOGGLE_BORDER 1
#define IFLATTREE_TOGGLE_SPACE  2

enum { IFLATTREE_EXPANDED, IFLATTREE_COLLAPSED };  /* state */
enum { IFLATTREE_BRANCH, IFLATTREE_LEAF };  /* kind */
enum { IFLATTREE_MARK_SINGLE, IFLATTREE_MARK_MULTIPLE };  /* mark_mode */

typedef struct _iFlatTreeNode 
{
  /* attributes */
  char* title;
  char* image;
  char* image_expanded;
  char *fg_color;
  char *bg_color;
  char *font;
  int selected;        /* bool */
  int kind;
  int state;
  int toggle_visible;  /* bool */
  int toggle_value;    /* bool */
  void* userdata;

  /* aux */
  int id, depth;
  int height,  /* image+title height, does not includes spacing */
      width,   /* image+title width, includes also indentation and toggle_size*/
      title_width;

  struct _iFlatTreeNode *parent;
  struct _iFlatTreeNode *first_child;
  struct _iFlatTreeNode *brother;
} iFlatTreeNode;

struct _IcontrolData
{
  iupCanvas canvas;  /* from IupCanvas (must reserve it) */

  iFlatTreeNode *root_node;  /* tree of nodes, root node always exists and it always invisible */
  Iarray *node_array;   /* array of nodes indexed by id, needs to be updated when nodes are added or removed */

  /* aux */
  int has_focus, focus_id;
  //int last_selected_id;
  int dragover_pos, dragged_pos;
  //int last_clock, min_clock;
  int toggle_size;
  int expander_size;

  /* attributes */
  int add_expanded;
  int indentation;   /* horizontal spacing between one depth and the next */
  int show_rename;
  int icon_spacing;  /* distance between image and text */
  int spacing;
  int horiz_alignment, vert_alignment;
  int border_width;
  int mark_mode, mark_start;
  int show_dragdrop;
  int last_add_node;
  int show_toggle;
  int auto_redraw;
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


static void iFlatTreeSetNodeDrawFont(Ihandle* ih, iFlatTreeNode *node, const char* font)
{
  if (node->font)
    iupAttribSet(ih, "DRAWFONT", node->font);
  else
    iupAttribSet(ih, "DRAWFONT", font);
}

static char *iFlatTreeGetNodeImage(Ihandle* ih, iFlatTreeNode *node, int check_state)
{
  char* image;

  if (node->kind == IFLATTREE_LEAF)
  {
    if (node->image)
      image = node->image;
    else
      image = iupAttribGetStr(ih, "IMAGELEAF");
  }
  else
  {
    if (!check_state || node->state == IFLATTREE_COLLAPSED)
    {
      if (node->image)
        image = node->image;
      else
        image = iupAttribGetStr(ih, "IMAGEBRANCHCOLLAPSED");
    }
    else
    {
      if (node->image_expanded)
        image = node->image_expanded;
      else
        image = iupAttribGetStr(ih, "IMAGEBRANCHEXPANDED");
    }
  }

  return image;
}


/********************** Node Hierarchy **********************/


static void iFlatTreeCalcNodeSize(Ihandle *ih, iFlatTreeNode *node, const char* font)
{
  int w, h;
  int img_w, img_h;
  int txt_w = 0, txt_h = 0;
  char *image = iFlatTreeGetNodeImage(ih, node, 0);

  iupImageGetInfo(image, &img_w, &img_h, NULL);

  iFlatTreeSetNodeDrawFont(ih, node, font);

  if (node->title)
    iupDrawGetTextSize(ih, node->title, 0, &txt_w, &txt_h, 0);
  else
    iupDrawGetTextSize(ih, "W", 0, &txt_w, &txt_h, 0);

  /* leave room for focus feedback */
  if (ih->iclass->is_interactive && iupAttribGetBoolean(ih, "CANFOCUS") && iupAttribGetBoolean(ih, "FOCUSFEEDBACK"))
  {
    txt_w += 2 * 2;
    txt_h += 2 * 2;
  }

  node->title_width = txt_w;

  w = img_w + ih->data->icon_spacing + txt_w;
  h = iupMAX(img_h, txt_h);

  node->height = h;
  node->width = w + ((node->depth + 1) * ih->data->indentation);

  if (ih->data->show_toggle && node->toggle_visible)
    node->width += ih->data->toggle_size;
}

static void iFlatTreeUpdateNodeSizeRec(Ihandle *ih, iFlatTreeNode *node, const char* font)
{
  while (node)
  {
    iFlatTreeCalcNodeSize(ih, node, font);

    if (node->kind == IFLATTREE_BRANCH && node->first_child)
      iFlatTreeUpdateNodeSizeRec(ih, node->first_child, font);

    node = node->brother;
  }
}

static void iFlatTreeUpdateNodeSize(Ihandle *ih, iFlatTreeNode *node)
{
  char* font = IupGetAttribute(ih, "FONT");
  iFlatTreeCalcNodeSize(ih, node, font);
}

static void iFlatTreeUpdateNodeSizeAll(Ihandle *ih)
{
  char* font = IupGetAttribute(ih, "FONT");
  iFlatTreeUpdateNodeSizeRec(ih, ih->data->root_node->first_child, font);
}

static void iFlatTreeUpdateNodeIdRec(iFlatTreeNode **nodes, iFlatTreeNode *node, int *id, int depth)
{
  while (node)
  {
    nodes[*id] = node;
    node->id = *id;
    node->depth = depth;
    (*id)++;

    if (node->kind == IFLATTREE_BRANCH && node->first_child)
      iFlatTreeUpdateNodeIdRec(nodes, node->first_child, id, depth + 1);

    node = node->brother;
  }
}

static void iFlatTreeRebuildArray(Ihandle *ih, int num)
{
  /* one or mode nodes were moved (num=0), removed (num<0) or added (num>0), must update all the ids */
  iFlatTreeNode *node = ih->data->root_node->first_child;
  int id = 0;
  int depth = 0;
  
  if (num > 0)
    iupArrayAdd(ih->data->node_array, num);  /* increment the array */
  else if (num < 0)
  {
    int count = iupArrayCount(ih->data->node_array);
    iupArrayRemove(ih->data->node_array, count-num, num);  /* decrement the array, memory is preserved */
  }
  else
  {
    if (!node)
    {
      int count = iupArrayCount(ih->data->node_array);
      iupArrayRemove(ih->data->node_array, 0, count);  /* clear the array, memory is preserved */
    }
  }
  
  iFlatTreeUpdateNodeIdRec(iupArrayGetData(ih->data->node_array), node, &id, depth);
}

static iFlatTreeNode *iFlatTreeGetNode(Ihandle *ih, int id)
{
  iFlatTreeNode **nodes = iupArrayGetData(ih->data->node_array);
  int count = iupArrayCount(ih->data->node_array);

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
    if (parent->state == IFLATTREE_COLLAPSED)
      return 0;
    parent = parent->parent;
  }

  return 1;
}

static int iFlatTreeGetNextVisibleNodeId(Ihandle *ih, int id)
{
  int count = iupArrayCount(ih->data->node_array);
  iFlatTreeNode **nodes = iupArrayGetData(ih->data->node_array);
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
  iFlatTreeNode **nodes = iupArrayGetData(ih->data->node_array);
  int i;

  for (i = id - 1; i >= 0; i--)
  {
    if (iFlatTreeNodeIsVisible(nodes[i]))
      return i;
  }

  return id;
}

static int iFlatTreeGetVisibleNodesRec(iFlatTreeNode *node)
{
  int count = 0;

  while (node)
  {
    count++;

    if (node->kind == IFLATTREE_BRANCH && node->first_child && node->state == IFLATTREE_EXPANDED)
      count += iFlatTreeGetVisibleNodesRec(node->first_child);

    node = node->brother;
  }

  return count;
}

static int iFlatTreeGetVisibleNodesCount(Ihandle *ih)
{
  return iFlatTreeGetVisibleNodesRec(ih->data->root_node->first_child);
}

static iFlatTreeNode *iFlatTreeNewNode(const char* title, int kind)
{
  iFlatTreeNode *newNode = (iFlatTreeNode *)malloc(sizeof(iFlatTreeNode));
  memset(newNode, 0, sizeof(iFlatTreeNode));

  newNode->title = iupStrDup(title);
  newNode->kind = kind;
  newNode->toggle_visible = 1;

  return newNode;
}

static iFlatTreeNode *iFlatTreeCloneNode(iFlatTreeNode *node)
{
  iFlatTreeNode *newNode = (iFlatTreeNode*)malloc(sizeof(iFlatTreeNode));
  memset(newNode, 0, sizeof(iFlatTreeNode));

  newNode->title = iupStrDup(node->title);
  newNode->image = iupStrDup(node->image);
  newNode->image_expanded = iupStrDup(node->image_expanded);
  newNode->bg_color = iupStrDup(node->bg_color);
  newNode->fg_color = iupStrDup(node->fg_color);
  newNode->font = iupStrDup(node->font);
  newNode->kind = node->kind;
  newNode->state = node->state;
  newNode->toggle_visible = node->toggle_visible;
  newNode->toggle_value = node->toggle_value;
  /* selected and userdata are NOT copied */

  if (node->first_child)
  {
    iFlatTreeNode *child = node->first_child;
    iFlatTreeNode *lastNode = NULL;

    while (child)
    {
      iFlatTreeNode *newChildNode = iFlatTreeCloneNode(child);
      if (!lastNode)
        newNode->first_child = newChildNode;
      else
        lastNode->brother = newChildNode;
      newChildNode->parent = newNode;
      lastNode = newChildNode;

      child = child->brother;
    }
  }

  return newNode;
}

static void iFlatTreeUnlinkNodeFromParent(iFlatTreeNode* node)
{
  /* removed the node from its parent */
  iFlatTreeNode *parent = node->parent;
  if (node == parent->first_child)
    parent->first_child = node->brother;
  else
  {
    iFlatTreeNode *brother = parent->first_child;
    while (brother)
    {
      if (brother->brother == node)
      {
        brother->brother = node->brother;
        break;
      }

      brother = brother->brother;
    }
  }
}

static void iFlatTreeRemoveNodeRec(iFlatTreeNode *node)
{
  /* remove node and its children */
  iFlatTreeNode *child = node->first_child;
  while (child)
  {
    iFlatTreeNode *brother = child->brother;
    iFlatTreeRemoveNodeRec(child);
    child = brother;
  }

  if (node->title)
    free(node->title);

  if (node->image)
    free(node->image);

  if (node->image_expanded)
    free(node->image_expanded);

  if (node->fg_color)
    free(node->fg_color);

  if (node->bg_color)
    free(node->bg_color);

  if (node->font)
    free(node->font);

  free(node);
}

static iFlatTreeNode *iFlatTreeCopyNode(Ihandle *ih, int srcId, int dstId)
{
  iFlatTreeNode *srcNode = iFlatTreeGetNode(ih, srcId);
  iFlatTreeNode *dstNode = iFlatTreeGetNode(ih, dstId);
  iFlatTreeNode *parent;
  iFlatTreeNode *newNode;

  if (!dstNode || !srcNode)
    return NULL;

  /* If srcNode is an ancestor of dstNode then return */
  parent = dstNode;
  while (parent)
  {
    if (parent == srcNode)
      return NULL;

    parent = parent->parent;
  }

  newNode = iFlatTreeCloneNode(srcNode);

  if (dstNode->kind == IFLATTREE_BRANCH && dstNode->state == IFLATTREE_EXPANDED)
  {
    /* copy as first child of expanded branch */
    newNode->parent = dstNode;
    newNode->brother = dstNode->first_child;
    dstNode->first_child = newNode;
  }
  else
  {
    newNode->parent = dstNode->parent;
    newNode->brother = dstNode->brother;
    dstNode->brother = newNode;
  }

  iFlatTreeRebuildArray(ih, +1);

  iFlatTreeUpdateNodeSize(ih, dstNode);

  return newNode;
}

static iFlatTreeNode *iFlatTreeMoveNode(Ihandle *ih, int srcId, int dstId)
{
  iFlatTreeNode *srcNode = iFlatTreeGetNode(ih, srcId);
  iFlatTreeNode *dstNode = iFlatTreeGetNode(ih, dstId);
  iFlatTreeNode *parent;

  if (!dstNode || !srcNode)
    return NULL;

  /* If srcNode is an ancestor of dstNode then return */
  parent = dstNode;
  while (parent)
  {
    if (parent == srcNode)
      return NULL;

    parent = parent->parent;
  }

  iFlatTreeUnlinkNodeFromParent(srcNode);

  if (dstNode->kind == IFLATTREE_BRANCH && dstNode->state == IFLATTREE_EXPANDED)
  {
    /* copy as first child of expanded branch */
    srcNode->parent = dstNode;
    srcNode->brother = dstNode->first_child;
    dstNode->first_child = srcNode;
  }
  else
  {
    srcNode->parent = dstNode->parent;
    srcNode->brother = dstNode->brother;
    dstNode->brother = srcNode;
  }

  iFlatTreeRebuildArray(ih, 0);

  return srcNode;
}

static void iFlatTreeAddNode(Ihandle* ih, int id, int kind, const char* title)
{
  iFlatTreeNode *refNode;
  iFlatTreeNode *newNode;

  if (id == -1)
    refNode = ih->data->root_node;
  else
    refNode = iFlatTreeGetNode(ih, id);

  if (!refNode)
    return;

  newNode = iFlatTreeNewNode(title, kind);

  if (refNode->kind == IFLATTREE_LEAF)
  {
    /* add as brother */
    newNode->parent = refNode->parent;
    newNode->brother = refNode->brother;

    refNode->brother = newNode;
  }
  else
  {
    /* add as first child */
    newNode->parent = refNode;
    newNode->brother = refNode->first_child;

    refNode->first_child = newNode;
  }

  if (newNode->kind == IFLATTREE_BRANCH)
    newNode->state = ih->data->add_expanded ? IFLATTREE_EXPANDED : IFLATTREE_COLLAPSED;

  iFlatTreeRebuildArray(ih, +1);
  iFlatTreeUpdateNodeSize(ih, newNode);

  ih->data->last_add_node = newNode->id;
}

static void iFlatTreeInsertNode(Ihandle* ih, int id, int kind, const char* title)
{
  iFlatTreeNode *refNode;
  iFlatTreeNode *newNode;

  if (id == -1)
    refNode = iFlatTreeGetNode(ih, 0);
  else
    refNode = iFlatTreeGetNode(ih, id);

  if (!refNode)
    return;

  newNode = iFlatTreeNewNode(title, kind);

  /* add as brother always */
  newNode->parent = refNode->parent;
  newNode->brother = refNode->brother;

  refNode->brother = newNode;

  if (newNode->kind == IFLATTREE_BRANCH)
    newNode->state = ih->data->add_expanded ? IFLATTREE_EXPANDED : IFLATTREE_COLLAPSED;

  iFlatTreeRebuildArray(ih, +1);
  iFlatTreeUpdateNodeSize(ih, newNode);

  ih->data->last_add_node = newNode->id;
}

//static int iFlatTreeRenameNode(Ihandle* ih)
//{
//  if (ih->data->show_rename && ih->data->has_focus)
//  {
//    iFlatTreeNode *nodeFocus = iFlatTreeGetNode(ih, ih->data->focus_id);
//    int pos, txt_x, txt_y;
//    Ihandle* text = NULL;
//    Ihandle* first_child = ih->firstchild;
//    char title[1024];

//    if (!nodeFocus->title || *(nodeFocus->title) == 0)
//      strcpy(title, "XXXXX");
//    else
//    {
//      strcpy(title, nodeFocus->title);
//      strcat(title, "X");

//    }

//    while (first_child)
//    {
//      if (iupStrEqual(first_child->iclass->name, "text"))
//      {
//        text = first_child;
//        break;
//      }
//      first_child = first_child->brother;
//    }

//    if (!text)
//      return 1;

//    iFlatTreeGetTitlePos(ih, ih->data->focus_id, &txt_x, &txt_y);

//    text->x = txt_x;
//    text->y = txt_y;

//    text->currentwidth = node->title_width;
//    text->currentheight = node->height;

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

//static void iFlatTreeUpdateText(Ihandle *text, int x, int y, int w, int h)
//{
//  if (text->x == x && text->y == y && text->currentwidth > w && text->currentheight > h)
//    return;

//  text->x = x;
//  text->y = y;

//  text->currentwidth = w;
//  text->currentheight = h;

//  iupClassObjectLayoutUpdate(text);
//}

//static int iFlatTreeConvertIdToY(Ihandle *ih, int id, int *h);

//static void iFlatTreeGetTitlePos(Ihandle *ih, int id, int *txt_x, int *txt_y)
//{
//  int total_h = iFlatTreeConvertIdToY(ih, id, NULL);
//  int posx = IupGetInt(ih, "POSX");
//  int posy = IupGetInt(ih, "POSY");
//  iFlatTreeNode *node = iFlatTreeGetNode(ih, id);
//  int border_width = ih->data->border_width;

//  int img_h = 0;
//  int img_w = 0;

//  iupImageGetInfo(image, &img_w, &img_h, NULL);

//  *txt_x = -posx + border_width;
//  *txt_y = -posy + border_width;

//  *txt_x += node->depth * ih->data->indentation + img_w;
//  *txt_y += total_h;

//  *txt_x += ih->data->icon_spacing;

//  if (ih->data->show_toggle && nodeFocus->toggle_visible)
//    text->x += ih->data->toggle_size;
//}

//static int iFlatTreeNodeIsVisible(iFlatTreeNode *node);

//static int iFlatTreeConvertIdToPos(Ihandle *ih, int id)
//{
//  int count = iupArrayCount(ih->data->node_array);
//  iFlatTreeNode **nodes = iupArrayGetData(ih->data->node_array);
//  int i, pos = 0;

//  if (id > count)
//    return -1;

//  for (i = 0; i < count; i++)
//  {
//    if (!iFlatTreeNodeIsVisible(nodes[i]))
//      continue;
//    if (nodes[i]->id == id)
//      break;
//    pos++;
//  }

//  return pos;
//}

//static int iFlatTreeConvertPosToId(Ihandle *ih, int pos)
//{
//  int count = iupArrayCount(ih->data->node_array);
//  iFlatTreeNode **nodes = iupArrayGetData(ih->data->node_array);
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
  iFlatTreeNode **nodes = iupArrayGetData(ih->data->node_array);
  int count = iupArrayCount(ih->data->node_array);
  for (i = 0; i < count; i++)
    nodes[i]->selected = !(nodes[i]->selected);
}

static void iFlatTreeSelectAll(Ihandle* ih)
{
  int i;
  iFlatTreeNode **nodes = iupArrayGetData(ih->data->node_array);
  int count = iupArrayCount(ih->data->node_array);
  for (i = 0; i < count; i++)
    nodes[i]->selected = 1;
}

static void iFlatTreeClearAllSelectionExcept(Ihandle* ih, iFlatTreeNode *nodeExcept)
{
  int i;
  iFlatTreeNode **nodes = iupArrayGetData(ih->data->node_array);
  int count = iupArrayCount(ih->data->node_array);
  for (i = 0; i < count; i++)
  {
    if (nodes[i] != nodeExcept)
      nodes[i]->selected = 0;
  }
}

static void iFlatTreeSelectRange(Ihandle* ih, int id1, int id2)
{
  int i;
  iFlatTreeNode **nodes = iupArrayGetData(ih->data->node_array);
  int count = iupArrayCount(ih->data->node_array);

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
//  int count = iupArrayCount(ih->data->node_array);
//  iFlatTreeNode **nodes = iupArrayGetData(ih->data->node_array);
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

//    py += nodes[i]->height + ih->data->spacing;
//    pos++;
//  }

//  (void)x;
//  return -1;
//}

//static int iFlatTreeConvertIdToY(Ihandle *ih, int id, int *h)
//{
//  int count = iupArrayCount(ih->data->node_array);
//  iFlatTreeNode **nodes = iupArrayGetData(ih->data->node_array);
//  int i;
//  int w, temp_h;
//  int total_h = 0;

//  if (id > count)
//    return -1;

//  for (i = 0; i < count; i++)
//  {
//    if (!iFlatTreeNodeIsVisible(nodes[i]))
//      continue;

//    if (h)
//      *h = nodes[i]->height;

//    if (i == id)
//      break;

//    total_h += nodes[i]->height + ih->data->spacing;

//  }

//  return total_h;
//}

//static int iFlatTreePageLastItemId(Ihandle *ih, int py)
//{
//  int count = iupArrayCount(ih->data->node_array);
//  iFlatTreeNode **nodes = iupArrayGetData(ih->data->node_array);
//  int posy = IupGetInt(ih, "POSY");
//  int dy = IupGetInt(ih, "DY");
//  int i, last_id = -1;
//  int total_y = posy + dy;
//  int total_h, h;

//  for (i = 0; i < count; i++)
//  {
//    if (!iFlatTreeNodeIsVisible(nodes[i]))
//      continue;

//    total_h = iFlatTreeConvertIdToY(ih, nodes[i]->id, &h);

//    if (total_h + h > total_y)
//      break;

//    last_id = nodes[i]->id;
//  }

//  return last_id;
//}

//static int iFlatTreePageDownFromFocus(Ihandle *ih)
//{
//  int count = iupArrayCount(ih->data->node_array);
//  iFlatTreeNode **nodes = iupArrayGetData(ih->data->node_array);
//  int focus_y = iFlatTreeConvertIdToY(ih, ih->data->focus_id, NULL);
//  int dy = IupGetInt(ih, "DY");
//  int i, last_id = -1;
//  int total_h = 0;

//  for (i = ih->data->focus_id; i < count; i++)
//  {
//    if (!iFlatTreeNodeIsVisible(nodes[i]))
//      continue;

//    total_h += (nodes[i]->height + ih->data->spacing);

//    if (total_h + nodes[i]->height > focus_y + dy)
//      break;

//    last_id = i;
//  }

//  return last_id;
//}

//static int iFlatTreePageUpFromFocus(Ihandle *ih)
//{
//  iFlatTreeNode **nodes = iupArrayGetData(ih->data->node_array);
//  int focus_y = iFlatTreeConvertIdToY(ih, ih->data->focus_id, NULL);
//  int dy = IupGetInt(ih, "DY");
//  int i, last_id = ih->data->focus_id;
//  int total_h = focus_y;

//  for (i = ih->data->focus_id - 1; i >= 0; i--)
//  {
//    if (!iFlatTreeNodeIsVisible(nodes[i]))
//      continue;

//    total_h -= (nodes[i]->height + ih->data->spacing);

//    if (total_h - nodes[i]->height < focus_y - dy)
//      break;

//    last_id = i;
//  }

//  return last_id;
//}

static void iFlatTreeGetViewSize(Ihandle *ih, int *view_width, int *view_height)
{
  int count = iupArrayCount(ih->data->node_array);
  iFlatTreeNode **nodes = iupArrayGetData(ih->data->node_array);
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
    view_height += ih->data->indentation / 2; /* additional space for drop area */

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

    IupSetfAttribute(ih, "LINEY", "%d", ih->data->indentation);  /* TODO use line height? */
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


static void iFlatTreeDrawToggle(Ihandle *ih, IdrawCanvas* dc, iFlatTreeNode *node, int x, int y, int h, const char* fore_color, const char* back_color, int active)
{
  int check_xmin = x + IFLATTREE_TOGGLE_MARGIN;
  int check_ymin = y + IFLATTREE_TOGGLE_MARGIN + ((h - ih->data->toggle_size) / 2);
  int check_size = ih->data->toggle_size - 2 * IFLATTREE_TOGGLE_MARGIN;

  /* check border */
  iupFlatDrawBorder(dc, check_xmin, check_xmin + check_size,
                        check_ymin, check_ymin + check_size,
                        IFLATTREE_TOGGLE_BORDER, fore_color, back_color, active);

  /* check mark */
  if (node->toggle_value)
  {
    if (node->toggle_value == -1)
      iupFlatDrawBox(dc, check_xmin + IFLATTREE_TOGGLE_SPACE + IFLATTREE_TOGGLE_BORDER, check_xmin + check_size - IFLATTREE_TOGGLE_SPACE - IFLATTREE_TOGGLE_BORDER,
                         check_ymin + IFLATTREE_TOGGLE_SPACE + IFLATTREE_TOGGLE_BORDER, check_ymin + check_size - IFLATTREE_TOGGLE_SPACE - IFLATTREE_TOGGLE_BORDER,
                         fore_color, back_color, active);
    else
      iupFlatDrawCheckMark(dc, check_xmin + IFLATTREE_TOGGLE_SPACE + IFLATTREE_TOGGLE_BORDER, check_xmin + check_size - IFLATTREE_TOGGLE_SPACE - IFLATTREE_TOGGLE_BORDER,
                               check_ymin + IFLATTREE_TOGGLE_SPACE + IFLATTREE_TOGGLE_BORDER, check_ymin + check_size - IFLATTREE_TOGGLE_SPACE - IFLATTREE_TOGGLE_BORDER,
                               fore_color, back_color, active);
  }
}

static int iFlatTreeDrawNodes(Ihandle *ih, IdrawCanvas* dc, iFlatTreeNode *node, int x, int y, const char *fg_color, const char *bg_color, int make_inactive, int active,
                              int text_flags, const char *font, int focus_feedback, int width, int border_width, int *pos)
{
  int node_x = x + (node->depth * ih->data->indentation);
  int node_y = y;

  /* TODO optimize not drawing non visible nodes */

  while (node)
  {
    char *fore_color = (node->fg_color) ? node->fg_color : fg_color;
    char *back_color = (node->bg_color) ? node->bg_color : bg_color;
    int node_h = node->height,
        title_x, image_gap = 0, toggle_gap = 0;
    const char *image = iFlatTreeGetNodeImage(ih, node, 1);

    iupImageGetInfo(image, &image_gap, NULL, NULL);
    image_gap += ih->data->icon_spacing;

    /* guidelines */
    if (node->depth != 0)
    {
      int px1 = (node_x - (ih->data->indentation / 2)) + 1;
      int py1 = node_y + node_h / 2;
      int px2 = node_x;
      int py2 = py1;  /* horizontal line */
      iupdrvDrawLine(dc, px1, py1, px2, py2, iupDrawColor(0, 0, 0, 255), IUP_DRAW_STROKE_DOT, 1);

      if (!node->brother)
      {
        px1 = node_x - (ih->data->indentation / 2);
        py1 = y;
        px2 = px1;   /* vertical line */
        py2 = node_y + node_h / 2;
        iupdrvDrawLine(dc, px1, py1, px2, py2, iupDrawColor(0, 0, 0, 255), IUP_DRAW_STROKE_DOT, 1);
      }
    }

    /* toggle */
    if (ih->data->show_toggle && node->toggle_visible)
    {
      iFlatTreeDrawToggle(ih, dc, node, node_x, node_y, node_h, fg_color, bg_color, active);
      toggle_gap = ih->data->toggle_size;
    }

    /* image */
    iupFlatDrawIcon(ih, dc, node_x + toggle_gap, node_y, image_gap - ih->data->icon_spacing, node_h,
                    IUP_IMGPOS_LEFT, ih->data->icon_spacing, IUP_ALIGN_ALEFT, IUP_ALIGN_ACENTER, 0, 0,
                    image, make_inactive, NULL, 0, 0, fore_color, back_color, active);

    title_x = node_x + toggle_gap + image_gap;

    /* title background */
    iupFlatDrawBox(dc, title_x, title_x + node->title_width - 1, node_y, node_y + node_h - 1, back_color, back_color, 1);

    /* title */
    iFlatTreeSetNodeDrawFont(ih, node, font);
    iupFlatDrawIcon(ih, dc, title_x, node_y, node->title_width, node_h,
                    IUP_IMGPOS_LEFT, ih->data->icon_spacing, IUP_ALIGN_ALEFT, IUP_ALIGN_ACENTER, 0, 0,
                    NULL, make_inactive, node->title, text_flags, 0, fore_color, back_color, active);

    /* title selection */
    if (node->selected || ih->data->dragover_pos == *pos)
    {
      unsigned char red, green, blue;
      char* hlcolor = iupAttribGetStr(ih, "HLCOLOR");
      unsigned char alpha = (unsigned char)iupAttribGetInt(ih, "HLCOLORALPHA");
      long selcolor;

      if (ih->data->dragover_pos == *pos)
        alpha = (2 * alpha) / 3;

      iupStrToRGB(hlcolor, &red, &green, &blue);
      selcolor = iupDrawColor(red, green, blue, alpha);

      iupdrvDrawRectangle(dc, title_x, node_y, title_x + node->title_width - 1, node_y + node_h - 1, selcolor, IUP_DRAW_FILL, 1);
    }

    /* title focus */
    if (ih->data->has_focus && ih->data->focus_id == node->id && focus_feedback)
      iupdrvDrawFocusRect(dc, title_x, node_y, title_x + node->title_width - 1, node_y + node_h - 1);

    /* next node */

    node_y += node_h + ih->data->spacing;

    (*pos)++;

    if (node->kind == IFLATTREE_BRANCH && node->state == IFLATTREE_EXPANDED && node->first_child)
      node_y = iFlatTreeDrawNodes(ih, dc, node->first_child, x, node_y, fg_color, bg_color, make_inactive, active, text_flags, font, focus_feedback, width, border_width, pos);

    node = node->brother;
  }

  return node_y;
}

static int iFlatTreeDrawExpander(Ihandle *ih, IdrawCanvas* dc, iFlatTreeNode *node, char *bgcolor, int x, int y)
{
  while (node)
  {
    if (node->kind == IFLATTREE_BRANCH && node->first_child)
    {
      /* TODO don't use images, just draw + and - */
      int px, py;
      char *exp = (node->state == IFLATTREE_EXPANDED) ? "IMGMINUS" : "IMGPLUS";

      py = y + ((node->height - ih->data->expander_size) / 2);
      px = x + ((node->depth - 1) * ih->data->indentation) + 1 + ((ih->data->indentation - ih->data->expander_size) / 2);

      if (node->depth > 0)
        iupdrvDrawImage(dc, exp, 0, bgcolor, px, py, 0, 0);
    }

    y += node->height + ih->data->spacing;

    if (node->kind == IFLATTREE_BRANCH && node->state == IFLATTREE_EXPANDED && node->first_child)
      y = iFlatTreeDrawExpander(ih, dc, node->first_child, bgcolor, x, y);

    node = node->brother;
  }

  return y;
}

static int iFlatTreeRedraw_CB(Ihandle* ih)
{
  const int text_flags = IUP_ALIGN_ALEFT;
  char* fg_color = iupAttribGetStr(ih, "FGCOLOR");
  char* bg_color = iupAttribGetStr(ih, "BGCOLOR");
  int posx = IupGetInt(ih, "POSX");
  int posy = IupGetInt(ih, "POSY");
  char* back_image = iupAttribGet(ih, "BACKIMAGE");
  int x, y, make_inactive = 0;
  int border_width = ih->data->border_width;
  int active = IupGetInt(ih, "ACTIVE");  /* native implementation */
  int focus_feedback = iupAttribGetBoolean(ih, "FOCUSFEEDBACK");
  iFlatTreeNode *node;
  int width, height, pos;
  char* font = IupGetAttribute(ih, "FONT");

  IdrawCanvas* dc = iupdrvDrawCreateCanvas(ih);

  iupdrvDrawGetSize(dc, &width, &height);

  iupFlatDrawBox(dc, border_width, width - border_width - 1, border_width, height - border_width - 1, bg_color, bg_color, 1);

  if (back_image)
  {
    int backimage_zoom = iupAttribGetBoolean(ih, "BACKIMAGEZOOM");
    if (backimage_zoom)
      iupdrvDrawImage(dc, back_image, 0, bg_color, border_width, border_width, width - border_width, height - border_width);
    else
      iupdrvDrawImage(dc, back_image, 0, bg_color, border_width, border_width, -1, -1);
  }

  if (!active)
    make_inactive = 1;

  x = -posx + border_width;
  y = -posy + border_width;

  node = ih->data->root_node->first_child;
  pos = 0;

  iFlatTreeDrawNodes(ih, dc, node, x, y, fg_color, bg_color, make_inactive, active, text_flags, font, focus_feedback, width, border_width, &pos);

  iFlatTreeDrawExpander(ih, dc, node, bg_color, x, y);

  if (border_width)
  {
    char* bordercolor = iupAttribGetStr(ih, "BORDERCOLOR");
    iupFlatDrawBorder(dc, 0, width - 1,
                          0, height - 1,
                          border_width, bordercolor, bg_color, active);
  }

  iupdrvDrawFlush(dc);

  iupdrvDrawKillCanvas(dc);

  return IUP_DEFAULT;
}

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
//  int txt_x, txt_y;
//  char val[1024];

//  strcpy(val, IupGetAttribute(text, "VALUE"));

//  iFlatTreeGetTitlePos(ih, ih->data->focus_id, &txt_x, &txt_y);

//  if (text->currentwidth > nodeFocus->title_width)
//    return IUP_DEFAULT;

//  iFlatTreeUpdateText(text, text->x, text->y, nodeFocus->title_width, nodeFocus->height);

//  return IUP_DEFAULT;
//}

//static void iFlatTreeCallSelectionCallback(Ihandle* ih, IFnii sel_cb, int id, int state)
//{
//  if (sel_cb(ih, id, state) == IUP_CLOSE)
//    IupExitLoop();
//}
//
//static void iFlatTreeMultipleCallActionCb(Ihandle* ih, IFnii sel_cb, IFnIi multi_cb, char* str, int count)
//{
//  if (multi_cb)
//  {
//    if (multi_cb(ih, str) == IUP_CLOSE)
//      IupExitLoop();
//  }
//  else /* sel_cb */
//  {
//    int i;
//
//    /* must simulate the click on each item */
//    for (i = 0; i < count; i++)
//    {
//      if (str[i] != 'x')
//      {
//        if (str[i] == '+')
//          iFlatTreeCallSelectionCallback(ih, sel_cb, i, 1);
//        else
//          iFlatTreeCallSelectionCallback(ih, sel_cb, i, 0);
//      }
//    }
//  }
//}
//
//static void iFlatTreeSelectNode(Ihandle* ih, int id, int ctrlPressed, int shftPressed)
//{
//  iFlatTreeNode **nodes = iupArrayGetData(ih->data->node_array);
//  IFnii sel_cb = (IFnii)IupGetCallback(ih, "SELECTION_CB");
//  IFnIi multi_cb = (IFnIi)IupGetCallback(ih, "MULTISELECTION_CB");
//  int count = iupArrayCount(ih->data->node_array);
//
//  ih->data->focus_id = id;
//
//  if (ih->data->mark_mode == IFLATTREE_MARK_MULTIPLE)
//  {
//    int i, start, end;
//    char* str;
//
//    if (id <= ih->data->last_selected_id)
//    {
//      start = id;
//      end = ih->data->last_selected_id;
//    }
//    else
//    {
//      start = ih->data->last_selected_id;
//      end = id;
//    }
//
//    str = malloc(count + 1);
//    memset(str, 'x', count); /* mark all as unchanged */
//    str[count] = 0;
//
//    if (!ctrlPressed)
//    {
//      /* un-select all */
//      for (i = 0; i < count; i++)
//      {
//        if (nodes[i]->selected)
//        {
//          str[i] = '-';
//          nodes[i]->selected = 0;
//        }
//      }
//    }
//
//    if (shftPressed)
//    {
//      /* select interval */
//      for (i = start; i <= end; i++)
//      {
//        if (!nodes[i]->selected)
//        {
//          str[i] = '+';
//          nodes[i]->selected = 1;
//        }
//      }
//    }
//    else
//    {
//      if (ctrlPressed)
//      {
//        /* toggle selection */
//        if (nodes[id]->selected)
//        {
//          str[id] = '-';
//          nodes[id]->selected = 0;
//        }
//        else
//        {
//          str[id] = '+';
//          nodes[id]->selected = 1;
//        }
//      }
//      else
//      {
//        if (!nodes[id]->selected)
//        {
//          str[id] = '+';
//          nodes[id]->selected = 1;
//        }
//      }
//    }
//
//    if (multi_cb || sel_cb)
//      iFlatTreeMultipleCallActionCb(ih, sel_cb, multi_cb, str, count);
//
//    free(str);
//  }
//  else
//  {
//    int i, old_id = -1;
//
//    for (i = 0; i < count; i++)
//    {
//      if (!nodes[i]->selected)
//        continue;
//      nodes[i]->selected = 0;
//      old_id = i;
//      break;
//    }
//
//    nodes[id]->selected = 1;
//
//    if (sel_cb)
//    {
//      if (old_id != -1)
//      {
//        if (old_id != id)
//        {
//          iFlatTreeCallSelectionCallback(ih, sel_cb, old_id, 0);
//          iFlatTreeCallSelectionCallback(ih, sel_cb, id, 1);
//        }
//      }
//      else
//        iFlatTreeCallSelectionCallback(ih, sel_cb, id, 1);
//    }
//  }
//
//  if (!shftPressed)
//    ih->data->last_selected_id = id;
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

//static int iFlatTreeHitToggle(Ihandle *ih, int x, int y, int id, int depth)
//{
//  int h;
//  int py = iFlatTreeConvertIdToY(ih, id, &h) + ((h - ih->data->toggle_size) / 2);
//  int px = (depth * ih->data->indentation) + 1 + ((ih->data->indentation - ih->data->toggle_size) / 2);
//  int posx = IupGetInt(ih, "POSX");
//  int posy = IupGetInt(ih, "POSY");

//  x += posx - ih->data->border_width;
//  y += posy - ih->data->border_width;

//  if (x > px && x < px + ih->data->toggle_size && y > py && y < py + ih->data->toggle_size)
//    return 1;

//  return 0;
//}

//static int iFlatTreeHitExpander(Ihandle *ih, int x, int y, int id, int depth)
//{
//  int h;
//  int py = iFlatTreeConvertIdToY(ih, id, &h) + ((h - ih->data->expander_size) / 2);
//  int px = ((depth - 1) * ih->data->indentation) + 1 + ((ih->data->indentation - ih->data->expander_size) / 2);
//  int posx = IupGetInt(ih, "POSX");
//  int posy = IupGetInt(ih, "POSY");

//  x += posx - ih->data->border_width;
//  y += posy - ih->data->border_width;

//  if (x > px && x < px + ih->data->expander_size && y > py && y < py + ih->data->expander_size)
//    return 1;

//  return 0;
//}

//static int iFlatTreeButton_CB(Ihandle* ih, int button, int pressed, int x, int y, char* status)
//{
//  iFlatTreeNode *node;
//  IFniiiis button_cb = (IFniiiis)IupGetCallback(ih, "FLAT_BUTTON_CB");
//  char *image;
//  int pos = iFlatTreeConvertXYToPos(ih, x, y);
//  int id, width, height, xmin, xmax;
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
//      iFlatTreeSelectNode(ih, droppedNode->id, 0, 0); /* force no ctrl and no shift for selection */
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

//  xmin = (node->depth * ih->data->indentation) + toggle_gap;
//  xmax = xmin + img_w + node->title_width;

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
//        if (node->state == IFLATTREE_EXPANDED)
//          node->state = IFLATTREE_COLLAPSED;
//        else
//          node->state = IFLATTREE_EXPANDED;

//        iFlatTreeUpdateNodePos(ih, id);
//      }
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
//    else if (iFlatTreeHitExpander(ih, x, y, id))
//    {
//      if (node->state == IFLATTREE_EXPANDED)
//        node->state = IFLATTREE_COLLAPSED;
//      else
//        node->state = IFLATTREE_EXPANDED;

//      iFlatTreeRebuildSize(ih);
//    }
//    else if (iFlatTreeHitToggle(ih, x, y, id))
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
//          int i, count = iupArrayCount((ih->data->node_array));

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

static int iFlatTreeResize_CB(Ihandle* ih, int width, int height)
{
  (void)width;
  (void)height;

  iFlatTreeUpdateScrollBar(ih);

  return IUP_DEFAULT;
}

//static void iFlatTreeScrollFocusVisible(Ihandle* ih, int direction)
//{
//  int focus_y;
//  iFlatTreeNode *node;
//  int posy;
//  int dy = IupGetInt(ih, "DY");
//  int ymin = IupGetInt(ih, "YMIN");
//  int ymax = IupGetInt(ih, "YMAX");
//  int w;
//
//  if (dy >= (ymax - ymin))
//    return;
//
//  focus_y = iFlatTreeConvertIdToY(ih, ih->data->focus_id, NULL);
//  node = iFlatTreeGetNode(ih, ih->data->focus_id);
//  posy = IupGetInt(ih, "POSY");
//
//  if (focus_y > posy && (focus_y + node->height) < (posy + dy))
//  {
//    posy = focus_y + node->height - dy;
//    IupSetInt(ih, "POSY", posy);
//  }
//  else if (direction == IFLATTREE_DOWN)
//  {
//    posy += (focus_y - posy - dy + node->height);
//    IupSetInt(ih, "POSY", posy);
//  }
//  else
//  {
//    posy -= (posy - focus_y);
//    IupSetInt(ih, "POSY", posy);
//  }
//}
//
//static int iFlatTreeKCr_CB(Ihandle* ih)
//{
//  if (ih->data->has_focus)
//  {
//    if (ih->data->focus_id >= 0)
//    {
//      iFlatTreeNode *node = iFlatTreeGetNode(ih, ih->data->focus_id);
//
//      if (node->kind == IFLATTREE_BRANCH)
//      {
//        if (node->state == IFLATTREE_EXPANDED)
//          node->state = IFLATTREE_COLLAPSED;
//        else
//          node->state = IFLATTREE_EXPANDED;

//        iFlatTreeUpdateNodePos(ih, ih->data->focus_id);
//      }
//      else
//      {
//        IFnis cb = (IFnis)IupGetCallback(ih, "EXECUTELEAF_CB");
//        if (cb)
//        {
//          if (cb(ih, ih->data->focus_id, node->title) == IUP_IGNORE)
//            return IUP_DEFAULT;
//        }
//      }
//
//      iFlatTreeScrollFocusVisible(ih, IFLATTREE_DOWN);
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
//  int count = iupArrayCount(ih->data->node_array);
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
//  Call Selection_CB OLD
//    iFlatTreeClearAllSelectionExcept(ih, node);

//  node->selected = !node->selected;
//  Call Selection_CB

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

//static void iFlatTreeDragDropCopyNode(Ihandle* ih_src, Ihandle *ih, iFlatTreeNode *srcNode, iFlatTreeNode *dstNode, int isControl)
//{
//  iFlatTreeNode *newNode;
//  int id_dst;

//  id_dst = dstNode->id;

//  if (isControl)
//    newNode = iFlatTreeCloneNode(srcNode);    /* Copy */
//  else
//  {
//    iFlatTreeUnlinkNodeFromParent(srcNode);  /* Move */
//    newNode = srcNode;
//  }

//  if (dstNode->kind == IFLATTREE_BRANCH && dstNode->state == IFLATTREE_EXPANDED)
//  {
//    /* copy as first child of expanded branch */
//    newNode->parent = dstNode;
//    newNode->brother = dstNode->first_child;
//    dstNode->first_child = newNode;
//  }
//  else
//  {
//    newNode->parent = dstNode->parent;
//    newNode->brother = dstNode->brother;
//    dstNode->brother = newNode;
//  }

//  iFlatTreeRebuildArray(ih, +1);
//  iFlatTreeUpdateNodeSize(ih, newNode);
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

//    iFlatTreeUpdateScrollBar(ih);
//    IupUpdate(ih);

//    iFlatTreeUpdateScrollBar(ih_source);
//    IupUpdate(ih_source);

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

static void iFlatTreeRedraw(Ihandle* ih)
{
  if (ih->data->auto_redraw)
    IupRedraw(ih, 0);
}

static char* iFlatTreeGetAutoRedrawAttrib(Ihandle* ih)
{
  return iupStrReturnBoolean(ih->data->auto_redraw);
}

static int iFlatTreeSetAutoRedrawAttrib(Ihandle* ih, const char* value)
{
  if (iupStrBoolean(value))
    ih->data->auto_redraw = 1;
  else
    ih->data->auto_redraw = 0;

  return 0;
}

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
  iFlatTreeUpdateNodeSizeAll(ih);
  iFlatTreeUpdateScrollBar(ih);
  iFlatTreeRedraw(ih);
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

  iFlatTreeUpdateNodeSizeAll(ih);
  iFlatTreeUpdateScrollBar(ih);
  iFlatTreeRedraw(ih);
  return 0;
}

static int iFlatTreeSetSpacingAttrib(Ihandle* ih, const char* value)
{
  iupStrToInt(value, &ih->data->spacing);
  iFlatTreeUpdateNodeSizeAll(ih);
  iFlatTreeUpdateScrollBar(ih);
  iFlatTreeRedraw(ih);
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

  if (node->state == IFLATTREE_EXPANDED)
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
    node->state = IFLATTREE_EXPANDED;
  else /* "HORIZONTAL" */
    node->state = IFLATTREE_COLLAPSED;

  iFlatTreeUpdateScrollBar(ih);
  iFlatTreeRedraw(ih);
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

  if (node->parent->id == -1)
    return NULL;

  return iupStrReturnInt(node->parent->id);
}

static char* iFlatTreeGetNextAttrib(Ihandle* ih, int id)
{
  iFlatTreeNode *node = iFlatTreeGetNode(ih, id);
  if (!node || !node->brother)
    return NULL;

  return iupStrReturnInt(node->brother->id);
}

static char* iFlatTreeGetPreviousAttrib(Ihandle* ih, int id)
{
  iFlatTreeNode *node = iFlatTreeGetNode(ih, id);
  iFlatTreeNode *brother;
  if (!node)
    return NULL;

  brother = node->parent->first_child;

  if (brother == node)
    return NULL;

  while (brother->brother != node)
    brother = brother->brother;

  return iupStrReturnInt(brother->id);
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

  return iupStrReturnInt(nodeLast->id);
}

static char* iFlatTreeGetFirstAttrib(Ihandle* ih, int id)
{
  iFlatTreeNode *node = iFlatTreeGetNode(ih, id);
  if (!node)
    return NULL;

  return iupStrReturnInt(node->parent->first_child->id);
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

  iFlatTreeUpdateNodeSize(ih, node);
  iFlatTreeUpdateScrollBar(ih);
  iFlatTreeRedraw(ih);
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

  iFlatTreeUpdateNodeSize(ih, node);
  iFlatTreeUpdateScrollBar(ih);
  iFlatTreeRedraw(ih);
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

  iFlatTreeRedraw(ih);
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

  iFlatTreeUpdateNodeSize(ih, node);
  iFlatTreeUpdateScrollBar(ih);
  iFlatTreeRedraw(ih);
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

static char* iFlatTreeGetLastAddNodeAttrib(Ihandle* ih)
{
  return iupStrReturnInt(ih->data->last_add_node);
}

static int iFlatTreeSetAddLeafAttrib(Ihandle* ih, int id, const char* value)
{
  iFlatTreeAddNode(ih, id, IFLATTREE_LEAF, value);

  iFlatTreeUpdateScrollBar(ih);
  iFlatTreeRedraw(ih);
  return 0;
}

static int iFlatTreeSetAddBranchAttrib(Ihandle* ih, int id, const char* value)
{
  iFlatTreeAddNode(ih, id, IFLATTREE_BRANCH, value);

  iFlatTreeUpdateScrollBar(ih);
  iFlatTreeRedraw(ih);
  return 0;
}

static int iFlatTreeSetInsertLeafAttrib(Ihandle* ih, int id, const char* value)
{
  iFlatTreeInsertNode(ih, id, IFLATTREE_LEAF, value);

  iFlatTreeUpdateScrollBar(ih);
  iFlatTreeRedraw(ih);
  return 0;
}

static int iFlatTreeSetInsertBranchAttrib(Ihandle* ih, int id, const char* value)
{
  iFlatTreeInsertNode(ih, id, IFLATTREE_BRANCH, value);

  iFlatTreeUpdateScrollBar(ih);
  iFlatTreeRedraw(ih);
  return 0;
}

static void iFlatTreeRemoveMarkedNodesRec(Ihandle *ih, iFlatTreeNode *node)
{
  while (node)
  {
    if (node->selected)
    {
      iFlatTreeNode *brother = node->brother;
      iFlatTreeRemoveNodeRec(node);
      node = brother;
    }
    else if (node->kind == IFLATTREE_BRANCH)
    {
      iFlatTreeRemoveMarkedNodesRec(ih, node->first_child);
      node = node->brother;
    }
    else
      node = node->brother;
  }
}

static int iFlatTreeGetChildCountRec(iFlatTreeNode *node);

static int iFlatTreeSetDelNodeAttrib(Ihandle* ih, int id, const char* value)
{
  int update = 0;

  if (iupStrEqualNoCase(value, "ALL"))
  {
    iFlatTreeNode *child = ih->data->root_node->first_child;
    while (child)
    {
      iFlatTreeNode *brother = child->brother;
      iFlatTreeRemoveNodeRec(child);
      child = brother;
    }
    ih->data->root_node->first_child = NULL;
    iFlatTreeRebuildArray(ih, 0);
    update = 1;
  }

  if (iupStrEqualNoCase(value, "SELECTED")) /* selected here means the reference one */
  {
    iFlatTreeNode *node = iFlatTreeGetNode(ih, id);
    if (node)
    {
      int count = iFlatTreeGetChildCountRec(node) + 1;
      iFlatTreeUnlinkNodeFromParent(node);
      iFlatTreeRemoveNodeRec(node);
      iFlatTreeRebuildArray(ih, -count);
      update = 1;
    }
  }
  else if (iupStrEqualNoCase(value, "CHILDREN"))  /* children of the reference node */
  {
    iFlatTreeNode *node = iFlatTreeGetNode(ih, id);
    if (node)
    {
      int count = 0;
      iFlatTreeNode *child = node->first_child;
      while (child)
      {
        iFlatTreeNode *brother = child->brother;
        count += iFlatTreeGetChildCountRec(child) + 1;
        iFlatTreeRemoveNodeRec(child);
        child = brother;
      }
      node->first_child = NULL;
      iFlatTreeRebuildArray(ih, -count);
      update = 1;
    }
  }
  else if (iupStrEqualNoCase(value, "MARKED"))
  {
    iFlatTreeRemoveMarkedNodesRec(ih, ih->data->root_node->first_child);
    iFlatTreeRebuildArray(ih, 0);
    update = 1;
  }

  if (update)
  {
    iFlatTreeUpdateScrollBar(ih);
    iFlatTreeRedraw(ih);
  }

  return 0;
}

static int iFlatTreeSetExpandAllAttrib(Ihandle* ih, const char* value)
{
  int count = iupArrayCount(ih->data->node_array);
  iFlatTreeNode **nodes = iupArrayGetData(ih->data->node_array);
  int i, state = iupStrBoolean(value)? IFLATTREE_EXPANDED: IFLATTREE_COLLAPSED;

  for (i = 0; i < count; i++)
  {
    if (nodes[i]->kind == IFLATTREE_LEAF)
      continue;

    nodes[i]->state = state;
  }

  iFlatTreeUpdateScrollBar(ih);
  iFlatTreeRedraw(ih);
  return 0;
}

static int iFlatTreeSetMoveNodeAttrib(Ihandle* ih, int id, const char* value)
{
  int dstId = IUP_INVALID_ID;
  if (iupStrToInt(value, &dstId))
  {
    iFlatTreeMoveNode(ih, id, dstId);

    iFlatTreeUpdateScrollBar(ih);
    iFlatTreeRedraw(ih);
  }

  return 0;
}

static int iFlatTreeSetCopyNodeAttrib(Ihandle* ih, int id, const char* value)
{
  int dstId = IUP_INVALID_ID;
  if (iupStrToInt(value, &dstId))
  {
    iFlatTreeCopyNode(ih, id, dstId);

    iFlatTreeUpdateScrollBar(ih);
    iFlatTreeRedraw(ih);
  }

  return 0;
}

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
//      int count = iupArrayCount(ih->data->node_array);
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
//  iFlatTreeRedraw(ih);
//  return 0;
//}

static char* iFlatTreeGetValueAttrib(Ihandle* ih)
{
  int count = iupArrayCount(ih->data->node_array);
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

  iFlatTreeRedraw(ih);
  return 0;
}

static int iFlatTreeSetMarkStartAttrib(Ihandle* ih, const char* value)
{
  int id;
  if (iupStrToInt(value, &id))
  {
    int count = iupArrayCount(ih->data->node_array);

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

  iFlatTreeRedraw(ih);
  return 0;
}

static char* iFlatTreeGetMarkedNodesAttrib(Ihandle* ih)
{
  iFlatTreeNode **nodes = iupArrayGetData(ih->data->node_array);
  int count = iupArrayCount(ih->data->node_array);
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

  count = iupArrayCount(ih->data->node_array);
  nodes = iupArrayGetData(ih->data->node_array);

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

  iFlatTreeRedraw(ih);
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

  iFlatTreeRedraw(ih);
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

  iFlatTreeUpdateNodeSize(ih, node);
  iFlatTreeUpdateScrollBar(ih);
  iFlatTreeRedraw(ih);
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

  iFlatTreeUpdateScrollBar(ih);
  iFlatTreeRedraw(ih);
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

static char* iFlatTreeGetShowDragDropAttrib(Ihandle* ih)
{
  return iupStrReturnBoolean(ih->data->show_dragdrop);
}

static int iFlatTreeSetShowDragDropAttrib(Ihandle* ih, const char* value)
{
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
  iFlatTreeUpdateNodeSizeAll(ih);
  iFlatTreeUpdateScrollBar(ih);
  iFlatTreeRedraw(ih);
  return 0;
}

static char* iFlatTreeGetIconSpacingAttrib(Ihandle *ih)
{
  return iupStrReturnInt(ih->data->icon_spacing);
}

static char* iFlatTreeGetCountAttrib(Ihandle* ih)
{
  return iupStrReturnInt(iupArrayCount(ih->data->node_array));
}

static char* iFlatTreeGetChildCountAttrib(Ihandle* ih, int id)
{
  int count;
  iFlatTreeNode *child;
  iFlatTreeNode *node = iFlatTreeGetNode(ih, id);
  if (!node)
    return NULL;

  count = 0;
  child = node->first_child;
  while (child != NULL)
  {
    count++;
    child = child->brother;
  }

  return iupStrReturnInt(count);
}

static int iFlatTreeGetChildCountRec(iFlatTreeNode *node)
{
  int count = 0;
  iFlatTreeNode *child = node->first_child;

  while (child)
  {
    if (child->first_child)
      count += iFlatTreeGetChildCountRec(child->first_child);
    count++;
    child = child->brother;
  }

  return count;
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
  iFlatTreeNode *node = iFlatTreeGetNode(ih, id);
  if (!node)
    return NULL;

  return iupStrReturnInt(node->depth);
}

static char* iFlatTreeGetColorAttrib(Ihandle* ih, int id)
{
  iFlatTreeNode *node = iFlatTreeGetNode(ih, id);
  if (!node)
    return NULL;

  return node->fg_color;
}

static int iFlatTreeSetColorAttrib(Ihandle* ih, int id, const char* value)
{
  iFlatTreeNode *node = iFlatTreeGetNode(ih, id);
  if (!node)
    return 0;

  if (node->fg_color)
    free(node->fg_color);
  node->fg_color = iupStrDup(value);

  iFlatTreeRedraw(ih);
  return 0;
}

static char* iFlatTreeGetBackColorAttrib(Ihandle* ih, int id)
{
  iFlatTreeNode *node = iFlatTreeGetNode(ih, id);
  if (!node)
    return NULL;

  return node->bg_color;
}

static int iFlatTreeSetBackColorAttrib(Ihandle* ih, int id, const char* value)
{
  iFlatTreeNode *node = iFlatTreeGetNode(ih, id);
  if (!node)
    return 0;

  if (node->bg_color)
    free(node->bg_color);
  node->bg_color = iupStrDup(value);

  iFlatTreeRedraw(ih);
  return 0;
}

static int iFlatTreeSetTopItemAttrib(Ihandle* ih, const char* value)
{
  int id = 0;
  if (iupStrToInt(value, &id))
  {
    int count = iupArrayCount(ih->data->node_array);
    int posy = 0;

    if (id < 0 || id > count-1)
      return 0;

//    posy = iFlatTreeConvertIdToY(ih, id, NULL);
    IupSetInt(ih, "POSY", posy);

    iFlatTreeRedraw(ih);
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
  iFlatTreeUpdateScrollBar(ih);
  iFlatTreeRedraw(ih);
  return 0;
}

static char* iFlatTreeGetBorderWidthAttrib(Ihandle *ih)
{
  return iupStrReturnInt(ih->data->border_width);
}

static int iFlatTreeSetAttribPostRedraw(Ihandle* ih, const char* value)
{
  (void)value;
  IupUpdate(ih);
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
  if (ih->data->root_node->first_child)
    iFlatTreeRemoveNodeRec(ih->data->root_node->first_child);

  iupArrayDestroy(ih->data->node_array);

  free(ih->data->root_node);
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

  /* non zero default values */
  ih->data->auto_redraw = 1;
  ih->data->icon_spacing = 2;
  ih->data->add_expanded = 1;
  ih->data->indentation = (iupRound(iupdrvGetScreenDpi()) > 120) ? 24 : 16;
  ih->data->toggle_size = ih->data->indentation;
  ih->data->dragover_pos = -1;
  ih->data->expander_size = 9;
 // ih->data->min_clock = 500;

  ih->data->root_node = (iFlatTreeNode*)malloc(sizeof(iFlatTreeNode));
  memset(ih->data->root_node, 0, sizeof(iFlatTreeNode));
  ih->data->root_node->id = -1;

  ih->data->node_array = iupArrayCreate(10, sizeof(iFlatTreeNode*));

  /* internal callbacks */
  IupSetCallback(ih, "ACTION", (Icallback)iFlatTreeRedraw_CB);
  //IupSetCallback(ih, "BUTTON_CB", (Icallback)iFlatTreeButton_CB);
  //IupSetCallback(ih, "MOTION_CB", (Icallback)iFlatTreeMotion_CB);
  IupSetCallback(ih, "RESIZE_CB", (Icallback)iFlatTreeResize_CB);
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
  iupClassRegisterAttribute(ic, "FGCOLOR", NULL, iFlatTreeSetAttribPostRedraw, IUP_FLAT_FORECOLOR, NULL, IUPAF_NOT_MAPPED);  /* force the new default value */
  iupClassRegisterAttribute(ic, "BGCOLOR", NULL, iFlatTreeSetAttribPostRedraw, IUP_FLAT_BACKCOLOR, NULL, IUPAF_NOT_MAPPED);  /* force the new default value */
  iupClassRegisterAttribute(ic, "HLCOLOR", NULL, NULL, IUPAF_SAMEASSYSTEM, "TXTHLCOLOR", IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "HLCOLORALPHA", NULL, NULL, IUPAF_SAMEASSYSTEM, "128", IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "INDENTATION", iFlatTreeGetIndentationAttrib, iFlatTreeSetIndentationAttrib, NULL, NULL, IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "SHOWTOGGLE", iFlatTreeGetShowToggleAttrib, iFlatTreeSetShowToggleAttrib, NULL, NULL, IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "SPACING", iFlatTreeGetSpacingAttrib, iFlatTreeSetSpacingAttrib, IUPAF_SAMEASSYSTEM, "0", IUPAF_NO_INHERIT | IUPAF_NOT_MAPPED);
  iupClassRegisterAttribute(ic, "SHOWRENAME", iFlatTreeGetShowRenameAttrib, iFlatTreeSetShowRenameAttrib, NULL, NULL, IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AUTOREDRAW", iFlatTreeGetAutoRedrawAttrib, iFlatTreeSetAutoRedrawAttrib, IUPAF_SAMEASSYSTEM, "Yes", IUPAF_NOT_MAPPED | IUPAF_WRITEONLY | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "BORDERCOLOR", NULL, NULL, IUPAF_SAMEASSYSTEM, IUP_FLAT_BORDERCOLOR, IUPAF_NOT_MAPPED | IUPAF_DEFAULT);  /* inheritable */
  iupClassRegisterAttribute(ic, "BORDERWIDTH", iFlatTreeGetBorderWidthAttrib, iFlatTreeSetBorderWidthAttrib, IUPAF_SAMEASSYSTEM, "0", IUPAF_NOT_MAPPED);  /* inheritable */
  iupClassRegisterAttribute(ic, "ICONSPACING", iFlatTreeGetIconSpacingAttrib, iFlatTreeSetIconSpacingAttrib, IUPAF_SAMEASSYSTEM, "2", IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "VISIBLECOLUMNS", NULL, NULL, NULL, NULL, IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "VISIBLELINES", NULL, NULL, "5", NULL, IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);

  /* IupTree Attributes - ACTION */
  iupClassRegisterAttribute(ic, "TOPITEM", NULL, iFlatTreeSetTopItemAttrib, NULL, NULL, IUPAF_NOT_MAPPED | IUPAF_WRITEONLY | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "EXPANDALL", NULL, iFlatTreeSetExpandAllAttrib, NULL, NULL, IUPAF_NOT_MAPPED | IUPAF_WRITEONLY | IUPAF_NO_INHERIT);
  //iupClassRegisterAttribute(ic, "RENAME", NULL, iFlatTreeSetRenameAttrib, NULL, NULL, IUPAF_NOT_MAPPED | IUPAF_WRITEONLY | IUPAF_NO_INHERIT);

  /* IupFlatTree Attributes - NODES */

  iupClassRegisterAttribute(ic, "COUNT", iFlatTreeGetCountAttrib, NULL, NULL, NULL, IUPAF_READONLY | IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "ROOTCOUNT", iFlatTreeGetRootCountAttrib, NULL, NULL, NULL, IUPAF_NOT_MAPPED | IUPAF_READONLY | IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "CHILDCOUNT", iFlatTreeGetChildCountAttrib, NULL, IUPAF_NOT_MAPPED | IUPAF_READONLY | IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "TOTALCHILDCOUNT", iFlatTreeGetTotalChildCountAttrib, NULL, IUPAF_NOT_MAPPED | IUPAF_READONLY | IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "COLOR", iFlatTreeGetColorAttrib, iFlatTreeSetColorAttrib, IUPAF_NO_INHERIT | IUPAF_NOT_MAPPED);
  iupClassRegisterAttributeId(ic, "BACKCOLOR", iFlatTreeGetBackColorAttrib, iFlatTreeSetBackColorAttrib, IUPAF_NO_INHERIT | IUPAF_NOT_MAPPED);
  iupClassRegisterAttributeId(ic, "DEPTH", iFlatTreeGetDepthAttrib, NULL, IUPAF_NOT_MAPPED | IUPAF_READONLY | IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "KIND", iFlatTreeGetKindAttrib, NULL, IUPAF_NOT_MAPPED | IUPAF_READONLY | IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "PARENT", iFlatTreeGetParentAttrib, NULL, IUPAF_NOT_MAPPED | IUPAF_READONLY | IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "NEXT", iFlatTreeGetNextAttrib, NULL, IUPAF_NOT_MAPPED | IUPAF_READONLY | IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "PREVIOUS", iFlatTreeGetPreviousAttrib, NULL, IUPAF_NOT_MAPPED | IUPAF_READONLY | IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "LAST", iFlatTreeGetLastAttrib, NULL, IUPAF_NOT_MAPPED | IUPAF_READONLY | IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "FIRST", iFlatTreeGetFirstAttrib, NULL, IUPAF_NOT_MAPPED | IUPAF_READONLY | IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "STATE", iFlatTreeGetStateAttrib, iFlatTreeSetStateAttrib, IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "TITLE", iFlatTreeGetTitleAttrib, iFlatTreeSetTitleAttrib, IUPAF_NO_INHERIT | IUPAF_NOT_MAPPED);
  iupClassRegisterAttributeId(ic, "TITLEFONT", iFlatTreeGetTitleFontAttrib, iFlatTreeSetTitleFontAttrib, IUPAF_NO_INHERIT | IUPAF_NOT_MAPPED);
  iupClassRegisterAttributeId(ic, "TITLEFONTSTYLE", iFlatTreeGetTitleFontStyleAttrib, iFlatTreeSetTitleFontStyleAttrib, IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "TITLEFONTSIZE", iFlatTreeGetTitleFontSizeAttrib, iFlatTreeSetTitleFontSizeAttrib, IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "TOGGLEVALUE", iFlatTreeGetToggleValueAttrib, iFlatTreeSetToggleValueAttrib, IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "TOGGLEVISIBLE", iFlatTreeGetToggleVisibleAttrib, iFlatTreeSetToggleVisibleAttrib, IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "USERDATA", iFlatTreeGetUserDataAttrib, iFlatTreeSetUserDataAttrib, IUPAF_NOT_MAPPED | IUPAF_NO_STRING | IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "IMAGE", iFlatTreeGetImageAttrib, iFlatTreeSetImageAttrib, IUPAF_IHANDLENAME | IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "IMAGEEXPANDED", iFlatTreeGetImageExpandedAttrib, iFlatTreeSetImageExpandedAttrib, IUPAF_IHANDLENAME | IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);

  /* IupFlatTree Attributes - IMAGES */

  iupClassRegisterAttribute(ic, "IMAGELEAF", NULL, iFlatTreeSetAttribPostRedraw, IUPAF_SAMEASSYSTEM, "IMGLEAF", IUPAF_NOT_MAPPED | IUPAF_IHANDLENAME | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "IMAGEBRANCHCOLLAPSED", NULL, iFlatTreeSetAttribPostRedraw, IUPAF_SAMEASSYSTEM, "IMGCOLLAPSED", IUPAF_NOT_MAPPED | IUPAF_IHANDLENAME | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "IMAGEBRANCHEXPANDED", NULL, iFlatTreeSetAttribPostRedraw, IUPAF_SAMEASSYSTEM, "IMGEXPANDED", IUPAF_NOT_MAPPED | IUPAF_IHANDLENAME | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "BACKIMAGE", NULL, iFlatTreeSetAttribPostRedraw, NULL, NULL, IUPAF_NOT_MAPPED | IUPAF_IHANDLENAME | IUPAF_NO_DEFAULTVALUE | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "BACKIMAGEZOOM", NULL, iFlatTreeSetAttribPostRedraw, NULL, NULL, IUPAF_NO_INHERIT);

  /* IupFlatTree Attributes - FOCUS NODE */

//  iupClassRegisterAttribute(ic, "VALUE", iFlatTreeGetValueAttrib, iFlatTreeSetValueAttrib, NULL, NULL, IUPAF_NOT_MAPPED | IUPAF_NO_SAVE | IUPAF_NO_DEFAULTVALUE | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "HASFOCUS", iFlatTreeGetHasFocusAttrib, NULL, NULL, NULL, IUPAF_NOT_MAPPED | IUPAF_READONLY | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "FOCUSFEEDBACK", NULL, NULL, IUPAF_SAMEASSYSTEM, "YES", IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);

  /* IupFlatTree Attributes - MARKS */

  iupClassRegisterAttribute(ic, "MARK", NULL, iFlatTreeSetMarkAttrib, NULL, NULL, IUPAF_NOT_MAPPED | IUPAF_WRITEONLY | IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "MARKED", iFlatTreeGetMarkedAttrib, iFlatTreeSetMarkedAttrib, IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "MARKEDNODES", iFlatTreeGetMarkedNodesAttrib, iFlatTreeSetMarkedNodesAttrib, NULL, NULL, IUPAF_NOT_MAPPED | IUPAF_NO_SAVE | IUPAF_NO_DEFAULTVALUE | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "MARKMODE", iFlatTreeGetMarkModeAttrib, iFlatTreeSetMarkModeAttrib, IUPAF_SAMEASSYSTEM, "SINGLE", IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "MARKSTART", iFlatTreeGetMarkStartAttrib, iFlatTreeSetMarkStartAttrib, NULL, NULL, IUPAF_NOT_MAPPED | IUPAF_NO_DEFAULTVALUE | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "MARKWHENTOGGLE", NULL, NULL, NULL, NULL, IUPAF_NO_INHERIT);

  /* IupFlatTree Attributes - HIERARCHY */

  iupClassRegisterAttributeId(ic, "ADDLEAF", NULL, iFlatTreeSetAddLeafAttrib, IUPAF_WRITEONLY | IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "ADDBRANCH", NULL, iFlatTreeSetAddBranchAttrib, IUPAF_WRITEONLY | IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "INSERTLEAF", NULL, iFlatTreeSetInsertLeafAttrib, IUPAF_WRITEONLY | IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "INSERTBRANCH", NULL, iFlatTreeSetInsertBranchAttrib, IUPAF_WRITEONLY | IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "COPYNODE", NULL, iFlatTreeSetCopyNodeAttrib, IUPAF_NOT_MAPPED | IUPAF_WRITEONLY | IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "DELNODE", NULL, iFlatTreeSetDelNodeAttrib, IUPAF_WRITEONLY | IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "MOVENODE", NULL, iFlatTreeSetMoveNodeAttrib, IUPAF_NOT_MAPPED | IUPAF_WRITEONLY | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "LASTADDNODE", iFlatTreeGetLastAddNodeAttrib, NULL, IUPAF_SAMEASSYSTEM, NULL, IUPAF_NOT_MAPPED | IUPAF_READONLY | IUPAF_NO_INHERIT);

  iupClassRegisterAttribute(ic, "SHOWDRAGDROP", iFlatTreeGetShowDragDropAttrib, iFlatTreeSetShowDragDropAttrib, NULL, NULL, IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "DRAGDROPTREE", NULL, iFlatTreeSetDragDropTreeAttrib, NULL, NULL, IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);

  iupClassRegisterReplaceAttribDef(ic, "SCROLLBAR", "YES", NULL);  /* change the default to Yes */
  iupClassRegisterAttribute(ic, "YAUTOHIDE", NULL, NULL, IUPAF_SAMEASSYSTEM, "YES", IUPAF_READONLY | IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);  /* will be always Yes */
  iupClassRegisterAttribute(ic, "XAUTOHIDE", NULL, NULL, IUPAF_SAMEASSYSTEM, "YES", IUPAF_READONLY | IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);  /* will be always Yes */

  iFlatTreeInitializeImages();

  /* Flat Scrollbar */
  iupFlatScrollBarRegister(ic);

  iupClassRegisterAttribute(ic, "FLATSCROLLBAR", NULL, iFlatTreeSetFlatScrollbarAttrib, NULL, NULL, IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);

  return ic;
}

/* TODO:
  doc - ADDROOT is always NO
  ----------------------
  DROPEQUALDRAG ??
  EMPTYTOGGLE - EMPTYAS3STATE equivalent
  _IUP_XY2POS_CB
  ----------------------
  TOGGLESIZE
  EXPANDERSIZE
  SHOWLINES / HIDELINES
  SHOWEXPANDER / HIDEBUTTONS
  LINECOLOR
  NODETIP
  NODEACTIVE
  NODEVISIBLE
*/
