#include <stdlib.h>
#include <stdio.h>
#include <iup.h>

void MultiBoxTest(void)
{
  Ihandle *dlg;
  Ihandle *fr1;
  Ihandle *mbox;

  fr1 = IupFrame
  (
    mbox = IupMultiBox
    (
      IupSetAttributes(IupLabel("1"), ""),
      IupSetAttributes(IupLabel("elem 2"), "FONTSTYLE=Bold"),
      IupSetAttributes(IupLabel("element 3"), "FONTSTYLE=Bold"),

      IupSetAttributes(IupLabel("very long element 4"), "FONTSTYLE=Bold"),
      IupSetAttributes(IupLabel("elem 5"), "XSIZE=50x12"),
      IupSetAttributes(IupButton("element 6", NULL), "XSIZE=50"),

      IupSetAttributes(IupLabel("label 7"), "FONTSTYLE=Bold"),
      IupSetAttributes(IupLabel("label 8"), "XSIZE=x12"),
      IupSetAttributes(IupButton("another button", NULL), "XEXPAND=Horizontal"),

      IupSetAttributes(IupLabel("label"), "FONTSTYLE=Bold"),
      IupSetAttributes(IupLabel("label large"), "XSIZE=x12"),
      IupSetAttributes(IupButton("button large", NULL), ""),
      NULL
    )
  );
  
//  IupSetAttribute(mbox, "ORIENTATION", "VERTICAL");

//  IupSetAttribute(mbox, "SIZE", "70x");

//  IupSetAttribute(mbox, "ALIGNMENTLIN", "ACENTER");
  IupSetAttribute(mbox, "MARGIN", "10x10");
  IupSetAttribute(mbox, "GAPHORIZ", "5");
  IupSetAttribute(mbox, "GAPVERT", "5");

  dlg = IupDialog
  (
    IupHbox
    (
      fr1,
      NULL
    )
  );

  IupSetAttribute(dlg, "TITLE", "IupMultiBox Test");
  IupSetAttribute(dlg, "MARGIN", "10x10");
  IupSetAttribute(fr1, "MARGIN", "0x0");   /* avoid attribute propagation */

  IupMap(dlg);
  IupSetAttribute(dlg, "SIZE", NULL);

  /* Shows dlg in the center of the screen */
  IupShowXY(dlg, IUP_CENTER, IUP_CENTER);
}

#ifndef BIG_TEST
int main(int argc, char* argv[])
{
  IupOpen(&argc, &argv);

  MultiBoxTest();

  IupMainLoop();

  IupClose();

  return EXIT_SUCCESS;
}
#endif
