/******************************************************************************
 * Automatically generated file (iuplua5). Please don't change anything.                *
 *****************************************************************************/

#include <stdlib.h>

#include <lua.h>
#include <lauxlib.h>

#include "iup.h"
#include "iuplua.h"
#include "iupweb.h"
#include "il.h"


static int webbrowser_navigate_cb(Ihandle *self, char * p0, char * p1)
{
  lua_State *L = iuplua_call_start(self, "navigate_cb");
  lua_pushstring(L, p0);
  lua_pushstring(L, p1);
  return iuplua_call(L, 2);
}

static int webbrowser_newwindow_cb(Ihandle *self, char * p0)
{
  lua_State *L = iuplua_call_start(self, "newwindow_cb");
  lua_pushstring(L, p0);
  return iuplua_call(L, 1);
}

static int WebBrowser(lua_State *L)
{
  Ihandle *ih = IupWebBrowser();
  iuplua_plugstate(L, ih);
  iuplua_pushihandle_raw(L, ih);
  return 1;
}

int iupwebbrowserlua_open(lua_State * L)
{
  iuplua_register(L, WebBrowser, "WebBrowser");

  iuplua_register_cb(L, "NAVIGATE_CB", (lua_CFunction)webbrowser_navigate_cb, NULL);
  iuplua_register_cb(L, "NEWWINDOW_CB", (lua_CFunction)webbrowser_newwindow_cb, NULL);

#ifdef IUPLUA_USELOH
#include "webbrowser.loh"
#else
#ifdef IUPLUA_USELZH
#include "webbrowser.lzh"
#else
  iuplua_dofile(L, "webbrowser.lua");
#endif
#endif

  return 0;
}

 
int iupweblua_open(lua_State* L)
{
  if (iuplua_opencall_internal(L))
    IupWebBrowserOpen();
    
  iuplua_get_env(L);
  iupwebcontrollua_open(L);
  return 0;
}

/* obligatory to use require"iupluaweb" */
int luaopen_iupluaweb(lua_State* L)
{
  return iupweblua_open(L);
}

