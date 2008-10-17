/** \file
 * \brief iupgl control for X11
 *
 * See Copyright Notice in iup.h
 * $Id: iupglx.c,v 1.1 2008-10-17 06:20:41 scuri Exp $
 */

#include "iup.h"
#include "iupcpi.h"
#include "iupcompat.h"
#include "iupgl.h"

#include <GL/glx.h>
#include <GL/glu.h>
#include <X11/Intrinsic.h>
#include <Xm/Xm.h>
#include <Xm/XmStrDefs.h>
#include <GL/GLwMDrawA.h>
              
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

/* from motcreat.c */
void iupmotCreateCanvasWithClass( Ihandle* n, WidgetClass c, ... );

static int defresize (Ihandle *cv, int width, int height)
{
  IupGLMakeCurrent(cv);
  glXWaitX();
  glViewport(0,0,width,height);
  return IUP_DEFAULT;
}

static Ihandle *create (Iclass *ic, Ihandle **params)
{
  char *action = params? (char*) params[0] : NULL;
  Ihandle *self = IupCanvas(action);
  IupSetCallback(self,IUP_RESIZE_CB,(Icallback)defresize);
  IupSetAttribute(self, "COLORMAP", (char*) None);
  return self;
}

#define N 40
/* set appropriate visual and create widget */
static void map (Ihandle* self, Ihandle* parent)
{
  int erb, evb;
  Widget widget = (Widget)iupGetNativeHandle(parent);
  Display* display = XtDisplay(widget);
  int screen = XScreenNumberOfScreen(XtScreen(widget));
  char* value;
  int number, printinfo;
  int isIndex = 0;
  int n = 0;
  int alist[N];
  XVisualInfo *vinfo;
  GLXContext context, shared_context = 0;
  Colormap colormap = None;
  Ihandle* shared;

  printinfo = IupGetInt(self, "PRINTINFO");
  if (printinfo) 
  {                               
    char *str;
    fprintf(stderr,"GL_VERSION=%s\n", (str = (char*)glGetString(GL_VERSION))? str: "NULL");
    fprintf(stderr,"GLU_VERSION=%s\n", (str = (char*)gluGetString(GLU_VERSION))? str: "NULL");
    fprintf(stderr,"GL_RENDERER=%s\n", (str = (char*)glGetString(GL_RENDERER))? str: "NULL");
    fprintf(stderr,"GL_VENDOR=%s\n", (str = (char*)glGetString(GL_VENDOR))? str: "NULL");
  }

  /* double or single buffer */
  value = IupGetAttribute(self,IUP_BUFFER);
  if (value && iupStrEqual(value,IUP_DOUBLE))
  {
    alist[n++] = GLX_DOUBLEBUFFER;
    if (printinfo) fprintf(stderr,"IUPGL: BUFFER=DOUBLE\n");
  }

  /* stereo */
  value = IupGetAttribute(self,IUP_STEREO);
  if (value && iupStrEqual(value,IUP_YES))
  {
    alist[n++] = GLX_STEREO;
    if (printinfo) fprintf(stderr,"IUPGL: STEREO=YES\n");
  }

  /* rgba or index */ 
  value = IupGetAttribute(self,IUP_COLOR);
  if (!value || !iupStrEqual(value,IUP_INDEX))
  {
    alist[n++] = GLX_RGBA;      /* assume rgba as default */
    if (printinfo) fprintf(stderr,"IUPGL: COLOR=RGBA\n");
  }
  else
  {
    isIndex = 1;
    if (printinfo) fprintf(stderr,"IUPGL: COLOR=INDEX\n");
    /* buffer size (for index mode) */
    value = IupGetAttribute(self,IUP_BUFFER_SIZE);
    if (value && (number = atoi(value)) >= 0)
    {
      alist[n++] = GLX_BUFFER_SIZE;
      alist[n++] = number;
      if (printinfo) fprintf(stderr,"IUPGL: BUFFER_SIZE=%d\n", number);
    }
    else
    {
      if (printinfo) fprintf(stderr,"IUPGL: BUFFER_SIZE=0\n");
    }
  }

  /* red, green, blue bits */
  value = IupGetAttribute(self,IUP_RED_SIZE);
  if (value && (number = atoi(value)) > 0)
  {
    alist[n++] = GLX_RED_SIZE;
    alist[n++] = number;
    if (printinfo) fprintf(stderr,"IUPGL: RED_SIZE=%d\n", number);
  }
  value = IupGetAttribute(self,IUP_GREEN_SIZE);
  if (value && (number = atoi(value)) > 0)
  {
    alist[n++] = GLX_GREEN_SIZE;
    alist[n++] = number;
    if (printinfo) fprintf(stderr,"IUPGL: GREEN_SIZE=%d\n", number);
  }
  value = IupGetAttribute(self,IUP_BLUE_SIZE);
  if (value && (number = atoi(value)) > 0)
  {
    alist[n++] = GLX_BLUE_SIZE;
    alist[n++] = number;
    if (printinfo) fprintf(stderr,"IUPGL: BLUE_SIZE=%d\n", number);
  }
  value = IupGetAttribute(self,IUP_ALPHA_SIZE);
  if (value && (number = atoi(value)) > 0)
  {
    alist[n++] = GLX_ALPHA_SIZE;
    alist[n++] = number;
    if (printinfo) fprintf(stderr,"IUPGL: ALPHA_SIZE=%d\n", number);
  }

  /* depth and stencil size */
  value = IupGetAttribute(self,IUP_DEPTH_SIZE);
  if (value && (number = atoi(value)) > 0)
  {
    alist[n++] = GLX_DEPTH_SIZE;
    alist[n++] = number;
    if (printinfo) fprintf(stderr,"IUPGL: DEPTH_SIZE=%d\n", number);
  }
  value = IupGetAttribute(self,IUP_STENCIL_SIZE);
  if (value && (number = atoi(value)) > 0)
  {
    alist[n++] = GLX_STENCIL_SIZE;
    alist[n++] = number;
    if (printinfo) fprintf(stderr,"IUPGL: STENCIL_SIZE=%d\n", number);
  }

  /* red, green, blue accumulation bits */
  value = IupGetAttribute(self,IUP_ACCUM_RED_SIZE);
  if (value && (number = atoi(value)) > 0)
  {
    alist[n++] = GLX_ACCUM_RED_SIZE;
    alist[n++] = number;
    if (printinfo) fprintf(stderr,"IUPGL: ACCUM_RED_SIZE=%d\n", number);
  }
  value = IupGetAttribute(self,IUP_ACCUM_GREEN_SIZE);
  if (value && (number = atoi(value)) > 0)
  {
    alist[n++] = GLX_ACCUM_GREEN_SIZE;
    alist[n++] = number;
    if (printinfo) fprintf(stderr,"IUPGL: ACCUM_GREEN_SIZE=%d\n", number);
  }
  value = IupGetAttribute(self,IUP_ACCUM_BLUE_SIZE);
  if (value && (number = atoi(value)) > 0)
  {
    alist[n++] = GLX_ACCUM_BLUE_SIZE;
    alist[n++] = number;
    if (printinfo) fprintf(stderr,"IUPGL: ACCUM_BLUE_SIZE=%d\n", number);
  }
  value = IupGetAttribute(self,IUP_ACCUM_ALPHA_SIZE);
  if (value && (number = atoi(value)) > 0)
  {
    alist[n++] = GLX_ACCUM_ALPHA_SIZE;
    alist[n++] = number;
    if (printinfo) fprintf(stderr,"IUPGL: ACCUM_ALPHA_SIZE=%d\n", number);
  }
  alist[n++] = None;
  assert(n<=N);

  /* check out X extension */
  if (!glXQueryExtension(display, &erb, &evb))
  {
    IupSetAttribute(self, "ERROR", "X server has no OpenGL GLX extension");
    fprintf(stderr,"IUPGL: X server has no OpenGL GLX extension.\n");
    return;
  }

  /* choose visual */
  if ((vinfo = glXChooseVisual(display,screen,alist)) == NULL)
  {
    IupSetAttribute(self, "ERROR", "No appropriate visual");
    fprintf(stderr,"IUPGL: No appropriate visual.\n");
    return;
  } 
  else
    IupSetAttribute(self, "VISUAL", (char*) vinfo);

  shared = IupGetAttributeHandle(self, "SHAREDCONTEXT");
  if (shared)
    shared_context = (GLXContext)IupGetAttribute(shared, "CONTEXT");

  /* create rendering context */
  if ((context = glXCreateContext(display,vinfo,shared_context,GL_TRUE)) == NULL)
  {
    IupSetAttribute(self, "ERROR", "Could not create a rendering context");
    fprintf(stderr,"IUPGL: Could not create a rendering context.\n");
    return;
  }
  else
    IupSetAttribute(self, "CONTEXT", (char*) context);



  /* create colormap for index mode */
  if (isIndex && vinfo->class != StaticColor && vinfo->class != StaticGray)
  {
    colormap = XCreateColormap(display,RootWindow(display,screen),
      vinfo->visual,AllocAll);
    IupSetAttribute(self, "COLORMAP", (char*) colormap);                                
  }

  {
    /* call iup function to create a widget of glw class */
    if (colormap != None)
    {
      iupmotCreateCanvasWithClass(self,glwMDrawingAreaWidgetClass,
        GLwNvisualInfo, (XtArgVal)vinfo,
        NULL
        );
      XtVaSetValues(iupGetNativeHandle(self),XmNcolormap,colormap,NULL);
      /* set default background as white */
      IupGLPalette(self,0,1,1,1);
    }
    else
    {
      iupmotCreateCanvasWithClass(self,glwMDrawingAreaWidgetClass,
        GLwNvisualInfo, (XtArgVal)vinfo,
        NULL
        );
    }

    XtVaSetValues((Widget)iupGetNativeHandle(self),
      XmNtraversalOn, True,
      NULL);

  }
}

