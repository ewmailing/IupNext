#include <stdlib.h>
#include <stdio.h>

#include "iup.h"


static int value_cb(Ihandle* ih)
{
  printf("VALUE_CB(%s)\n", IupGetAttribute(ih, "VALUE"));
  return IUP_DEFAULT;
}

static int set_value(Ihandle* ih)
{
  IupSetAttribute(IupGetBrother(ih), "VALUE", "11/07/1970");
  IupSetAttribute(IupGetBrother(IupGetBrother(ih)), "VALUE", "11/07/1970");
  return IUP_DEFAULT;
}

void CalendarTest(void)
{
  Ihandle *dlg, *box;

  box = IupVbox(
    IupSetCallbacks(IupButton("Set Value", NULL), "ACTION", set_value, NULL),
    IupSetAttributes(IupSetCallbacks(IupCalendar(), "VALUECHANGED_CB", value_cb, NULL), "BACKCOLOR=\"128 0 32\", BORDER=10"),
    IupSetAttributes(IupSetCallbacks(IupDatePick(), "VALUECHANGED_CB", value_cb, NULL), "ZEROPRECED=Yes, XMONTHSHORTNAMES=Yes"),
    NULL);
    IupSetAttribute(box, "MARGIN", "10x10");
  IupSetAttribute(box, "GAP", "10");

  dlg = IupDialog(box);
//  IupSetAttribute(box, "FONTSIZE", "36");

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
