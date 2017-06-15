#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "iup.h"
#include "iup_scintilla.h"
#include "iup_config.h"

#include "iuplua.h"

#include "iup_object.h"
#include "iup_attrib.h"
#include "iup_str.h"
#include "iup_strmessage.h"
#include "iup_register.h"
#include "iup_childtree.h"


void load_all_images_step_images(void);


/********************************** Utilities *****************************************/

static char* getLuaKeywords(void)
{
  /* Common */
  return "and break do else elseif end for function if in local load nil not or repeat return then until while "
  "false true assert collectgarbage dofile error _G getmetatable ipairs loadfile next pairs "
  "pcall print rawequal rawget rawset setmetatable tonumber tostring type _VERSION select"
  "require xpcall "
  "string string.byte string.char string.dump string.find string.format string.gsub string.len string.lower "
  "string.rep string.sub string.upper string.gmatch string.match string.reverse"
  "table table.concat table.insert table.remove table.sort "
  "math math.abs math.acos math.asin math.atan math.ceil math.cos math.deg math.exp math.floor "
  "math.log math.max math.min math.pi math.rad math.random math.randomseed math.fmod "
  "math.sin math.sqrt math.tan math.huge math.modf "
  "coroutine coroutine.create coroutine.resume coroutine.status coroutine.wrap coroutine.yield coroutine.running"
  "io io.close io.flush io.input io.lines io.open io.output io.read io.tmpfile io.type io.write "
  "io.stdin io.stdout io.stderr io.popen"
  "os os.clock os.date os.difftime os.execute os.exit "
  "os.getenv os.remove os.rename os.setlocale os.time os.tmpname"
  "debug debug.getfenv debug.getmetatable debug.getregistry debug.setfenv debug.setmetatable debug.debug debug.gethook "
  "debug.setlocal debug.setupvalue debug.sethook debug.traceback debug.getinfo debug.getlocal debug.getupvalue "
  "package package.seeall package.cpath package.loaded "
  "package.loadlib package.path package.preload"
#if LUA_VERSION_NUM == 501  /* Lua 5.1 Only */
  "module setfenv getfenv math.log10 loadstring table.maxn unpack package.loaders"
  "math.atan2 math.cosh math.sinh math.tanh math.pow math.frexp math.ldexp"
#elif LUA_VERSION_NUM == 502  /* Lua 5.2 Only */
  "package.searchers goto rawlen table.pack table.unpack package.config debug.getuservalue debug.setuservalue debug.upvalueid debug.upvaluejoin _ENV"
  "bit32 bit32.arshift bit32.band bit32.bnot bit32.bor bit32.btest bit32.bxor bit32.extract bit32.replace bit32.lrotate bit32.lshift bit32.rrotate bit32.rshift"
  "package.searchpath"
  "math.atan2 math.cosh math.sinh math.tanh math.pow math.frexp math.ldexp"
#elif LUA_VERSION_NUM == 503  /* Lua 5.3 Only */
  "package.searchers goto rawlen table.pack table.unpack package.config debug.getuservalue debug.setuservalue debug.upvalueid debug.upvaluejoin _ENV"
  "table.move string.pack string.unpack string.packsize"
  "utf8.char utf8.charpattern utf8.codes utf8.codepoint utf8.len utf8.offset"
#endif  
  ;
}


#define IUP_STR_EQUAL(str1, str2)      \
{                                      \
if (str1 == str2)                    \
  return 1;                          \
  \
if (!str1 || !str2)                  \
  return 0;                          \
  \
while (*str1 && *str2 &&              \
  SF(*str1) == SF(*str2))        \
{                                    \
  EXTRAINC(str1);                    \
  EXTRAINC(str2);                    \
  str1++;                            \
  str2++;                            \
}                                    \
  \
  /* check also for terminator */      \
if (*str1 == *str2) return 1;        \
}


static const char *getLastNonAlphaNumeric(const char *text)
{
  int len = (int)strlen(text);
  const char *c = text + len - 1;
  if (*c == '\n')
    c--;
  for (; c != text; c--)
  {
    if (*c < 48 || (*c > 57 && *c < 65) || (*c > 90 && *c < 97) || *c > 122)
      return c + 1;
  }
  return NULL;
}

static const char* strNextValue(const char* str, int str_len, int *len, char sep)
{
  int ignore_sep = 0;

  *len = 0;

  if (!str) return NULL;

  while (*str != 0 && (*str != sep || ignore_sep) && *len < str_len)
  {
    if (*str == '\"')
    {
      if (ignore_sep)
        ignore_sep = 0;
      else
        ignore_sep = 1;
    }

    (*len)++;
    str++;
  }

  if (*str == sep)
    return str + 1;
  else
    return str;  /* no next value */
}

static int strEqualPartial(const char* str1, const char* str2)
{
#define EXTRAINC(_x) (void)(_x)
#define SF(_x) (_x)
  IUP_STR_EQUAL(str1, str2);
#undef SF
#undef EXTRAINC
  if (*str2 == 0)
    return 1;  /* if second string is at terminator, then it is partially equal */
  return 0;
}

static char *filterList(const char *text, const char *list)
{
  char *filteredList[1024];
  char *retList;
  int count = 0;

  int i, len;
  const char *lastValue = list;
  const char *nextValue = strNextValue(list, (int)strlen(list), &len, ' ');
  while (len != 0)
  {
    if ((int)strlen(text) <= len && strEqualPartial(lastValue, text))
    {
      char *value = malloc(80);

      strncpy(value, lastValue, len);
      value[len] = 0;
      filteredList[count++] = value;
    }
    lastValue = nextValue;
    nextValue = strNextValue(nextValue, (int)strlen(nextValue), &len, ' ');
  }

  retList = malloc(1024);
  retList[0] = 0;
  for (i = 0; i < count; i++)
  {
    if (i == 0)
    {
      strcpy(retList, filteredList[i]);
      strcat(retList, " ");
    }
    else
    {
      strcat(retList, filteredList[i]);
      strcat(retList, " ");
    }
  }

  for (i = 0; i < count; i++)
    free(filteredList[i]);

  return retList;
}


/********************************** Callbacks *****************************************/


static int exit_cb(Ihandle *ih)
{
  lua_State* L = (lua_State*)IupGetAttribute(ih, "LUASTATE");
  lua_getglobal(L, "iupDebuggerExit");
  lua_call(L, 0, 0);
  return IUP_DEFAULT;
}

static int debuggerHasBreakpoint(lua_State *L)
{
  Ihandle* multitext = iuplua_checkihandle(L, 1);
  int lin = (int)luaL_checkinteger(L, 2);
  unsigned int markerMask = (unsigned int)IupGetIntId(multitext, "MARKERGET", lin);
  int has_breakpoint = markerMask & 0x0002; /* 0010 - marker=1 */
  lua_pushboolean(L, has_breakpoint);
  return 1;
}

static int debuggerGetBreakpoints(lua_State *L)
{
  Ihandle* multitext = iuplua_checkihandle(L, 1);
  int lin = 0, i = 1;

  lua_newtable(L);

  while (lin >= 0)
  {
    IupSetAttributeId(multitext, "MARKERNEXT", lin, "2");  /* 0010 - marker=1 */
    lin = IupGetInt(multitext, "LASTMARKERFOUND");
    if (lin >= 0)
    {
      lua_pushinteger(L, i);
      lua_pushinteger(L, lin + 1);
      lua_settable(L, -3);

      lin++;
      i++;
    }
  }

  return 1;
}

