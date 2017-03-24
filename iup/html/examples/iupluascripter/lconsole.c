/*
 * daVinci - Lua Console
 *
 * Copyright (C) 1999-2008, Antonio E. Scuri, Mauro Charão, Tecgraf/PUC-Rio.
 */
#include "lconsole.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <iupkey.h>
#include <lauxlib.h>

extern lua_State *lcmd_state;

extern Ihandle *main_dialog;

console_st console;

char* utlIndent(int level)
{
  switch (level)
  {
    case 0: return "";
    case 1: return "  ";
    case 2: return "    ";
    case 3: return "      ";
    case 4: return "        ";
    case 5: return "          ";
    default:
    case 6: return "            ";
  }
}

char* utlStr(int num_char)
{
  char *new_str = malloc(num_char + 1);

  new_str[0] = '\0';
  return new_str;
}

//====================================================================================//
//                                       Print                                        //
//====================================================================================//
void lcmdConsolePrintObject(lua_State *state, char *name, int idx)
{
  int hold_caret;
  double number;
	lua_Debug ar;

  if(lua_type(state, idx)==LUA_TSTRING)
    lcmdConsoleEnterMessagef("%s%s = \"%s\"", utlIndent(console.ind_level), name, (char*)lua_tostring(state, idx));
  else if(lua_isuserdata(state, idx))
    lcmdConsoleEnterMessagef("%s%s = <userdata> (type = %s)", utlIndent(console.ind_level), name, lua_typename(state, lua_type(state, idx)));
  else if(lua_isnil(state, idx))
    lcmdConsoleEnterMessagef("%s%s = nil", utlIndent(console.ind_level), name);
  else if(lua_isnumber(state, idx))
  {
    if((number = lua_tonumber(state, idx))== floor(number)) // check if it an integer
      lcmdConsoleEnterMessagef("%s%s = %d", utlIndent(console.ind_level), name, (int)number);
    else lcmdConsoleEnterMessagef("%s%s = %f", utlIndent(console.ind_level), name, number);
  }
  else if(lua_istable(state, idx))
  {
    if(console.ind_level < 6)
    {
      lcmdConsoleEnterMessagef("%s%s = ", utlIndent(console.ind_level), name);
      lcmdConsoleEnterMessagef("%s{", utlIndent(console.ind_level));
      console.ind_level++;
      hold_caret = console.hold_caret;
      if(!hold_caret) lcmdConsoleHoldCaret(1);
      lua_getfield(state, LUA_GLOBALSINDEX, "cmd"); // push 'cmd' table in the stack
      lua_getfield(state, -1, "consolePrintTable"); // push the function cmd.consolePrintTable
      lua_remove(state, -2);          // remove 'cmd' table from the stack
      lua_pushvalue(state, idx);      // push the first argument
      lua_call(state, 1, 0);
      if(!hold_caret) lcmdConsoleHoldCaret(0);
      console.ind_level--;
      lcmdConsoleEnterMessagef("%s}", utlIndent(console.ind_level));
    }
    else lcmdConsoleEnterMessagef("%s{table too deep}", utlIndent(console.ind_level));
  }
  else if(lua_isfunction(state, idx)|| lua_iscfunction(state, idx))
  {
    lua_pushvalue(state, idx);        // put the function in the stack
    lua_getinfo(state, ">Snlu", &ar); // get the function info
    if(lua_iscfunction(state, idx))
    {
      if(ar.name != NULL)
        lcmdConsoleEnterMessagef("%s%s = <cfunction> (\"%s\" %s)", utlIndent(console.ind_level), name, ar.name, ar.namewhat);
      else lcmdConsoleEnterMessagef("%s%s = <cfunction>", utlIndent(console.ind_level), name);
    } // lua_isfunction
    else if(ar.name != NULL)
    {
      if(ar.source[0] != '@')
        lcmdConsoleEnterMessagef("%s%s = <function> (\"%s\" %s) [defined in a string at line %d]", utlIndent(console.ind_level), name, ar.name, ar.namewhat, ar.linedefined);
      else lcmdConsoleEnterMessagef("%s%s = <function> (\"%s\" %s) [defined in the file \"%s\" at line %d]", utlIndent(console.ind_level), name, ar.name, ar.namewhat, ar.source, ar.linedefined);
    }
    else if(ar.source[0] != '@')
      lcmdConsoleEnterMessagef("%s%s = <function> [defined in a string at line %d]", utlIndent(console.ind_level), name, ar.linedefined);
    else lcmdConsoleEnterMessagef("%s%s = <function> [defined in the file \"%s\" at line %d]", utlIndent(console.ind_level), name, ar.source, ar.linedefined);
  }
  else lcmdConsoleEnterMessagef("%s%s = <unknown> (type = %s)", utlIndent(console.ind_level), name, lua_typename(state, lua_type(state, idx)));
}

