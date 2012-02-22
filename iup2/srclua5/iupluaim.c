/** \file
 * \brief IULUA core - Bindig of iup to Lua 5.
 *
 * See Copyright Notice in iup.h
 * $Id: iupluaim.c,v 1.2 2012-02-22 14:55:34 scuri Exp $
 */
 
#include "iup.h"
#include "iupim.h"

#include <lua.h>
#include <lauxlib.h>

#include "iuplua.h"
#include "iupluaim.h"
#include "il.h"

static int SaveImage(lua_State *L)
{
  Ihandle *image = iuplua_checkihandle(L,1);
  const char *filename = luaL_checkstring(L, 2);
  const char *format = luaL_checkstring(L, 3);
  lua_pushnumber(L, IupSaveImage(image, filename, format));
  return 1;
}

static int LoadImage(lua_State *L)
{
  const char *filename = luaL_checkstring(L, 1);
  Ihandle* image = IupLoadImage(filename);
  iuplua_plugstate(L, image);
  iuplua_pushihandle(L, image);
  return 1;
}

int iupimlua_open(lua_State *L)
{
  iuplua_changeEnv(L);
  iuplua_register(L, LoadImage, "LoadImage");
  iuplua_register(L, SaveImage, "SaveImage");
  iuplua_returnEnv(L);
  return 0; /* nothing in stack */
}

/* obligatory to use require"iupluaim" */
int luaopen_iupluaim(lua_State* L)
{
  return iupimlua_open(L);
}

