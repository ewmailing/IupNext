
#include <stdio.h>
#include <string.h>

#include <iup.h>

int item_autosave_cb(void)
{
  Ihandle *item_autosave;

  item_autosave = IupGetHandle("item_autosave");
  if(!strcmp(IupGetAttribute(item_autosave, IUP_VALUE),IUP_ON))
  {
    IupMessage("Auto Save","OFF");
    IupSetAttribute(item_autosave, IUP_VALUE, IUP_OFF);
  }
  else
  {
    IupMessage("Auto Save","ON");
    IupSetAttribute(item_autosave, IUP_VALUE, IUP_ON);
  }
  
  return IUP_DEFAULT;
}

int item_exit_cb(void)
{
  return IUP_CLOSE;
}

int main(int argc, char **argv)
{
  Ihandle *dlg;
  Ihandle *text;
  Ihandle *menu, *menu_file;
  Ihandle *submenu_file ;
  Ihandle *item_save, *item_autosave, *item_exit;

  IupOpen(&argc, &argv);
 
  text = IupText(NULL);

  IupSetAttribute(text, IUP_VALUE, "This is an empty text");

  item_save = IupItem("Save\tCtrl+S", NULL);
  item_autosave = IupItem("Auto Save", NULL);
  item_exit = IupItem("Exit", "item_exit_act");

  IupSetAttribute(item_save,     IUP_ACTIVE, "NO");
  IupSetAttribute(item_save,     IUP_KEY, "K_cS");
  IupSetAttribute(item_autosave, IUP_KEY, "K_a");
  IupSetAttribute(item_exit,     IUP_KEY, "K_x");

  IupSetCallback(item_exit, "ACTION", (Icallback) item_exit_cb);
  IupSetCallback(item_save, "ACTION", (Icallback) item_autosave_cb);

  IupSetAttribute(item_autosave, IUP_VALUE, IUP_ON);
  IupSetHandle("item_autosave", item_autosave); /* giving a name to a iup handle */

  menu_file = IupMenu(item_save, item_autosave, item_exit, NULL);
  
  submenu_file = IupSubmenu("File", menu_file);
  
  menu = IupMenu(submenu_file, NULL);
 
  IupSetHandle("menu", menu);
                                
  dlg = IupDialog(text);

  IupSetAttribute(dlg, "TITLE", "IupItem");
  IupSetAttribute(dlg, "MENU", "menu");

  IupShowXY(dlg, IUP_CENTER, IUP_CENTER);
  IupMainLoop();
  IupDestroy(dlg);
  IupClose();

  return 0;
}