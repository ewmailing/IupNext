#include <stdlib.h>
#include <stdio.h>
#include "iup.h"
#include "cd.h"
#include "cdiup.h"


/* Virtual space size: 600x400
   The canvas will be a window into that space.
   If canvas is smaller than the virtual space, scrollbars are active.
   
   The drawing is a red cross from the corners of the virtual space.
   But CD must draw in the canvas space. So the position of the scroolbar
   will define the convertion between canvas space and virtual space.
*/

static int action(Ihandle *ih, float posx, float posy)
{
  cdCanvas *cdcanvas = (cdCanvas*)IupGetAttribute(ih, "_CD_CANVAS");
  int iposx = (int)posx;
  int iposy = (int)posy;
  int virtual_width = 600;
  int virtual_height = 400;
  
  /* invert scroll reference (YMAX-DY - POSY) */
  iposy = virtual_height-1 - IupGetInt(ih, "DY") - iposy;      /* can use IupGetInt since we set as integers */
  
  cdCanvasClear(cdcanvas);

  /* to convert from virtual space into CD space 
     offset the coordinates by (-iposx, -iposy) */
  
  cdCanvasForeground(cdcanvas, CD_RED);
  cdCanvasLine(cdcanvas, 0-iposx, 0-iposy, virtual_width-1-iposx, virtual_height-1-iposy);
  cdCanvasLine(cdcanvas, 0-iposx, virtual_height-1-iposy, virtual_width-1-iposx, 0-iposy);

  return IUP_DEFAULT;
}

static int resize_cb(Ihandle *ih, int w, int h)
{
  cdCanvas *cdcanvas = (cdCanvas*)IupGetAttribute(ih, "_CD_CANVAS");

  /* update CD canvas size */
  cdCanvasActivate(cdcanvas);
 
  /* update page size, it is always the client size of the canvas */
  IupSetfAttribute(ih, "DX", "%d", w);  /* although DX and DY internally are float, we can use integers */
  IupSetfAttribute(ih, "DY", "%d", h);
  
  /* refresh scroolbar in IUP 2.x */
  IupStoreAttribute(ih, "POSX", IupGetAttribute(ih, "POSX"));
  IupStoreAttribute(ih, "POSY", IupGetAttribute(ih, "POSY"));
  return IUP_DEFAULT;
}

static int map_cb(Ihandle *ih)
{
  cdCreateCanvas(CD_IUP, ih);
  return IUP_DEFAULT;
}

static int unmap_cb(Ihandle *ih)
{
  cdCanvas *cdcanvas = (cdCanvas*)IupGetAttribute(ih, "_CD_CANVAS");
  cdKillCanvas(cdcanvas);
  return IUP_DEFAULT;
}

void CanvasScrollbarTest(void)
{
  Ihandle *dlg, *canvas;

  canvas = IupCanvas(NULL);
  IupSetAttribute(canvas, "RASTERSIZE", "300x200"); /* initial size */
  IupSetAttribute(canvas, "SCROLLBAR", "YES");
  IupSetAttribute(canvas, "XMAX", "599");
  IupSetAttribute(canvas, "YMAX", "399");

  IupSetCallback(canvas, "RESIZE_CB",  (Icallback)resize_cb);
  IupSetCallback(canvas, "ACTION",  (Icallback)action);
  IupSetCallback(canvas, "MAP_CB",  (Icallback)map_cb);
                   
  dlg = IupDialog(IupVbox(canvas, NULL));
  IupSetAttribute(dlg, "TITLE", "Scrollbar Test");
  IupSetAttribute(dlg, "MARGIN", "10x10");

  IupMap(dlg);
  IupSetAttribute(canvas, "RASTERSIZE", NULL);  /* release the minimum limitation */
 
  IupShowXY(dlg,IUP_CENTER,IUP_CENTER);
}

#ifndef BIG_TEST
int main(int argc, char* argv[])
{
  IupOpen(&argc, &argv);

  CanvasScrollbarTest();

  IupMainLoop();

  IupClose();

  return EXIT_SUCCESS;
}
#endif