static int savemarkers_cb(Ihandle *ih)
{
  int lin = 0, i = 1;
  char mark[10240];

  Ihandle* multitext = IupGetDialogChild(ih, "MULTITEXT");
  char* filename = IupGetAttribute(multitext, "FILENAME");
  Ihandle* config = IupGetAttributeHandle(multitext, "CONFIG");

  while (lin >= 0)
  {
    IupSetAttributeId(multitext, "MARKERNEXT", lin, "2");  /* 0010 - marker=1 */
    lin = IupGetInt(multitext, "LASTMARKERFOUND");
    if (lin >= 0)
    {
      sprintf(mark, "%s#%d", filename, lin);
      IupConfigSetVariableStrId(config, "Breakpoints", "FileLine", i, mark);

      lin++;
      i++;
    }
  }

  IupConfigSetVariableStrId(config, "Breakpoints", "FileLine", i, NULL);

  return IUP_DEFAULT;
}

static int restoremarkers_cb(Ihandle *ih)
{
  const char* mark;
  int i = 1;
  char filename_str[10240];
  char line_str[10];

  lua_State* L = (lua_State*)IupGetAttribute(ih, "LUASTATE");
  Ihandle* multitext = IupGetDialogChild(ih, "MULTITEXT");
  char* filename = IupGetAttribute(multitext, "FILENAME");
  Ihandle* config = IupGetAttributeHandle(multitext, "CONFIG");

  IupSetInt(multitext, "MARKERDELETEALL", 1);

  do
  {
    mark = IupConfigGetVariableStrId(config, "Breakpoints", "FileLine", i);
    if (mark)
    {
      iupStrToStrStr(mark, filename_str, line_str, '#');
      if (iupStrEqual(filename, filename_str))
      {
        int lin;
        iupStrToInt(line_str, &lin);
        IupSetIntId(multitext, "MARKERADD", lin, 1);
      }
    }
    i++;
  } while (mark != NULL);

  lua_getglobal(L, "iupDebuggerUpdateBreakpointsList");
  lua_call(L, 0, 0);

  return IUP_DEFAULT;
}

static int marker_changed_cb(Ihandle *ih, int lin, int margin)
{
  if (margin == 2)
  {
    lua_State* L = (lua_State*)IupGetAttribute(ih, "LUASTATE");
    Ihandle* multitext = IupGetDialogChild(ih, "MULTITEXT");
    unsigned int markerMask = (unsigned int)IupGetIntId(multitext, "MARKERGET", lin);
    int has_breakpoint = markerMask & 0x0002; /* 0010 - marker=1 */

    char* filename = IupGetAttribute(multitext, "FILENAME");
    if (!filename)
    {
      IupMessageError(IupGetDialog(ih), "Must have a filename to add a breakpoint.");
      return IUP_DEFAULT;
    }

    if (has_breakpoint)
      IupSetIntId(multitext, "MARKERDELETE", lin, 1);
    else
      IupSetIntId(multitext, "MARKERADD", lin, 1);

    lua_getglobal(L, "iupDebuggerUpdateBreakpointsList");
    lua_call(L, 0, 0);
  }

  (void)ih;
  return IUP_DEFAULT;
}

static int multitext_kesc_cb(Ihandle *ih)
{
  Ihandle* multitext = IupGetDialogChild(ih, "MULTITEXT");

  if (!IupGetInt(multitext, "AUTOCOMPLETION"))
    return IUP_CONTINUE;

  if (IupGetInt(multitext, "AUTOCACTIVE"))
    IupSetAttribute(multitext, "AUTOCCANCEL", "YES");

  return IUP_CONTINUE;
}

static int multitext_valuechanged_cb(Ihandle* multitext)
{
  Icallback old_valuechanged_cb = IupGetCallback(multitext, "OLD_VALUECHANGED_CB");
  old_valuechanged_cb(multitext);

  if (IupGetInt(multitext, "AUTOCOMPLETION"))
  {
    const char *t;
    int pos = IupGetInt(multitext, "CARETPOS");
    char *text = IupGetAttribute(multitext, "VALUE");
    text[pos + 1] = '\0';
    t = getLastNonAlphaNumeric(text);
    if (t != NULL && *t != '\n' && *t != 0)
    {
      char *fList = filterList(t, getLuaKeywords());
      if (strlen(fList) > 0)
        IupSetAttributeId(multitext, "AUTOCSHOW", (int)strlen(t) - 1, fList);
      free(fList);
    }

    return IUP_DEFAULT;
  }

  return IUP_CONTINUE;
}

static int item_autocomplete_action_cb(Ihandle* ih)
{
  Ihandle* multitext = IupGetDialogChild(ih, "MULTITEXT");

  if (IupGetInt(ih, "VALUE"))
  {
    IupSetAttribute(ih, "VALUE", "OFF");
    IupSetAttribute(multitext, "AUTOCOMPLETION", "OFF");
  }
  else
  {
    IupSetAttribute(ih, "VALUE", "ON");
    IupSetAttribute(multitext, "AUTOCOMPLETION", "ON");
  }

  return IUP_DEFAULT;
}

static void debug_set_state(lua_State *L, const char* state)
{
  lua_getglobal(L, "iupDebuggerSetStateString");
  lua_pushstring(L, state);
  lua_call(L, 1, 0);
}

static int save_check(Ihandle* ih_item)
{
  Ihandle* multitext = IupGetDialogChild(ih_item, "MULTITEXT");
  if (IupGetInt(multitext, "DIRTY"))
  {
    if (IupMessageAlarm(IupGetDialog(ih_item), "Attention!", "File must be saved for debugging.\n  Save it now? (No will cancel debug)", "YESNO") == 1)
      IupSetAttribute(IupGetDialog(ih_item), "SAVEFILE", NULL);
    else
      return 0;
  }
  return 1;
}

static int item_debug_action_cb(Ihandle* item)
{
  char* filename;
  Ihandle* multitext;
  lua_State* L;

  if (!IupGetInt(IupGetDialogChild(item, "ITM_DEBUG"), "ACTIVE")) /* can be called by the hot key in the dialog */
    return IUP_DEFAULT;

  if (!save_check(item))
    return IUP_DEFAULT;

  L = (lua_State*)IupGetAttribute(item, "LUASTATE");
  lua_getglobal(L, "iupDebuggerStartDebug");
  lua_call(L, 0, 0);

  multitext = IupGetDialogChild(item, "MULTITEXT");
  filename = IupGetAttribute(multitext, "FILENAME");
  iuplua_dofile(L, filename);

  lua_getglobal(L, "iupDebuggerEndDebug");
  lua_pushboolean(L, 0);
  lua_call(L, 1, 0);
  return IUP_DEFAULT;
}

static int item_run_action_cb(Ihandle *item)
{
  Ihandle* multitext;
  lua_State* L;
  char* filename;

  if (!IupGetInt(IupGetDialogChild(item, "ITM_RUN"), "ACTIVE")) /* can be called by the hot key in the dialog */
    return IUP_DEFAULT;

  L = (lua_State*)IupGetAttribute(item, "LUASTATE");
  multitext = IupGetDialogChild(item, "MULTITEXT");
  filename = IupGetAttribute(multitext, "FILENAME");

  if (filename && !IupGetInt(multitext, "DIRTY"))
    iuplua_dofile(L, filename);
  else
  {
    char* value = IupGetAttribute(multitext, "VALUE");
    char* title = IupGetAttribute(IupGetDialog(multitext), "TITLE");
    iuplua_dostring(L, value, title);
  }
  return IUP_DEFAULT;
}

