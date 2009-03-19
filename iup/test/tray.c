#include <stdlib.h>
#include <stdio.h>
#include "iup.h"

static unsigned char pixmap [ ] = 
{
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,1,2,3,3,3,3,3,3,3,2,1,0,0,0, 
  0,0,2,1,2,3,3,3,3,3,2,1,2,0,0,0, 
  0,0,3,2,1,2,3,3,3,2,1,2,3,0,0,0,
  0,0,3,3,2,1,2,3,2,1,2,3,3,0,0,0, 
  0,0,3,3,3,2,1,2,1,2,3,3,3,0,0,0, 
  0,0,3,3,3,3,2,1,2,3,3,3,3,0,0,0, 
  0,0,3,3,3,2,1,2,1,2,3,3,3,0,0,0, 
  0,0,3,3,2,1,2,3,2,1,2,3,3,0,0,0, 
  0,0,3,2,1,2,3,3,3,2,1,2,3,0,0,0, 
  0,0,2,1,2,3,3,3,3,3,2,1,2,0,0,0, 
  0,0,1,2,3,3,3,3,3,3,3,2,1,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};

static int close(void)
{
  return IUP_CLOSE;
}

static int showmenu(void)
{
  Ihandle* menu = IupMenu(IupItem("Exit", "close"), NULL);
  IupSetFunction("close", (Icallback) close);
  IupPopup(menu, IUP_MOUSEPOS, IUP_MOUSEPOS);
  IupDestroy(menu);
  return IUP_DEFAULT;
}

static int hidedialog(Ihandle* ih)
{
  IupSetAttribute(IupGetDialog(ih), "HIDETASKBAR", "YES");  
  return IUP_IGNORE;
}

static int trayclick(Ihandle *ih, int button, int pressed, int dclick)
{
printf("trayclick_cb(button=%d, pressed=%d, dclick=%d)\n", button, pressed, dclick);
  if (button == 1 && pressed)
    IupSetAttribute(ih, "HIDETASKBAR", "NO");  
//  else if (button == 3 && pressed)
//    showmenu();
  return IUP_DEFAULT;
}

void TrayTest(void)
{
  Ihandle *dlg, *img;

  img = IupImage(16, 16, pixmap);
  IupSetAttribute(img, "0", "BGCOLOR");
  IupSetAttribute(img, "1", "255 255 0");
  IupSetAttribute(img, "2", "255 0 0"); 
  IupSetAttribute(img, "3", "255 255 0");
  IupSetHandle ("img", img);

  dlg = IupDialog(NULL);

  IupSetAttribute(dlg, "TITLE", "Tray Test");
  IupSetAttribute(dlg, "TRAY", "YES");
  IupSetAttribute(dlg, "TRAYTIP", "Tip at Tray");
  IupSetAttribute(dlg, "TRAYIMAGE", "img");
  IupSetAttribute(dlg, "SIZE", "100x100");
  IupSetCallback(dlg, "TRAYCLICK_CB", (Icallback)trayclick);
  IupSetCallback(dlg, "CLOSE_CB", (Icallback)hidedialog);
  IupSetCallback(dlg, "K_ESC", (Icallback)close);

  IupShowXY(dlg, IUP_CENTER, IUP_CENTER);

  /* start only the task bar icon */
  IupSetAttribute(dlg, "HIDETASKBAR", "YES");  
}


#ifndef BIG_TEST
int main(int argc, char* argv[])
{
  IupOpen(&argc, &argv);

  TrayTest();

  IupMainLoop();

  IupClose();

  return EXIT_SUCCESS;
}
#endif
