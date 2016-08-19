
#include <iup.h>

#include "IupPreviewCanvas.h"

#include <cd.h>
#include <cdiup.h>
#include <cdcgm.h>


static int canvas_redraw(Ihandle* ih)
{
  cdCanvas* canvas = (cdCanvas*)IupGetAttribute(ih, "_CD_CANVAS");
  char* filename = IupGetAttribute(ih, "PATHFILE");

  cdCanvasPlay(canvas, CD_CGM, 0, 0, 0, 0, 0);

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
