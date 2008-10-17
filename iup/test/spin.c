#include <stdlib.h>
#include <stdio.h>

#include "iup.h"
#include "iupspin.h"


static int spin_cb(Ihandle* ih, int inc)
{
  (void)ih;
  printf("SPIN_CB(%d)\n", inc);
  return IUP_DEFAULT;
}

int main(int argc, char* argv[])
{
  Ihandle *dlg, *spinbox;

  IupOpen(&argc, &argv);

  spinbox = IupSpinbox(IupSetAttributes(IupText(NULL), "SIZE=50x"));

  IupSetCallback(spinbox, "SPIN_CB", (Icallback)spin_cb);

  dlg = IupDialog(IupVbox(spinbox, NULL));
  IupSetAttribute(dlg, "MARGIN", "10x10");

  IupSetAttribute(dlg, "TITLE", "IupSpin Test");
  IupShow(dlg);

  IupMainLoop();

  IupDestroy(dlg);

  IupClose();  
  return 0;
}
