/** \file
 * \brief iuptree control
 * Functions used to find nodes in the tree
 *
 * See Copyright Notice in iup.h
 */

#include <stdio.h>
#include <stdlib.h>

#include "iup.h"
#include "iupcbs.h"
#include "iuptree.h"
#include "iupkey.h"

#include "iup_object.h"
#include "iup_attrib.h"
#include "iup_str.h"
#include "iup_drv.h"
#include "iup_drvfont.h"
#include "iup_stdcontrols.h"
#include "iup_controls.h"

#include "treedef.h" 
#include "itgetset.h"
#include "itfind.h"
#include "itdraw.h"
#include "itmouse.h"


int iTreeFindNodeIdFromXY(Ihandle* ih, int xmouse, int ymouse)
{
  Node node  = (Node)ih->data->root;
  int id = 0;
  int kind;
  int x;
  int y = ih->data->YmaxC - ITREE_TREE_TOP_MARGIN;

  while(node)
  {
    int text_x = 0;

    x = ITREE_TREE_LEFT_MARGIN + ITREE_NODE_X * node->depth;
    if(node->visible == YES)
      y -= ITREE_NODE_Y;

    kind = node->kind;

    /* if node has a text associated to it... */
    if(node->name)
    {
      /* Calculates its dimensions */
      text_x = iupdrvFontGetStringWidth(ih, node->name);
    }

    switch(kind)
    {
      case ITREE_BRANCH:
        {
          if(node->state == ITREE_EXPANDED)
          {
            /* if node isn't root, looks for button to collapse */
            if(node != ih->data->root && iTreeMouseCollapseButton(ih, xmouse, ymouse, x, y))
              return id;
          }
          else
          {
            /* if node isn't root, looks for button to expand */
            if((node != ih->data->root && iTreeMouseExpandButton(ih, xmouse, ymouse, x, y)))
              return id;
          }
      
          /* Looks for branch */
          if(iTreeMouseCollapsedBranch(ih, xmouse, ymouse, x, y))
            return id;
        }
        break;

      case ITREE_LEAF:
        {
          /* Looks for leaf */
          if(iTreeMouseLeaf(ih, xmouse, ymouse, x, y))
            return id;  
        }
        break;

      default:
        {
          return -1;
        }
    }

    /* Looks for associated text */
    if(iTreeMouseText(ih, node, text_x, xmouse, ymouse, x, y))
      return id;

    node = node->next;
    id++;
  }
  
  return -1;
}

void iTreeFindStartEndSelection(Ihandle* ih, int* begin, int* end)
{
  *begin = atoi(IupGetAttribute(ih, "STARTING"));
  *end   = atoi(IupGetAttribute(ih, "VALUE"));

  if(*begin > *end)
  {
    int temp;
    temp   = *end;
    *end   = *begin;
    *begin = temp;      
  }
}

Node iTreeFindNodeFromId(Ihandle* ih, int id)
{
  Node node = ih->data->root;

  while(node != NULL && id != 0)
  {
    node = node->next;
    id--;
  }
  return node;
}

/*  If given id_string is "" returns the selected node */
Node iTreeFindNodeFromString(Ihandle* ih, const char* id_string)
{
  Node node = (Node)ih->data->selected;

  if(id_string[0])
  {
    int id = atoi(id_string);
  
    node = ih->data->root;

    while(node && id)
    {
      node = node->next;
      id--;
    }
    return node;
  }
  return ih->data->selected;
}

/* Returns the first marked node found (root does not count!!!) */
Node iTreeFindMarked(Ihandle* ih, const char* id_string)
{
  Node node;

  if(id_string[0])
  {
    node = (Node) iTreeFindNodeFromString(ih, id_string);    
  }
  else
  {
    node = (Node)ih->data->root;    
  }
  
  while(node)
  {
    if(node->marked == YES && node != ih->data->root)
    {
      return node;
    }
    
    node = node->next;
  }
  return NULL;  
}
  
Node iTreeFindPrevious(Node temp, Node node)
{
  if(node == NULL)
    return NULL;

  while(temp && temp->next != node)
    temp = temp->next;
  
  return temp;
}

Node iTreeFindParent(Ihandle* ih, const char* id_string)
{

  Node node = iTreeFindNodeFromString(ih, id_string);
  Node temp = (Node)ih->data->root;

  if(node == temp)return NULL;
  
  while(node && (node->depth >= (iTreeFindNodeFromString(ih, id_string))->depth))
  {
    node = iTreeFindPrevious(temp, node);    
  }
  
  return node;
}

int iTreeFindMaxDepth(Node node)
{
  int depth = 0;
  
  while(node)
  {
    if(node->depth + (node->kind == ITREE_BRANCH) > depth) depth = node->depth + (node->kind == ITREE_BRANCH);
    node = node->next;
  }
  return depth;
}

int iTreeFindNumNodes(Ihandle* ih)
{

  Node node = (Node)ih->data->root;
  int i = 0;
    
  while(node)
  {
    if(node->visible == YES)i++;
    node = node->next;
  }

  return i; 
}

int iTreeFindTotNumNodes(Ihandle* ih)
{

  Node node = (Node)ih->data->root;
  int i = 0;
    
  while(node)
  {
    i++;
    node = node->next;
  }

  return i; 
}

float iTreeFindNumNodesInCanvas(Ihandle* ih)
{
  return ((float)(ih->data->YmaxC - (float)ITREE_TREE_TOP_MARGIN) / (float)ITREE_NODE_Y);
}

int iTreeFindNodeId(Ihandle* ih, Node node)
{
  Node tmp = ih->data->root;
  int i = 0;

  while(node != tmp)
  {
    if(tmp == NULL)
      return -1;
    tmp = tmp->next;
    i++;
  }
  return i;
}
