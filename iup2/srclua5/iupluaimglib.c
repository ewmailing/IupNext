/** \file
 * \brief Iup Image Library Lua 5 Binding
 *
 * See Copyright Notice in cd.h
 */

#include <stdlib.h>
#include <stdio.h>

#include <lua.h>
#include <lauxlib.h>

#include "iup.h"
#include "iuplua.h"
#include "iupcontrols.h"
#include "il.h"


static int imagelibopen(lua_State *L)
{
  (void)L;
  IupImageLibOpen();
  return 0;
}

#if (IUP_VERSION_NUMBER < 300000)
static int imagelibclose(lua_State *L)
{
  (void)L;
  IupImageLibClose();
  return 0;
}
#endif

static int iupluaimglib_open (lua_State *L)
{
  IupImageLibOpen();
  
  iuplua_changeEnv(L);
  iuplua_register(L, imagelibopen, "ImageLibOpen");
#if (IUP_VERSION_NUMBER < 300000)
  iuplua_register(L, imagelibclose, "ImageLibClose");
#endif
  iuplua_returnEnv(L);
  return 0; /* nothing in stack */
}

int luaopen_iupluaimglib(lua_State* L)
{
  return iupluaimglib_open(L);
}

int luaopen_iupluaimglib51(lua_State* L)
{
  return iupluaimglib_open(L);
}
