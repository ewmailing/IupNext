#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "iup.h"
#include "iupmatrix.h"

static Ihandle *matrixHandle = 0;


Ihandle *create_mat(void)
{
  int i, j;
  Ihandle *mat = IupMatrix(NULL); 
  
  IupSetAttribute(mat,IUP_NUMCOL,"2"); 
  IupSetAttribute(mat,IUP_NUMLIN,"100"); 
  
  IupSetAttribute(mat,IUP_NUMCOL_VISIBLE,"2") ;
  IupSetAttribute(mat,IUP_NUMLIN_VISIBLE,"20") ;

  for(i = 0; i < 100; i++)
  {
    for(j = 0; j <= 2; j++)
    {
      char pos[10];
      char pos2[10];
      sprintf(pos, "%d:%d", i, j);
      strcpy(pos2, pos);
      IupStoreAttribute(mat, pos, pos2);
    }
  }

  IupSetAttribute(mat,"WIDTHDEF","34");

  return mat;
}


int matrixClickCallback(Ihandle *handle, int row, int column, char *r)
{
  char str[1000];
  char buf[10];

  sprintf(buf, "%d", row);
  sprintf(str, "Deleting row %s...", buf);
  //IupMessage("Delete Row", str);

  IupSetAttribute(matrixHandle, IUP_DELLIN, buf);

  return IUP_DEFAULT;
}

int main(int argc, char **argv)
{
  Ihandle *dlg;
  IupOpen(&argc, &argv);       
  IupMatrixOpen();

  matrixHandle = create_mat();
  IupSetAttribute(matrixHandle, IUP_CLICK_CB, "CLICK_ACTION");
  IupSetFunction("CLICK_ACTION", (Icallback)matrixClickCallback);

  dlg = IupDialog(matrixHandle);

  IupShowXY (dlg,IUP_CENTER,IUP_CENTER) ;
  IupMainLoop () ;
  IupClose () ;  
  return 0 ;
}
