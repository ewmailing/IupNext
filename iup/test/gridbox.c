#include <stdlib.h>
#include <stdio.h>
#include <iup.h>

void GridBoxTest(void)
{
  Ihandle *dlg;
  Ihandle *fr1;
  Ihandle *gbox;

  fr1 = IupFrame
  (
    gbox = IupGridBox
    (
      IupSetAttributes(IupLabel("lbl"), "XSIZE=50x12"),
      IupSetAttributes(IupButton("but", NULL), "XSIZE=50"),
      IupSetAttributes(IupLabel("label"), "XSIZE=x12"),
      IupSetAttributes(IupButton("button", NULL), "XEXPAND=Horizontal"),
      IupSetAttributes(IupLabel("label large"), "XSIZE=x12"),
      IupSetAttributes(IupButton("button large", NULL), ""),
      NULL
    )
  );
  
  IupSetAttribute(gbox, "SIZECOL", "1");
  IupSetAttribute(gbox, "SIZELIN", "2");
  IupSetAttribute(gbox, "NUMDIV", "2");
//  IupSetAttribute(gbox, "ORIENTATION", "VERTICAL");
//  IupSetAttribute(gbox, "NUMDIV", "2");
//  IupSetAttribute(gbox, "NUMDIV", "AUTO");

  IupSetAttribute(gbox, "ALIGNMENTLIN", "ACENTER");
  IupSetAttribute(gbox, "MARGIN", "10x10");
  IupSetAttribute(gbox, "GAPLIN", "5");
  IupSetAttribute(gbox, "GAPCOL", "5");

//  IupSetAttribute(gbox, "HOMOGENEOUSLIN", "YES");  /* same space in the normal direction */
//  IupSetAttribute(gbox, "EXPANDCHILDREN", "YES");  /* expand size in the oposite direction */
//  IupSetAttribute(gbox, "NORMALIZESIZE", "BOTH");  /* same size, hor and/or vert */

  dlg = IupDialog
  (
    IupHbox
    (
      fr1,
      NULL
    )
  );

  IupSetAttribute(dlg, "TITLE", "IupGridBox Test");
  IupSetAttribute(dlg, "MARGIN", "10x10");
  IupSetAttribute(fr1, "MARGIN", "0x0");   /* avoid attribute propagation */

  /* Shows dlg in the center of the screen */
  IupShowXY(dlg, IUP_CENTER, IUP_CENTER);
}

#ifndef BIG_TEST
int main(int argc, char* argv[])
{
  IupOpen(&argc, &argv);

  GridBoxTest();

  IupMainLoop();

  IupClose();

  return EXIT_SUCCESS;
}
#endif
