/** \file
 * \brief Windows Driver Toggle management.
 *
 * See Copyright Notice in iup.h
 *  */
 
#ifndef __WINTOGGLE_H 
#define __WINTOGGLE_H

#ifdef __cplusplus
extern "C" {
#endif

LRESULT CALLBACK iupwinToggleProc (HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

#ifdef __cplusplus
}
#endif

#endif
