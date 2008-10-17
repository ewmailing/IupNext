/* IupGetParam Example in C 
   Shows a dialog with all the possible fields. 
*/
#include <stdlib.h>
#include <stdio.h>

#include <iup.h>
#include <iupcontrols.h>

int param_action(Ihandle* dialog, int param_index, void* user_data)
{                
  switch(param_index)
  {
  case -1:
    printf("IupGetParam - OK\n");
    break;
  case -2:
    printf("IupGetParam - Map\n");
    break;
  case -3:
    printf("IupGetParam - Cancel\n");
    break;
  default:
    {
      Ihandle* param;
      char param_str[50];
      sprintf(param_str, "PARAM%d", param_index);
      param = (Ihandle*)IupGetAttribute(dialog, param_str);
      printf("%s = %s\n", param_str, IupGetAttribute(param, IUP_VALUE));
      break;
    }
  }
  return 1;
}
                            
int main(int argc, char **argv)
{
  int pboolean = 1;
  int pinteger = 3456;
  float preal = 3.543f;
  int pinteger2 = 192;
  float preal2 = 0.5f;
  float pangle = 90;
  char pstring[100] = "string text";
  int plist = 2;
  char pstring2[200] = "second text\nsecond line";
  char file_name[500] = "test.jpg";
  
  IupOpen(&argc, &argv);
  IupControlsOpen();
  IupSetLanguage(IUP_ENGLISH);

  if (!IupGetParam("Title", param_action, 0,
                   "Boolean: %b[No,Yes]\n"
                   "Integer: %i\n"
                   "Real 1: %r\n"
                   "Sep1 %t\n"
                   "Integer: %i[0,255]\n"
                   "Real 2: %r[-1.5,1.5,0.05]\n"
                   "Sep2 %t\n"
                   "Angle: %a[0,360]\n"
                   "String: %s\n"
                   "List: %l|item1|item2|item3|\n" 
                   "File: %f[OPEN|*.bmp;*.jpg|CURRENT|NO|NO]\n"
                   "Sep3 %t\n"
                   "Multiline: %m\n",
                   &pboolean, &pinteger, &preal, &pinteger2, &preal2, &pangle, pstring, &plist, file_name, pstring2, NULL))
    return IUP_DEFAULT;
  
  IupMessagef("IupGetParam",
              "Boolean Value: %d\n"
              "Integer: %d\n"
              "Real 1: %g\n"
              "Integer: %d\n"
              "Real 2: %g\n"
              "Angle: %g\n"
              "String: %s\n"
              "List Index: %d\n" 
              "FileName: %s\n"
              "Multiline: %s",
              pboolean, pinteger, (double)preal, pinteger2, (double)preal2, (double)pangle, pstring, plist, file_name, pstring2);

  IupControlsClose();
  IupClose();

  return 0;
}
