#include <stdio.h>
#include <stdlib.h>

#include <iup.h>
#include <iup_scintilla.h>
#include <iup_config.h>


static int item_help_action_cb(void)
{
  IupHelp("http://www.tecgraf.puc-rio.br/iup");
  return IUP_DEFAULT;
}

static int item_about_action_cb(void)
{
  IupMessage("About", "   Scintilla Notepad\n\nAutors:\n   Camilo Freire\n   Antonio Scuri");
  return IUP_DEFAULT;
}

int main(int argc, char **argv)
{
  Ihandle *main_dialog;
  Ihandle *config;
  Ihandle *menu;

  IupOpen(&argc, &argv);
  IupImageLibOpen();

  IupScintillaOpen();

  config = IupConfig();
  IupSetAttribute(config, "APP_NAME", "scintilla_notepad");
  IupConfigLoad(config);

  main_dialog = IupScintillaDlg();

  IupSetAttributeHandle(main_dialog, "CONFIG", config);
  IupSetAttribute(main_dialog, "SUBTITLE", "Scintilla Notepad");

  menu = IupGetAttributeHandle(main_dialog, "MENU");
  IupAppend(menu, IupSubmenu("&Help", IupMenu(
    IupSetCallbacks(IupItem("&Help...", NULL), "ACTION", (Icallback)item_help_action_cb, NULL),
    IupSetCallbacks(IupItem("&About...", NULL), "ACTION", (Icallback)item_about_action_cb, NULL),
    NULL)));

  /* show the dialog at the last position, with the last size */
  IupConfigDialogShow(config, main_dialog, "MainWindow");

  /* initialize the current file */
  IupSetAttribute(main_dialog, "NEWFILE", NULL);

  /* open a file from the command line (allow file association in Windows) */
  if (argc > 1 && argv[1])
  {
    const char* filename = argv[1];
    IupSetStrAttribute(main_dialog, "OPENFILE", filename);
  }

  IupMainLoop();

  IupDestroy(config);
  IupDestroy(main_dialog);

  IupClose();
  return EXIT_SUCCESS;
}

