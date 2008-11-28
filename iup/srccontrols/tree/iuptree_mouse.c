/** \file
 * \brief iuptree control
 * Functions used to handle the mouse
 *
 * See Copyright Notice in iup.h
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

#include "iuptree_draw.h"
#include "iuptree_def.h"
#include "iuptree_callback.h"
#include "iuptree_key.h"
#include "iuptree_find.h"
#include "iuptree_getset.h"
#include "iuptree_mouse.h"
#include "iuptree_edit.h"


/* Variable that stores the control iTreeKey status */
extern int tree_ctrl;

/* Variable that stores the tree_shift iTreeKey status */
extern int tree_shift;

static int tree_drag = 0;
static int tree_drag_id;
static int tree_drag_mouse = 0;
static int tree_drag_mouse_x = 0;
static int tree_drag_mouse_y = 0;


/**************************************************************************
*   Internal functions
***************************************************************************/

static void iTreeMouseDrag(Ihandle* ih, int xmouse, int ymouse)
{
  Node node = (Node)ih->data->root;
  int x;
  int y = ih->data->YmaxC - ITREE_TREE_TOP_MARGIN;

  tree_drag = 0;
  tree_drag_mouse = 0;

  while(node)
  {
    int text_x;
    text_x = 0;

    x = ITREE_TREE_LEFT_MARGIN + ITREE_NODE_X * node->depth;
    if(node->visible == YES)
      y-= ITREE_NODE_Y;

    /* if node has a text associated to it... */
    if(node->name)
    {
      /* Calculates its dimensions */
      text_x = iupdrvFontGetStringWidth(ih, node->name);
    }

    /* Looks for associated text (leaf or branch) */
    if(iTreeMouseText(ih, node, text_x, xmouse, ymouse, x, y))
    {
      tree_drag = 1;
      tree_drag_id = iTreeFindNodeId(ih, node);
      tree_drag_mouse = 1;
      tree_drag_mouse_x = xmouse;
      tree_drag_mouse_y = ymouse;
      break;
    }

    node = node->next;
  }
}

static void iTreeMouseDrop(Ihandle* ih, int xmouse, int ymouse, int shift, int control)
{
  Node node = (Node)ih->data->root;
  int x;
  int y = ih->data->YmaxC - ITREE_TREE_TOP_MARGIN;
  int tree_drop_id = 0;
  (void)control;
  (void)shift;

  while(node)
  {
    int text_x;
    text_x = 0;

    x = ITREE_TREE_LEFT_MARGIN + ITREE_NODE_X * node->depth;
    if(node->visible == YES)
      y-= ITREE_NODE_Y;

    /* if node has a text associated to it... */
    if(node->name)
    {
      /* Calculates its dimensions */
      text_x = iupdrvFontGetStringWidth(ih, node->name);
    }

    /* Looks for associated text (leaf or branch) */
    if(iTreeMouseText(ih, node, text_x, xmouse, ymouse, x, y))
    {
      tree_drag = 0;
      tree_drop_id = iTreeFindNodeId(ih, node);
      break;
    }

    node = node->next;
  }

  IupSetAttribute(ih, "CURSOR", "ARROW");
  tree_drag_mouse = 0;

  if(tree_drag || tree_drag_id==tree_drop_id)
    tree_drag = 0;
  else
    iTreeCallDragDropCB(ih, tree_drag_id, tree_drop_id, tree_shift, tree_ctrl);
}

