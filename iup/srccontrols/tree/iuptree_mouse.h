/** \file
 * \brief iuptree control
 * Functions used to handle the mouse.
 *
 * See Copyright Notice in iup.h
 */
 
#ifndef __ITMOUSE_H 
#define __ITMOUSE_H

#ifdef __cplusplus
extern "C" {
#endif

int iTreeMouseMotionCB(Ihandle* ih, int x, int y, char* r);
int iTreeMouseButtonCB(Ihandle* ih, int b, int press, int x, int y, char* r);
int iTreeMouseCollapseButton(Ihandle* ih, int xmouse, int ymouse, int x, int y);
int iTreeMouseExpandButton(Ihandle* ih, int xmouse, int ymouse, int x, int y);
int iTreeMouseCollapsedBranch(Ihandle* ih, int xmouse, int ymouse, int x, int y);
int iTreeMouseLeaf(Ihandle* ih, int xmouse, int ymouse, int x, int y);
int iTreeMouseText(Ihandle* ih, Node node, int text_x, int xmouse, int ymouse, int x, int y);

#ifdef __cplusplus
}
#endif

#endif
