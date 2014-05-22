/** \file
 * \brief GL Controls Class Initialization functions.
 *
 * See Copyright Notice in "iup.h"
 */
 
#ifndef __IUP_GLCONTROLS_H 
#define __IUP_GLCONTROLS_H


#ifdef __cplusplus
extern "C" {
#endif


Iclass* iupGLCanvasBoxNewClass(void);
Iclass* iupGLSubCanvasNewClass(void);
Iclass* iupGLLabelNewClass(void);
Iclass* iupGLSeparatorNewClass(void);

void iupGLSubCanvasSetTransform(Ihandle* ih, Ihandle* gl_parent);
void iupGLSubCanvasSaveState(void);
void iupGLSubCanvasRestoreState(Ihandle* gl_parent);

void iupGLSubCanvasRedrawFront(Ihandle* ih);
void iupGLSubCanvasUpdateSizeFromFont(Ihandle* ih);

void iupGLImageGetInfo(const char* name, int *w, int *h, int *bpp);
unsigned char* iupGLImageGetData(Ihandle* ih, int active);
void iupGLColorMakeInactive(unsigned char *r, unsigned char *g, unsigned char *b);

void iupGLFontGetMultiLineStringSize(Ihandle* ih, const char* str, int *w, int *h);
int iupGLSetStandardFontAttrib(Ihandle* ih, const char* value);
void iupGLFontGetCharSize(Ihandle* ih, int *charwidth, int *charheight);
void iupGLFontRenderString(Ihandle* ih, const char* str, int len);
void iupGLFontInit(void);
void iupGLFontFinish(void);
void iupGLFontRelease(Ihandle* gl_parent);
void iupGLFontGetDim(Ihandle* ih, int *maxwidth, int *height, int *ascent, int *descent);

void iupGLDrawRect(double xmin, double xmax, double ymin, double ymax);
void iupGLDrawText(Ihandle* ih, double x, double y, const char* str, const char* color, int active);
void iupGLDrawImage(Ihandle* ih, double x, double y, const char* name, int active);

void iupGLIconRegisterAttrib(Iclass* ic);
void iupGLIconDraw(Ihandle* ih, int icon_width, int icon_height, const char *image, const char* title, const char* fgcolor, int active);
void iupGLIconGetNaturalSize(Ihandle* ih, const char* image, const char* title, int *w, int *h);
Ihandle* iupGLIconGetImageHandle(Ihandle* ih, const char* name, int active);



#ifdef __cplusplus
}
#endif

#endif
