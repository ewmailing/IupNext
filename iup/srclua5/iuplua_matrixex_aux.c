/** \file
 * \brief matrixex binding for Lua 5.
 *
 * See Copyright Notice in "iup.h"
 */

#include <lua.h>
#include <lauxlib.h>

#include "iup.h"
#include "iupmatrixex.h"

#include "iuplua.h"
#include "il.h"

#include "iup_attrib.h"
#include "iup_object.h"


static int MatrixExInit(lua_State *L)
{
  Ihandle *ih = iuplua_checkihandle(L,1);
  IupMatrixExInit(ih);
  return 0;
}

void iuplua_matrixexfuncs_open (lua_State *L)
{
  iuplua_register(L, MatrixExInit, "MatrixExInit");
}