static int item_stop_action_cb(Ihandle *item)
{
  lua_State* L;

  if (!IupGetInt(IupGetDialogChild(item, "ITM_STOP"), "ACTIVE")) /* can be called by the hot key in the dialog */
    return IUP_DEFAULT;

  L = (lua_State*)IupGetAttribute(item, "LUASTATE");
  debug_set_state(L, "DEBUG_STOPPED");
  return IUP_DEFAULT;
}

static int item_pause_action_cb(Ihandle *item)
{
  lua_State* L;

  if (!IupGetInt(IupGetDialogChild(item, "ITM_PAUSE"), "ACTIVE")) /* can be called by the hot key in the dialog */
    return IUP_DEFAULT;

  L = (lua_State*)IupGetAttribute(item, "LUASTATE");
  debug_set_state(L, "DEBUG_PAUSED");
  return IUP_DEFAULT;
}

static int item_continue_action_cb(Ihandle *item)
{
  lua_State* L;

  if (!IupGetInt(IupGetDialogChild(item, "ITM_CONTINUE"), "ACTIVE")) /* can be called by the hot key in the dialog */
    return IUP_DEFAULT;

  L = (lua_State*)IupGetAttribute(item, "LUASTATE");
  debug_set_state(L, "DEBUG_ACTIVE");
  return IUP_DEFAULT;
}

static int item_stepinto_action_cb(Ihandle *item)
{
  lua_State* L;

  if (!IupGetInt(IupGetDialogChild(item, "ITM_STEPINTO"), "ACTIVE")) /* can be called by the hot key in the dialog */
    return IUP_DEFAULT;

  L = (lua_State*)IupGetAttribute(item, "LUASTATE");
  debug_set_state(L, "DEBUG_STEP_INTO");
  return IUP_DEFAULT;
}

static int item_stepover_action_cb(Ihandle *item)
{
  lua_State* L;

  if (!IupGetInt(IupGetDialogChild(item, "ITM_STEPOVER"), "ACTIVE")) /* can be called by the hot key in the dialog */
    return IUP_DEFAULT;

  L = (lua_State*)IupGetAttribute(item, "LUASTATE");
  debug_set_state(L, "DEBUG_STEP_OVER");
  return IUP_IGNORE; /* avoid system default behavior for F10 key */
}

static int item_stepout_action_cb(Ihandle *item)
{
  lua_State* L;

  if (!IupGetInt(IupGetDialogChild(item, "ITM_STEPOUT"), "ACTIVE")) /* can be called by the hot key in the dialog */
    return IUP_DEFAULT;

  L = (lua_State*)IupGetAttribute(item, "LUASTATE");
  debug_set_state(L, "DEBUG_STEP_OUT");
  return IUP_DEFAULT;
}

static int item_currentline_cb(Ihandle *item)
{
  lua_State* L = (lua_State*)IupGetAttribute(item, "LUASTATE");
  lua_getglobal(L, "iupDebuggerShowCurrentLine");
  lua_call(L, 0, 0);
  return IUP_DEFAULT;
}

static int k_pause_action_cb(Ihandle* item)
{
  char* mod = IupGetGlobal("MODKEYSTATE");

  if (mod[1] == 'C' && mod[2] == 'A')
    item_pause_action_cb(item);

  return IUP_DEFAULT;
}

#ifndef WIN32
/* TODO WORKAROUND: 
   In GTK, IupText seems to work ok for K_CR and K_ESC. 
   But in this configuration, for some unknown reason,
   the K_ANY callback is NOT being called for the IupText, 
   but is being called for the dialog. So we did this workaround.
*/
static int dialog_kany_cb(Ihandle *ih, int c)
{
  lua_State* L = (lua_State*)IupGetAttribute(ih, "LUASTATE");

  switch (c)
  {
  case K_CR:
    lua_getglobal(L, "iupConsoleEnterCommand");
    lua_call(L, 0, 0);
    return IUP_IGNORE;
  case K_ESC:
    IupSetAttribute(IupGetDialogChild(ih, "TXT_CMDLINE"), IUP_VALUE, "");
    return IUP_IGNORE;
  }
  return IUP_DEFAULT;
}
#endif

static int txt_cmdline_kany_cb(Ihandle *ih, int c)
{
  lua_State* L = (lua_State*)IupGetAttribute(ih, "LUASTATE");

  switch (c)
  {
#ifdef WIN32
    case K_CR:
      lua_getglobal(L, "iupConsoleEnterCommand");
      lua_call(L, 0, 0);
      return IUP_IGNORE;
    case K_ESC:
      IupSetAttribute(ih, IUP_VALUE, "");
      return IUP_IGNORE;
#endif
    case K_UP:
      lua_getglobal(L, "iupConsoleKeyUpCommand");
      lua_call(L, 0, 0);
      return IUP_IGNORE;
    case K_DOWN:
      lua_getglobal(L, "iupConsoleKeyDownCommand");
      lua_call(L, 0, 0);
      return IUP_IGNORE;
  }
  return IUP_CONTINUE;
}

static int item_listfuncs_action_cb(Ihandle *ih_item)
{
  Ihandle* ih = (Ihandle*)IupGetAttribute(ih_item, "MAINDIALOG");
  lua_State* L = (lua_State*)IupGetAttribute(ih, "LUASTATE");
  lua_getglobal(L, "iupConsoleListFuncs");
  lua_call(L, 0, 0);
  return IUP_DEFAULT;
}

static int item_listvars_action_cb(Ihandle *ih_item)
{
  Ihandle* ih = (Ihandle*)IupGetAttribute(ih_item, "MAINDIALOG");
  lua_State* L = (lua_State*)IupGetAttribute(ih, "LUASTATE");
  lua_getglobal(L, "iupConsoleListVars");
  lua_call(L, 0, 0);
  return IUP_DEFAULT;
}

static int item_clear_action_cb(Ihandle *ih_item)
{
  Ihandle* ih = (Ihandle*)IupGetAttribute(ih_item, "MAINDIALOG");
  Ihandle* mtlOutput = IupGetDialogChild(ih, "MTL_OUTPUT");
  IupSetAttribute(mtlOutput, "VALUE", "");
  return IUP_DEFAULT;
}

static int btn_tools_action_cb(Ihandle *ih)
{
  int x, y;
  Ihandle* item_listfuncs, *item_listvars, *item_clear, *tools_menu;

  item_listfuncs = IupItem("List Global Functions", NULL);
  IupSetCallback(item_listfuncs, "ACTION", (Icallback)item_listfuncs_action_cb);

  item_listvars = IupItem("List Global Variables", NULL);
  IupSetCallback(item_listvars, "ACTION", (Icallback)item_listvars_action_cb);

  item_clear = IupItem("Clear Output", NULL);
  IupSetCallback(item_clear, "ACTION", (Icallback)item_clear_action_cb);

  tools_menu = IupMenu(item_listfuncs, item_listvars, IupSeparator(), item_clear, NULL);
  IupSetAttribute(tools_menu, "MAINDIALOG", (char*)IupGetDialog(ih));

  x = IupGetInt(ih, "X");
  y = IupGetInt(ih, "Y");
  y += IupGetInt2(ih, "RASTERSIZE");

  IupPopup(tools_menu, x, y);

  IupDestroy(tools_menu);

  return IUP_DEFAULT;
}

