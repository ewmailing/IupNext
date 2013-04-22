/******************************************************************************
 * Automatically generated file. Please don't change anything.                *
 *****************************************************************************/

#include <stdlib.h>

#include <lua.h>
#include <lauxlib.h>

#include "iup.h"
#include "iuplua.h"
#include "iup_scintilla.h"
#include "il.h"


static int scintilla_hotspotclick_cb(Ihandle *self, int p0, int p1, int p2, char * p3)
{
  lua_State *L = iuplua_call_start(self, "hotspotclick_cb");
  lua_pushinteger(L, p0);
  lua_pushinteger(L, p1);
  lua_pushinteger(L, p2);
  lua_pushstring(L, p3);
  return iuplua_call(L, 4);
}

static int scintilla_savepoint_cb(Ihandle *self, int p0)
{
  lua_State *L = iuplua_call_start(self, "savepoint_cb");
  lua_pushinteger(L, p0);
  return iuplua_call(L, 1);
}

static int scintilla_action(Ihandle *self, int p0, int p1, int p2, char * p3)
{
  lua_State *L = iuplua_call_start(self, "action");
  lua_pushinteger(L, p0);
  lua_pushinteger(L, p1);
  lua_pushinteger(L, p2);
  lua_pushstring(L, p3);
  return iuplua_call(L, 4);
}

static int scintilla_marginclick_cb(Ihandle *self, int p0, int p1, char * p2)
{
  lua_State *L = iuplua_call_start(self, "marginclick_cb");
  lua_pushinteger(L, p0);
  lua_pushinteger(L, p1);
  lua_pushstring(L, p2);
  return iuplua_call(L, 3);
}

static int Scintilla(lua_State *L)
{
  Ihandle *ih = IupScintilla();
  iuplua_plugstate(L, ih);
  iuplua_pushihandle_raw(L, ih);
  return 1;
}

int iupscintillalua_open(lua_State * L)
{
  iuplua_register(L, Scintilla, "Scintilla");

  iuplua_register_cb(L, "HOTSPOTCLICK_CB", (lua_CFunction)scintilla_hotspotclick_cb, NULL);
  iuplua_register_cb(L, "SAVEPOINT_CB", (lua_CFunction)scintilla_savepoint_cb, NULL);
  iuplua_register_cb(L, "ACTION", (lua_CFunction)scintilla_action, "scintilla");
  iuplua_register_cb(L, "MARGINCLICK_CB", (lua_CFunction)scintilla_marginclick_cb, NULL);

#ifdef IUPLUA_USELOH
#include "scintilla.loh"
#else
#ifdef IUPLUA_USELH
#include "scintilla.lh"
#else
  iuplua_dofile(L, "scintilla.lua");
#endif
#endif

  return 0;
}

