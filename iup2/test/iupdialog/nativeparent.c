
#include <stdio.h>
#include "iup.h"
#include "cd.h"
#include "cdiup.h"

static cdCanvas *cdcanvas = NULL ;
  
int repaint_cb( Ihandle *self, float x, float y )
{
  cdCanvas *oldCanvas = cdActiveCanvas() ;
  cdActivate( cdcanvas ) ;
  cdForeground( CD_BLUE ) ;
  cdBox(0, 100, 0, 100) ;
  cdActivate(oldCanvas) ;
  return IUP_DEFAULT ;
}

int main(int argc, char **argv)
{
  Ihandle *dg, *dg2, *cnvs;
  IupOpen(&argc, &argv);

  IupSetFunction( "repaint_act",( Icallback )repaint_cb ) ;

  cnvs = IupCanvas("repaint_act") ;
  IupSetAttributes(cnvs, "SIZE=300x100, XMIN=0, XMAX=99, POSX=0, DX=10");

  dg = IupDialog(IupLabel("AAAA"));
  dg2 = IupDialog(cnvs);
  IupMap(dg2);  
  IupSetAttribute(dg, "NATIVEPARENT", (char*) IupGetAttribute(dg2, IUP_WID));

  IupMap(dg) ;
  cdcanvas = cdCreateCanvas( CD_IUP, cnvs ) ;

  IupShow(dg);
  IupShow(dg2);
  IupMainLoop();
  IupClose();
  return 1;
}
