/** \file
 * \brief iuptree binding for Lua 5.
 *
 * See Copyright Notice in "iup.h"
 */

#include <lua.h>
#include <lauxlib.h>

#include "iup.h"
#include "iupcontrols.h"

#include "iuplua.h"
#include "il.h"
#include "il_controls.h"

/* 
   The REGISTRY is used to store references to the associated Lua objects.
   Given an ID, to retreive a Lua object is quite simple.

   But given a user_id to obtain the ID is more complicated.
   The IUPTREEREFTABLE is used to do this mapping.
   We use the object as the index to this table.
*/

/* iup.IUPTREEREFTABLE[object at pos] = ref */
static void settableref(lua_State *L, int pos, int ref)
{
  lua_getglobal(L, "iup");
  lua_pushstring(L, "IUPTREEREFTABLE");
  lua_gettable(L, -2);
  lua_remove(L, -2);
  lua_pushvalue(L, pos);
  if(ref == 0)
    lua_pushnil(L);
  else
    lua_pushnumber(L, ref);
  lua_settable(L, -3);
  lua_pop(L, 1);
}

/* ref = iup.IUPTREEREFTABLE[object at pos] */
static int gettableref(lua_State *L, int pos)
{
  int ref = 0;
  lua_getglobal(L, "iup");
  lua_pushstring(L, "IUPTREEREFTABLE");
  lua_gettable(L, -2);
  lua_remove(L, -2);
  lua_pushvalue(L, pos);
  lua_gettable(L, -2);
  if(lua_isnil(L, -1))
  {
    lua_pop(L, 1);
    return 0;
  }
  ref = (int) lua_tonumber(L, -1);
  lua_pop(L, 1);
  return ref;
}

/*****************************************************************************
 * Userdata/Table <-> id functions 
 ****************************************************************************/

static int TreeGetId(lua_State *L)
{
  Ihandle *h = iuplua_checkihandle(L,1);
  int ref = gettableref(L, 2);
  if(ref == 0)
  {
    lua_pushnil(L);
  }
  else
  {
    int id = IupTreeGetId(h, (char*) ref);
    if(id < 0)
      lua_pushnil(L);
    else
      lua_pushnumber(L,id);
  }
  return 1;        
}

static int TreeGetUserId(lua_State *L)
{  
  int ref;
  Ihandle *h = iuplua_checkihandle(L,1);
  int id = (int)luaL_checknumber(L,2);
  ref = (int) IupTreeGetUserId(h, id) - 1;
  lua_getref(L, ref);
  return 1;
}

static int TreeSetUserId(lua_State *L)
{  
  Ihandle *h = iuplua_checkihandle(L,1);
  int id = (int)luaL_checknumber(L,2);
  int ref = (int)IupTreeGetUserId(h, id) - 1;
  if (ref != LUA_NOREF)
  {
    /* always remove old references */
    lua_getref(L, ref);
    settableref(L, 4, 0);
    lua_unref(L, ref);
    lua_pop(L, 1);
  }

  if (lua_isnil(L, 3))
    IupTreeSetUserId(h, id, NULL);
  else
  {
    /* add a new reference */
    lua_pushvalue(L, 3);
    ref = lua_ref(L, 1) + 1;
    settableref(L, 3, ref);
    IupTreeSetUserId(h, id, (char*)ref);
  }

  return 0;
}

static int tree_multiselection_cb(Ihandle *self, int* ids, int p1)
{
  int i;
  lua_State *L = iuplua_call_start(self, "multiselection_cb");
  lua_newtable(L);
  for (i = 0; i < p1; i++)
  {
    lua_pushnumber(L,i+1);
    lua_pushnumber(L,ids[i]);
    lua_settable(L,-3);
  }
  lua_pushnumber(L, p1);
  return iuplua_call(L, 2);
}

static int tree_noderemoved_cb(Ihandle *self, int id, char * p1)
{
  int ref;
  lua_State *L = iuplua_call_start(self, "noderemoved_cb");
  lua_pushnumber(L, id);
  ref = ((int)p1) - 1;
  lua_getref(L, ref);
  return iuplua_call(L, 2);
}

void iuplua_treefuncs_open (lua_State *L)
{
  iuplua_dostring(L, "IUPTREEREFTABLE={}", "");

  iuplua_register_cb(L, "MULTISELECTION_CB", (lua_CFunction)tree_multiselection_cb, NULL);
  iuplua_register_cb(L, "NODEREMOVED_CB", (lua_CFunction)tree_noderemoved_cb, NULL);

/* In Lua 5:
  TreeSetTableId = TreeSetUserId
  TreeGetTable   = TreeGetUserId
  TreeGetTableId = TreeGetId
*/

  /* Userdata <-> id */
  iuplua_register(L, TreeGetId, "TreeGetId");
  iuplua_register(L, TreeGetUserId, "TreeGetUserId");
  iuplua_register(L, TreeSetUserId, "TreeSetUserId");

  /* Table <-> id */
  iuplua_register(L, TreeGetId, "TreeGetTableId");
  iuplua_register(L, TreeGetUserId, "TreeGetTable");
  iuplua_register(L, TreeSetUserId, "TreeSetTableId");
}
