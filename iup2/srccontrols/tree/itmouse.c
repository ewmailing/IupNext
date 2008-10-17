/** \file
 * \brief iuptree control
 * Functions used to handle the mouse
 *
 * See Copyright Notice in iup.h
 * $Id: itmouse.c,v 1.1 2008-10-17 06:20:41 scuri Exp $
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#include <iup.h>
#include <iupcpi.h>
#include <cd.h>

#include "iuptree.h"
#include "iupcompat.h"
#include "treedef.h"
#include "itcallback.h"
#include "treecd.h"
#include "itdraw.h"
#include "itlimits.h"
#include "itmouse.h"
#include "itkey.h"
#include "itfind.h"
#include "itgetset.h"
#include "itedit.h"

/* Variable that stores the control treeKey status */
extern int tree_ctrl;

/* Variable that stores the tree_shift treeKey status */
extern int tree_shift;

static int tree_drag = 0;
static int tree_drag_id;
static int tree_drag_mouse = 0;
static int tree_drag_mouse_x = 0;
static int tree_drag_mouse_y = 0;

/**************************************************************************
***************************************************************************
*
*   Internal functions
*
***************************************************************************
***************************************************************************/

static void treeDrag(Ihandle* h, int xmouse, int ymouse)
{
  TtreePtr tree=(TtreePtr)tree_data(h);
  Node node=(Node)tree_root(tree);
  int x,y=YmaxCanvas(tree)-TREE_TOP_MARGIN;

  tree_drag = 0;
  tree_drag_mouse = 0;

  while(node)
  {
    int text_x;
    text_x = 0;

    x = TREE_LEFT_MARGIN + NODE_X* node_depth(node);
    if( node_visible(node) == YES ) y-= NODE_Y;

    /* if node has a text associated to it... */
    if(node_name(node))
    {
      /* Calculates its dimensions */
      iupdrvStringSize(tree->self,node_name(node), &text_x, NULL);
    }

    /* Looks for associated text (leaf or branch) */
    if(treeText(h,node,text_x,xmouse,ymouse,x,y))
    {
      tree_drag = 1;
      tree_drag_id = treefindNodeId(h, node);
      tree_drag_mouse = 1;
      tree_drag_mouse_x = xmouse;
      tree_drag_mouse_y = ymouse;
      break;
    }

    node = node_next(node);
  }
}

static void treeDrop(Ihandle* h, int xmouse, int ymouse, int shift, int control)
{
  TtreePtr tree=(TtreePtr)tree_data(h);
  Node node=(Node)tree_root(tree);
  int x,y=YmaxCanvas(tree)-TREE_TOP_MARGIN;
  int tree_drop_id = 0;

  while(node)
  {
    int text_x;
    text_x = 0;

    x = TREE_LEFT_MARGIN + NODE_X* node_depth(node);
    if( node_visible(node) == YES ) y-= NODE_Y;

    /* if node has a text associated to it... */
    if(node_name(node))
    {
      /* Calculates its dimensions */
      iupdrvStringSize(tree->self,node_name(node), &text_x, NULL);
    }

    /* Looks for associated text (leaf or branch) */
    if(treeText(h,node,text_x,xmouse,ymouse,x,y))
    {
      tree_drag = 0;
      tree_drop_id = treefindNodeId(h, node);
      break;
    }

    node = node_next(node);
  }

  IupSetAttribute(h, IUP_CURSOR, "ARROW");
  tree_drag_mouse = 0;

  if (tree_drag || tree_drag_id==tree_drop_id)
  {
    tree_drag = 0;
  }
  else
  {
    treecallDragDropCb(h, tree_drag_id, tree_drop_id, tree_shift, tree_ctrl);
  }
}

/* 
 * This function returns 1 when the branch was clicked and 0 otherwise
 */
