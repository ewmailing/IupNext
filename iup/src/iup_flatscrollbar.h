/** \file
 * \brief flat scrollbar utilities
 *
 * See Copyright Notice in "iup.h"
 */
 
#ifndef __IUP_FLATSCROLLBAR_H 
#define __IUP_FLATSCROLLBAR_H

#ifdef __cplusplus
extern "C" {
#endif


void iupFlatScrollBarCreate(Ihandle* ih);
void iupFlatScrollBarRegister(Iclass* ic);

void iupFlatScrollBarSetChildrenCurrentSize(Ihandle* ih, int shrink);
void iupFlatScrollBarSetChildrenPosition(Ihandle* ih);

void iupFlatScrollBarWheelUpdate(Ihandle* ih, float delta);
void iupFlatScrollBarSetPos(Ihandle *ih, int posx, int posy);


#ifdef __cplusplus
}
#endif

#endif
