/** \file
 * \brief iuptree control
 * Functions to set and get attributes
 *
 * See Copyright Notice in iup.h
 * $Id: itgetset.c,v 1.1 2008-10-17 06:20:41 scuri Exp $
 */

#include <stdlib.h> /* malloc, realloc */
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <math.h>

#include <iup.h>
#include <iupcpi.h>
#include <cd.h>

#include "iuptree.h" 
#include "treedef.h"
#include "itgetset.h"
#include "itfind.h"
#include "itcallback.h"
#include "itdraw.h"
#include "itscroll.h"
#include "iupcdutil.h"

#include "iupcompat.h"
#define NO      0
#define YES     1

extern int tree_ctrl;
extern int tree_shift;

typedef int (*ICmultisel)(Ihandle* self, int* nodes, int n);

/**************************************************************************
***************************************************************************
*
*   Sets and Gets tree attributes
*
***************************************************************************
***************************************************************************/

/*

%F Sets the created branch state. It can be created collapsed (IUP_NO) or 
   expanded (IUP_YES).
%i h : tree handle
value   : IUP_YES or IUP_NO
%o returns 1 if sucessfull and 0 if fails

*/

int treegetsetAddExpanded (Ihandle *h, char* value)
{
  TtreePtr tree = (TtreePtr)tree_data(h);

  if(iupStrEqual(value,IUP_YES))tree_addexpanded(tree) = YES ;
  else if(iupStrEqual(value,IUP_NO))tree_addexpanded(tree) = NO ;
  else return 0 ;
  
  return 1;
}

/*

%F Adds a node at the specified path. If the path does not exist, does nothing
%i h : tree handle
id   : node identifier
kind : node kind (LEAF or BRANCH)
%o returns 1 if sucessfull and 0 if fails

*/
int treegetsetAddNode(Ihandle *h, char* id_string, int kind)
{
  TtreePtr tree = (TtreePtr)tree_data(h);
  Node node = treefindNodeFromString(h, id_string);

  if(node)
  {
    {
      treegetsetCreateNode(tree, node, kind);
      return 1;
    }
  }
  
  return 0;
}

/*

%F Creates a node in the place pointed by selected. The new node is pointed by the selected node and points to the node following the selected node.

   |--|  -->  |--|  -->   |--|
 selected    new     node following
   node      node    selected node

%i selected: pointer to selected node   
kind : node kind (LEAF or BRANCH)  
%o
  else parent = node_next(tree_root(tree));returns 0 if sucessfull and 1 if fails

*/
int treegetsetCreateNode(TtreePtr tree, Node selected, int kind)
{
  Node newnode;

  if(selected == NULL) return 0 ;

  newnode = (Node)malloc(sizeof(struct Node_));
  if(newnode == NULL)return 0 ;

  /* Initializes new node's depth */
  node_depth(newnode) = node_depth(selected)+(node_kind(selected)==BRANCH);

  /* Node kind */
  node_kind(newnode) = kind;

  /* Branch state depends on the attribute addexpanded */
  if(tree_addexpanded(tree) == YES)node_state(newnode) = EXPANDED;
    else node_state(newnode) = COLLAPSED;

  node_visible(newnode) = YES; /* Node is visible by default */
  node_marked(newnode) = NO;   /* Node is not marked by default */
  node_name(newnode) = NULL;   /* Node value is NULL by default */
  
  /* New node points to the node following the selected node */
  node_next(newnode)=node_next(selected);

  node_next(selected)=newnode;  /* Current node now points to the new node */
  node_userid(newnode)=NULL;    /* Created node's user id is NULL */
  node_imageinuse(newnode) = NO ; /* Create node's image is not in use */
  node_expandedimageinuse(newnode) = NO ;/* Create node's expanded image is not
					    in use */
  node_text_color(newnode) = 0x000000L;
  return 1;
}

