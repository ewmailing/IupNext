
#include <stdio.h>
#include "iup.h"

int exit_cb()
{
	printf("callback chamada\n");
  return IUP_CLOSE;
}

int open_cb()
{
	printf("callback open chamada\n");
  return IUP_DEFAULT;
}

void open_menu()
{
  Ihandle *item_save, *item_open, *item_undo, *item_exit, *file_menu;
  item_open = IupItem ("Open", "open");
  IupSetAttribute (item_open, IUP_KEY, "O");

  item_save = IupItem ("Save", "save");
  IupSetAttribute (item_save, IUP_KEY, "S");

  item_undo = IupItem ("Undo", "undo");
  IupSetAttribute (item_undo, IUP_KEY, "U");
  IupSetAttribute (item_undo, IUP_ACTIVE, IUP_NO);

  item_exit = IupItem ("Exit", "exit");
  IupSetAttribute (item_exit, IUP_KEY, "x");

  file_menu = IupMenu(item_open, 
                      item_save, 
                      IupSeparator(),
                      item_undo,
                      item_exit,
                      NULL);

  IupPopup(file_menu, IUP_MOUSEPOS, IUP_MOUSEPOS);
}

int main()
{
  Ihandle *cv;

  IupOpen();

  IupSetFunction("exit", (Icallback) exit_cb);
  IupSetFunction("open_menu", (Icallback) open_menu);
  IupSetFunction("open", (Icallback) open_cb);

	cv = IupCanvas("");
	IupSetAttribute(cv, IUP_SIZE, "200x200");
	IupSetAttribute(cv, IUP_BUTTON_CB, "open_menu");

	IupShow(IupDialog(cv));

  IupMainLoop();
  IupClose();
	return 1;
}

