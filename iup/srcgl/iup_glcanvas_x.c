/** \file
 * \brief iupgl control for X11
 *
 * See Copyright Notice in "iup.h"
 */

#include <X11/Xlib.h>
#include <GL/glx.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>

#include "iup.h"
#include "iupcbs.h"
#include "iupgl.h"

#include "iup_object.h"
#include "iup_attrib.h"
#include "iup_str.h"
#include "iup_stdcontrols.h"
#include "iup_assert.h"
#include "iup_register.h"


struct _IcontrolData 
{
  iupCanvas canvas;  /* from IupCanvas (must reserve it) */

  Display* display;
  Drawable window;

  Colormap colormap;
  XVisualInfo *vinfo;
  GLXContext context;
};

static int xGLCanvasDefaultResize(Ihandle *ih, int width, int height)
{
  if (ih->data->window == None)
    ih->data->window = (XID)IupGetAttribute(ih, "XWINDOW");

  IupGLMakeCurrent(ih);
  glViewport(0,0,width,height);
  return IUP_DEFAULT;
}

static int xGLCanvasCreateMethod(Ihandle* ih, void** params)
{
  (void)params;
  free(ih->data); /* allocated by the iCanvasCreateMethod of IupCanvas */
  ih->data = iupALLOCCTRLDATA();
  IupSetCallback(ih, "RESIZE_CB", (Icallback)xGLCanvasDefaultResize);
  return IUP_NOERROR;
}

static int xGLCanvasMapMethod(Ihandle* ih)
{
  /* the IupCanvas is already mapped, just keep a copy of these attributes */
  ih->data->window = (XID)IupGetAttribute(ih, "XWINDOW");  /* works for Motif and GTK */
  return IUP_NOERROR;
}