static int but_printlocal_cb(Ihandle *ih)
{
  lua_State* L = (lua_State*)IupGetAttribute(ih, "LUASTATE");
  lua_getglobal(L, "iupDebuggerPrintLocalVariable");
  lua_call(L, 0, 0);
  return IUP_DEFAULT;
}

static int but_printalllocals_cb(Ihandle *ih)
{
  lua_State* L = (lua_State*)IupGetAttribute(ih, "LUASTATE");
  lua_getglobal(L, "iupDebuggerPrintAllLocalVariables");
  lua_call(L, 0, 0);
  return IUP_DEFAULT;
}

static int but_setlocal_cb(Ihandle *ih)
{
  lua_State* L = (lua_State*)IupGetAttribute(ih, "LUASTATE");
  lua_getglobal(L, "iupDebuggerSetLocalVariable");
  lua_call(L, 0, 0);
  return IUP_DEFAULT;
}

static int lst_locals_action_cb(Ihandle *ih, char *t, int index, int v)
{
  lua_State* L;
  (void)t;

  if (v == 0)
    return IUP_DEFAULT;

  L = (lua_State*)IupGetAttribute(ih, "LUASTATE");
  lua_getglobal(L, "iupDebuggerLocalVariablesListAction");
  iuplua_pushihandle(L, ih);
  lua_pushinteger(L, index);
  lua_call(L, 2, 0);
  return IUP_DEFAULT;
}

static int lst_stack_action_cb(Ihandle *ih, char *t, int index, int v)
{
  lua_State* L;
  (void)t;

  if (v == 0)
    return IUP_DEFAULT;

  L = (lua_State*)IupGetAttribute(ih, "LUASTATE");
  lua_getglobal(L, "iupDebuggerStackListAction");
  lua_pushinteger(L, index);
  lua_call(L, 1, 0);
  return IUP_DEFAULT;
}

static int but_printlevel_cb(Ihandle *ih)
{
  lua_State* L = (lua_State*)IupGetAttribute(ih, "LUASTATE");
  lua_getglobal(L, "iupDebuggerPrintStackLevel");
  lua_call(L, 0, 0);
  return IUP_DEFAULT;
}

static int but_printstack_cb(Ihandle *ih)
{
  lua_State* L = (lua_State*)IupGetAttribute(ih, "LUASTATE");
  lua_getglobal(L, "iupDebuggerPrintStack");
  lua_call(L, 0, 0);
  return IUP_DEFAULT;
}

static int list_breaks_dblclick_cb(Ihandle *ih, int index, char *t)
{
  lua_State* L = (lua_State*)IupGetAttribute(ih, "LUASTATE");
  lua_getglobal(L, "iupDebuggerBreaksListAction");
  lua_pushinteger(L, index);
  lua_call(L, 1, 0);
  (void)t;
  return IUP_DEFAULT;
}

static int but_togglebreak_cb(Ihandle *ih)
{
  int lin, col;
  Ihandle* multitext = IupGetDialogChild(ih, "MULTITEXT");
  int pos = IupGetInt(multitext, "CARETPOS");
  IupTextConvertPosToLinCol(multitext, pos, &lin, &col);
  IupSetAttributeId(IupGetDialog(ih), "TOGGLEMARKER", lin, "2");  /* margin=2 */
  return IUP_IGNORE;
}

static int but_newbreak_cb(Ihandle* ih)
{
  lua_State* L = (lua_State*)IupGetAttribute(ih, "LUASTATE");
  lua_getglobal(L, "iupDebuggerNewBreakpoint");
  lua_call(L, 0, 0);
  return IUP_DEFAULT;
}

static int but_removebreak_cb(Ihandle *ih)
{
  lua_State* L;
  Ihandle* listBreak = IupGetDialogChild(ih, "LIST_BREAK");
  int value = IupGetInt(listBreak, "VALUE");

  if (value == 0)
  {
    IupMessageError(IupGetDialog(ih), "Must select a breakpoint on the list.");
    return IUP_DEFAULT;
  }

  L = (lua_State*)IupGetAttribute(ih, "LUASTATE");
  lua_getglobal(L, "iupDebuggerRemoveBreakpoint");
  lua_pushinteger(L, value);
  lua_call(L, 1, 0);

  return IUP_DEFAULT;
}

static int but_removeallbreaks_cb(Ihandle *ih)
{
  lua_State* L = (lua_State*)IupGetAttribute(ih, "LUASTATE");
  lua_getglobal(L, "iupDebuggerRemoveAllBreakpoints");
  lua_call(L, 0, 0);
  return IUP_DEFAULT;
}

/********************************** Main *****************************************/

static Ihandle *buildTabConsole(void)
{
  Ihandle *txt_cmdLine, *btn_tools, *console_bts;
  Ihandle *frm_consolebts, *ml_output, *output;

  txt_cmdLine = IupText(NULL);
  IupSetAttribute(txt_cmdLine, "EXPAND", "HORIZONTAL");
  IupSetAttribute(txt_cmdLine, "NAME", "TXT_CMDLINE");
  IupSetCallback(txt_cmdLine, "K_ANY", (Icallback)txt_cmdline_kany_cb);

  btn_tools = IupButton(NULL, NULL);
  IupSetCallback(btn_tools, "ACTION", (Icallback)btn_tools_action_cb);
  IupSetAttribute(btn_tools, "IMAGE", "IUP_ToolsSettings");
  IupSetAttribute(btn_tools, "FLAT", "Yes");
  IupSetAttribute(btn_tools, "TIP", "Console Tools");
  IupSetAttribute(btn_tools, "CANFOCUS", "No");

  console_bts = IupHbox(txt_cmdLine, btn_tools, NULL);
  IupSetAttribute(console_bts, "MARGIN", "5x5");
  IupSetAttribute(console_bts, "GAP", "5");
  IupSetAttribute(console_bts, "ALIGNMENT", "ACENTER");

  frm_consolebts = IupFrame(console_bts);
  IupSetAttribute(frm_consolebts, "TITLE", "Command Line:");

  ml_output = IupMultiLine(NULL);
  IupSetAttribute(ml_output, "NAME", "MTL_OUTPUT");
  IupSetAttribute(ml_output, "EXPAND", "YES");
  IupSetAttribute(ml_output, "RASTERSIZE", "x250");
  IupSetAttribute(ml_output, "READONLY", "YES");
  IupSetAttribute(ml_output, "BGCOLOR", "224 224 2254");

  output = IupVbox(frm_consolebts, ml_output, NULL);
  IupSetAttribute(output, "MARGIN", "5x5");
  IupSetAttribute(output, "GAP", "5");
  IupSetAttribute(output, "TABTITLE", "Console");

  return output;
}

