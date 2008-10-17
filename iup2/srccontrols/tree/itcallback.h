/** \file
 * \brief iuptree control
 * Calls the user callbacks.
 *
 * See Copyright Notice in iup.h
 * $Id: itcallback.h,v 1.1 2008-10-17 06:20:41 scuri Exp $
 */
 
#ifndef __ITCALLBACK_H 
#define __ITCALLBACK_H

#include <iup.h>

#ifdef __cplusplus
extern "C" {
#endif

void treecallDragDropCb(Ihandle* h, int drag_id, int drop_id, int isshift, int iscontrol);
int treecallBranchOpenCb(Ihandle* h, Node n);
int treecallBranchCloseCb(Ihandle* h, Node n);
void treecallExecuteLeafCb(Ihandle* h);
void treecallRenameNodeCb(Ihandle* h);
int treecallShowRenameCb(Ihandle* h);
int treecallRenameCb(Ihandle* h, char* after);
int treecallKeypressCb(Ihandle* h, int c, int press);
void treecallRightClickCb(Ihandle* h, int id, char* r);
int treecallSelectionCb(Ihandle* h, int node, int s);

#ifdef __cplusplus
}
#endif

#endif
