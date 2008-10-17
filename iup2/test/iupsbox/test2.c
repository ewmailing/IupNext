/*
#include <stdio.h>
#include <assert.h>
#include "iup.h"
#include "sbox.h"

Ihandle *dg, *bt, *label, *box;

int bt_action(Ihandle *bt)
{
  printf("size=%s\nx=%s  y=%s\n", 
    IupGetAttribute(bt, IUP_SIZE), 
    IupGetAttribute(bt, IUP_X), 
    IupGetAttribute(bt, IUP_Y));
  return IUP_DEFAULT;
}

int main(int argc, char **argv)
{
  IupOpen(&argc, &argv);
  IupSboxOpen();

  IupSetFunction("bt_action", (Icallback) bt_action);
  bt = IupButton("Teste", "bt_action");
  //IupSetAttribute(bt, "EXPAND", "YES");

  label = IupMultiLine("seila");
  IupSetAttribute(label, "EXPAND", "YES");
  box = IupSbox(label);
  dg = IupDialog(IupHbox(IupVbox(bt, box, NULL), IupButton("dummy", ""), NULL));
  IupSetAttribute(dg, "SIZE", "200x200");

  //IupSetAttribute(dg, IUP_SIZE, "300x300");
  IupShow(dg);

  IupMainLoop();
  IupClose();
  return 1;
}
*/

#include <stdio.h>
#include <assert.h>
#include "iup.h"
#include "iupcontrols.h"

static Ihandle *dg, *A, *label, *box, *C, *B, *box2, *vbox;

static Ihandle* createtree(void)
{
  Ihandle* tree = IupTree();

  IupSetAttributes(tree,"FONT=COURIER_NORMAL_10, \
                         NAME=Figures, \
                         ADDBRANCH=3D, \
                         ADDBRANCH=2D, \
                         ADDLEAF1=trapeze, \
                         ADDBRANCH1=parallelogram, \
                         ADDLEAF2=diamond, \
                         ADDLEAF2=square, \
                         ADDBRANCH4=triangle, \
                         ADDLEAF5=scalenus, \
                         ADDLEAF5=isoceles, \
                         ADDLEAF5=equilateral, \
                         VALUE=6, \
                         CTRL=NO, \
                         SHIFT=NO, \
                         ADDEXPANDED=NO");
  IupSetAttribute(tree, IUP_RASTERSIZE, "200x200");
  IupSetAttribute(tree, IUP_REDRAW, IUP_YES);
  return tree;
}

static int bt_action(Ihandle *bt)
{
  return IUP_DEFAULT;
}

int main(int argc, char **argv)
{
  IupOpen(&argc, &argv);
  IupControlsOpen();

  IupSetFunction("bt_action", (Icallback) bt_action);

  A = IupFrame(createtree()); //IupButton("Teste", "bt_action");
  IupSetAttribute(A, "EXPAND", "YES");
  box = IupSbox(A);
  IupSetAttribute(box, "DIRECTION", "EAST");
  //IupSetAttribute(A, IUP_EXPAND, "YES");
  B = IupCanvas(NULL);
  IupSetAttribute(B, IUP_EXPAND, "YES");
  IupSetAttribute(B, IUP_BGCOLOR, "0 255 0");
  vbox = IupHbox(box, B, NULL);
  IupSetAttribute(vbox, IUP_EXPAND, "YES");

  C = IupLabel("Label inferior");
  IupSetAttribute(C, IUP_EXPAND, "YES");
  //#IupSetAttribute(C, IUP_EXPAND, "NO"); //# testar
  dg = IupDialog(IupVbox(vbox, C, NULL));

  IupShow(dg);
  IupMainLoop();
  IupDestroy(dg);
  IupControlsClose();
  IupClose();
  return 1;
}
