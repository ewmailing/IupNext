/** \file
 * \brief iuptree control
 * Functions to set and get attributes
 *
 * See Copyright Notice in iup.h
 */

#include <stdio.h>
#include <stdlib.h> /* malloc, realloc */
#include <string.h>
#include <math.h>

#include "iup.h"
#include "iupcbs.h"
#include "iuptree.h"
#include "iupkey.h"

#include <cd.h>
#include <cdiup.h>
#include <cddbuf.h>

#include "iup_object.h"
#include "iup_attrib.h"
#include "iup_str.h"
#include "iup_drv.h"
#include "iup_drvfont.h"
#include "iup_stdcontrols.h"
#include "iup_controls.h"
#include "iup_cdutil.h"

#include "itdraw.h"
#include "itdef.h"
#include "itgetset.h"
#include "itfind.h"
#include "itcallback.h"
#include "itscroll.h"

#define NO      0
#define YES     1

extern int tree_ctrl;
extern int tree_shift;


/**************************************************************************
*   Sets and Gets tree attributes
***************************************************************************/

/* Sets the created branch state. It can be created collapsed ("NO") or 
   expanded ("YES").
*/
int iTreeGSAddExpanded(Ihandle* ih, const char* value)
{
  if(iupStrEqualNoCase(value, "YES"))
    ih->data->addexpanded = YES;
  if(iupStrEqualNoCase(value, "NO"))
    ih->data->addexpanded = NO;
  else
    return 0;
  
  return 1;
}

/* Adds a node at the specified path. If the path does not exist, does nothing.
   - id   : node identifier
   - kind : node kind (ITREE_LEAF or ITREE_BRANCH)
*/
int iTreeGSAddNode(Ihandle* ih, const char* id_string, int kind)
{
  Node node = iTreeFindNodeFromString(ih, id_string);

  if(node)
  {
    iTreeGSCreateNode(ih, node, kind);
    return 1;
  }
  
  return 0;
}

/* Creates a node in the place pointed by selected. The new node is pointed
   by the selected node and points to the node following the selected node.

   |--|  -->  |--|  -->   |--|
 selected    new     node following
   node      node    selected node

   - selected: pointer to selected node
   - kind    : node kind (ITREE_LEAF or ITREE_BRANCH)  
*/
int iTreeGSCreateNode(Ihandle* ih, Node selected, int kind)
{
  Node newnode;

  if(selected == NULL)
    return 0;

  newnode = (Node)malloc(sizeof(struct Node_));
  if(newnode == NULL)
    return 0;

  /* Initializes new node's depth */
  newnode->depth = selected->depth + (selected->kind == ITREE_BRANCH);

  /* Node kind */
  newnode->kind = kind;

  /* Branch state depends on the attribute addexpanded */
  if(ih->data->addexpanded == YES)
    newnode->state = ITREE_EXPANDED;
  else
    newnode->state = ITREE_COLLAPSED;

  newnode->visible = YES;   /* Node is visible by default    */
  newnode->marked  = NO;    /* Node is not marked by default */
  newnode->name    = NULL;  /* Node value is NULL by default */
  
  /* New node points to the node following the selected node */
  newnode->next = selected->next;

  selected->next  = newnode;  /* Current node now points to the new node */
  newnode->userid = NULL;     /* Created node's user id is NULL */
  newnode->imageinuse = NO;   /* Create node's image is not in use */
  newnode->expandedimageinuse = NO;  /* Create node's expanded image is not in use */
  newnode->text_color  = 0x000000L;
  return 1;
}

static void iTreeGSDeleteLeaf(Ihandle* ih, Node node)
{
  Node next; 
  Node prev; 

  /* Parameter checking */
  if(!node || node->kind != ITREE_LEAF)
    return;
  
  if(node == ih->data->root)
    return; /* Cannot delete tree root */

  /* Basic variables */
  next = node->next;
  prev = iTreeFindPrevious(ih->data->root, node);

  prev->next = next;

  if(node->name)
    free(node->name);
  free(node);

  if(iTreeFindNodeId(ih, ih->data->selected) == -1)
  {
    ih->data->selected = prev;
    ih->data->starting = prev;
  }
}

