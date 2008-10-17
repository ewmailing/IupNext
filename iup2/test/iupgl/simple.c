#ifdef WIN32
#include <windows.h>
#endif

#include "iup.h"          
#include "iupcontrols.h"
#include "iupgl.h"
#include <GL/gl.h>
#include <stdio.h>

int keypress_cb(Ihandle *self, int c, int pressed)
{
  printf("keypress_cb %c \"%s\"\n", (char)c, pressed? "pressed": "released");
  return IUP_DEFAULT;
}

int k_any(Ihandle *self, int c)
{
  printf("k_any %c\n", (char)c);

  if(c == K_q)
    return IUP_CLOSE;
  else
    return IUP_DEFAULT;
}

int resize_cb(Ihandle *self, int width, int height)
{
  printf("resize_cb %d %d\n", width, height);
  return IUP_DEFAULT;
}

int map_cb(Ihandle *self)
{
  printf("map_cb\n");
  return IUP_DEFAULT;
}

int action(Ihandle *self, float x, float y)
{
  printf("action %g %g\n", (double)x, (double)y);
  IupGLMakeCurrent(self);

  glClearColor(1.0, 1.0, 1.0, 1.f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//  glMatrixMode( GL_PROJECTION );
//  glViewport( 0, 0, 123, 200);
//  glLoadIdentity();

  glBegin( GL_LINES ); 
    glColor4d(1.0, 0.0, 0.0, 1.0);
    glVertex2d(0.0, 0.0);
    glVertex2d(10.0, 10.0);
  glEnd();

  
  IupGLSwapBuffers(self); 

  return IUP_DEFAULT;
}

int main(int argc, char **argv)
{
  Ihandle *canvas, *dg;

  IupOpen(&argc, &argv);
  IupTabsOpen();
  IupGLCanvasOpen();

  IupSetFunction("action", (Icallback) action);
  IupSetFunction("map_cb", (Icallback) map_cb);
  IupSetFunction("resize_cb", (Icallback) resize_cb);
  IupSetFunction("keypress_cb", (Icallback) keypress_cb);
  IupSetFunction("k_any", (Icallback) k_any);

  canvas = IupGLCanvas("action");
  IupSetAttribute(canvas, IUP_BUFFER, IUP_DOUBLE);
//  IupSetAttribute(canvas, IUP_RASTERSIZE, "150x200");
//  IupSetAttribute(canvas, IUP_SCROLLBAR, "YES");
//  IupSetAttribute(canvas, IUP_BORDER, "NO");
//  IupSetAttribute(canvas, IUP_BGCOLOR, "255 255 255");

  IupSetAttribute(canvas, IUP_MAP_CB, "map_cb");
  IupSetAttribute(canvas, IUP_RESIZE_CB, "resize_cb");
//  IupSetAttribute(canvas, IUP_KEYPRESS_CB, "keypress_cb");
  IupSetAttribute(canvas, IUP_K_ANY, "k_any");

  //dg = IupDialog(
  //        IupTabs(
  //          IupSetAttributes(
  //            IupVbox(canvas, IupText(""), NULL), 
  //            "MARGIN=10x10, GAP=10, TABTITLE=Test1"), 
  //          NULL)) ;

  dg = IupDialog(canvas);
  IupSetAttribute(dg, IUP_SIZE, "100x100");
  IupSetAttribute(dg, IUP_TITLE, "IupGLCanvas");

  printf("IupMap\n");
  IupMap(dg);
  IupGLMakeCurrent(canvas);
  printf("Vendor: %s\n", glGetString(GL_VENDOR));
  printf("Renderer: %s\n", glGetString(GL_RENDERER));
  printf("Version: %s\n", glGetString(GL_VERSION));

  printf("IupShow\n");
  IupShow(dg);

  IupMainLoop();
  IupDestroy(dg);
  IupClose();

  return 1;
}

