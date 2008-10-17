
#include <stdio.h>
#ifdef WIN32
#include <windows.h> 
#include "GL/gl.h"
#include "GL/glu.h"
#else
//#include <GL/glut.h>
#endif
#include "iup.h"
#include "iupgl.h"

Ihandle *dg, *canvas;

void acoxcalcmiddle(HWND hwnd, int *x, int *y)
{
  RECT rect;
  GetWindowRect(hwnd, &rect);
  *x = ((rect.right + rect.left)/2);
  *y = ((rect.top + rect.bottom)/2);
}

int cbMouseMove(Ihandle *self, int x, int y, char *r)
{
  char tmp[20];
  int xx, yy;
  acoxcalcmiddle((HWND)IupGetAttribute(self, IUP_WID), &xx, &yy);
  sprintf(tmp, "%dx%d", xx, yy);
  IupSetGlobal("CURSORPOS", tmp);

  return IUP_DEFAULT;
}

static int cbRedraw(Ihandle *s, double x, double y)
{
  glClearColor(1,0,0,0);
  glClear(GL_COLOR_BUFFER_BIT);
  glDrawBuffer(GL_FRONT);

  glMatrixMode (GL_MODELVIEW);
  glLoadIdentity ();

  return IUP_DEFAULT;
}

int cbReshape(Ihandle *self, int telaW, int telaH)
{
  glViewport (0, 0, telaW, telaH);

  glMatrixMode (GL_PROJECTION);
  glLoadIdentity ();

  gluOrtho2D (0, telaW, 0, telaH);

  glMatrixMode (GL_MODELVIEW);
  glLoadIdentity ();    

  return IUP_DEFAULT;
}


static int cbIupKAny(Ihandle *self, int c)
{     
  printf("\nFocus OK --> Canvas\n");        

	switch(c)
	{
	case K_ESC:
    return IUP_CLOSE;

	case K_F:
  	case K_f:
    printf("\n FFFF \n");        
  	break;
  }
 
	return IUP_DEFAULT;
}

int main(int argc, char **argv)
{
  IupOpen(&argc, &argv);
  IupGLCanvasOpen();    /* inicializa o controlador OpenGL */

  canvas = IupGLCanvas("redraw_cb");
  IupSetAttribute(canvas, "BUFFER", IUP_DOUBLE);
  IupSetAttribute(canvas, "RGBA", IUP_YES);
  IupSetAttribute(canvas, "MOTION_CB", "mousemove_cb");

  IupSetHandle("canvas", canvas);

  dg = IupDialog(canvas);

  /* Uncomment this line for FULLSCREEN mode
  IupSetAttribute(dg, "FULLSCREEN", "YES");  */
  IupSetAttribute(dg, IUP_K_ANY, "iup_k_any_cb");

  IupShow(dg);

  IupSetFunction("redraw_cb", (Icallback)cbRedraw );
  IupSetFunction("iup_k_any_cb",(Icallback)cbIupKAny);
  IupSetFunction("reshape_cb", (Icallback)cbReshape);
  IupSetFunction("mousemove_cb", (Icallback) cbMouseMove);

  IupGLMakeCurrent(canvas); 
  
//  IupSetAttribute(dg, IUP_RASTERSIZE, "800x600"); 
  cbReshape(canvas, 800, 600);
  
  IupMainLoop();
  IupClose();
  return 1;
}

