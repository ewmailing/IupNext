/******************************************************************************
 * Automatically generated file. Please don't change anything.                *
 *****************************************************************************/

#include <stdlib.h>

#include <lua.h>
#include <lauxlib.h>

#include "iup.h"
#include "iuplua.h"
#include "il.h"


static int flattabs_tabextra_cb(Ihandle *self, int p0)
{
  lua_State *L = iuplua_call_start(self, "tabextra_cb");
  lua_pushinteger(L, p0);
  return iuplua_call(L, 1);
}

static int FlatTabs(lua_State *L)
{
  Ihandle *ih = IupFlatTabs(NULL);
  iuplua_plugstate(L, ih);
  iuplua_pushihandle_raw(L, ih);
  return 1;
}

int iupflattabslua_open(lua_State * L)
{
  iuplua_register(L, FlatTabs, "FlatTabs");

  iuplua_register_cb(L, "TABEXTRA_CB", (lua_CFunction)flattabs_tabextra_cb, NULL);

#ifdef IUPLUA_USELOH
#include "flattabs.loh"
#else
#ifdef IUPLUA_USELH
#include "flattabs.lh"
#else
  iuplua_dofile(L, "flattabs.lua");
#endif
#endif

  return 0;
}

