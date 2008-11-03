#include <stdio.h>
#include <stdlib.h>

#include "iup.h"
#include "iupcontrols.h"

void main(int argc, char* argv[])
{
  unsigned char r = 10, g = 100, b = 25;

  IupOpen(&argc, &argv);
  IupControlsOpen();
  
  if (IupGetColor(100, 100, &r, &g, &b))
    printf("RGB = %d, %d, %d  Hex=[%.2X%.2X%.2X]\n", r, g, b, r, g, b);

  IupClose();  
}

