/*IupMenu Example in C 
Creates a dialog with a menu with two submenus. */

#include <stdio.h>
#include "iup.h"

int exit_cb(void)
{
  return IUP_CLOSE;
}

void main(int argc, char **argv)
{
  Ihandle *dg, *menu, *item_save, *item_open, *item_undo, *item_exit, *file_menu;
  Ihandle *sub1_menu;

  IupOpen(&argc, &argv);

  item_open = IupItem ("Open", NULL);
  IupSetAttribute(item_open, IUP_KEY, "O");

  item_save = IupItem ("Save", NULL);
  IupSetAttribute(item_save, IUP_KEY, "S");

  item_undo = IupItem ("Undo", NULL);
  IupSetAttribute(item_undo, IUP_KEY, "U");
  IupSetAttribute(item_undo, IUP_ACTIVE, IUP_NO);

  item_exit = IupItem ("Exit", NULL);
  IupSetAttribute(item_exit, IUP_KEY, "x");
  IupSetCallback(item_exit, "ACTION", (Icallback)exit_cb);

  file_menu = IupMenu(item_open, 
                      item_save, 
                      IupSeparator(),
                      item_undo,
                      item_exit,
                      NULL);

  sub1_menu = IupSubmenu("File", file_menu);

  menu = IupMenu(sub1_menu, NULL);
  IupSetHandle("mymenu", menu);
  dg = IupDialog(IupCanvas(""));
  IupSetAttribute(dg, "MENU", "mymenu");

  IupShow(dg);
  IupMainLoop();
  IupDestroy(dg);
  IupClose();
}
