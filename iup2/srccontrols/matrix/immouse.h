/** \file
 * \brief iupmatrix control
 * mouse events.
 *
 * See Copyright Notice in iup.h
 *  */
 
#ifndef __IMMOUSE_H 
#define __IMMOUSE_H

#ifdef __cplusplus
extern "C" {
#endif

int iupmatMouseButtonCb (Ihandle *hm, int b, int press, int x, int y, char *r);
int iupmatMouseMoveCb   (Ihandle *hm, int x, int y);

#ifdef __cplusplus
}
#endif

#endif
