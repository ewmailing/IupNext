/** \file
* \brief IUP binding for Lua 5.
*
* See Copyright Notice in iup.h
* */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "iup.h"
#include "iupkey.h"

#include <lua.h>
#include <lauxlib.h>

#include "iuplua.h"
#include "il.h"


int iupIsPointer(const char *attr); /* re-declared here to avoid inclusion of iglobal.h */
int iupIsInternal(const char* name);

static int Append(lua_State *L)
{
  iuplua_pushihandle(L, IupAppend(iuplua_checkihandle(L,1),
                                  iuplua_checkihandle(L,2)));
  return 1;
}

static int Destroy(lua_State *L)
{
  Ihandle* ih = iuplua_checkihandle(L,1);
  iuplua_removeihandle(L, ih);
  IupDestroy(ih);
  return 0;
}

static int Detach(lua_State *L)
{
  IupDetach(iuplua_checkihandle(L,1));
  return 0;
}

static int Flush(lua_State *L)
{
  (void)L; /* not used */
  IupFlush();
  return 0;
}

static int Version(lua_State *L)
{
  lua_pushstring(L, IupVersion());
  return 1;
}                                                                             

static int GetAttributeData (lua_State *L)
{
  Ihandle *ih = iuplua_checkihandle(L,1);
  const char *attribute = luaL_checkstring(L,2);
  const char *value = IupGetAttribute(ih, (char *)attribute);
  if (!value)
    lua_pushnil(L);
  else
    lua_pushlightuserdata(L, (void*)value);
  return 1;
}

static int GetAttribute (lua_State *L)
{
  Ihandle *ih = iuplua_checkihandle(L,1);
  const char *attribute = luaL_checkstring(L,2);
  const char *value = IupGetAttribute(ih, (char *)attribute);
  if (!value || iupIsInternal(attribute))
    lua_pushnil(L);
  else
  {
    if (iupIsPointer(attribute))
      lua_pushlightuserdata(L, (void*)value);
    else
      lua_pushstring(L,value);
  }
  return 1;
}

static int GetAttributes(lua_State *L)
{
  Ihandle *ih = iuplua_checkihandle(L,1);
  const char *value = IupGetAttributes(ih);
  lua_pushstring(L,value);
  return 1;
}

static int GetAllDialogs(lua_State *L)
{
  int max_n = luaL_checkint(L,1);
  char **names = (char **) malloc (max_n * sizeof(char *));
  int i,n;
  if (!names)
    lua_pushnil(L);
  n = IupGetAllDialogs(names, max_n);
  lua_newtable(L);
  for (i=0; i<n; i++)
  {
    lua_pushnumber(L,i+1);
    lua_pushstring(L,names[i]);
    lua_settable(L,-3);
  }
  lua_pushnumber(L,n);
  free(names);
  return 2;
}

static int GetAllNames(lua_State *L)
{
  int max_n = luaL_checkint(L,1);
  char **names = (char **) malloc (max_n * sizeof(char *));
  int i,n;
  if (!names)
    lua_pushnil(L);
  n = IupGetAllNames(names,max_n);
  lua_newtable(L);
  for (i=0; i<n; i++)
  {
    lua_pushnumber(L,i+1);
    lua_pushstring(L,names[i]);
    lua_settable(L,-3);
  }
  lua_pushnumber(L,n);
  free(names);
  return 2;
}

static int GetDialog(lua_State *L)
{
  iuplua_pushihandle(L, IupGetDialog(iuplua_checkihandle(L,1)));
  return 1;
}

static int GetFile (lua_State *L)
{
  const char *fname = luaL_checkstring(L,1);
  char returned_fname[10240];
  int ret;
  strcpy(returned_fname, fname);
  ret = IupGetFile(returned_fname);
  lua_pushstring(L, returned_fname);
  lua_pushnumber(L, ret);
  return 2;
}

static int GetFocus(lua_State *L)
{
  iuplua_pushihandle(L, IupGetFocus());
  return 1;
}

static int GetClassName(lua_State *L)
{
  Ihandle *ih = iuplua_checkihandle(L, 1);
  lua_pushstring(L, IupGetClassName(ih));
  return 1;
}

static int GetGlobal(lua_State *L)
{
  const char *a = luaL_checkstring(L,1);
  const char *v = IupGetGlobal(a);
  lua_pushstring(L,v);
  return 1;
}

static int GetHandle(lua_State *L)
{
  const char *name = luaL_checkstring(L,1);
  Ihandle *ih = IupGetHandle(name);
  iuplua_pushihandle(L,ih);
  return 1;
}

