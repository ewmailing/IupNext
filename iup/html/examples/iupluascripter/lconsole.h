/*
 * daVinci - Lua Console
 *
 * Copyright (C) 1999-2008, Antonio E. Scuri, Mauro Charão, Tecgraf/PUC-Rio.
 */
#ifndef __LCONSOLE_H__
#define __LCONSOLE_H__
#include "utl_dlist.h"
#include <iup.h>
#include <lua.h>

#if	defined(__cplusplus)
extern "C" {
#endif

typedef struct
{
  Ihandle *txCmdLine, *mlOutput;
  Ihandle *btConsoleClear, *btConsoleBufSize, *btConsoleListFunc, *btConsoleListVar;
  utlDList *command_list;      // command history list
  utlDListNode *cur_list_node; // contains the last command entered at the bottom of the list
  int ind_level,               // table identation level
      output_size,             // size of the output buffer
      hold_caret;              // flag that disables caret update, used for long prints
} console_st;

extern console_st console;

void lcmdConsoleInitLuaState(lua_State *state);
void lcmdConsoleCreate(lua_State *state);
void lcmdConsoleKill(void);

int lcmdConsoleOutputSize(int size);
void lcmdConsoleClear();

void lcmdConsoleEnterMessage(char *text, int insert_new_line);
void lcmdConsoleEnterMessagef(char *format, ...);
void lcmdConsoleEnterCommandStr(char *text);
void lcmdConsoleEnterCommandStrf(char *format, ...);
void lcmdConsoleEnterCommand(char *command);
void lcmdConsolePrintObject(lua_State *state, char *name, int idx);
void lcmdConsoleHoldCaret(int hold);
int lcmdConsoleErrorMsgFunc(lua_State *state);
int lcmdConsoleCmdLineKAnyCb(Ihandle *ih, int c);
int lcmdConsoleListFuncCb(Ihandle *ih);
int lcmdConsoleListVarCb(Ihandle *ih);
int lcmdConsoleClearOutputCb(Ihandle *ih);
int lcmdConsoleOutputSizeDlgCb(Ihandle *ih);

#if defined(__cplusplus)
}
#endif

#endif
