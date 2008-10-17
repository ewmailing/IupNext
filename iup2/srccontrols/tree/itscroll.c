/** \file
 * \brief iuptree control
 * Functions used to scroll the tree
 *
 * See Copyright Notice in iup.h
 * $Id: itscroll.c,v 1.1 2008-10-17 06:20:41 scuri Exp $
 */

#include <stdio.h>
#include <stdlib.h>

#include "iup.h"
#include "iuptree.h"
#include "treedef.h" 
#include "itscroll.h"
#include "itdraw.h"
#include "itfind.h"
#include "itgetset.h"

int treeScrollDown(Ihandle *h)
{
  TtreePtr tree = (TtreePtr)tree_data(h);
	
  if(tree_selected_y(tree) < NODE_Y+NODE_Y/2)
  {
    char posy[10];
    sprintf(posy,"%f",IupGetFloat(h,IUP_POSY) + IupGetFloat(h,IUP_DY)/treefindNumNodesInCanvas(h));
    IupStoreAttribute(tree_self(tree),IUP_POSY,posy);
  }

  return IUP_DEFAULT;
}

int treeScrollUp(Ihandle *h)
{
  TtreePtr tree = (TtreePtr)tree_data(h);
	
  if(tree_selected_y(tree) > YmaxCanvas(tree)-TREE_TOP_MARGIN - NODE_Y - NODE_Y)
  {
    char posy[10];
    sprintf(posy,"%f",IupGetFloat(h,IUP_POSY) - IupGetFloat(h,IUP_DY)/treefindNumNodesInCanvas(h));
    IupStoreAttribute(tree_self(tree),IUP_POSY,posy);
  }	  

  return IUP_DEFAULT;
}

int treeScrollPgUp(Ihandle *h)
{
  TtreePtr tree = (TtreePtr)tree_data(h);
  float temp = IupGetFloat(h,IUP_POSY) - IupGetFloat(h,IUP_DY);
  char posy[10];
  if(temp<0.0)temp = 0.0;
  sprintf(posy,"%f",temp);
  IupStoreAttribute(tree_self(tree),IUP_POSY,posy);
  return IUP_DEFAULT;
}

int treeScrollPgDn(Ihandle *h)
{
  TtreePtr tree = (TtreePtr)tree_data(h);
  double temp = IupGetFloat(h,IUP_POSY) + IupGetFloat(h,IUP_DY);
  char posy[10];
  if(temp > 1.0 - IupGetFloat(h,IUP_DY))temp = 1.0 - IupGetFloat(h,IUP_DY);
  sprintf(posy,"%f",temp);
  IupStoreAttribute(tree_self(tree),IUP_POSY,posy);
  return IUP_DEFAULT;
}

int treeScrollEnd(Ihandle *h)
{
  TtreePtr tree = (TtreePtr)tree_data(h);
  char buffer[10];
  sprintf(buffer,"%f", 1.0 - IupGetFloat(tree_self(tree),IUP_DY));
  IupStoreAttribute(tree_self(tree),IUP_POSY,buffer);
  return IUP_DEFAULT;
}

int treeScrollBegin(Ihandle *h)
{
  TtreePtr tree = (TtreePtr)tree_data(h);
  IupSetAttribute(tree_self(tree),IUP_POSY,"0.0");
  return IUP_DEFAULT;
}

int treeScrollShow(Ihandle *h)
{
  TtreePtr tree = (TtreePtr)tree_data(h);
    
  /* If node is above the canvas, scrolls down to make it visible */
  if(tree_selected_y(tree) > YmaxCanvas(tree) - TREE_TOP_MARGIN)
  {
    char posy[10];
    sprintf(posy,"%f",atof(treegetsetGetValue(h))/(float)treefindNumNodes(h));
    IupStoreAttribute(tree_self(tree),IUP_POSY,posy);
  }
  else
  /* If node is below the canvas, scrolls up to make it visible */
  if(tree_selected_y(tree)< 0 )
  {
    char posy[10];
    sprintf(posy,"%f",(atof(treegetsetGetValue(h))-(treefindNumNodesInCanvas(h)-1))/(float)treefindNumNodes(h));
    IupStoreAttribute(tree_self(tree),IUP_POSY,posy);
  }

  return IUP_DEFAULT;
}

