#include <stdio.h>
#include <stdlib.h>
#include "iup.h"          
#include "iupgl.h"
#ifdef _WIN32
#include <windows.h>
#endif
#include "GL/gl.h"


void centercursor (Ihandle* handle)
{
  char* rastersize = IupGetAttribute(handle,"DRAWSIZE");
  char* X = IupGetAttribute(handle, IUP_X);
  char* Y = IupGetAttribute(handle, IUP_Y);
  int w, h;
  int posxw;
  int posyw;
  sscanf(rastersize,"%dx%d",&w,&h);
  posxw = (int)(0.5f*w);
  posyw = (int)(0.5f*h);
  if (X && Y)
  {
    int windowx = atoi(X);
    int windowy = atoi(Y);
    char text[20];
    sprintf(text,"%dx%d",windowx+posxw,windowy+h-posyw-1);
    printf("IupSetGlobal(\"CURSORPOS\",\"%s\");\n",text);
    IupSetGlobal("CURSORPOS",text);
  }
}


int redraw(Ihandle *self, float x, float y)
{
  int w, h;
  char *size = IupGetAttribute(self, IUP_RASTERSIZE);
  sscanf(size, "%dx%d", &w, &h);


  IupGLMakeCurrent(self);
  glViewport(0, 0, w, h);
  glClearColor(1.0, 1.0, 1.0, 1.f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


  glColor3f(1.0,0.0,0.0);
  glBegin(GL_QUADS); 
  glVertex2f(0.9f,0.9f); 
  glVertex2f(0.9f,-0.9f); 
  glVertex2f(-0.9f,-0.9f); 
  glVertex2f(-0.9f,0.9f); 
  glEnd();


  IupGLSwapBuffers(self); 


  return IUP_DEFAULT;
}


int mousefunc (Ihandle* handle, int bt, int st, int x, int y, char* r)
{
  printf("IUP: MouseButton(st=%s)\n",st?"PRESS":"RELEASE");
  return IUP_DEFAULT;
}


int enterfunc (Ihandle* handle)
{
  printf("IUP: Enter()\n");
  centercursor(handle);
  return IUP_DEFAULT;
}


int main(void)
{
  Ihandle *canvas, *finale, *dg;


  IupOpen();
  IupGLCanvasOpen();


  canvas = IupGLCanvas(NULL);
  IupSetCallback(canvas, "ACTION", (Icallback) redraw);
  IupSetAttribute(canvas, IUP_BUFFER, IUP_DOUBLE);
  IupSetAttribute(canvas, IUP_RASTERSIZE, "123x200");
  IupSetAttribute(canvas,IUP_BUTTON_CB,"vgl_button");
  IupSetAttribute(canvas,IUP_ENTERWINDOW_CB,"vgl_enter");
  IupSetFunction("vgl_button",(Icallback)mousefunc);
  IupSetFunction("vgl_enter",(Icallback)enterfunc);


  finale = IupHbox(IupFill(), 
                   canvas, 
                   IupFill(), 
                   NULL);


  dg = IupDialog(finale);
  IupSetAttribute(dg, IUP_TITLE, "IupGLCanvas");


  IupShow(dg);
  IupMainLoop();
  IupDestroy(dg);
  IupClose();


  return 1;
}
