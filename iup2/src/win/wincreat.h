/** \file
 * \brief Windows Driver Controls creation.
 *
 * See Copyright Notice in iup.h
 * $Id: wincreat.h,v 1.1 2008-10-17 06:19:26 scuri Exp $
 */
 
#ifndef __WINCREAT_H 
#define __WINCREAT_H

#ifdef __cplusplus
extern "C" {
#endif

void  iupwinCreatInit(void);
void  iupwinSetHScrollInfo(Ihandle* n);
void  iupwinSetVScrollInfo(Ihandle* n);
char* iupwinMenuLabel( Ihandle* h, char* label );
HMENU iupwinCreatePopupMenu (Ihandle* n);
HMENU iupwinGetMenu (Ihandle* n);
void  iupwinCleanidIhandle(int id);
Ihandle* iupwinGetMenuIhandle(int id);
void iupwinShowLastError(void);

/* implements also iupdrvCreateNativeObject, iupdrvCreateObjects, iupdrvCreateObject                                       
*/

/* estruturas de dados utilizadas primariamente no modulo wincreat.c */

typedef struct IwinBitmap_
{
	void* bmpinfo;
	void* bitmap;        /* data bits */
  void* bitmask;       /* transparency mask, used only for cursors and icons */
} IwinBitmap;

HBITMAP iupwinCreateBitmap(Ihandle* hImage, HDC hDC, Ihandle* parent);

#ifdef __cplusplus
}
#endif

#endif
