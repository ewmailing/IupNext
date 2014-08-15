/** \file
 * \brief Windows Driver ICON management.
 *
 * See Copyright Notice in iup.h
 *  */
 
#ifndef __WINICON_H 
#define __WINICON_H

#ifdef __cplusplus
extern "C" {
#endif

HICON iupwinGetIcon(char *v, HWND hwnd, char *type);
HBITMAP iupwinGetBitmap(Ihandle *image, Ihandle* parent);

#ifdef __cplusplus
}
#endif

#endif