static Ihandle *buildTabLocals(void)
{
  Ihandle *list_local, *button_printLocal, *button_printAllLocals, *button_setLocal, *vbox_local, *frame_local;
  Ihandle *list_stack, *button_printLevel, *button_printStack, *vbox_stack, *frame_stack, *locals;

  list_local = IupList(NULL);
  IupSetAttribute(list_local, "EXPAND", "YES");
  IupSetAttribute(list_local, "NAME", "LIST_LOCAL");
  IupSetAttribute(list_local, "TIP", "List of local variables at selected stack level (ordered by pos)");
  IupSetCallback(list_local, "ACTION", (Icallback)lst_locals_action_cb);

  button_printLocal = IupButton("Print", NULL);
  IupSetAttribute(button_printLocal, "ACTIVE", "NO");
  IupSetAttribute(button_printLocal, "TIP", "Prints debug information about the selected local variable.");
  IupSetAttribute(button_printLocal, "NAME", "PRINT_LOCAL");
  IupSetStrAttribute(button_printLocal, "PADDING", IupGetGlobal("DEFAULTBUTTONPADDING"));
  IupSetCallback(button_printLocal, "ACTION", (Icallback)but_printlocal_cb);

  button_printAllLocals = IupButton("Print All", NULL);
  IupSetAttribute(button_printAllLocals, "ACTIVE", "NO");
  IupSetAttribute(button_printAllLocals, "TIP", "Prints debug information about the all local variables.");
  IupSetAttribute(button_printAllLocals, "NAME", "PRINT_ALLLOCALS");
  IupSetStrAttribute(button_printAllLocals, "PADDING", IupGetGlobal("DEFAULTBUTTONPADDING"));
  IupSetCallback(button_printAllLocals, "ACTION", (Icallback)but_printalllocals_cb);

  button_setLocal = IupButton("Set...", NULL);
  IupSetAttribute(button_setLocal, "ACTIVE", "NO");
  IupSetAttribute(button_setLocal, "NAME", "SET_LOCAL");
  IupSetStrAttribute(button_setLocal, "PADDING", IupGetGlobal("DEFAULTBUTTONPADDING"));
  IupSetCallback(button_setLocal, "ACTION", (Icallback)but_setlocal_cb);

  vbox_local = IupVbox(button_printLocal, button_printAllLocals, button_setLocal, NULL);
  IupSetAttribute(vbox_local, "MARGIN", "0x0");
  IupSetAttribute(vbox_local, "GAP", "4");
  IupSetAttribute(vbox_local, "NORMALIZESIZE", "HORIZONTAL");

  frame_local = IupFrame(IupHbox(list_local, vbox_local, NULL));
  IupSetAttribute(frame_local, "MARGIN", "4x4");
  IupSetAttribute(frame_local, "GAP", "4");
  IupSetAttribute(frame_local, "TITLE", "Locals:");

  list_stack = IupList(NULL);
  IupSetAttribute(list_stack, "EXPAND", "YES");
  IupSetAttribute(list_stack, "NAME", "LIST_STACK");
  IupSetAttribute(list_stack, "TIP", "List of call stack (ordered by level)");
  IupSetCallback(list_stack, "ACTION", (Icallback)lst_stack_action_cb);

  button_printLevel = IupButton("Print", NULL);
  IupSetAttribute(button_printLevel, "TIP", "Prints debug information about the selected call stack level.");
  IupSetAttribute(button_printLevel, "ACTIVE", "NO");
  IupSetAttribute(button_printLevel, "NAME", "PRINT_LEVEL");
  IupSetStrAttribute(button_printLevel, "PADDING", IupGetGlobal("DEFAULTBUTTONPADDING"));
  IupSetCallback(button_printLevel, "ACTION", (Icallback)but_printlevel_cb);

  button_printStack = IupButton("Print All", NULL);
  IupSetAttribute(button_printStack, "TIP", "Prints debug information about all the call stack levels.");
  IupSetAttribute(button_printStack, "ACTIVE", "NO");
  IupSetAttribute(button_printStack, "NAME", "PRINT_STACK");
  IupSetStrAttribute(button_printStack, "PADDING", IupGetGlobal("DEFAULTBUTTONPADDING"));
  IupSetCallback(button_printStack, "ACTION", (Icallback)but_printstack_cb);

  vbox_stack = IupVbox(button_printLevel, button_printStack, NULL);
  IupSetAttribute(vbox_stack, "MARGIN", "0x0");
  IupSetAttribute(vbox_stack, "GAP", "4");
  IupSetAttribute(vbox_stack, "NORMALIZESIZE", "HORIZONTAL");

  frame_stack = IupFrame(IupHbox(list_stack, vbox_stack, NULL));
  IupSetAttribute(frame_stack, "MARGIN", "4x4");
  IupSetAttribute(frame_stack, "GAP", "4");
  IupSetAttribute(frame_stack, "TITLE", "Call Stack:");

  locals = IupHbox(frame_local, frame_stack, NULL);
  IupSetAttribute(locals, "MARGIN", "0x0");
  IupSetAttribute(locals, "GAP", "4");
  IupSetAttribute(locals, "TABTITLE", "Debug");

  return locals;
}

static Ihandle *buildTabBreaks(void)
{
  Ihandle *button_newbreak, *button_removebreak, *button_removeallbreaks, *hbox, *list, *vbox, *frame, *button_togglebreak;

  button_togglebreak = IupButton("Toggle", NULL);
  IupSetAttribute(button_togglebreak, "TIP", "Toggle a breakpoint at current line.");
  IupSetCallback(button_togglebreak, "ACTION", (Icallback)but_togglebreak_cb);
  IupSetStrAttribute(button_togglebreak, "PADDING", IupGetGlobal("DEFAULTBUTTONPADDING"));

  button_newbreak = IupButton("New...", NULL);
  IupSetAttribute(button_newbreak, "TIP", "Adds a new breakpoint at given function and line.");
  IupSetCallback(button_newbreak, "ACTION", (Icallback)but_newbreak_cb);
  IupSetStrAttribute(button_newbreak, "PADDING", IupGetGlobal("DEFAULTBUTTONPADDING"));

  button_removebreak = IupButton("Remove", NULL);
  IupSetAttribute(button_removebreak, "TIP", "Removes the selected breakpoint at the list.");
  IupSetCallback(button_removebreak, "ACTION", (Icallback)but_removebreak_cb);
  IupSetStrAttribute(button_removebreak, "PADDING", IupGetGlobal("DEFAULTBUTTONPADDING"));
  IupSetAttribute(button_removebreak, "NAME", "BTN_REMOVE");
  IupSetAttribute(button_removebreak, "ACTIVE", "NO");

  button_removeallbreaks = IupButton("Remove All", NULL);
  IupSetAttribute(button_removeallbreaks, "TIP", "Removes all breakpoints.");
  IupSetCallback(button_removeallbreaks, "ACTION", (Icallback)but_removeallbreaks_cb);
  IupSetStrAttribute(button_removeallbreaks, "PADDING", IupGetGlobal("DEFAULTBUTTONPADDING"));
  IupSetAttribute(button_removeallbreaks, "NAME", "BTN_REMOVEALL");
  IupSetAttribute(button_removeallbreaks, "ACTIVE", "NO");

  vbox = IupVbox(button_togglebreak, button_newbreak, button_removebreak, button_removeallbreaks, NULL);
  IupSetAttribute(vbox, "MARGIN", "0x0");
  IupSetAttribute(vbox, "GAP", "4");
  IupSetAttribute(vbox, "NORMALIZESIZE", "HORIZONTAL");

  list = IupList(NULL);
  IupSetAttribute(list, "EXPAND", "YES");
  IupSetAttribute(list, "NAME", "LIST_BREAK");
  IupSetCallback(list, "DBLCLICK_CB", (Icallback)list_breaks_dblclick_cb);

  frame = IupFrame(IupHbox(list, vbox, NULL));
  IupSetAttribute(frame, "MARGIN", "4x4");
  IupSetAttribute(frame, "GAP", "4");
  IupSetAttribute(frame, "TITLE", "Breakpoints:");

  hbox = IupVbox(frame, NULL);
  IupSetAttribute(hbox, "MARGIN", "0x0");
  IupSetAttribute(hbox, "GAP", "4");
  IupSetAttribute(hbox, "TABTITLE", "Breaks");

  return hbox;
}

