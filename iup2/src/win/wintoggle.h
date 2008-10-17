/** \file
 * \brief Windows Driver Toggle management.
 *
 * See Copyright Notice in iup.h
 * $Id: wintoggle.h,v 1.1 2008-10-17 06:19:26 scuri Exp $
 */
 
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
