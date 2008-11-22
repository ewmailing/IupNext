
#include <stdio.h>
#include <assert.h>
#include "iup.h"
#include "iupcontrols.h"

int main(int argc, char **argv)
{
  Ihandle *dg, *bt, *label, *box, *lb, *ml, *box2, *vbox;
  IupOpen(&argc, &argv);
  IupControlsOpen();

  bt = IupButton("Test", "");
  IupSetAttribute(bt, "EXPAND", "YES");
  box = IupSbox(bt);
  IupSetAttribute(box, "DIRECTION", "SOUTH");
  IupSetAttribute(box, "COLOR", "0 0 255");

  ml = IupMultiLine(NULL);
  IupSetAttribute(ml, "EXPAND", "YES");
  vbox = IupVbox(box, ml, NULL);

  lb = IupLabel("Label");
  IupSetAttribute(lb, "EXPAND", "YES");
  dg = IupDialog(IupHbox(vbox, lb, NULL));

  IupShow(dg);

  IupMainLoop();
  IupDestroy(dg);
  IupControlsClose() ;
  IupClose();
  return 1;
}
