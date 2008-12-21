/** \file
 * \brief iuptree control
 * Functions used to handle the keyboard
 *
 * See Copyright Notice in "iup.h"
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "iup.h"
#include "iupcbs.h"
#include "iupcontrols.h"
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
#include "iuptree_callback.h"
#include "iuptree_key.h"
#include "iuptree_find.h"
#include "iuptree_getset.h"
#include "iuptree_edit.h"

int tree_ctrl  = 0;
int tree_shift = 0;


int iupTreeKeyNodeCalcPos(Ihandle* ih, int* x, int* y, int* text_x)
{
  ItreeNodePtr node  = (ItreeNodePtr)ih->data->root;
  float posy = IupGetFloat(ih, "POSY");
  float dy   = IupGetFloat(ih, "DY");
  float posx = IupGetFloat(ih, "POSX");
  float dx   = IupGetFloat(ih, "DX");

  cdCanvasActivate(ih->data->cddbuffer);

  *y = (int)((1.0 + posy/dy)*(ih->data->YmaxC-ITREE_TREE_TOP_MARGIN));

  while(node != ih->data->selected)
  {
    if(node->visible == YES)
      *y -= ITREE_NODE_Y;

    node = node->next;
    if(node == NULL)
      return 0;
  }

  *y -= ITREE_NODE_Y;
  *x = (int)(ITREE_TREE_LEFT_MARGIN - (ih->data->XmaxC - ITREE_NODE_X) * posx / dx) + ITREE_NODE_X * node->depth;

  /* if node has a text associated to it... */
  *text_x = 0;
  if(node->name)
  {
    /* Calculates its dimensions */
    *text_x = iupdrvFontGetStringWidth(ih, node->name);
  }

  return 1;
}

int iupTreeKey(Ihandle* ih, int c)
{
  tree_ctrl = NO;
  tree_shift = NO;
  
  switch(c)
  {
    case K_sHOME:
      if(iupStrEqualNoCase(IupGetAttribute(ih, "SHIFT"), "NO"))
        break;
      tree_shift = YES;
    case K_cHOME:
      if(tree_shift == NO)
        tree_ctrl = YES;      
    case K_HOME:
      iupTreeGSSetValue(ih, "ROOT", 1);
      if(tree_shift == NO && tree_ctrl == NO)
        iupTreeGSSetStarting(ih, iupTreeGSGetValue(ih));
      iupTreeRepaint(ih);
    break;
    
    case K_sEND:
      if(iupStrEqualNoCase(IupGetAttribute(ih, "SHIFT"), "NO"))
        break ;
      tree_shift = YES;
    case K_cEND:
      if(tree_shift == NO)
      {  
        if(iupStrEqualNoCase(IupGetAttribute(ih, "CTRL"), "NO"))
          break ;
        tree_ctrl = YES;
      }
    case K_END:
      iupTreeGSSetValue(ih, "LAST", 1);
      if(tree_shift == NO && tree_ctrl == NO)
        iupTreeGSSetStarting(ih, iupTreeGSGetValue(ih));
      iupTreeRepaint(ih);
    break;

    case K_sPGUP: 
      if(iupStrEqualNoCase(IupGetAttribute(ih, "SHIFT"), "NO"))
        break ;
      tree_shift = YES;
    case K_cPGUP:
      if(tree_shift == NO)
      {
        if(iupStrEqualNoCase(IupGetAttribute(ih, "CTRL"), "NO"))
          break ;
        tree_ctrl = YES; 
      }
    case K_PGUP:
      iupTreeGSSetValue(ih, "PGUP", 1);
      if(tree_shift == NO && tree_ctrl == NO)
        iupTreeGSSetStarting(ih, iupTreeGSGetValue(ih));
      iupTreeRepaint(ih);
    break;
     
    case K_sPGDN:
      if(iupStrEqualNoCase(IupGetAttribute(ih, "SHIFT"), "NO"))
        break ;
      tree_shift = YES;
    case K_cPGDN:
      if(tree_shift == NO)
      {
        if(iupStrEqualNoCase(IupGetAttribute(ih, "CTRL"), "NO"))
          break ;
        tree_ctrl = YES;
      }
    case K_PGDN:
      iupTreeGSSetValue(ih, "PGDN", 1);
      if(tree_shift == NO && tree_ctrl == NO)
        iupTreeGSSetStarting(ih, iupTreeGSGetValue(ih));
      iupTreeRepaint(ih);
    break;
    
    case K_sUP:
      if(iupStrEqualNoCase(IupGetAttribute(ih, "SHIFT"), "NO"))
        break;
      tree_shift = YES;
    case K_cUP:
      if(tree_shift == NO)
      {
        if(iupStrEqualNoCase(IupGetAttribute(ih, "CTRL"), "NO"))
          break ;
        tree_ctrl = YES;
      }
    case K_UP:
      iupTreeGSSetValue(ih, "PREVIOUS", 1);
      if(tree_shift == NO && tree_ctrl == NO)
        iupTreeGSSetStarting(ih, iupTreeGSGetValue(ih));
      iupTreeRepaint(ih);
    break;

    case K_sDOWN:
      if(iupStrEqualNoCase(IupGetAttribute(ih, "SHIFT"), "NO"))
        break;
      tree_shift = YES;
    case K_cDOWN:
      if(tree_shift == NO)
      {
        if(iupStrEqualNoCase(IupGetAttribute(ih, "CTRL"), "NO"))
          break;
        tree_ctrl = YES;
      }
    case K_DOWN:
      iupTreeGSSetValue(ih, "NEXT", 1);
      if(tree_shift == NO && tree_ctrl == NO)
        iupTreeGSSetStarting(ih, iupTreeGSGetValue(ih));
      iupTreeRepaint(ih);
    break;

    case K_LEFT:
      iupTreeGSSetState(ih, "", "COLLAPSED");
      iupTreeRepaint(ih);
    break;

    case K_RIGHT:
      iupTreeGSSetState(ih, "", "EXPANDED");
      iupTreeRepaint(ih);
    break;

    case K_F2:
      {
        int x, y, text_x;

        if(!iupTreeKeyNodeCalcPos(ih, &x, &y, &text_x))
          break;

        if(IupGetInt(ih, "SHOWRENAME"))
          iupTreeEditShow(ih, text_x, x, y);
        else
          iupTreeCallbackRenameNodeCB(ih);
      }
    break; 

    case K_cSP:
      if(iupStrEqualNoCase(IupGetAttribute(ih, "CTRL"), "YES"))
      {
        tree_ctrl = 1;
        iupTreeGSSetValue(ih, "INVERT", 1);
        iupTreeGSSetStarting(ih, iupTreeGSGetValue(ih));
        iupTreeRepaint(ih);
      }
    break; 

    case K_CR:
      if(iupStrEqualNoCase(IupGetAttribute(ih, "KIND"), "BRANCH"))
      {
        if(iupStrEqualNoCase(IupGetAttribute(ih, "STATE"), "EXPANDED"))
          iupTreeGSSetState(ih, "", "COLLAPSED");
        else
          iupTreeGSSetState(ih, "", "EXPANDED");

        iupTreeRepaint(ih);
      }
      else
        iupTreeCallbackExecuteLeafCB(ih);
    break;

    default:
      return IUP_CONTINUE;
  }
  
  return IUP_IGNORE;  /* to avoid arrow keys being processed by the system */
}
