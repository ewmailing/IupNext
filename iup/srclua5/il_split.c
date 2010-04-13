/******************************************************************************
 * Automatically generated file (iuplua5). Please don't change anything.                *
 *****************************************************************************/

#include <stdlib.h>

#include <lua.h>
#include <lauxlib.h>

#include "iup.h"
#include "iuplua.h"
#include "il.h"


static int Split(lua_State *L)
{
  Ihandle *ih = IupSplit(iuplua_checkihandleornil(L, 1), iuplua_checkihandleornil(L, 2));
  iuplua_plugstate(L, ih);
  iuplua_pushihandle_raw(L, ih);
  return 1;
}

int iupsplitlua_open(lua_State * L)
{
  iuplua_register(L, Split, "Split");


#ifdef IUPLUA_USELOH
#ifdef TEC_BIGENDIAN
#ifdef TEC_64
#include "loh/split_be64.loh"
#else
#include "loh/split_be32.loh"
#endif
#else
#ifdef TEC_64
#ifdef WIN64
#include "loh/split_le64w.loh"
#else
#include "loh/split_le64.loh"
#endif
#else
#include "loh/split.loh"
#endif
#endif
#else
  iuplua_dofile(L, "split.lua");
#endif

  return 0;
}

