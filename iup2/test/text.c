/*IupText Example in C 
Creates a IupText that shows asterisks instead of characters (password-like).*/

#include <stdio.h>
#include <string.h>
#include "iup.h"
#include "iupcontrols.h"
#include "iupkey.h"

Ihandle *text, *dlg, *pwd;
static char password[100];

static int k_any(Ihandle *self, int c)
{
  switch (c)
  {
    case K_CR:
    case K_SP:
    case K_ESC:
    case K_INS:
    case K_DEL:
    case K_TAB:
    case K_HOME:
    case K_UP:
    case K_PGUP:
    case K_LEFT:
    case K_MIDDLE:
    case K_RIGHT:
    case K_END:
    case K_DOWN:
    case K_PGDN:
      return IUP_IGNORE;
    default:
      return IUP_DEFAULT;
  }
}

static int action(Ihandle *self, int c, char *after)
{
  int size = strlen(password);
  switch (c)
  {
    case K_CR:
      printf("%d\n", c);
      //IupMessage("Ok", after);
    case K_BS:
      if (size==0)
        return IUP_IGNORE;
      password[size-1] = 0;
      IupSetAttribute(pwd, IUP_VALUE, password);
      return IUP_DEFAULT;
    default:
      if (k_any(self, c) == IUP_IGNORE)
        return IUP_IGNORE;

      password[size] = (char)c;
      password[size+1] = 0;
      break;
  }
  IupSetAttribute(pwd, IUP_VALUE, password);
  return K_asterisk;
}

int main(int argc, char **argv)
{
  IupOpen(&argc, &argv);
  IupControlsOpen();

  IupSetFunction("action", (Icallback) action);
  IupSetFunction("k_any",  (Icallback) k_any);

  memset(password, 0, 100);

  text = IupText("action");
  IupSetAttribute(text, IUP_SIZE,  "200x");
  IupSetAttribute(text, IUP_K_ANY, "k_any");

  pwd = IupText("");
  IupSetAttribute(pwd, IUP_READONLY, IUP_YES);
  IupSetAttribute(pwd, IUP_SIZE, "200x");

  dlg = IupDialog(IupVbox(text, pwd, NULL));
  IupSetAttribute(dlg, "TITLE", "IupText");

  printf("%p\n", IupGetNextChild(dlg, NULL));
  //IupShowXY(dlg, IUP_CENTER, IUP_CENTER);
  IupPopup(dlg, IUP_CENTER, IUP_CENTER);

  IupMainLoop();
  IupClose();
  return 0;
}