/* Deletes a branch and returns the next node */
static Node iTreeGSDeleteBranch(Ihandle* ih, Node node, int del_self)
{
  Node next, prev;
  int depth;

  /* Parameter checking */
  if(node == NULL)
    return NULL;     /* Node cannot be NULL     */
  
  if(node == ih->data->root && del_self == 1)
    return NULL;     /* Cannot delete tree root */
  
  if(node->kind != ITREE_BRANCH)
    return NULL;     /* We only deal branches   */

  /* Basic variables */
  depth = node->depth;
  prev  = iTreeFindPrevious(ih->data->root, node);

  next  = node->next;
  
  /* Loop with recursion */
  while(next && depth < next->depth)
  {
    Node tmp = next->next;
  
    if(next->kind == ITREE_BRANCH)
      tmp = iTreeGSDeleteBranch(ih, next, 1);
    else
      iTreeGSDeleteLeaf(ih, next);

    next = tmp;
  }

  if(del_self)
  {
    if(node->name)
      free(node->name);
    free(node);
    prev->next = next;
  }
  else
    node->next = next;

  if(iTreeFindNodeId(ih, ih->data->selected) == -1)
  {
    ih->data->selected = prev;
    ih->data->starting = prev;
  }
  return next;
}

int iTreeGSSetColor(Ihandle* ih, const char* id, const char* color)
{
  Node current = (Node)iTreeFindNodeFromString(ih, id);

  if(current)
  {
    unsigned char r, g, b;
    iupStrToRGB(color, &r, &g, &b);      
    current->text_color = cdEncodeColor((unsigned char) r, (unsigned char) g, (unsigned char) b);
    return 1;
  }
  return 0;
}

char* iTreeGSGetColor(Ihandle* ih, const char* id)
{
  unsigned char r, g, b;
  char* buffer = iupStrGetMemory(12);
  Node current = (Node)iTreeFindNodeFromString(ih, id);

  cdDecodeColor(current->text_color, &r, &g, &b);
  sprintf(buffer, "%d %d %d", r, g, b);
  return buffer;
}

/* Deletes the specified node and all its children.
   - id_string : node identifier
   - mode      : "CHILDREN": deletes the selected node's children,
                 "SELECTED": deletes the selected node (and it's children)
                 "MARKED"  : deletes all marked nodes and it's children
*/
int iTreeGSDelNode(Ihandle* ih, const char* id_string, const char* mode)
{
  if(iupStrEqualNoCase(mode, "SELECTED"))
  {
    Node current = (Node)iTreeFindNodeFromString(ih, id_string);
    if(!current)
      return 0;
    
    if(current->kind == ITREE_BRANCH)
      iTreeGSDeleteBranch(ih, current, 1);
    else
      iTreeGSDeleteLeaf(ih, current);

    return 1;
  }
  else if(iupStrEqualNoCase(mode, "CHILDREN"))
  {
    Node node = (Node)iTreeFindNodeFromString(ih, id_string);
    if(!node)
      return 0;

    /* Only delete children if it is a ITREE_BRANCH */
    if(node->kind == ITREE_BRANCH)
      iTreeGSDeleteBranch(ih, node, 0);
    
    return 1;
  }
  else if(iupStrEqualNoCase(mode, "MARKED"))
  {
    Node node = iTreeFindMarked(ih, id_string);
    while(node)
    {
      if(node->kind == ITREE_BRANCH)
        iTreeGSDeleteBranch(ih, node, 1);
      else
        iTreeGSDeleteLeaf(ih, node);
     
      node = iTreeFindMarked(ih, id_string);
    }
    return 1;
  }
  return 0;
}

