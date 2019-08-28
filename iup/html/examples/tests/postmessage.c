#include <stdlib.h>
#include <stdio.h>
#include "iup.h"
#include "iupcbs.h"

static Ihandle *timer1;
static Ihandle *button;

static int postmessage_cb(Ihandle *ih, char* s, int i, double d)
{
//  IupLog("DEBUG", "In postmessage_cb\n");
  IupLog("DEBUG", "In postmessage_cb, ih:%p, sp:%p, s:%s, i:%d, d:%lf\n", ih, s, s, i, d);
  int is_active = IupGetInt(ih, "ACTIVE");
  IupSetInt(ih, "ACTIVE", !is_active);

  static int flip = 0;
  if(flip)
  {
    IupSetAttribute(ih, "TITLE", "BUTTON");
  }
  else
  {
    IupSetAttribute(ih, "TITLE", "button");
  }
  flip = !flip;
			
  return IUP_DEFAULT;
}

static int timer_cb(Ihandle *ih)
{
  IupPostMessage(button, NULL, 0, 0);
//  IupPostMessage(button, "foo", (int)(intptr_t)ih, (double)(intptr_t)ih);
  return IUP_DEFAULT;
}

//#define USE_PTHREADS 1
#define USE_PTHREADS 0
#if USE_PTHREADS
#include <pthread.h>
#include <unistd.h>
#include <assert.h>

void* thread_main(void* param)
{
  while(1)
  {
    usleep(4000*1000);
    IupPostMessage(button, "foo", (int)(intptr_t)button, (double)(intptr_t)button);
  }
}

void PostMessageTest(void)
{
  Ihandle *dlg;
  button = IupButton("Button", NULL);
  // button = IupLabel("Button"); // hack because we haven't finished implementing all the button features for IupEmscripten yet
  IupSetCallback(button, "POSTMESSAGE_CB", (Icallback)postmessage_cb);
  dlg = IupDialog(button);
  IupSetAttribute(dlg, "TITLE", "IupPostMessage Test");
  IupSetAttribute(dlg, "SIZE", "200x100");
  IupShow(dlg);

  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
  pthread_t thread;
  int rc = pthread_create(&thread, &attr, thread_main, 0);
}

#else


void PostMessageTest(void)
{
  Ihandle *dlg;
  button = IupButton("Button", NULL);
  IupSetCallback(button, "POSTMESSAGE_CB", (Icallback)postmessage_cb);
  dlg = IupDialog(button);
  IupSetAttribute(dlg, "TITLE", "IupPostMessage Test");
  IupSetAttribute(dlg, "SIZE", "200x100");

  IupShow(dlg);
  timer1 = IupTimer();
  IupSetAttribute(timer1, "TIME",  "4000");
  IupSetAttribute(timer1, "RUN",   "YES");
  IupSetCallback(timer1, "ACTION_CB", (Icallback)timer_cb);
}
#endif

#ifndef BIG_TEST
#ifndef IUP_LEGACY

void IupExitPoint()
{
	IupClose();
}

void IupEntryPoint()
{
	IupSetFunction("EXIT_CB", (Icallback)IupExitPoint);
	PostMessageTest();
}


int main(int argc, char * argv[])
{
	IupOpen(0, NULL);
	IupSetFunction("ENTRY_POINT", (Icallback)IupEntryPoint);
	IupMainLoop();
}



#else
int main(int argc, char* argv[])
{
  IupOpen(&argc, &argv);

  PostMessageTest();

  IupMainLoop();

  IupClose();

  return EXIT_SUCCESS;
}
#endif
#endif
