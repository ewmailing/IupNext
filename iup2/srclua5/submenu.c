/******************************************************************************
 * Automatically generated file (iuplua5). Please don't change anything.                *
 *****************************************************************************/

#include <stdlib.h>

#include <lua.h>
#include <lauxlib.h>

#include "iup.h"
#include "iuplua.h"
#include "il.h"


static int Submenu(lua_State *L)
{
  Ihandle *ih = IupSubmenu((char *) luaL_optstring(L, 1, NULL), iuplua_checkihandle(L, 2));
  iuplua_plugstate(L, ih);
  iuplua_pushihandle_raw(L, ih);
  return 1;
}

int iupsubmenulua_open(lua_State * L)
{
  iuplua_register(L, Submenu, "Submenu");


#ifdef IUPLUA_USELH
#include "submenu.lh"
#else
  iuplua_dofile(L, "submenu.lua");
#endif

  return 0;
}

