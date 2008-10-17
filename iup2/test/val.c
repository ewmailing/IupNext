/*
  Program to exemplify the IupVal element.
*/
#include <stdlib.h>
#include <stdio.h>

#include "iup.h"
#include "iupcontrols.h"

Ihandle *lbl_h=NULL, *lbl_v=NULL;


static int mousemove(Ihandle *c, double a)
{
  char buffer[40];
  char *type=NULL;

//  printf("mousemove %g\n", a);

  sprintf(buffer, "VALUE=%.2g", a);

  type = IupGetAttribute(c, "TYPE");

  switch(type[0])
  {
    case 'V':
      IupStoreAttribute(lbl_v, IUP_TITLE, buffer);
      break;

    case 'H':
      IupStoreAttribute(lbl_h, IUP_TITLE, buffer);
      break;
  }

  return IUP_DEFAULT;
}

static int button_press(Ihandle *c, double a)
{
  char *type = IupGetAttribute(c, "TYPE");

//  printf("button_press %g\n", a);

  switch(type[0])
  {
    case 'V':
      IupSetAttribute(lbl_v, IUP_FGCOLOR, "255 0 0");
      break;

    case 'H':
      IupSetAttribute(lbl_h, IUP_FGCOLOR, "255 0 0");
      break;
  }

  mousemove(c, a);

  return IUP_DEFAULT;
}

static int button_release(Ihandle *c, double a)
{
  char *type = IupGetAttribute(c, "TYPE");

//  printf("button_release %g\n", a);

  switch(type[0])
  {
    case 'V':
      IupSetAttribute(lbl_v, IUP_FGCOLOR, "0 0 0");
      break;

    case 'H':
      IupSetAttribute(lbl_h, IUP_FGCOLOR, "0 0 0");
      break;
  }

  mousemove(c, a);

  return IUP_DEFAULT;
}


void main(int argc, char* argv[])
{
  Ihandle *dlg_val, *val_h, *val_v;

  IupOpen(&argc, &argv);
  IupControlsOpen();

  val_v = IupVal(ICTL_VERTICAL);
  val_h = IupVal(ICTL_HORIZONTAL);
  lbl_v = IupLabel("VALUE=");
  lbl_h = IupLabel("VALUE=");
  IupSetAttribute(lbl_v, IUP_SIZE, "50x");
  IupSetAttribute(lbl_h, IUP_SIZE, "50x");
  IupSetAttribute(val_v, "SHOWTICKS", "5");
  IupSetAttribute(val_v, "PAGESTEP", ".2");
  IupSetAttribute(val_v, "STEP", ".1");

  dlg_val = IupDialog
  (
    IupTabs(IupHbox
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
      NULL),
    NULL)
  );

  IupSetAttribute(val_v, ICTL_BUTTON_PRESS_CB,  "button_press");
  IupSetAttribute(val_v, ICTL_BUTTON_RELEASE_CB,  "button_release");
  IupSetAttribute(val_v, ICTL_MOUSEMOVE_CB, "mousemove"); 

  IupSetAttribute(val_h, ICTL_BUTTON_PRESS_CB,  "button_press");
  IupSetAttribute(val_h, ICTL_BUTTON_RELEASE_CB,  "button_release");
  IupSetAttribute(val_h, ICTL_MOUSEMOVE_CB, "mousemove"); 

  /* Sets mouse movement callback */
  IupSetFunction("button_press",  (Icallback) button_press);
  IupSetFunction("button_release",(Icallback) button_release);
  IupSetFunction("mousemove", (Icallback) mousemove);

  IupSetAttribute(dlg_val, IUP_TITLE, "IupVal");
  IupSetAttribute(dlg_val, IUP_MARGIN, "10x10");
  IupShowXY(dlg_val,IUP_CENTER,IUP_CENTER);

  IupMainLoop();
  IupDestroy(dlg_val);
  IupClose();
}


