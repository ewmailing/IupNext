/******************************************************************************
 * Automatically generated file. Please don't change anything.                *
 *****************************************************************************/

#include <stdlib.h>

#include <lua.h>
#include <lauxlib.h>

#include "iup.h"
#include "iuplua.h"
#include "iup_plot.h"
#include "il.h"


static int plot_edit_cb(Ihandle *self, int p0, int p1, float p2, float p3, float p4, float p5)
{
  lua_State *L = iuplua_call_start(self, "edit_cb");
  lua_pushinteger(L, p0);
  lua_pushinteger(L, p1);
  lua_pushnumber(L, p2);
  lua_pushnumber(L, p3);
  lua_pushnumber(L, p4);
  lua_pushnumber(L, p5);
  return iuplua_call(L, 6);
}

static int plot_deleteend_cb(Ihandle *self)
{
  lua_State *L = iuplua_call_start(self, "deleteend_cb");
  return iuplua_call(L, 0);
}

static int plot_plotmotion_cb(Ihandle *self, float p0, float p1)
{
  lua_State *L = iuplua_call_start(self, "plotmotion_cb");
  lua_pushnumber(L, p0);
  lua_pushnumber(L, p1);
  return iuplua_call(L, 2);
}

static int plot_editbegin_cb(Ihandle *self)
{
  lua_State *L = iuplua_call_start(self, "editbegin_cb");
  return iuplua_call(L, 0);
}

static int plot_select_cb(Ihandle *self, int p0, int p1, float p2, float p3, int p4)
{
  lua_State *L = iuplua_call_start(self, "select_cb");
  lua_pushinteger(L, p0);
  lua_pushinteger(L, p1);
  lua_pushnumber(L, p2);
  lua_pushnumber(L, p3);
  lua_pushinteger(L, p4);
  return iuplua_call(L, 5);
}

static int plot_postdraw_cb(Ihandle *self, int p0)
{
  lua_State *L = iuplua_call_start(self, "postdraw_cb");
  lua_pushinteger(L, p0);
  return iuplua_call(L, 1);
}

static int plot_editend_cb(Ihandle *self)
{
  lua_State *L = iuplua_call_start(self, "editend_cb");
  return iuplua_call(L, 0);
}

static int plot_selectbegin_cb(Ihandle *self)
{
  lua_State *L = iuplua_call_start(self, "selectbegin_cb");
  return iuplua_call(L, 0);
}

static int plot_selectend_cb(Ihandle *self)
{
  lua_State *L = iuplua_call_start(self, "selectend_cb");
  return iuplua_call(L, 0);
}

static int plot_delete_cb(Ihandle *self, int p0, int p1, float p2, float p3)
{
  lua_State *L = iuplua_call_start(self, "delete_cb");
  lua_pushinteger(L, p0);
  lua_pushinteger(L, p1);
  lua_pushnumber(L, p2);
  lua_pushnumber(L, p3);
  return iuplua_call(L, 4);
}

static int plot_deletebegin_cb(Ihandle *self)
{
  lua_State *L = iuplua_call_start(self, "deletebegin_cb");
  return iuplua_call(L, 0);
}

static int plot_predraw_cb(Ihandle *self, int p0)
{
  lua_State *L = iuplua_call_start(self, "predraw_cb");
  lua_pushinteger(L, p0);
  return iuplua_call(L, 1);
}

static int plot_plotbutton_cb(Ihandle *self, int p0, int p1, float p2, float p3, char * p4)
{
  lua_State *L = iuplua_call_start(self, "plotbutton_cb");
  lua_pushinteger(L, p0);
  lua_pushinteger(L, p1);
  lua_pushnumber(L, p2);
  lua_pushnumber(L, p3);
  lua_pushstring(L, p4);
  return iuplua_call(L, 5);
}

static int Plot(lua_State *L)
{
  Ihandle *ih = IupPlot();
  iuplua_plugstate(L, ih);
  iuplua_pushihandle_raw(L, ih);
  return 1;
}

void iuplua_plotfuncs_open(lua_State *L);

int iupplotlua_open(lua_State * L)
{
  iuplua_register(L, Plot, "Plot");

  iuplua_register_cb(L, "EDIT_CB", (lua_CFunction)plot_edit_cb, "plot");
  iuplua_register_cb(L, "DELETEEND_CB", (lua_CFunction)plot_deleteend_cb, NULL);
  iuplua_register_cb(L, "PLOTMOTION_CB", (lua_CFunction)plot_plotmotion_cb, NULL);
  iuplua_register_cb(L, "EDITBEGIN_CB", (lua_CFunction)plot_editbegin_cb, NULL);
  iuplua_register_cb(L, "SELECT_CB", (lua_CFunction)plot_select_cb, NULL);
  iuplua_register_cb(L, "POSTDRAW_CB", (lua_CFunction)plot_postdraw_cb, NULL);
  iuplua_register_cb(L, "EDITEND_CB", (lua_CFunction)plot_editend_cb, NULL);
  iuplua_register_cb(L, "SELECTBEGIN_CB", (lua_CFunction)plot_selectbegin_cb, NULL);
  iuplua_register_cb(L, "SELECTEND_CB", (lua_CFunction)plot_selectend_cb, NULL);
  iuplua_register_cb(L, "DELETE_CB", (lua_CFunction)plot_delete_cb, NULL);
  iuplua_register_cb(L, "DELETEBEGIN_CB", (lua_CFunction)plot_deletebegin_cb, NULL);
  iuplua_register_cb(L, "PREDRAW_CB", (lua_CFunction)plot_predraw_cb, NULL);
  iuplua_register_cb(L, "PLOTBUTTON_CB", (lua_CFunction)plot_plotbutton_cb, NULL);

  iuplua_plotfuncs_open(L);

#ifdef IUPLUA_USELOH
#include "plot.loh"
#else
#ifdef IUPLUA_USELH
#include "plot.lh"
#else
  iuplua_dofile(L, "plot.lua");
#endif
#endif

  return 0;
}

