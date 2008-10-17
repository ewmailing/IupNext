
#include <stdio.h>
#include <stdlib.h>

#include <iup.h>

int button(Ihandle* h)
{
  IupMessage("Message", "Hi");
  return IUP_CLOSE;
}

int main()
{
  Ihandle *dg, *bt;
  IupOpen();
  
  IupSetFunction("button", button);

  bt = IupButton("Test", "button");
  dg = IupDialog(bt);
  IupShow(dg);

  IupMainLoop();
  IupClose();
  return 1;
}