static void deleteLeaf(Ihandle *h, Node node)
{
  TtreePtr tree = (TtreePtr)tree_data(h);
  Node next; 
  Node prev; 

  /* Parameter checking */
  if(!node || node_kind(node) != LEAF) return;
  if(node == tree_root(tree)) return; /* Cannot delete tree root */

  /* Basic variables */
  next = node_next(node);
  prev = treefindPrevious(tree_root(tree), node);
  assert(prev);

  node_next(prev) = next;

  if(node_name(node))
    free(node_name(node));
  free(node);

  if(treefindNodeId(h, tree_selected(tree)) == -1)
  {
    tree_selected(tree) = prev;
    tree_starting(tree) = prev;
  }
}

/*
 * Deletes a branch and returns the next node
 */
static Node deleteBranch(Ihandle *h, Node node, int del_self)
{
  Node next, prev;
  int depth;
  TtreePtr tree = (TtreePtr)tree_data(h);

  /* Parameter checking */
  if (node == NULL) return NULL;              /* Node cannot be NULL     */
  if (node == tree_root(tree) &&
      del_self == 1) return NULL;             /* Cannot delete tree root */
  if (node_kind(node) != BRANCH) return NULL; /* We only deal branches   */

  /* Basic variables */
  depth = node_depth(node);
  prev  = treefindPrevious(tree_root(tree), node);

  next  = node_next(node);
  /* Loop with recursion */
  while(next && depth < node_depth(next))
  {
    Node tmp = node_next(next);
    if (node_kind(next) == BRANCH)
      tmp = deleteBranch(h, next, 1);
    else
      deleteLeaf(h, next);
    next = tmp;
  }

  if(del_self)
  {
    if(node_name(node))
      free(node_name(node));
    free(node);
    node_next(prev) = next;
  }
  else
    node_next(node) = next;

  if(treefindNodeId(h, tree_selected(tree)) == -1)
  {
    tree_selected(tree) = prev;
    tree_starting(tree) = prev;
  }
  return next;
}

int treegetsetSetColor(Ihandle *h, char* id, char* color)
{
	Node current = (Node)treefindNodeFromString(h, id);
	if(current)
	{
		int r, g, b;
		sscanf(color, "%d %d %d", &r, &g, &b);
		node_text_color(current) = cdEncodeColor((unsigned char) r, (unsigned char) g, (unsigned char) b);
    return 1;
	}
  return 0;
}

char *treegetsetGetColor(Ihandle *h, char* id)
{
  unsigned char r, g, b;
	char *buffer = (char*) malloc(sizeof(char)*12);
	Node current = (Node)treefindNodeFromString(h, id);
	cdDecodeColor(node_text_color(current), &r, &g, &b);
	sprintf(buffer, "%d %d %d", r, g, b);
  return buffer;
}

/*
%F Deletes the specified node and all its children.
%i h      : tree handle
id_string : node identifier
mode      : IUP_CHILDREN: deletes the selected node's children,
            IUP_SELECTED: deletes the selected node (and it's children)
            IUP_MARKED:   deletes all marked nodes and it's children
%o returns 1 if sucessfull and 0 if fails
*/
int treegetsetDelNode(Ihandle *h, char* id_string, char* mode)
{
  if(iupStrEqual(mode, IUP_SELECTED))
  {
    Node current = (Node)treefindNodeFromString(h, id_string);
    if(!current) return 0;
    if(node_kind(current) == BRANCH)
      deleteBranch(h, current, 1);
    else
      deleteLeaf(h, current);
    return 1;
  }
  else if(iupStrEqual(mode,IUP_CHILDREN))
  {
    Node node = (Node)treefindNodeFromString(h, id_string);
    if(!node) return 0;

    /* Only delete children if it is a BRANCH */
    if(node_kind(node) == BRANCH)
      deleteBranch(h, node, 0);
    return 1;
  }
  else if(iupStrEqual(mode,IUP_MARKED))
  {
    Node node = treefindMarked(h, id_string);
    while(node)
    {
      if(node_kind(node) == BRANCH)
        deleteBranch(h, node, 1);
      else
        deleteLeaf(h, node);
      node = treefindMarked(h, id_string);
    }
    return 1;
  }
  return 0;
}

