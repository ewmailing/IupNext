
#include <lua.h>
#include <lualib.h>

#include <iup.h>
#include <iupcontrols.h>

#include <iuplua.h>
#include <iupluacontrols.h>


int main(int argc, char **argv)
{
  IupOpen(&argc, &argv);
  IupControlsOpen();

  /* Lua 3 initialization */
  lua_open();   
  lua_iolibopen();
  lua_strlibopen();
  lua_mathlibopen();

  iuplua_open();      /* Initialize Binding Lua */
  iupcontrolslua_open(); /* Inicialize CPI controls binding Lua */

/* do other things, like running a lua script */
  lua_dofile("myfile.lua");

  IupMainLoop();

  lua_close();

  IupControlsClose();
  IupClose();
  return 0;
}
