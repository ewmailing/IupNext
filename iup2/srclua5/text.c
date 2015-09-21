/******************************************************************************
 * Automatically generated file (iuplua5). Please don't change anything.                *
 *****************************************************************************/

#include <stdlib.h>

#include <lua.h>
#include <lauxlib.h>

#include "iup.h"
#include "iuplua.h"
#include "il.h"


static int text_action(Ihandle *self, int p0, char * p1)
{
  lua_State *L = iuplua_call_start(self, "action");
  lua_pushnumber(L, p0);
  lua_pushstring(L, p1);
  return iuplua_call(L, 2);
}

static int text_caret_cb(Ihandle *self, int p0, int p1)
{
  lua_State *L = iuplua_call_start(self, "caret_cb");
  lua_pushnumber(L, p0);
  lua_pushnumber(L, p1);
  return iuplua_call(L, 2);
}

static int Text(lua_State *L)
{
  Ihandle *ih = IupText(NULL);
  iuplua_plugstate(L, ih);
  iuplua_pushihandle_raw(L, ih);
  return 1;
}

int iuptextlua_open(lua_State * L)
{
  iuplua_register(L, Text, "Text");

  iuplua_register_cb(L, "ACTION", (lua_CFunction)text_action, "text");
  iuplua_register_cb(L, "CARET_CB", (lua_CFunction)text_caret_cb, NULL);

#ifdef IUPLUA_USELH
#include "text.lh"
#else
  iuplua_dofile(L, "text.lua");
#endif

  return 0;
}

