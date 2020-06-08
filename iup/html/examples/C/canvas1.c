/*
 * IupCanvas example
 * Description : Creates a IUP canvas and uses CD to draw on it
 *      Remark : IUP must be linked to the CD library      
 * libraries for static linking: cdcontextplus gdiplus iupcd iup cd freetype6 zlib1
 * libraries for dynamic linking: cdcontextplus iupcd iup cd
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <iup.h>
#include <cd.h>
#include <cdiup.h>

static cdCanvas *cdcanvas = NULL;
int draw = 0;

int button_cb(Ihandle* self, int but, int press, int x, int y)
{
  if (but == IUP_BUTTON1 && press)
  {
    cdCanvasUpdateYAxis(cdcanvas, &y);
    cdCanvasPixel(cdcanvas, x, y, CD_BLUE);
    draw = 1;
  }
  else
  {
    cdCanvasClear(cdcanvas);
    draw = 0;
  }
	
  return IUP_DEFAULT;	
}

int motion_cb(Ihandle* self, int x, int y)
{
  if (draw)
  {
    cdCanvasUpdateYAxis(cdcanvas, &y);
    cdCanvasPixel(cdcanvas, x, y, CD_BLUE);
  }

  return IUP_DEFAULT;
}

int redraw_cb( Ihandle *self, float x, float y )
{
  cdCanvasActivate(cdcanvas);
  cdCanvasClear(cdcanvas);
  cdCanvasForeground(cdcanvas, CD_BLUE);
  cdCanvasBox(cdcanvas, 10, 100, 10, 100);
  cdCanvasForeground(cdcanvas, CD_RED);
  cdCanvasRect(cdcanvas, 10, 100, 10, 100);
//  cdCanvasLine(cdcanvas, 10,10,10,100);
//  cdCanvasLine(cdcanvas, 100,10,100,100);
//  cdCanvasLine(cdcanvas, 10,10,100,10);
//  cdCanvasLine(cdcanvas, 10,100,100,100);
  return IUP_DEFAULT;
}

#ifdef WIN32
int touch_cb(Ihandle *self, int id, int x, int y, char* state)
{
  printf("touch_cb(id=%d x=%d y=%d state=%s)\n", id, x, y, state);
  cdCanvasPixel(cdcanvas, x, cdCanvasInvertYAxis(cdcanvas, y), CD_RED);
  return IUP_DEFAULT;
}
#endif

int multitouch_cb(Ihandle *self, int count, int* id, int* px, int* py, int *pstate)
{
  int i;
  printf("multitouch_cb(count=%d)\n", count);
  for (i = 0; i < count; i++)
  {
    printf("i=%d    id=%d x=%d y=%d state=%c\n", i, id[i], px[i], py[i], pstate[i]);

    cdCanvasPixel(cdcanvas, px[i], cdCanvasInvertYAxis(cdcanvas, py[i]), CD_RED);
  }
  return IUP_DEFAULT;
}

int main(int argc, char **argv)
{
  Ihandle *dlg, *cnvs;
  IupOpen(&argc, &argv);

  cnvs = IupCanvas( NULL );
  IupSetCallback(cnvs, "ACTION",( Icallback )redraw_cb );
  IupSetAttribute(cnvs, "SIZE", "300x100");
//  IupSetCallback(cnvs, "BUTTON_CB",(Icallback)button_cb);
//  IupSetCallback(cnvs, "MOTION_CB",(Icallback)motion_cb);

#ifdef WIN32
  IupSetAttribute(cnvs, "TOUCH", "YES");
  //IupSetCallback(cnvs, "TOUCH_CB",(Icallback)touch_cb);
  IupSetCallback(cnvs, "MULTITOUCH_CB",(Icallback)multitouch_cb);
#endif

  dlg = IupDialog( IupVbox( cnvs, NULL ) );
  IupSetAttribute(dlg, "TITLE", "IupCanvas + Canvas Draw" );
  IupSetAttribute(dlg, "MARGIN", "10x10");
  IupMap( dlg );

  printf("TOUCH=%s\n", IupGetAttribute(cnvs, "TOUCH"));
  
  cdInitContextPlus();
  cdUseContextPlus(1);

  cdcanvas = cdCreateCanvas( CD_IUP, cnvs );
  cdCanvasForeground(cdcanvas, CD_BLUE);
  cdCanvasSetAttribute(cdcanvas, "ANTIALIAS", "0");
  
  IupShowXY( dlg, IUP_CENTER, IUP_CENTER );
  IupMainLoop();
  IupClose();  
  return EXIT_SUCCESS;
}
