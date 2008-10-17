#include <stdlib.h>
#include <stdio.h>
#include <iup.h>

int main(int argc, char **argv)
{
  IupOpen(&argc, &argv);      
  IupMessagef("Globals", 
    "  System: %s\n"
    "  System Version: %s\n"
    "  Motif Version: %s\n"
    "  Motif Number: %s\n"
    "  Screen Size: %s\n"
    "  Screen Depth: %s\n",
    IupGetGlobal("SYSTEM"),
    IupGetGlobal("SYSTEMVERSION"),
    IupGetGlobal("MOTIFVERSION")? IupGetGlobal("MOTIFVERSION"): "-----",
    IupGetGlobal("MOTIFNUMBER")? IupGetGlobal("MOTIFNUMBER"): "-----",
    IupGetGlobal("SCREENSIZE"),
    IupGetGlobal("SCREENDEPTH"));
  IupClose();  
  return 0;
}
