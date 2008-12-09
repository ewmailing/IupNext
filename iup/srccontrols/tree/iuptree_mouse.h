/** \file
 * \brief iuptree control
 * Functions used to handle the mouse.
 *
 * See Copyright Notice in "iup.h"
 */
 
#ifndef __IUPTREE_MOUSE_H 
#define __IUPTREE_MOUSE_H

#ifdef __cplusplus
extern "C" {
#endif

int iupTreeMouseMotionCB(Ihandle* ih, int x, int y, char* r);
int iupTreeMouseButtonCB(Ihandle* ih, int b, int press, int x, int y, char* r);
int iupTreeMouseCollapseButton(Ihandle* ih, int xmouse, int ymouse, int x, int y);
int iupTreeMouseExpandButton(Ihandle* ih, int xmouse, int ymouse, int x, int y);
int iupTreeMouseCollapsedBranch(Ihandle* ih, int xmouse, int ymouse, int x, int y);
int iupTreeMouseLeaf(Ihandle* ih, int xmouse, int ymouse, int x, int y);
int iupTreeMouseText(Ihandle* ih, ItreeNodePtr node, int text_x, int xmouse, int ymouse, int x, int y);

#ifdef __cplusplus
}
#endif

#endif
