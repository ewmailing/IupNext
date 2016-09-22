#include <stdlib.h>
#include <stdio.h>
#include "iup.h"

void FlatFrameTest(void)
{
  Ihandle *dlg, *frame1, *frame2;

  frame1 = IupFlatFrame
          (
            IupVbox
            (
              IupLabel("Label1"),
              IupSetAttributes(IupLabel("Label2"), "SIZE=70x"),
              IupLabel("Label3"),
              NULL
            )
          );

  frame2 = IupFlatFrame
          (
            IupVbox
            (
              IupSetAttributes(IupLabel("Label4"), "EXPAND=HORIZONTAL"),
              IupLabel("Label5"),
              IupLabel("Label6"),
              NULL
            )
          );

  IupSetAttribute(frame1, "TITLE", "Title Text");
  IupSetAttribute(frame1, "MARGIN", "0x0");
//  IupSetAttribute(frame1, "FGCOLOR", "255 0 0");
  IupSetAttribute(frame1, "BACKCOLOR", "0 128 0");
  IupSetAttribute(frame1, "FRAMECOLOR", "255 0 0");
  IupSetAttribute(frame1, "TITLELINECOLOR", "128 0 0");
  IupSetAttribute(frame1, "TITLEBGCOLOR", "0 200 0");
  IupSetAttribute(frame1, "TITLECOLOR", "0 0 255");
//  IupSetAttribute(frame1, "TITLELINE", "No");
  IupSetAttribute(frame1, "TITLELINEWIDTH", "5");
  IupSetAttribute(frame1, "FRAMEWIDTH", "5");
  IupSetAttribute(frame1, "FRAMESPACE", "10");

  IupSetAttribute(frame2, "MARGIN", "0x0");
  //IupSetAttribute(frame2, "BACKCOLOR", "0 128 0");
  dlg = IupDialog(IupHbox(frame1, frame2, NULL));

  IupSetAttribute(dlg, "TITLE", "IupFlatFrame Test");
  IupSetAttribute(dlg, "MARGIN", "10x10");
  IupSetAttribute(dlg, "GAP", "5");
  IupSetAttribute(dlg, "FONTSIZE", "14");
//  IupSetAttribute(dlg, "RASTERSIZE", "300x200");
//  IupSetAttribute(dlg, "BGCOLOR", "128 0 0");

  IupShow(dlg);
  printf("RASTERSIZE(%s)\n", IupGetAttribute(frame1, "RASTERSIZE"));
  printf("CLIENTSIZE(%s)\n", IupGetAttribute(frame1, "CLIENTSIZE"));
  printf("RASTERSIZE(%s)\n", IupGetAttribute(frame2, "RASTERSIZE"));
  printf("CLIENTSIZE(%s)\n", IupGetAttribute(frame2, "CLIENTSIZE"));
}

#ifndef BIG_TEST
int main(int argc, char* argv[])
{
  IupOpen(&argc, &argv);

  FlatFrameTest();

  IupMainLoop();

  IupClose();

  return EXIT_SUCCESS;
}
#endif
