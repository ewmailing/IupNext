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

