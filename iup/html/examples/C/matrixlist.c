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
  //IupSetInt(self, "DELLIN", lin);
  return IUP_DEFAULT;
}


int main(int argc, char **argv)
{
  Ihandle *dlg, *mlist;
  IupOpen(&argc, &argv);       
  IupControlsOpen();

  mlist = IupMatrixList();
  IupSetInt(mlist, "NUMLIN", 10);
  IupSetInt(mlist, "NUMLIN_VISIBLE", 5);
  IupSetAttribute(mlist, "COLUMNORDER", "LABEL:COLOR");

  /* Bluish style */
  IupSetAttribute(mlist, "TITLE", "Test");
//  IupSetAttribute(mlist, "BGCOLOR", "220 230 240");
//  IupSetAttribute(mlist, "FRAMECOLOR", "120 140 160");
//  IupSetAttribute(mlist, "BGCOLOR0:*", "120 140 160");
//  IupSetAttribute(mlist, "FGCOLOR0:1", "255 255 255");

  IupSetAttribute(mlist, "1", "AAA");
  IupSetAttribute(mlist, "2", "BBB");
  IupSetAttribute(mlist, "3", "CCC");
  IupSetAttribute(mlist, "4", "DDD");
  IupSetAttribute(mlist, "5", "EEE");
  IupSetAttribute(mlist, "6", "FFF");
  IupSetAttribute(mlist, "7", "GGG");
  IupSetAttribute(mlist, "8", "HHH");
  IupSetAttribute(mlist, "9", "III");
  IupSetAttribute(mlist, "10","JJJ");

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
//  IupSetAttribute(mlist, "EDIT_MODE", "NO");

//  IupSetCallback(mlist,"CLICK_CB",(Icallback)click);

  dlg = IupDialog(IupVbox(mlist, NULL));
  IupSetAttribute(dlg, "TITLE", "IupMatrixList");
  IupSetAttribute(dlg, "MARGIN", "10x10");
  IupShowXY(dlg, IUP_CENTER, IUP_CENTER);

  IupMainLoop();
  IupClose();  
  return EXIT_SUCCESS;
}
