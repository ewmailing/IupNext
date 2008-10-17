#include <lua.h>
#include <iup.h>
#include <iupcb.h>
#include <iuplua.h>
#include <luacb.h>

int main(void) {
  lua_open();
  IupOpen();
  IupColorBrowserOpen();
  iuplua_open();
	cblua_open();
  lua_dofile("a.lua");
  IupMainLoop();
  return 0;
}
