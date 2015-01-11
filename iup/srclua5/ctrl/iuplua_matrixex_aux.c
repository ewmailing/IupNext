/** \file
 * \brief matrixex binding for Lua 5.
 *
 * See Copyright Notice in "iup.h"
 */

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "iup.h"
#include "iupmatrixex.h"

#include "iuplua.h"
#include "iupluamatrixex.h"
#include "il.h"

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
  int lin1 = luaL_checkint(L, 1);
  int col1 = luaL_checkint(L, 2);
  int lin2 = luaL_checkint(L, 3);
  int col2 = luaL_checkint(L, 4);
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

  int lin = luaL_checkint(L, 1);
  int col = luaL_checkint(L, 2);

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

static void iMatrixExShowFormulaError(Ihandle* ih, lua_State *L)
{
  const char* str_message = IupGetLanguageString("IUP_ERRORINVALIDFORMULA");
  const char* error = lua_tostring(L, -1);
  char msg[1024];
  sprintf(msg, "%s\n  Lua error: %s", str_message, error);
  iupShowError(IupGetDialog(ih), msg);
}

static lua_State* iMatrixExInitFormula(Ihandle* ih, const char* init)
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

static int iMatrixExLoadFormula(lua_State *L, const char* formula)
{
  char formula_func[1024];

  sprintf(formula_func, "function matrix_formula(lin, col)\n"
          "  return %s\n"
          "end\n", formula);

  if (luaL_dostring(L, formula_func) != 0)
    return 0;

  return 1;
}

static int iMatrixExExecFormula(lua_State *L, int lin, int col)
{
  lua_getglobal(L, "matrix_formula");
  lua_pushinteger(L, lin);
  lua_pushinteger(L, col);

  if (lua_pcall(L, 2, 1, 0) != 0)
    return 0;

  return 1;
}

void IupMatrixExSetFormula(Ihandle* ih, int col, const char* formula, const char* init)
{
  int lin, numlin;

  lua_State *L = iMatrixExInitFormula(ih, init);
  if (!L)
    return;

  if (!iMatrixExLoadFormula(L, formula))
  {
    iMatrixExShowFormulaError(ih, L);
    lua_close(L);
    return;
  }

  numlin = IupGetInt(ih, "NUMLIN");

  for (lin = 1; lin <= numlin; lin++)
  {
    if (!iMatrixExExecFormula(L, lin, col))
    {
      iMatrixExShowFormulaError(ih, L);
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

static char* iMatrixExDynamicTranslateValue_CB(Ihandle* ih, int lin, int col, char* value)
{
  if (value && value[0] == '=')
  {
    lua_State* L = (lua_State*)iupAttribGet(ih, "_IUPMATRIXEX_LUASTATE");

    if (!iMatrixExLoadFormula(L, value+1))
    {
      const char* str_message = IupGetLanguageString("IUP_ERRORINVALIDFORMULA");
      return (char*)str_message;
    }

    if (!iMatrixExExecFormula(L, lin, col))
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

static int iMatrixExLDestroy_CB(Ihandle* ih)
{
  Icallback cb = IupGetCallback(ih, "OLD_LDESTROY_CB");
  lua_State* L = (lua_State*)iupAttribGet(ih, "_IUPMATRIXEX_LUASTATE");
  IupSetCallback(ih, "TRANSLATEVALUE_CB", NULL);
  lua_close(L);
  if (cb)
    cb(ih);
  return IUP_DEFAULT;
}

void IupMatrixExSetDynamic(Ihandle* ih, const char* init)
{
  Icallback cb;

  lua_State *L = iMatrixExInitFormula(ih, init);
  if (!L)
    return;

  iupAttribSet(ih, "_IUPMATRIXEX_LUASTATE", (char*)L);

  cb = IupGetCallback(ih, "LDESTROY_CB");
  if (cb)
    IupSetCallback(ih, "OLD_LDESTROY_CB", cb);
  IupSetCallback(ih, "LDESTROY_CB", iMatrixExLDestroy_CB);

  IupSetCallback(ih, "TRANSLATEVALUE_CB", (Icallback)iMatrixExDynamicTranslateValue_CB);
}

static int MatrixExSetFormula(lua_State *L)
{
  Ihandle *ih = iuplua_checkihandle(L,1);
  IupMatrixExSetFormula(ih, luaL_checkint(L, 2), luaL_checkstring(L, 3), luaL_optstring(L, 4, NULL));
  return 0;
}

static int MatrixExSetDynamic(lua_State *L)
{
  Ihandle *ih = iuplua_checkihandle(L, 1);
  IupMatrixExSetDynamic(ih, luaL_optstring(L, 2, NULL));
  return 0;
}

static int MatrixExInit(lua_State *L)
{
  Ihandle *ih = iuplua_checkihandle(L,1);
  IupMatrixExInit(ih);
  return 0;
}

void iuplua_matrixexfuncs_open (lua_State *L)
{
  iuplua_register(L, MatrixExInit, "MatrixExInit");
  iuplua_register(L, MatrixExSetFormula, "MatrixExSetFormula");
  iuplua_register(L, MatrixExSetDynamic, "MatrixExSetDynamic");
}

int iup_matrixexlua_open(lua_State * L);

int iupmatrixexlua_open(lua_State* L)
{
  if (iuplua_opencall_internal(L))
    IupMatrixExOpen();
    
  iuplua_get_env(L);
  iup_matrixexlua_open(L);
  return 0;
}

/* obligatory to use require"iupluamatrixex" */
int luaopen_iupluamatrixex(lua_State* L)
{
  return iupmatrixexlua_open(L);
}

