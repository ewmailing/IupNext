/* IupScanf Example in C 
   Shows a dialog with three fields to be filled. 
   One receives a string, the other receives a real number and 
   the last receives an integer number. */

#include <stdio.h>
#include "iup.h"

int main(int argc, char **argv)
{
  int fields_read;
  int integer = 12;
  float real = 1e-3;
  char text[300]="This is a vector of characters";
  char string[350];
  char *fmt =
  {
   "Data entering example\n"
   "text = %300.40%s\n"
   "real = %20.10%g\n"
   "integer = %20.10%d\n"
  };
  
  IupOpen(&argc, &argv);
  IupSetLanguage(IUP_ENGLISH);
  fields_read = IupScanf (fmt, text, &real, &integer);
  
  if(fields_read == -1)
  { 
    IupMessage("IupScanf","Operation canceled");
  }
  else
  {
    sprintf(string,"Text: %s\nReal: %f\nInteger: %d\nFields read sucessfully: %d",text,real,integer,fields_read);
    IupMessage("IupScanf",string); 	  
  }

  IupClose();

  return 0;
}
