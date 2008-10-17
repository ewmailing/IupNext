/** \file
 * \brief Image Resource Private Declarations
 *
 * See Copyright Notice in iup.h
 */
 
#ifndef __IUP_IMAGE_H 
#define __IUP_IMAGE_H

#ifdef __cplusplus
extern "C" {
#endif

void* iupdrvImageCreateIcon(Ihandle *ih);
void* iupdrvImageCreateCursor(Ihandle *ih);
void* iupdrvImageCreateImage(Ihandle *ih, const char* bgcolor, int make_inactive);

enum {IUPIMAGE_IMAGE, IUPIMAGE_ICON, IUPIMAGE_CURSOR};
void* iupdrvImageLoad(const char* name, int type);
void  iupdrvImageDestroy(void* image, int type);
void iupdrvImageGetInfo(void* image, int *w, int *h, int *bpp);  /* only for IUPIMAGE_IMAGE */

void* iupImageGetIcon(const char* name);
void* iupImageGetCursor(const char* name);
void* iupImageGetImage(const char* name, Ihandle* parent, int make_inactive, const char* attrib_name);
void iupImageGetInfo(const char* name, int *w, int *h, int *bpp);
void iupImageUpdateParent(Ihandle *parent);

typedef struct _iupColor { 
  unsigned char r, g, b, a; 
} iupColor;

int iupImageInitColorTable(Ihandle *ih, iupColor* colors, int *colors_count);
void iupImageColorMakeInactive(unsigned char *r, unsigned char *g, unsigned char *b, 
                               unsigned char bg_r, unsigned char bg_g, unsigned char bg_b);

#define iupALPHABLEND(_src,_dst,_alpha) (unsigned char)(((_src) * (_alpha) + (_dst) * (255 - (_alpha))) / 255)


void iupImageStockInit(void);
void iupImageStockFinish(void);
typedef Ihandle* (*iupImageStockFunc)(void);
void iupImageStockSet(const char *name, iupImageStockFunc func, const char* native_name);


#ifdef __cplusplus
}
#endif

#endif
