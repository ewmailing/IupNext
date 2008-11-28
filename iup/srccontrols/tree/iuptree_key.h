/** \file
 * \brief iuptree control
 * Functions used to handle the keyboard.
 *
 * See Copyright Notice in iup.h
 */
 
#ifndef __ITKEY_H 
#define __ITKEY_H

#ifdef __cplusplus
extern "C" {
#endif

int iTreeKeyNodeCalcPos(Ihandle* ih, int* x, int* y, int* text_x);
int iTreeKey(Ihandle* ih, int c); 

#ifdef __cplusplus
}
#endif

#endif
