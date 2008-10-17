/** \file
 * \brief Windows Driver Frame redraw management.
 *
 * See Copyright Notice in iup.h
 * $Id: winframe.h,v 1.1 2008-10-17 06:19:26 scuri Exp $
 */
 
#ifndef __WINFRAME_H 
#define __WINFRAME_H

#ifdef __cplusplus
extern "C" {
#endif

LRESULT CALLBACK iupwinFrameProc (HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

#ifdef __cplusplus
}
#endif

#endif
