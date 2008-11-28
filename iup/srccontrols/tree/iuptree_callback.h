/** \file
 * \brief iuptree control
 * Calls the user callbacks.
 *
 * See Copyright Notice in iup.h
 */
 
#ifndef __ITCALLBACK_H 
#define __ITCALLBACK_H

#ifdef __cplusplus
extern "C" {
#endif

void iTreeCallDragDropCB(Ihandle* ih, int drag_id, int drop_id, int isshift, int iscontrol);
int  iTreeCallBranchOpenCB(Ihandle* ih, Node n);
int  iTreeCallBranchCloseCB(Ihandle* ih, Node n);
void iTreeCallExecuteLeafCB(Ihandle* ih);
void iTreeCallRenameNodeCB(Ihandle* ih);
int  iTreeCallShowRenameCB(Ihandle* ih);
int  iTreeCallRenameCB(Ihandle* ih, char* after);
int  iTreeCallKeyPressCB(Ihandle* ih, int c, int press);
void iTreeCallRightClickCB(Ihandle* ih, int id, char* r);
int  iTreeCallSelectionCB(Ihandle* ih, int node, int s);

#ifdef __cplusplus
}
#endif

#endif
