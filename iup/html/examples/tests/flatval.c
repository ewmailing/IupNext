#include <stdlib.h>
#include <stdio.h>

#include "iup.h"
#include "iupkey.h"


static Ihandle *lbl_h=NULL, *lbl_v=NULL;

static void print_value(Ihandle *ih, double a, char *orientation)
{
  switch (orientation[0])
  {
    case 'V':
      IupSetfAttribute(lbl_v, "TITLE", "VALUE=%.2f", a);
      break;
    case 'H':
      IupSetfAttribute(lbl_h, "TITLE", "VALUE=%.2f", a);
      break;
  }

  IupSetfAttribute(ih, "TIP", "%.2f", a);
  IupSetAttribute(ih, "TIPVISIBLE", "Yes");
}

static int valuechanging_cb(Ihandle *ih, int change)
{
  char *orientation = IupGetAttribute(ih, "ORIENTATION");

  printf("valuechanging_cb(%d)\n", change);
  print_value(ih, IupGetDouble(ih, "VALUE"), orientation);

  if (change)
  {
    switch (orientation[0])
    {
    case 'V':
      IupSetAttribute(lbl_v, "FGCOLOR", "255 0 0");
      break;
    case 'H':
      IupSetAttribute(lbl_h, "FGCOLOR", "255 0 0");
      break;
    }
  }
  else
  {
    switch (orientation[0])
    {
    case 'V':
      IupSetAttribute(lbl_v, "FGCOLOR", "0 0 0");
      break;

    case 'H':
      IupSetAttribute(lbl_h, "FGCOLOR", "0 0 0");
      break;
    }
  }

  return IUP_DEFAULT;
}

static int valuechanged_cb(Ihandle *ih)
{
  char *orientation = IupGetAttribute(ih, "ORIENTATION");

  printf("valuechanged_cb()\n");
  print_value(ih, IupGetDouble(ih, "VALUE"), orientation);

  return IUP_DEFAULT;
}

char *iupKeyCodeToName(int code);

static int k_any(Ihandle *ih, int c)
{
  if (iup_isprint(c))
    printf("K_ANY(%d = %s \'%c\')\n", c, iupKeyCodeToName(c), (char)c);
  else
    printf("K_ANY(%d = %s)\n", c, iupKeyCodeToName(c));
  return IUP_CONTINUE;
}

static int getfocus_cb(Ihandle *ih)
{
  printf("GETFOCUS_CB()\n");
  return IUP_DEFAULT;
}

static int help_cb(Ihandle* ih)
{
  printf("HELP_CB()\n");
  return IUP_DEFAULT;
}
     
static int killfocus_cb(Ihandle *ih)
{
  printf("KILLFOCUS_CB()\n");
  return IUP_DEFAULT;
}

static int leavewindow_cb(Ihandle *ih)
{
  printf("LEAVEWINDOW_CB()\n");
  return IUP_DEFAULT;
}

static int enterwindow_cb(Ihandle *ih)
{
  printf("ENTERWINDOW_CB()\n");
  return IUP_DEFAULT;
}

void FlatValTest(void)
{
  Ihandle *dlg, *val_h, *val_v;

  lbl_v = IupFlatLabel("VALUE=");
  IupSetAttribute(lbl_v, "SIZE", "70x");

  lbl_h = IupFlatLabel("VALUE=");
  IupSetAttribute(lbl_h, "SIZE", "70x");

  val_v = IupFlatVal("VERTICAL");
  IupSetAttribute(val_v, "MAX", "10.0");
  IupSetAttribute(val_v, "STEP", ".02");
  IupSetAttribute(val_v, "PAGESTEP", ".2");
  IupSetCallback(val_v, "HELP_CB",      (Icallback)help_cb);
  IupSetCallback(val_v, "GETFOCUS_CB",  (Icallback)getfocus_cb); 
  IupSetCallback(val_v, "KILLFOCUS_CB", (Icallback)killfocus_cb);
  IupSetCallback(val_v, "ENTERWINDOW_CB", (Icallback)enterwindow_cb);
  IupSetCallback(val_v, "LEAVEWINDOW_CB", (Icallback)leavewindow_cb);
  IupSetCallback(val_v, "K_ANY", (Icallback)k_any);
  IupSetAttribute(val_v, "EXPAND", "VERTICAL");
  IupSetAttribute(val_v, "TIP", "Val Tip");

  val_h = IupFlatVal("HORIZONTAL");
  IupSetAttribute(val_h, "MAX", "100.0");
  IupSetAttribute(val_h, "RASTERSIZE", "x60");
  IupSetAttribute(val_h, "EXPAND", "HORIZONTAL");
//  IupSetAttribute(val_h, "CANFOCUS", "NO");
  
  dlg = IupDialog
  (
    IupHbox
    (
      IupSetAttributes(IupHbox
      (
        val_v,
        lbl_v,
        NULL
      ), "ALIGNMENT=ACENTER"),
      IupSetAttributes(IupVbox
      (
        val_h,
        lbl_h,
        NULL
      ), "ALIGNMENT=ACENTER"),
      NULL
    )
  );

  IupSetCallback(val_v, "VALUECHANGING_CB", (Icallback)valuechanging_cb);
  IupSetCallback(val_h, "VALUECHANGING_CB", (Icallback)valuechanging_cb);
  IupSetCallback(val_v, "VALUECHANGED_CB", (Icallback)valuechanged_cb);
  IupSetCallback(val_h, "VALUECHANGED_CB", (Icallback)valuechanged_cb);

  IupSetAttribute(dlg, "TITLE", "IupFlatVal Test");
//  IupSetAttribute(IupGetChild(dlg, 0), "BGCOLOR", "50 50 255");
//  IupSetAttribute(IupGetChild(dlg, 0), "ACTIVE", "NO");
  IupSetAttribute(dlg, "MARGIN", "10x10");
  IupShowXY(dlg,IUP_CENTER,IUP_CENTER);
}

#ifndef BIG_TEST
int main(int argc, char* argv[])
{
  IupOpen(&argc, &argv);

  FlatValTest();

  IupMainLoop();

  IupClose();

  return EXIT_SUCCESS;
}
#endif
