
#include <stdio.h>
#include <assert.h>
#include "iup.h"
#include "iupcontrols.h"

int show_cb()
{
  return IUP_DEFAULT;
}

int getfocus_cb(Ihandle *d)
{
  printf("GETFOCUS no %s\n", IupGetClassName(d));
  return IUP_DEFAULT;
}

int mm(Ihandle *n, int x, int y)
{
  char v[100];
  sprintf(v, "%dx%d", x, y);
  IupSetAttribute(n, IUP_TIP, v);
  return IUP_DEFAULT;
}

int button_cb()
{
  Ihandle *dlg, *cv = IupCanvas("");
  IupSetAttribute(cv, IUP_SIZE, "100x100");
  IupSetAttribute(cv, IUP_MOTION_CB, "mm");
  IupSetAttribute(cv, IUP_TIP, "SEGUNDO ELEMENT");
  dlg = IupDialog(IupVbox(IupText(""), cv, NULL));
  IupSetAttribute(dlg, IUP_SHOW_CB, "showcb");
  IupSetAttribute(dlg, IUP_GETFOCUS_CB, "getfocus_cb");
  IupPopup(dlg, IUP_CENTER, IUP_CENTER);
  IupSetAttribute(dlg, IUP_SHOW_CB, NULL);
  printf("saiu do popup\n");
  return IUP_DEFAULT;
}

int main(int argc, char **argv)
{
  Ihandle *dg, *A, *B;

  IupOpen(&argc, &argv);
  IupControlsOpen();

  IupSetFunction("showcb", (Icallback) show_cb);
  IupSetFunction("buttoncb", (Icallback) button_cb);
  IupSetFunction("getfocus_cb", (Icallback) getfocus_cb);
  IupSetFunction("mm", (Icallback) mm);
  
  A = IupButton("Open Popup", "buttoncb");
  B = IupButton("Dummy", "");
  IupSetAttribute(A, IUP_TIP, "TIP DO PRIMEIRO ELEMENTO");
  dg = IupDialog(IupVbox(A, B, NULL));
  IupPopup(dg, IUP_CENTER, IUP_CENTER);

  IupMainLoop();
  IupClose();
  return 1;
}