/* Sets the selected node's name */
int iTreeGSSetName(Ihandle* ih, const char* id_string, const char* name)
{
  Node node = iTreeFindNodeFromString(ih, id_string);

  if(iupStrEqual(name,""))
    name = NULL;
  
  if(node)
  {
    if(node->name)
    {
      free(node->name);
      node->name = NULL;
    }
    
    if(name!=NULL)
    {
      node->name = malloc(sizeof(char)*strlen(name)+1);     
      strcpy(node->name, name);
    }

    return 1;
  }

  return 0;
}

/* Gets the selected node's name. */
char* iTreeGSGetName(Ihandle* ih, const char* id_string)
{
  Node node = iTreeFindNodeFromString(ih, id_string);

  if(node == NULL)
    return NULL;

  if(node->name)
    return node->name;

  /* cannot return NULL */
  return "";
}

/* Changes the current path, starting from root.
   If the path does not exist, does nothing
   - path : string with the specified path
*/
int iTreeGSSetPath(Ihandle* ih, const char* path)
{
  Node node = (Node)ih->data->root;

  /* If path is NULL, node is the root */
  if(path == NULL)
  {
    ih->data->selected = node;
    return 1;  
  }

  /* Otherwise, searches in the given path */
   node = iTreeFindNodeFromString(ih, path);

   if(node)
   {
     ih->data->selected = node;
     return 1;
   }
   return 0;
}

static void iTreeGSKeyboardMark(Ihandle* ih, Node node, int call_cb)
{
  int test = IUP_DEFAULT;
  int id   = iTreeFindNodeId(ih, node);

  /* When control iTreeKey is pressed, does not mark or unmark anything */
  if(tree_ctrl == YES)
    return;

  /* Without the tree_shift iTreeKey, unselect all */
  if(tree_shift == NO)
    iTreeGSSetMarked(ih, NO, call_cb);

  if(call_cb)
    test = iTreeCallSelectionCB(ih, id, 1);
  if(test != IUP_IGNORE)
    node->marked = YES;
}

