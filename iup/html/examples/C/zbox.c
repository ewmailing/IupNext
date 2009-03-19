/*IupZbox Example in C 
An application of a zbox could be a program requesting several entries 
from the user according to a previous selection. 
In this example, a list of possible layouts, each one consisting of an element, 
is presented, and according to the selected option the dialog below the list is changed. */

#include <stdlib.h>
#include <stdio.h>
#include "iup.h"

int list_cb (Ihandle *h, char *t, int o, int selected)
{
  Ihandle* zbox = IupGetHandle ("zbox");
  
  if (selected == 1)
    IupSetAttribute (zbox, "VALUE", t);
  
  return IUP_DEFAULT;
}

int main(int argc, char **argv)
{
  Ihandle *dlg;
  Ihandle *frm;
  Ihandle *zbox;
  Ihandle *text;
  Ihandle *list;
  Ihandle *lbl;
  Ihandle *btn;
  Ihandle *fill;

  IupOpen(&argc, &argv);

  fill = IupFill();

  text = IupText("");

  IupSetAttributes (text, "EXPAND = YES, VALUE = \"Enter your text here\"");
  
  /* Creates a label */
  lbl = IupLabel("This element is a label");

  /* Creatas a button */
  btn = IupButton ("This button does nothing", "");

  /* Creates handles for manipulating the zbox VALUE */
  IupSetHandle ("fill", fill);
  IupSetHandle ("text", text);
  IupSetHandle ("lbl", lbl);
  IupSetHandle ("btn", btn);
	
  /* Creates zbox with four elements */
  zbox = IupZbox (fill, text, lbl, btn, NULL);

  /* Associates handle "zbox" with zbox */
  IupSetHandle ("zbox", zbox);

  /* Sets zbox alignment */
  IupSetAttribute (zbox, "ALIGNMENT", "ACENTER");

  /* Creates frame */
  frm = IupFrame
  (
    IupHbox
    (
      IupFill(),
      list = IupList(NULL),
      IupFill(),
      NULL
    )
  ),

  /* Creates dialog */
  dlg = IupDialog
  (
    IupVbox
    (
      frm,
      zbox,
      NULL
    )
  );

  IupSetAttributes (list, "1 = fill, 2 = text, 3 = lbl, 4 = btn");
  IupSetAttribute (frm, "TITLE", "Select an element");
  IupSetAttributes (dlg, "SIZE = QUARTER, TITLE = \"IupZbox Example\"");
  IupSetCallback (list, "ACTION", (Icallback) list_cb);

  IupShowXY (dlg, IUP_CENTER, IUP_CENTER );
  IupMainLoop ();
  IupClose ();
  return EXIT_SUCCESS;

}