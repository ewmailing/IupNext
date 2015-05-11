#include <stdio.h>
#include <stdlib.h>
#include <iup.h>

int open_cb(void)
{
  Ihandle *filedlg;
  filedlg = IupFileDlg();
  IupSetAttributes(filedlg, "DIALOGTYPE = OPEN, TITLE = \"File Open\"");
  IupSetAttributes(filedlg, "FILTER = \"*.txt\", FILTERINFO = \"Text Files\"");
  if (IupGetInt(filedlg, "STATUS") != -1)
  {
        printf("Open file %s\n", IupGetAttribute(filedlg, "VALUE"));
  }
  else
        printf("CANCEL\n");

  IupDestroy(filedlg);
}

int save_cb(void)
{
  Ihandle *filedlg;
  filedlg = IupFileDlg();

  IupSetAttributes(filedlg, "DIALOGTYPE = SAVE, TITLE = \"File Save\"");
  IupSetAttributes(filedlg, "FILTER = \"*.txt\", FILTERINFO = \"Text Files\"");

  IupPopup(filedlg, IUP_CENTER, IUP_CENTER);

  switch(IupGetInt(filedlg, "STATUS"))
  {
    case 1:
      IupMessage("New file",IupGetAttribute(filedlg, "VALUE"));
      break;

    case 0 :
      IupMessage("File already exists",IupGetAttribute(filedlg, "VALUE"));
      break;

    case -1 :
      IupMessage("IupFileDlg","Operation Canceled");
      return 1;
      break;
  }

  IupDestroy(filedlg);
}

int font_cb(void)
{
        Ihandle* fontdlg = IupFontDlg();
        IupSetAttribute(fontdlg, "COLOR", "128 0 255");
        IupSetAttribute(fontdlg, "VALUE", "Times New Roman, Bold 20");
        IupSetAttribute(fontdlg, "TITLE", "IupFontDlg Test");

        IupPopup(fontdlg, IUP_CURRENT, IUP_CURRENT);

        if (IupGetInt(fontdlg, "STATUS"))
        {
                printf("OK\n");
                printf("  VALUE(%s)\n", IupGetAttribute(fontdlg, "VALUE"));
                printf("  COLOR(%s)\n", IupGetAttribute(fontdlg, "COLOR"));
        }
        else
                printf("CANCEL\n");

        IupDestroy(fontdlg);
}

int exit_cb(void)
{
  return IUP_CLOSE;
}

int main(int argc, char **argv)
{
  Ihandle *dlg, *multitext, *vbox;
  Ihandle *file_menu, *item_exit, *item_open, *item_save;
  Ihandle *edit_menu, *item_edit;
  Ihandle *format_menu, *item_font;
  Ihandle *help_menu, *item_about;

  Ihandle *sub_menu_file, *sub_menu_edit, *sub_menu_format, *sub_menu_help, *menu;

  IupOpen(&argc, &argv);
  multitext = IupText(NULL);
  IupSetAttribute(multitext, "MULTILINE", "YES");
  IupSetAttribute(multitext, "EXPAND", "YES");

  item_open = IupItem ("Open", NULL);
  item_save = IupItem ("Save", NULL);
  item_exit = IupItem ("Exit", NULL);

  item_font = IupItem ("Font", NULL);

  item_about = IupItem ("About", NULL);

  IupSetCallback(item_exit, "ACTION", (Icallback)exit_cb);
  IupSetCallback(item_open, "ACTION", (Icallback)open_cb);
  IupSetCallback(item_save, "ACTION", (Icallback)save_cb);

  IupSetCallback(item_font, "ACTION", (Icallback)font_cb);

  file_menu = IupMenu(item_open,
                      item_save,
                      IupSeparator(),
                      item_exit,
                      NULL);

  edit_menu = IupMenu(item_edit,
                      NULL);

  format_menu = IupMenu(item_font,
                      NULL);

  help_menu = IupMenu(item_about,
                      NULL);

  sub_menu_file = IupSubmenu("File", file_menu);
  sub_menu_edit = IupSubmenu("Edit", edit_menu);
  sub_menu_format = IupSubmenu("Format", format_menu);
  sub_menu_help = IupSubmenu("Help", help_menu);

  menu = IupMenu(sub_menu_file, sub_menu_edit, sub_menu_format, sub_menu_help, NULL);

  vbox = IupVbox(
    multitext,
    NULL
  );

  dlg = IupDialog(vbox);
  IupSetHandle("main_menu", menu);
  IupSetAttribute(dlg, "MENU", "main_menu");
  IupSetAttribute(dlg, "TITLE", "Simple Notepad");
  IupSetAttribute(dlg, "SIZE", "QUARTERxQUARTER");

  IupShowXY(dlg, IUP_CENTER, IUP_CENTER);
  IupMainLoop();

  IupClose();

  return EXIT_SUCCESS;
}
