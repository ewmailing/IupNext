
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "iup.h"
#include "iupkey.h"
#include "iupmatrix.h"
#include "cd.h"
#include "cdiup.h"


// ---------------------------------------------------------------------------
// Defines.
//

#define _SPRINGSTIFF_CPP_


// ---------------------------------------------------------------------------
// Global variables and symbols.
//

static int col = 2;
static int lin = 1;
static unsigned int limit_dimen = 2;


// ---------------------------------------------------------------------------
// User-interface object variables. Declares global IUP handles.
//

static Ihandle *dlg, *frm, *mtrx;


// ---------------------------------------------------------------------------
// Prototype of local functions.
//

static void _ssInitMenu ( void );
static void _ssInitMtrx ( void );
static int  _ssUpdate   ( void );
static void _ssChangeLin( char* mode, int pos );
static void _ssAlign    ( char* pos );


// ---------------------------------------------------------------------------
// Prototype of user-interface callback functions.
//

static int _ssAddLineAbove_CB( void );
static int _ssAddLineBelow_CB( void );
static int _ssDelLineAbove_CB( void );
static int _ssDelLineBelow_CB( void );
static int _ssAlignLeft_CB   ( void );
static int _ssAlignCent_CB   ( void );
static int _ssAlignRight_CB  ( void );


// ---------------------------------------------------------------------------
// Local functions.
//


// ----------------------------- _ssInitMenu ---------------------------------
// 
static void _ssInitMenu( void )
{
  Ihandle  *addlinabove;
  Ihandle  *addlinbelow;

  Ihandle  *dellinabove;
  Ihandle  *dellinbelow;

  Ihandle  *alinleft;
  Ihandle  *alincent;
  Ihandle  *alinright;

  Ihandle  *add_menu;
  Ihandle  *add;

  Ihandle  *del_menu;
  Ihandle  *del;

  Ihandle  *alignment_menu;
  Ihandle  *alignment;

  Ihandle  *change_menu; 
 
  Ihandle  *change_submenu;
  Ihandle  *main_menu;

  // Creating the items that compose the ‘add’ menu.
  addlinabove = IupItem("Line above", "addlinabove_act");
  addlinbelow = IupItem("Line below", "addlinbelow_act");

  // Creating the items that compose the ‘delete’ menu.
  dellinabove = IupItem("Line above", "dellinabove_act");
  dellinbelow = IupItem("Line below", "dellinbelow_act");

  // Creating the items that compose the ‘alignment’ menu.
  alinleft = IupItem("Left", "alinleft_act");
  alincent = IupItem("Centered", "alincent_act");
  alinright = IupItem("Right", "alinright_act");

  // Creating the ‘add’ submenu.
  add_menu = IupMenu(addlinabove, addlinbelow, NULL);
  add = IupSubmenu("Add", add_menu);

  // Creating the ‘delete’ submenu.
  del_menu = IupMenu(dellinabove, dellinbelow, NULL);
  del = IupSubmenu("Delete", del_menu);

  // Creating the ‘alignment’ submenu.
  alignment_menu = IupMenu( alinleft, alincent, alinright, NULL);
  alignment = IupSubmenu("Alignment", alignment_menu);

  // Creating the ‘change’ menu.
  change_menu = IupMenu(add, del, alignment, NULL); 
 
  // Creating the bar menu.
  change_submenu = IupSubmenu("Change", change_menu);
  main_menu   = IupMenu(change_submenu, NULL);

  // Adding a dialog title.
  IupSetAttribute(dlg, IUP_TITLE, "Systruct");
 
  // Associating the menu to the dialog.
  IupSetHandle("main_menu_handle", main_menu);
  IupSetAttribute(dlg, IUP_MENU, "main_menu_handle");

  // Registering callbacks.
  IupSetFunction("addlinabove_act", (Icallback) _ssAddLineAbove_CB);
  IupSetFunction("addlinbelow_act", (Icallback) _ssAddLineBelow_CB);

  IupSetFunction("dellinabove_act", (Icallback) _ssDelLineAbove_CB);
  IupSetFunction("dellinbelow_act", (Icallback) _ssDelLineBelow_CB);

  IupSetFunction("alinleft_act", (Icallback) _ssAlignLeft_CB);
  IupSetFunction("alincent_act", (Icallback) _ssAlignCent_CB);
  IupSetFunction("alinright_act", (Icallback) _ssAlignRight_CB);
}


