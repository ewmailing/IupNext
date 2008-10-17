#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

#include "iup.h"
#include "iuplua.h"
#include "iupcontrols.h"
#include "luacontrols.h"


int TryIt();

void main() 
{
  TryIt();
  TryIt();
  TryIt();
  TryIt();
  TryIt();
}


int TryIt() 
{
  lua_State *L;

  IupOpen();
  IupControlsOpen();

  L = lua_open();

  luaopen_base(L);
  luaopen_string(L);
  luaopen_table(L);
  luaopen_io(L);
  luaopen_math(L);
  luaopen_debug(L);

  iuplua_open(L);
  controlslua_open(L);

  lua_dofile( L, "test.lua" );

  IupMainLoop();

  lua_close(L);

  IupControlsClose();
  IupClose();

  return 1;
}
