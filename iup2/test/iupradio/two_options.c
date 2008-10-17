
#include <stdio.h>
#include "iup.h"

int action (Ihandle* self, int v)
{
  printf("%s - %d\n", IupGetAttribute(self, IUP_TITLE), v);
  return IUP_DEFAULT;
}

int set(Ihandle* self)
{
  Ihandle* tgl = IupGetHandle("tg1");
  IupSetAttribute(tgl, IUP_VALUE, IUP_ON);
  return IUP_DEFAULT;
}

int main(int argc, char **argv)
{
  Ihandle *male, *female, *exclusive, *frame, *dialog;

  IupOpen(&argc, &argv);

  male  = IupToggle ("Male", "action");
  female = IupToggle ("Female", "action");
  IupSetHandle("tg1", male);

  IupSetFunction("action", (Icallback)action);
  IupSetFunction("set", (Icallback)set);

//  IupSetAttribute(male, IUP_VALUE, IUP_ON);
  IupSetAttribute(female, IUP_VALUE, IUP_ON);

  exclusive = IupRadio
  (
    IupVbox
    (
      male,
      female,
      NULL
    )
  );
  IupSetHandle("male", male);
  IupSetHandle("female", female);
//  IupSetAttribute(exclusive, IUP_VALUE, "female");
  IupSetAttribute(exclusive, IUP_TIP, "Two state button - Exclusive - RADIO");

  frame = IupFrame
  (
    exclusive
  );
  IupSetAttribute (frame, IUP_TITLE, "Gender");

  dialog = IupDialog
  (
    IupHbox
    (
      IupFill(),
      frame,
      IupFill(),
      IupButton("set", "set"),
      NULL
    )
  );

  IupSetAttributes(dialog, "SIZE=140, TITLE=IupRadio, RESIZE=NO, MINBOX=NO, MAXBOX=NO");

  IupShow(dialog);
  IupMainLoop();
  IupDestroy(dialog);
  IupClose();

  return 0; 
}


