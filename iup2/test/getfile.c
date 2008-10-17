#include <stdio.h>
#include <string.h>
#include "iup.h"

int main(int argc, char **argv)
{
  char file[256];
  IupSetLanguage(IUP_ENGLISH);
  IupOpen(&argc, &argv);

  //strcpy(file, "\\*.*");
  strcpy(file, "d:\\mark\\");

  switch(IupGetFile(file))
  {
    case 1: 
      IupMessage("New file",file);	    
    break ;	    
    
    case 0 : 
      IupMessage("File already exists",file);
    break ;	    
    
    case -1 : 
      IupMessage("IupFileDlg","Operation canceled");
      return 1;
      
    case -2 : 
      IupMessage("IupFileDlg","Allocation error");
      return 1;
      
    case -3 : 
      IupMessage("IupFileDlg","Invalid parameter");
      return 1;
    break ;	    
  }

  IupMainLoop();
  IupClose();

  return 0;
}

