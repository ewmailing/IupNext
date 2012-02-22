/******************************************************************************
 * Automatically generated file (iuplua5). Please don't change anything.                *
 *****************************************************************************/

#include <stdlib.h>

#include <lua.h>
#include <lauxlib.h>

#include "iup.h"
#include "iuplua.h"
#include "iupsbox.h"
#include "il.h"


static int Sbox(lua_State *L)
{
  Ihandle *ih = IupSbox(iuplua_checkihandle(L, 1));
  iuplua_plugstate(L, ih);
  iuplua_pushihandle_raw(L, ih);
  return 1;
}

int iupsboxlua_open(lua_State * L)
{
  iuplua_register(L, Sbox, "Sbox");


#ifdef IUPLUA_USELH
#include "sbox.lh"
#else
  iuplua_dofile(L, "sbox.lua");
#endif

  return 0;
}

