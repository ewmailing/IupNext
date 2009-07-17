#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "iup.h"
#include "iupcontrols.h"

static char data[3][3][50] = 
{
  {"1:1", "1:2", "1:3"}, 
  {"2:1", "2:2", "2:3"}, 
  {"3:1", "3:2", "3:3"}, 
};


static char* value_cb(Ihandle *self, int lin, int col)
{
  if (lin == 0 || col == 0)
    return NULL;
  return data[lin-1][col-1];
}

static int value_edit_cb(Ihandle *self, int lin, int col, char* newvalue)
{
  strcpy(data[lin-1][col-1], newvalue);
  return IUP_DEFAULT;
}

static Ihandle* create_matrix(void)
{
  Ihandle* mat = IupMatrix(NULL); 
  
  IupSetAttribute(mat, "NUMCOL", "3"); 
  IupSetAttribute(mat, "NUMLIN", "3"); 
  
  IupSetAttribute(mat, "NUMCOL_VISIBLE", "3");
  IupSetAttribute(mat, "NUMLIN_VISIBLE", "3");
  
//  IupSetAttribute(mat, "HEIGHT0", "10");

//  IupSetAttribute(mat, "WIDTH2", "90");
//  IupSetAttribute(mat, "HEIGHT2", "30");
//  IupSetAttribute(mat, "WIDTHDEF", "34");
//  IupSetAttribute(mat,"RESIZEMATRIX", "YES");
  IupSetAttribute(mat,"SCROLLBAR", "NO");
  IupSetCallback(mat,"VALUE_CB",(Icallback)value_cb);
  IupSetCallback(mat,"VALUE_EDIT_CB",(Icallback)value_edit_cb);


  //IupSetAttribute(mat, "NUMCOL_VISIBLE_LAST", "YES");
  //IupSetAttribute(mat, "NUMLIN_VISIBLE_LAST", "YES");
//  IupSetAttribute(mat, "WIDTHDEF", "15");

  return mat;
}

void MatrixCbModeTest(void)
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

  MatrixCbModeTest();

  IupMainLoop();

  IupClose();

  return EXIT_SUCCESS;
}
#endif
