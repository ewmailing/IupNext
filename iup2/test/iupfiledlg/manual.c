

#include <stdlib.h>
#include <iup.h>


int main(int argc, char **argv){  
  Ihandle *filedlg;   
  IupOpen(&argc, &argv);  
  IupSetLanguage(IUP_ENGLISH);    
  filedlg = IupFileDlg();   
  //filedlg = IupCreate("filedlg");

  IupSetAttributes(filedlg, "DIALOGTYPE = DIR, TITLE = \"Dialog title\"");  
  IupSetAttributes(filedlg, "FILTER = \"*.bmp\", FILTERINFO = \"Bitmap Files\"");	
  //IupSetAttribute(filedlg, "STARTDIR", "/home/n/mark/");	
  IupSetAttribute(filedlg, "DIRECTORY", "v:\\tecgraf");  
  //IupSetAttribute(filedlg, IUP_NOOVERWRITEPROMPT, "YES");
  IupPopup(filedlg, IUP_CENTER, IUP_CENTER); 
  switch(IupGetInt(filedlg, IUP_STATUS))  
  {    
  case 1:       
    IupMessage("New file",IupGetAttribute(filedlg, IUP_VALUE));       
    break;     
  case 0:      
    IupMessage("File already exists",IupGetAttribute(filedlg, IUP_VALUE));      
    break;     
  case -1:      
    IupMessage("IupFileDlg","Operation Canceled");      
    return 1;      
    break;   
  }  
  
  IupMainLoop();  
  IupClose();   
  return 0;
}