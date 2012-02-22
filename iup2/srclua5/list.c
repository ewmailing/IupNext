/******************************************************************************
 * Automatically generated file (iuplua5). Please don't change anything.                *
 *****************************************************************************/

#include <stdlib.h>

#include <lua.h>
#include <lauxlib.h>

#include "iup.h"
#include "iuplua.h"
#include "il.h"


static int list_multiselect_cb(Ihandle *self, char * p0)
{
  lua_State *L = iuplua_call_start(self, "multiselect_cb");
  lua_pushstring(L, p0);
  return iuplua_call(L, 1);
}

static int list_action(Ihandle *self, char * p0, int p1, int p2)
{
  lua_State *L = iuplua_call_start(self, "action");
  lua_pushstring(L, p0);
  lua_pushnumber(L, p1);
  lua_pushnumber(L, p2);
  return iuplua_call(L, 3);
}

static int list_edit_cb(Ihandle *self, int p0, char * p1)
{
  lua_State *L = iuplua_call_start(self, "edit_cb");
  lua_pushnumber(L, p0);
  lua_pushstring(L, p1);
  return iuplua_call(L, 2);
}

static int List(lua_State *L)
{
  Ihandle *ih = IupList(NULL);
  iuplua_plugstate(L, ih);
  iuplua_pushihandle_raw(L, ih);
  return 1;
}

int iuplistlua_open(lua_State * L)
{
  iuplua_register(L, List, "List");

  iuplua_register_cb(L, "MULTISELECT_CB", (lua_CFunction)list_multiselect_cb, NULL);
  iuplua_register_cb(L, "ACTION", (lua_CFunction)list_action, "list");
  iuplua_register_cb(L, "EDIT_CB", (lua_CFunction)list_edit_cb, "list");

#ifdef IUPLUA_USELH
#include "list.lh"
#else
  iuplua_dofile(L, "list.lua");
#endif

  return 0;
}