/* This function returns 1 when the branch was clicked and 0 otherwise */
static int iTreeMouseLeftPressOnBranch(Ihandle* ih, Node node, int dclick, int xmouse, int ymouse, int x, int y, int* willmark)
{
  if(node->state == ITREE_EXPANDED)
  {
    if(iTreeMouseCollapsedBranch(ih, xmouse, ymouse, x, y))
      *willmark = 1;

    /* if node isn't root, looks for button to collapse */
    if((node != ih->data->root && iTreeMouseCollapseButton(ih, xmouse, ymouse, x, y)) ||
      /* looks for branch if it is a double click */
      (dclick && iTreeMouseCollapsedBranch(ih, xmouse, ymouse, x, y)))
    {
      iTreeGSSetStateOfNode(ih, node, "COLLAPSED");
      while(ih->data->selected->visible == NO)
        iTreeGSSetValue(ih, "PREVIOUS", 1);
      return 1;
    }
  }
  else
  {
    if(iTreeMouseCollapsedBranch(ih, xmouse, ymouse, x, y))
      *willmark = 1;

    /* if node isn't root, looks for button to expand */
    if((node != ih->data->root && iTreeMouseExpandButton(ih, xmouse, ymouse, x, y)) ||
      /* If it is a double click, looks for branch */
      ((dclick && iTreeMouseCollapsedBranch(ih, xmouse, ymouse, x, y))))
    {
      iTreeGSSetStateOfNode(ih, node, "EXPANDED");
      return 1;
    }
  }

  /* Looks for branch - user click in the branch image */
  if(iTreeMouseCollapsedBranch(ih, xmouse, ymouse, x, y))
  {
    *willmark = 1;
    
    /* If it is a double click */
    if(dclick)
    {
      Node temp = ih->data->selected;
      ih->data->selected = node;
      iTreeGSSetState(ih, "", "EXPANDED");
      ih->data->selected = temp;
      return 1;
    }
  }
  return 0;
}

static void iTreeMouseLeftPress(Ihandle* ih, int xmouse, int ymouse, int mouse_shift, int mouse_ctrl, int dclick)
{
  Node node = (Node)ih->data->root;
  int willmark = 0;
  int x;
  int y = ih->data->YmaxC - ITREE_TREE_TOP_MARGIN;

  tree_ctrl  = mouse_ctrl;
  tree_shift = mouse_shift;

  while(node)
  {
    int text_x;
    text_x = 0;

    x = ITREE_TREE_LEFT_MARGIN + ITREE_NODE_X * node->depth;
    if(node->visible == YES)
      y-= ITREE_NODE_Y;

    /* if node has a text associated to it... */
    if(node->name)
    {
      /* Calculates its dimensions */
      text_x = iupdrvFontGetStringWidth(ih, node->name);
    }

    if(node->kind == ITREE_BRANCH)
    {
      /* Looks for click on branch image only */
      if(iTreeMouseLeftPressOnBranch(ih, node, dclick, xmouse, ymouse, x, y, &willmark))
      {
        /* User only pressed in the expand/collapse button */
        iTreeRepaint(ih);
        return;
      }
    }
    else /* node is a leaf */
    {
      /* Looks for click on leaf image only */
      if(iTreeMouseLeaf(ih, xmouse, ymouse, x, y))
      {
        willmark = 1;
  
        /* If it is a double click */
        if(dclick)
        {
          iTreeCallExecuteLeafCB(ih);
          break;
        }
      }
    }

    /* Looks for associated text (leaf or branch) */
    if(iTreeMouseText(ih, node, text_x, xmouse, ymouse, x, y))
    {
      willmark = 1;
      
      /* If it is a double click */
      if(dclick)
      {
        if(IupGetInt(ih, "SHOWRENAME"))
        {
          if(!iTreeKeyNodeCalcPos(ih, &x, &y, &text_x))
            break;

          iTreeEditShow(ih, text_x, x, y);

#ifdef _MOTIF_
          if(atoi(IupGetGlobal("MOTIFNUMBER")) < 2203) /* since OpenMotif version 2.2.3 this is not necessary */
            IupSetAttribute(ih, "_IUPTREE_DOUBLE_CLICK", "1");
#endif
        }
        else
          iTreeCallRenameNodeCB(ih);
        break;
      }
    }

    if(willmark)
    {
      ih->data->selected = node;
      break;
    }

    node = node->next;
  }

  /* Deselects all nodes if control and tree_shift were not pressed */
  if(!tree_ctrl && !tree_shift)
    iTreeGSSetValue(ih, "CLEARALL", 1);

  if(node)
  {
    if(tree_ctrl == YES)
    {
      iTreeGSSetValue(ih, "INVERT", 1); /* Control inverts selection */
    }
    else
    {
      if(willmark || node->kind != ITREE_BRANCH)
      {
        int test = IUP_DEFAULT;
        int id = iTreeFindNodeId(ih, node);
        if(id == -1)
          return;

        if((tree_shift != YES || !IupGetCallback(ih, "MULTISELECTION_CB")) && (node->visible == YES))
        {
          if(node->marked == NO)
            test = iTreeCallSelectionCB(ih, id, 1);
          if(test != IUP_IGNORE)
            node->marked = YES;
        }
      }
    }
  
    if(tree_shift == YES)
    {
      /* Marks block (which is from STARTING to last selected node) */
      iTreeGSSetValue(ih, "BLOCK", 1);
    }
    else
    {
      /* If tree_shift is not pressed, STARTING becomes the selected node */
      iTreeGSSetStarting(ih, iTreeGSGetValue(ih));
    }
  }

  iTreeRepaint(ih);
}

