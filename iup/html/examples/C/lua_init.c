
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include <iup.h>
#include <iupcontrols.h>

#include <iuplua.h>
#include <iupluacontrols.h>


int main(int argc, char **argv)
{
  lua_State *L;

  IupOpen(&argc, &argv);
  IupControlsOpen();

  /* Lua 5 initialization */
  L = lua_open();   
  luaL_openlibs(L);

  iuplua_open(L);      /* Initialize Binding Lua */
  iupcontrolslua_open(L); /* Inicialize CPI controls binding Lua */

  /* do other things, like running a lua script */
  lua_dofile(L, "myfile.lua");

  IupMainLoop(); /* could be here or inside "myfile.lua" */

  lua_close(L);

  IupControlsClose();
  IupClose();

  return 0;
}