/*
 
%F Sets the selected node's name
%i h     : tree's handle
   value : node's value
%o returns 0 if sucessfull and 1 if fails

*/

int treegetsetSetName (Ihandle* h, char* id_string, char* name)
{
  Node node = treefindNodeFromString(h, id_string);

  if(iupStrEqual(name,""))name = NULL;
  
  if(node)
  {
    if(node_name(node))
    {
      free(node_name(node));
      node_name(node) = NULL;
    }
    
    if(name!=NULL)
    {
      node_name(node) = malloc(sizeof(char)*strlen(name)+1); 	  
      strcpy(node_name(node), name);
    }

    return 1;
  }

  return 0;
}

/*

%F Gets the selected node's name
%i h     : tree's handle
%o returns 0 if sucessfull and 1 if fails
*/

char* treegetsetGetName (Ihandle* h, char* id_string)
{
  Node node = treefindNodeFromString(h, id_string);

  if(node == NULL) return NULL;

  if(node_name(node))return node_name(node);

  /* cannot return NULL */
  return "";
}

/*

%F Changes the current path, starting from root.
   If the path does not exist, does nothing
%i h : tree handle
path : string with the specified path
	%o returns 1 if sucessfull and 0 if fails

*/
int treegetsetSetPath (Ihandle *h, char* path)
{
  TtreePtr tree = (TtreePtr)tree_data(h);
  Node node = (Node)tree_root(tree);

  /* If path is NULL, node is the root */
  if(path == NULL)
  {
    tree_selected(tree)=node;
    return 1;	
  }

  /* Otherwise, searches in the given path */
   node = treefindNodeFromString( h, path ) ;

   if(node)
   {
     tree_selected(tree) = node ;
     return 1;
   }
   return 0;
}

static void KeyboardMark(Ihandle *h, Node node, int call_cb)
{
  int test = IUP_DEFAULT;
  int id   = treefindNodeId(h, node);

  /* When control treeKey is pressed, does not mark or unmark anything */
  if(tree_ctrl == YES)
    return;

  /* Without the tree_shift treeKey, unselect all */
  if(tree_shift == NO)
    treegetsetSetMarked(h, NO, call_cb);

  if(call_cb)
    test = treecallSelectionCb(h, id, 1);
  if(test != IUP_IGNORE)
    node_marked(node) = YES;
}

/*
 *
 * %F Changes the selected node, starting from current branch
 * %i h : tree handle
 * value : IUP_ROOT, IUP_LAST, IUP_NEXT, IUP_PREVIOUS, IUP_INVERT, IUP_BLOCK,
 *         IUP_CLEARALL, IUP_MARKALL, IUP_INVERTALL
 *         or id of the node that will be the current
 * callcb: indicates if any callback should be called (user actions call callbacks,
 *         atribute setting does not call cb)
 * %o returns 1 if sucessfull and 0 if fails
 * 
 */