// ----------------------------- _ssInitMtrx ---------------------------------
// 
static void _ssInitMtrx( void )
{
  mtrx = IupMatrix(NULL); 
  frm = IupFrame(mtrx);

  // Sets the number of lines and columns in the matrix. 
  IupSetAttribute(mtrx, IUP_NUMCOL, "2"); 
  IupSetAttribute(mtrx, IUP_NUMLIN, "1"); 

  // Sets the number of visible lines and columns.
  IupSetAttribute(mtrx, IUP_NUMCOL_VISIBLE, "2");
  IupSetAttribute(mtrx, IUP_NUMLIN_VISIBLE, "1");

  // Configures the matrix to mark each cell.
  IupSetAttribute(mtrx, IUP_MARK_MODE, "CELL");

  // Defines if more than one entity defined by MARK_MODE can be marked.
  IupSetAttribute(mtrx, IUP_MULTIPLE, IUP_YES);

  // Defines if the width of a column can be interactively changed.
  IupSetAttribute(mtrx, IUP_RESIZEMATRIX, IUP_YES);

  // Defines the column width.
  IupSetAttribute(mtrx, IUP_WIDTHDEF, "60");
  IupSetAttribute(mtrx, IUP_WIDTH"0", "30");

  // Defines the alignment of the cells in column.
  IupSetAttribute(mtrx, IUP_ALIGNMENT"0", IUP_ACENTER);

  // Sets the title between lines and columns.
  IupSetAttribute(mtrx, "0:0", "Point");

  // Sets line titles. 
  IupSetAttribute(mtrx, "1:0", "1");

  // Sets column titles. 
  IupSetAttribute(mtrx, "0:1", "Rotation (X)"); 
  IupSetAttribute(mtrx, "0:2", "Moment (Y)"); 

  // Sets cell values. 
  IupSetAttribute(mtrx, "1:1", "0.10");
  IupSetAttribute(mtrx, "1:2", "0.00");
}


// ------------------------------ _ssUpdate ----------------------------------
// 
static int _ssUpdate( void )
{
  int    limit_title = 10;
  char   *value  = NULL;
  char   *string = NULL;
  int    i;

  if(lin > 99)
  {
    IupMessage("Systruc","The maximum number (99) of points was exceeded.");
    return(0);
  }

  value  = (char  *)calloc(2+1,sizeof(char));               // xx + NULL.
  string = (char  *)calloc(2+strlen(":")+1+1,sizeof(char)); // xx:0 + NULL.

  for(i = 1; i <= lin; i++)
  {
    sprintf(value, "%d", i);       
    strcpy(string, value);
    strcat(string, ":0");
    IupStoreAttribute(mtrx, string, value);
  }
  free(string);
  free(value);

  return(1);
}


// ----------------------------- _ssChangeLin --------------------------------
//
static void _ssChangeLin( char* mode, int pos )
{
  if(IupGetAttribute(mtrx, IUP_MARKED) == NULL)
  {
    IupMessage("Systruct", "There are not marked cells");
  }
  else if((strcmp(mode, IUP_DELLIN) == 0) && (lin == 1))
  {
    IupMessage("Systruct", "Last line can not be removed.");
  }
  else
  {
    int i = 0 , l = pos;
    char* marked;
    char* temp;
    marked = (char*)malloc((sizeof(char)*lin*col)+1);
    strcpy(marked, IupGetAttribute(mtrx,IUP_MARKED));
    temp = (char*)malloc((sizeof(char)*limit_dimen)+1);
        
    while(i < col*lin && marked[i] == '0')
      i++;

    if((((strcmp(mode,IUP_DELLIN) == 0) && (pos == 0) && (i/col+pos == 0))) ||
       (((strcmp(mode,IUP_DELLIN) == 0) && (pos == 1) && (i/col+pos == lin))))
    {
      IupMessage("Systruct", "Line is out of the matrix.");
    }
    else
    {
      if(strcmp(mode, IUP_ADDLIN) == 0)
      {
        sprintf(temp, "%d", i/col+pos);
        IupSetAttribute(mtrx, mode, temp);
              lin++;
      }
      else if (strcmp(mode,IUP_DELLIN) == 0 && pos == 1)
      {
        sprintf(temp, "%d", i/col+pos+1);
        IupSetAttribute(mtrx, mode, temp);
        lin--;
      }
      else
      {
        sprintf(temp, "%d", i/col+pos);
        IupSetAttribute(mtrx, mode, temp);
        lin--;
      }
    }
    free(temp);
    free(marked);

    if(!_ssUpdate())    
      _ssChangeLin(IUP_DELLIN, pos);
  }
}


