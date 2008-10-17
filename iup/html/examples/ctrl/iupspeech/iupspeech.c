/*IupSpeech Example in C */

#include <stdio.h>
#include <string.h>
#include "iup.h"
#include "iupspeech.h"

int action_cb(Ihandle *self, char *text)
{
  printf("xml1: %s\n", text);
  if(!strcmp(text, "Computer Quit") || !strcmp(text, "Computer Close"))
    return IUP_CLOSE;
  else
    return IUP_DEFAULT;
}

int action2_cb(Ihandle *self, char *text)
{
  printf("xml2: %s\n", text);
  return IUP_DEFAULT;
}

int main(int argc, char **argv)
{
  Ihandle *speech, *speech2, *dlg;

  IupOpen(&argc, &argv);
  IupSpeechOpen();

  speech = IupCreate("speech");
  IupSetCallback(canvas, "ACTION", (Icallback) redraw);
  IupSetAttribute(speech, "GRAMMAR", "xml1.xml");
  IupSetCallback(speech, "ACTION_CB", (Icallback) action_cb);
  IupSetAttribute(speech, "SAY", "xml 1 loaded!");

  speech2 = IupCreate("speech");
  IupSetAttribute(speech2, "GRAMMAR", "xml2.xml");
  IupSetCallback(speech2, "ACTION_CB", (Icallback) action2_cb);
  IupSetAttribute(speech2, "SAY", "xml 2 loaded!");

  dlg = IupDialog(IupLabel("SPEECH"));
  IupShow(dlg);
  IupMainLoop();

  IupDestroy(dlg);
  IupSpeechClose();
  IupClose();

  return 1;
}