#include <stdio.h>
#include "iup.h"

static char pixmap [ ] = 
{
  1,2,3,3,3,3,3,3,3,2,1,1,1,1,1,1, 
  2,1,2,3,3,3,3,3,2,1,2,1,1,1,1,1, 
  3,2,1,2,3,3,3,2,1,2,3,1,1,1,1,1,
  3,3,2,1,2,3,2,1,2,3,3,1,1,1,1,1, 
  3,3,3,2,1,2,1,2,3,3,3,1,1,1,1,1, 
  3,3,3,3,2,1,2,3,3,3,3,1,1,1,1,1, 
  3,3,3,2,1,2,1,2,3,3,3,1,1,1,1,1, 
  3,3,2,1,2,3,2,1,2,3,3,1,1,1,1,1, 
  3,2,1,2,3,3,3,2,1,2,3,1,1,1,1,1, 
  2,1,2,3,3,3,3,3,2,1,2,1,1,1,1,1, 
  1,2,3,3,3,3,3,3,3,2,1,1,1,1,1,1,
  1,2,3,3,3,3,3,3,3,2,1,1,1,1,1,1,
  1,2,3,3,3,3,3,3,3,2,1,1,1,1,1,1,
  1,2,3,3,3,3,3,3,3,2,1,1,1,1,1,1,
  1,2,3,3,3,3,3,3,3,2,1,1,1,1,1,1,
  1,2,3,3,3,3,3,3,3,2,1,1,1,1,1,1
};

int menuexit(void)
{
  return IUP_CLOSE;
}

int showmenu(void)
{
  Ihandle* menu = IupMenu(IupItem("Exit", "menuexit"), NULL);
  IupPopup(menu, IUP_MOUSEPOS, IUP_MOUSEPOS);
  IupDestroy(menu);
  return IUP_DEFAULT;
}

int hidedialog(Ihandle* ih)
{
  IupSetAttribute(IupGetDialog(ih), "HIDETASKBAR", "YES");  
  return IUP_DEFAULT;
}

int trayclick(Ihandle *ih, int button, int pressed)
{
  if (button == 1 && pressed)
    IupSetAttribute(ih, "HIDETASKBAR", "NO");  
  else if (button == 3 && pressed)
    showmenu();
  return IUP_DEFAULT;
}

int main(int argc, char **argv)
{
  Ihandle *dlg;
  Ihandle *img;

  IupOpen(&argc, &argv);

  IupSetFunction("menuexit", (Icallback) menuexit);
  IupSetFunction("trayclick", (Icallback) trayclick);
  IupSetFunction("showmenu", (Icallback) showmenu);
  IupSetFunction("hidedialog", (Icallback) hidedialog);

  img = IupImage(16, 16, pixmap);
  IupSetAttribute(img, "1", "0 1 0");
  IupSetAttribute(img, "2", "255 0 0"); 
  IupSetAttribute(img, "3", "255 255 0");
  IupSetHandle ("img", img);

  dlg = IupDialog
        (
          IupVbox
          (
            IupButton("Show Popup Menu Here", "showmenu"),
            IupButton("Hide to TaskBar", "hidedialog"),
            NULL
          )
        );

  IupSetAttributes(dlg, "TITLE=\"Tray Example\", GAP=5, MARGIN=10x10");

  IupSetAttribute(dlg, "TRAY", "YES");
  IupSetAttribute(dlg, "TRAYTIP", "Tip at Tray");
  IupSetAttribute(dlg, "TRAYIMAGE", "img");
  IupSetAttribute(dlg, "TRAYCLICK_CB", "trayclick");

  IupShowXY(dlg, IUP_CENTER, IUP_CENTER);

  /* when you hide using HIDETASKBAR, you must show using HIDETASKBAR also. */
  IupSetAttribute(dlg, "HIDETASKBAR", "YES");  

  IupMainLoop();

  IupSetAttribute(dlg, "TRAY", "NO"); /* refresh tray before destroy */
  IupDestroy(dlg);
  IupDestroy(img);

  IupClose();
  return 0 ;
}

