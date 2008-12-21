/** \file
 * \brief iuptree control
 * Functions used to find nodes in the tree
 *
 * See Copyright Notice in "iup.h"
 */

#include <stdio.h>
#include <stdlib.h>

#include "iup.h"
#include "iupcbs.h"
#include "iupkey.h"

#include "iup_object.h"
#include "iup_attrib.h"
#include "iup_str.h"
#include "iup_drv.h"
#include "iup_drvfont.h"
#include "iup_stdcontrols.h"
#include "iup_controls.h"

#include <cd.h>

#include "iuptree_draw.h"
#include "iuptree_def.h" 
#include "iuptree_getset.h"
#include "iuptree_find.h"
#include "iuptree_mouse.h"


int iupTreeFindNodeIdFromXY(Ihandle* ih, int xmouse, int ymouse)
{
  ItreeNodePtr node  = (ItreeNodePtr)ih->data->root;
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
            if(node != ih->data->root && iupTreeMouseCollapseButton(ih, xmouse, ymouse, x, y))
              return id;
          }
          else
          {
            /* if node isn't root, looks for button to expand */
            if((node != ih->data->root && iupTreeMouseExpandButton(ih, xmouse, ymouse, x, y)))
              return id;
          }
      
          /* Looks for branch */
          if(iupTreeMouseCollapsedBranch(ih, xmouse, ymouse, x, y))
            return id;
        }
        break;

      case ITREE_LEAF:
        {
          /* Looks for leaf */
          if(iupTreeMouseLeaf(ih, xmouse, ymouse, x, y))
            return id;  
        }
        break;

      default:
        {
          return -1;
        }
    }

    /* Looks for associated text */
    if(iupTreeMouseText(ih, node, text_x, xmouse, ymouse, x, y))
      return id;

    node = node->next;
    id++;
  }
  
  return -1;
}

void iupTreeFindStartEndSelection(Ihandle* ih, int* begin, int* end)
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

ItreeNodePtr iupTreeFindNodeFromId(Ihandle* ih, int id)
{
  ItreeNodePtr node = ih->data->root;

  while(node != NULL && id != 0)
  {
    node = node->next;
    id--;
  }
  return node;
}

/*  If given id_string is "" returns the selected node */
ItreeNodePtr iupTreeFindNodeFromString(Ihandle* ih, const char* id_string)
{
  ItreeNodePtr node = (ItreeNodePtr)ih->data->selected;

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
ItreeNodePtr iupTreeFindMarked(Ihandle* ih, const char* id_string)
{
  ItreeNodePtr node;

  if(id_string[0])
  {
    node = (ItreeNodePtr) iupTreeFindNodeFromString(ih, id_string);    
  }
  else
  {
    node = (ItreeNodePtr)ih->data->root;    
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
  
ItreeNodePtr iupTreeFindPrevious(ItreeNodePtr temp, ItreeNodePtr node)
{
  if(node == NULL)
    return NULL;

  while(temp && temp->next != node)
    temp = temp->next;
  
  return temp;
}

ItreeNodePtr iupTreeFindParent(Ihandle* ih, const char* id_string)
{

  ItreeNodePtr node = iupTreeFindNodeFromString(ih, id_string);
  ItreeNodePtr temp = (ItreeNodePtr)ih->data->root;

  if(node == temp)return NULL;
  
  while(node && (node->depth >= (iupTreeFindNodeFromString(ih, id_string))->depth))
  {
    node = iupTreeFindPrevious(temp, node);    
  }
  
  return node;
}

int iupTreeFindMaxDepth(ItreeNodePtr node)
{
  int depth = 0;
  
  while(node)
  {
    if(node->depth + (node->kind == ITREE_BRANCH) > depth) depth = node->depth + (node->kind == ITREE_BRANCH);
    node = node->next;
  }
  return depth;
}

int iupTreeFindNumNodes(Ihandle* ih)
{

  ItreeNodePtr node = (ItreeNodePtr)ih->data->root;
  int i = 0;
    
  while(node)
  {
    if(node->visible == YES)i++;
    node = node->next;
  }

  return i; 
}

int iupTreeFindTotNumNodes(Ihandle* ih)
{

  ItreeNodePtr node = (ItreeNodePtr)ih->data->root;
  int i = 0;
    
  while(node)
  {
    i++;
    node = node->next;
  }

  return i; 
}

float iupTreeFindNumNodesInCanvas(Ihandle* ih)
{
  return ((float)(ih->data->YmaxC - (float)ITREE_TREE_TOP_MARGIN) / (float)ITREE_NODE_Y);
}

int iupTreeFindNodeId(Ihandle* ih, ItreeNodePtr node)
{
  ItreeNodePtr tmp = ih->data->root;
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