static void iTreeMouseRightPress(Ihandle* ih, int xmouse, int ymouse, char* r)
{
  Node node = (Node)ih->data->root;
  int kind;
  int x;
  int y = ih->data->YmaxC - ITREE_TREE_TOP_MARGIN;
  int id = 0;

  while(node)
  {
    int text_x;
    text_x = 0;

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

    if(kind == ITREE_BRANCH)
    {
      if(node->state == ITREE_EXPANDED)
      {
        /* if node isn't root, looks for button to collapse */
        if(node != ih->data->root && iTreeMouseCollapseButton(ih, xmouse, ymouse, x, y))
        {
          iTreeCallRightClickCB(ih, id, r);
          return;
        }
      }
      else
      {
        /* if node isn't root, looks for button to expand */
        if((node != ih->data->root && iTreeMouseExpandButton(ih, xmouse, ymouse, x, y)))
        {
          iTreeCallRightClickCB(ih, id, r);
          return;
        }
      }
      
      /* Looks for click on branch image only */
      if(iTreeMouseCollapsedBranch(ih, xmouse, ymouse, x, y))
      {
        iTreeCallRightClickCB(ih, id, r);
        return;
      }
    }
    else /* node is a leaf */
    {
      /* Looks for click on leaf image only */
      if(iTreeMouseLeaf(ih, xmouse, ymouse, x, y))
      {
        iTreeCallRightClickCB(ih, id, r);
        return;
      }
    }

    /* Looks for associated text */
    if(iTreeMouseText(ih, node, text_x, xmouse, ymouse, x, y))
    {
      iTreeCallRightClickCB(ih, id, r);
      return;
    }

    node = node->next;
    id++;
  }
  
  iTreeCallRightClickCB(ih, -1, r);
}


/**************************************************************************
*   Exported functions
***************************************************************************/

/* Callback called when a mouse button is pressed or released
   - b     : mouse button identifier [IUP_BUTTON1, IUP_BUTTON2 or IUP_BUTTON3]
   - press : 1 if the button has been pressed 0, if it has been released
   - x, y  : mouse position
   - r     : string containing which keys [SHIFT, CTRL and ALT] are pressed
*/
int iTreeMouseButtonCB(Ihandle* ih, int b, int press, int x, int y, char* r)
{
  if(press)
  {
    /* The edit Kill Focus is not called when the user clicks in the parent
       canvas. So we have to compensate that. */
    iTreeEditCheckHidden(ih);
  }

  if(b == IUP_BUTTON1)
  {
    if(!press && tree_drag)
    {
      int control = 0;
      int shift   = 0;

      if(ih->data->tree_ctrl == YES)
        control = iscontrol(r);
      if(ih->data->tree_shift == YES)
        shift = isshift(r);

      iTreeMouseDrop(ih, x, y, shift, control);
    }

    if((!press || isdouble(r)) && !tree_drag)
    {
      int dclick  = isdouble(r); 
      int control = 0;
      int shift   = 0;

      if(ih->data->tree_ctrl == YES)
        control = iscontrol(r);
      if(ih->data->tree_shift == YES)
        shift = isshift(r);

      iTreeMouseLeftPress(ih, x, y, shift, control, dclick);
    }

    if(press && !isdouble(r))
    {
      if(iupAttribGetInt(ih, "SHOWDRAGDROP"))
        iTreeMouseDrag(ih, x, y);
    }
  }

  if(b == IUP_BUTTON3)
  { 
    if(!press)
    {
      iTreeMouseRightPress(ih, x, y, r);
    }
  }

  return IUP_DEFAULT;
}

