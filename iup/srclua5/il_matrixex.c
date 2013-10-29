/******************************************************************************
 * Automatically generated file. Please don't change anything.                *
 *****************************************************************************/

#include <stdlib.h>

#include <lua.h>
#include <lauxlib.h>

#include "iup.h"
#include "iuplua.h"
#include "iupmatrixex.h"
#include "il.h"


static int matrixex_pastesize_cb(Ihandle *self, int p0, int p1)
{
  lua_State *L = iuplua_call_start(self, "pastesize_cb");
  lua_pushinteger(L, p0);
  lua_pushinteger(L, p1);
  return iuplua_call(L, 2);
}

static int matrixex_busy_cb(Ihandle *self, int p0, int p1, char * p2)
{
  lua_State *L = iuplua_call_start(self, "busy_cb");
  lua_pushinteger(L, p0);
  lua_pushinteger(L, p1);
  lua_pushstring(L, p2);
  return iuplua_call(L, 3);
}

static int MatrixEx(lua_State *L)
{
  Ihandle *ih = IupMatrixEx();
  iuplua_plugstate(L, ih);
  iuplua_pushihandle_raw(L, ih);
  return 1;
}

void iuplua_matrixexfuncs_open(lua_State *L);

int iup_matrixexlua_open(lua_State * L)
{
  iuplua_register(L, MatrixEx, "MatrixEx");

  iuplua_register_cb(L, "PASTESIZE_CB", (lua_CFunction)matrixex_pastesize_cb, NULL);
  iuplua_register_cb(L, "BUSY_CB", (lua_CFunction)matrixex_busy_cb, NULL);

  iuplua_matrixexfuncs_open(L);

#ifdef IUPLUA_USELOH
#include "matrixex.loh"
#else
#ifdef IUPLUA_USELH
#include "matrixex.lh"
#else
  iuplua_dofile(L, "matrixex.lua");
#endif
#endif

  return 0;
}

