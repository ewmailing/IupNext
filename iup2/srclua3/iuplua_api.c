/** \file
 * \brief Iup API in Lua
 *
 * See Copyright Notice in iup.h
 * $Id: iuplua_api.c,v 1.3 2009-03-16 21:01:28 scuri Exp $
 */
 
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <lua.h>
#include <lauxlib.h>

#include "iup.h"
#include "iupkey.h"

#include "iuplua.h"
#include "il.h"

#if (IUP_VERSION_NUMBER >= 300000)
#include "iup_attrib.h"
static void Reparent(void)
{
  lua_pushnumber(IupReparent(iuplua_checkihandle(1),
                             iuplua_checkihandle(2)));
}
#define iupIsPointer iupAttribIsPointer
#define iupIsInternal iupAttribIsInternal
#define xCODE IUPxCODE
#else
int iupIsPointer(const char *attr); /* re-declared here to avoid inclusion of iglobal.h */
int iupIsInternal(const char* name);
#endif


static void IupLuaPreviousField(void)
{
  iuplua_pushihandle(IupPreviousField(iuplua_checkihandle(1)));
}

static void IupLuaNextField(void)
{
  iuplua_pushihandle(IupNextField(iuplua_checkihandle(1)));
}

static void cf_isprint(void)
{
  int cod = luaL_check_int(1);
  lua_pushnumber(iup_isprint(cod));
}

static void codekey(void)
{
  int cod = luaL_check_int(1);
  lua_pushnumber(xCODE(cod));
}

static void iscodekey(void)
{
  int cod = luaL_check_int(1);
  lua_pushnumber(isxkey(cod));
}

static void cf_isbutton1(void)
{
  if(isbutton1(luaL_check_string(1)))
    lua_pushnumber(1);
  else
    lua_pushnil();
}

static void cf_isbutton2(void)
{
  if(isbutton2(luaL_check_string(1)))
    lua_pushnumber(1);
  else
    lua_pushnil();
}

static void cf_isbutton3(void)
{
  if(isbutton3(luaL_check_string(1)))
    lua_pushnumber(1);
  else
    lua_pushnil();
}

static void cf_isshift(void)
{
  if(isshift(luaL_check_string(1)))
    lua_pushnumber(1);
  else
    lua_pushnil();
}

static void cf_isdouble(void)
{
  if(isdouble(luaL_check_string(1)))
    lua_pushnumber(1);
  else
    lua_pushnil();
}

static void cf_isalt(void)
{
  if(isalt(luaL_check_string(1)))
    lua_pushnumber(1);
  else
    lua_pushnil();
}

static void cf_isbutton4(void)
{
  if(isbutton4(luaL_check_string(1)))
    lua_pushnumber(1);
  else
    lua_pushnil();
}

static void cf_isbutton5(void)
{
  if(isbutton5(luaL_check_string(1)))
    lua_pushnumber(1);
  else
    lua_pushnil();
}

static void cf_issys(void)
{
  if(issys(luaL_check_string(1)))
    lua_pushnumber(1);
  else
    lua_pushnil();
}

static void cf_iscontrol(void)
{
  if(iscontrol(luaL_check_string(1)))
    lua_pushnumber(1);
  else
    lua_pushnil();
}

static void GetAttributeData(void)
{
  char *value = IupGetAttribute(iuplua_checkihandle(1), luaL_check_string(2));
  if (value)
    lua_pushuserdata((void*)value);
  else
    lua_pushnil();
}

static void GetAttribute(void)
{
  char *name = luaL_check_string(2);
  char *value = IupGetAttribute(iuplua_checkihandle(1), name);
  if (!value || iupIsInternal(name))
    lua_pushnil();
  else
  {
    if (iupIsPointer(name))
      lua_pushuserdata((void*)value);
    else
      lua_pushstring(value);
  }
}

static void GetDialog(void)
{
  iuplua_pushihandle(IupGetDialog(iuplua_checkihandle(1)));
}

static void GetHandle(void)
{
  iuplua_pushihandle(IupGetHandle(luaL_check_string(1)));
}

static void SetAttribute(void)
{
  if (lua_isnil(lua_getparam(3)))
    IupSetAttribute(iuplua_checkihandle(1), luaL_check_string(2), NULL);
  else
    IupStoreAttribute(iuplua_checkihandle(1), luaL_check_string(2), luaL_check_string(3));
}

static void SetHandle(void)
{
  if (lua_isnil(lua_getparam(2)))
    IupSetHandle(luaL_check_string(1), NULL);
  else
    IupSetHandle(luaL_check_string(1), iuplua_checkihandle(2));
}

static void Destroy(void)
{
  IupDestroy(iuplua_checkihandle(1));
}

static void Detach(void)
{
  IupDetach(iuplua_checkihandle(1));
}

static void Map(void)
{
  lua_pushnumber(IupMap(iuplua_checkihandle(1)));
}

#if (IUP_VERSION_NUMBER >= 300000)
static void Unmap(void)
{
  IupUnmap(iuplua_checkihandle(1));
}
#endif