int luaConsolePrintObject(lua_State *state)
{
  char *name = (char*)lua_tostring(state, 1);
  int idx = (int)lua_tonumber(state, 2);

  lcmdConsolePrintObject(state, name, idx);
  return 0;
}

int luaConsolePrintObjectValue(lua_State *state)
{
  char *name = (char*)lua_tostring(state, 2);

  lcmdConsolePrintObject(state, name, 1);
  return 0;
}

int luaHoldCaret(lua_State *state)
{
  int hold = (int)lua_tonumber(state, 1);

  lcmdConsoleHoldCaret(hold);
  return 0;
}

int cmdPrintFuncVar(lua_State *state)
{ // used only by the cmd.consoleListFunc
  int name = lua_type(state, 1);
  int func = lua_type(state, 2);

  if(name == LUA_TNONE || func == LUA_TNONE || !lua_isstring(state, 1) || !lua_isfunction(state, 2))
  {
    lua_pushstring(state, "cmd.consolePrintFuncVar: invalid parameters.");
    lua_error(state);
  }
  if(lua_isfunction(state, 2))
    lcmdConsoleEnterMessagef("%s (C)", (char*)lua_tostring(state, 1));
  else lcmdConsoleEnterMessage((char*)lua_tostring(state, 1), 1);
  return 0;
}

int cmdPrintVar(lua_State *state)
{ // used only by the cmd.consolePrintTable
  char name[512];
	int table_flag = lua_type(state, 3);

  if(table_flag == LUA_TNONE)
    sprintf(name, "%s", (char*)lua_tostring(state, 1));
  else if(lua_type(state, 1)==LUA_TSTRING)
    sprintf(name, "[\"%s\"]", (char*)lua_tostring(state, 1));
  else sprintf(name, "[%s]", (char*)lua_tostring(state, 1));
  lcmdConsolePrintObject(state, name, 2);
  return 0;
}

int cmdPrintVarType(lua_State *state)
{ // used only by the cmd.consoleListVar
  char name[512];
	int table_flag = lua_type(state, 3);

  if(table_flag == LUA_TNONE)
    sprintf(name, "%s", (char*)lua_tostring(state, 1));
  else if(lua_type(state, 1)==LUA_TSTRING)
    sprintf(name, "[\"%s\"]", (char*)lua_tostring(state, 1));
  else sprintf(name, "[%s]", (char*)lua_tostring(state, 1));
  lcmdConsoleEnterMessagef("%s%s = \"%s\"", utlIndent(console.ind_level), name, lua_typename(state, lua_type(state, 2)));
  return 0;
}

void lcmdConsolePrintResult(lua_State *state)
{
  char buf[16];
  int i, n = lua_gettop(state); // number of arguments

  for(i = 1; i <= n; i++)
  {
    if(lua_type(state,i)!=LUA_TNONE && lua_type(state,i)!=LUA_TNIL)
    {
      sprintf(buf, "<%dº return>", i);
      console.ind_level = 0;
      lcmdConsolePrintObject(state, buf, i);
    }
  }
}

//====================================================================================//
//                              Lua Overriden Functions                               //
//====================================================================================//
int lua_print(lua_State *state)
{
  char buffer[10240] = "";
  int i, n = lua_gettop(state); // number of arguments

  for(i = 1; i <= n; i++)
  {
    if(i > 1)
      strcat(buffer, "\t");
    if(lua_isstring(state, i))
      strcat(buffer, (char*)lua_tostring(state, i));
    else if(lua_isnil(state, i))
      strcat(buffer, "nil");
    else if(lua_istable(state, i))
      strcat(buffer, "<table>");
    else if(lua_iscfunction(state, i))
      strcat(buffer, "<cfunction>");
    else if(lua_isfunction(state, i))
      strcat(buffer, "<function>");
    else if(lua_isuserdata(state, i))
      strcat(buffer, "<userdata>");
    else strcat(buffer, "<unknown>");
  }
  lcmdConsoleEnterMessage(buffer, 0);
  return 0;
}

