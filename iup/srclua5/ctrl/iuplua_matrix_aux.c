/** \file
 * \brief matrix binding for Lua 5.
 *
 * See Copyright Notice in "iup.h"
 */

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

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
#include "iup_assert.h"
#include "iup_predialogs.h"
#include "iup_str.h"



typedef int(*IFnL)(Ihandle*, lua_State *L);


static int math_sum(lua_State *L)
{
  int n = lua_gettop(L);  /* number of arguments */
  lua_Number dsum = luaL_checknumber(L, 1);
  int i;
  for (i = 2; i <= n; i++)
  {
    lua_Number d = luaL_checknumber(L, i);
    dsum += d;
  }
  lua_pushnumber(L, dsum);
  return 1;
}

static int math_average(lua_State *L)
{
  int n = lua_gettop(L);  /* number of arguments */
  lua_Number dsum = luaL_checknumber(L, 1);
  int i;
  for (i = 2; i <= n; i++)
  {
    lua_Number d = luaL_checknumber(L, i);
    dsum += d;
  }
  dsum /= (double)n;
  lua_pushnumber(L, dsum);
  return 1;
}

static int luamatrix_pushvalue(lua_State *L, const char* value, int only_number)
{
  double num;

  if (!value || value[0] == 0)
  {
    if (only_number)
      return 0;

    lua_pushnil(L);
    return 1;
  }

  if (sscanf(value, "%lf", &num) == 1)
    lua_pushnumber(L, num);
  else
  {
    if (only_number)
      return 0;

    lua_pushstring(L, value);
  }

  return 1;
}

static int formula_range(lua_State *L)
{
  Ihandle *ih;
  int lin1 = luaL_checkinteger(L, 1);
  int col1 = luaL_checkinteger(L, 2);
  int lin2 = luaL_checkinteger(L, 3);
  int col2 = luaL_checkinteger(L, 4);
  int only_number = 0;
  int lin, col, count;

  if (lua_isboolean(L, 5))
    only_number = lua_toboolean(L, 5);

  lua_getglobal(L, "matrix");
  ih = (Ihandle*)lua_touserdata(L, -1);

  count = 0;

  for (lin = lin1; lin <= lin2; lin++)
  {
    for (col = col1; col <= col2; col++)
    {
      char* value = IupGetAttributeId2(ih, "", lin, col);

      if (luamatrix_pushvalue(L, value, only_number))
        count++;
    }
  }

  return count;
}

static int formula_cell(lua_State *L)
{
  Ihandle *ih;
  char* value;

  int lin = luaL_checkinteger(L, 1);
  int col = luaL_checkinteger(L, 2);

  lua_getglobal(L, "matrix");
  ih = (Ihandle*)lua_touserdata(L, -1);

  value = IupGetAttributeId2(ih, "", lin, col);
  return luamatrix_pushvalue(L, value, 0);
}

static int formula_ifelse(lua_State *L)
{
  lua_toboolean(L, 1) ? lua_pushvalue(L, 2) : lua_pushvalue(L, 3);
  return 1;
}

static void iMatrixShowFormulaError(Ihandle* ih, lua_State *L)
{
  const char* str_message = IupGetLanguageString("IUP_ERRORINVALIDFORMULA");
  const char* error = lua_tostring(L, -1);
  char msg[1024];
  sprintf(msg, "%s\n  Lua error: %s", str_message, error);
  iupShowError(IupGetDialog(ih), msg);
}

static lua_State* iMatrixInitFormula(Ihandle* ih, const char* init)
{
  lua_State *L;
  IFnL init_cb;

  iupASSERT(iupObjectCheck(ih));
  if (!iupObjectCheck(ih))
    return NULL;

  /* must be an IupMatrix */
  if (ih->iclass->nativetype != IUP_TYPECANVAS ||
      !IupClassMatch(ih, "matrix"))
      return NULL;

  L = (lua_State*)iupAttribGet(ih, "_IUPMATRIXEX_LUASTATE");
  if (L)
    lua_close(L);

  L = luaL_newstate();
  luaL_openlibs(L);

  {
    const char* register_global =
      "function openpackage(ns)\n"
      "  for n, v in pairs(ns) do _G[n] = v end\n"
      "end\n"
      "openpackage(math)\n";
    luaL_dostring(L, register_global);
  }

  lua_register(L, "sum", math_sum);
  lua_register(L, "average", math_average);
  lua_register(L, "range", formula_range);
  lua_register(L, "cell", formula_cell);
  lua_register(L, "ifelse", formula_ifelse);

  if (init)
    luaL_dostring(L, init);

  init_cb = (IFnL)IupGetCallback(ih, "FORMULAINIT_CB");
  if (init_cb)
    init_cb(ih, L);

  lua_pushlightuserdata(L, ih);
  lua_setglobal(L, "matrix");

  return L;
}

static int iMatrixLoadFormula(lua_State *L, const char* formula)
{
  char formula_func[1024];

  sprintf(formula_func, "function matrix_formula(lin, col)\n"
          "  return %s\n"
          "end\n", formula);

  if (luaL_dostring(L, formula_func) != 0)
    return 0;

  return 1;
}

static int iMatrixExecFormula(lua_State *L, int lin, int col)
{
  lua_getglobal(L, "matrix_formula");
  lua_pushinteger(L, lin);
  lua_pushinteger(L, col);

  if (lua_pcall(L, 2, 1, 0) != 0)
    return 0;

  return 1;
}

