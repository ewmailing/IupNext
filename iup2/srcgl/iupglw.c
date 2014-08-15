/** \file
 * \brief iupgl control for Windows
 *
 * See Copyright Notice in iup.h
 *  */

#include "iup.h"
#include "iupcpi.h"
#include "iupcompat.h"
#include "iupgl.h"

#include <windows.h>
#include <GL/gl.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>


typedef struct _GLData
{
  HDC   device;
  HGLRC context;
  HPALETTE palette;
} GLData;

static int defresize (Ihandle *cv, int width, int height)
{
  IupGLMakeCurrent(cv);
  glViewport(0,0,width,height);
  return IUP_DEFAULT;
}

static Ihandle *create (Iclass *ic, Ihandle **params)
{
  char *action = params? (char*) params[0] : NULL;
  Ihandle *self = IupCanvas(action);
  GLData *d = (GLData*) malloc(sizeof(GLData)); assert(d);
  d->device = NULL;
  d->context = NULL;
  d->palette = NULL;

  IupSetAttribute(self,"_IUPGL_DATA",(char*) d);	
  IupSetCallback(self,IUP_RESIZE_CB,(Icallback)defresize);

  return self;
}

static int setvisual (Ihandle* self)
{
  HWND hwnd = (HWND)iupGetNativeHandle(self);
  int pixelFormat;
  PIXELFORMATDESCRIPTOR test_pfd;
  PIXELFORMATDESCRIPTOR pfd = { 
    sizeof(PIXELFORMATDESCRIPTOR),  //  size of this pfd 
      1,                     // version number 
      PFD_DRAW_TO_WINDOW |   // support window 
      PFD_SUPPORT_OPENGL,     // support OpenGL
      PFD_TYPE_RGBA,         // RGBA type 
      24,                    // 24-bit color depth 
      0, 0, 0, 0, 0, 0,      // color bits ignored 
      0,                     // no alpha buffer 
      0,                     // shift bit ignored 
      0,                     // no accumulation buffer 
      0, 0, 0, 0,            // accum bits ignored 
      16,                    // 32-bit z-buffer	 
      0,                     // no stencil buffer 
      0,                     // no auxiliary buffer 
      PFD_MAIN_PLANE,        // main layer 
      0,                     // reserved 
      0, 0, 0                // layer masks ignored 
  };
  GLData* d = (GLData*)IupGetAttribute(self,"_IUPGL_DATA");
  Ihandle* shared;
  char* value;
  int number;
  int isIndex = 0;

  /* double or single buffer */
  value = IupGetAttribute(self,IUP_BUFFER);
  if (value && iupStrEqual(value,IUP_DOUBLE))
    pfd.dwFlags |= PFD_DOUBLEBUFFER;

  /* stereo */
  value = IupGetAttribute(self,IUP_STEREO);
  if (value && iupStrEqual(value,IUP_YES))
    pfd.dwFlags |= PFD_STEREO;

  /* rgba or index */ 
  value = IupGetAttribute(self,IUP_COLOR);
  if (value && iupStrEqual(value,IUP_INDEX))
  {
    isIndex = 1;
    pfd.iPixelType = PFD_TYPE_COLORINDEX;
    pfd.cColorBits = 8;	// assume 8 bits as default  
    value = IupGetAttribute(self,IUP_BUFFER_SIZE);
    if (value && (number = atoi(value)) > 0)
      pfd.cColorBits = number;
  }

  /* red, green, blue bits */
  value = IupGetAttribute(self,IUP_RED_SIZE);
  if (value && (number = atoi(value)) > 0)
    pfd.cRedBits = number;
  pfd.cRedShift = 0;
  value = IupGetAttribute(self,IUP_GREEN_SIZE);
  if (value && (number = atoi(value)) > 0)
    pfd.cGreenBits = number;
  pfd.cGreenShift = pfd.cRedBits;
  value = IupGetAttribute(self,IUP_BLUE_SIZE);
  if (value && (number = atoi(value)) > 0)
    pfd.cBlueBits = number;
  pfd.cBlueShift = pfd.cRedBits + pfd.cGreenBits;
  value = IupGetAttribute(self,IUP_ALPHA_SIZE);
  if (value && (number = atoi(value)) > 0)
    pfd.cAlphaBits = number;
  pfd.cAlphaShift = pfd.cRedBits + pfd.cGreenBits + pfd.cBlueBits;

  /* depth and stencil size */
  value = IupGetAttribute(self,IUP_DEPTH_SIZE);
  if (value && (number = atoi(value)) > 0)
    pfd.cDepthBits = number;

  /* stencil */
  value = IupGetAttribute(self,IUP_STENCIL_SIZE);
  if (value && (number = atoi(value)) > 0)
    pfd.cStencilBits = number;

  /* red, green, blue accumulation bits */
  value = IupGetAttribute(self,IUP_ACCUM_RED_SIZE);
  if (value && (number = atoi(value)) > 0)
    pfd.cAccumRedBits = number;
  value = IupGetAttribute(self,IUP_ACCUM_GREEN_SIZE);
  if (value && (number = atoi(value)) > 0)
    pfd.cAccumGreenBits = number;
  value = IupGetAttribute(self,IUP_ACCUM_BLUE_SIZE);
  if (value && (number = atoi(value)) > 0)
    pfd.cAccumBlueBits = number;
  value = IupGetAttribute(self,IUP_ACCUM_ALPHA_SIZE);
  if (value && (number = atoi(value)) > 0)
    pfd.cAccumAlphaBits = number;
  pfd.cAccumBits = pfd.cAccumRedBits + pfd.cAccumGreenBits+
    pfd.cAccumBlueBits + pfd.cAccumAlphaBits;

  /* get a device context */
  d->device = GetDC(hwnd);

  /* choose pixel format */
  if ((pixelFormat = ChoosePixelFormat(d->device, &pfd)) == 0) 
  {
    IupSetAttribute(self, "ERROR", "No appropriate pixel format.");
    fprintf(stderr,"No appropriate pixel format.\n");
    return 0;
  } 
  SetPixelFormat(d->device,pixelFormat,&pfd);

  /* create rendering context */
  if ((d->context = wglCreateContext(d->device)) == NULL)
  {
    IupSetAttribute(self, "ERROR", "Could not create a rendering context.");
    fprintf(stderr,"Could not create a rendering context.\n");
    return 0;
  }

  shared = IupGetAttributeHandle(self, "SHAREDCONTEXT");
  if (shared)
  {
    GLData* d1 = (GLData*)IupGetAttribute(shared,"_IUPGL_DATA");
    wglShareLists(d1->context, d->context);
  }

  DescribePixelFormat(d->device, pixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &test_pfd);

  value = IupGetAttribute(self,IUP_STEREO);

  if (value && iupStrEqual(value,IUP_YES))
  {
    if (!(test_pfd.dwFlags & PFD_STEREO))
    {
      IupSetAttribute(self, "ERROR", "Stereo not available.");
      fprintf(stderr,"Stereo not available.\n");
      return 0;
    }
  }

  /* create colormap for index mode */
  if (isIndex)
  {
    LOGPALETTE lp = {0x300,1,{255,255,255,PC_NOCOLLAPSE}};
    d->palette = CreatePalette(&lp);
    ResizePalette(d->palette,1<<pfd.cColorBits);
    SelectPalette(d->device,d->palette,FALSE);
    RealizePalette(d->device);
  }

  return 1;
}