static int LeftPressOnBranch(Ihandle *h, Node node, int dclick, int xmouse, int ymouse, int x, int y, int *willmark)
{
  TtreePtr tree=(TtreePtr)tree_data(h);

  if(node_state(node) == EXPANDED)
  {
    if(treeCollapsedBranch(h,xmouse,ymouse,x,y))
      *willmark = 1;

    /* if node isn't root, looks for button to collapse */
    if((node!=tree_root(tree) && treeCollapseButton(h,xmouse,ymouse,x,y)) ||
      /* looks for branch if it is a double click */
      (dclick && treeCollapsedBranch(h,xmouse,ymouse,x,y)))
    {
      treegetsetSetStateOfNode(h, node, IUP_COLLAPSED);
      while(node_visible(tree_selected(tree)) == NO)
        treegetsetSetValue(h, IUP_PREVIOUS, 1);
	    return 1;
    }
  }
  else
  {
    if(treeCollapsedBranch(h,xmouse,ymouse,x,y))
      *willmark = 1;

    /* if node isn't root, looks for button to expand */
    if((node!=tree_root(tree) && treeExpandButton(h,xmouse,ymouse,x,y))||
      /* If it is a double click, looks for branch */
      ((dclick && treeCollapsedBranch(h,xmouse,ymouse,x,y))))
    {
      treegetsetSetStateOfNode(h, node, IUP_EXPANDED);
	    return 1;
    }
  }

  /* Looks for branch - user click in the branch image */
  if(treeCollapsedBranch(h, xmouse, ymouse, x, y))
  {
    *willmark = 1;
	  
    /* If it is a double click */
    if(dclick)
	  {
      Node temp = tree_selected(tree);
      tree_selected(tree) = node;
      treegetsetSetState(h, "", IUP_EXPANDED);
      tree_selected(tree) = temp;
      return 1;
    }
  }
  return 0;
}

static void LeftPress(Ihandle *h, int xmouse, int ymouse, int mouse_shift, int mouse_ctrl, int dclick)
{
  TtreePtr tree=(TtreePtr)tree_data(h);
  Node node=(Node)tree_root(tree);
  int willmark = 0;
  int x,y=YmaxCanvas(tree)-TREE_TOP_MARGIN;

  tree_ctrl = mouse_ctrl;
  tree_shift = mouse_shift;

  while(node)
  {
    int text_x;
    text_x = 0;

    x = TREE_LEFT_MARGIN + NODE_X* node_depth(node);
    if( node_visible(node) == YES ) y-= NODE_Y;

    /* if node has a text associated to it... */
    if(node_name(node))
    {
      /* Calculates its dimensions */
      iupdrvStringSize(tree->self,node_name(node), &text_x, NULL);
    }

    if(node_kind(node) == BRANCH)
    {
      /* Looks for click on branch image only */
      if(LeftPressOnBranch(h, node, dclick, xmouse, ymouse, x, y, &willmark))
      {
        /* User only pressed in the expand/collapse button */
        treeRepaint(h);
        return;
      }
    }
    else /* node is a leaf */
    {
      /* Looks for click on leaf image only */
      if(treeLeaf(h,xmouse, ymouse, x, y))
      {
	      willmark = 1;
	
	      /* If it is a double click */
        if(dclick)
        {
	        treecallExecuteLeafCb(h);
          break;
        }
      }
    }

    /* Looks for associated text (leaf or branch) */
    if(treeText(h,node,text_x,xmouse,ymouse,x,y))
    {
      willmark = 1; 
	    
      /* If it is a double click */
      if(dclick)
      {
        if (IupGetInt(h, "SHOWRENAME"))
        {
          if (!treeNodeCalcPos(h, &x, &y, &text_x))
            break;

          treeEditShow(h,text_x,x,y);

#ifdef _MOTIF_
          if (atoi(IupGetGlobal("MOTIFNUMBER")) < 2203) /* since OpenMotif version 2.2.3 this is not necessary */
            IupSetAttribute(h, "_IUPTREE_DOUBLE_CLICK", "1");
#endif
        }
        else
	        treecallRenameNodeCb(h);
        break;
      }
    }

    if(willmark)
    {
      tree_selected(tree) = node;
      break;
    }

    node = node_next(node);
  }

  /* Deselects all nodes control and tree_shift were not pressed */
  if(!tree_ctrl && !tree_shift)
    treegetsetSetValue(h, IUP_CLEARALL, 1);

  if(node)
  {
    if(tree_ctrl == YES)
    {
      treegetsetSetValue(h, IUP_INVERT, 1); /* Control inverts selection */
    }
    else
    {
      if(willmark || node_kind(node) != BRANCH)
      {
        int test = IUP_DEFAULT;
        int id = treefindNodeId(h, node);
        if(id == -1)
          return;

        if ((tree_shift != YES || !IupGetCallback(h,"MULTISELECTION_CB")) && (node_visible(node) == YES))
        {
          if(node_marked(node) == NO)
            test = treecallSelectionCb(h, id, 1);
          if(test != IUP_IGNORE)
            node_marked(node) = YES;
        }
      }
    }
  
    if(tree_shift == YES)
    {
      /* Marks block (which is from STARTING to last selected node) */
      treegetsetSetValue(h, IUP_BLOCK, 1);
    }
    else
    {
      /* If tree_shift is not pressed, STARTING becomes the selected node */
      treegetsetSetStarting(h,treegetsetGetValue(h));
    }
  }

  treeRepaint(h);
}