int treegetsetSetValue(Ihandle *h, char* value, int call_cb)
{
  TtreePtr tree = (TtreePtr)tree_data(h);
  Node node = (Node)tree_selected(tree);

  if(iupStrEqual(value,IUP_ROOT))
  {
    tree_selected(tree) = tree_root(tree);
    IupSetAttribute(tree_self(tree),IUP_POSY,"0.0");
    KeyboardMark(h, tree_selected(tree), 1);
    return 1;
  }
  else if(iupStrEqual(value,IUP_LAST))
  {
    char buffer[10];	  
    while(node_next(node))node = node_next(node);
    tree_selected(tree) = node;
    
    sprintf(buffer,"%f",1.0-IupGetFloat(tree_self(tree),IUP_DY));
    IupStoreAttribute(tree_self(tree),IUP_POSY,buffer);
    KeyboardMark(h, node, 1);
    return 1;
  }
  else if(iupStrEqual(value,IUP_PGUP))
  {
    char buffer[10];
    Node tmp;
    int i, newid;

    i = (int)treefindNumNodesInCanvas(h);

    tmp = node;
    newid = treefindNodeId(h, node);

    /* Now we skip i visible nodes */
    while(tmp && i>0 && newid>0)
    {
      if(node_visible(tmp) == YES)
        i--;
      tmp = treefindNodeFromId(h, --newid);
    }

    sprintf(buffer,"%d", newid);
    treegetsetSetValue(h, buffer, call_cb);
    treeScrollPgUp(h);
    treeScrollShow(h);
  }
  else if(iupStrEqual(value,IUP_PGDN))
  {
    char buffer[10];
    Node tmp;
    int newid, i;

    i = (int)treefindNumNodesInCanvas(h);
    newid = treefindNodeId(h, node);

    /* Now we skip i visible nodes */
    tmp = node;

    while(tmp && i>0 && newid<=treefindTotNumNodes(h))
    {
      if(node_visible(tmp) == YES)
        i--;
      newid++;
      tmp = node_next(tmp);
    }

    if(tmp == NULL)
      newid--;
    else
    {
      while(tmp && node_visible(tmp) == NO)
      {
        tmp = node_next(tmp);
        newid++;
      }
      if(tmp == NULL)
        newid--;

    }

    sprintf(buffer, "%d", newid);
    treegetsetSetValue(h, buffer, call_cb);
    treeScrollPgDn(h);
    treeScrollShow(h);
  }
  else if(iupStrEqual(value,IUP_NEXT))
  {
    while(node_next(node))
    {
      if(node_visible(node_next(node)) == YES)
      {
        tree_selected(tree) = node_next(node) ;
        
	      treeScrollDown(h);

	      /* If node is the last one, scrolls to the end */
        if(node_next(node_next(node)) == NULL)
        {
          treeScrollEnd(h);
        }
	      treeScrollShow(h);
        treeRepaint(h);
	
        KeyboardMark(h, tree_selected(tree), 1);
        return 1;
      }
      node = node_next(node);
    }
    treeScrollEnd(h);

  }
  else if(iupStrEqual(value,IUP_PREVIOUS))
  {
    Node temp = tree_root(tree);
    while(temp != node)
    {
      while(node_next(temp)!=node)temp = node_next(temp);
      if(node_visible(temp) == YES)
      {
        tree_selected(tree) = temp;
        KeyboardMark(h, temp, 1);
	      treeScrollUp(h);

        /* If node is the first one, scrolls to the begin */
        if(temp == tree_root(tree))
        {
	        treeScrollBegin(h);	
        }
	      treeScrollShow(h);
        treeRepaint(h);
	
        return 1;
      }
      node = temp ;
      temp = tree_root(tree);
    }
    treeScrollBegin(h);
  }
  else if(iupStrEqual(value,IUP_BLOCK))
  {
    ICmultisel ms_cb;
    Node node; 
    int begin;
    int end;
    int i;

    treefindStartEndSelection(h, &begin, &end);
    node = treefindNodeFromId(h, begin);

    ms_cb = (ICmultisel)IupGetCallback(h,"MULTISELECTION_CB");
    if (ms_cb)
    {
      int* ms_nodes = (int*)malloc((end-begin+1)* sizeof(int));
      int ms_n = 0;
      Node begin_node = node; 

      for(i = begin ; i <= end ; i++)
      {
        if(node_marked(node) == NO)
        {
          ms_nodes[ms_n] = i; 
          ms_n++;
        }
        node = node_next(node);
      }

      if (ms_cb(h, ms_nodes, ms_n) != IUP_IGNORE)
      {
        node = begin_node;
        for(i = begin ; i <= end ; i++)
        {
          if(node_marked(node) == NO)
            node_marked(node) = YES;
          node = node_next(node);
        }
      }

      free(ms_nodes);
    }
    else
    {
      for(i = begin ; i <= end ; i++)
      {
        int test = IUP_DEFAULT;
        if(node_marked(node) == NO)
          test = treecallSelectionCb(h, i, 1);
        if(test != IUP_IGNORE)
          node_marked(node) = YES ;
        node = node_next(node);
      }
    }
  }
  else if(iupStrEqual(value,IUP_CLEARALL))
  {
    treegetsetSetMarked(h, NO, call_cb);
  }
  else if(iupStrEqual(value,IUP_MARKALL))
  {
    treegetsetSetMarked(h, YES, call_cb);
  }
  /* IUP_INVERTALL *MUST* appear before IUP_INVERT, or else IUP_INVERTALL will
  * never be called...*/
  else if(iupStrEqual(value,IUP_INVERTALL))
  {
    treegetsetSetMarked(h, INVERT, call_cb);
  }
  else if (iupStrEqualPartial(value,IUP_INVERT))
  {
    treegetsetInvertSelection(h,&value[strlen(IUP_INVERT)], call_cb);
  }
  else
  {
    tree_selected(tree) = treefindNodeFromString( h, value );
    KeyboardMark(h, tree_selected(tree), 1);
  }
  
  if(tree_selected(tree))
    return 1;

  return 0;
} 

