/******************************************************************************
 * Automatically generated file (iuplua5). Please don't change anything.                *
 *****************************************************************************/

#include <stdlib.h>

#include <lua.h>
#include <lauxlib.h>

#include "iup.h"
#include "iuplua.h"
#include "iupole.h"
#include "il.h"


static int OleControl(lua_State *L)
{
  Ihandle *ih = IupOleControl((char *) luaL_checkstring(L, 1));
  iuplua_plugstate(L, ih);
  iuplua_pushihandle_raw(L, ih);
  return 1;
}

int iupolecontrollua_open(lua_State * L)
{
  iuplua_register(L, OleControl, "OleControl");


#ifdef IUPLUA_USELH
#include "olecontrol.lh"
#else
  iuplua_dofile(L, "olecontrol.lua");
#endif

  return 0;
}

 
int iupolelua_open(lua_State* L)
{
  if (iuplua_opencall_internal(L))
    IupOleControlOpen();
    
  iuplua_changeEnv(L);
  iupolecontrollua_open(L);
  iuplua_returnEnv(L);
  return 0;
}

/* obligatory to use require"iupluaole" */
int luaopen_iupluaole(lua_State* L)
{
  return iupolelua_open(L);
}

/* obligatory to use require"iupluaole51" */
int luaopen_iupluaole51(lua_State* L)
{
  return iupolelua_open(L);
}

