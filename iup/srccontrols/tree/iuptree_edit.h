/** \file
 * \brief iuptree control
 * Functions used to edit a node name in place.
 *
 * See Copyright Notice in iup.h
 */
 
#ifndef __IUPTREE_EDIT_H 
#define __IUPTREE_EDIT_H

#ifdef __cplusplus
extern "C" {
#endif

void iTreeEditShow(Ihandle* ih, int text_x, int x, int y);
void iTreeEditCheckHidden(Ihandle* ih);
void iTreeEditCreate(Ihandle* ih);

#ifdef __cplusplus
}
#endif

#endif
