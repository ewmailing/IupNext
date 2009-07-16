#include <stdio.h>
#include "iup.h"

/* defines icon's image */
static char img[] =
{
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,2,
  1,3,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,4,2,
  1,3,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,4,2,
  1,3,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,4,2,
  1,3,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,4,2,
  1,3,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,4,2,
  1,3,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,4,2,
  1,3,5,5,6,6,6,6,6,6,5,5,6,6,5,5,6,6,5,5,6,6,6,6,6,6,6,5,5,5,4,2,
  1,3,5,5,6,6,6,6,6,6,5,5,6,6,5,5,6,6,5,5,6,6,6,6,6,6,6,6,5,5,4,2,
  1,3,5,5,5,5,6,6,5,5,5,5,6,6,5,5,6,6,5,5,6,6,6,5,5,6,6,6,5,5,4,2,
  1,3,5,5,5,5,6,6,5,5,5,5,6,6,5,5,6,6,5,5,6,6,5,5,5,5,6,6,5,5,4,2,
  1,3,5,5,5,5,6,6,5,5,5,5,6,6,5,5,6,6,5,5,6,6,5,5,5,5,6,6,5,5,4,2,
  1,3,5,5,5,5,6,6,5,5,5,5,6,6,5,5,6,6,5,5,6,6,5,5,5,5,6,6,5,5,4,2,
  1,3,5,5,5,5,6,6,5,5,5,5,6,6,5,5,6,6,5,5,6,6,6,5,5,6,6,6,5,5,4,2,
  1,3,5,5,5,5,6,6,5,5,5,5,6,6,5,5,6,6,5,5,6,6,6,6,6,6,6,6,5,5,4,2,
  1,3,5,5,5,5,6,6,5,5,5,5,6,6,5,5,6,6,5,5,6,6,6,6,6,6,6,5,5,5,4,2,
  1,3,5,5,5,5,6,6,5,5,5,5,6,6,5,5,6,6,5,5,6,6,5,5,5,5,5,5,5,5,4,2,
  1,3,5,5,5,5,6,6,5,5,5,5,6,6,5,5,6,6,5,5,6,6,5,5,5,5,5,5,5,5,4,2,
  1,3,5,5,5,5,6,6,5,5,5,5,6,6,5,5,6,6,5,5,6,6,5,5,5,5,5,5,5,5,4,2,
  1,3,5,5,5,5,6,6,5,5,5,5,6,6,5,5,6,6,5,5,6,6,5,5,5,5,5,5,5,5,4,2,
  1,3,5,5,5,5,6,6,5,5,5,5,6,6,5,5,6,6,5,5,6,6,5,5,5,5,5,5,5,5,4,2,
  1,3,5,5,6,6,6,6,6,6,5,5,6,6,6,6,6,6,5,5,6,6,5,5,5,5,5,5,5,5,4,2,
  1,3,5,5,6,6,6,6,6,6,5,5,5,6,6,6,6,5,5,5,6,6,5,5,5,5,5,5,5,5,4,2,
  1,3,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,4,2,
  1,3,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,4,2,
  1,3,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,4,2,
  1,3,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,4,2,
  1,3,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,4,2,
  1,3,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,4,2,
  1,3,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,2,
  1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
};

int quit_cb(void)
{
  return IUP_CLOSE;
}

int kany(Ihandle *self, int c)
{
  printf("%d\n", c);
  return IUP_DEFAULT;
}

int main(int argc, char **argv)
{
  Ihandle *dialog, *quit_bt, *icon;
  Ihandle *menu, *submenu, *options, *list;

  IupOpen(&argc, &argv);

  /* Creating dialog's icon */
  icon = IupImage(32, 32, img);
  IupSetAttribute (icon, "1", "255 255 255");
  IupSetAttribute (icon, "2", "000 000 000");
  IupSetAttribute (icon, "3", "226 226 226");
  IupSetAttribute (icon, "4", "128 128 128");
  IupSetAttribute (icon, "5", "192 192 192");
  IupSetAttribute (icon, "6", "000 000 255");
  IupSetHandle ("icon", icon);

  /* Creating dialog's content */  
  quit_bt = IupButton("Quit", "quit_cb");
  IupSetFunction("quit_cb", (Icallback)quit_cb);
  IupSetHandle("quit", quit_bt);

  /* Creating dialog's menu */  
  options = IupCreatep("menu", IupItem("Exit","quit_cb"),NULL);
  submenu = IupSubmenu("File", options);
  menu = IupCreatep("menu", submenu,NULL);
  IupSetHandle("menu", menu);

  list = IupCreate("list");
  IupSetAttribute(list, "1", "A");
  IupSetAttribute(list, "2", "B");
  IupSetAttribute(list, "SIZE", "200x100");

  /* Creating main dialog */  
  dialog = IupCreatep("dialog", IupVbox(quit_bt, list, NULL), NULL);
  IupSetAttribute(dialog, IUP_TITLE, "IupDialog");
  IupSetAttribute(dialog, IUP_MENU, "menu");
  IupSetAttribute(dialog, IUP_CURSOR, "CROSS");
  IupSetAttribute(dialog, IUP_ICON, "icon");
  IupSetAttribute(dialog, IUP_DEFAULTESC, "quit");
  IupSetAttribute(dialog, IUP_K_ANY, "kany");
  IupSetFunction("kany", (Icallback) kany);
  //IupSetAttribute(dialog, "SIZE", "400x200");

  IupShowXY(dialog, IUP_CENTER, IUP_CENTER);
  
  IupMainLoop();
  IupDestroy(dialog);
  IupClose();

  return 0;
}