static void Show(void)
{
  lua_pushnumber(IupShow(iuplua_checkihandle(1)));
}

static void Refresh(void)
{
  IupRefresh(iuplua_checkihandle(1));
}

static void Update(void)
{
  IupUpdate(iuplua_checkihandle(1));
}

static void VersionNumber(void)
{
  lua_pushnumber(IupVersionNumber());
}

static void ShowXY(void)
{
  lua_pushnumber(IupShowXY(iuplua_checkihandle(1), 
                           luaL_opt_int(2, IUP_CURRENT), luaL_opt_int(3, IUP_CURRENT)));
}

static void Hide(void)
{
  lua_pushnumber(IupHide(iuplua_checkihandle(1)));
}

static void Popup(void)
{
  lua_pushnumber(IupPopup(iuplua_checkihandle(1),
                          luaL_opt_int(2, IUP_CURRENT), luaL_opt_int(3, IUP_CURRENT)));
}

static void Append(void)
{
  iuplua_pushihandle(IupAppend(iuplua_checkihandle(1), iuplua_checkihandle(2)));
}

static void GetNextChild(void)
{
  if (lua_isnil(lua_getparam(2)))
    iuplua_pushihandle(IupGetNextChild(iuplua_checkihandle(1), NULL));
  else
    iuplua_pushihandle(IupGetNextChild(iuplua_checkihandle(1), iuplua_checkihandle(2)));
}

static void GetBrother(void)
{
  iuplua_pushihandle(IupGetBrother(iuplua_checkihandle(1)));
}

static void ClassName(void)
{
  lua_pushstring(IupGetClassName(iuplua_checkihandle(1)));
}

static void GetFocus(void)
{
  iuplua_pushihandle(IupGetFocus());
}

static void SetFocus(void)
{
  IupSetFocus(iuplua_checkihandle(1));
}

static void GetName(void)
{
  char* s = IupGetName(iuplua_checkihandle(1));
  lua_pushstring(s);
}

static void Alarm(void)
{
  lua_pushnumber(IupAlarm(luaL_check_string(1), 
                          luaL_check_string(2),
                          luaL_check_string(3), 
                          luaL_opt_string(4, NULL),
                          luaL_opt_string(5, NULL)));
}

static void GetFile(void)
{
  char buffer[10240];
  int ret;
  strcpy(buffer, luaL_check_string(1));
  ret = IupGetFile(buffer);
  lua_pushstring(buffer);
  lua_pushnumber(ret);
}

static void GetParent(void)
{
  Ihandle *p = IupGetParent(iuplua_checkihandle(1));
  iuplua_pushihandle(p);
}

static void MapFont(void)
{
  lua_pushstring(IupMapFont(luaL_check_string(1)));
}

static void UnMapFont(void)
{
  lua_pushstring(IupUnMapFont(luaL_check_string(1)));
}

static void GetLanguage(void)
{
  lua_pushstring(IupGetLanguage());
}

static void SetLanguage(void)
{
  IupSetLanguage(luaL_check_string(1));
}

static void ListDialog(void)
{
  lua_Object strings = luaL_tablearg(4);
  lua_Object flags = luaL_tablearg(8);
  int tipo, tam, opt, max_col, max_lin, i, ret;
  char *tit;
  char **lista;
  int *marcas;

  tipo = luaL_check_int(1);
  tit = luaL_check_string(2);
  tam = luaL_check_int(3);
  opt = luaL_check_int(5);
  max_col = luaL_check_int(6);
  max_lin = luaL_check_int(7);
  lista = malloc(sizeof(char *) * tam);
  marcas = malloc(sizeof(int) * tam);
  for (i = 0; i < tam; i++) 
  {
    lua_beginblock();
    lua_pushobject(strings);
    lua_pushnumber(i + 1);
    lista[i] = lua_getstring(lua_gettable());
    lua_pushobject(flags);
    lua_pushnumber(i + 1);
    marcas[i] = (int) lua_getnumber(lua_gettable());
    lua_endblock();
  }

  ret = IupListDialog(tipo, tit, tam, lista, opt, max_col, max_lin, marcas);

  if (tipo==2 && ret!=-1)
  {
    for (i = 0; i < tam; i++) 
    {
      lua_beginblock();
      lua_pushobject(flags);
      lua_pushnumber(i + 1);
      lua_pushnumber(marcas[i]);
      lua_settable();
      lua_endblock();
    }
  }

  lua_pushnumber(ret);
}

static void Message(void)
{
  IupMessage(luaL_check_string(1), luaL_check_string(2));
}

static void GetText(void)
{
  char buffer[10240];
  strcpy(buffer, luaL_check_string(2));
  if(IupGetText(luaL_check_string(1), buffer))
    lua_pushstring(buffer);
  else
    lua_pushnil();
}

static void GetAllNames(void)
{
  int max_n = luaL_check_int(1);
  char **names = (char **) malloc(max_n * sizeof(char *));
  int i;
  int n = IupGetAllNames(names, max_n);

  lua_Object tb = lua_createtable();
  for (i = 0; i < n; i++) 
  {
    lua_beginblock();
    lua_pushobject(tb);
    lua_pushnumber(i);
    lua_pushstring(names[i]);
    lua_settable();
    lua_endblock();                /* end a section and starts another */
  }

  lua_pushobject(tb);
  lua_pushnumber(n);
  free(names);
}