/* Changes the selected node, starting from current branch
   - value : "ROOT", "LAST", "NEXT", "PREVIOUS", "INVERT", "BLOCK",
             "CLEARALL", "MARKALL", "INVERTALL"
             or id of the node that will be the current
   - callcb: indicates if any callback should be called (user actions
             call callbacks, attribute setting does not call cb)
*/
int iTreeGSSetValue(Ihandle* ih, const char* value, int call_cb)
{
  Node node = (Node)ih->data->selected;

  if(iupStrEqualNoCase(value, "ROOT"))
  {
    ih->data->selected = ih->data->root;
    IupSetAttribute(ih, "POSY", "0.0");
    iTreeGSKeyboardMark(ih, ih->data->selected, 1);
    return 1;
  }
  else if(iupStrEqualNoCase(value, "LAST"))
  {
    while(node->next)
      node = node->next;
    
    ih->data->selected = node;
    
    IupSetfAttribute(ih, "POSY", "%f", 1.0 - IupGetFloat(ih,"DY"));
    iTreeGSKeyboardMark(ih, node, 1);
    return 1;
  }
  else if(iupStrEqualNoCase(value, "PGUP"))
  {
    char* buffer = iupStrGetMemory(10);
    Node tmp;
    int i, newid;

    i = (int)iTreeFindNumNodesInCanvas(ih);

    tmp = node;
    newid = iTreeFindNodeId(ih, node);

    /* Now we skip i visible nodes */
    while(tmp && i > 0 && newid > 0)
    {
      if(tmp->visible == YES)
        i--;
      tmp = iTreeFindNodeFromId(ih, --newid);
    }

    sprintf(buffer, "%d", newid);
    iTreeGSSetValue(ih, buffer, call_cb);
    iTreeScrollPgUp(ih);
    iTreeScrollShow(ih);
  }
  else if(iupStrEqualNoCase(value, "PGDN"))
  {
    char* buffer = iupStrGetMemory(10);
    Node tmp;
    int newid, i;

    i = (int)iTreeFindNumNodesInCanvas(ih);
    newid = iTreeFindNodeId(ih, node);

    /* Now we skip i visible nodes */
    tmp = node;

    while(tmp && i > 0 && newid <= iTreeFindTotNumNodes(ih))
    {
      if(tmp->visible == YES)
        i--;
      newid++;
      tmp = tmp->next;
    }

    if(tmp == NULL)
      newid--;
    else
    {
      while(tmp && tmp->visible == NO)
      {
        tmp = tmp->next;
        newid++;
      }
      if(tmp == NULL)
        newid--;
    }

    sprintf(buffer, "%d", newid);
    iTreeGSSetValue(ih, buffer, call_cb);
    iTreeScrollPgDn(ih);
    iTreeScrollShow(ih);
  }
  else if(iupStrEqualNoCase(value, "NEXT"))
  {
    while(node->next)
    {
      if(node->next->visible == YES)
      {
        ih->data->selected = node->next;
        
        iTreeScrollDown(ih);

        /* If node is the last one, scrolls to the end */
        if(node->next->next == NULL)
        {
          iTreeScrollEnd(ih);
        }
        iTreeScrollShow(ih);
        iTreeRepaint(ih);
  
        iTreeGSKeyboardMark(ih, ih->data->selected, 1);
        return 1;
      }
      node = node->next;
    }
    iTreeScrollEnd(ih);

  }
  else if(iupStrEqualNoCase(value, "PREVIOUS"))
  {
    Node temp = ih->data->root;
    while(temp != node)
    {
      while(temp->next != node)
        temp = temp->next;
      if(temp->visible == YES)
      {
        ih->data->selected = temp;
        iTreeGSKeyboardMark(ih, temp, 1);
        iTreeScrollUp(ih);

        /* If node is the first one, scrolls to the begin */
        if(temp == ih->data->root)
        {
          iTreeScrollBegin(ih);  
        }
        iTreeScrollShow(ih);
        iTreeRepaint(ih);
  
        return 1;
      }
      node = temp;
      temp = ih->data->root;
    }
    iTreeScrollBegin(ih);
  }
  else if(iupStrEqualNoCase(value, "BLOCK"))
  {
    IFnIi ms_cb;
    Node node; 
    int begin;
    int end;
    int i;

    iTreeFindStartEndSelection(ih, &begin, &end);
    node = iTreeFindNodeFromId(ih, begin);

    ms_cb = (IFnIi)IupGetCallback(ih, "MULTISELECTION_CB");
    if(ms_cb)
    {
      int* ms_nodes = (int*)malloc((end - begin + 1)* sizeof(int));
      int ms_n = 0;
      Node begin_node = node;

      for(i = begin; i <= end; i++)
      {
        if(node->marked == NO)
        {
          ms_nodes[ms_n] = i; 
          ms_n++;
        }
        node = node->next;
      }

      if(ms_cb(ih, ms_nodes, ms_n) != IUP_IGNORE)
      {
        node = begin_node;
        for(i = begin; i <= end; i++)
        {
          if(node->marked == NO)
            node->marked = YES;
          node = node->next;
        }
      }

      free(ms_nodes);
    }
    else
    {
      for(i = begin; i <= end; i++)
      {
        int test = IUP_DEFAULT;
        if(node->marked == NO)
          test = iTreeCallSelectionCB(ih, i, 1);
        if(test != IUP_IGNORE)
          node->marked = YES;
        node = node->next;
      }
    }
  }
  else if(iupStrEqualNoCase(value, "CLEARALL"))
  {
    iTreeGSSetMarked(ih, NO, call_cb);
  }
  else if(iupStrEqualNoCase(value, "MARKALL"))
  {
    iTreeGSSetMarked(ih, YES, call_cb);
  }
  /* INVERTALL *MUST* appear before INVERT, or else INVERTALL will never be called. */
  else if(iupStrEqualNoCase(value, "INVERTALL"))
  {
    iTreeGSSetMarked(ih, ITREE_INVERT, call_cb);
  }
  else if(iupStrEqualPartial(value, "INVERT"))   /* allows the use of "INVERTid" form */
  {
    iTreeGSInvertSelection(ih,&value[strlen("INVERT")], call_cb);
  }
  else
  {
    ih->data->selected = iTreeFindNodeFromString(ih, value );
    iTreeGSKeyboardMark(ih, ih->data->selected, 1);
  }
  
  if(ih->data->selected)
    return 1;

  return 0;
} 

