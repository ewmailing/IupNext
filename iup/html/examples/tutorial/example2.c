#include <stdlib.h>
#include <iup.h>

int main(int argc, char **argv)
{
  Ihandle *dlg, *label;

  IupOpen(&argc, &argv);

  label =  IupLabel("Hello!");
  dlg = IupDialog(IupVbox(label, NULL));
  IupSetAttribute(dlg, "TITLE", "Hello from IUP Tutorial!");

  IupShowXY(dlg, IUP_CENTER, IUP_CENTER);

  IupMainLoop();

  IupClose();

  return EXIT_SUCCESS;
}