static void destroy (Ihandle* self)
{
  Widget widget      = iupGetNativeHandle(self);
  Display* display   = XtDisplay(widget);
  XVisualInfo *vinfo = (XVisualInfo*) IupGetAttribute(self, "VISUAL");
  GLXContext context = (GLXContext) IupGetAttribute(self, "CONTEXT");
  Colormap colormap  = (Colormap) IupGetAttribute(self, "COLORMAP");
  if (context)
    glXDestroyContext(display, context);
  if (colormap != None)
    XFreeColormap(display, colormap);
  if (vinfo)
    XFree(vinfo); 
}


/******************************************* Exported functions */

void IupGLCanvasOpen(void)
{
  Iclass* ICglcanvas = iupCpiCreateNewClass("glcanvas","n");

  iupCpiSetClassMethod(ICglcanvas, ICPI_CREATE, (Imethod) create);
  iupCpiSetClassMethod(ICglcanvas, ICPI_MAP, (Imethod) map);
  iupCpiSetClassMethod(ICglcanvas, ICPI_DESTROY, (Imethod) destroy);
}

Ihandle *IupGLCanvas (const char *redraw)
{
  return IupCreatep("glcanvas", (void*) redraw, NULL);
}

int IupGLIsCurrent (Ihandle* self)
{
  GLXContext context = (GLXContext) IupGetAttribute(self, "CONTEXT");
  if (context)
  {
    if (context == glXGetCurrentContext())
      return 1;
  }
  return 0;
}