static int GetLanguage (lua_State *L)
{
  char * value = IupGetLanguage();
  lua_pushstring(L,value);
  return 1;
}

static int GetName(lua_State *L)
{
  Ihandle *ih = iuplua_checkihandle(L,1);
  char * name = IupGetName(ih);
  lua_pushstring(L,name);
  return 1;
}

static int Help(lua_State *L)
{
  char *s = (char *) luaL_checkstring(L,1);
  IupHelp(s);
  return 0;
}

static int Hide(lua_State *L)
{
  Ihandle *ih = iuplua_checkihandle(L,1);
  lua_pushnumber(L, IupHide(ih));
  return 1;
}

static int Load(lua_State *L)
{
  char *s = (char *) luaL_checkstring(L,1);
  const char *r = IupLoad(s);
  lua_pushstring(L,r);
  return 1;
}

static int LoopStep(lua_State *L)
{
  lua_pushnumber(L,IupLoopStep());
  return 1;
}

static int ExitLoop(lua_State *L)
{
  (void)L;
  IupExitLoop();
  return 0;
}

static int MainLoop(lua_State *L)
{
  lua_pushnumber(L,IupMainLoop());
  return 1;
}

static int Map(lua_State *L)
{
  Ihandle *ih = iuplua_checkihandle(L,1);
  lua_pushnumber(L, IupMap(ih));
  return 1;
}

static int MapFont(lua_State *L)
{
  char *font = (char *) luaL_checkstring(L,1);
  const char *nfont = IupMapFont(font);
  lua_pushstring(L, nfont);
  return 1;
}

static int Message(lua_State *L)
{
  char *title = (char *) luaL_checkstring(L,1);
  char *message = (char *) luaL_checkstring(L,2);
  IupMessage(title, message);
  return 0;
}

static int Alarm(lua_State *L)
{
  int n = IupAlarm((char *) luaL_checkstring(L, 1), 
                   (char *) luaL_checkstring(L, 2), 
                   (char *) luaL_checkstring(L, 3), 
                   (char *) luaL_optstring(L, 4, NULL), 
                   (char *) luaL_optstring(L, 5, NULL));
  lua_pushnumber(L, n);
  return 1;
}

static int ListDialog(lua_State *L)
{
  int type = luaL_checkint(L,1);
  int size = luaL_checkint(L,3);
  char** list = iuplua_checkstring_array(L, 4);
  int* marks = lua_isnoneornil(L, 8)? NULL: iuplua_checkint_array(L,8);
  int i, ret;

  if (size != iuplua_getn(L, 4))
    luaL_error(L, "invalid number of elements in the list.");
  if (!marks && type==2)
    luaL_error(L, "invalid marks, must not be nil.");
  if (marks && type==2 && size != iuplua_getn(L, 8))
    luaL_error(L, "invalid number of elements in the marks.");

  ret = IupListDialog(type, luaL_checkstring(L, 2), 
                            size, 
                            list, 
                            luaL_checkint(L, 5), 
                            luaL_checkint(L, 6), 
                            luaL_checkint(L, 7), 
                            marks);

  if (marks && type==2 && ret!=-1)
  {
    for (i=0; i<size; i++)
    {
      lua_pushnumber(L, i+1);
      lua_pushnumber(L, marks[i]);
      lua_settable(L, 8);
    }
  }

  lua_pushnumber(L, ret);
    
  if (marks) free(marks);
  free(list);

  return 1;
}

static int GetText(lua_State *L)
{
  char buffer[10240];
  char *title = (char *) luaL_checkstring(L,1);
  char *text = (char *) luaL_checkstring(L,2);
  strcpy(buffer, text);
  if (IupGetText(title, buffer))
  {
    lua_pushstring(L, buffer);
    return 1;
  }
  return 0;
}

static int NextField(lua_State *L)
{
  Ihandle *h1 = iuplua_checkihandle(L,1);
  Ihandle *h2 = IupNextField(h1);
  iuplua_pushihandle(L,h2);
  return 1;
}

static int PreviousField(lua_State *L)
{
  Ihandle *h1 = iuplua_checkihandle(L,1);
  Ihandle *h2 = IupNextField(h1);
  iuplua_pushihandle(L,h2);
  return 1;
}

static int Popup(lua_State *L)
{
  Ihandle *ih = iuplua_checkihandle(L,1);
  int x = luaL_optint(L,2, IUP_CURRENT);
  int y = luaL_optint(L,3, IUP_CURRENT);
  lua_pushnumber(L,IupPopup(ih,x,y));
  return 1;
}

static int cf_isprint(lua_State *L)
{
  int value = luaL_checkint(L, 1);
  lua_pushnumber(L, iup_isprint(value));
  return 1;
}

