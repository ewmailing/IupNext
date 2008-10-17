/** \file
 * \brief Bindig of iupole to Lua 3.
 *
 * See Copyright Notice in iup.h
 * $Id: luaole.c,v 1.1 2008-10-17 06:21:03 scuri Exp $
 */
 
#include <stdlib.h>

#include <lua.h>
#include <lauxlib.h>

#include "iup.h"
#include "iupole.h"
#include "iupluaole.h"

static void createOleControl (void)
{
  int tag = (int)lua_getnumber(lua_getglobal("iuplua_tag"));
  lua_pushusertag(IupOleControl(luaL_check_string(1)),tag);
}

int iupolelua_open (void)
{
  lua_register("iupCreateOleControl",createOleControl);

  return lua_dostring("IUPOLECONTROL = {parent = WIDGET}\n"
                      "function IUPOLECONTROL:CreateIUPelement (obj)\n"
                      "  return iupCreateOleControl (obj[1])\n"
                      "end\n"
                      "function iupolecontrol (o)\n"
                      "  return IUPOLECONTROL:Constructor (o)\n"
                      "end\n"
                      "iup.olecontrol = iupolecontrol\n");
}
