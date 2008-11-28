/** \file
 * \brief iuptree control
 * Calls the user callbacks
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


void iTreeCallDragDropCB(Ihandle* ih, int drag_id, int drop_id, int isshift, int iscontrol)
{
  IFniiii cb = (IFniiii)IupGetCallback(ih, "DRAGDROP_CB");
  
  if(cb)
    cb(ih, drag_id, drop_id, isshift, iscontrol);
}

/* Calls the user callback when a node is selected/unselected.
   - node: node's users.
   - s   : node's status (1 = selected, 0 = deselected).
*/
int iTreeCallSelectionCB(Ihandle* ih, int node, int s)
{
  IFnii cb = (IFnii)IupGetCallback(ih, "SELECTION_CB");
  
  if(cb)
    return cb(ih, node, s);

  return IUP_DEFAULT;
}

/* Calls the user callback associated with the event of opening a folder. */
int iTreeCallBranchOpenCB(Ihandle* ih, Node n)
{
  IFni cb = (IFni)IupGetCallback(ih, "BRANCHOPEN_CB");

  if(cb && n->kind  == ITREE_BRANCH && 
           n->state == ITREE_COLLAPSED)
  {
    int id = iTreeFindNodeId(ih, n);
    return cb(ih, id);
  }

  return IUP_DEFAULT;
}

/* Calls the user callback associated with the event of closing a folder. */
int iTreeCallBranchCloseCB(Ihandle* ih, Node n)
{
  IFni cb = (IFni)IupGetCallback(ih, "BRANCHCLOSE_CB");

  if(cb && n->kind  == ITREE_BRANCH && 
           n->state == ITREE_EXPANDED)
  {
    int id = iTreeFindNodeId(ih, n);
    return cb(ih, id);
  }

  return IUP_DEFAULT;
}

/* Calls the user callback associated with the event of double clicking a leaf. */
void iTreeCallExecuteLeafCB(Ihandle* ih)
{
  IFni cb = (IFni)IupGetCallback(ih, "EXECUTELEAF_CB");

  if(cb)
  {
    int id;
    char* id_string = iTreeGSGetValue(ih);

    id = atoi(id_string);
    cb(ih, id);
  }
}

/* Calls the user callback associated with the event of renaming a node. */
void iTreeCallRenameNodeCB(Ihandle* ih)
{
  IFnis cb = (IFnis)IupGetCallback(ih, "RENAMENODE_CB");
  
  if(cb)
  {
    int id ;
    char* v;
    char* id_string = iTreeGSGetValue(ih);

    if(id_string == NULL)
      return;

    v = (char*) malloc(sizeof(char) * (5 + strlen(id_string)));
    strcpy(v, "NAME");
    strcat(v, id_string);

    id = atoi(id_string);
    cb(ih, id, IupGetAttribute(ih, v));
    free(v);
  }
}

/* This will update the node name: if the callback does not exists 
   or if it exists and it returns IUP_DEFAULT */
int iTreeCallRenameCB(Ihandle* ih, char* after)
{
  char* v;
  char* id_string;
  int   ret = IUP_DEFAULT;
  IFnis cb;

  id_string = iTreeGSGetValue(ih);
  if(id_string == NULL)
    return IUP_IGNORE;

  v = (char*) malloc(sizeof(char) * (5 + strlen(id_string)));
  strcpy(v, "NAME");
  strcat(v, id_string);

  cb = (IFnis)IupGetCallback(ih, "RENAME_CB");
  if(cb)
  {
    int id = atoi(id_string);
    ret = cb(ih,id, after);
  }

  if(ret == IUP_DEFAULT)
  {
    iTreeGSSetName(ih, id_string, after);
    IupSetAttribute(ih, "REDRAW", "YES");
  }

  free(v);
  return ret;
}

int iTreeCallShowRenameCB(Ihandle* ih)
{
  IFni cb = (IFni)IupGetCallback(ih, "SHOWRENAME_CB");

  if(cb)
  {
    int id ;
    char* id_string = iTreeGSGetValue(ih);

    if(id_string == NULL)
      return IUP_IGNORE;

    id = atoi(id_string);

    if(cb(ih, id) == IUP_IGNORE)
      return IUP_IGNORE;
  }

  return IUP_DEFAULT;
}

/* Calls the user callback associated to the right click event.
   - id : node's id
   - r  : vector containing mouse button info
*/
void iTreeCallRightClickCB(Ihandle* ih, int id, char* r)
{
  IFnis cb = (IFnis)IupGetCallback(ih, "RIGHTCLICK_CB");
  
  if(cb)
  {
    char* id_string = iTreeGSGetValue(ih);
    
    if(id == -1)
      id = atoi(id_string);

    cb(ih, id, r);
  }
}

/* Calls the user callback associated when a key is pressed,
   being the focus with the tree.
   - c : character pressed.
*/
int iTreeCallKeyPressCB(Ihandle* ih, int c, int press)
{
  int ret = IUP_DEFAULT;
  if(press)
  {
    ret = iTreeKey(ih, c);
  }
  return ret;
}
