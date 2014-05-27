/** \file
 * \brief GL Controls.
 *
 * See Copyright Notice in "iup.h"
 */
 
#ifndef __IUPGLCONTROLS_H 
#define __IUPGLCONTROLS_H

#ifdef __cplusplus
extern "C" {
#endif


int  IupGLControlsOpen(void);

Ihandle* IupGLCanvasBoxv(Ihandle** children);
Ihandle* IupGLCanvasBox(Ihandle* child, ...);

Ihandle* IupGLSubCanvas(void);

Ihandle* IupGLLabel(const char* title);
Ihandle* IupGLSeparator(void);
Ihandle* IupGLButton(const char* title);
Ihandle* IupGLToggle(const char* title);


#ifdef __cplusplus
}
#endif

#endif
