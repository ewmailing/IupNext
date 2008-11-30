/** \file
 * \brief iuptree control
 * Calls the user callbacks.
 *
 * See Copyright Notice in iup.h
 */
 
#ifndef __IUPTREE_CALLBACK_H 
#define __IUPTREE_CALLBACK_H

#ifdef __cplusplus
extern "C" {
#endif

void iupTreeCallbackDragDropCB   (Ihandle* ih, int drag_id, int drop_id, int isshift, int iscontrol);
int  iupTreeCallbackBranchOpenCB (Ihandle* ih, ItreeNodePtr n);
int  iupTreeCallbackBranchCloseCB(Ihandle* ih, ItreeNodePtr n);
void iupTreeCallbackExecuteLeafCB(Ihandle* ih);
void iupTreeCallbackRenameNodeCB (Ihandle* ih);
int  iupTreeCallbackShowRenameCB (Ihandle* ih);
int  iupTreeCallbackRenameCB     (Ihandle* ih, char* after);
int  iupTreeCallbackKeyPressCB   (Ihandle* ih, int c, int press);
void iupTreeCallbackRightClickCB (Ihandle* ih, int id, char* r);
int  iupTreeCallbackSelectionCB  (Ihandle* ih, int node, int s);

#ifdef __cplusplus
}
#endif

#endif
