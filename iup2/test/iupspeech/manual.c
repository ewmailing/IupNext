
#include <stdio.h>
#include <string.h>
#include "iup.h"
#include "iupspeech.h"

int action_cb(Ihandle *self, char *text)
{
  printf("Recognized: %s\n", text);
  if(!strcmp(text, "Computer Quit") || !strcmp(text, "Computer Close"))
    return IUP_CLOSE;
  else
    return IUP_DEFAULT;
}

int action2_cb(Ihandle *self, char *text)
{
  printf("Coffee: %s\n", text);
  return IUP_DEFAULT;
}

int main(int argc, char **argv)
{
  Ihandle *speech, *speech2;

  IupOpen(&argc, &argv);
  IupSpeechOpen();

  IupSetFunction("action_cb", (Icallback) action_cb);
  IupSetFunction("action2_cb", (Icallback) action2_cb);

  speech = IupCreate("speech");
  IupSetAttribute(speech, "GRAMMAR", "test.xml");
  IupSetAttribute(speech, "ACTION_CB", "action_cb");
  IupSetAttribute(speech, "SAY", "Test1 loaded!");

  speech2 = IupCreate("speech");
  IupSetAttribute(speech2, "GRAMMAR", "test2.xml");
  IupSetAttribute(speech2, "ACTION_CB", "action2_cb");
  IupSetAttribute(speech2, "SAY", "Test2 loaded.");

  IupShow(IupDialog(IupLabel("SPEECH")));
  IupMainLoop();

  IupSpeechClose();
  IupClose();

	return 1;
}