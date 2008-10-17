#include <stdio.h>
#include <stdlib.h>

#include "iup.h"
#include "iupcontrols.h"
#include "iupgc.h"

int main(int argc, char **argv)
{
  unsigned char r, g, b;
  
  IupOpen(&argc, &argv);
  IupControlsOpen();
  
  if(IupGetColor(100, 100, &r, &g, &b))
    printf("RGB = %.2X%.2X%.2X\n", r, g, b);

  IupMainLoop();
  IupControlsClose();
  IupClose();  
  return 1;
}

