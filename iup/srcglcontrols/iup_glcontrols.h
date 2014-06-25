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
Iclass* iupGLButtonNewClass(void);
Iclass* iupGLToggleNewClass(void);
Iclass* iupGLLinkNewClass(void);
Iclass* iupGLProgressBarNewClass(void);
Iclass* iupGLValNewClass(void);
Iclass* iupGLFrameNewClass(void);
Iclass* iupGLExpanderNewClass(void);

int  iupGLSubCanvasSetTransform(Ihandle* ih, Ihandle* gl_parent);
void iupGLSubCanvasSaveState(Ihandle* gl_parent);
void iupGLSubCanvasRestoreState(Ihandle* gl_parent);
void iupGLSubCanvasRedrawFront(Ihandle* ih);
void iupGLSubCanvasUpdateSizeFromFont(Ihandle* ih);
int  iupGLSubCanvasRedraw(Ihandle* ih);

void iupGLImageGetInfo(const char* name, int *w, int *h, int *bpp);
unsigned char* iupGLImageGetData(Ihandle* ih, int active);
void iupGLColorMakeInactive(unsigned char *r, unsigned char *g, unsigned char *b);

int iupGLFontGetStringWidth(Ihandle* ih, const char* str, int len);
void iupGLFontGetMultiLineStringSize(Ihandle* ih, const char* str, int *w, int *h);
int iupGLSetStandardFontAttrib(Ihandle* ih, const char* value);
void iupGLFontGetCharSize(Ihandle* ih, int *charwidth, int *charheight);
void iupGLFontRenderString(Ihandle* ih, const char* str, int len);
void iupGLFontInit(void);
void iupGLFontFinish(void);
void iupGLFontRelease(Ihandle* gl_parent);
void iupGLFontGetDim(Ihandle* ih, int *maxwidth, int *height, int *ascent, int *descent);

void iupGLDrawText(Ihandle* ih, int x, int y, const char* str, const char* color, int active);
void iupGLDrawImage(Ihandle* ih, int x, int y, const char* name, int active);
void iupGLDrawLine(Ihandle* ih, int x1, int y1, int x2, int y2, float linewidth, const char* color, int active);
void iupGLDrawPolygon(Ihandle* ih, const int* points, int count, const char* color, int active);
void iupGLDrawPolyline(Ihandle* ih, const int* points, int count, float linewidth, const char* color, int active);
void iupGLDrawRect(Ihandle* ih, int xmin, int xmax, int ymin, int ymax, float linewidth, const char* color, int active, int round);
void iupGLDrawBox(Ihandle* ih, int xmin, int xmax, int ymin, int ymax, const char* color, int active);
void iupGLDrawFrameRect(Ihandle* ih, int xmin, int xmax, int ymin, int ymax, float linewidth, const char* color, int active, int title_x, int title_width, int title_height);
enum { IUPGL_ARROW_LEFT, IUPGL_ARROW_RIGHT, IUPGL_ARROW_TOP, IUPGL_ARROW_BOTTOM };
void iupGLDrawArrow(Ihandle *ih, int x, int y, const char* color, int active, int dir, int size, int space);

void iupGLIconRegisterAttrib(Iclass* ic);
void iupGLIconDraw(Ihandle* ih, int icon_x, int icon_y, int icon_width, int icon_height, const char *image, const char* title, const char* fgcolor, int active);
void iupGLIconGetSize(Ihandle* ih, const char* image, const char* title, int *w, int *h);
Ihandle* iupGLIconGetImageHandle(Ihandle* ih, const char* name, int active);



#ifdef __cplusplus
}
#endif

#endif