static char* xglCanvasGetVisualAttrib(Ihandle* ih)
{
  int erb, evb, number;
  int isIndex = 0;
  int n = 0;
  int alist[40];
  GLXContext shared_context = NULL;
  Ihandle* ih_shared;

  if (ih->data->vinfo)
    return (char*)ih->data->vinfo->visual;

  ih->data->display = (Display*)IupGetAttribute(ih, "XDISPLAY");  /* works for Motif and GTK */

  /* double or single buffer */
  if (iupStrEqualNoCase(IupGetAttribute(ih,"BUFFER"), "DOUBLE"))
  {
    alist[n++] = GLX_DOUBLEBUFFER;
  }

  /* stereo */
  if (IupGetInt(ih,"STEREO"))
  {
    alist[n++] = GLX_STEREO;
  }

  /* rgba or index */ 
  if (iupStrEqualNoCase(IupGetAttribute(ih,"COLOR"), "INDEX"))
  {
    isIndex = 1;
    /* buffer size (for index mode) */
    number = IupGetInt(ih,"BUFFER_SIZE");
    if (number > 0)
    {
      alist[n++] = GLX_BUFFER_SIZE;
      alist[n++] = number;
    }
  }
  else
  {
    alist[n++] = GLX_RGBA;      /* assume rgba as default */
  }

  /* red, green, blue bits */
  number = IupGetInt(ih,"RED_SIZE");
  if (number > 0) 
  {
    alist[n++] = GLX_RED_SIZE;
    alist[n++] = number;
  }

  number = IupGetInt(ih,"GREEN_SIZE");
  if (number > 0) 
  {
    alist[n++] = GLX_GREEN_SIZE;
    alist[n++] = number;
  }

  number = IupGetInt(ih,"BLUE_SIZE");
  if (number > 0) 
  {
    alist[n++] = GLX_BLUE_SIZE;
    alist[n++] = number;
  }

  number = IupGetInt(ih,"ALPHA_SIZE");
  if (number > 0) 
  {
    alist[n++] = GLX_ALPHA_SIZE;
    alist[n++] = number;
  }

  /* depth and stencil size */
  number = IupGetInt(ih,"DEPTH_SIZE");
  if (number > 0) 
  {
    alist[n++] = GLX_DEPTH_SIZE;
    alist[n++] = number;
  }

  number = IupGetInt(ih,"STENCIL_SIZE");
  if (number > 0) 
  {
    alist[n++] = GLX_STENCIL_SIZE;
    alist[n++] = number;
  }

  /* red, green, blue accumulation bits */
  number = IupGetInt(ih,"ACCUM_RED_SIZE");
  if (number > 0) 
  {
    alist[n++] = GLX_ACCUM_RED_SIZE;
    alist[n++] = number;
  }

  number = IupGetInt(ih,"ACCUM_GREEN_SIZE");
  if (number > 0) 
  {
    alist[n++] = GLX_ACCUM_GREEN_SIZE;
    alist[n++] = number;
  }

  number = IupGetInt(ih,"ACCUM_BLUE_SIZE");
  if (number > 0) 
  {
    alist[n++] = GLX_ACCUM_BLUE_SIZE;
    alist[n++] = number;
  }

  number = IupGetInt(ih,"ACCUM_ALPHA_SIZE");
  if (number > 0) 
  {
    alist[n++] = GLX_ACCUM_ALPHA_SIZE;
    alist[n++] = number;
  }
  alist[n++] = None;

  /* check out X extension */
  if (!glXQueryExtension(ih->data->display, &erb, &evb))
  {
    IupSetAttribute(ih, "ERROR", "X server has no OpenGL GLX extension");
    return NULL;
  }

  /* choose visual */
  if ((ih->data->vinfo = glXChooseVisual(ih->data->display, DefaultScreen(ih->data->display), alist)) == NULL)
  {
    IupSetAttribute(ih, "ERROR", "No appropriate visual");
    return NULL;
  } 

  ih_shared = IupGetAttributeHandle(ih, "SHAREDCONTEXT");
  if (ih_shared)
    shared_context = ih_shared->data->context;

  /* create rendering context */
  if ((ih->data->context = glXCreateContext(ih->data->display, ih->data->vinfo, shared_context, GL_TRUE)) == NULL)
  {
    IupSetAttribute(ih, "ERROR", "Could not create a rendering context");
    return NULL;
  }
  iupAttribSetStr(ih, "CONTEXT", (char*)ih->data->context);

  /* create colormap for index mode */
  if (isIndex && ih->data->vinfo->class != StaticColor && ih->data->vinfo->class != StaticGray)
  {
    ih->data->colormap = XCreateColormap(ih->data->display, RootWindow(ih->data->display, DefaultScreen(ih->data->display)), ih->data->vinfo->visual, AllocAll);
    iupAttribSetStr(ih, "COLORMAP", (char*)ih->data->colormap);
  }

  if (ih->data->colormap != None)
  {
    /* set default background as white */
    IupGLPalette(ih,0,1,1,1);
  }

  return (char*)ih->data->vinfo->visual;
}

static void xGLCanvasUnMapMethod(Ihandle* ih)
{
  if (ih->data->context)
  {
    if (ih->data->context == glXGetCurrentContext())
      glXMakeCurrent(ih->data->display, None, NULL);

    glXDestroyContext(ih->data->display, ih->data->context);
  }

  if (ih->data->colormap != None)
    XFreeColormap(ih->data->display, ih->data->colormap);

  if (ih->data->vinfo)
    XFree(ih->data->vinfo); 
}

static Iclass* xGlCanvasGetClass(void)
{
  Iclass* ic = iupClassNew(iupCanvasGetClass());

  ic->name = "glcanvas";
  ic->format = "A"; /* one optional callback name */
  ic->nativetype = IUP_TYPECANVAS;
  ic->childtype = IUP_CHILDNONE;
  ic->is_interactive = 1;

  ic->Create = xGLCanvasCreateMethod;
  ic->Map = xGLCanvasMapMethod;
  ic->UnMap = xGLCanvasUnMapMethod;

  iupClassRegisterAttribute(ic, "VISUAL", xglCanvasGetVisualAttrib, iupBaseNoSetAttrib, NULL, IUP_NOT_MAPPED, IUP_NO_INHERIT);

  return ic;
}