static void appendDebugButtons(Ihandle *dialog)
{
  Ihandle *toolbar, *btn_debug, *btn_run, *btn_stop, *btn_pause, *btn_continue, *btn_currentline;
  Ihandle *zbox_debug_continue, *btn_stepinto, *btn_stepover, *btn_stepout;

  toolbar = IupGetChild(IupGetChild(dialog, 0), 0);

  btn_debug = IupButton(NULL, NULL);
  IupSetAttribute(btn_debug, "NAME", "BTN_DEBUG");
  IupSetAttribute(btn_debug, "IMAGE", "IUP_MediaGoToEnd");
  IupSetAttribute(btn_debug, "FLAT", "Yes");
  IupSetCallback(btn_debug, "ACTION", (Icallback)item_debug_action_cb);
  IupSetAttribute(btn_debug, "TIP", "Debug (F5)");
  IupSetAttribute(btn_debug, "CANFOCUS", "No");

  btn_run = IupButton(NULL, NULL);
  IupSetAttribute(btn_run, "NAME", "BTN_RUN");
  IupSetAttribute(btn_run, "IMAGE", "IUP_MediaPlay");
  IupSetAttribute(btn_run, "FLAT", "Yes");
  IupSetCallback(btn_run, "ACTION", (Icallback)item_run_action_cb);
  IupSetAttribute(btn_run, "TIP", "Run (Ctrl+F5)");
  IupSetAttribute(btn_run, "CANFOCUS", "No");

  btn_stop = IupButton(NULL, NULL);
  IupSetAttribute(btn_stop, "NAME", "BTN_STOP");
  IupSetAttribute(btn_stop, "ACTIVE", "NO");
  IupSetAttribute(btn_stop, "IMAGE", "IUP_MediaStop");
  IupSetAttribute(btn_stop, "FLAT", "Yes");
  IupSetCallback(btn_stop, "ACTION", (Icallback)item_stop_action_cb);
  IupSetAttribute(btn_stop, "TIP", "Stop (Shift+F5)");
  IupSetAttribute(btn_stop, "CANFOCUS", "No");

  btn_pause = IupButton(NULL, NULL);
  IupSetAttribute(btn_pause, "NAME", "BTN_PAUSE");
  IupSetAttribute(btn_pause, "ACTIVE", "NO");
  IupSetAttribute(btn_pause, "IMAGE", "IUP_MediaPause");
  IupSetAttribute(btn_pause, "FLAT", "Yes");
  IupSetCallback(btn_pause, "ACTION", (Icallback)item_pause_action_cb);
  IupSetAttribute(btn_pause, "TIP", "Debug (F5)");
  IupSetAttribute(btn_pause, "CANFOCUS", "No");

  btn_continue = IupButton(NULL, NULL);
  IupSetAttribute(btn_continue, "NAME", "BTN_CONTINUE");
  IupSetAttribute(btn_continue, "ACTIVE", "NO");
  IupSetAttribute(btn_continue, "IMAGE", "IUP_MediaGoToEnd");
  IupSetAttribute(btn_continue, "FLAT", "Yes");
  IupSetCallback(btn_continue, "ACTION", (Icallback)item_continue_action_cb);
  IupSetAttribute(btn_continue, "TIP", "Continue (F5)");
  IupSetAttribute(btn_continue, "CANFOCUS", "No");

  zbox_debug_continue = IupZbox(btn_debug, btn_continue, NULL);
  IupSetAttribute(zbox_debug_continue, "NAME", "ZBOX_DEBUG_CONTINUE");

  btn_stepover = IupButton(NULL, NULL);
  IupSetAttribute(btn_stepover, "NAME", "BTN_STEPOVER");
  IupSetAttribute(btn_stepover, "ACTIVE", "NO");
  IupSetAttribute(btn_stepover, "IMAGE", "IUP_stepover");
  IupSetAttribute(btn_stepover, "FLAT", "Yes");
  IupSetCallback(btn_stepover, "ACTION", (Icallback)item_stepover_action_cb);
  IupSetAttribute(btn_stepover, "TIP", "Executes one step over the execution (F10).");
  IupSetAttribute(btn_stepover, "CANFOCUS", "No");

  btn_stepinto = IupButton(NULL, NULL);
  IupSetAttribute(btn_stepinto, "NAME", "BTN_STEPINTO");
  IupSetAttribute(btn_stepinto, "ACTIVE", "NO");
  IupSetAttribute(btn_stepinto, "IMAGE", "IUP_stepinto");
  IupSetAttribute(btn_stepinto, "FLAT", "Yes");
  IupSetCallback(btn_stepinto, "ACTION", (Icallback)item_stepinto_action_cb);
  IupSetAttribute(btn_stepinto, "TIP", "Executes one step into the execution (F11).");
  IupSetAttribute(btn_stepinto, "CANFOCUS", "No");

  btn_stepout = IupButton(NULL, NULL);
  IupSetAttribute(btn_stepout, "NAME", "BTN_STEPOUT");
  IupSetAttribute(btn_stepout, "ACTIVE", "NO");
  IupSetAttribute(btn_stepout, "IMAGE", "IUP_stepout");
  IupSetAttribute(btn_stepout, "FLAT", "Yes");
  IupSetCallback(btn_stepout, "ACTION", (Icallback)item_stepout_action_cb);
  IupSetAttribute(btn_stepout, "TIP", "Executes one step out of the execution (Shift+F11).");
  IupSetAttribute(btn_stepout, "CANFOCUS", "No");

  btn_currentline = IupButton(NULL, NULL);
  IupSetAttribute(btn_currentline, "NAME", "BTN_CURRENTLINE");
  IupSetAttribute(btn_currentline, "ACTIVE", "NO");
  IupSetAttribute(btn_currentline, "IMAGE", "IUP_ArrowRight");
  IupSetAttribute(btn_currentline, "FLAT", "Yes");
  IupSetCallback(btn_currentline, "ACTION", (Icallback)item_currentline_cb);
  IupSetAttribute(btn_currentline, "TIP", "Shows the debugger current line.");
  IupSetAttribute(btn_currentline, "CANFOCUS", "No");

  IupAppend(toolbar, IupSetAttributes(IupLabel(NULL), "SEPARATOR=VERTICAL"));
  IupAppend(toolbar, btn_run);
  IupAppend(toolbar, zbox_debug_continue);
  IupAppend(toolbar, btn_stop);
  IupAppend(toolbar, btn_pause);
  IupAppend(toolbar, btn_stepover);
  IupAppend(toolbar, btn_stepinto);
  IupAppend(toolbar, btn_stepout);
  IupAppend(toolbar, btn_currentline);
}