static int cf_xCODE(lua_State *L)
{
  int value = luaL_checkint(L, 1);
  lua_pushnumber(L, xCODE(value));
  return 1;
}

static int cf_isxkey(lua_State *L)
{
  int value = luaL_checkint(L, 1);
  lua_pushnumber(L, isxkey(value));
  return 1;
}

static int cf_isbutton1(lua_State *L)
{
  const char *value = luaL_checkstring(L, 1);
  if (isbutton1(value)) lua_pushnumber(L, 1);
  else lua_pushnil(L);
  return 1;
}

static int cf_isbutton2(lua_State *L)
{
  const char *value = luaL_checkstring(L, 1);
  if (isbutton2(value)) lua_pushnumber(L, 1);
  else lua_pushnil(L);
  return 1;
}

static int cf_isbutton3(lua_State *L)
{
  const char *value = luaL_checkstring(L, 1);
  if (isbutton3(value)) lua_pushnumber(L, 1);
  else lua_pushnil(L);
  return 1;
}

static int cf_isshift(lua_State *L)
{
  const char *value = luaL_checkstring(L, 1);
  if (isshift(value)) lua_pushnumber(L, 1);
  else lua_pushnil(L);
  return 1;
}

static int cf_iscontrol(lua_State *L)
{
  const char *value = luaL_checkstring(L, 1);
  if (iscontrol(value)) lua_pushnumber(L, 1);
  else lua_pushnil(L);
  return 1;
}

static int cf_isdouble(lua_State *L)
{
  const char *value = luaL_checkstring(L, 1);
  if (isdouble(value)) lua_pushnumber(L, 1);
  else lua_pushnil(L);
  return 1;
}

static int cf_isalt(lua_State *L)
{
  const char *value = luaL_checkstring(L, 1);
  if (isalt(value)) lua_pushnumber(L, 1);
  else lua_pushnil(L);
  return 1;
}

static int cf_issys(lua_State *L)
{
  const char *value = luaL_checkstring(L, 1);
  if (issys(value)) lua_pushnumber(L, 1);
  else lua_pushnil(L);
  return 1;
}

static int cf_isbutton4(lua_State *L)
{
  const char *value = luaL_checkstring(L, 1);
  if (isbutton4(value)) lua_pushnumber(L, 1);
  else lua_pushnil(L);
  return 1;
}

static int cf_isbutton5(lua_State *L)
{
  const char *value = luaL_checkstring(L, 1);
  if (isbutton5(value)) lua_pushnumber(L, 1);
  else lua_pushnil(L);
  return 1;
}

static int GetParent(lua_State *L)
{
  Ihandle * ih = iuplua_checkihandle(L,1);
  Ihandle * parent = IupGetParent(ih);
  iuplua_pushihandle(L, parent);
  return 1;
}

static int VersionNumber(lua_State *L)
{
  lua_pushnumber(L, IupVersionNumber());
  return 1;
}

static int GetNextChild(lua_State *L)
{
  Ihandle * parent = iuplua_checkihandle(L,1);
  Ihandle * next = iuplua_checkihandleornil(L,2);
  Ihandle * nextchild = IupGetNextChild(parent, next);
  iuplua_pushihandle(L, nextchild);
  return 1;
}

static int GetBrother(lua_State *L)
{
  Ihandle * ih = iuplua_checkihandle(L,1);
  Ihandle * brother = IupGetBrother(ih);
  iuplua_pushihandle(L, brother);
  return 1;
}

static int SetAttributes(lua_State *L)
{
  Ihandle * ih = iuplua_checkihandle(L,1);
  char *attributes = (char *) luaL_checkstring(L,2);
  IupSetAttributes(ih, attributes);
  iuplua_pushihandle(L,ih);
  return 1;
}

static int SetFocus(lua_State *L)
{
  Ihandle *h1 = iuplua_checkihandle(L,1);
  Ihandle *h2 = IupSetFocus(h1);
  iuplua_pushihandle(L,h2);
  return 1;
}

static int SetGlobal(lua_State *L)
{
  char *a = (char *) luaL_checkstring(L,1);
  char *v = (char *) luaL_checkstring(L,2);
  IupSetGlobal(a,v);
  return 0;
}

static int SetHandle(lua_State *L)
{
  const char *name = luaL_checkstring(L,1);
  Ihandle *ih = iuplua_checkihandle(L,2);
  Ihandle *last = IupSetHandle((char*)name, ih);
  iuplua_pushihandle(L, last);
  return 1;
}

static int SetLanguage(lua_State *L)
{
  IupSetLanguage((char *) luaL_checkstring(L,1));
  return 0;
}

