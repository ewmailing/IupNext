/*IupMultiline Simple Example in C 
Shows a multiline that ignores the treatment of the DEL key, canceling its effect. */


#include <stdio.h>
#include "iup.h"
#include "iupkey.h"

int mlaction(Ihandle *self, int c, char* after)
{
  if(c==K_DEL)
    return IUP_IGNORE;
  else
    return IUP_DEFAULT;
}

void main(int argc, char **argv)
{
  Ihandle *dlg, *ml;

  IupOpen(&argc, &argv);

  ml = IupMultiLine(NULL);
  IupSetCallback(ml, "ACTION", (Icallback)mlaction);
  IupSetAttribute(ml, IUP_EXPAND, IUP_YES);
  IupSetAttribute(ml, IUP_VALUE,  "I ignore the DEL key!");
  IupSetAttribute(ml, IUP_BORDER, IUP_YES);

  dlg = IupDialog(ml);
  IupSetAttribute(dlg, IUP_TITLE, "IupMultiline");
  IupSetAttribute(dlg, IUP_SIZE,  "QUARTERxQUARTER");

  IupShow(dlg);
  IupMainLoop();
  IupDestroy(dlg);
  IupClose();
}