/*****************************************************************************
 *                             IupCanvas example                             * 
 *   Description : Creates a IUP canvas and uses CD to draw on it            *
 *        Remark : IUP must be linked to the CD library                      *
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include "iup.h"
#include "cd.h"
#include "cdiup.h"
#include "iupcontrols.h"

static cdCanvas *cdcanvas = NULL ;

/******************************************************************************
 * Function:                                                                  *
 * Repaint                                                                    *
 *                                                                            *
 * Description:                                                               *
 * Repaints canvas                                                            *
 *                                                                            *
 * Parameters received:                                                       *
 * self - IUP handle for the canvas                                           *     
 * x    - Thumb position in the horizontal scrollbar                          * 
 * y    - Thumb position in the vertical scrollbar                            *
 *                                                                            * 
 * Value returned:                                                            *
 * IUP_DEFAULT                                                                *
 ******************************************************************************/ 
int repaint_cb( Ihandle *self, float x, float y )
{
  /* Activates canvas cdcanvas */
  cdActivate( cdcanvas ) ;
  cdClear();
  
  /* Sets the active color to blue */
  cdForeground( CD_BLUE ) ;
  
  /* Draws a rectangle on the canvas */
  cdBox( 10, 100, 10, 100 ) ;
  
  /* Function executed sucessfully */
  return IUP_DEFAULT ;
}

int motion_cb(Ihandle *self, int x, int y, char *r) 
{
  printf("motion_cb(x=%d, y=%d, r=%s)\n", x, y, r);
  return IUP_DEFAULT;
}

int item(Ihandle *self) 
{
  printf("item=%s\n", IupGetAttribute(self, IUP_TITLE));
  return IUP_DEFAULT;
}

/* IUP handles */
Ihandle* menu = NULL;
Ihandle *dlg = NULL ;
Ihandle *cnvs = NULL ;
Ihandle *bt = NULL ;

int button_cb(Ihandle* self,int but, int pressed, int x, int y, char* r)
{
  printf("button_cb(but=%d, pressed=%d, x=%d, y=%d, r=%s)\n", but, pressed, x, y, r);
  if (but == IUP_BUTTON3 && pressed)
  {
//    IupSetAttribute(bt, IUP_ACTIVE, IUP_NO);
    IupPopup(menu, IUP_MOUSEPOS, IUP_MOUSEPOS);
  //  IupSetAttribute(bt, IUP_ACTIVE, IUP_YES);
  }
  return IUP_DEFAULT;
}

void showlistdialog(void)
{
  int error; 	
  int size = 8 ;
  int marks[8] = { 0,0,0,0,1,1,0,0 };
  char *options[] = {
    "Blue"   ,
    "Red"    ,
    "Green"  ,
    "Yellow" ,
    "Black"  ,
    "White"  ,
    "Gray"   ,
    "Brown"  } ;
	  
  error = IupListDialog(1,"Color Selection",size,options,0,16,5,marks);

  if(error == -1)
  {
    printf("IupListDialog-Operation canceled");
  }
  else
  {
    int i;
    int selection = 0;
	  
    for(i = 0 ; i < size ; i++)
    {
      if (marks[i])
      {
        selection = 1;
        break;
      }
    }

    if (selection)
      printf("IupListDialog-Options selected\n");	  
    else
      printf("IupListDialog-No option selected\n");
  }
}

void showfile(void)
{
  Ihandle *filedlg; 
  
  filedlg = IupFileDlg(); 
 
  IupSetAttributes(filedlg, "DIALOGTYPE = OPEN, TITLE = \"File Open\"");
  IupSetAttributes(filedlg, "FILTER = \"*.*\", FILTERINFO = \"All Files\"");

  IupPopup(filedlg, IUP_CENTER, IUP_CENTER); 

  switch(IupGetInt(filedlg, IUP_STATUS))
  {
    case 1: 
      printf("IupFileDlg-New file(%s)\n",IupGetAttribute(filedlg, IUP_VALUE)); 
      break; 

    case 0 : 
      printf("IupFileDlg-File already exists(%s)\n",IupGetAttribute(filedlg, IUP_VALUE));
      break; 

    case -1 : 
      printf("IupFileDlg-Operation Canceled\n");
      return 1;
      break; 
  }

  IupDestroy(filedlg);
}

int pos(Ihandle *c)
{
//  printf("canvas position %sx%s\n", 
//      IupGetAttribute(cnvs, IUP_X), 
//      IupGetAttribute(cnvs, IUP_Y));
  showlistdialog();
  return IUP_DEFAULT;
}

/* Main program */
int main(int argc, char **argv)
{
  
  /* Initializes IUP */
  IupOpen(&argc, &argv);

  /* Program begin */
  
  /* Registers callbacks */  
  IupSetFunction("action",( Icallback )repaint_cb);
  IupSetFunction("motion_cb",( Icallback )motion_cb);
  IupSetFunction("button_cb",( Icallback )button_cb);
  IupSetFunction("item",( Icallback )item);
  IupSetFunction("pos",( Icallback )pos);

  menu = IupMenu(
      IupItem("item1", "item"),
      IupItem("item2", "item"),
      IupItem("item3", "item"),
      IupItem("item4", "item"),
      IupSubmenu ("Submenu", 
        IupMenu(
              IupItem("sub-item1", "item"),
              IupItem("sub-item2", "item"),
              IupItem("sub-item3", "item"),
              IupItem("sub-item4", "item"),
              NULL)
        ),
      NULL);
  
  /* Creates a canvas associated with the repaint action */
  cnvs = IupCanvas("action") ;
  bt = IupButton("position", "pos");
  IupSetAttribute(bt, IUP_EXPAND, IUP_YES);
  
  /* Sets size, mininum and maximum values, position and size of the thumb   */
  /* of the horizontal scrollbar of the canvas                               */
  IupSetAttributes(cnvs, "SIZE=100x100, XMIN=0, XMAX=99, POSX=0, DX=10,"
                         " BORDER=YES, MOTION_CB=motion_cb, BUTTON_CB=button_cb");

  /* Creates a dialog with a frame containing a canvas */
  dlg = IupDialog(IupHbox(IupVbox(bt, NULL), cnvs, NULL));
  
  /* Sets the dialog's title, so that it is mapped properly */
  IupSetAttribute(dlg, IUP_TITLE, "IupCanvas & Canvas Draw");
  
  /* Maps the dialog. This must be done before the creation of the CD canvas */
  IupMap(dlg);
  
  /* Creates a CD canvas of type CD_IUP */ 
  cdcanvas = cdCreateCanvas(CD_IUP, cnvs);
   
  /* Shows dialog on the center of the screen */
  IupShowXY(dlg, 100, 100);

  /* Initializes IUP main loop */
  IupMainLoop();

  /* Finishes IUP */
  IupClose();  

  /* Program finished sucessfully */
  return 0 ;
}
