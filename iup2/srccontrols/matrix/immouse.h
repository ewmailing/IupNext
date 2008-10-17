/** \file
 * \brief iupmatrix control
 * mouse events.
 *
 * See Copyright Notice in iup.h
 * $Id: immouse.h,v 1.1 2008-10-17 06:20:15 scuri Exp $
 */
 
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
