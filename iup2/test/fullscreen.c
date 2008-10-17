
#include <stdio.h>
#include <GL/glut.h>
#include "iup.h"
#include "iupgl.h"
/*
#include "Xm/Xm.h"
#include "Xm/XmForm.h"
#include "Xm/XmLabel.h"
#include "Xm/PushB.h"
#include "Xm/List.h"
#include "Xm/Scale.h"
*/

static int cbRedraw(Ihandle *self)
{
  glClearColor(1,0,1,0);
  glClear(GL_COLOR_BUFFER_BIT);
  glDrawBuffer(GL_FRONT);


  glMatrixMode (GL_MODELVIEW);
  glLoadIdentity ();

  IupGLMakeCurrent(self); 
  
  return IUP_DEFAULT;
}

int cbReshape(Ihandle *self, int telaW, int telaH)
{
  glViewport(0, 0, telaW, telaH);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  gluOrtho2D(0, telaW, 0, telaH);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();    

  IupGLMakeCurrent(self); 
  return IUP_DEFAULT;
}


static int cbIupKAny(Ihandle *self, int c)
{     
  printf("k_any: %p %c\n", self, c);        

	if(c == K_ESC) return IUP_CLOSE;

	return IUP_DEFAULT;
}

Ihandle *dg;
Ihandle *imageCanvas;

int toggle()
{
  static int full = 1;
  if(full)
    IupSetAttribute(dg, "FULLSCREEN", "NO");
  else
    IupSetAttribute(dg, "FULLSCREEN", "YES");

  full = !full;
}

int main()
{
  IupOpen();
  IupGLCanvasOpen();
 
  //imageCanvas = IupCanvas("redraw_cb");
  imageCanvas = IupGLCanvas("redraw_cb");
  IupSetAttribute(imageCanvas, IUP_BUFFER, IUP_DOUBLE);
  IupSetAttribute(imageCanvas, IUP_RGBA, IUP_YES);
  IupSetHandle("imageCanvas",imageCanvas);

  dg = IupDialog(IupFrame(IupHbox(imageCanvas, IupButton("toggle fullscreen", "toggle"), NULL)));

  IupSetAttribute(dg, IUP_K_ANY, "iup_k_any_cb");

  IupSetAttribute(dg, "FULLSCREEN", "YES");  
  //IupSetAttribute(dg, "RASTERSIZE", "1280x960");  
  IupShow(dg);
	/*
  {
    Widget w=(Widget)IupGetAttribute(dg, IUP_WID);
    XRaiseWindow(XtDisplay(w), XtWindow(w));
  }
  */

  IupSetFunction("redraw_cb", (Icallback)cbRedraw );
  IupSetFunction("toggle", (Icallback)toggle );
  IupSetFunction("iup_k_any_cb",(Icallback)cbIupKAny);
  IupSetFunction("reshape_cb", (Icallback)cbReshape);

//  IupSetAttribute(dg, IUP_RASTERSIZE, "800x600"); 
  cbReshape(imageCanvas, 800, 600);
  IupSetFocus(imageCanvas);
  
  IupMainLoop();
  IupClose();
  return 1;
}