/*

%F Changes the selected branch state. 
%i h : tree handle
id_string : node identifier
mode : IUP_EXPANDED or IUP_COLLAPSED.
%o returns 1 if sucessfull and 0 if fails

 */
int treegetsetSetStateOfNode(Ihandle *h, Node node, char* mode)
{
  if(iupStrEqual(mode,IUP_COLLAPSED))
  {
    if(treecallBranchCloseCb(h, node) == IUP_DEFAULT) 
    {
      node_state(node) = COLLAPSED ;
      treedrawSetVisibility(h); /* Reset all nodes visibility status */
    }
  }
  else if(iupStrEqual(mode,IUP_EXPANDED))
  {
    if(treecallBranchOpenCb(h,node) == IUP_DEFAULT)  
    {
      node_state(node) = EXPANDED ;
      treedrawSetVisibility(h); /* Reset all nodes visibility status */
    }
  }
  else 
    return 0;

  return 1;
}

/*

%F Changes the selected branch state. 
%i h : tree handle
id_string : node identifier
mode : IUP_EXPANDED or IUP_COLLAPSED.
%o returns 1 if sucessfull and 0 if fails

 */
int treegetsetSetState (Ihandle *h, char* id_string, char* mode)
{
  Node node = treefindNodeFromString(h, id_string);
  return treegetsetSetStateOfNode(h, node, mode);
}

/*
  
%F Sets the depth of a node. 
%i h      : tree handle
id_string : node identifier
dep       : node depth
%o returns the 1 if sucessfull and 0 otherwise
*/
int treegetsetSetDepth (Ihandle *h, char* id_string, char* dep)
{
  Node node = (Node)treefindNodeFromString(h, id_string);
  int depth;
  depth = atoi(dep);

  if(node == NULL) return 0;

  /* After all these conditions, node_depth is changed */
  node_depth(node) = depth ; 

  return 1;
}

/*

%F Turns on and off the tree_ctrl treeKey. 
%i h : tree handle
mode : IUP_YES or IUP_NO.
%o returns the 1 if sucessfull and 0 otherwise

 */
int treegetsetSetCtrl (Ihandle *h, char* mode)
{
  TtreePtr tree = (TtreePtr)tree_data(h);
  if(iupStrEqual(mode,IUP_YES))
    tree_ctrl(tree) = YES ;	  
  else 
    tree_ctrl(tree) = NO ;

  return 1 ;
}

