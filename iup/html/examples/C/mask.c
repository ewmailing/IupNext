/*  IupMask: Example in C 
    Creates an IupText that accepts only numbers.
*/

#include <stdlib.h>
#include "iup.h"
#include "iupcontrols.h"

void main(int argc, char **argv)
{
  int test;
  Ihandle *text, *dg;

  IupOpen(&argc, &argv);
  IupControlsOpen () ;

  text = IupText("");
  test = iupmaskSet(text, "/d*", 0, 1);

  IupSetAttribute(text, "SIZE",   "100x");
  IupSetAttribute(text, "EXPAND", "HORIZONTAL");

  dg = IupDialog(text);
  IupSetAttribute(dg, "TITLE", "IupMask");

  IupShow(dg);
  IupMainLoop();
  iupmaskRemove(text);
  IupDestroy(dg);
  IupControlsClose() ;
  IupClose();
}