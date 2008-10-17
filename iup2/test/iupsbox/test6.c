
#include <stdio.h>
#include <assert.h>
#include "iup.h"
#include "iupcontrols.h"
#ifdef WIN32
#include <windows.h>
#endif
#include "GL/gl.h" 
#include "iupgl.h"
#include <time.h>

static Ihandle *dg, *A, *box, *B;

static Ihandle* createtree(void)
{
  Ihandle* tree = IupTree();

  IupSetAttributes(tree,"FONT=COURIER_NORMAL_10, \
                         NAME=Figures, \
                         ADDBRANCH=3D, \
                         ADDBRANCH=2D, \
                         ADDLEAF1=trapeze, \
                         ADDBRANCH1=parallelogram, \
                         ADDLEAF2=diamond, \
                         ADDLEAF2=square, \
                         ADDBRANCH4=triangle, \
                         ADDLEAF5=scalenus, \
                         ADDLEAF5=isoceles, \
                         ADDLEAF5=equilateral, \
                         VALUE=6, \
                         CTRL=ON, \
                         SHIFT=ON, \
                         ADDEXPANDED=NO");
  IupSetAttribute(tree, IUP_RASTERSIZE, "80x80");
  IupSetAttribute(tree, IUP_REDRAW, IUP_YES);
  return tree;
}

static int redraw(Ihandle *self, float x, float y)
{
  int w, h;
  char *size = IupGetAttribute(self, IUP_RASTERSIZE);
  sscanf(size, "%dx%d", &w, &h);

  printf("REDRAW CHAMADO %d. x=%d, y=%d\n", clock(), x, y);
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

static int resize_cb(Ihandle *self, int width, int height)
{
  printf("resize %p %s %dx%d\n", self, IupGetClassName(self), width, height);
  return IUP_DEFAULT;
}

static int map_cb(Ihandle *self)
{
  printf("map %p %s\n", self, IupGetClassName(self));
  return IUP_DEFAULT;
}

static int k_any(Ihandle *self, int c)
{
  printf("k_any %p %s %d\n", self, IupGetClassName(self), c);
  if(c == K_minus)
    printf("menos\n");
  return IUP_DEFAULT;
}

int main(int argc, char **argv)
{
  IupOpen(&argc, &argv);
  IupControlsOpen();
  IupGLCanvasOpen();

  IupSetFunction("redraw", (Icallback) redraw);
  IupSetFunction("resize_cb", (Icallback) resize_cb);
  IupSetFunction("map_cb", (Icallback) map_cb);
  IupSetFunction("k_any", (Icallback) k_any);
  
  A = createtree(); 
  IupSetAttribute(A, "EXPAND", "YES");
  box = IupSbox(A);

  B = IupGLCanvas("redraw");
  IupSetAttribute(B, IUP_BUFFER, IUP_DOUBLE);
  IupSetAttribute(B, IUP_RASTERSIZE, "123x200");
  IupSetAttribute(B, IUP_EXPAND, "YES");
  IupSetAttribute(B, IUP_MAP_CB, "map_cb");
  IupSetAttribute(B, IUP_RESIZE_CB, "resize_cb");
  IupSetAttribute(B, IUP_K_ANY, "k_any");

  dg = IupDialog(IupHbox(box, B, NULL));
  IupShow(dg);

  IupMainLoop();
  IupDestroy(dg);
  IupClose();
  return 1;
}