/*

%F Turns on and off the tree_shift treeKey. 
%i h : tree handle
mode : IUP_YES or IUP_NO.
%o returns the 1 if sucessfull and 0 otherwise

 */
int treegetsetSetShift (Ihandle *h, char* mode)
{
  TtreePtr tree = (TtreePtr)tree_data(h);
  if(iupStrEqual(mode,IUP_YES))
    tree_shift(tree) = YES ;	  
  else 
    tree_shift(tree) = NO ;

  return 1 ;
}

/*

%F Sets the image of all the leafs 
%i h : tree handle
image        : image pixmap 
color        : color array 
marked_color : marked color array
range        : NODE or TREE
%o returns 1 if sucessfull and 0 otherwise

 */
int treegetsetSetImage(char* name, unsigned char* image, unsigned long int* color, unsigned long int* marked_color, int range, Node node)
{
  Ihandle *img = IupGetHandle(name);	
  int x, y;
  unsigned char* pointer;

  if(IupGetInt(img, IUP_WIDTH) != NODE_WIDTH || 
     IupGetInt(img, IUP_HEIGHT) != NODE_HEIGHT) 
    return 0 ;

  if (range == NODE)
  {
    if(iupStrEqual(name,""))
      node_imageinuse(node) = NO ;
    else
      node_imageinuse(node) = YES ;
  }
  else if(range == NODEEXPANDED)
  {
    if(iupStrEqual(name,""))
      node_expandedimageinuse(node) = NO ;
    else
      node_expandedimageinuse(node) = YES ;
  }

  pointer = iupGetImageData(img);

  for(y = NODE_HEIGHT-1 ; y >=0 ; y--)
  {
    for(x = 0 ; x<NODE_WIDTH ; x++)
    {
      image[x+NODE_HEIGHT*y] = *pointer;
      pointer++;
    }
  }
  
  for(y = 0 ; y <NODE_HEIGHT; y++)
  {
    for(x = 0 ; x<NODE_WIDTH ; x++)
    {
      char colorstr[20];
      char *value;
      unsigned int r, g, b ;
      int index ;
      index = (int)image[x+NODE_HEIGHT*y] ;
	    
      sprintf(colorstr, "%d",index);

      value = IupGetAttribute(img,colorstr);
      
      if(iupStrEqual(value,"BGCOLOR")) /* retrieve the background color */
        value = TREE_BGCOLORSTRING;

      iupGetRGB(value,&r,&g,&b);
      
      color[index] = cdEncodeColor((unsigned char)r,(unsigned char)g,(unsigned char)b);

      if (index != 1)
      {
        long mcolor;
        cdIupCalcShadows(color[index], NULL, NULL, &mcolor);
        marked_color[index] = mcolor;
      }
      else
        marked_color[index] = color[index];
    }
  }
 
  return IUP_DEFAULT ;
}
      
/*

%F Retrieves the selected branch state. 
%i h : tree handle
%o returns the selected branch state if sucessfull
     and 0 otherwise
  
 */
char* treegetsetGetState (Ihandle *h, char* id_string)
{
  Node node = treefindNodeFromString(h, id_string);

  if(node_state(node) == COLLAPSED)
    return IUP_COLLAPSED;

  if(node_state(node) == EXPANDED)
    return IUP_EXPANDED;

  return 0 ;
}

/*

%F Retrieves the tree_ctrl treeKey state. 
%i h : tree handle
%o returns the tree_ctrl treeKey state if sucessfull and -1 otherwise
  
 */
char* treegetsetGetCtrl (Ihandle *h)
{
  TtreePtr tree = (TtreePtr)tree_data(h);
  if(tree_ctrl(tree) == YES)
    return IUP_YES;

  return IUP_NO;
}

/*

%F Retrieves the tree_ctrl treeKey state. 
%i h : tree handle
%o returns the tree_ctrl treeKey state if sucessfull and NULL otherwise
  
 */