static void RightPress(Ihandle *h, int xmouse, int ymouse, char *r)
{
  TtreePtr tree=(TtreePtr)tree_data(h);
  Node node=(Node)tree_root(tree);
  int kind;
  int x,y=YmaxCanvas(tree)-TREE_TOP_MARGIN;
  int id = 0;

  while(node)
  {
    int text_x;
    text_x = 0;

    x = TREE_LEFT_MARGIN + NODE_X* node_depth(node);
    if( node_visible(node) == YES ) y-= NODE_Y;

    kind = node_kind(node);

    /* if node has a text associated to it... */
    if(node_name(node))
    {
      /* Calculates its dimensions */
      iupdrvStringSize(tree->self,node_name(node), &text_x, NULL);
    }

    if(kind == BRANCH)
    {
      if(node_state(node) == EXPANDED)
      {
        /* if node isn't root, looks for button to collapse */
        if(node!=tree_root(tree) && treeCollapseButton(h,xmouse,ymouse,x,y))
        {
          treecallRightClickCb (h, id, r);
	        return;
        }
      }
      else
      {
        /* if node isn't root, looks for button to expand */
        if((node!=tree_root(tree) && treeExpandButton(h,xmouse,ymouse,x,y)))
        {
	        treecallRightClickCb (h, id, r);
	        return;
        }
      }
      
      /* Looks for click on branch image only */
      if(treeCollapsedBranch(h,xmouse,ymouse,x,y))
      {
	      treecallRightClickCb (h, id, r);
        return;
      }
    }
    else /* node is a leaf */
    {
      /* Looks for click on leaf image only */
      if(treeLeaf(h,xmouse, ymouse, x, y))
      {
	      treecallRightClickCb (h, id, r);
        return;	
      }
    }

    /* Looks for associated text */
    if(treeText(h,node,text_x,xmouse,ymouse,x,y))
    {
      treecallRightClickCb (h, id, r);
      return;
    }

    node = node_next(node);
    id++;
  }
  
  treecallRightClickCb(h, -1, r);
}


/**************************************************************************
***************************************************************************
*
*   Exported functions
*
***************************************************************************
***************************************************************************/

/*

%F Callback called when a mouse button is pressed or released
%i hm : tree's handle,
   b : mouse button identifier [IUP_BUTTON1,IUP_BUTTON2 or IUP_BUTTON3],
   press : 1 if the button has been pressed 0, if it has been released,
   x,y : mouse position,
   r : string containing which keys [SHIFT,CTRL e ALT] are pressed
%o Returns IUP_DEFAULT.

*/

