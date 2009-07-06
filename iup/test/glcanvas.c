#ifdef USE_OPENGL
#ifdef WIN32
#include <windows.h>
#endif
#include <GL/gl.h>

#include <stdlib.h>
#include <stdio.h>

#include "iup.h"          
#include "iupgl.h"


static int action(Ihandle *ih)
{
  IupGLMakeCurrent(ih);

  glClearColor(1.0, 0.0, 1.0, 1.f);  /* pink */
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glColor3f(1.0,0.0,0.0);  /* red */
  glBegin(GL_QUADS); 
  glVertex2f(0.9f,0.9f); 
  glVertex2f(0.9f,-0.9f); 
  glVertex2f(-0.9f,-0.9f); 
  glVertex2f(-0.9f,0.9f); 
  glEnd();

  IupGLSwapBuffers(ih); 

  return IUP_DEFAULT;
}

void GLCanvasTest(void)
{
  Ihandle *dlg, *canvas, *box;

  IupGLCanvasOpen();

  box = IupVbox(NULL);
  IupSetAttribute(box, "MARGIN", "5x5");

  canvas = IupGLCanvas(NULL);
  IupSetCallback(canvas, "ACTION", action);
  IupSetAttribute(canvas, "BUFFER", "DOUBLE");
  IupSetAttribute(canvas, "BORDER", "NO");
  IupSetAttribute(canvas, "RASTERSIZE", "300x200");
  IupAppend(box, canvas);

  dlg = IupDialog(box);
  IupSetAttribute(dlg, "TITLE", "IupGLCanvas Test");

  IupMap(dlg);
  IupGLMakeCurrent(canvas);
  printf("Vendor: %s\n", glGetString(GL_VENDOR));
  printf("Renderer: %s\n", glGetString(GL_RENDERER));
  printf("Version: %s\n", glGetString(GL_VERSION));

  IupShowXY(dlg, IUP_CENTER, IUP_CENTER);
}

#ifndef BIG_TEST
int main(int argc, char* argv[])
{
  IupOpen(&argc, &argv);
  IupGLCanvasOpen();

  GLCanvasTest();

  IupMainLoop();

  IupClose();

  return EXIT_SUCCESS;
}
#endif
#endif