void IupMatrixSetFormula(Ihandle* ih, int col, const char* formula, const char* init)
{
  int lin, numlin;

  lua_State *L = iMatrixInitFormula(ih, init);
  if (!L)
    return;

  if (!iMatrixLoadFormula(L, formula))
  {
    iMatrixShowFormulaError(ih, L);
    lua_close(L);
    return;
  }

  numlin = IupGetInt(ih, "NUMLIN");

  for (lin = 1; lin <= numlin; lin++)
  {
    if (!iMatrixExecFormula(L, lin, col))
    {
      iMatrixShowFormulaError(ih, L);
      lua_close(L);
      return;
    }

    /* get the result */

    if (lua_isnumber(L, -1))
    {
      double num = lua_tonumber(L, -1);
      IupSetDoubleId2(ih, "", lin, col, num);
    }
    else if (lua_isnil(L, -1))
      IupSetAttributeId2(ih, "", lin, col, NULL);
    else if (lua_isboolean(L, -1))
    {
      int num = lua_toboolean(L, -1);
      IupSetIntId2(ih, "", lin, col, num);
    }
    else
    {
      const char* value = lua_tostring(L, -1);
      IupSetStrAttributeId2(ih, "", lin, col, value);
    }

    lua_pop(L, 1);  /* remove the result from the stack */
  }

  lua_close(L);
}

static char* iMatrixDynamicTranslateValue_CB(Ihandle* ih, int lin, int col, char* value)
{
  if (value && value[0] == '=' && !IupGetInt(ih, "EDITING"))
  {
    lua_State* L = (lua_State*)iupAttribGet(ih, "_IUPMATRIXEX_LUASTATE");

    if (!iMatrixLoadFormula(L, value + 1))
    {
      const char* str_message = IupGetLanguageString("IUP_ERRORINVALIDFORMULA");
      return (char*)str_message;
    }

    if (!iMatrixExecFormula(L, lin, col))
    {
      const char* str_message = IupGetLanguageString("IUP_ERRORINVALIDFORMULA");
      return (char*)str_message;
    }

    /* get the result */

    if (lua_isnumber(L, -1))
    {
      double num = lua_tonumber(L, -1);
      iupAttribSetDouble(ih, "FORMULA_RETURN", num);
    }
    else
    {
      const char* str = lua_tostring(L, -1);
      iupAttribSetStr(ih, "FORMULA_RETURN", str);
    }

    lua_pop(L, 1);  /* remove the result from the stack */
    return iupAttribGet(ih, "FORMULA_RETURN");
  }

  return value;
}

static int iMatrixDynamicLDestroy_CB(Ihandle* ih)
{
  Icallback cb = IupGetCallback(ih, "OLD_LDESTROY_CB");
  lua_State* L = (lua_State*)iupAttribGet(ih, "_IUPMATRIXEX_LUASTATE");
  iupAttribSet(ih, "_IUPMATRIXEX_LUASTATE", NULL);
  IupSetCallback(ih, "TRANSLATEVALUE_CB", NULL);
  lua_close(L);
  if (cb)
    cb(ih);
  return IUP_DEFAULT;
}

void IupMatrixSetDynamic(Ihandle* ih, const char* init)
{
  Icallback cb;

  lua_State *L = iMatrixInitFormula(ih, init);
  if (!L)
    return;

  iupAttribSet(ih, "_IUPMATRIXEX_LUASTATE", (char*)L);

  cb = IupGetCallback(ih, "LDESTROY_CB");
  if (cb)
    IupSetCallback(ih, "OLD_LDESTROY_CB", cb);
  IupSetCallback(ih, "LDESTROY_CB", iMatrixDynamicLDestroy_CB);

  IupSetCallback(ih, "TRANSLATEVALUE_CB", (Icallback)iMatrixDynamicTranslateValue_CB);
}

static int MatrixSetFormula(lua_State *L)
{
  Ihandle *ih = iuplua_checkihandle(L, 1);
  IupMatrixSetFormula(ih, luaL_checkinteger(L, 2), luaL_checkstring(L, 3), luaL_optstring(L, 4, NULL));
  return 0;
}

static int MatrixSetDynamic(lua_State *L)
{
  Ihandle *ih = iuplua_checkihandle(L, 1);
  IupMatrixSetDynamic(ih, luaL_optstring(L, 2, NULL));
  return 0;
}

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
  int lin = luaL_checkinteger(L,3);
  int col = luaL_checkinteger(L,4);
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
  int lin = luaL_checkinteger(L,3);
  int col = luaL_checkinteger(L,4);

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
  int p0 = luaL_checkinteger(L, 2);
  int p1 = luaL_checkinteger(L, 3);
  int p2 = luaL_checkinteger(L, 4);
  int p3 = luaL_checkinteger(L, 5);
  const char* p4 = luaL_checkstring(L, 6);
  int ret = iMatrixOriginalButton_CB(ih, p0, p1, p2, p3, (char*)p4);
  lua_pushinteger(L, ret);
  return 1;
}

static int MatMotionCB(lua_State *L)
{
  Ihandle *ih = iuplua_checkihandle(L, 1);
  int p0 = luaL_checkinteger(L, 2);
  int p1 = luaL_checkinteger(L, 3);
  const char* p2 = luaL_checkstring(L, 4);
  int ret = iMatrixOriginalMotion_CB(ih, p0, p1, (char*)p2);
  lua_pushinteger(L, ret);
  return 1;
}

static int MatKeyPressCB(lua_State *L)
{
  Ihandle *ih = iuplua_checkihandle(L, 1);
  int p0 = luaL_checkinteger(L, 2);
  int p1 = luaL_checkinteger(L, 3);
  int ret = iMatrixOriginalKeyPress_CB(ih, p0, p1);
  lua_pushinteger(L, ret);
  return 1;
}

void iuplua_matrixfuncs_open(lua_State *L)
{
  iuplua_register(L, MatrixSetFormula, "MatrixSetFormula");
  iuplua_register(L, MatrixSetDynamic, "MatrixSetDynamic");

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