/******************************************* Exported functions */

void IupGLCanvasOpen(void)
{
  iupRegisterClass(xGlCanvasGetClass());
}

Ihandle* IupGLCanvas(const char *action)
{
  void *params[2];
  params[0] = (void*)action;
  params[1] = NULL;
  return IupCreatev("glcanvas", params);
}

int IupGLIsCurrent(Ihandle* ih)
{
  iupASSERT(iupObjectCheck(ih));
  if (!iupObjectCheck(ih))
    return 0;

  /* must be a IupGLCanvas */
  if (!iupStrEqual(ih->iclass->name, "glcanvas"))
    return 0;

  /* must be mapped */
  if (!ih->handle)
    return 0;

  if (ih->data->context == glXGetCurrentContext())
    return 1;

  return 0;
}

void IupGLMakeCurrent(Ihandle* ih)
{
  iupASSERT(iupObjectCheck(ih));
  if (!iupObjectCheck(ih))
    return;

  /* must be a IupGLCanvas */
  if (!iupStrEqual(ih->iclass->name, "glcanvas"))
    return;

  /* must be mapped */
  if (!ih->handle)
    return;

  glXMakeCurrent(ih->data->display, ih->data->window, ih->data->context);
  glXWaitX();
}

void IupGLSwapBuffers(Ihandle* ih)
{
  iupASSERT(iupObjectCheck(ih));
  if (!iupObjectCheck(ih))
    return;

  /* must be a IupGLCanvas */
  if (!iupStrEqual(ih->iclass->name, "glcanvas"))
    return;

  /* must be mapped */
  if (!ih->handle)
    return;

  glXSwapBuffers(ih->data->display, ih->data->window);
}

static int xGLCanvasIgnoreError(Display *param1, XErrorEvent *param2)
{
  (void)param1;
  (void)param2;
  return 0;
}

void IupGLPalette(Ihandle* ih, int index, float r, float g, float b)
{
  XColor color;
  int rShift, gShift, bShift;
  XVisualInfo *vinfo;
  XErrorHandler old_handler;

  iupASSERT(iupObjectCheck(ih));
  if (!iupObjectCheck(ih))
    return;

  /* must be a IupGLCanvas */
  if (!iupStrEqual(ih->iclass->name, "glcanvas"))
    return;

  /* must be mapped */
  if (!ih->handle)
    return;

  /* must have a colormap */
  if (ih->data->colormap == None)
    return;

  /* code fragment based on the toolkit library provided with OpenGL */
  old_handler = XSetErrorHandler(xGLCanvasIgnoreError);

  vinfo = ih->data->vinfo;
  switch (vinfo->class) 
  {
  case DirectColor:
    rShift = ffs((unsigned int)vinfo->red_mask) - 1;
    gShift = ffs((unsigned int)vinfo->green_mask) - 1;
    bShift = ffs((unsigned int)vinfo->blue_mask) - 1;
    color.pixel = ((index << rShift) & vinfo->red_mask) |
                  ((index << gShift) & vinfo->green_mask) |
                  ((index << bShift) & vinfo->blue_mask);
    color.red = (unsigned short)(r * 65535.0 + 0.5);
    color.green = (unsigned short)(g * 65535.0 + 0.5);
    color.blue = (unsigned short)(b * 65535.0 + 0.5);
    color.flags = DoRed | DoGreen | DoBlue;
    XStoreColor(ih->data->display, ih->data->colormap, &color);
    break;
  case GrayScale:
  case PseudoColor:
    if (index < vinfo->colormap_size) 
    {
      color.pixel = index;
      color.red = (unsigned short)(r * 65535.0 + 0.5);
      color.green = (unsigned short)(g * 65535.0 + 0.5);
      color.blue = (unsigned short)(b * 65535.0 + 0.5);
      color.flags = DoRed | DoGreen | DoBlue;
      XStoreColor(ih->data->display, ih->data->colormap, &color);
    }
    break;
  }

  XSync(ih->data->display, 0);
  XSetErrorHandler(old_handler);
}
