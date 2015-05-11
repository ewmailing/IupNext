#include <stdlib.h>
#include <iup.h>

int btn_msg_cb( Ihandle *self )
{
  IupMessage("Hello World Example", "Hello from IUP.");
  /* Returns to main loop */
  return IUP_DEFAULT;
}

int btn_exit_cb( Ihandle *self )
{
  /* Exits the main loop */
  return IUP_CLOSE;
}

int main(int argc, char **argv)
{
  Ihandle *dlg, *button_msg, *button_exit, *vbox;

  IupOpen(&argc, &argv);
  button_msg = IupButton("Message", "button");
  button_exit = IupButton("Close", "button");
  vbox = IupVbox(
    button_msg,
    button_exit,
    NULL
  );
  IupSetAttribute(vbox, "GAP", "5");
  IupSetAttribute(vbox, "MARGIN", "10x10");

  dlg = IupDialog(vbox);
  IupSetAttribute(dlg, "TITLE", "Hello from IUP Tutorial!");

  /* Registers callbacks */
  IupSetCallback( button_msg, "ACTION", (Icallback) btn_msg_cb );
  IupSetCallback( button_exit, "ACTION", (Icallback) btn_exit_cb );

  IupShowXY(dlg, IUP_CENTER, IUP_CENTER);
  IupMainLoop();

  IupClose();

  return EXIT_SUCCESS;
}