static void appendDebugMenuItens(Ihandle *menu)
{
  Ihandle *item_debug, *item_run, *item_stop, *item_pause, *item_continue, *item_stepinto, *item_autocomplete,
          *item_stepover, *item_stepout, *debugMneu, *subMenuDebug, *item_currentline,
          *item_togglebreakpoint, *item_newbreakpoint, *item_removeallbreakpoints;

  item_run = IupItem("&Run\tCtrl+F5", NULL);
  IupSetAttribute(item_run, "NAME", "ITM_RUN");
  IupSetCallback(item_run, "ACTION", (Icallback)item_run_action_cb);
  IupSetAttribute(item_run, "IMAGE", "IUP_MediaPlay");

  item_debug = IupItem("&Debug\tF5", NULL);
  IupSetAttribute(item_debug, "NAME", "ITM_DEBUG");
  IupSetCallback(item_debug, "ACTION", (Icallback)item_debug_action_cb);
  IupSetAttribute(item_debug, "IMAGE", "IUP_MediaGoToEnd");

  item_continue = IupItem("&Continue\tF5", NULL);
  IupSetAttribute(item_continue, "NAME", "ITM_CONTINUE");
  IupSetCallback(item_continue, "ACTION", (Icallback)item_continue_action_cb);
  IupSetAttribute(item_continue, "ACTIVE", "NO");
  IupSetAttribute(item_continue, "IMAGE", "IUP_MediaGoToEnd");

  item_stop = IupItem("&Stop\tShift+F5", NULL);
  IupSetAttribute(item_stop, "NAME", "ITM_STOP");
  IupSetCallback(item_stop, "ACTION", (Icallback)item_stop_action_cb);
  IupSetAttribute(item_stop, "ACTIVE", "NO");
  IupSetAttribute(item_stop, "IMAGE", "IUP_MediaStop");

  item_pause = IupItem("&Pause\tCtrl+Alt+Break", NULL);
  IupSetAttribute(item_pause, "NAME", "ITM_PAUSE");
  IupSetCallback(item_pause, "ACTION", (Icallback)item_pause_action_cb);
  IupSetAttribute(item_pause, "ACTIVE", "NO");
  IupSetAttribute(item_pause, "IMAGE", "IUP_MediaPause");

  item_stepover = IupItem("Step &Over\tF10", NULL);
  IupSetAttribute(item_stepover, "NAME", "ITM_STEPOVER");
  IupSetCallback(item_stepover, "ACTION", (Icallback)item_stepover_action_cb);
  IupSetAttribute(item_stepover, "ACTIVE", "NO");
  IupSetAttribute(item_stepover, "IMAGE", "IUP_stepover");

  item_stepinto = IupItem("Step &Into\tF11", NULL);
  IupSetAttribute(item_stepinto, "NAME", "ITM_STEPINTO");
  IupSetCallback(item_stepinto, "ACTION", (Icallback)item_stepinto_action_cb);
  IupSetAttribute(item_stepinto, "ACTIVE", "NO");
  IupSetAttribute(item_stepinto, "IMAGE", "IUP_stepinto");

  item_stepout = IupItem("Step Ou&t\tShift+F11", NULL);
  IupSetAttribute(item_stepout, "NAME", "ITM_STEPOUT");
  IupSetCallback(item_stepout, "ACTION", (Icallback)item_stepout_action_cb);
  IupSetAttribute(item_stepout, "ACTIVE", "NO");
  IupSetAttribute(item_stepout, "IMAGE", "IUP_stepout");

  item_currentline = IupItem("Show Current Line", NULL);
  IupSetAttribute(item_currentline, "NAME", "ITM_CURRENTLINE");
  IupSetCallback(item_currentline, "ACTION", (Icallback)item_currentline_cb);
  IupSetAttribute(item_currentline, "ACTIVE", "NO");
  IupSetAttribute(item_currentline, "IMAGE", "IUP_ArrowRight");

  item_autocomplete = IupItem("Auto Completion", NULL);
  IupSetCallback(item_autocomplete, "ACTION", (Icallback)item_autocomplete_action_cb);
  IupSetAttribute(item_autocomplete, "VALUE", "ON");

  item_togglebreakpoint = IupItem("Toggle Breakpoint\tF9", NULL);
  IupSetCallback(item_togglebreakpoint, "ACTION", (Icallback)but_togglebreak_cb);

  item_newbreakpoint = IupItem("New Breakpoint...", NULL);
  IupSetCallback(item_newbreakpoint, "ACTION", (Icallback)but_newbreak_cb);

  item_removeallbreakpoints = IupItem("Remove All Breakpoints", NULL);
  IupSetCallback(item_removeallbreakpoints, "ACTION", (Icallback)but_removeallbreaks_cb);

  debugMneu = IupMenu(
    item_run,
    item_debug,
    item_continue,
    item_stop,
    item_pause,
    item_stepover,
    item_stepinto,
    item_stepout,
    item_currentline,
    IupSeparator(),
    item_togglebreakpoint,
    item_newbreakpoint,
    item_removeallbreakpoints,
    IupSeparator(),
    item_autocomplete,
    NULL);

  subMenuDebug = IupSubmenu("&Lua", debugMneu);

  IupAppend(menu, subMenuDebug);
}

static int multitext_map_cb(Ihandle* multitext)
{
  IupSetAttribute(multitext, "LEXERLANGUAGE", "lua");
  IupSetAttribute(multitext, "KEYWORDS0", getLuaKeywords());

  IupSetAttribute(multitext, "STYLEFGCOLOR1", "0 128 0");    /* 1-Lua comment */
  IupSetAttribute(multitext, "STYLEFGCOLOR2", "0 128 0");    /* 2-Lua comment line  */
  IupSetAttribute(multitext, "STYLEFGCOLOR4", "255 128 0");  /* 4-Number  */
  IupSetAttribute(multitext, "STYLEFGCOLOR5", "0 0 255");    /* 5-Keyword  */
  IupSetAttribute(multitext, "STYLEFGCOLOR6", "164 0 164");  /* 6-String  */
  IupSetAttribute(multitext, "STYLEFGCOLOR7", "164 0 164");  /* 7-Character  */
  IupSetAttribute(multitext, "STYLEFGCOLOR10", "164 0 0"); /* 10-Operator  */
  IupSetAttribute(multitext, "STYLEBOLD10", "YES");

  return IUP_DEFAULT;
}

