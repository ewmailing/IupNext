
#include <stdio.h>
#include <stdlib.h>
#include "iup.h"

Ihandle *dg, *cv, *bt, *tx;

int btcb()
{
  printf("button: x=%s\n", IupGetAttribute(bt, IUP_X));
  printf("button: y=%s\n", IupGetAttribute(bt, IUP_Y));
  printf("text  : x=%s\n", IupGetAttribute(tx, IUP_X));
  printf("text  : y=%s\n", IupGetAttribute(tx, IUP_Y));
  printf("dialog: x=%s\n", IupGetAttribute(dg, IUP_X));
  printf("dialog: y=%s\n", IupGetAttribute(dg, IUP_Y));
  return IUP_DEFAULT;
}

int motion()
{
  char v[100];
  int x;
  int y;
  char *size = IupGetAttribute(cv, "RASTERSIZE");
  printf("size=%s\n", size);
  sscanf(size, "%dx%d", &x, &y);
  x /= 2; y /= 2;
  x += atoi(IupGetAttribute(cv, IUP_X));
  y += atoi(IupGetAttribute(cv, IUP_Y));
  sprintf(v, "%dx%d", x, y);
  printf("setando %dx%d\n", x, y);
  IupSetGlobal("CURSORPOS", v);
  return IUP_DEFAULT;
}

int main(int argc, char **argv)
{
  //int i, j;
  IupOpen(&argc, &argv);

  IupSetFunction("btcb", (Icallback) btcb);
  IupSetFunction("motion", (Icallback) motion);
  bt = IupButton("Size", "btcb");
  tx = IupText("Text");
  cv = IupCanvas("");
  IupSetAttribute(cv, "SIZE", "300x300");
  IupSetAttribute(cv, "MOTION_CB", "motion");
  dg = IupDialog(IupVbox(IupHbox(bt, tx, NULL), cv, NULL));
  IupShow(dg);
  /*
  for(i=0; i<800; i+=10)
    for(j=0; j<800; j+=10)
    {
      char v[100];
      sprintf(v, "%dx%d", i, j);
      printf("%s\n", v);
      IupSetGlobal("CURSORPOS", v);
      sleep(1);
    }
    */
  IupMainLoop();

  IupClose();
  return 0;
}

