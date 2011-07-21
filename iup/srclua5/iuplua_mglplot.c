/** \file
 * \brief mglplot binding for Lua 5.
 *
 * See Copyright Notice in "iup.h"
 */

#include <stdlib.h>

#include <lua.h>
#include <lauxlib.h>

#include "iup.h"
#include "iup_mglplot.h"

#include "iuplua.h"
#include "iuplua_mglplot.h"
#include "il.h"

#if 0
static int mglplot_edit_cb(Ihandle *self, int p0, int p1, float p2, float p3, float *p4, float *p5)
{
  int ret;
  lua_State *L = iuplua_call_start(self, "edit_cb");
  lua_pushinteger(L, p0);
  lua_pushinteger(L, p1);
  lua_pushnumber(L, p2);
  lua_pushnumber(L, p3);
  ret = iuplua_call_raw(L, 4+2, LUA_MULTRET);  /* 4 args + 2 args(errormsg, handle), variable number of returns */
  if (ret || lua_isnil(L, -1))
    return IUP_DEFAULT;
  ret = lua_tointeger(L,-1);

  if (ret == IUP_IGNORE) 
  {
    lua_pop(L, 1);
    return IUP_IGNORE;
  }

  *p4 = (float)lua_tonumber(L, -3); 
  *p5 = (float)lua_tonumber(L, -2); 
  lua_pop(L, 1);
  return IUP_DEFAULT;
}
#endif

static int PlotBegin(lua_State *L)
{
  Ihandle *ih = iuplua_checkihandle(L,1);
  IupMglPlotBegin(ih, luaL_checkint(L,2));
  return 0;
}

static int PlotAdd(lua_State *L)
{
  Ihandle *ih = iuplua_checkihandle(L,1);
  IupMglPlotAdd(ih, (float)luaL_checknumber(L,2), (float)luaL_checknumber(L,3));
  return 0;
}

static int PlotAddStr(lua_State *L)
{
  Ihandle *ih = iuplua_checkihandle(L,1);
  IupMglPlotAddStr(ih, luaL_checkstring(L,2), (float)luaL_checknumber(L,3));
  return 0;
}

static int PlotEnd(lua_State *L)
{
  Ihandle *ih = iuplua_checkihandle(L,1);
  int ret = IupMglPlotEnd(ih);
  lua_pushinteger(L, ret);
  return 1;
}

static int PlotInsertStr(lua_State *L)
{
  Ihandle *ih = iuplua_checkihandle(L,1);
  IupMglPlotInsertStr(ih, luaL_checkint(L,2), luaL_checkint(L,3), luaL_checkstring(L,4), (float)luaL_checknumber(L,5));
  return 0;
}

static int PlotInsert(lua_State *L)
{
  Ihandle *ih = iuplua_checkihandle(L,1);
  IupMglPlotInsert(ih, luaL_checkint(L,2), luaL_checkint(L,3), (float)luaL_checknumber(L,4), (float)luaL_checknumber(L,5));
  return 0;
}

static int PlotInsertPoints(lua_State *L)
{
  float *px, *py;
  int count = luaL_checkinteger(L, 6);
  px = iuplua_checkfloat_array(L, 4, count);
  py = iuplua_checkfloat_array(L, 5, count);
  IupMglPlotInsertPoints(iuplua_checkihandle(L,1), luaL_checkint(L,2), luaL_checkint(L,3), px, py, count);
  free(px);
  free(py);
  return 0;
}

static int PlotInsertStrPoints(lua_State *L)
{
  float *py;
  char* *px;
  int count = luaL_checkinteger(L, 6);
  px = iuplua_checkstring_array(L, 4, count);
  py = iuplua_checkfloat_array(L, 5, count);
  IupMglPlotInsertStrPoints(iuplua_checkihandle(L,1), luaL_checkint(L,2), luaL_checkint(L,3), px, py, count);
  free(px);
  free(py);
  return 0;
}

static int PlotAddPoints(lua_State *L)
{
  float *px, *py;
  int count = luaL_checkinteger(L, 5);
  px = iuplua_checkfloat_array(L, 3, count);
  py = iuplua_checkfloat_array(L, 4, count);
  IupMglPlotAddPoints(iuplua_checkihandle(L,1), luaL_checkint(L,2), px, py, count);
  free(px);
  free(py);
  return 0;
}

static int PlotAddStrPoints(lua_State *L)
{
  float *py;
  char* *px;
  int count = luaL_checkinteger(L, 5);
  px = iuplua_checkstring_array(L, 3, count);
  py = iuplua_checkfloat_array(L, 4, count);
  IupMglPlotAddStrPoints(iuplua_checkihandle(L,1), luaL_checkint(L,2), px, py, count);
  free(px);
  free(py);
  return 0;
}

static int PlotTransform(lua_State *L)
{
  Ihandle *ih = iuplua_checkihandle(L,1);
  int ix, iy;
  IupMglPlotTransform(ih, (float)luaL_checknumber(L,2), (float)luaL_checknumber(L,3), &ix, &iy);
  lua_pushinteger(L, ix);
  lua_pushinteger(L, iy);
  return 2;
}

static int PlotPaintTo(lua_State *L)
{
  Ihandle *ih = iuplua_checkihandle(L,1);
  IupMglPlotPaintTo(ih, cdlua_checkcanvas(L,2));
  return 0;
}

void iuplua_mglplotfuncs_open (lua_State *L)
{
//  iuplua_register_cb(L, "EDIT_CB", (lua_CFunction)mglplot_edit_cb, "mglplot");

  iuplua_register(L, PlotBegin       ,"MglPlotBegin");
  iuplua_register(L, PlotAdd         ,"MglPlotAdd");
  iuplua_register(L, PlotAddStr      ,"MglPlotAddStr");
  iuplua_register(L, PlotEnd         ,"MglPlotEnd");
  iuplua_register(L, PlotInsertStr   ,"MglPlotInsertStr");
  iuplua_register(L, PlotInsert      ,"MglPlotInsert");
  iuplua_register(L, PlotInsertPoints    ,"MglPlotInsertPoints");
  iuplua_register(L, PlotInsertStrPoints ,"MglPlotInsertStrPoints");
  iuplua_register(L, PlotAddPoints       ,"MglPlotAddPoints");
  iuplua_register(L, PlotAddStrPoints    ,"MglPlotAddStrPoints");
  iuplua_register(L, PlotTransform   ,"MglPlotTransform");
  iuplua_register(L, PlotPaintTo     ,"MglPlotPaintTo");
}

int iupmglplotlua_open(lua_State * L);

int iup_mglplotlua_open(lua_State * L)
{
  if (iuplua_opencall_internal(L))
    IupMglPlotOpen();

  iuplua_get_env(L);
  iupmglplotlua_open(L);
  return 0;
}

/* obligatory to use require"iuplua_mglplot" */
int luaopen_iuplua_mglplot(lua_State* L)
{
  return iup_mglplotlua_open(L);
}

