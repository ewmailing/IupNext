/** \file
 * \brief IUP Driver
 *
 * See Copyright Notice in iup.h
 */

#include <stdlib.h>
#include <stdio.h>

#include "iup.h"

#include <cd.h>
#include <cd_private.h>
#include <cdiup.h>
#include <cdnative.h>


static void (*cdcreatecanvasNATIVE)(cdCanvas* canvas, void* data) = NULL;

static void cdcreatecanvasIUP(cdCanvas* canvas, Ihandle *iupcanvas)
{
#ifdef WIN32
  char* data = IupGetAttribute(iupcanvas, "HWND");  /* new IUP 3 attribute, works for Windows and GTK */
  if (!data)
    data = IupGetAttribute(iupcanvas, "WID"); /* OLD IUP 2 attribute */
  if (!data)
    return;
#else
  char data[50];
  void *dpy = IupGetAttribute(iupcanvas, "XDISPLAY");   /* works for Motif and GTK */
  unsigned long wnd = (unsigned long)IupGetAttribute(iupcanvas, "XWINDOW");
  if (!wnd || !dpy)
    return;
#ifdef SunOS_OLD
  sprintf(data, "%d %lu", (int)dpy, wnd); 
#else
  sprintf(data, "%p %lu", dpy, wnd); 
#endif
#endif
  
  /* Inicializa driver NativeWindow */
  cdcreatecanvasNATIVE(canvas, data);

  IupSetAttribute(iupcanvas, "_CD_CANVAS", (char*)canvas);
}

static cdContext cdIupContext;

cdContext* cdContextIup(void)
{
  /* call cdContextNativeWindow every time, because of ContextPlus */
  cdContext* ctx = cdContextNativeWindow();

  cdcreatecanvasNATIVE = ctx->cxCreateCanvas;

  cdIupContext = *ctx;
  cdIupContext.cxCreateCanvas = cdcreatecanvasIUP;

  return &cdIupContext;
}
