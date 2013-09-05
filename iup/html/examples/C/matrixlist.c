#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "iup.h"
#include "iupcontrols.h"
#include "cd.h"

static int click(Ihandle *self, int lin, int col, char *status)
{
  char* value = IupGetAttributeId(self, "", lin);
  if (!value) value = "NULL";
  printf("click_cb(%d, %d)\n", lin, col);
  printf("  VALUE%d:%d = %s\n", lin, col, value);
  //IupSetInt(self, "ADDLIN", lin);
  return IUP_DEFAULT;
}


int main(int argc, char **argv)
{
  Ihandle *dlg, *mlist;
  IupOpen(&argc, &argv);       
  IupControlsOpen();
  mlist = IupMatrixList();
  IupSetInt(mlist, "NUMLIN", 10);
  IupSetAttribute(mlist, "SHOWCOLOR", "YES");
  IupSetAttribute(mlist, "ORDERCOLORCOL", "LEFT");
  IupSetAttribute(mlist, "COLOR1", "255 0 0");
  IupSetAttribute(mlist, "COLOR2", "255 255 0");
  //IupSetAttribute(mlist, "COLOR3", "0 255 0");
  IupSetAttribute(mlist, "COLOR4", "0 255 255");
  IupSetAttribute(mlist, "COLOR5", "0 0 255");
  IupSetAttribute(mlist, "COLOR6", "255 0 255");
  IupSetAttribute(mlist, "COLOR7", "255 128 0");
  IupSetAttribute(mlist, "COLOR8", "255 128 128");
  IupSetAttribute(mlist, "COLOR9", "0 255 128");
  IupSetAttribute(mlist, "COLOR10", "128 255 128");
  IupSetAttribute(mlist, "EDIT_MODE", "NO");
  IupSetAttribute(mlist, "1", "A");
  IupSetAttribute(mlist, "2", "B");
  IupSetAttribute(mlist, "3", "C");
  IupSetAttribute(mlist, "4", "D");
  IupSetAttribute(mlist, "5", "E");
  IupSetAttribute(mlist, "6", "F");
  IupSetAttribute(mlist, "7", "G");
  IupSetAttribute(mlist, "8", "H");
  IupSetAttribute(mlist, "9", "I");
  IupSetAttribute(mlist, "10", "J");

  //IupSetCallback(mlist,"CLICK_CB",(Icallback)click);

  dlg = IupDialog(mlist);
  IupSetAttribute(dlg, "TITLE", "IupMatrixList");
  IupShowXY(dlg, IUP_CENTER, IUP_CENTER);
  IupMainLoop();
  IupClose();  
  return EXIT_SUCCESS;
}
