/** \file
 * \brief iuptree control
 * Functions used to scroll the tree.
 *
 * See Copyright Notice in iup.h
 */
 
#ifndef __IUPTREE_SCROLL_H 
#define __IUPTREE_SCROLL_H

#ifdef __cplusplus
extern "C" {
#endif

int iTreeScrollDown(Ihandle* ih);
int iTreeScrollUp(Ihandle* ih);
int iTreeScrollPgUp(Ihandle* ih);
int iTreeScrollPgDn(Ihandle* ih);
int iTreeScrollEnd(Ihandle* ih);
int iTreeScrollBegin(Ihandle* ih);
int iTreeScrollShow(Ihandle* ih);

#ifdef __cplusplus
}
#endif

#endif