/* Sets the selected branch state. */
int iTreeGSSetStateOfNode(Ihandle* ih, Node node, const char* mode)
{
  if(iupStrEqualNoCase(mode, "COLLAPSED"))
  {
    if(iTreeCallBranchCloseCB(ih, node) == IUP_DEFAULT) 
    {
      node->state = ITREE_COLLAPSED;
      iTreeDrawSetVisibility(ih); /* Reset all nodes visibility status */
    }
  }
  else if(iupStrEqualNoCase(mode, "EXPANDED"))
  {
    if(iTreeCallBranchOpenCB(ih, node) == IUP_DEFAULT)  
    {
      node->state = ITREE_EXPANDED;
      iTreeDrawSetVisibility(ih); /* Reset all nodes visibility status */
    }
  }
  else 
    return 0;

  return 1;
}

/* Changes the selected branch state.
   - id_string : node identifier
   - mode : EXPANDED or COLLAPSED.
*/
int iTreeGSSetState(Ihandle* ih, const char* id_string, const char* mode)
{
  Node node = iTreeFindNodeFromString(ih, id_string);
  return iTreeGSSetStateOfNode(ih, node, mode);
}

/* Sets the depth of a node.
   - id_string : node identifier
   - dep       : node depth
*/
int iTreeGSSetDepth(Ihandle* ih, const char* id_string, const char* dep)
{
  Node node = (Node)iTreeFindNodeFromString(ih, id_string);
  int depth;
  depth = atoi(dep);

  if(node == NULL)
    return 0;

  /* After all these conditions, node->depth is changed */
  node->depth = depth; 

  return 1;
}

/* Turns on and off the tree_ctrl iTreeKey.
   - mode : "YES" or "NO".
*/
int iTreeGSSetCtrl(Ihandle* ih, const char* mode)
{
  if(iupStrEqualNoCase(mode, "YES"))
    ih->data->tree_ctrl = YES;    
  else 
    ih->data->tree_ctrl = NO;

  return 1;
}

/* Turns on and off the tree_shift iTreeKey.
   - mode : "YES" or "NO".
*/
int iTreeGSSetShift(Ihandle* ih, const char* mode)
{
  if(iupStrEqualNoCase(mode, "YES"))
    ih->data->tree_shift = YES;    
  else 
    ih->data->tree_shift = NO;

  return 1;
}

/* Sets the image of all the leafs.
   - image        : image pixmap
   - color        : color array
   - marked_color : marked color array
   - range        : ITREE_NODE or ITREE_TREE
*/
int iTreeGSSetImage(const char* name, unsigned char* image, unsigned long int* color, unsigned long int* marked_color, int range, Node node)
{
  Ihandle *img = IupGetHandle(name);  
  int x, y;
  unsigned char* pointer;

  if(IupGetInt(img, "WIDTH")  != ITREE_NODE_WIDTH  || 
     IupGetInt(img, "HEIGHT") != ITREE_NODE_HEIGHT) 
    return 0;

  if(range == ITREE_NODE)
  {
    if(iupStrEqualNoCase(name, ""))
      node->imageinuse = NO;
    else
      node->imageinuse = YES;
  }
  else if(range == ITREE_NODEEXPANDED)
  {
    if(iupStrEqualNoCase(name, ""))
      node->expandedimageinuse = NO;
    else
      node->expandedimageinuse = YES;
  }

  pointer = (unsigned char*)img->handle;

  for(y = ITREE_NODE_HEIGHT - 1; y >= 0; y--)
  {
    for(x = 0; x < ITREE_NODE_WIDTH; x++)
    {
      image[x + ITREE_NODE_HEIGHT * y] = *pointer;
      pointer++;
    }
  }
  
  for(y = 0; y < ITREE_NODE_HEIGHT; y++)
  {
    for(x = 0; x < ITREE_NODE_WIDTH; x++)
    {
      char* colorstr = iupStrGetMemory(10);
      char* value;
      unsigned char r = 0, g = 0, b = 0;
      int index;

      index = (int)image[x + ITREE_NODE_HEIGHT * y];
      
      sprintf(colorstr, "%d",index);
      value = IupGetAttribute(img, colorstr);
      
      if(iupStrEqualNoCase(value, "BGCOLOR")) /* retrieve the background color */
        value = ITREE_TREE_BGCOLORSTRING;

      iupStrToRGB(value, &r, &g, &b);    
      color[index] = cdEncodeColor((unsigned char)r, (unsigned char)g, (unsigned char)b);

      if(index != 1)
      {
        long mcolor;
        cdIupCalcShadows(color[index], NULL, NULL, &mcolor);
        marked_color[index] = mcolor;
      }
      else
        marked_color[index] = color[index];
    }
  }
 
  return IUP_DEFAULT;
}
      
