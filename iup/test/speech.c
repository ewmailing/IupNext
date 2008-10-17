
#include <stdio.h>
#include <string.h>
#include "iup.h"
#include "iupspeech.h"

int action1_cb(Ihandle *ih, char *text, int rule, int val)
{
  printf("Recognized1: %s (%d %d)\n", text, rule, val);
  return IUP_DEFAULT;
}

int action2_cb(Ihandle *ih, char *text, int rule, int val)
{
  printf("Recognized2: %s (%d %d)\n", text, rule, val);
  return IUP_DEFAULT;
}

int main(int argc, char* argv[])
{
  Ihandle *speech1, *speech2, *dlg;

  IupOpen(&argc, &argv);
  IupSpeechOpen();

  speech1 = IupSpeech();
  IupSetAttribute(speech1, "GRAMMAR", "speech1.xml");
  IupSetCallback(speech1, "ACTION_CB", (Icallback)action1_cb);
  IupSetAttribute(speech1, "SAY", "Test1 loaded!");

  speech2 = IupSpeech();
  IupSetAttribute(speech2, "GRAMMAR", "speech2.xml");
  IupSetCallback(speech2, "ACTION_CB", (Icallback)action2_cb);
  IupSetAttribute(speech2, "SAY", "Test2 loaded.");

  dlg = IupDialog(IupVbox(IupLabel("SPEECH!"), NULL));
  IupSetAttribute(dlg, "TITLE", "IupSpeech Test");
  IupSetAttribute(dlg, "MARGIN", "50x50");

  IupShow(dlg);
  IupMainLoop();

  IupDestroy(dlg);
  IupDestroy(speech1);
  IupDestroy(speech2);

  IupClose();

	return 1;
}
