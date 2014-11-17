/** \file
 * \brief matrix binding for Lua 5.
 *
 * See Copyright Notice in "iup.h"
 */

#include <lua.h>
#include <lauxlib.h>

#include "iup.h"
#include "iupcontrols.h"
#include "iupcbs.h"
#include <cd.h>
#include <cdlua.h>

#include "iuplua.h"
#include "il.h"
#include "il_controls.h"

#include "iup_attrib.h"
#include "iup_object.h"


static int matrix_draw_cb(Ihandle *self, int p0, int p1, int p2, int p3, int p4, int p5, cdCanvas* cnv)
{
  lua_State *L = iuplua_call_start(self, "draw_cb");
  lua_pushinteger(L, p0);
  lua_pushinteger(L, p1);
  lua_pushinteger(L, p2);
  lua_pushinteger(L, p3);
  lua_pushinteger(L, p4);
  lua_pushinteger(L, p5);
  cdlua_pushcanvas(L, cnv);
  return iuplua_call(L, 7);
}

static int matrix_color_cb(Ihandle *self, int p0, int p1, unsigned int *p2, unsigned int *p3, unsigned int *p4, const char* name)
{
  int status;
  lua_State *L = iuplua_call_start(self, name);
  int top = lua_gettop(L) - 3; /* 3 is the number of pushed values in iuplua_call_start */
  /* don't have control over the number of returned values because of LUA_MULTRET,
     so must restore stack manually */

  lua_pushinteger(L, p0);
  lua_pushinteger(L, p1);

  /* similar to iuplua_call */
  status = iuplua_call_raw(L, 2 + 2, LUA_MULTRET);   /* 2 args + 2 args(errormsg, handle), variable number of returns */

  if (status != LUA_OK)
    return IUP_DEFAULT;
  else
  {
    /* can do:
       return r, g, b, iup.DEFAULT
       return r, g, b, iup.IGNORE
       return iup.IGNORE
       return                       -- same as iup.IGNORE
    */
    int tmp = (int)lua_isnil(L, -1)? IUP_IGNORE: (int)lua_tointeger(L, -1);
    if (tmp == IUP_IGNORE)
    {
      lua_settop(L, top);  /* remove the results */
      return IUP_IGNORE;
    }

    *p2 = (unsigned int)lua_tointeger(L, -4);  /* R */
    *p3 = (unsigned int)lua_tointeger(L, -3);  /* G */
    *p4 = (unsigned int)lua_tointeger(L, -2);  /* B */
    lua_settop(L, top);  /* remove the results */
    return IUP_DEFAULT;  
  }
}

static int matrix_bgcolor_cb(Ihandle *self, int p0, int p1, unsigned int *p2, unsigned int *p3, unsigned int *p4)
{
  return matrix_color_cb(self, p0, p1, p2, p3, p4, "bgcolor_cb");
}

static int matrix_fgcolor_cb(Ihandle *self, int p0, int p1, unsigned int *p2, unsigned int *p3, unsigned int *p4)
{
  return matrix_color_cb(self, p0, p1, p2, p3, p4, "fgcolor_cb");
}

static int MatGetAttribute(lua_State *L)
{
  Ihandle *ih = iuplua_checkihandle(L,1);
  const char *name = luaL_checkstring(L,2);
  int lin = luaL_checkint(L,3);
  int col = luaL_checkint(L,4);
  const char *value = IupGetAttributeId2(ih, name, lin, col);
  if (!value || iupATTRIB_ISINTERNAL(name))
    lua_pushnil(L);
  else
  {
    if (iupAttribIsNotString(ih, name))
    {
      if (iupAttribIsIhandle(ih, name))
        iuplua_pushihandle(L, (Ihandle*)value);
      else
        lua_pushlightuserdata(L, (void*)value);
    }
    else
      lua_pushstring(L,value);
  }
  return 1;
}

static int MatStoreAttribute(lua_State *L)
{
  Ihandle *ih = iuplua_checkihandle(L,1);
  const char *a = luaL_checkstring(L,2);
  int lin = luaL_checkint(L,3);
  int col = luaL_checkint(L,4);

  if (lua_isnil(L,5)) 
    IupSetAttributeId2(ih,a,lin, col,NULL);
  else 
  {
    const char *v;
    if(lua_isuserdata(L,5)) 
    {
      v = lua_touserdata(L,5);
      IupSetAttributeId2(ih,a,lin, col,v);
    }
    else 
    {
      v = luaL_checkstring(L,5);
      IupStoreAttributeId2(ih,a,lin, col,v);
    }
  }
  return 0;
}

static IFnii iMatrixOriginalKeyPress_CB = NULL;
static IFniiiis iMatrixOriginalButton_CB = NULL;
static IFniis iMatrixOriginalMotion_CB = NULL;

static int MatButtonCB(lua_State *L)
{
  Ihandle *ih = iuplua_checkihandle(L, 1);
  int p0 = luaL_checkint(L, 2);
  int p1 = luaL_checkint(L, 3);
  int p2 = luaL_checkint(L, 4);
  int p3 = luaL_checkint(L, 5);
  const char* p4 = luaL_checkstring(L, 6);
  int ret = iMatrixOriginalButton_CB(ih, p0, p1, p2, p3, (char*)p4);
  lua_pushinteger(L, ret);
  return 1;
}

static int MatMotionCB(lua_State *L)
{
  Ihandle *ih = iuplua_checkihandle(L, 1);
  int p0 = luaL_checkint(L, 2);
  int p1 = luaL_checkint(L, 3);
  const char* p2 = luaL_checkstring(L, 4);
  int ret = iMatrixOriginalMotion_CB(ih, p0, p1, (char*)p2);
  lua_pushinteger(L, ret);
  return 1;
}

static int MatKeyPressCB(lua_State *L)
{
  Ihandle *ih = iuplua_checkihandle(L, 1);
  int p0 = luaL_checkint(L, 2);
  int p1 = luaL_checkint(L, 3);
  int ret = iMatrixOriginalKeyPress_CB(ih, p0, p1);
  lua_pushinteger(L, ret);
  return 1;
}

void iuplua_matrixfuncs_open(lua_State *L)
{
  /* DEPRECATED backward compatibility */
  iuplua_register(L, MatGetAttribute, "MatGetAttribute");
  iuplua_register(L, MatStoreAttribute, "MatStoreAttribute");
  iuplua_register(L, MatStoreAttribute, "MatSetAttribute");

  /* Original Callback Export */
  iuplua_register(L, MatButtonCB, "MatButtonCb");
  iuplua_register(L, MatMotionCB, "MatMotionCb");
  iuplua_register(L, MatKeyPressCB, "MatKeyPressCb");

  {
    Ihandle* mat = IupMatrix(NULL);
    iMatrixOriginalKeyPress_CB = (IFnii)IupGetCallback(mat, "KEYPRESS_CB");
    iMatrixOriginalButton_CB = (IFniiiis)IupGetCallback(mat, "BUTTON_CB");
    iMatrixOriginalMotion_CB = (IFniis)IupGetCallback(mat, "MOTION_CB");
    IupDestroy(mat);
  }

  iuplua_register_cb(L, "BGCOLOR_CB", (lua_CFunction)matrix_bgcolor_cb, NULL);
  iuplua_register_cb(L, "FGCOLOR_CB", (lua_CFunction)matrix_fgcolor_cb, NULL);
  iuplua_register_cb(L, "DRAW_CB", (lua_CFunction)matrix_draw_cb, NULL);
  iuplua_register_cb(L, "LISTDRAW_CB", (lua_CFunction)matrix_draw_cb, NULL);
}
