#include <stdlib.h>
#include <stdio.h>

#include "iup.h"


static int value_cb(Ihandle* ih)
{
  printf("VALUE_CB(%s)\n", IupGetAttribute(ih, "VALUE"));
  return IUP_DEFAULT;
}

void CalendarTest(void)
{
  Ihandle *dlg, *box;

  box = IupVbox(
    IupSetCallbacks(IupCalendar(), "VALUECHANGED_CB", value_cb, NULL),
    IupSetCallbacks(IupDatePick(), "VALUECHANGED_CB", value_cb, NULL),
    NULL);
    IupSetAttribute(box, "MARGIN", "10x10");
  IupSetAttribute(box, "GAP", "10");

  dlg = IupDialog(box);
  IupSetAttribute(box, "FONTSIZE", "36");

  IupSetAttribute(dlg, "TITLE", "IupCalendar Test");
  IupShow(dlg);
}

#ifndef BIG_TEST
int main(int argc, char* argv[])
{
  IupOpen(&argc, &argv);

  CalendarTest();

  IupMainLoop();

  IupClose();

  return EXIT_SUCCESS;
}
#endif