static void GetAllDialogs(void)
{
  int max_n = luaL_check_int(1);
  char **names = (char **) malloc(max_n * sizeof(char *));
  int i;
  int n = IupGetAllDialogs(names, 100);

  lua_Object tb = lua_createtable();
  for (i = 0; i < n; i++) 
  {
    lua_beginblock();
    lua_pushobject(tb);
    lua_pushnumber(i);
    lua_pushstring(names[i]);
    lua_settable();
    lua_endblock();                /* end a section and starts another */
  }

  lua_pushobject(tb);
  lua_pushnumber(n);
  free(names);
}

static void GetGlobal(void)
{
  lua_pushstring(IupGetGlobal(luaL_check_string(1)));
}

static void SetGlobal(void)
{
  IupStoreGlobal(luaL_check_string(1), luaL_check_string(2));
}

static void LoopStep(void)
{
  lua_pushnumber(IupLoopStep());
}

static void ExitLoop(void)
{
  IupExitLoop();
}

static void Version(void)
{
  lua_pushstring(IupVersion());
}

static void MainLoop(void)
{
  lua_pushnumber(IupMainLoop());
}

static void Open(void)
{
  lua_pushnumber(IupOpen(NULL, NULL));
}

static void Help(void)
{
  IupHelp(luaL_check_string(1));
}

int iupluaapi_open(void)
{
  struct FuncList {
    char *name;
    lua_CFunction func;
  } FuncList[] = {
    { "IupGetAttribute", GetAttribute },
    { "IupGetAttributeData", GetAttributeData },
    { "IupGetHandle", GetHandle },
    { "IupGetDialog", GetDialog },
    { "IupSetAttribute", SetAttribute },
    { "IupSetHandle", SetHandle },
    { "IupDestroy", Destroy },
    { "IupDetach", Detach },
    { "IupMap", Map },
#if (IUP_VERSION_NUMBER >= 300000)
    { "IupUnmap", Unmap },
#endif
    { "IupShow", Show },
    { "IupRefresh", Refresh },
    { "IupUpdate", Update },
    { "IupVersionNumber", VersionNumber },
    { "IupShowXY", ShowXY },
    { "IupHide", Hide },
    { "IupPopup", Popup },
    { "IupAppend", Append },
#if (IUP_VERSION_NUMBER >= 300000)
    { "IupReparent", Reparent },
#endif
    { "IupGetNextChild", GetNextChild },
    { "IupGetBrother", GetBrother },
    { "IupGetClassName", ClassName },
    { "IupGetFocus", GetFocus },
    { "IupSetFocus", SetFocus },
    { "IupGetName", GetName },
    { "IupAlarm", Alarm },
    { "IupGetFile", GetFile },
    { "IupMapFont", MapFont },
    { "IupGetParent", GetParent },
    { "IupUnMapFont", UnMapFont },
    { "IupSetLanguage", SetLanguage },
    { "IupGetLanguage", GetLanguage },
    { "IupListDialog", ListDialog },
    { "IupMessage", Message },
    { "IupGetText", GetText },
    { "IupGetAllNames", GetAllNames },
    { "IupGetAllDialogs", GetAllDialogs },
    { "IupGetGlobal", GetGlobal },
    { "IupSetGlobal", SetGlobal },
    { "IupLoopStep", LoopStep },
    { "IupExitLoop", ExitLoop },
    { "IupMainLoop", MainLoop },
    { "IupOpen", Open },
    { "IupClose", (lua_CFunction)IupClose },
    { "IupFlush", IupFlush },
    { "IupVersion", Version },
    { "IupHelp", Help },
    { "IupScanf", iupluaScanf },
    { "IupPreviousField", IupLuaPreviousField },
    { "IupNextField", IupLuaNextField },
    { "IupMainLoop", (lua_CFunction)IupMainLoop }
  };
  int SizeFuncList = (sizeof(FuncList)/sizeof(struct FuncList));
  int i;

  for (i = 0; i < SizeFuncList; i++)
    iuplua_register(FuncList[i].name, FuncList[i].func);

  iuplua_register_macro("isbutton1",cf_isbutton1);
  iuplua_register_macro("isbutton2",cf_isbutton2);
  iuplua_register_macro("isbutton3",cf_isbutton3);
  iuplua_register_macro("isshift",cf_isshift);
  iuplua_register_macro("iscontrol",cf_iscontrol);
  iuplua_register_macro("isdouble",cf_isdouble);
  iuplua_register_macro("isalt",cf_isalt);
  iuplua_register_macro("issys",cf_issys);
  iuplua_register_macro("isbutton4",cf_isbutton4);
  iuplua_register_macro("isbutton5",cf_isbutton5);
  iuplua_register_macro("isprint",cf_isprint);
  iuplua_register_macro("xCODE", codekey);
  iuplua_register_macro("isxkey", iscodekey);

  return 1;
}
