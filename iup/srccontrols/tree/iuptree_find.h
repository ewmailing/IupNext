/** \file
 * \brief iuptree control
 * Functions used to find nodes in the tree.
 *
 * See Copyright Notice in iup.h
 */
 
#ifndef __IUPTREE_FIND_H 
#define __IUPTREE_FIND_H

#ifdef __cplusplus
extern "C" {
#endif

int   iTreeFindNodeIdFromXY(Ihandle* ih, int xmouse, int ymouse);
void  iTreeFindStartEndSelection(Ihandle* ih, int* begin, int* end);
Node  iTreeFindNodeFromId(Ihandle* ih, int id);
Node  iTreeFindNodeFromString(Ihandle* ih, const char* id_string);
Node  iTreeFindMarked(Ihandle* ih, const char* id_string);
Node  iTreeFindPrevious(Node temp, Node node);
Node  iTreeFindParent(Ihandle* ih, const char* id_string);
int   iTreeFindMaxDepth(Node node);
int   iTreeFindNumNodes(Ihandle* ih);
float iTreeFindNumNodesInCanvas(Ihandle* ih);
int   iTreeFindNodeId(Ihandle* ih, Node node);
int   iTreeFindTotNumNodes(Ihandle* ih);

#ifdef __cplusplus
}
#endif

#endif