int lcmdConsoleErrorMsgFunc(lua_State *state)
{
  lcmdConsoleEnterMessagef("lua: %s\n", lua_isstring(state, 1) ? (char*)lua_tostring(state,1):"(no messsage)");
  return 0;
}

void lcmdConsolePrintExpression(lua_State *state, char *command)
{ // used only by lcmdConsoleEnterCommand
  char *buffer = malloc(strlen(command)+10);

  // compile an expression that can be evaluated
  sprintf(buffer, "%s", command);//xx sprintf(buffer, "return (%s)", command);
  if(luaL_dostring(state, buffer)==0)
    lcmdConsolePrintResult(state);     // no problems, just print the results
  else lcmdConsoleErrorMsgFunc(state); // show the error into console
  free(buffer);
  lua_pop(state, lua_gettop(state));   // clean the stack
}

void lcmdConsoleEnterCommand(char *command)
{
  int hold_caret = console.hold_caret;

  if(!hold_caret) lcmdConsoleHoldCaret(1);
  command = iupStrDup(command);
  // prints the command in the console
  lcmdConsoleEnterCommandStr(command);
	lua_getfield(lcmd_state, LUA_GLOBALSINDEX, command);
  if(lua_gettop(lcmd_state)>0 && !lua_isnil(lcmd_state, lua_gettop(lcmd_state)))
  {
    console.ind_level = 0;
    lcmdConsolePrintObject(lcmd_state, command, lua_gettop(lcmd_state));
  } // if its not an object, may be an expresssion
  else lcmdConsolePrintExpression(lcmd_state, command);
  free(command);
  lua_pop(lcmd_state, lua_gettop(lcmd_state)); // clean the stack
  if(!hold_caret) lcmdConsoleHoldCaret(0);
  IupSetAttribute(console.txCmdLine, IUP_VALUE, "");
  IupSetFocus(console.txCmdLine);
}

void lcmdConsoleHoldCaret(int hold)
{
  if(!hold) IupSetAttribute(console.mlOutput, IUP_CARET, "65535,65535");
  console.hold_caret = hold;
}

int lcmdConsoleOutputSizeDlgCb(Ihandle *ih)
{
  int size = lcmdConsoleOutputSize(0);

  if(IupScanf("Console Output\nSize: %5.10%d\n",&size)!=-1)
    lcmdConsoleOutputSize(size);
  return IUP_DEFAULT;
}

int lcmdConsoleOutputSize(int size)
{
  int len, old_size = console.output_size;
  char *value, *buffer;

  if(size == 0) return old_size;
  console.output_size = size;
  if(size < old_size)
  {
    value = IupGetAttribute(console.mlOutput, IUP_VALUE);
    if((int)strlen(value)> size)
    {
      len = size-1;
      while(len > 0 && value[len] != '\n')
        len--;
      buffer = utlStr(len+2);
      memcpy(buffer, value, len);
      buffer[len] = '\n';
      buffer[len+1] = 0;
      IupStoreAttribute(console.mlOutput, IUP_VALUE, buffer);
      if(!console.hold_caret) IupSetAttribute(console.mlOutput, IUP_CARET, "65535,65535");
      free(buffer);
    }
  }
  return old_size;
}

int lcmdConsoleClearOutputCb(Ihandle *ih)
{
  IupSetAttribute(console.mlOutput, IUP_VALUE, "");
  return IUP_DEFAULT;
}

int luaConsoleClearOutput(lua_State *state)
{
  IupSetAttribute(console.mlOutput, IUP_VALUE, "");
  return 0;
}

void lcmdConsoleEnterCommandStr(char *text)
{
  utlDListNode *node = utlDListFindStr(console.command_list, text);

  if(node!=NULL) console.cur_list_node = node;
  else
  { // insert just new commands
    utlDListInsertBottom(console.command_list, iupStrDup(text));
    console.cur_list_node = console.command_list->bottom;
  }
  lcmdConsoleEnterMessagef("> %s", text);
}

