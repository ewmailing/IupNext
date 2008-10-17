
#include <stdio.h>
#include <iup.h>

int main()
{
  char text[1024] = "IupGetText example";
  IupOpen();
  IupGetText("TITLE", text);
  IupMainLoop();
  IupClose();
  return 0;
}

