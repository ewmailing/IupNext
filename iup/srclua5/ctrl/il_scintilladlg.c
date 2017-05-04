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


static int scintilladlg_markerchanged_cb(Ihandle *self, int p0, int p1, int p2)
{
  lua_State *L = iuplua_call_start(self, "markerchanged_cb");
  lua_pushinteger(L, p0);
  lua_pushinteger(L, p1);
  lua_pushinteger(L, p2);
  return iuplua_call(L, 3);
}

static int ScintillaDlg(lua_State *L)
{
  Ihandle *ih = IupScintillaDlg();
  iuplua_plugstate(L, ih);
  iuplua_pushihandle_raw(L, ih);
  return 1;
}

int iupscintilladlglua_open(lua_State * L)
{
  iuplua_register(L, ScintillaDlg, "ScintillaDlg");

  iuplua_register_cb(L, "MARKERCHANGED_CB", (lua_CFunction)scintilladlg_markerchanged_cb, NULL);

#ifdef IUPLUA_USELOH
#include "scintilladlg.loh"
#else
#ifdef IUPLUA_USELH
#include "scintilladlg.lh"
#else
  iuplua_dofile(L, "scintilladlg.lua");
#endif
#endif

  return 0;
}

