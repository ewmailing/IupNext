/*IupMultiline Simple Example in C 
Shows a multiline that ignores the treatment of the DEL key, canceling its effect. */


#include <stdio.h>
#include "iup.h"
#include "iupkey.h"

char *iupKeyEncode(int code);

int k_f2(Ihandle *ih)
{
  printf("K_F2\n");
  return IUP_DEFAULT;
}

int file_open(Ihandle *ih)
{
  char filename[100] = "";
  IupGetFile(filename);  // BUG: se chama outro dialog a tecla processada no hook, 
                         // e´ correta, mas depois a tecla errada e´ processada no Multiline
                         // add "o" to the text
  printf(filename);
  return IUP_DEFAULT;
}

int k_any(Ihandle *ih, int c)
{
  printf("K_ANY(%d = %s)\n", c, iupKeyEncode(c));
  if (c == K_cA)
    return IUP_IGNORE;   // Sound a beep in Windows
  if (c == K_cP)
  {
    file_open(ih);
    return IUP_IGNORE;   // Sound a beep in Windows, and add "p" to the text
  }
  return IUP_CONTINUE;
}

int mlaction(Ihandle *self, int c, char* after)
{
  printf("ACTION(%d = %s)\n", c, iupKeyEncode(c));
  if (c == K_i)
    return IUP_IGNORE;   // OK
  if (c == K_cD)
    return IUP_IGNORE;   // Sound a beep in Windows
  return IUP_DEFAULT;
}

void main(int argc, char **argv)
{
  Ihandle *dlg, *ml;

  IupOpen(&argc, &argv);

  ml = IupMultiLine(NULL);
  IupSetCallback(ml, "ACTION", (Icallback)mlaction);
  IupSetCallback(ml, "K_ANY", (Icallback)k_any);
  IupSetCallback(ml, "K_F2", (Icallback)k_f2);
  IupSetAttribute(ml, IUP_EXPAND, IUP_YES);
  IupSetAttribute(ml, "WORDWRAP", "YES");

  dlg = IupDialog(IupVbox(ml, IupButton("test", NULL), NULL));
  IupSetAttribute(dlg, IUP_TITLE, "IupMultiline");
  IupSetAttribute(dlg, IUP_SIZE,  "QUARTERxQUARTER");
  IupSetCallback(dlg, "K_cO", (Icallback)file_open);

  IupShow(dlg);
  IupMainLoop();
  IupDestroy(dlg);
  IupClose();
}
