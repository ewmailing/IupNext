#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include <iup.h>
#include <iup_config.h>

#include <iuplua.h>
#include <iupluascripterdlg.h>


static int item_help_action_cb(void)
{
  IupHelp("http://www.tecgraf.puc-rio.br/iup");
  return IUP_DEFAULT;
}

static int item_about_action_cb(void)
{
  IupMessage("About", "   IupLua Scripter\n\nAutors:\n   Camilo Freire\n   Antonio Scuri");
  return IUP_DEFAULT;
}

static void createargtable(lua_State *L, char **argv, int argc)
{
  int i, narg = argc - 1;  /* number of positive indices (non-zero) */
  lua_createtable(L, narg, 1);
  for (i = 0; i < argc; i++)
  {
    lua_pushstring(L, argv[i]);
    lua_rawseti(L, -2, i);
  }
  lua_setglobal(L, "arg");
}

int main(int argc, char **argv)
{
  Ihandle *config;
  lua_State *L;
  Ihandle *menu;
  Ihandle *main_dialog;

  L = luaL_newstate();
  luaL_openlibs(L);

  createargtable(L, argv, argc);  /* create table 'arg' */

  iuplua_open(L);
  IupLuaScripterDlgOpen();

  IupSetGlobal("GLOBALLAYOUTDLGKEY", "Yes");

  config = IupConfig();
  IupSetAttribute(config, "APP_NAME", "iupluascripter");
  IupConfigLoad(config);

  main_dialog = IupLuaScripterDlg();

  IupSetAttributeHandle(main_dialog, "CONFIG", config);
  IupSetAttribute(main_dialog, "SUBTITLE", "IupLuaScriter");

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

  lua_close(L);
  return EXIT_SUCCESS;
}
