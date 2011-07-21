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

static int PlotAdd1D(lua_State *L)
{
  Ihandle *ih = iuplua_checkihandle(L,1);
  IupMglPlotAdd1D(ih, luaL_checkstring(L,2), (float)luaL_checknumber(L,3));
  return 0;
}

static int PlotAdd2D(lua_State *L)
{
  Ihandle *ih = iuplua_checkihandle(L,1);
  IupMglPlotAdd2D(ih, (float)luaL_checknumber(L,2), (float)luaL_checknumber(L,3));
  return 0;
}

static int PlotAdd3D(lua_State *L)
{
  Ihandle *ih = iuplua_checkihandle(L,1);
  IupMglPlotAdd3D(ih, (float)luaL_checknumber(L,2), (float)luaL_checknumber(L,3), (float)luaL_checknumber(L,4));
  return 0;
}

static int PlotEnd(lua_State *L)
{
  Ihandle *ih = iuplua_checkihandle(L,1);
  int ret = IupMglPlotEnd(ih);
  lua_pushinteger(L, ret);
  return 1;
}

static int PlotInsert1D(lua_State *L)
{
  float *py;
  char* *px;
  int count = luaL_checkinteger(L, 6);
  px = iuplua_checkstring_array(L, 4, count);
  py = iuplua_checkfloat_array(L, 5, count);
  IupMglPlotInsert1D(iuplua_checkihandle(L,1), luaL_checkint(L,2), luaL_checkint(L,3), px, py, count);
  free(px);
  free(py);
  return 0;
}

static int PlotInsert2D(lua_State *L)
{
  float *px, *py;
  int count = luaL_checkinteger(L, 6);
  px = iuplua_checkfloat_array(L, 4, count);
  py = iuplua_checkfloat_array(L, 5, count);
  IupMglPlotInsert2D(iuplua_checkihandle(L,1), luaL_checkint(L,2), luaL_checkint(L,3), px, py, count);
  free(px);
  free(py);
  return 0;
}

static int PlotInsert3D(lua_State *L)
{
  float *px, *py, *pz;
  int count = luaL_checkinteger(L, 6);
  px = iuplua_checkfloat_array(L, 4, count);
  py = iuplua_checkfloat_array(L, 5, count);
  pz = iuplua_checkfloat_array(L, 6, count);
  IupMglPlotInsert3D(iuplua_checkihandle(L,1), luaL_checkint(L,2), luaL_checkint(L,3), px, py, pz, count);
  free(px);
  free(py);
  return 0;
}

static int PlotTransform(lua_State *L)
{
  Ihandle *ih = iuplua_checkihandle(L,1);
  int ix, iy;
  IupMglPlotTransform(ih, (float)luaL_checknumber(L,2), (float)luaL_checknumber(L,3), (float)luaL_checknumber(L,4), &ix, &iy);
  lua_pushinteger(L, ix);
  lua_pushinteger(L, iy);
  return 2;
}

static int PlotTransformXYZ(lua_State *L)
{
  Ihandle *ih = iuplua_checkihandle(L,1);
  float x, y, z;
  IupMglPlotTransformXYZ(ih, luaL_checkint(L,2), luaL_checkint(L,3), &x, &y, &z);
  lua_pushnumber(L, x);
  lua_pushnumber(L, y);
  lua_pushnumber(L, z);
  return 3;
}

static int PlotPaintTo(lua_State *L)
{
  Ihandle *ih = iuplua_checkihandle(L,1);
  const char* format = luaL_checkstring(L,2);
  void* data = NULL;

  if (*format == 'E' || *format == 'S') //EPS or SVG
    data = (void*)luaL_checkstring(L, 6);
  else if (*format == 'R') //RGB
  {
    if (!lua_islightuserdata(L, 6))
      luaL_argerror(L, 6, "data must be a light user data");
    data = lua_touserdata(L, 6);
  }
  else
    luaL_argerror(L, 2, "invalid format");

  IupMglPlotPaintTo(ih, format, luaL_checkint(L,3), luaL_checkint(L,4), (float)luaL_checknumber(L,5), data);
  return 0;
}

#if 0
int IupMglPlotNewDataSet(Ihandle *ih, int dim);

void IupMglPlotSet1D(Ihandle* ih, int ds_index, const char** names, const float* y, int count);
void IupMglPlotSet2D(Ihandle* ih, int ds_index, const float* x, const float* y, int count);
void IupMglPlotSet3D(Ihandle* ih, int ds_index, const float* x, const float* y, const float* z, int count);
void IupMglPlotSetFormula(Ihandle* ih, int ds_index, const char* formulaX, const char* formulaY, const char* formulaZ, int count);

void IupMglPlotSetData(Ihandle* ih, int ds_index, const float* data, int count_x, int count_y, int count_z);
void IupMglPlotLoadData(Ihandle* ih, int ds_index, const char* filename, int count_x, int count_y, int count_z);
void IupMglPlotSetFromFormula(Ihandle* ih, int ds_index, const char* formula, int count_x, int count_y, int count_z);

void IupMglPlotDrawMark(Ihandle* ih, float x, float y, float z);
void IupMglPlotDrawLine(Ihandle* ih, float x1, float y1, float z1, float x2, float y2, float z2);
void IupMglPlotDrawText(Ihandle* ih, const char* text, float x, float y, float z);
#endif

void iuplua_mglplotfuncs_open (lua_State *L)
{
//  iuplua_register_cb(L, "EDIT_CB", (lua_CFunction)mglplot_edit_cb, "mglplot");

  iuplua_register(L, PlotBegin       ,"MglPlotBegin");
  iuplua_register(L, PlotAdd1D       ,"MglPlotAdd1D");
  iuplua_register(L, PlotAdd2D      ,"MglPlotAdd2D");
  iuplua_register(L, PlotAdd3D      ,"MglPlotAdd3D");
  iuplua_register(L, PlotEnd         ,"MglPlotEnd");
  iuplua_register(L, PlotInsert1D   ,"MglPlotInsert1D");
  iuplua_register(L, PlotInsert2D   ,"MglPlotInsert2D");
  iuplua_register(L, PlotInsert3D   ,"MglPlotInsert3D");
  iuplua_register(L, PlotTransform    ,"MglPlotTransform");
  iuplua_register(L, PlotTransformXYZ ,"MglPlotTransformXYZ");
  iuplua_register(L, PlotPaintTo      ,"MglPlotPaintTo");
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

