
#include <stdio.h>
#include <assert.h>
#include "iup.h"
#include "iupcontrols.h"

Ihandle *dg, *bt, *label, *box, *lb, *ml, *box2, *vbox;

int main(int argc, char **argv)
{
  IupOpen(&argc, &argv);
  IupControlsOpen();

  bt = IupButton("Test", "");
  IupSetAttribute(bt, "EXPAND", "YES");

  box = IupSbox(bt);
  IupSetAttribute(box, "DIRECTION", "SOUTH");

  ml = IupMultiLine(NULL);
  IupSetAttribute(ml, IUP_EXPAND, "YES");
  vbox = IupVbox(box, ml, NULL);

  lb = IupLabel("Label");
  IupSetAttribute(lb, IUP_EXPAND, "YES");

  dg = IupDialog(IupHbox(vbox, IupFrame(lb), NULL));
  IupSetAttribute(dg, IUP_MARGIN, "10x20");

  //IupSetAttribute(dg,"COMPOSITED", "YES");
  //IupSetAttribute(dg,"LAYERED", "YES");
  //IupSetAttribute(dg,"LAYERALPHA", "192");

  IupShow(dg);

  IupMainLoop();
  IupDestroy(dg);
  IupControlsClose();
  IupClose();
  return 1;
}
