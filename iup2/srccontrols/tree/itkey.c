/** \file
 * \brief iuptree control
 * Functions used to handle the keyboard
 *
 * See Copyright Notice in iup.h
 *  */

#include <iupkey.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "iup.h"
#include "iuptree.h"
#include "iupcompat.h"
#include "istrutil.h"

#include "itkey.h"
#include "treedef.h"
#include "itcallback.h"
#include "itgetset.h"
#include "itfind.h"
#include "itedit.h"
#include "treecd.h"

int tree_ctrl = 0;
int tree_shift = 0;

int treeNodeCalcPos(Ihandle* h, int *x, int *y, int *text_x)
{
  int err;
  TtreePtr tree=(TtreePtr)tree_data(h);
  Node node = (Node)tree_root(tree);
  float posy = IupGetFloat(h, IUP_POSY);     
  float dy = IupGetFloat(h, IUP_DY);
  float posx = IupGetFloat(h, IUP_POSX);
  float dx = IupGetFloat(h, IUP_DX);

  CdActivate(tree,err);

  *y = (int)((1.0 + posy/dy)*(YmaxCanvas(tree)-TREE_TOP_MARGIN));

  while(node != tree_selected(tree))
  {
    if( node_visible(node) == YES ) *y -= NODE_Y;

    node = node_next(node);
    if (node == NULL)
      return 0;
  }

  *y -= NODE_Y;
  *x = (int)(TREE_LEFT_MARGIN - (XmaxCanvas(tree)-NODE_X)*posx/dx) + NODE_X * node_depth(node);

  /* if node has a text associated to it... */
  *text_x = 0;
  if(node_name(node))
  {
    /* Calculates its dimensions */
    iupdrvStringSize(tree->self, node_name(node), text_x, NULL);
  }

  return 1;
}

int treeKey(Ihandle *h, int c)
{
  tree_ctrl = NO;
  tree_shift = NO;
	
  switch(c)
  {
    case K_sHOME:
      if(iupStrEqual(IupGetAttribute(h,IUP_SHIFT),IUP_NO)) break ;
      tree_shift = YES;
    case K_cHOME:
      if(tree_shift == NO)tree_ctrl = YES;	    
    case K_HOME:
      treegetsetSetValue(h,IUP_ROOT,1);
      if(tree_shift == NO && tree_ctrl == NO)
        treegetsetSetStarting(h, treegetsetGetValue(h));
      treeRepaint(h);
    break;
    
    case K_sEND:
      if(iupStrEqual(IupGetAttribute(h,IUP_SHIFT),IUP_NO)) break ;
      tree_shift = YES;
    case K_cEND:
      if(tree_shift == NO)
      {  
        if(iupStrEqual(IupGetAttribute(h,IUP_CTRL),IUP_NO)) break ;
        tree_ctrl = YES;
      }
    case K_END:
      treegetsetSetValue(h,IUP_LAST,1);
      if(tree_shift == NO && tree_ctrl == NO)
        treegetsetSetStarting(h, treegetsetGetValue(h));
      treeRepaint(h);
    break;

    case K_sPGUP: 
      if(iupStrEqual(IupGetAttribute(h,IUP_SHIFT),IUP_NO)) break ;
      tree_shift = YES;
    case K_cPGUP:
      if(tree_shift == NO)
      {
        if(iupStrEqual(IupGetAttribute(h,IUP_CTRL),IUP_NO)) break ;
        tree_ctrl = YES; 
      }
    case K_PGUP:
      treegetsetSetValue(h,IUP_PGUP,1);
      if(tree_shift == NO && tree_ctrl == NO)
        treegetsetSetStarting(h, treegetsetGetValue(h));
      treeRepaint(h);
    break;
     
    case K_sPGDN:
      if(iupStrEqual(IupGetAttribute(h,IUP_SHIFT),IUP_NO)) break ;
      tree_shift = YES;
    case K_cPGDN:
      if(tree_shift == NO)
      {
        if(iupStrEqual(IupGetAttribute(h,IUP_CTRL),IUP_NO)) break ;
        tree_ctrl = YES;
      }
    case K_PGDN:
      treegetsetSetValue(h,IUP_PGDN,1);
      if(tree_shift == NO && tree_ctrl == NO)
        treegetsetSetStarting(h, treegetsetGetValue(h));
      treeRepaint(h);
    break;
    
    case K_sUP:
      if(iupStrEqual(IupGetAttribute(h,IUP_SHIFT),IUP_NO)) break ;
      tree_shift = YES;
    case K_cUP:
      if(tree_shift == NO)
      {
        if(iupStrEqual(IupGetAttribute(h,IUP_CTRL),IUP_NO)) break ;
        tree_ctrl = YES;
      }
    case K_UP:
      treegetsetSetValue(h,IUP_PREVIOUS,1);
      if(tree_shift == NO && tree_ctrl == NO)
        treegetsetSetStarting(h, treegetsetGetValue(h));
      treeRepaint(h);
    break;

    case K_sDOWN:
      if(iupStrEqual(IupGetAttribute(h,IUP_SHIFT),IUP_NO)) break ;
      tree_shift = YES;
    case K_cDOWN:
      if(tree_shift == NO)
      {
        if(iupStrEqual(IupGetAttribute(h,IUP_CTRL),IUP_NO)) break ;
        tree_ctrl = YES;
      }
    case K_DOWN:
      treegetsetSetValue(h,IUP_NEXT,1);
      if(tree_shift == NO && tree_ctrl == NO)
        treegetsetSetStarting(h, treegetsetGetValue(h));
      treeRepaint(h);
    break;

    case K_LEFT:
      treegetsetSetState(h, "", IUP_COLLAPSED);
      treeRepaint(h);
    break;

    case K_RIGHT:
      treegetsetSetState(h, "", IUP_EXPANDED);
      treeRepaint(h);
    break;

    case K_F2:
      {
        int x, y, text_x;

        if (!treeNodeCalcPos(h, &x, &y, &text_x))
          break;

        if (IupGetInt(h, "SHOWRENAME"))
          treeEditShow(h,text_x,x,y);
        else
  	      treecallRenameNodeCb(h);
      }
    break; 

    case K_cSP:
      if(iupStrEqual(IupGetAttribute(h,IUP_CTRL),IUP_YES))
      {
        tree_ctrl = 1;	    
        treegetsetSetValue(h,IUP_INVERT,1);
        treegetsetSetStarting(h, treegetsetGetValue(h));
        treeRepaint(h);
      }
    break; 

    case K_CR:
      if(iupStrEqual(IupGetAttribute(h,IUP_KIND),IUP_BRANCH))
      {
        if(iupStrEqual(IupGetAttribute(h,IUP_STATE),IUP_EXPANDED))
          treegetsetSetState(h, "", IUP_COLLAPSED);
        else
          treegetsetSetState(h, "", IUP_EXPANDED);

        treeRepaint(h);
      }
      else
        treecallExecuteLeafCb(h);
    break;

    default:
      return IUP_CONTINUE;
  }
  
  return IUP_IGNORE;
}
