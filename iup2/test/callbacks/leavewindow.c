
#include <stdio.h>
#include <stdlib.h>

#include "iup.h"

int leavewindow(Ihandle *self)
{
  printf("LEAVEWINDOW(%s)\n", IupGetAttribute(self, "NAME"));
  return IUP_DEFAULT;
}

int enterwindow(Ihandle *self)
{
  printf("ENTERWINDOW(%s)\n", IupGetAttribute(self, "NAME"));
  return IUP_DEFAULT;
}

/* Main program */
int main(int argc, char **argv)
{
  Ihandle *dlg, *cnvs, *bt;
  
  /* Initializes IUP */
  IupOpen(&argc, &argv);

  /* Program begin */
  
  /* Registers callbacks */  
  IupSetFunction( "leavewindow",( Icallback )leavewindow) ;
  IupSetFunction( "enterwindow",( Icallback )enterwindow ) ;
  
  /* Creates a canvas associated with the repaint action */
  cnvs = IupCanvas( "" ) ;
  IupSetAttribute(cnvs, IUP_LEAVEWINDOW_CB, "leavewindow");
  IupSetAttribute(cnvs, IUP_ENTERWINDOW_CB, "enterwindow");
  IupSetAttribute(cnvs, "NAME", "canvas1");
  IupSetAttribute(cnvs, "SIZE", "300x100");
  
  bt = IupButton( "Test", "");
  IupSetAttribute(bt, IUP_LEAVEWINDOW_CB, "leavewindow");
  IupSetAttribute(bt, IUP_ENTERWINDOW_CB, "enterwindow");
  IupSetAttribute(bt, "NAME", "button1");

  /* Sets size, mininum and maximum values, position and size of the thumb   */
  /* of the horizontal scrollbar of the canvas                               */

  /* Creates a dialog with a frame containing a canvas */
  dlg = IupDialog(IupSetAttributes(IupVbox(cnvs, bt, IupCanvas(""), IupButton("teste2", ""), NULL), "MARGIN=10x10, GAP=10"));
  
  /* Sets the dialog's title, so that it is mapped properly */
  IupSetAttribute( dlg, IUP_TITLE, "Enter-Leave Window" ) ;
  
  /* Shows dialog on the center of the screen */
  IupShowXY( dlg, IUP_CENTER, IUP_CENTER ) ;

  /* Initializes IUP main loop */
  IupMainLoop() ;

  /* Finishes IUP */
  IupClose() ;  

  /* Program finished sucessfully */
  return 0 ;
}


