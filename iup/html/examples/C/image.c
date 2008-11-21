/*IupImage Example in C 
Creates a button, a label, a toggle and a radio using an image. Uses an image for the cursor as well.. */

#include <stdio.h>
#include "iup.h"

static unsigned char pixmap_x [ ] = 
{
  1,2,3,3,3,3,3,3,3,2,1, 
  2,1,2,3,3,3,3,3,2,1,2, 
  3,2,1,2,3,3,3,2,1,2,3, 
  3,3,2,1,2,3,2,1,2,3,3, 
  3,3,3,2,1,2,1,2,3,3,3, 
  3,3,3,3,2,1,2,3,3,3,3, 
  3,3,3,2,1,2,1,2,3,3,3, 
  3,3,2,1,2,3,2,1,2,3,3, 
  3,2,1,2,3,3,3,2,1,2,3, 
  2,1,2,3,3,3,3,3,2,1,2, 
  1,2,3,3,3,3,3,3,3,2,1
};

static unsigned char pixmap_cursor [ ] =
{
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,2,2,2,2,2,2,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,1,1,1,1,2,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,1,1,1,1,2,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,1,1,1,1,1,2,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,2,1,1,1,1,1,1,2,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,2,1,1,1,1,1,2,2,2,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,2,1,1,1,1,1,2,0,0,2,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,2,1,1,1,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,2,2,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
} ;

int main(int argc, char **argv)
{
  char string_size [ 40 ] ;
  
  Ihandle *dlg ;
  Ihandle *img_x, *img_cursor ;
  Ihandle *lbl, *lbl_size ;
  Ihandle *btn ;
  Ihandle *tgl, *tgl_radio_1, *tgl_radio_2 ;
  Ihandle *frm_lbl, *frm_btn, *frm_tgl, *frm_tgl_radio ;

  IupOpen(&argc, &argv);

  img_x = IupImage ( 11, 11, pixmap_x ) ;
  IupSetAttribute ( img_x, "1", "0 1 0") ;
  IupSetAttribute ( img_x, "2", "255 0 0") ; 
  IupSetAttribute ( img_x, "3", "255 255 0") ;

  IupSetHandle ( "img_x", img_x );
  img_cursor = IupImage ( 32, 32, pixmap_cursor ) ;

  IupSetAttribute ( img_cursor, "1", "255 0 0" ) ; 
  IupSetAttribute ( img_cursor, "2", "128 0 0" ) ; 

  IupSetAttribute ( img_cursor, IUP_HOTSPOT, "21:10" ) ;
  
  IupSetHandle ( "img_cursor", img_cursor );

  btn = IupButton ( "Dummy", "" ) ;
  IupSetAttribute ( btn, IUP_IMAGE, "img_x" ) ;

  lbl = IupLabel ( "Dummy" ) ;
  IupSetAttribute ( lbl, IUP_IMAGE, "img_x" ) ;

  tgl = IupToggle ( "Dummy", NULL ) ;
  IupSetAttribute ( tgl, IUP_IMAGE, "img_x" ) ;

  tgl_radio_1 = IupToggle ( "Dummy", NULL ) ;
  tgl_radio_2 = IupToggle ( "Dummy", NULL ) ;

  IupSetAttribute ( tgl_radio_1, IUP_IMAGE, "img_x" ) ;
  IupSetAttribute ( tgl_radio_2, IUP_IMAGE, "img_x" ) ;

  sprintf ( string_size, "\"X\" image width = %s; \"X\" image height = %s", IupGetAttribute ( img_x, IUP_WIDTH ), IupGetAttribute ( img_x, IUP_HEIGHT ) ) ; 

  lbl_size = IupLabel ( string_size ) ;
  
  frm_btn = IupFrame ( btn ) ;
  frm_lbl = IupFrame ( lbl ) ;
  frm_tgl = IupFrame ( tgl ) ;
  frm_tgl_radio = IupFrame ( IupRadio ( IupVbox ( tgl_radio_1, tgl_radio_2, NULL ) ) ) ;

  IupSetAttributes ( frm_btn, "TITLE=button, SIZE=EIGHTHxEIGHTH" ) ;
  IupSetAttributes ( frm_lbl, "TITLE=label , SIZE=EIGHTHxEIGHTH" ) ;
  IupSetAttributes ( frm_tgl, "TITLE=toggle, SIZE=EIGHTHxEIGHTH" ) ;
  IupSetAttributes ( frm_tgl_radio, "TITLE=radio, SIZE=EIGHTHxEIGHTH" ) ;
  
  dlg = IupDialog
        (
          IupVbox
          (
            IupHbox ( frm_btn, frm_lbl, frm_tgl, frm_tgl_radio, NULL ),
            IupFill(),
            IupHbox ( IupFill(), lbl_size, IupFill(), NULL ), NULL
          )
        ) ;

  IupSetAttributes ( dlg, "TITLE=\"IupImage Example\", SIZE=HALFxQUARTER, CURSOR=img_cursor" ) ;

  IupShowXY ( dlg, IUP_CENTER, IUP_CENTER ) ;

  IupMainLoop ( ) ;
  IupDestroy(dlg);
  IupDestroy(img_x);
  IupDestroy(img_cursor);
  IupClose ( ) ;
  return 0 ;
}