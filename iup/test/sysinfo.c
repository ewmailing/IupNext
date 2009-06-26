#include <stdlib.h>
#include <stdio.h>
#include "iup.h"

void SysInfoTest(void)
{
  char* sysver;
  printf("IUP System Info:\n");
  printf("  Version: %s\n", IupVersion());
  printf("  Copyright: %s\n", IUP_COPYRIGHT);
  printf("  Driver: %s\n", IupGetGlobal("DRIVER"));
  printf("  System: %s\n", IupGetGlobal("SYSTEM"));
  printf("  System Version: %s\n\n", IupGetGlobal("SYSTEMVERSION"));
  printf("  System Language: %s\n\n", IupGetGlobal("SYSTEMLANGUAGE"));

  sysver = IupGetGlobal("MOTIFVERSION");
  if (sysver) 
  {
    printf("  Motif Version: %s\n", sysver);
    printf("  Motif Number: %s\n", IupGetGlobal("MOTIFNUMBER"));
    printf("  X Server Vendor: %s\n", IupGetGlobal("XSERVERVENDOR"));
    printf("  X Vendor Release: %s\n\n", IupGetGlobal("XVENDORRELEASE"));
  }

  sysver = IupGetGlobal("GTKVERSION");
  if (sysver) 
    printf("  GTK Version: %s\n", sysver);

  printf("  Screen Depth: %s\n", IupGetGlobal("SCREENDEPTH"));
  printf("  Screen Size: %s\n", IupGetGlobal("SCREENSIZE"));
  printf("  Full Screen Size: %s\n", IupGetGlobal("FULLSIZE"));
  printf("  True Color Canvas: %s\n\n", IupGetGlobal("TRUECOLORCANVAS"));

  printf("  Computer Name: %s\n", IupGetGlobal("COMPUTERNAME"));
  printf("  User Name: %s\n", IupGetGlobal("USERNAME"));
  printf("  Default Font: %s\n", IupGetGlobal("DEFAULTFONT"));
}


#ifndef BIG_TEST
int main(int argc, char* argv[])
{
  IupOpen(&argc, &argv);

  SysInfoTest();

  IupClose();

  return EXIT_SUCCESS;
}
#endif
