#include <stdio.h>
#include "iup.h"
#include "cd.h"
#include "cdiup.h"
#include "cddbuf.h"



void appUpdateRender(cdCanvas *cddbuffer)
{
  int width, height;

  cdCanvasBackground(cddbuffer, CD_WHITE);
  cdCanvasClear(cddbuffer);

  cdCanvasForeground(cddbuffer, CD_RED);
  cdCanvasGetSize(cddbuffer, &width, &height, NULL, NULL);
  cdCanvasLine(cddbuffer, 0, 0, width-1, height-1);
  cdCanvasLine(cddbuffer, 0, height-1, width-1, 0);
}

int redraw_cb(Ihandle *ih)
{
  cdCanvas *cddbuffer = (cdCanvas*)IupGetAttribute(ih, "_APP_CDDBUFFER");
  if (!cddbuffer)
    return IUP_DEFAULT;

  /* update display */
  cdCanvasFlush(cddbuffer);
  return IUP_DEFAULT;
}

int resize_cb(Ihandle *ih)
{
  cdCanvas *cddbuffer = (cdCanvas*)IupGetAttribute(ih, "_APP_CDDBUFFER");
  if (!cddbuffer)
  {
    cdCanvas *cdcanvas = (cdCanvas*)IupGetAttribute(ih, "_APP_CDCANVAS");

    /* update canvas size */
    cdCanvasActivate(cdcanvas);

    /* this can fail if canvas size is zero */
    cddbuffer = cdCreateCanvas(CD_DBUFFER, cdcanvas);
  }

  if (!cddbuffer)
    return IUP_DEFAULT;

  /* update size */
  cdCanvasActivate(cddbuffer);

  /* update render */
  appUpdateRender(cddbuffer);

  return IUP_DEFAULT;
}

int map_cb(Ihandle *ih)
{
  cdCanvas *cddbuffer, *cdcanvas;

  cdcanvas = cdCreateCanvas(CD_IUP, ih);
  if (!cdcanvas)
    return IUP_ERROR;

  /* this can fail if canvas size is zero */
  cddbuffer = cdCreateCanvas(CD_DBUFFER, cdcanvas);

  IupSetAttribute(ih, "_APP_CDCANVAS", (char*)cdcanvas);
  IupSetAttribute(ih, "_APP_CDDBUFFER", (char*)cddbuffer);

  return IUP_DEFAULT;
}

int unmap_cb(Ihandle *ih)
{
  cdCanvas *cddbuffer = (cdCanvas*)IupGetAttribute(ih, "_APP_CDDBUFFER");
  cdCanvas *cdcanvas = (cdCanvas*)IupGetAttribute(ih, "_APP_CDCANVAS");

  if (cddbuffer)
    cdKillCanvas(cddbuffer);

  if (cdcanvas)
    cdKillCanvas(cdcanvas);

  return IUP_DEFAULT;
}

int main(int argc, char* argv[])
{
  Ihandle *dlg, *canvas;

  IupOpen(&argc, &argv);

  canvas = IupCanvas(NULL);
  IupSetAttribute(canvas, "RASTERSIZE", "300x200"); /* initial size */

  IupSetCallback(canvas, "RESIZE_CB",  (Icallback)resize_cb);
  IupSetCallback(canvas, "ACTION",  (Icallback)redraw_cb);
  IupSetCallback(canvas, "MAP_CB",  (Icallback)map_cb);
  IupSetCallback(canvas, "UNMAP_CB",  (Icallback)unmap_cb);
                   
  dlg = IupDialog(canvas);
  IupSetAttribute(dlg, "TITLE", "CD Double Buffer Test");

  IupMap(dlg);
  IupSetAttribute(canvas, "RASTERSIZE", NULL);  /* release the minimum limitation */
 
  IupShowXY(dlg,IUP_CENTER,IUP_CENTER);

  IupMainLoop();

  IupDestroy(dlg);

  IupClose();

  return 0;
}
