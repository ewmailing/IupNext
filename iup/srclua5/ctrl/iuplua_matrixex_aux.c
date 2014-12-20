/** \file
 * \brief matrixex binding for Lua 5.
 *
 * See Copyright Notice in "iup.h"
 */

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "iup.h"
#include "iupcbs.h"
#include "iupmatrixex.h"

#include "iuplua.h"
#include "il.h"

#include "iup_attrib.h"
#include "iup_object.h"
#include "iup_assert.h"
#include "iup_predialogs.h"


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

  int lin = luaL_checkint(L, 1);
  int col = luaL_checkint(L, 2);

  lua_getglobal(L, "matrix");
  ih = (Ihandle*)lua_touserdata(L, -1);

  char* value = IupGetAttributeId2(ih, "", lin, col);
  return luamatrix_pushvalue(L, value, 0);
}

static int formula_ifelse(lua_State *L)
{
  lua_toboolean(L, 1) ? lua_pushvalue(L, 2) : lua_pushvalue(L, 3);
  return 1;
}

static void ShowFormulaError(Ihandle* ih, lua_State *L)
{
  const char* str_message = IupGetLanguageString("IUP_ERRORINVALIDFORMULA");
  const char* error = lua_tostring(L, -1);
  char msg[1024];
  sprintf(msg, "%s\n  Lua error: %s", str_message, error);
  iupShowError(IupGetDialog(ih), msg);
}

void IupMatrixExSetFormula(Ihandle* ih, int col, const char* formula, const char* init)
{
  lua_State *L;
  int lin, numlin;
  char formula_func[1024];
  IFnL init_cb;

  iupASSERT(iupObjectCheck(ih));
  if (!iupObjectCheck(ih))
    return;

  /* must be an IupMatrix */
  if (ih->iclass->nativetype != IUP_TYPECANVAS ||
      !IupClassMatch(ih, "matrix"))
      return;

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

  init_cb = (IFnL)IupGetCallback(ih, "INIT_CB");
  if (init_cb)
    init_cb(ih, L);

  numlin = IupGetInt(ih, "NUMLIN");

  lua_pushlightuserdata(L, ih);
  lua_setglobal(L, "matrix");

  sprintf(formula_func, "function matrix_formula(lin, col)\n"
                        "  return %s\n"
                        "end\n", formula);

  if (luaL_dostring(L, formula_func) != 0)
  {
    ShowFormulaError(ih, L);
    lua_close(L);
    return;
  }

  for (lin = 1; lin <= numlin; lin++)
  {
    lua_getglobal(L, "matrix_formula");
    lua_pushinteger(L, lin);
    lua_pushinteger(L, col);

    if (lua_pcall(L, 2, 1, 0) != 0)
    {
      ShowFormulaError(ih, L);
      lua_close(L);
      return;
    }

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

    lua_pop(L, 1);  /* removed the result from the stack */
  }

  lua_close(L);
}

#if 0

  double DynamicColumn::GetValue(lua_State *L) const
  {
    int top1 = lua_gettop(L);
    luaL_dostring(L, macro.c_str());
    int top2 = lua_gettop(L);
    if (top2 - top1 == 1 && lua_isnumber(L, -1))
      return lua_tonumber(L, -1);
    return 0.0;
  }
}

int Dmatrix::Lua_callback_cell(lua_State *L)
{
  Dmatrix *D = current_dmatrix;
  if (lua_gettop(L) < 1)
  {
    lua_pushstring(L, "Error in script! Function cell() expected one or two numbers as parameters.");
    lua_error(L);
    return 0;
  }
  int lin = D->Lmanager.curr_lin;
  int col = D->Lmanager.curr_col;
  if (lua_gettop(L) <= 2)
  {
    if (lua_type(L, -2) == LUA_TNUMBER)
      col = static_cast<int>(lua_tonumber(L, -1));
    else
    {
      lua_pushstring(L, "Error in script! Function cell() expected one or two numbers as parameters.");
      lua_error(L);
      return 0;
    }
  }
  if (lua_type(L, -1) == LUA_TNUMBER)
    lin = static_cast<int>(lua_tonumber(L, -2));
  else
  {
    lua_pushstring(L, "Error in script! Function cell() expected one or two numbers as parameters.");
    lua_error(L);
    return 0;
  }

  if (col == D->Lmanager.curr_col)
  {
    lua_pushstring(L, "Error in script! Circular reference found!");
    lua_error(L);
    return 0;
  }

  std::string s = D->GetValue(lin, col);
  if (s.empty())
  {
    lua_pushnil(L);
    return 1;
  }
  double num(0.0);
  if (sscanf(s.c_str(), "%lf", &num) == 1)
  {
    lua_pushnumber(L, num);
    return 1;
  }
  else
  {
    s.insert('\"', 0);
    s.append("\"");
    lua_pushstring(L, s.c_str());
    return 1;
  }
  lua_pushstring(L, "Error in script! Function cell() expected one or two numbers as parameters.");
  lua_error(L);
  return 0;
}

char *Dmatrix::GetValueLua(Column *C, int lin, int col)
{
  if (lin == 0)
    return (char*)C->macro.c_str();
  bool was_running = Lmanager.running;
  if (C->macro.empty() || !C->macro_ok)
    return "nil";
  if (!Lmanager.running)
  {
    current_dmatrix = this;
    Lmanager.running = true;
    Lmanager.curr_lin = lin;
    Lmanager.curr_col = col;
  }
  //luaL_dostring(Lmanager.L,C->macro.c_str());
  std::string e;
  std::string macro = "dmatrix_variable =";
  macro.append(C->macro);
  if ((luaL_loadstring(Lmanager.L, macro.c_str())) != 0)
  {
    e = lua_tostring(Lmanager.L, -1);
    e.append("\n");
    //e.append(C->macro_ok?"true":"false");
    //printf(e.c_str());
    C->macro_ok = false;
    IupMessage("Dmatrix Lua Alert!", e.c_str());
    if (!was_running) Lmanager.running = false;
    return "nil";
  }
  if ((lua_pcall(Lmanager.L, 0, 0, 0)) != 0)
  {
    e = lua_tostring(Lmanager.L, -1);
    e.append("\n");
    //printf(e.c_str());
    C->macro_ok = false;
    IupMessage("Dmatrix Lua Alert!", e.c_str());
    if (!was_running) Lmanager.running = false;
    return "nil";
  }
  lua_getglobal(Lmanager.L, "dmatrix_variable");
  if (!was_running)
    Lmanager.running = false;
  return (char*)lua_tostring(Lmanager.L, -1);
}

#endif

static int MatrixExSetFormula(lua_State *L)
{
  Ihandle *ih = iuplua_checkihandle(L,1);
  IupMatrixExSetFormula(ih, luaL_checkint(L, 2), luaL_checkstring(L, 3), luaL_optstring(L, 4, NULL));
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

