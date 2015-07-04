/** \file
 * \brief IupConfig bindig to Lua 5.
 *
 * See Copyright Notice in "iup.h"
 */
 
#include <stdlib.h>
#include <string.h>

#include <lua.h>
#include <lauxlib.h>

#include "iup.h"
#include "iup_config.h"

#include "iuplua.h"
#include "il.h"


static int config_recent_cb(Ihandle *self)
{
  lua_State *L = iuplua_call_start(self, "recent_cb");
  return iuplua_call(L, 0);
}

void IupConfigRecentInit(Ihandle* ih, Ihandle* menu, Icallback recent_cb, int max_recent);
int IupConfigLoad(Ihandle* ih);
int IupConfigSave(Ihandle* ih);

static int ConfigRecentUpdate(lua_State *L)
{
  Ihandle* ih = iuplua_checkihandle(L, 1);
  const char *filename = luaL_checkstring(L, 2);
  IupConfigRecentUpdate(ih, filename);
  return 0;
}

static int ConfigDialogShow(lua_State *L)
{
  Ihandle* ih = iuplua_checkihandle(L, 1);
  Ihandle* dialog = iuplua_checkihandle(L, 2);
  const char *name = luaL_checkstring(L, 3);
  IupConfigDialogShow(ih, dialog, name);
  return 0;
}

static int ConfigDialogClosed(lua_State *L)
{
  Ihandle* ih = iuplua_checkihandle(L, 1);
  Ihandle* dialog = iuplua_checkihandle(L, 2);
  const char *name = luaL_checkstring(L, 3);
  IupConfigDialogClosed(ih, dialog, name);
  return 0;
}

void iupconfiglua_open(lua_State * L)
{
  iuplua_register_cb(L, "RECENT_CB", (lua_CFunction)config_recent_cb, NULL);

  iuplua_register(L, ConfigDialogShow, "ConfigDialogShow");
  iuplua_register(L, ConfigDialogClosed, "ConfigDialogClosed");
  iuplua_register(L, ConfigRecentUpdate, "ConfigRecentUpdate");
}
