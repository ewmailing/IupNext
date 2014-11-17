/** \file
 * \brief plot binding for Lua 5.
 *
 * See Copyright Notice in "iup.h"
 */

#include <stdlib.h>

#include <lua.h>
#include <lauxlib.h>

#include "iup.h"
#include "iup_plot.h"

#include <cd.h>
#include <cdlua.h>

#include "iuplua.h"
#include "iuplua_plot.h"
#include "il.h"


static int plot_postdraw_cb(Ihandle *self, cdCanvas* cnv)
{
  lua_State *L = iuplua_call_start(self, "postdraw_cb");
  cdlua_pushcanvas(L, cnv);
  return iuplua_call(L, 1);
}

static int plot_predraw_cb(Ihandle *self, cdCanvas* cnv)
{
  lua_State *L = iuplua_call_start(self, "predraw_cb");
  cdlua_pushcanvas(L, cnv);
  return iuplua_call(L, 1);
}

static int PlotBegin(lua_State *L)
{
  Ihandle *ih = iuplua_checkihandle(L,1);
  IupPlotBegin(ih, luaL_checkint(L,2));
  return 0;
}

static int PlotAdd(lua_State *L)
{
  Ihandle *ih = iuplua_checkihandle(L,1);
  IupPlotAdd(ih, luaL_checknumber(L,2), luaL_checknumber(L,3));
  return 0;
}

static int PlotAddStr(lua_State *L)
{
  Ihandle *ih = iuplua_checkihandle(L,1);
  IupPlotAddStr(ih, luaL_checkstring(L,2), luaL_checknumber(L,3));
  return 0;
}

static int PlotEnd(lua_State *L)
{
  Ihandle *ih = iuplua_checkihandle(L,1);
  int ret = IupPlotEnd(ih);
  lua_pushinteger(L, ret);
  return 1;
}

static int PlotLoadData(lua_State *L)
{
  Ihandle *ih = iuplua_checkihandle(L, 1);
  int ret = IupPlotLoadData(ih, luaL_checkstring(L, 2), luaL_checkint(L, 3));
  lua_pushinteger(L, ret);
  return 1;
}

static int PlotInsertStr(lua_State *L)
{
  Ihandle *ih = iuplua_checkihandle(L,1);
  IupPlotInsertStr(ih, luaL_checkint(L,2), luaL_checkint(L,3), luaL_checkstring(L,4), luaL_checknumber(L,5));
  return 0;
}

static int PlotInsert(lua_State *L)
{
  Ihandle *ih = iuplua_checkihandle(L,1);
  IupPlotInsert(ih, luaL_checkint(L,2), luaL_checkint(L,3), luaL_checknumber(L,4), luaL_checknumber(L,5));
  return 0;
}

static int PlotInsertPoints(lua_State *L)
{
  double *px, *py;
  int count = luaL_checkint(L, 6);
  px = iuplua_checkdouble_array(L, 4, count);
  py = iuplua_checkdouble_array(L, 5, count);
  IupPlotInsertPoints(iuplua_checkihandle(L,1), luaL_checkint(L,2), luaL_checkint(L,3), px, py, count);
  free(px);
  free(py);
  return 0;
}

static int PlotInsertStrPoints(lua_State *L)
{
  double *py;
  char* *px;
  int count = luaL_checkint(L, 6);
  px = iuplua_checkstring_array(L, 4, count);
  py = iuplua_checkdouble_array(L, 5, count);
  IupPlotInsertStrPoints(iuplua_checkihandle(L,1), luaL_checkint(L,2), luaL_checkint(L,3), px, py, count);
  free(px);
  free(py);
  return 0;
}

static int PlotAddPoints(lua_State *L)
{
  double *px, *py;
  int count = luaL_checkint(L, 5);
  px = iuplua_checkdouble_array(L, 3, count);
  py = iuplua_checkdouble_array(L, 4, count);
  IupPlotAddPoints(iuplua_checkihandle(L,1), luaL_checkint(L,2), px, py, count);
  free(px);
  free(py);
  return 0;
}

