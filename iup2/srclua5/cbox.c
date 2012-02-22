/******************************************************************************
 * Automatically generated file (iuplua5). Please don't change anything.                *
 *****************************************************************************/

#include <stdlib.h>

#include <lua.h>
#include <lauxlib.h>

#include "iup.h"
#include "iuplua.h"
#include "iupcbox.h"
#include "il.h"


static int Cboxv(lua_State *L)
{
  Ihandle **hlist = iuplua_checkihandle_array(L, 1);
  Ihandle *h = IupCboxv(hlist);
  iuplua_plugstate(L, h);
  iuplua_pushihandle_raw(L, h);
  free(hlist);
  return 1;
}
  int iupcboxlua_open(lua_State * L)
{
  iuplua_register(L, Cboxv, "Cboxv");


#ifdef IUPLUA_USELH
#include "cbox.lh"
#else
  iuplua_dofile(L, "cbox.lua");
#endif

  return 0;
}