void IupGLMakeCurrent (Ihandle* self)
{
  GLXContext context = (GLXContext) IupGetAttribute(self, "CONTEXT");
  if (context)
  {
    Widget widget = iupGetNativeHandle(self);
    glXMakeCurrent(XtDisplay(widget),XtWindow(widget),context);
  }
}

void IupGLSwapBuffers (Ihandle* self)
{
  Widget widget = iupGetNativeHandle(self);
  if(widget)
    glXSwapBuffers(XtDisplay(widget),XtWindow(widget));
}

static int Ignore(Display *parm1, XErrorEvent *parm2)
{
  return 0;
}

void IupGLPalette (Ihandle* self, int index, float r, float g, float b)
{
  XVisualInfo *vinfo = (XVisualInfo*) IupGetAttribute(self, "VISUAL");
  Colormap colormap = (Colormap) IupGetAttribute(self, "COLORMAP");

  if (vinfo)
  {
    /* code fragment based on the toolkit library provided with OpenGL */
    Widget widget = iupGetNativeHandle(self);
    XErrorHandler old_handler;
    XColor color;
    int rShift, gShift, bShift;
    XWindowAttributes watt;

#if _TEST_CODE_
    XGetWindowAttributes(XtDisplay(widget),XtWindow(widget),&watt);
#else
    watt.colormap = colormap;
#endif

    old_handler = XSetErrorHandler(Ignore);

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
      XStoreColor(XtDisplay(widget), watt.colormap, &color);
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
        XStoreColor(XtDisplay(widget), watt.colormap, &color);
      }
      break;
    }

    XSync(XtDisplay(widget), 0);
    XSetErrorHandler(old_handler);
  }
}
