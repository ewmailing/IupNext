/** \file
 * \brief iuptree control
 * Functions used to edit a node name in place.
 *
 * See Copyright Notice in iup.h
 */
 
#ifndef __ITEDIT_H 
#define __ITEDIT_H

#include "iuptree.h"

#ifdef __cplusplus
extern "C" {
#endif

void treeEditShow(Ihandle *h, int text_x, int x, int y);
void treeEditCheckHidden(Ihandle* h);
void treeEditCreate(TtreePtr tree);
void treeEditMap(TtreePtr tree, Ihandle* self);

#ifdef __cplusplus
}
#endif

#endif
