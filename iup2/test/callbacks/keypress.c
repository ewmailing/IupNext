
#include <stdio.h>
#include <assert.h>
#include "iup.h"

static int keypress_cb (Ihandle *cv, int key, int press)
{
  if (key > 32 && key < 127) 
    printf("keypress_cb %c %d=%s (%s)\n", (char)key, key, iupKeyEncode(key), press? "press":"release");
  else
    printf("keypress_cb %d=%s (%s)\n", key, iupKeyEncode(key), press? "press":"release");
  return IUP_DEFAULT;
}

static int k_any(Ihandle *cb, int key)
{
  iscntrl(key);
  printf("k_any %d=%s\n", key, iupKeyEncode(key));
  return IUP_DEFAULT;
}

int main(int argc, char **argv)
{
  Ihandle* dlg, *iupcanvas;	

  IupOpen(&argc, &argv);

//  iupcanvas = IupCanvas(NULL);
  iupcanvas = IupText(NULL);
  IupSetAttribute(iupcanvas,IUP_KEYPRESS_CB,"keypress_cb"); 
  IupSetFunction("keypress_cb", (Icallback) keypress_cb);
  
  dlg = IupDialog(iupcanvas);
  IupSetAttribute(dlg, "TITLE", "IupCanvas");
//  IupSetAttribute(dlg, "K_ANY", "k_any");
//  IupSetFunction("k_any", (Icallback) k_any);

  IupShowXY(dlg,100,100);
  
  IupMainLoop();
  IupDestroy(dlg);
  IupClose();
  return 0;
}
