/*
 * IupCanvas example
 * Description : Creates a IUP canvas and uses CD to draw on it
 *      Remark : IUP must be linked to the CD library                      
 */

#include <stdio.h>
#include <stdlib.h>

#include "iup.h"
#include "cd.h"
#include "cdiup.h"

static cdCanvas *cdcanvas = NULL;

int redraw_cb( Ihandle *self, float x, float y )
{
  cdCanvasActivate(cdcanvas);
  cdCanvasBackground(cdcanvas, CD_BLACK);
  cdCanvasClear(cdcanvas);
  cdCanvasForeground(cdcanvas, CD_BLUE);
  cdCanvasBox(cdcanvas, 0, 100, 0, 100);
  return IUP_DEFAULT;
}

int main(int argc, char **argv)
{
  Ihandle *dlg, *cnvs;
  IupOpen(&argc, &argv);

  cnvs = IupCanvas( NULL );
  IupSetCallback(cnvs, "ACTION",( Icallback )redraw_cb );
  IupSetAttribute(cnvs, "SIZE", "300x100");
  
  dlg = IupDialog( IupFrame( cnvs ) );
  IupSetAttribute( dlg, "TITLE", "IupCanvas + Canvas Draw" );
  IupMap( dlg );
  
  cdcanvas = cdCreateCanvas( CD_IUP, cnvs );
  
  IupShowXY( dlg, IUP_CENTER, IUP_CENTER );
  IupMainLoop();
  IupClose();  
  return EXIT_SUCCESS;

}