char* treegetsetGetShift (Ihandle *h)
{
  TtreePtr tree = (TtreePtr)tree_data(h);
  if(tree_shift(tree) == YES)
    return IUP_YES;

  return IUP_NO;
}


/*
  
%F Retrieves the selected node kind. 
%i h : tree handle
%o returns the selected branch state if sucessfull and NULL otherwise
 
 */
char* treegetsetGetKind (Ihandle* h, char* id_string)
{
  Node node = treefindNodeFromString(h, id_string);

  if(node_kind(node) == LEAF)
  return IUP_LEAF;

  if(node_kind(node) == BRANCH)
  return IUP_BRANCH;

  return NULL ;
}

/*
 
%F Retrieves the selected node id. 
%i h : tree handle
   %o returns the selected branch state if sucessfull
      and 0 otherwise
       
 */
char* treegetsetGetValue (Ihandle *h)
{
  TtreePtr tree = (TtreePtr)tree_data(h);
  Node node = (Node)tree_root(tree);

  int i = 0 ;
  static char id[16];

  while(node != tree_selected(tree))
  {
    node = node_next(node);
    /* This is a serious error, it means that the field tree_selected has
     * an invalid node */
    assert(node);
    if(node == NULL)
      return 0;
    i++;
  }

  sprintf( id, "%d", i );

  return id ;
}

/*

 %F Retrieves the selected node's parent. 
 %i h : tree handle
 %o returns the selected node's parent if sucessfull
    and -1 otherwise
             
 */
char* treegetsetGetParent (Ihandle *h, char* id_string)
{
  TtreePtr tree = (TtreePtr)tree_data(h);
  Node temp = (Node)tree_root(tree);  
  Node node;
  int i = 0 ;
  static char id[10];

  node = treefindParent(h,id_string);

  if(node)
  {
    while(temp != node)
    {
      temp = node_next(temp);
      i++;
    }
    sprintf( id, "%d", i );

    return id ;
  }
  return NULL;
}

/*
 
%F Retrieves the selected node depth. 
%i h : tree handle
%o returns the selected branch depth
   and -1 otherwise
       
*/
char* treegetsetGetDepth (Ihandle *h, char* id_string)
{
  Node node = (Node)treefindNodeFromString(h,id_string);
  int dep = -1; 
  static char depth[10];

  if(node)
  {
    dep = node_depth(node);
  }
  sprintf(depth, "%d", dep);

  return depth;
}

/*

%F Sets the node's user id
%i h         : tree handle
   id_string : node identifier
   userid    : user id that identifies the node
%o returns 1 if sucessfull
   and 0 otherwise
*/
int IupTreeSetUserId(Ihandle *h, int id, void *userid)
{
  Node node ;
  static char id_string[10];

  sprintf(id_string,"%d",id);
 
  node = (Node)treefindNodeFromString(h, id_string);

  if(node)
  {
    node_userid(node) = userid;
    return 1;
  }
  return 0;
}

/*

%F Gets a node's id
%i h   : tree handle
userid : user id that identifies the node
%o returns node's id if sucessfull
   and -1 otherwise
   
*/
int IupTreeGetId(Ihandle *h, void *userid)
{
  TtreePtr tree = (TtreePtr)tree_data(h);
  Node node = (Node)tree_root(tree);
  int id = 0;

  while(node)
  {
    if(userid == node_userid(node))
    return id ;    

    node = node_next(node);
    id++;
  }
  
  return -1;
}

/*
 *
 * %F Gets the user id
 * %i h   : tree handle
 * id_string : node's id that identifies the node
 * %o returns the user id if sucessfull
 *    and NULL otherwise
 *       
 */
void* IupTreeGetUserId(Ihandle *h, int id)
{
  Node node ;
  static char id_string[10];

  sprintf(id_string,"%d",id);
 
  node = (Node)treefindNodeFromString(h, id_string);

  if(node)
  {
     return node_userid(node);
  }
  return NULL;
}