static void map (Ihandle* self, Ihandle* parent)
{
  iupCpiDefaultMap(self,parent); 	/* call default method */
  setvisual(self);
}

static void destroy (Ihandle* self)
{
  HWND hwnd = (HWND)iupGetNativeHandle(self);
  GLData* d = (GLData*)IupGetAttribute(self,"_IUPGL_DATA");

  if (d->context)
  {
    wglMakeCurrent(d->device,NULL);
    wglDeleteContext(d->context);
  }

  if (d->palette != NULL)
    DeleteObject((HGDIOBJ)d->palette);
  if (d->device)
    ReleaseDC(hwnd,d->device);

  free(d);

  iupSetEnv(self, "_IUPGL_DATA", NULL);
}


/******************************************* Exported functions */

void IupGLCanvasOpen(void)
{
  Iclass *ICglcanvas = iupCpiCreateNewClass("glcanvas","n");

  iupCpiSetClassMethod(ICglcanvas, ICPI_CREATE, (Imethod) create);
  iupCpiSetClassMethod(ICglcanvas, ICPI_DESTROY, (Imethod) destroy);
  iupCpiSetClassMethod(ICglcanvas, ICPI_MAP, (Imethod) map);
}

Ihandle *IupGLCanvas (const char *redraw)
{
  return IupCreatep("glcanvas", (void*) redraw, NULL);
}

int IupGLIsCurrent (Ihandle* self)
{
  GLData* d = (GLData*)IupGetAttribute(self,"_IUPGL_DATA"); 
  if (d)
  {
    if (d->context == wglGetCurrentContext())
      return 1;
  }
  return 0;
}

void IupGLMakeCurrent (Ihandle* self)
{
  GLData* d = (GLData*)IupGetAttribute(self,"_IUPGL_DATA"); 
  if (d)
    wglMakeCurrent(d->device,d->context);
}

void IupGLSwapBuffers (Ihandle* self)
{
  GLData* d = (GLData*)IupGetAttribute(self,"_IUPGL_DATA");
  if (d)
    SwapBuffers(d->device);
}

void IupGLPalette (Ihandle* self, int index, float r, float g, float b)
{
  GLData* d = (GLData*)IupGetAttribute(self,"_IUPGL_DATA");
  if (d && d->palette)
  {
    PALETTEENTRY entry;
    entry.peRed    = (BYTE)(r*255);
    entry.peGreen  = (BYTE)(g*255);
    entry.peBlue   = (BYTE)(b*255);
    entry.peFlags  = PC_NOCOLLAPSE;
    SetPaletteEntries(d->palette,index,1,&entry);
    UnrealizeObject(d->device);
    SelectPalette(d->device,d->palette,FALSE);
    RealizePalette(d->device);
  }
}

