
#include <stdio.h>
#include <stdlib.h>

#include "iup.h"
#include "cd.h"
#include "cdiup.h"
#include "iupcontrols.h"

/* IUP handles */
static Ihandle *cnvs = NULL ;

/* CD canvas */
static cdCanvas *cdcanvas = NULL ;

int repaint_cb( Ihandle *self)
{
  if (!cdcanvas) return IUP_DEFAULT;

  /* Activates canvas cdcanvas */
  cdActivate( cdcanvas ) ;
  cdClear();
  
  /* Draws a rectangle on the canvas */
  cdBegin(CD_FILL);
  cdVertex(50, 50);
  cdVertex(150, 50);
  cdVertex(100, 150);
  cdEnd();
  
  /* Function executed sucessfully */
  return IUP_DEFAULT;
}

char* cell_cb(Ihandle* self, int cell)
{
  unsigned char r, g, b;
  int ri, gi, bi;
  static char str[30];
  sprintf(str, "CELL%d", cell);
  sscanf(IupGetAttribute(self, str), "%d %d %d", &ri, &gi, &bi);
  r = (unsigned char)ri; g = (unsigned char)gi; b = (unsigned char)bi;
  if (IupGetColor(IUP_MOUSEPOS, IUP_MOUSEPOS, &r, &g, &b))
  {
    sprintf(str, "%d %d %d", (int)r, (int)g, (int)b);
    cdActivate( cdcanvas ) ;
    if (IupGetInt(self, "PRIMARY_CELL") == cell)
      cdForeground(cdEncodeColor(r, g, b));
    else
      cdBackground(cdEncodeColor(r, g, b));
    repaint_cb(cnvs);
    return str;
  }
  return NULL;
}

int select_cb(Ihandle* self, int cell, int type)
{
  long cd_color;
  char str[30];
  int ri, gi, bi;
  sprintf(str, "CELL%d", cell);
  sscanf(IupGetAttribute(self, str), "%d %d %d", &ri, &gi, &bi);
  cd_color = cdEncodeColor((unsigned char)ri,(unsigned char)gi,(unsigned char)bi);
   
  cdActivate( cdcanvas ) ;
  if (type == IUP_PRIMARY)
    cdForeground(cd_color);
  else
    cdBackground(cd_color);

  repaint_cb(cnvs);

  return IUP_DEFAULT;
}

int switch_cb(Ihandle* self, int primcell, int seccell)
{
  long fgcolor;
  cdActivate(cdcanvas) ;
  fgcolor = cdForeground(CD_QUERY);
  cdForeground(cdBackground(CD_QUERY));
  cdBackground(fgcolor);
  repaint_cb(cnvs);
  return IUP_DEFAULT;
}

/* Main program */
int main(void)
{
  Ihandle *dlg, *cb;
  
  /* Initializes IUP */
  IupOpen() ;
  IupControlsOpen() ;

  /* Program begin */
  
  /* Registers callbacks */  
  IupSetFunction("repaint_cb",( Icallback )repaint_cb);
  IupSetFunction("select_cb",( Icallback )select_cb);
  IupSetFunction("cell_cb",( Icallback )cell_cb);
  IupSetFunction("switch_cb",( Icallback )switch_cb);

  /* Creates a canvas associated with the repaint action */
  cnvs = IupCanvas("repaint_cb") ;
  
  /* Sets size, mininum and maximum values, position and size of the thumb   */
  /* of the horizontal scrollbar of the canvas                               */
  IupSetAttributes(cnvs, "RASTERSIZE=200x300");

  cb = IupColorbar(); 
  IupSetAttribute(cb, "RASTERSIZE", "70x");
  IupSetAttribute(cb, "EXPAND", "VERTICAL");
  IupSetAttribute(cb, "NUM_PARTS", "2");
  IupSetAttribute(cb, "SHOW_SECONDARY", "YES");
  IupSetAttribute(cb, "SELECT_CB", "select_cb");
  IupSetAttribute(cb, "CELL_CB", "cell_cb");
  IupSetAttribute(cb, "SWITCH_CB", "switch_cb");
//  IupSetAttribute(cb, "SQUARED", "NO");
  IupSetAttribute(cb, "PREVIEW_SIZE", "60");

  /* Creates a dialog with a vbox containing the canvas and the colorbar. */
  dlg = IupDialog(IupHbox(cnvs, cb, NULL));
  
  /* Sets the dialog's title, so that it is mapped properly */
  IupSetAttribute(dlg, IUP_TITLE, "IupColorbar");
  
  /* Maps the dialog. This must be done before the creation of the CD canvas */
  IupMap(dlg);
  
  /* Creates a CD canvas of type CD_IUP */ 
  cdcanvas = cdCreateCanvas(CD_IUP, cnvs);
   
  /* Shows dialog on the center of the screen */
  IupShow(dlg);

  /* Initializes IUP main loop */
  IupMainLoop();

  IupDestroy(dlg);
  cdKillCanvas(cdcanvas);

  /* Finishes IUP */
  IupControlsClose();  
  IupClose();  

  /* Program finished sucessfully */
  return 0 ;
}
