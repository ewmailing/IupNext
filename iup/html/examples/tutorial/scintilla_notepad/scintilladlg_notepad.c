#include <stdio.h>
#include <stdlib.h>

#include <iup.h>
#include <iup_scintilla.h>
#include <iup_config.h>


int main(int argc, char **argv)
{
  Ihandle *main_dialog;
  Ihandle *config;

  IupOpen(&argc, &argv);
  IupImageLibOpen();

  IupScintillaOpen();

  config = IupConfig();
  IupSetAttribute(config, "APP_NAME", "scintilla_notepad");
  IupConfigLoad(config);

  main_dialog = IupScintillaDlg();

  IupSetAttributeHandle(main_dialog, "CONFIG", config);
  IupSetAttribute(main_dialog, "SUBTITLE", "Scintilla Notepad");

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

  IupClose();
  return EXIT_SUCCESS;
}

