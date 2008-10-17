
#include <stdio.h>
#include <assert.h>
#include "iup.h"
#include "iupcontrols.h"

static Ihandle *dg, *A, *label, *box, *C, *B, *box2, *vbox;

static int show_cb()
{
  return IUP_CLOSE;
}

int main(int argc, char **argv)
{
  IupOpen(&argc, &argv);
  IupControlsOpen();

  A = IupHbox(IupButton("Teste", "bt_action"), IupLabel("aaaa"), 
              IupSetAttributes(IupText(""), "EXPAND=YES"), NULL);
  IupSetAttribute(A, "EXPAND", "NO");
  box = IupSbox(A);
  
  IupSetAttribute(box, "DIRECTION", "NORTH");
  //IupSetAttribute(A, IUP_EXPAND, "YES");
  B = IupMultiLine(NULL);
  IupSetAttribute(B, IUP_EXPAND, "YES");
  vbox = IupVbox(B, box, NULL);

//  IupSetFunction("showcb", (Icallback) show_cb);
  C = IupLabel("Label lateral");
  IupSetAttribute(C, IUP_EXPAND, "YES");
  dg = IupDialog(IupHbox(vbox, C, NULL));
  IupSetAttribute(dg, IUP_SHOW_CB, "showcb");

  IupPopup(dg, IUP_CENTER, IUP_CENTER);

  IupMainLoop();
  IupDestroy(dg);
  IupClose();
  return 1;
}
