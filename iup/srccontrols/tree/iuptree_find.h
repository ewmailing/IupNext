/** \file
 * \brief iuptree control
 * Functions used to find nodes in the tree.
 *
 * See Copyright Notice in "iup.h"
 */
 
#ifndef __IUPTREE_FIND_H 
#define __IUPTREE_FIND_H

#ifdef __cplusplus
extern "C" {
#endif

int   iupTreeFindNodeIdFromXY(Ihandle* ih, int xmouse, int ymouse);
void  iupTreeFindStartEndSelection(Ihandle* ih, int* begin, int* end);
ItreeNodePtr  iupTreeFindNodeFromId(Ihandle* ih, int id);
ItreeNodePtr  iupTreeFindNodeFromString(Ihandle* ih, const char* id_string);
ItreeNodePtr  iupTreeFindMarked(Ihandle* ih, const char* id_string);
ItreeNodePtr  iupTreeFindPrevious(ItreeNodePtr temp, ItreeNodePtr node);
ItreeNodePtr  iupTreeFindParent(Ihandle* ih, const char* id_string);
int   iupTreeFindMaxDepth(ItreeNodePtr node);
int   iupTreeFindNumNodes(Ihandle* ih);
float iupTreeFindNumNodesInCanvas(Ihandle* ih);
int   iupTreeFindNodeId(Ihandle* ih, ItreeNodePtr node);
int   iupTreeFindTotNumNodes(Ihandle* ih);

#ifdef __cplusplus
}
#endif

#endif