int iTreeMouseMotionCB(Ihandle* ih, int x, int y, char* r)
{
  (void)r;
  if(tree_drag_mouse && (tree_drag_mouse_x != x || tree_drag_mouse_y != y))
  {
    tree_drag_mouse = 0;
    IupSetAttribute(ih, "CURSOR", "IupTreeDragCursor");
  }
  
  return IUP_DEFAULT;
}

/* Determines if a pixel is inside a region
   c   : canvas 
   x   : pixel horizontal coordinate
   y   : pixel vertical coordinate
   x0  : region left border
   y0  : region top border
   w   : region width
   h   : region height
*/
static int iTreeLimitsInsideRegion(cdCanvas* c, int x, int y, int x0, int y0, int w, int h )
{
  cdCanvasUpdateYAxis(c, &y);
  y++;
  
  return (x >= x0 && x <= x0 + w && y >= y0 && y <= y0 + h);
}

int iTreeMouseCollapseButton(Ihandle* ih, int xmouse, int ymouse, int x, int y)
{
  return iTreeLimitsInsideRegion(ih->data->cddbuffer, - iTreeDrawGetLeft(ih) + xmouse,
                             iTreeDrawGetCanvasTop(ih) + ymouse,
                              x + (ITREE_NODE_WIDTH - ITREE_TREE_COLLAPSE_WIDTH) / 2 + ITREE_BUTTON_X,
                             y + ITREE_BUTTON_LINE_Y - ITREE_TREE_COLLAPSE_HEIGHT / 2 + 1,
                             ITREE_TREE_COLLAPSE_WIDTH, ITREE_TREE_COLLAPSE_HEIGHT);
}

int iTreeMouseCollapsedBranch(Ihandle* ih, int xmouse, int ymouse, int x, int y)
{
  return iTreeLimitsInsideRegion(ih->data->cddbuffer, - iTreeDrawGetLeft(ih) + xmouse,
                             iTreeDrawGetCanvasTop(ih) + ymouse,
                             x, y, ITREE_NODE_WIDTH, ITREE_NODE_HEIGHT);
}

int iTreeMouseExpandButton(Ihandle* ih, int xmouse, int ymouse, int x, int y)
{
  return iTreeLimitsInsideRegion(ih->data->cddbuffer, - iTreeDrawGetLeft(ih) + xmouse,
                              iTreeDrawGetCanvasTop(ih) + ymouse,
                             x + (ITREE_NODE_WIDTH - ITREE_TREE_EXPAND_WIDTH) / 2 + ITREE_BUTTON_X,
                             y + ITREE_BUTTON_LINE_Y - ITREE_TREE_EXPAND_HEIGHT / 2 + 1,
                             ITREE_TREE_EXPAND_WIDTH, ITREE_TREE_EXPAND_HEIGHT);
}

int iTreeMouseLeaf(Ihandle* ih, int xmouse, int ymouse, int x, int y)
{
  return iTreeLimitsInsideRegion(ih->data->cddbuffer, - iTreeDrawGetLeft(ih) + xmouse,
                             iTreeDrawGetCanvasTop(ih) + ymouse,
                             x, y, ITREE_NODE_WIDTH, ITREE_NODE_HEIGHT);
}

int iTreeMouseText(Ihandle* ih, Node node, int text_x, int xmouse, int ymouse, int x, int y)
{
  return (node->name &&
      iTreeLimitsInsideRegion(ih->data->cddbuffer, - iTreeDrawGetLeft(ih) + xmouse,
                          iTreeDrawGetCanvasTop(ih) + ymouse,
                          x + ITREE_NODE_WIDTH, y - ITREE_TEXT_BOX_OFFSET_Y,
                          2 * ITREE_TEXT_MARGIN_X - ITREE_TEXT_RIGHT_FIX + text_x,
                          ITREE_NODE_HEIGHT));
}
