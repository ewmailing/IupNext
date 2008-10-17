
#include <stdlib.h>
#include <stdio.h>
#include "iup.h"
#include "iupval.h"
#include "iupdial.h"

int quit_cb(void)
{
  return IUP_CLOSE;
}

int main(int argc, char **argv)
{
  char *error = NULL;
  Ihandle *dialog, *val;

  IupOpen(&argc, &argv);
  IupValOpen();
  IupDialOpen();

  error = IupLoad("..\\tests\\led\\val.led");
  if(error)
  {
    printf("%s\n", error);
    getchar();
    exit(1);
  }

  dialog = IupGetHandle("dlg");
  printf("%s\n", IupGetClassName(dialog));
  val = IupGetHandle("valv");
  printf("%p\n", val);
  printf("%s\n", IupGetClassName(val));
  IupShow(dialog);
  //printf("%s\n", IupGetClassName(dialog));

  IupMainLoop();
  IupClose();

  return 0;
}


