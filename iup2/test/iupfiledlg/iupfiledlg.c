#include <stdio.h>
#include <iup.h>

int main(int argc, char **argv)
{
  Ihandle *filedlg; 
  IupOpen(&argc, &argv);
  
  filedlg = IupFileDlg(); 
 
  IupSetAttributes(filedlg, "DIALOGTYPE = SAVE, TITLE = \"File Save\"");
  IupSetAttributes(filedlg, "FILTER = \"*.bmp\", FILTERINFO = \"Bitmap Files\"");
//  IupSetAttributes(filedlg, "FILE = \"\\tecgraf\\iup\\test.bmp\""); // OK
//  IupSetAttributes(filedlg, "FILE = \"/tecgraf/iup/test.bmp\""); // OK
//  IupSetAttributes(filedlg, "FILE = \"test.bmp\", DIRECTORY = \"/tecgraf/iup\"");   // OK
//  IupSetAttributes(filedlg, "FILE = \"test.bmp\", DIRECTORY = \"\\tecgraf\\iup\"");  // OK

  IupPopup(filedlg, IUP_CENTER, IUP_CENTER); 

  switch(IupGetInt(filedlg, IUP_STATUS))
  {
    case 1: 
      IupMessage("New file",IupGetAttribute(filedlg, IUP_VALUE)); 
      break; 

    case 0 : 
      IupMessage("File already exists",IupGetAttribute(filedlg, IUP_VALUE));
      break; 

    case -1 : 
      IupMessage("IupFileDlg","Operation Canceled");
      return 1;
      break; 
  }

  IupDestroy(filedlg);
  IupClose(); 
  return 0;
}