/* Retrieves the selected branch state */
char* iTreeGSGetState(Ihandle* ih, const char* id_string)
{
  Node node = iTreeFindNodeFromString(ih, id_string);

  if(node->state == ITREE_COLLAPSED)
    return "COLLAPSED";

  if(node->state == ITREE_EXPANDED)
    return "EXPANDED";

  return 0;
}

/* Retrieves the tree_ctrl iTreeKey state */
char* iTreeGSGetCtrl(Ihandle* ih)
{
  if(ih->data->tree_ctrl == YES)
    return "YES";

  return "NO";
}

/* Retrieves the tree_shift iTreeKey state */
char* iTreeGSGetShift(Ihandle* ih)
{
  if(ih->data->tree_shift == YES)
    return "YES";

  return "NO";
}

/* Retrieves the selected node kind */
char* iTreeGSGetKind(Ihandle* ih, const char* id_string)
{
  Node node = iTreeFindNodeFromString(ih, id_string);

  if(node->kind == ITREE_LEAF)
    return "LEAF";

  if(node->kind == ITREE_BRANCH)
    return "BRANCH";

  return NULL;
}

/* Retrieves the selected node id */
char* iTreeGSGetValue(Ihandle* ih)
{
  Node node = (Node)ih->data->root;

  int i = 0;
  char* id = iupStrGetMemory(16);

  while(node != ih->data->selected)
  {
    node = node->next;
    /* This is a serious error, it means that the field ih->data->selected has
     * an invalid node */
    if(node == NULL)
      return 0;
    i++;
  }

  sprintf(id, "%d", i);

  return id;
}

/* Retrieves the selected node's parent */
char* iTreeGSGetParent(Ihandle* ih, const char* id_string)
{
  Node temp = (Node)ih->data->root;  
  Node node;
  int i = 0;
  char* id = iupStrGetMemory(10);

  node = iTreeFindParent(ih, id_string);

  if(node)
  {
    while(temp != node)
    {
      temp = temp->next;
      i++;
    }
    sprintf(id, "%d", i);

    return id;
  }
  return NULL;
}

/* Retrieves the selected node depth */
char* iTreeGSGetDepth(Ihandle* ih, const char* id_string)
{
  Node node = (Node)iTreeFindNodeFromString(ih, id_string);
  int dep = -1; 
  char* depth = iupStrGetMemory(10);

  if(node)
  {
    dep = node->depth;
  }
  sprintf(depth, "%d", dep);

  return depth;
}

/* Sets the node's user id
   - id : node identifier
   - userid : user id that identifies the node
*/
int IupTreeSetUserId(Ihandle* ih, int id, void* userid)
{
  Node node;
  char* id_string = iupStrGetMemory(10);

  sprintf(id_string, "%d", id);
 
  node = (Node)iTreeFindNodeFromString(ih, id_string);

  if(node)
  {
    node->userid = userid;
    return 1;
  }
  return 0;
}

