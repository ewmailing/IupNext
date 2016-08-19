
#include <iup.h>

#include "IupPreviewCanvas.h"

#include <cd.h>
#include <cdiup.h>
#include <cdcgm.h>


static int canvas_redraw(Ihandle* ih)
{
  cdCanvas* canvas = (cdCanvas*)IupGetAttribute(ih, "_CD_CANVAS");
  char* filename = IupGetAttribute(ih, "PATHFILE");

  cdCanvasActivate(canvas);
  cdCanvasBackground(canvas, CD_WHITE);
  cdCanvasClear(canvas);

  int w, h;
  cdCanvasGetSize(canvas, &w, &h, 0, 0);

  cdCanvasForeground(canvas, CD_RED);
  cdCanvasLine(canvas, 0, 0, w - 1, h - 1);
  cdCanvasLine(canvas, 0, h - 1, w - 1, 0);

  cdCanvasPlay(canvas, CD_CGM, 0, w - 1, 0, h - 1, filename);

  return IUP_DEFAULT;
}

static int canvas_map(Ihandle* ih)
{
  cdCreateCanvas(CD_IUP, ih);
  return IUP_DEFAULT;
}

static int canvas_unmap(Ihandle* ih)
{
  cdCanvas* canvas = (cdCanvas*)IupGetAttribute(ih, "_CD_CANVAS");
  cdKillCanvas(canvas);
  return IUP_DEFAULT;
}

Ihandle* IupPreviewCanvasCreate()
{
  Ihandle* cnv = IupCanvas(0);

  IupSetCallback(cnv, "ACTION", canvas_redraw);
  IupSetCallback(cnv, "MAP_CB", canvas_map);
  IupSetCallback(cnv, "UNMAP_CB", canvas_unmap);

  return cnv;
}
