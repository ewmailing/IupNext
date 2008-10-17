
#include <stdio.h>
#include <assert.h>
#include "iup.h"
#include "iupsbox.h"

static Ihandle *dg, *bt, *box, *cve, *cvw, *cvs, *cvn;

static int bt_action(Ihandle *bt)
{
  return IUP_DEFAULT;
}

int main(int argc, char **argv)
{
  IupOpen(&argc, &argv);
  IupSboxOpen();

  IupSetFunction("bt_action", (Icallback) bt_action);

  bt = IupButton("Teste", "bt_action");
  IupSetAttribute(bt, IUP_EXPAND, "YES");
  cve = IupSetAttributes(IupCanvas(NULL), "BORDER=YES");
  cvw = IupSetAttributes(IupCanvas(NULL), "BORDER=YES");
  cvs = IupSetAttributes(IupCanvas(NULL), "BORDER=YES");
  cvn = IupSetAttributes(IupCanvas(NULL), "BORDER=YES");

  box = IupSbox(bt);
  IupSetAttribute(box, "DIRECTION", "EAST");

  dg = IupDialog(IupVbox(cvn, IupHbox(cvw, box, cve, NULL), cvs, NULL));
  IupShow(dg);
  IupMainLoop();
  IupDestroy(dg);
  IupClose();
  return 1;
}
