/*IupHbox Example in C 
Creates a dialog with buttons placed side by side, with the purpose of showing the organization possibilities of elements inside an hbox. The IUP_ALIGNMENT attribute is explored in all its possibilities to obtain the given effect. */

/*
	Program to exemplify the IupHbox element.
*/

/* IupHbox example */
#include <stdio.h>
#include "iup.h"

int main(int argc, char **argv)
{
  /* IUP identifiers */
  Ihandle *fr1, *fr2, *fr3, *dialog;
  Ihandle *b11, *b12, *b13;
  Ihandle *b21, *b22, *b23;
  Ihandle *b31, *b32, *b33;
  Ihandle *h1,  *h2,  *h3;

  /* Initializes IUP */
  IupOpen(&argc, &argv);

  /* Creates frame with three top aligned buttons */
  fr1 = IupFrame
  (
    h1=IupHbox
    (
      IupFill(),
      b11=IupButton ("1", ""),
      b12=IupButton ("2", ""),
      b13=IupButton ("3", ""),
      IupFill(),
      NULL
    )
  );
  IupSetAttribute(fr1, IUP_TITLE, "ALIGNMENT=ATOP, GAP=10, SIZE=200");
  IupSetAttribute(b11, IUP_SIZE, "30x30") ;
  IupSetAttribute(b12, IUP_SIZE, "30x40") ;
  IupSetAttribute(b13, IUP_SIZE, "30x50") ;
  IupSetAttributes(h1, "ALIGNMENT=ATOP, GAP=10, SIZE=200") ; /* Sets hbox's alignment, gap and size */

  /* Creates frame with three buttons */
  fr2 = IupFrame
  (
    h2=IupHbox
    (
      IupFill(),
      b21=IupButton("1", ""),
      b22=IupButton("2", ""),
      b23=IupButton("3", ""),
      IupFill(),
      NULL
    )
  ) ;
  IupSetAttribute(fr2, IUP_TITLE, "ALIGNMENT=ACENTER, GAP=20") ;
  IupSetAttribute(b21, IUP_SIZE, "30x30") ;
  IupSetAttribute(b22, IUP_SIZE, "30x40") ;
  IupSetAttribute(b23, IUP_SIZE, "30x50") ;
  IupSetAttributes(h2, "ALIGNMENT=ACENTER, GAP=20") ; /* Sets hbox's alignment and gap */

  /* Creates frame with three bottom aligned buttons */
  fr3 = IupFrame
  (
    h3=IupHbox
    (
      IupFill(),
      b31=IupButton ("1", ""),
      b32=IupButton ("2", ""),
      b33=IupButton ("3", ""),
      IupFill(),
      NULL
    )
  ) ;
  IupSetAttribute(fr3, IUP_TITLE, "ALIGNMENT = ABOTTOM, SIZE = 150") ;
  IupSetAttribute(b31, IUP_SIZE, "30x30") ;
  IupSetAttribute(b32, IUP_SIZE, "30x40") ;
  IupSetAttribute(b33, IUP_SIZE, "30x50") ;
  IupSetAttributes(h3, "ALIGNMENT = ABOTTOM, SIZE = 150") ; /* Sets hbox's alignment and size */

  /* Creates dialog with the three frames */
  dialog = IupDialog
  (
    IupVbox
    (
      fr1,
      fr2,
      fr3,
      NULL
    )
  ) ;

  IupSetAttribute(dialog, IUP_TITLE, "IupHbox") ; /* Sets dialog's title */

  IupShowXY( dialog, IUP_CENTER, IUP_CENTER ) ; /* Shows dialog in the center of the screen */
  IupMainLoop() ;                               /* Initializes IUP main loop */
  IupDestroy(dialog);
  IupClose() ;                                  /* Finishes IUP */

  return 0 ;
}