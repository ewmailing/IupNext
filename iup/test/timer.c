#include <stdio.h>
#include "iup.h"

Ihandle *timer1, *timer2, *timer3;

int timer_cb(Ihandle *ih)
{
  if (ih == timer1)
    printf("timer 1 called\n");

  if (ih == timer2)
  {
    printf("timer 2 called and stopped\n");
    IupSetAttribute(ih, "RUN", "NO");
  }

  if (ih == timer3)
  {
    printf("timer 3 called and CLOSE returned.\n");
    return IUP_CLOSE;
  }

  return IUP_DEFAULT;
}

int main(int argc, char* argv[])
{
  Ihandle *dlg;

  IupOpen(&argc, &argv);

  dlg = IupDialog(NULL);
  IupSetAttribute(dlg, "TITLE", "IupTimer Test");
  IupSetAttribute(dlg, "SIZE", "200x100");

  IupShow(dlg);

  timer1 = IupTimer();
  timer2 = IupTimer();
  timer3 = IupTimer();

  IupSetAttribute(timer1, "TIME",  "100");
  IupSetAttribute(timer1, "RUN",   "YES");
  IupSetCallback(timer1, "ACTION_CB", (Icallback)timer_cb);

  IupSetAttribute(timer2, "TIME",  "400");
  IupSetAttribute(timer2, "RUN",   "YES");
  IupSetCallback(timer2, "ACTION_CB", (Icallback)timer_cb);

  IupSetAttribute(timer3, "TIME",  "5000");
  IupSetAttribute(timer3, "RUN",   "YES");
  IupSetCallback(timer3, "ACTION_CB", (Icallback)timer_cb);

  IupMainLoop();

  IupDestroy(dlg);
  IupDestroy(timer1);
  IupDestroy(timer2);
  IupDestroy(timer3);

  IupClose();

  return 0;
}


