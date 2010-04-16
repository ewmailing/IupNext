/** \file
 * \brief Simple Draw API.
 *
 * See Copyright Notice in "iup.h"
 */
 
#ifndef __IUP_DRAW_H 
#define __IUP_DRAW_H

#ifdef __cplusplus
extern "C"
{
#endif

/** \defgroup draw Simple Draw API
 * \par
 * See \ref iup_draw.h
 * \ingroup util */



struct _IdrawCanvas;
typedef struct _IdrawCanvas IdrawCanvas;


/** Creates a draw canvas based on an IupCanvas.
 * This will create an image for offscreen drawing.
 * \ingroup draw */
IdrawCanvas* iupDrawCreateCanvas(Ihandle* ih);

/** Destroys the IdrawCanvas.
 * \ingroup draw */
void iupDrawKillCanvas(IdrawCanvas* dc);

/** Draws the ofscreen image on the screen.
 * \ingroup draw */
void iupDrawFlush(IdrawCanvas* dc);

/** Returns the canvas size available for drawing.
 * \ingroup draw */
void iupDrawGetSize(IdrawCanvas* dc, int *w, int *h);

/** Draws the parent background.
 * \ingroup draw */
void iupDrawParentBackground(IdrawCanvas* dc);

/** Draws a filled rectangle.
 * \ingroup draw */
void iupDrawRectangle(IdrawCanvas* dc, int x1, int y1, int x2, int y2, unsigned char r, unsigned char g, unsigned char b, int filled);

/*
TO DO:

Update
void iupDrawSetClipRect(IdrawCanvas* dc, int x1, int y1, int x2, int y2);
void iupDrawResetClip(IdrawCanvas* dc);
void iupDrawText(IdrawCanvas* dc, const char* text, int x, int y, unsigned char r, unsigned char g, unsigned char b);
void iupDrawImage(IdrawCanvas* dc, Ihandle* image, int x, int y);
void iupDrawLine(IdrawCanvas* dc, int x1, int y1, int x2, int y2, unsigned char r, unsigned char g, unsigned char b);
- polygon
- arc

*/


#ifdef __cplusplus
}
#endif

#endif

