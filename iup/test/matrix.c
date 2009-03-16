#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "iup.h"
#include "iupcontrols.h"

Ihandle* create_matrix(void)
{
  Ihandle* mat = IupMatrix(NULL); 
  
  IupSetAttribute(mat, "NUMCOL", "15"); 
  IupSetAttribute(mat, "NUMLIN", "20"); 
  
//  IupSetAttribute(mat, "NUMCOL_VISIBLE", "2"); /* default is 4 */
//  IupSetAttribute(mat, "NUMLIN_VISIBLE", "3"); /* default is 3 */
  
  IupSetAttribute(mat, "0:0", "Inflation");
  IupSetAttribute(mat, "1:0", "Medicine\nFarma");
  IupSetAttribute(mat, "2:0", "Food"); 
  IupSetAttribute(mat, "3:0", "Energy"); 
  IupSetAttribute(mat, "0:1", "January 2000"); 
  IupSetAttribute(mat, "0:2", "February 2000"); 
  IupSetAttribute(mat, "1:1", "5.6\n3.33");
  IupSetAttribute(mat, "2:1", "2.2");
  IupSetAttribute(mat, "3:1", "7.2");
  IupSetAttribute(mat, "1:2", "4.5");
  IupSetAttribute(mat, "2:2", "8.1");
  IupSetAttribute(mat, "3:2", "3.4");
  IupSetAttribute(mat, "WIDTH2", "90");
  IupSetAttribute(mat, "HEIGHT2", "30");
//  IupSetAttribute(mat, "WIDTHDEF", "34");
//  IupSetAttribute(mat,"RESIZEMATRIX", "YES");
  //IupSetAttribute(mat,"SCROLLBAR", "NO");

  //IupSetAttribute(mat, "NUMCOL_VISIBLE_END", "YES");
  //IupSetAttribute(mat, "NUMLIN_VISIBLE_END", "YES");
  IupSetAttribute(mat, "WIDTHDEF", "15");
  IupSetAttribute(mat, "20:15", "The End");
  IupSetAttribute(mat, "10:0", "Middle Line");
  IupSetAttribute(mat, "15:0", "Middle Line");
  IupSetAttribute(mat, "0:7", "Middle Column");
  IupSetAttribute(mat, "0:11", "Middle Column");
  IupSetAttribute(mat, "20:0", "Line Title Test");
  IupSetAttribute(mat, "0:15", "Column Title Test");
  IupSetAttribute(mat, "NUMCOL_VISIBLE", "6");
  IupSetAttribute(mat, "NUMLIN_VISIBLE", "8");

  return mat;
}

int main(int argc, char* argv[])
{
  Ihandle* dlg, *box;

  IupOpen(&argc, &argv);
  IupControlsOpen();

  box = IupVbox(create_matrix(), NULL);
  IupSetAttribute(box, "MARGIN", "10x10");

  dlg = IupDialog(box);
  IupSetAttribute(dlg, "TITLE", "IupMatrix Simple Test");
  IupShowXY(dlg, IUP_CENTER, IUP_CENTER);

  IupMainLoop();
  IupDestroy(dlg);
  IupClose();  

  return 0;
}
