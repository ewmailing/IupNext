#include <stdlib.h>
#include <stdio.h>
#include "iup.h"
#include "cd.h"

//#define USE_IUPDRAW
#ifdef USE_IUPDRAW
#include "iupdraw_cd.h"
#else
#include "cdiup.h"
#endif


static int redraw_cb(Ihandle *ih)
{
  int width, height;
  cdCanvas *cdcanvas = (cdCanvas*)IupGetAttribute(ih, "_APP_CDCANVAS");

  cdCanvasActivate(cdcanvas);

  cdCanvasBackground(cdcanvas, CD_WHITE);
  cdCanvasClear(cdcanvas);

  cdCanvasForeground(cdcanvas, CD_RED);
  cdCanvasGetSize(cdcanvas, &width, &height, NULL, NULL);
  cdCanvasLine(cdcanvas, 0, 0, width-1, height-1);
  cdCanvasLine(cdcanvas, 0, height-1, width-1, 0);

  cdCanvasFlush(cdcanvas);

  return IUP_DEFAULT;
}

static int map_cb(Ihandle *ih)
{
  cdCanvas *cdcanvas;

#ifdef USE_IUPDRAW
  cdcanvas = cdCreateCanvas(CD_IUPDRAW, ih);
#else
  cdcanvas = cdCreateCanvas(CD_IUP, ih);
#endif
  if (!cdcanvas)
    return IUP_DEFAULT;

  IupSetAttribute(ih, "_APP_CDCANVAS", (char*)cdcanvas);

  return IUP_DEFAULT;
}

static int unmap_cb(Ihandle *ih)
{
  cdCanvas *cdcanvas = (cdCanvas*)IupGetAttribute(ih, "_APP_CDCANVAS");

  if (cdcanvas)
    cdKillCanvas(cdcanvas);

  return IUP_DEFAULT;
}

void CanvasCDSimpleTest(void)
{
  Ihandle *dlg, *canvas;

  canvas = IupCanvas(NULL);
  IupSetAttribute(canvas, "RASTERSIZE", "300x200"); /* initial size */

  IupSetCallback(canvas, "ACTION",  (Icallback)redraw_cb);
  IupSetCallback(canvas, "MAP_CB",  (Icallback)map_cb);
  IupSetCallback(canvas, "UNMAP_CB",  (Icallback)unmap_cb);
                   
  dlg = IupDialog(canvas);
  IupSetAttribute(dlg, "TITLE", "CD Simple Buffer Test");

  IupMap(dlg);
  IupSetAttribute(canvas, "RASTERSIZE", NULL);  /* release the minimum limitation */
 
  IupShowXY(dlg,IUP_CENTER,IUP_CENTER);
}

#ifndef BIG_TEST
int main(int argc, char* argv[])
{
  IupOpen(&argc, &argv);

  CanvasCDSimpleTest();

  IupMainLoop();

  IupClose();

  return EXIT_SUCCESS;
}
#endif
