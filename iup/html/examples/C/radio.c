/*IupRadio Example in C 
Creates a dialog for the user to select his/her gender. In this case, the radio element is essential to prevent the user from selecting both options. */

/*
	Program to exemplify the IupRadio element.
*/
#include <stdio.h>
#include "iup.h"

int main(int argc, char **argv)
{
  Ihandle *male, *female, *exclusive, *frame, *dialog;

  IupOpen(&argc, &argv);

  male	= IupToggle ("Male", "");
  female = IupToggle ("Female", "");

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
  IupSetAttribute(exclusive, IUP_VALUE, "female");
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