int luaConsoleEnterCommandStr(lua_State *state)
{
  char *text = (char*)lua_tostring(state, 1);
  lcmdConsoleEnterCommandStr(text);
  return 0;
}

void lcmdConsoleEnterCommandStrf(char *format, ...)
{
  char value[65535];
  va_list arglist;

  va_start(arglist, format);
  vsprintf(value, format, arglist);
  va_end(arglist);
  lcmdConsoleEnterCommandStr(value);
}

void lcmdConsoleEnterMessagef(char *format, ...)
{
  char value[65535];
  va_list arglist;

  va_start(arglist, format);
  vsprintf(value, format, arglist);
  va_end(arglist);
  lcmdConsoleEnterMessage(value, 1);
}

int luaConsoleEnterMessage(lua_State *state)
{
  char *text = (char*)lua_tostring(state, 1);
  lcmdConsoleEnterMessage(text, 1);
  return 0;
}

void lcmdConsoleEnterMessage(char *text, int insert_new_line)
{
  char *buffer, *value = IupGetAttribute(console.mlOutput, IUP_VALUE);
  int start_value = 0, len_value = (int)strlen(value);
  int start_text = 0, len_text = (int)strlen(text);

  if(len_value + len_text > console.output_size)
  {
    start_value = len_value + len_text - console.output_size;
    if(start_value > len_value)
    {
      len_value = 0;
      if(len_text > console.output_size)
      {
        start_text = len_text - console.output_size;
        len_text = console.output_size-2;
      }
    }
    else
    {
      while(start_value < len_value && value[start_value] != '\n')
        start_value++;
      start_value++;
      len_value -= start_value;
    }
  }
  buffer = utlStr(len_text+len_value+2);
  if(len_value) memcpy(buffer, value + start_value, len_value);
  memcpy(buffer+len_value, text+start_text, len_text);
  if(insert_new_line)
  {
    buffer[len_text+len_value] = '\n';
    buffer[len_text+len_value+1] = '\0';
  }
  else buffer[len_text+len_value] = '\0';
  IupStoreAttribute(console.mlOutput, IUP_VALUE, buffer);
  if(!console.hold_caret) IupSetAttribute(console.mlOutput, IUP_CARET, "65535,65535");
  free(buffer);
}


//====================================================================================//
//                                       Window                                       //
//====================================================================================//
int lcmdConsoleListFuncCb(Ihandle *ih)
{
  lcmdConsoleEnterCommandStr("cmd.consoleListFunc()");
  lua_getfield(lcmd_state, LUA_GLOBALSINDEX, "cmd"); // push 'cmd' table in the stack
  lua_getfield(lcmd_state, -1, "consoleListFunc");   // push the function cmd.consoleListFunc
  lua_remove(lcmd_state, -2); // remove 'cmd' table from the stack
  lua_call(lcmd_state, 0, 0);
  return IUP_DEFAULT;
}

int lcmdConsoleListVarCb(Ihandle *ih)
{
  lcmdConsoleEnterCommandStr("cmd.consoleListVar()");
  lua_getfield(lcmd_state, LUA_GLOBALSINDEX, "cmd"); // push 'cmd' table in the stack
  lua_getfield(lcmd_state, -1, "consoleListVar");    // push the function cmd.consoleListVar
  lua_remove(lcmd_state, -2); // remove 'cmd' table from the stack
	lua_call(lcmd_state, 0, 0);
  return IUP_DEFAULT;
}

int lcmdConsoleCmdLineKAnyCb(Ihandle *ih, int c)
{
  char *text;

  switch(c)
  {
    case K_CR: // enter
      text = IupGetAttribute(console.txCmdLine, IUP_VALUE);
      if(text && text[0] != 0)
        lcmdConsoleEnterCommand(text);
    case K_ESC:
      IupSetAttribute(console.txCmdLine, IUP_VALUE, "");
    break;
    case K_UP:
      if(console.command_list->total > 0) // not empty list
      {
        if( console.cur_list_node->previous != NULL
         && !strcmp(console.cur_list_node->value,IupGetAttribute(console.txCmdLine,IUP_VALUE)))
          console.cur_list_node = console.cur_list_node->previous;
        IupStoreAttribute(console.txCmdLine, IUP_VALUE, (char*)console.cur_list_node->value);
      }
    break;
    case K_DOWN:
      if(console.command_list->total > 0) // not empty list
      {
        if( console.cur_list_node->next != NULL
         && !strcmp(console.cur_list_node->value,IupGetAttribute(console.txCmdLine,IUP_VALUE)))
          console.cur_list_node = console.cur_list_node->next;
        IupStoreAttribute(console.txCmdLine, IUP_VALUE, (char*)console.cur_list_node->value);
      }
    break;
  }
  return IUP_CONTINUE;
}

