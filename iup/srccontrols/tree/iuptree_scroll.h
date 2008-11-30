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

int iupTreeScrollDown (Ihandle* ih);
int iupTreeScrollUp   (Ihandle* ih);
int iupTreeScrollPgUp (Ihandle* ih);
int iupTreeScrollPgDn (Ihandle* ih);
int iupTreeScrollEnd  (Ihandle* ih);
int iupTreeScrollBegin(Ihandle* ih);
int iupTreeScrollShow (Ihandle* ih);

#ifdef __cplusplus
}
#endif

#endif
