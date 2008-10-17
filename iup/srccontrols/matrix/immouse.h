/** \file
 * \brief iMatrixrix control
 * mouse events.
 *
 * See Copyright Notice in iup.h
 * $Id: immouse.h,v 1.1 2008-10-17 06:05:36 scuri Exp $
 */
 
#ifndef __IMMOUSE_H 
#define __IMMOUSE_H

#ifdef __cplusplus
extern "C" {
#endif

int iMatrixMouseButtonCB (Ihandle* ih, int b, int press, int x, int y, char* r);
int iMatrixMouseMoveCB   (Ihandle* ih, int x, int y);

#ifdef __cplusplus
}
#endif

#endif
