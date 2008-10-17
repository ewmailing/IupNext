#include <stdio.h>
#include "iup.h"

Ihandle *timer1, *timer2, *timer3;

int timer_cb(Ihandle *n)
{
  if(n == timer1)
    printf("timer 1 called\n");

  if(n == timer2)
  {
    printf("timer 2 called\n");
    IupSetAttribute(n, "RUN", IUP_NO);
    IupSetAttribute(n, "RUN", IUP_NO);
  }

  if(n == timer3)
  {
    printf("timer 3 called\n");
    return IUP_CLOSE;
  }

  return IUP_DEFAULT;
}

int main(int argc, char **argv)
{
  Ihandle *dialog, *text;

  IupOpen(&argc, &argv);

  IupSetFunction("timer", (Icallback) timer_cb);

  text = IupLabel("Timer example - will close in 5secs");
  IupSetHandle("quit", text);

  /* Creating main dialog */  
  dialog = IupDialog(IupVbox(text, NULL));
  IupSetAttribute(dialog, IUP_TITLE, "timer example");
  IupSetAttribute(dialog, IUP_SIZE, "200x200");

  IupShowXY(dialog, IUP_CENTER, IUP_CENTER);

  timer1 = IupCreate("timer");
  timer2 = IupCreate("timer");
  timer3 = IupCreate("timer");

  IupSetAttribute(timer1, "TIME",  "100");
  IupSetAttribute(timer1, "RUN",   "YES");
  IupSetAttribute(timer1, "ACTION_CB", "timer");

  IupSetAttribute(timer2, "TIME",  "400");
  IupSetAttribute(timer2, "RUN",   "YES");
  IupSetAttribute(timer2, "ACTION_CB", "timer");

  IupSetAttribute(timer3, "TIME",  "5000");
  IupSetAttribute(timer3, "RUN",   "YES");
  IupSetAttribute(timer3, "ACTION_CB", "timer");
  printf("%s test\n", IupGetClassName(timer1));

  IupMainLoop();
  IupDestroy(dialog);
  IupDestroy(timer1);
  IupDestroy(timer2);
  IupDestroy(timer3);
  IupClose();

  return 0;
}


