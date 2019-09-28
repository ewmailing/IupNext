#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include <iup.h>

void ClassInfo(void)
{
  Ihandle* dlg = IupClassInfoDialog(NULL);
  IupPopup(dlg, IUP_CENTER, IUP_CENTER);
  IupDestroy(dlg);
}

#ifndef BIG_TEST
int main(int argc, char* argv[])
{
  IupOpen(&argc, &argv);

  ClassInfo();

  IupMainLoop();

  IupClose();

  return EXIT_SUCCESS;
}
#endif