int luaConsoleVersionInfo(lua_State *state)
{
  lcmdConsoleEnterMessagef("daVinci 1.0\n"
   "  (CNPQ project)\n"
   "%s  %s\n"
   "  (written by %s)\n"
   "IUP %s\n"
   "  (copyright (c) 1995-2008 Tecgraf/PUC-Rio - SEPROC/CENPES/Petrobrás)\n\n",
   LUA_VERSION, LUA_COPYRIGHT, LUA_AUTHORS, IupGetGlobal("VERSION")
  );
  IupSetAttribute(IupGetDialogChild(main_dialog, "debugTabs"), IUP_VALUE, "tabOutput");
  IupSetFocus(console.txCmdLine);
  return 0;
}

void lcmdConsoleInitLuaState(lua_State *state)
{
  static const luaL_Reg cmd_consolefuncs[] = {
    {"consoleClearOutput", luaConsoleClearOutput},
    {"consoleEnterMessage", luaConsoleEnterMessage},
    {"consoleEnterCommandStr", luaConsoleEnterCommandStr},
    {"consoleHoldCaret", luaHoldCaret},
    {"consolePrintFuncVar", cmdPrintFuncVar},
    {"consolePrintVar", cmdPrintVar},
    {"consolePrintVarType", cmdPrintVarType},
    {"consolePrintObject", luaConsolePrintObject},
    {"consolePrintObjectValue", luaConsolePrintObjectValue},
    {"consoleVersionInfo", luaConsoleVersionInfo},
    {NULL, NULL}
  };

  // replace "print" by lua_print
  lua_pushcfunction(state, lua_print);
  lua_setglobal(state, "print"); // set global 'print'
  // replace "io.write" by lua_print
  lua_getfield(state, LUA_GLOBALSINDEX, "io");
  if(lua_istable(state, -1))
  {
    lua_pushstring(state, "write");
    lua_pushcfunction(state, lua_print);
    lua_settable(state, -3);
  }
  else lua_pop(state, 1);
  luaL_register(state, "cmd", cmd_consolefuncs); // create the 'cmd' module, and register console functions
  lua_pop(state, 1); // remove 'cmd' table from the stack
  luaL_dostring(state,
   "cmd.consoleListFunc = function()\n"
   "  local global = getfenv(0)\n"
   "  local n,v = next(global, nil)\n"
   "  cmd.consoleHoldCaret(1)\n"
   "  while n ~= nil do\n"
   "    if type(v) == \"function\" then\n"
   "      cmd.consolePrintFuncVar(n, v)\n"
   "    end\n"
   "    n,v = next(global, n)\n"
   "  end\n"
   "  cmd.consoleHoldCaret(0)\n"
   "end\n"
   "cmd.consoleListVar = function()\n"
   "  local global = getfenv(0)\n"
   "  local n,v = next(global, nil)\n"
   "  cmd.consoleHoldCaret(1)\n"
   "  while n ~= nil do\n"
   "    if type(v) ~= \"function\" and n ~= \"_G\" then\n"
   "      cmd.consolePrintVarType(n, v)\n"
   "    end\n"
   "    n,v = next(global, n)\n"
   "  end\n"
   "  cmd.consoleHoldCaret(0)\n"
   "end\n"
   "cmd.consolePrintTable = function(t)\n"
   "  local n,v = next(t, nil)\n"
   "  while n ~= nil do\n"
   "    cmd.consolePrintVar(n, v, 1)\n"
   "    n,v = next(t, n)\n"
   "  end\n"
   "end\n"
  );
}

void lcmdConsoleCreate(lua_State *state)
{
  console.output_size = 32768;
  console.command_list = utlCreateDList();
  console.cur_list_node = NULL;
}

void lcmdConsoleKill(void)
{
  utlKillDList(console.command_list, free);
}
