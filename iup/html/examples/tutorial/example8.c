#include <stdio.h>
#include <stdlib.h>
#include <iup.h>

int open_cb(void)
{
  Ihandle *filedlg = IupFileDlg();
  IupSetAttribute(filedlg, "DIALOGTYPE", "OPEN");
  IupSetAttribute(filedlg, "FILTER", "*.txt");
  IupSetAttribute(filedlg, "FILTERINFO", "Text Files");

  IupPopup(filedlg, IUP_CENTER, IUP_CENTER);

  if (IupGetInt(filedlg, "STATUS") != -1)
    IupMessage("Open file", IupGetAttribute(filedlg, "VALUE"));

  IupDestroy(filedlg);
  return IUP_DEFAULT;
}

int save_cb(void)
{
  Ihandle *filedlg = IupFileDlg();
  IupSetAttribute(filedlg, "DIALOGTYPE", "SAVE");
  IupSetAttribute(filedlg, "FILTER", "*.txt");
  IupSetAttribute(filedlg, "FILTERINFO", "Text Files");

  IupPopup(filedlg, IUP_CENTER, IUP_CENTER);

  if (IupGetInt(filedlg, "STATUS") == 1)
    IupMessage("Save file", IupGetAttribute(filedlg, "VALUE"));

  IupDestroy(filedlg);
  return IUP_DEFAULT;
}

int font_cb(void)
{
  Ihandle* fontdlg = IupFontDlg();
  IupSetAttribute(fontdlg, "VALUE", "Times New Roman, Bold 20");
  IupSetAttribute(fontdlg, "TITLE", "IupFontDlg Test");

  IupPopup(fontdlg, IUP_CENTER, IUP_CENTER);

  if (IupGetInt(fontdlg, "STATUS"))
    IupMessage("Selected Font", IupGetAttribute(fontdlg, "VALUE"));

  IupDestroy(fontdlg);
  return IUP_DEFAULT;
}

int exit_cb(void)
{
  return IUP_CLOSE;
}

int main(int argc, char **argv)
{
  Ihandle *dlg, *multitext, *vbox;
  Ihandle *file_menu, *item_exit, *item_open, *item_save;
  Ihandle *format_menu, *item_font;
  Ihandle *help_menu, *item_about;
  Ihandle *sub_menu_file, *sub_menu_format, *sub_menu_help, *menu;

  IupOpen(&argc, &argv);

  multitext = IupText(NULL);
  IupSetAttribute(multitext, "MULTILINE", "YES");
  IupSetAttribute(multitext, "EXPAND", "YES");

  item_open = IupItem("Open...", NULL);
  item_save = IupItem("Save...", NULL);
  item_exit = IupItem("Exit", NULL);
  item_font = IupItem("Font...", NULL);
  item_about = IupItem("About...", NULL);

  IupSetCallback(item_exit, "ACTION", (Icallback)exit_cb);
  IupSetCallback(item_open, "ACTION", (Icallback)open_cb);
  IupSetCallback(item_save, "ACTION", (Icallback)save_cb);
  IupSetCallback(item_font, "ACTION", (Icallback)font_cb);

  file_menu = IupMenu(item_open,
    item_save,
    IupSeparator(),
    item_exit,
    NULL);
  format_menu = IupMenu(item_font,
    NULL);
  help_menu = IupMenu(item_about,
    NULL);

  sub_menu_file = IupSubmenu("File", file_menu);
  sub_menu_format = IupSubmenu("Format", format_menu);
  sub_menu_help = IupSubmenu("Help", help_menu);

  menu = IupMenu(
    sub_menu_file, 
    sub_menu_format, 
    sub_menu_help, 
    NULL);

  vbox = IupVbox(
    multitext,
    NULL);

  dlg = IupDialog(vbox);
  IupSetAttributeHandle(dlg, "MENU", menu);
  IupSetAttribute(dlg, "TITLE", "Simple Notepad");
  IupSetAttribute(dlg, "SIZE", "QUARTERxQUARTER");

  IupShowXY(dlg, IUP_CENTER, IUP_CENTER);
  IupSetAttribute(dlg, "USERSIZE", NULL);

  IupMainLoop();

  IupClose();
  return EXIT_SUCCESS;
}