static int iLuaScripterDlgCreateMethod(Ihandle* ih, void** params)
{
  lua_State *L;
  Ihandle *multitext, *menu, *stabs, *box, *statusBar;
  Ihandle *tabConsole, *tabLocals, *tabBreaks, *debugTabs;

  L = (lua_State*)IupGetGlobal("_IUP_LUA_DEFAULT_STATE");

  load_all_images_step_images();

#if LUA_VERSION_NUM < 502
  lua_pushliteral(L, LUA_RELEASE "  " LUA_COPYRIGHT);
#else
  lua_pushliteral(L, LUA_COPYRIGHT);
#endif
  lua_setglobal(L, "_COPYRIGHT");  /* set global _COPYRIGHT */

  IupSetAttribute(ih, "LUASTATE", (char*)L);
  IupSetAttribute(ih, "EXTRAFILTERS", "Lua Files|*.lua|");

  IupSetCallback(ih, "K_F5", (Icallback)item_debug_action_cb);
  IupSetCallback(ih, "K_F5", (Icallback)item_continue_action_cb);
  IupSetCallback(ih, "K_cF5", (Icallback)item_run_action_cb);
  IupSetCallback(ih, "K_sF5", (Icallback)item_stop_action_cb);
  IupSetCallback(ih, "K_PAUSE", (Icallback)k_pause_action_cb);
  IupSetCallback(ih, "K_F10", (Icallback)item_stepover_action_cb);
  IupSetCallback(ih, "K_F11", (Icallback)item_stepinto_action_cb);
  IupSetCallback(ih, "K_sF11", (Icallback)item_stepout_action_cb);
  IupSetCallback(ih, "K_F9", (Icallback)but_togglebreak_cb);
#ifndef WIN32
  IupSetCallback(ih, "K_ANY", (Icallback)dialog_kany_cb);
#endif

  IupSetCallback(ih, "MARKERCHANGED_CB", (Icallback)marker_changed_cb);
  IupSetCallback(ih, "EXIT_CB", (Icallback)exit_cb);
  IupSetCallback(ih, "SAVEMARKERS_CB", (Icallback)savemarkers_cb);
  IupSetCallback(ih, "RESTOREMARKERS_CB", (Icallback)restoremarkers_cb);

  multitext = IupGetDialogChild(ih, "MULTITEXT");
  IupSetCallback(multitext, "OLD_VALUECHANGED_CB", IupGetCallback(multitext, "VALUECHANGED_CB"));
  IupSetCallback(multitext, "VALUECHANGED_CB", (Icallback)multitext_valuechanged_cb);
  IupSetCallback(multitext, "K_ESC", (Icallback)multitext_kesc_cb);
  IupSetCallback(multitext, "MAP_CB", (Icallback)multitext_map_cb);
  IupSetAttribute(multitext, "AUTOCOMPLETION", "ON");

  /* breakpoints margin=2 */
  IupSetInt(multitext, "MARGINWIDTH2", 15);
  IupSetAttribute(multitext, "MARGINTYPE2", "SYMBOL");
  IupSetAttribute(multitext, "MARGINSENSITIVE2", "YES");
  IupSetAttribute(multitext, "MARGINMASKFOLDERS2", "NO");  /* (disable folding) */
  IupSetAttributeId(multitext, "MARGINMASK", 2, "14");  /* 1110 - marker=1 and marker=2 ad marker=3 */

  /* breakpoints marker=1 */
  IupSetAttributeId(multitext, "MARKERFGCOLOR", 1, "255 0 0");
  IupSetAttributeId(multitext, "MARKERBGCOLOR", 1, "255 0 0");
  IupSetAttributeId(multitext, "MARKERSYMBOL", 1, "CIRCLE");

  /* current line marker=2 (not shown in a margin, but uses margin=2 mask) */
  IupSetAttributeId(multitext, "MARKERBGCOLOR", 2, "0 255 0");
  IupSetAttributeId(multitext, "MARKERALPHA", 2, "80");
  IupSetAttributeId(multitext, "MARKERSYMBOL", 2, "BACKGROUND");

  /* current line marker=3 */
  IupSetAttributeId(multitext, "MARKERFGCOLOR", 3, "0 0 0");
  IupSetAttributeId(multitext, "MARKERBGCOLOR", 3, "0 255 0");
  IupSetAttributeId(multitext, "MARKERALPHA", 3, "80");
  IupSetAttributeId(multitext, "MARKERSYMBOL", 3, "SHORTARROW");

#ifdef WIN32
  IupSetAttribute(multitext, "FONT", "Consolas, 11");
#else
  IupSetAttribute(multitext, "FONT", "Monospace, 12");
  /* Other alternatives for "Consolas" in Linux: "DejaVu Sans Mono", "Liberation Mono", "Nimbus Mono L", "FreeMono" */
#endif

  appendDebugButtons(ih);

  menu = IupGetAttributeHandle(ih, "MENU");
  appendDebugMenuItens(menu);

  tabConsole = buildTabConsole();

  tabLocals = buildTabLocals();

  tabBreaks = buildTabBreaks();

  debugTabs = IupTabs(tabConsole, tabLocals, tabBreaks, NULL);
  IupSetAttribute(debugTabs, "EXPAND", "YES");
  IupSetAttribute(debugTabs, "MARGIN", "0x0");
  IupSetAttribute(debugTabs, "GAP", "4");
  IupSetAttribute(debugTabs, "TABTYPE", "BOTTOM");
  IupSetAttribute(debugTabs, "NAME", "DEBUG_TABS");

  stabs = IupSbox(debugTabs);
  IupSetAttribute(stabs, "EXPAND", "YES");
  IupSetAttribute(stabs, "DIRECTION", "NORTH");
  IupSetAttribute(stabs, "MARGIN", "0x0");
  IupSetAttribute(stabs, "GAP", "4");

  box = IupGetChild(ih, 0);
  statusBar = IupGetDialogChild(ih, "STATUSBAR");
  IupInsert(box, statusBar, stabs);

#ifdef IUPLUA_USELOH
#include "debugger.loh"
#include "console.loh"
#else
#ifdef IUPLUA_USELH
#include "debugger.lh"
#include "console.lh"
#else
  iuplua_dofile(L, "console.lua");
  iuplua_dofile(L, "debugger.lua");
#endif
#endif

  lua_pushcfunction(L, debuggerHasBreakpoint);
  lua_setglobal(L, "iupDebuggerHasBreakpoint");
  lua_pushcfunction(L, debuggerGetBreakpoints);
  lua_setglobal(L, "iupDebuggerGetBreakpoints");

  lua_getglobal(L, "iupConsoleInit");
  iuplua_pushihandle(L, IupGetDialogChild(ih, "TXT_CMDLINE"));
  iuplua_pushihandle(L, IupGetDialogChild(ih, "MTL_OUTPUT"));
  lua_call(L, 2, 0);

  lua_getglobal(L, "iupDebuggerInit");
  iuplua_pushihandle(L, ih);
  lua_call(L, 1, 0);

  (void)params;
  return IUP_NOERROR;
}

static Iclass* iupLuaScripterDlgNewClass(void)
{
  Iclass* ic = iupClassNew(iupRegisterFindClass("scintilladlg"));

  ic->New = iupLuaScripterDlgNewClass;
  ic->Create = iLuaScripterDlgCreateMethod;

  ic->name = "luascripterdlg";
  ic->nativetype = IUP_TYPEDIALOG;
  ic->is_interactive = 1;
  ic->childtype = IUP_CHILDNONE;
  ic->has_attrib_id = 1;   /* has attributes with IDs that must be parsed */

  iupClassRegisterAttribute(ic, "SUBTITLE", NULL, NULL, IUPAF_SAMEASSYSTEM, "Scripter", IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);

  return ic;
}

Ihandle* IupLuaScripterDlg(void)
{
  return IupCreate("luascripterdlg");
}

void IupLuaScripterDlgOpen(void)
{
  if (!IupGetGlobal("_IUP_LUASCRIPTERDLG_OPEN"))
  {
    IupScintillaOpen();
    IupImageLibOpen();

    iupRegisterClass(iupLuaScripterDlgNewClass());
    IupSetGlobal("_IUP_LUASCRIPTERDLG_OPEN", "1");
  }
}

/* TODO:
*** multiple files (IupFlatTabs) - option for save in config
** Watch for globals - save in config
* detachable Console, Debug, Breakpoints - save in config
- dialog for style colors - save in config
- Debug Strings
- multi-language (portuguese, spanish)
- replace IupSbox by IupSplit ?
*/
