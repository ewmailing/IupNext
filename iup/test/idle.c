#include <stdio.h>
#include "iup.h"

static int idle_count = 0;

static int idle(void)
{
  printf("IDLE_ACTION(count = %d)\n", idle_count);
  idle_count++;

//  if (idle_count == 10000)
//    return IUP_IGNORE;

  return IUP_DEFAULT;
}

static int motion_cb(Ihandle* ih)
{
  printf("MOTION_CB()\n");
  if (idle_count > 30000)
    IupSetFunction ("IDLE_ACTION", NULL);
  return IUP_DEFAULT;
}

void main(int argc, char* argv[])
{
  Ihandle* dlg, *canvas;

  IupOpen(&argc, &argv);

  canvas = IupCanvas(NULL);
  IupSetCallback(canvas, "MOTION_CB", motion_cb);
 
  dlg = IupDialog(canvas);
  IupSetAttribute(dlg, "TITLE", "Idle Test");
  IupSetAttribute(dlg, "RASTERSIZE", "500x500");

  IupShowXY(dlg, IUP_CENTER, IUP_CENTER);

  IupSetFunction ("IDLE_ACTION", (Icallback)idle);

  IupMainLoop();
  IupDestroy(dlg);
  IupClose();
}
