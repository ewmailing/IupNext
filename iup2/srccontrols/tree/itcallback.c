/** \file
 * \brief iuptree control
 * Calls the user callbacks
 *
 * See Copyright Notice in iup.h
 * $Id: itcallback.c,v 1.1 2008-10-17 06:20:41 scuri Exp $
 */

#include <stdlib.h>
#include <string.h>

#include "iup.h"
#include "iupcbs.h"
#include "iuptree.h"
#include "treedef.h"
#include "itcallback.h"
#include "itkey.h"
#include "itfind.h"
#include "itgetset.h"

void treecallDragDropCb(Ihandle* h, int drag_id, int drop_id, int isshift, int iscontrol)
{
  IFniiii cb = (IFniiii)IupGetCallback(h,"DRAGDROP_CB");
  if (cb) cb(h, drag_id, drop_id, isshift, iscontrol);
}

/*
%F Calls the user callback when a node is selected/unselected.
%i h     : tree's handle.
%i u     : node's useris.
%i s     : node's status (1=selected, 0=deselected).
%o returns IUP_DEFAULT 
*/
int treecallSelectionCb(Ihandle* h, int node, int s)
{
  IFnii cb = (IFnii)IupGetCallback(h,IUP_SELECTION_CB);
  if (cb)
    return cb(h, node, s);

  return IUP_DEFAULT;
}

/*
%F Calls the user callback associated with the event of opening a folder
%i h     : tree's handle.
%o returns IUP_DEFAULT to open the branch or not defined
           IUP_IGNORE to keep the branch closed
*/
int treecallBranchOpenCb(Ihandle* h, Node n )
{
  IFni cb = (IFni)IupGetCallback(h,IUP_BRANCHOPEN_CB);
  if (cb && node_kind(n) == BRANCH && 
            node_state(n) == COLLAPSED)
  {
    int id = treefindNodeId(h, n);
    return cb(h,id);
  }

  return IUP_DEFAULT;
}

/*
%F Calls the user callback associated with the event of closing a folder
%i h     : tree's handle.
%o returns IUP_DEFAULT to close the branch or not defined
           IUP_IGNORE to keep the branch opened
*/
int treecallBranchCloseCb(Ihandle* h, Node n)
{
  IFni cb = (IFni)IupGetCallback(h,IUP_BRANCHCLOSE_CB);
  if (cb && node_kind(n) == BRANCH && 
            node_state(n) == EXPANDED)
  {
    int id = treefindNodeId(h, n);
    return cb(h,id);
  }

  return IUP_DEFAULT;
}

/*
%F Calls the user callback associated with the event of double clicking a leaf
%i h     : tree's handle.
*/
void treecallExecuteLeafCb(Ihandle* h)
{
  IFni cb = (IFni)IupGetCallback(h,IUP_EXECUTELEAF_CB);
  if (cb)
  {
    char *id_string = treegetsetGetValue(h);
    int id ;

    id = atoi(id_string);
    cb(h,id);
  }
}

/*
%F Calls the user callback associated with the event of renaming a node
%i h     : tree's handle.
%o returns IUP_DEFAULT if the callback is defined
           IUP_IGNORE if it is not definied
*/
typedef int (*Irenamecb) (Ihandle*, int, char*);
void treecallRenameNodeCb(Ihandle* h)
{
  Irenamecb cb = (Irenamecb)IupGetCallback(h,IUP_RENAMENODE_CB);
  if (cb)
  {
    char *id_string = treegetsetGetValue(h);
    int id ;
    char *v;

    if(id_string == NULL)
      return;

    v = (char*) malloc(sizeof(char) * (5 + strlen(id_string)));
    strcpy(v, "NAME");
    strcat(v, id_string);

    id = atoi(id_string);
    cb(h,id,IupGetAttribute(h, v));
    free(v);
  }
}

/* this will update the node name 
   if the callback does not exists 
   or if it exists and it returns IUP_DEFAULT */
int treecallRenameCb(Ihandle* h, char* after)
{
  char *id_string;
  int ret = IUP_DEFAULT;
  Irenamecb cb;
  char *v;

  id_string = treegetsetGetValue(h);
  if (id_string == NULL)
    return IUP_IGNORE;

  v = (char*) malloc(sizeof(char) * (5 + strlen(id_string)));
  strcpy(v, "NAME");
  strcat(v, id_string);

  cb = (Irenamecb)IupGetCallback(h,"RENAME_CB");
  if (cb)
  {
    int id = atoi(id_string);
    ret = cb(h,id, after);
  }

  if (ret == IUP_DEFAULT)
  {
    treegetsetSetName(h, id_string, after);
    IupSetAttribute(h, "REDRAW", "YES");
  }

  free(v);
  return ret;
}

int treecallShowRenameCb(Ihandle* h)
{
  IFni cb = (IFni)IupGetCallback(h,"SHOWRENAME_CB");
  if (cb)
  {
    char *id_string = treegetsetGetValue(h);
    int id ;

    if(id_string == NULL)
      return IUP_IGNORE;

    id = atoi(id_string);

    if (cb(h,id) == IUP_IGNORE)
      return IUP_IGNORE;
  }

  return IUP_DEFAULT;
}

/*

%F Calls the user callback associated to the right click event.
%i h : tree's handle
id   : node's id
r    : vector containing mouse button info
%o returns IUP_DEFAULT if the callback is defined
           IUP_IGNORE if it is not definied
*/
void treecallRightClickCb(Ihandle* h, int id, char* r)
{
  IFnis cb = (IFnis)IupGetCallback(h,IUP_RIGHTCLICK_CB);
  if (cb)
  {
    char *id_string = treegetsetGetValue(h);
    if(id == -1)id = atoi(id_string);

    cb(h,id,r);
  }
}

/*
 *
 * %F Callback chamada quando uma tecla e pressionada, estando o foco com a
 * tree. Chama a callback do usuario.
 * %i h : Handle da tree,
 *    c : caracter digitado.
 * %o Retorno depende do retornao da callback do usuario
 *
 */
int treecallKeypressCb(Ihandle *h, int c, int press)
{
  int ret = IUP_DEFAULT;
  if (press)
  {
    ret = treeKey(h,c);
  }
  return ret;
}
