#include <stdlib.h>
#include <stdio.h>
#include "iup.h"
#include "iupcontrols.h"

static int notify(Ihandle *ih, int status)
{
  if(!status)
    printf("Detachment in progress...\n");
  else if(status == 1)
    printf("Detachment completed!\n");
  else
    printf("Detachment canceled!\n");

  (void)ih;
  return IUP_DEFAULT;
}

int main(int argc, char **argv)
{
  Ihandle *dlg, *bt, *box, *lbl, *ml, *hbox;
  IupOpen(&argc, &argv);

  bt = IupButton("Detach me!", NULL);
  IupSetAttribute(bt, "EXPAND", "YES");

  ml = IupMultiLine(NULL);
  IupSetAttribute(ml, "EXPAND", "YES");
  IupSetAttribute(ml, "VISIBLELINES", "5");
  hbox = IupHbox(bt, ml, NULL);

  box = IupDbox(hbox);
  IupSetAttribute(box, "ORIENTATION", "VERTICAL");
  IupSetCallback(box, "DETACH_CB", (Icallback)notify);

  lbl = IupLabel("Label");
  IupSetAttribute(lbl, "EXPAND", "VERTICAL");

  dlg = IupDialog(IupVbox(box, lbl, NULL));
  IupSetAttribute(dlg, "TITLE", "IupDbox Example");
  IupSetAttribute(dlg, "MARGIN", "10x10");
  IupSetAttribute(dlg, "GAP", "10");
  IupSetAttribute(dlg, "RASTERSIZE", "250x250");

  IupShow(dlg);

  IupMainLoop();
  IupClose();
  return EXIT_SUCCESS;
}
