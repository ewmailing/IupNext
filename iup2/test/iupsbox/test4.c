
#include <stdio.h>
#include <assert.h>
#include "iup.h"
#include "iupsbox.h"

static Ihandle *dg, *A, *label, *box, *C, *B, *box2, *vbox, *D, *box3, *E;

static int bt_action(Ihandle *bt)
{
  return IUP_DEFAULT;
}

int main(int argc, char **argv)
{
  IupOpen(&argc, &argv);
  IupSboxOpen();

  IupSetFunction("bt_action", (Icallback) bt_action);

  A = IupButton("Teste", "bt_action");
  IupSetAttribute(A, "EXPAND", "YES");
  box = IupSbox(A);
  IupSetHandle("A", A);
  IupSetAttribute(box, "DIRECTION", "EAST");
  B = IupCanvas(NULL);
  IupSetAttribute(B, IUP_EXPAND, "YES");

  C = IupMultiLine("");
  IupSetAttribute(C, "EXPAND", "YES");
  box2 = IupSbox(C);
  IupSetHandle("C", C);
  IupSetAttribute(box2, "SIZE", "200x100");
  IupSetAttribute(box2, "DIRECTION", "WEST");

  vbox = IupHbox(box, B, box2, NULL);
  IupSetAttribute(vbox, IUP_EXPAND, "YES");

  D = IupLabel("Label inferior");
  IupSetAttribute(D, IUP_EXPAND, "YES");
  box3 = IupSbox(D);
  IupSetHandle("D", D);
  IupSetAttribute(box3, "DIRECTION", "NORTH");

  E = IupSbox(IupButton("botao superior", ""));
  IupSetHandle("E", E);
  IupSetAttribute(E, "DIRECTION", "SOUTH");
  dg = IupDialog(IupVbox(E, vbox, box3, NULL));
  IupSetAttribute(dg, IUP_SIZE, "400x400");

  IupShow(dg);
  IupMainLoop();
  IupDestroy(dg);
  IupClose();
  return 1;
}
