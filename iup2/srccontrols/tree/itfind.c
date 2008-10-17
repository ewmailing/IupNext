/** \file
 * \brief iuptree control
 * Functions used to find nodes in the tree
 *
 * See Copyright Notice in iup.h
 * $Id: itfind.c,v 1.1 2008-10-17 06:20:41 scuri Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "iup.h"
#include "iuptree.h"
#include "iupcompat.h"
#include "treedef.h" 
#include "itgetset.h"
#include "itfind.h"
#include "itdraw.h"
#include "itmouse.h"

int treefindNodeIdFromXY(Ihandle *h, int xmouse, int ymouse )
{
  TtreePtr tree = (TtreePtr)tree_data(h);
  Node node  = (Node)tree_root(tree);
  int kind;
  int id = 0 ;
  int x,
      y = YmaxCanvas(tree)-TREE_TOP_MARGIN;

  while(node)
  {
    int text_x = 0 ;

    x = TREE_LEFT_MARGIN + NODE_X* node_depth(node);
    if( node_visible(node) == YES ) y-= NODE_Y ;

    kind = node_kind(node);

    /* if node has a text associated to it... */
    if(node_name(node))
    {
      /* Calculates its dimensions */
      iupdrvStringSize(tree->self,node_name(node), &text_x, NULL);
    }

    switch(kind)
    {
      case BRANCH:
        {
          if(node_state(node) == EXPANDED)
          {
            /* if node isn't root, looks for button to collapse */
            if(node!=tree_root(tree) && treeCollapseButton(h,xmouse,ymouse,x,y))
	            return id;
          }
          else
          {
            /* if node isn't root, looks for button to expand */
            if((node!=tree_root(tree) && treeExpandButton(h,xmouse,ymouse,x,y)))
	            return id;
          }
      
          /* Looks for branch */
          if(treeCollapsedBranch(h,xmouse,ymouse,x,y))
	          return id;
        }
        break;

      case LEAF:
        {
          /* Looks for leaf */
          if(treeLeaf(h,xmouse, ymouse, x, y))
            return id;	
        }
        break;

      default:
        {
          assert(0); /* Alguma coisa tem que ser */
          return -1;
        }
    }

    /* Looks for associated text */
    if(treeText(h,node,text_x,xmouse,ymouse,x,y))
      return id;

    node = node_next(node);
    id++;
  }
  
  return -1;
}

void treefindStartEndSelection(Ihandle *h, int *begin, int *end)
{
  *begin = atoi( IupGetAttribute(h, IUP_STARTING) );
  *end   = atoi( IupGetAttribute(h, IUP_VALUE)    );

  if(*begin > *end)
  {
    int temp ;
    temp   = *end;
    *end   = *begin;
    *begin = temp;      
  }
}

Node treefindNodeFromId(Ihandle *h, int id)
{
  TtreePtr tree = (TtreePtr)tree_data(h);
  Node node = tree_root(tree);

  while(node != NULL && id != 0)
  {
    node = node_next(node);
    id--;
  }
  assert(node);
  return node;
}

/* 
 * If given id_string is "" retuns the selected node
 */
Node treefindNodeFromString( Ihandle *h, char* id_string )
{
  TtreePtr tree = (TtreePtr)tree_data(h);
  Node node = (Node)tree_selected(tree);

  if(id_string[0])
  {
    int id = atoi(id_string);
  
    node = tree_root(tree);

    while(node && id)
    {
      node = node_next(node);
      id--;
    }
    return node;
  }
  return tree_selected(tree);
}

/* 
 * Returns the first marked node found (root does not count!!!)
 */
Node treefindMarked( Ihandle *h, char* id_string )
{
  TtreePtr tree = (TtreePtr)tree_data(h);
  Node node ;

  if(id_string[0])
  {
    node = (Node) treefindNodeFromString(h, id_string) ;	  
  }
  else
  {
    node = (Node)tree_root(tree);	  
  }
  
  while(node)
  {
    if(node_marked(node) == YES && node != tree_root(tree))
    {
      return node ;
    }
    
    node = node_next(node);
  }
  return NULL ;  
}
	
Node treefindPrevious(Node temp, Node node)
{
  assert(node);
  if(node == NULL)
    return NULL;

  while(temp && node_next(temp)!=node)
    temp = node_next(temp);
  
  return temp;
}

Node treefindParent(Ihandle* h, char* id_string)
{
  TtreePtr tree = (TtreePtr)tree_data(h);
  Node node = treefindNodeFromString(h, id_string);
  Node temp = (Node)tree_root(tree);

  if(node == temp)return NULL;
  
  while(node && (node_depth(node) >= node_depth(treefindNodeFromString(h, id_string))))
  {
    node = treefindPrevious(temp, node);	  
  }
  
  return node ;
}

int treefindMaxDepth(Node node)
{
  int depth = 0;
	
  while(node)
  {
    if(node_depth(node) + (node_kind(node) == BRANCH) > depth) depth = node_depth(node) + (node_kind(node) == BRANCH) ;
    node = node_next(node);
  }
  return depth ;
}

int treefindNumNodes(Ihandle *h)
{
  TtreePtr tree = (TtreePtr)tree_data(h);
  Node node = (Node)tree_root(tree);
  int i = 0 ;
	  
  while(node)
  {
    if(node_visible(node) == YES)i++;
    node = node_next(node);
  }

  return i; 
}

int treefindTotNumNodes(Ihandle *h)
{
  TtreePtr tree = (TtreePtr)tree_data(h);
  Node node = (Node)tree_root(tree);
  int i = 0 ;
	  
  while(node)
  {
    i++;
    node = node_next(node);
  }

  return i; 
}

float treefindNumNodesInCanvas(Ihandle *h)
{
  TtreePtr tree = (TtreePtr)tree_data(h);
  return ((float)(YmaxCanvas(tree)-(float)TREE_TOP_MARGIN)/(float)NODE_Y);
}

int treefindNodeId(Ihandle *h, Node node)
{
  TtreePtr tree = (TtreePtr)tree_data(h);
  Node tmp = tree_root(tree);
  int i = 0;

  while(node != tmp)
  {
    if(tmp == NULL)
      return -1;
    tmp = node_next(tmp);
    i++;
  }
  return i;
}
