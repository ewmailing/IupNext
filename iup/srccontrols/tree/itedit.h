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

void iTreeEditShow(Ihandle* ih, int text_x, int x, int y);
void iTreeEditCheckHidden(Ihandle* ih);
void iTreeEditCreate(Ihandle* ih);

#ifdef __cplusplus
}
#endif

#endif