void treegetsetMark(Ihandle *h, char *node, char *attr)
{
  Node n = treefindNodeFromString( h, node );

  if (attr && iupStrEqual(attr, IUP_NO))
     node_marked(n) = NO;

  if(attr && iupStrEqual(attr, IUP_YES))
    node_marked(n) = YES;
}

void treegetsetSetMarked(Ihandle *h, int mode, int call_cb)
{
  TtreePtr tree = (TtreePtr)tree_data(h);
  Node node = (Node)tree_root(tree);
  int i = 0;

  while(node)
  {
    if(mode == INVERT)
    {
      if(node_marked(node) == YES)
      {
        int test = IUP_DEFAULT;
        if(call_cb)
          test = treecallSelectionCb(h, i, 0);
        if(test != IUP_IGNORE)
          node_marked(node) = NO ;
      }
      else
      {
        int test = IUP_DEFAULT;
        if(call_cb)
          test = treecallSelectionCb(h, i, 1);
        if(test != IUP_IGNORE)
	        node_marked(node) = YES ;
      }
    }
    else
    {
      int test = IUP_DEFAULT;

      if (mode == YES && node_marked(node) == NO && call_cb == 1)
        test = treecallSelectionCb(h, i, 1);
      else if(mode == NO && node_marked(node) == YES && call_cb == 1)
        test = treecallSelectionCb(h, i, 0);

      if(test != IUP_IGNORE)
        node_marked(node) = mode ;
    }
    node = node_next(node);
    i++;
  }
}


/*
 *
 * %F Sets the starting node when tree_shift is pressed
 * %i h : tree handle
 * value : id of the node 
 * %o returns 1 if sucessfull and 0 if fails
 * 
 */
int treegetsetSetStarting (Ihandle *h, char* value)
{
  TtreePtr tree = (TtreePtr)tree_data(h);
	
  tree_starting(tree) = treefindNodeFromString( h, value );
  if(tree_starting(tree))return 1 ;
  return 0;
}

/*
 
%F Retrieves the starting node when tree_shift is pressed
%i h : tree handle
%o returns the node's id
       
 */
char* treegetsetGetStarting (Ihandle *h)
{
  TtreePtr tree = (TtreePtr)tree_data(h);
  Node node = (Node)tree_root(tree);

  int i = 0 ;
  static char id[10];

  while(node != tree_starting(tree))
  {
    node = node_next(node);
    i++;
  }

  sprintf( id, "%d", i );

  return id ;
}

/*
 
%F Invert the selected node's selection
%i h     : tree's handle
   value : node's value
%o returns 0 if sucessfull and 1 if fails

*/

int treegetsetInvertSelection(Ihandle* h, char* id_string, int call_cb)
{
  Node node = treefindNodeFromString(h, id_string);
  int id = treefindNodeId(h, node);
  
  if(node)
  {
    if(node_marked(node) == YES)
    {
      int test = IUP_DEFAULT;
      if(call_cb)
        test = treecallSelectionCb(h, id, 0);

      if(test != IUP_IGNORE)
        node_marked(node) = NO;
    }
    else
    {
      int test = IUP_DEFAULT;
      if(call_cb)
        test = treecallSelectionCb(h, id, 1);

      if(test != IUP_IGNORE)
        node_marked(node) = YES;
    }

    return 1 ;
  }

  return 0;
}

/*
 
%F Retrieves the marking state of the node
%i h : tree handle
%o returns IUP_YES if the node is marked, IUP_NO if it is not and NULL
if the node does not exists
       
 */
char* treegetsetGetMarked(Ihandle *h, char* id_string)
{
  Node node = treefindNodeFromString(h, id_string);

  if(node)
  {
     if(node_marked(node) == YES) 
       return IUP_YES ;

     if(node_marked(node) == NO) 
       return IUP_NO ;
  }
  return NULL;
}