/* Gets a node's id
   - userid : user id that identifies the node
*/
int IupTreeGetId(Ihandle* ih, void* userid)
{
  Node node = (Node)ih->data->root;
  int id = 0;

  while(node)
  {
    if(userid == node->userid)
      return id;    

    node = node->next;
    id++;
  }
  
  return -1;
}

/* Gets the user id
   - id_string : node's id that identifies the node
*/
void* IupTreeGetUserId(Ihandle* ih, int id)
{
  Node node;
  char* id_string = iupStrGetMemory(10);

  sprintf(id_string, "%d", id);
 
  node = (Node)iTreeFindNodeFromString(ih, id_string);

  if(node)
  {
     return node->userid;
  }
  return NULL;
}

void iTreeGSMark(Ihandle* ih, const char* node, const char* attr)
{
  Node n = iTreeFindNodeFromString(ih, node );

  if(attr && iupStrEqualNoCase(attr, "NO"))
    n->marked = NO;

  if(attr && iupStrEqualNoCase(attr, "YES"))
    n->marked = YES;
}

void iTreeGSSetMarked(Ihandle* ih, int mode, int call_cb)
{
  Node node = (Node)ih->data->root;
  int i = 0;

  while(node)
  {
    if(mode == ITREE_INVERT)
    {
      if(node->marked == YES)
      {
        int test = IUP_DEFAULT;
        if(call_cb)
          test = iTreeCallSelectionCB(ih, i, 0);
        if(test != IUP_IGNORE)
          node->marked = NO;
      }
      else
      {
        int test = IUP_DEFAULT;
        if(call_cb)
          test = iTreeCallSelectionCB(ih, i, 1);
        if(test != IUP_IGNORE)
          node->marked = YES;
      }
    }
    else
    {
      int test = IUP_DEFAULT;

      if(mode == YES && node->marked == NO && call_cb == 1)
        test = iTreeCallSelectionCB(ih, i, 1);
      else if(mode == NO && node->marked == YES && call_cb == 1)
        test = iTreeCallSelectionCB(ih, i, 0);

      if(test != IUP_IGNORE)
        node->marked = mode;
    }
    node = node->next;
    i++;
  }
}

/* Sets the starting node when tree_shift is pressed
   - value : id of the node 
*/
int iTreeGSSetStarting(Ihandle* ih, const char* value)
{
  ih->data->starting = iTreeFindNodeFromString(ih, value);
  if(ih->data->starting)
    return 1;
  
  return 0;
}

/* Retrieves the starting node when tree_shift is pressed */
char* iTreeGSGetStarting(Ihandle* ih)
{
  Node node = (Node)ih->data->root;

  int i = 0;
  char* id = iupStrGetMemory(10);

  while(node != ih->data->starting)
  {
    node = node->next;
    i++;
  }

  sprintf(id, "%d", i);

  return id;
}

/* Invert the selected node's selection
   - value : node's value
*/
int iTreeGSInvertSelection(Ihandle* ih, const char* id_string, int call_cb)
{
  Node node = iTreeFindNodeFromString(ih, id_string);
  int id = iTreeFindNodeId(ih, node);
  
  if(node)
  {
    if(node->marked == YES)
    {
      int test = IUP_DEFAULT;
      if(call_cb)
        test = iTreeCallSelectionCB(ih, id, 0);

      if(test != IUP_IGNORE)
        node->marked = NO;
    }
    else
    {
      int test = IUP_DEFAULT;
      if(call_cb)
        test = iTreeCallSelectionCB(ih, id, 1);

      if(test != IUP_IGNORE)
        node->marked = YES;
    }

    return 1;
  }

  return 0;
}

/* Retrieves the marking state of the node */
char* iTreeGSGetMarked(Ihandle* ih, const char* id_string)
{
  Node node = iTreeFindNodeFromString(ih, id_string);

  if(node)
  {
     if(node->marked == YES) 
       return "YES";

     if(node->marked == NO) 
       return "NO";
  }
  return NULL;
}