static int PlotAddStrPoints(lua_State *L)
{
  double *py;
  char* *px;
  int count = luaL_checkint(L, 5);
  px = iuplua_checkstring_array(L, 3, count);
  py = iuplua_checkdouble_array(L, 4, count);
  IupPlotAddStrPoints(iuplua_checkihandle(L,1), luaL_checkint(L,2), px, py, count);
  free(px);
  free(py);
  return 0;
}

static int PlotGetSample(lua_State *L)
{
  Ihandle *ih = iuplua_checkihandle(L, 1);
  double x, y;
  IupPlotGetSample(ih, luaL_checkint(L, 2), luaL_checkint(L, 3), &x, &y);
  lua_pushnumber(L, x);
  lua_pushnumber(L, y);
  return 2;
}

static int PlotGetSampleStr(lua_State *L)
{
  Ihandle *ih = iuplua_checkihandle(L, 1);
  double y;
  const char* x;
  IupPlotGetSampleStr(ih, luaL_checkint(L, 2), luaL_checkint(L, 3), &x, &y);
  lua_pushstring(L, x);
  lua_pushnumber(L, y);
  return 2;
}

static int PlotTransform(lua_State *L)
{
  Ihandle *ih = iuplua_checkihandle(L,1);
  double ix, iy;
  IupPlotTransform(ih, luaL_checknumber(L,2), luaL_checknumber(L,3), &ix, &iy);
  lua_pushnumber(L, ix);
  lua_pushnumber(L, iy);
  return 2;
}

static int PlotTransformTo(lua_State *L)
{
  Ihandle *ih = iuplua_checkihandle(L,1);
  double rx, ry;
  IupPlotTransformTo(ih, (int)luaL_checkinteger(L, 2), (int)luaL_checkinteger(L, 3), &rx, &ry);
  lua_pushnumber(L, rx);
  lua_pushnumber(L, ry);
  return 2;
}

static int PlotPaintTo(lua_State *L)
{
  Ihandle *ih = iuplua_checkihandle(L,1);
  IupPlotPaintTo(ih, cdlua_checkcanvas(L,2));
  return 0;
}

void iuplua_plotfuncs_open (lua_State *L)
{
  iuplua_register_cb(L, "PREDRAW_CB", (lua_CFunction)plot_predraw_cb, NULL);
  iuplua_register_cb(L, "POSTDRAW_CB", (lua_CFunction)plot_postdraw_cb, NULL);

  iuplua_register(L, PlotBegin       ,"PlotBegin");
  iuplua_register(L, PlotAdd         ,"PlotAdd");
  iuplua_register(L, PlotAddStr      ,"PlotAddStr");
  iuplua_register(L, PlotEnd         ,"PlotEnd"); 
  iuplua_register(L, PlotLoadData    ,"PlotLoadData");
  iuplua_register(L, PlotInsertStr   ,"PlotInsertStr");
  iuplua_register(L, PlotInsert      ,"PlotInsert");
  iuplua_register(L, PlotInsertPoints    ,"PlotInsertPoints");
  iuplua_register(L, PlotInsertStrPoints ,"PlotInsertStrPoints");
  iuplua_register(L, PlotAddPoints       ,"PlotAddPoints");
  iuplua_register(L, PlotAddStrPoints    ,"PlotAddStrPoints");
  iuplua_register(L, PlotGetSample, "PlotGetSample");
  iuplua_register(L, PlotGetSampleStr, "PlotGetSampleStr");
  iuplua_register(L, PlotTransform, "PlotTransform");
  iuplua_register(L, PlotTransformTo, "PlotTransformTo");
  iuplua_register(L, PlotPaintTo, "PlotPaintTo");
}

int iupplotlua_open(lua_State * L);

int iup_plotlua_open(lua_State * L)
{
  if (iuplua_opencall_internal(L))
    IupPlotOpen();

  iuplua_get_env(L);
  iupplotlua_open(L);
  return 0;
}

/* obligatory to use require"iuplua_plot" */
int luaopen_iuplua_plot(lua_State* L)
{
  return iup_plotlua_open(L);
}

