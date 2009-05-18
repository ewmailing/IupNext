#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "iup.h"
#include "iupcontrols.h"

static int dropcheck_cb(Ihandle *self, int lin, int col)
{
  if (lin == 3 && col == 1)
    return IUP_DEFAULT;
  return IUP_IGNORE;
}

static Ihandle* create_matrix(void)
{
  Ihandle* mat = IupMatrix(NULL); 
  
  IupSetAttribute(mat, "NUMCOL", "15"); 
  IupSetAttribute(mat, "NUMLIN", "20"); 
//  IupSetAttribute(mat, "NUMCOL", "2"); 
//  IupSetAttribute(mat, "NUMLIN", "3"); 
  
  IupSetAttribute(mat, "0:0", "Inflation");
  IupSetAttribute(mat, "1:0", "Medicine\nPharma");
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
  IupSetAttribute(mat, "3:3", "Font Test");
  IupSetAttribute(mat, "WIDTH2", "90");
  IupSetAttribute(mat, "HEIGHT2", "30");
  IupSetAttribute(mat,"SORTSIGN2","DOWN");
//  IupSetAttribute(mat, "WIDTHDEF", "34");
  IupSetAttribute(mat,"MULTILINE", "YES");
  IupSetAttribute(mat,"RESIZEMATRIX", "YES");
  //IupSetAttribute(mat,"SCROLLBAR", "NO");
  //IupSetAttribute(mat, "BGCOLOR1:2", "255 92 255");
  //IupSetAttribute(mat, "BGCOLOR2:*", "92 92 255");
  //IupSetAttribute(mat, "BGCOLOR*:3", "255 92 92");
  //IupSetAttribute(mat, "FGCOLOR1:2", "255 0 0");
  //IupSetAttribute(mat, "FGCOLOR2:*", "0 128 0");
  //IupSetAttribute(mat, "FGCOLOR*:3", "0 0 255");
  //IupSetAttribute(mat, "FONT1:2", "Helvetica, 24");
  //IupSetAttribute(mat, "FONT2:*", "Courier, 14");
  //IupSetAttribute(mat, "FONT*:3", "Times, Bold 14");
  //IupSetAttribute(mat, "ALIGNMENT2", "ARIGHT");
//  IupSetAttribute(mat, "ACTIVE", "NO");

//  IupSetAttribute(mat,"MARKMODE","CELL");
  IupSetAttribute(mat,"MARKMODE","CELL");
  IupSetAttribute(mat,"MARKMULTIPLE","YES");
  IupSetAttribute(mat,"MARKAREA","NOT_CONTINUOUS");
  IupSetAttribute(mat,"MARK2:2","YES");
  IupSetAttribute(mat,"MARK2:3","YES");
  IupSetAttribute(mat,"MARK3:3","YES");


//  IupSetAttribute(mat,"MARKMODE","LINCOL");

  //IupSetAttribute(mat, "NUMCOL_VISIBLE_LAST", "YES");
  //IupSetAttribute(mat, "NUMLIN_VISIBLE_LAST", "YES");
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

  IupSetCallback(mat, "DROPCHECK_CB", (Icallback)dropcheck_cb);

  return mat;
}

void MatrixTest(void)
{
  Ihandle* dlg, *box;

  box = IupVbox(create_matrix(), NULL);
  IupSetAttribute(box, "MARGIN", "10x10");

  dlg = IupDialog(box);
  IupSetAttribute(dlg, "TITLE", "IupMatrix Simple Test");
  IupShowXY(dlg, IUP_CENTER, IUP_CENTER);
}

#ifndef BIG_TEST
int main(int argc, char* argv[])
{
  IupOpen(&argc, &argv);
  IupControlsOpen();

  MatrixTest();

  IupMainLoop();

  IupClose();

  return EXIT_SUCCESS;
}
#endif
