/** \file
 * \brief Canvas Draw API
 *
 * See Copyright Notice in "iup.h"
 */
 
#ifndef __IUPDRAW_H 
#define __IUPDRAW_H

#ifdef __cplusplus
extern "C" {
#endif

/* all functions can be used only in IUP canvas and inside the ACTION callback */

void IupDrawBegin(Ihandle* ih);
void IupDrawEnd(Ihandle* ih);

/* all primitives can be called only between calls to Begin and End */

void IupDrawParentBackground(Ihandle* ih);
void IupDrawLine(Ihandle* ih, int x1, int y1, int x2, int y2);
void IupDrawRectangle(Ihandle* ih, int x1, int y1, int x2, int y2);
void IupDrawArc(Ihandle* ih, int x1, int y1, int x2, int y2, double a1, double a2);
void IupDrawPolygon(Ihandle* ih, int* points, int count);
void IupDrawText(Ihandle* ih, const char* text, int len, int x, int y);
void IupDrawGetTextSize(Ihandle* ih, const char* str, int *w, int *h);
void IupDrawImage(Ihandle* ih, const char* name, int make_inactive, int x, int y);
void IupDrawGetImageInfo(const char* name, int *w, int *h, int *bpp);
void IupDrawSetClipRect(Ihandle* ih, int x1, int y1, int x2, int y2);
void IupDrawResetClip(Ihandle* ih);
void IupDrawSelectRect(Ihandle* ih, int x, int y, int w, int h);
void IupDrawFocusRect(Ihandle* ih, int x, int y, int w, int h);


#ifdef __cplusplus
}
#endif

#endif