int treeMouseButtonCb (Ihandle *ht, int b, int press, int x, int y, char *r)
{
  TtreePtr tree = (TtreePtr)tree_data(ht);

  if (press)
  {
    /* The edit Kill Focus is not called when the user clicks in the parent canvas. 
       so we have to compensate that. */
    treeEditCheckHidden(ht);
  }

  if (b==IUP_BUTTON1)
  {
    if (!press && tree_drag)
    {
      int control = 0;
      int shift = 0;

      if(tree_ctrl(tree) == YES)
        control = iscontrol(r);
      if(tree_shift(tree) == YES)
        shift = isshift(r);

      treeDrop(ht, x, y, shift, control);
    }

    if ((!press || isdouble(r)) && !tree_drag)
    {
      int dclick = isdouble(r); 
      int control = 0;
      int shift = 0;

      if(tree_ctrl(tree) == YES)
        control = iscontrol(r);
      if(tree_shift(tree) == YES)
        shift = isshift(r);

      LeftPress(ht, x, y, shift, control, dclick);
    }

    if (press && !isdouble(r))
    {
      if (iupCheck(ht, "SHOWDRAGDROP")==YES)
        treeDrag(ht, x, y);
    }
  }

  if (b==IUP_BUTTON3)
  { 
    if (!press)
    {
      RightPress (ht,x,y,r);
    }
  }

  return IUP_DEFAULT;
}

int treeMouseMotionCb (Ihandle *ht, int x, int y, char *r)
{
  if (tree_drag_mouse && (tree_drag_mouse_x != x || tree_drag_mouse_y != y))
  {
    tree_drag_mouse = 0;
    IupSetAttribute(ht, IUP_CURSOR, "IupTreeDragCursor");
  }
  return IUP_DEFAULT;
}

int treeCollapseButton(Ihandle* h, int xmouse, int ymouse, int x, int y)
{
  TtreePtr tree = (TtreePtr)tree_data(h);
  return treeInsideRegion(tree->cddbuffer,-treedrawGetLeft(h)+xmouse,
                             treedrawGetCanvasTop(h)+ymouse,
       		                   x+(NODE_WIDTH-TREE_COLLAPSE_WIDTH)/2+BUTTON_X,
                             y+BUTTON_LINE_Y - TREE_COLLAPSE_HEIGHT/2+1,
			                       TREE_COLLAPSE_WIDTH,
	                           TREE_COLLAPSE_HEIGHT);
}

int treeCollapsedBranch(Ihandle* h, int xmouse, int ymouse, int x, int y)
{
  TtreePtr tree = (TtreePtr)tree_data(h);
  return treeInsideRegion(tree->cddbuffer,-treedrawGetLeft(h)+xmouse,
                             treedrawGetCanvasTop(h)+ymouse,
			                       x,
	                           y,
	                           NODE_WIDTH,
	                           NODE_HEIGHT);
}

int treeExpandButton(Ihandle* h, int xmouse, int ymouse, int x, int y)
{
  TtreePtr tree = (TtreePtr)tree_data(h);
  return treeInsideRegion(tree->cddbuffer,-treedrawGetLeft(h)+xmouse,
 			     treedrawGetCanvasTop(h)+ymouse,
			     x+(NODE_WIDTH-TREE_EXPAND_WIDTH)/2+BUTTON_X,
	         y+BUTTON_LINE_Y - TREE_EXPAND_HEIGHT/2+1,
			     TREE_EXPAND_WIDTH,
			     TREE_EXPAND_HEIGHT);
}

int treeLeaf(Ihandle* h, int xmouse, int ymouse, int x, int y)
{
  TtreePtr tree = (TtreePtr)tree_data(h);
  return treeInsideRegion(tree->cddbuffer,-treedrawGetLeft(h)+xmouse,
	                     treedrawGetCanvasTop(h)+ymouse,
	                     x,
	                     y,
	                     NODE_WIDTH,
	                     NODE_HEIGHT);
}

int treeText(Ihandle* h, Node node, int text_x, int xmouse, int ymouse, int x, int y)
{
  TtreePtr tree = (TtreePtr)tree_data(h);
  return (node_name(node) &&
      treeInsideRegion(tree->cddbuffer,-treedrawGetLeft(h)+xmouse,
    	                  treedrawGetCanvasTop(h)+ymouse,
			  x + NODE_WIDTH,
			  y - TEXT_BOX_OFFSET_Y,
			  2*TEXT_MARGIN_X-TEXT_RIGHT_FIX + text_x,
                          NODE_HEIGHT));
}