// ------------------------------- _ssAlign ----------------------------------
//
static void _ssAlign( char* pos )
{
  if(IupGetAttribute(mtrx, IUP_MARKED) == NULL)
  {
    IupMessage("Alignment", "Não há células marcadas");
  }
  else
  {
    int i = 0 , c = 1;
    char  *marked;
    char  *string;
    char  *temp;
    marked = (char *)malloc(sizeof(char)*(lin*col)+1);
    strcpy(marked, IupGetAttribute(mtrx,IUP_MARKED));
    string = (char *)malloc((sizeof(char)*strlen("ALIGNMENT")+limit_dimen)+1);
    temp = (char *)malloc((sizeof(char)*limit_dimen)+1);
        
    while(i < col*lin)
    {
      if(marked[i] == '1') 
      {
        sprintf(temp, "%d", c);
        strcpy(string, "ALIGNMENT");
        strcat(string, temp);
        IupSetAttribute(mtrx, string, pos);
      }
     i++;
     c++;
     if(c == col+1) c=1;
    }
    free(temp);
    free(string);
    free(marked);
  }  
}


// ---------------------------------------------------------------------------
// Callback functions.
//


// -------------------------- _ssAddLineAbove_CB -----------------------------
//
static int _ssAddLineAbove_CB( void )
{
  _ssChangeLin(IUP_ADDLIN, 0);
  
  return(IUP_DEFAULT);
}


// -------------------------- _ssAddLineBelow_CB -----------------------------
//
static int _ssAddLineBelow_CB( void )
{
  _ssChangeLin(IUP_ADDLIN, 1);
  
  return(IUP_DEFAULT);
}


// -------------------------- _ssDelLineAbove_CB -----------------------------
//
static int _ssDelLineAbove_CB( void )
{
  _ssChangeLin(IUP_DELLIN, 0);
  
  return(IUP_DEFAULT);
}


// -------------------------- _ssDelLineBelow_CB -----------------------------
//
static int _ssDelLineBelow_CB( void )
{
  _ssChangeLin(IUP_DELLIN, 1);
  
  return(IUP_DEFAULT);
}


// ---------------------------- _ssAlignLeft_CB ------------------------------
//
static int _ssAlignLeft_CB( void )
{
  _ssAlign(IUP_ALEFT);

  return(IUP_DEFAULT);
}


// -------------------------- _ssAddLineAbove_CB -----------------------------
//
static int _ssAlignCent_CB( void )
{
  _ssAlign(IUP_ACENTER);

  return(IUP_DEFAULT);
}


// -------------------------- _ssAddLineAbove_CB -----------------------------
//
static int _ssAlignRight_CB( void )
{
  _ssAlign(IUP_ARIGHT);

  return(IUP_DEFAULT);
}


// ---------------------------------------------------------------------------
// Global functions.
//


// ----------------------------------- main ----------------------------------
// Main program.
//

int main(int argc, char **argv)
{
  // Initializes IUP.
  IupOpen(&argc, &argv);       

  // Initializes the matrix library; must be called only once during the
  // program.
  IupMatrixOpen();  

  // Initializes the matrix.
  _ssInitMtrx();

  // Places the matrix in the dialog.
  dlg = IupDialog(frm);

  // Initializes the menu.
  _ssInitMenu();
  
  // Shows dialog in the center of the screen.
  IupShowXY(dlg, IUP_CENTER, IUP_CENTER);

  printf("%s %f\n", IupGetAttribute(mtrx, "1:1"), IupGetFloat(mtrx, "1:1"));
  
  // Initializes IUP main loop.
  IupMainLoop();

  // Finishes IUP_MATRIX.
  IupMatrixClose();

  // Finishes IUP.
  IupClose();  

  // Program finished sucessfully.
  return(0);
}

