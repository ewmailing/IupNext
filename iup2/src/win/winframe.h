/** \file
 * \brief Windows Driver Frame redraw management.
 *
 * See Copyright Notice in iup.h
 *  */
 
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
