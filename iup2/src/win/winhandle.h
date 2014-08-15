/** \file
 * \brief HWND to ihandle table
 *
 * See Copyright Notice in iup.h
 *  */
 
#ifndef __WINHANDLE_H 
#define __WINHANDLE_H

#ifdef __cplusplus
extern "C" {
#endif

Ihandle* iupwinHandleGet(HWND hwnd);
void iupwinHandleAdd(HWND hwnd, Ihandle *n);
void iupwinHandleRemove(Ihandle *n);
void iupwinHandleInit(void);
void iupwinHandleFinish(void);

#ifdef __cplusplus
}
#endif

#endif