static int Show (lua_State *L)
{
  lua_pushnumber(L, IupShow(iuplua_checkihandle(L,1)));
  return 1;
}

static int Refresh (lua_State *L)
{
  IupRefresh(iuplua_checkihandle(L,1));
  return 0;
}

static int Update (lua_State *L)
{
  IupUpdate(iuplua_checkihandle(L,1));
  return 0;
}

static int ShowXY(lua_State *L)
{
  Ihandle *ih = iuplua_checkihandle(L,1);
  int x = luaL_optint(L,2, IUP_CURRENT);
  int y = luaL_optint(L,3, IUP_CURRENT);
  lua_pushnumber(L,IupShowXY(ih,x,y));
  return 1;
}

static int StoreAttribute(lua_State *L)
{
  Ihandle *ih = iuplua_checkihandle(L,1);
  char *a = (char *) luaL_checkstring(L,2);
  if (lua_isnil(L,3)) 
    IupSetAttribute(ih,a,NULL);
  else 
  {
    char *v;
    if(lua_isuserdata(L,3)) 
    {
      v = (char *) lua_touserdata(L,3);
      IupSetAttribute(ih,a,v);
    }
    else 
    {
      v = (char *) luaL_checkstring(L,3);
      IupStoreAttribute(ih,a,v);
    }
  }
  return 0;
}

static int StoreGlobal(lua_State *L)
{
  char *a = (char *) luaL_checkstring(L,1);
  char *v = (char *) luaL_checkstring(L,2);
  IupStoreGlobal(a,v);
  return 0;
}

static int UnMapFont (lua_State *L)
{
  char *s = (char *) luaL_checkstring(L,1);
  char *n = (char *) IupUnMapFont(s);
  lua_pushstring(L,n);
  return 1;
}


/*****************************************************************************
* iupluaapi_open                                                               *
****************************************************************************/


int iupluaapi_open(lua_State * L)
{
  struct luaL_Reg funcs[] = {
    {"Append", Append},
    {"Destroy", Destroy},
    {"Detach", Detach},
    {"Flush", Flush},
    {"Version", Version},
    {"GetAttribute", GetAttribute},
    {"GetAttributeData", GetAttributeData},
    {"GetAttributes", GetAttributes},
    {"GetAllDialogs", GetAllDialogs},
    {"GetAllNames", GetAllNames},
    {"GetDialog", GetDialog},
    {"GetFile", GetFile},
    {"GetFocus", GetFocus},
    {"GetClassName", GetClassName},
    {"GetGlobal", GetGlobal},
    {"GetHandle", GetHandle},
    {"GetLanguage", GetLanguage},
    {"GetName", GetName},
    {"Help", Help},
    {"Hide", Hide},
    {"Load", Load},
    {"LoopStep", LoopStep},
    {"ExitLoop", ExitLoop},
    {"MainLoop", MainLoop},
    {"Map", Map},
    {"MapFont", MapFont},
    {"Message", Message},
    {"Alarm", Alarm},  
    {"ListDialog", ListDialog},
    {"GetText", GetText},
    {"NextField", NextField},
    {"Popup", Popup},
    {"PreviousField", PreviousField},
    {"SetAttribute", StoreAttribute},
    {"SetAttributes", SetAttributes},
    {"isbutton1", cf_isbutton1},
    {"isbutton2", cf_isbutton2},
    {"isbutton3", cf_isbutton3},
    {"isshift", cf_isshift},
    {"iscontrol", cf_iscontrol},
    {"isdouble", cf_isdouble},
    {"isalt", cf_isalt},
    {"issys", cf_issys},
    {"isbutton4", cf_isbutton4},
    {"isbutton5", cf_isbutton5},
    {"GetParent", GetParent},
    {"GetNextChild", GetNextChild},
    {"VersionNumber", VersionNumber},
    {"GetBrother", GetBrother},
    {"SetFocus", SetFocus},
    {"SetGlobal", SetGlobal},
    {"SetHandle", SetHandle},
    {"SetLanguage", SetLanguage},
    {"Show", Show},
    {"Refresh", Refresh},
    {"Update", Update},
    {"ShowXY", ShowXY},
    {"StoreAttribute", StoreAttribute},
    {"StoreGlobal", StoreGlobal},
    {"UnMapFont", UnMapFont},
    {"Scanf", iupluaScanf},
    {"isprint", cf_isprint},
    {"xCODE", cf_xCODE},
    {"isxkey", cf_isxkey},
    {NULL, NULL},
  };

  /* Registers functions in iup namespace */
  luaL_register(L, NULL, funcs);

  return 0; /* nothing in stack */
}
