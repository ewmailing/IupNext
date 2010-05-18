/** \file
 * \brief Windows Driver iupdrvSetGlobal
 *
 * See Copyright Notice in "iup.h"
 */

#include <stdio.h>
#include <string.h>
#include <windows.h>

#include "iup.h"

#include "iup_str.h"
#include "iup_drv.h"
#include "iup_drvinfo.h"
#include "iup_strmessage.h"

#include "iupwin_drv.h"

/* Not defined in compilers older than VC9 */
#ifndef MAPVK_VK_TO_VSC
#define MAPVK_VK_TO_VSC     (0)
#endif


static int win_monitor_index = 0;
static HANDLE win_singleintance = NULL;
static HWND win_findwindow = NULL;


static int winGlobalSetMutex(const char* name)
{
  if (win_singleintance)
    ReleaseMutex(win_singleintance);

  /* try to create a mutex (will fail if already one of that name) */
  win_singleintance = CreateMutex(NULL, FALSE, name);

  /* Return TRUE if existing semaphore opened */
  if (win_singleintance != NULL && GetLastError()==ERROR_ALREADY_EXISTS)
  {
    CloseHandle(win_singleintance);
    return 1;
  }

  /* wasn’t found, new one created therefore return FALSE */
  return (win_singleintance == NULL);
}

static BOOL CALLBACK winGlobalEnumWindowProc(HWND hWnd, LPARAM lParam)
{
  char* name = (char*)lParam;
  char str[256];
  int len = GetWindowText(hWnd, str, 256);
  if (len)
  {
    if (iupStrEqualPartial(str, name))
    {
      win_findwindow = hWnd;
      return FALSE;
    }
  }

  return TRUE;  /* continue searching */
}

static HWND winGlobalFindWindow(const char* name)
{
  win_findwindow = NULL;
  EnumWindows(winGlobalEnumWindowProc, (LPARAM)name);
  return win_findwindow;
}

static void winGlobalFindInstance(const char* name)
{
  HWND hWnd = winGlobalFindWindow(name);
  if (hWnd)
  {
    LPTSTR cmdLine = GetCommandLine();

    SetForegroundWindow(hWnd);

    /* Command line is not empty. Send it to the first instance. */ 
    if (strlen(cmdLine) != 0) 
    {
      COPYDATASTRUCT cds;
      cds.dwData = (ULONG_PTR)"IUP_DATA";
      cds.cbData = strlen(cmdLine)+1;
      cds.lpData = cmdLine;
      SendMessage(hWnd, WM_COPYDATA, 0, (LPARAM)&cds);
    }
  }
}

static void winGlobalSendKey(int key, int press)
{
  unsigned int keyval, state;
  INPUT input[2];
  LPARAM extra_info;
  WORD state_scan = 0, key_scan;
  ZeroMemory(input, 2*sizeof(INPUT));

  iupwinKeyEncode(key, &keyval, &state);
  if (!keyval)
    return;

  extra_info = GetMessageExtraInfo();
  if (state)
    state_scan = (WORD)MapVirtualKey(state, MAPVK_VK_TO_VSC);
  key_scan = (WORD)MapVirtualKey(keyval, MAPVK_VK_TO_VSC);

  if (press & 0x01)
  {
    if (state)
    {
      /* modifier first */
      input[0].type = INPUT_KEYBOARD;
      input[0].ki.wVk = (WORD)state;
      input[0].ki.wScan = state_scan;
      input[0].ki.dwExtraInfo = extra_info;

      /* key second */
      input[1].type = INPUT_KEYBOARD;
      input[1].ki.wVk = (WORD)keyval;
      input[1].ki.wScan = key_scan;
      input[1].ki.dwExtraInfo = extra_info;

      SendInput(2, input, sizeof(INPUT));
    }
    else
    {
      input[0].type = INPUT_KEYBOARD;
      input[0].ki.wVk = (WORD)keyval;
      input[0].ki.wScan = key_scan;
      input[0].ki.dwExtraInfo = extra_info;

      SendInput(1, input, sizeof(INPUT));
    }
  }

  if (press & 0x02)
  {
    if (state)
    {
      /* key first */
      input[0].type = INPUT_KEYBOARD;
      input[0].ki.dwFlags = KEYEVENTF_KEYUP;
      input[0].ki.wVk = (WORD)keyval;
      input[0].ki.wScan = key_scan;
      input[0].ki.dwExtraInfo = extra_info;

      /* modifier second */
      input[1].type = INPUT_KEYBOARD;
      input[1].ki.dwFlags = KEYEVENTF_KEYUP;
      input[1].ki.wVk = (WORD)state;
      input[1].ki.wScan = state_scan;
      input[1].ki.dwExtraInfo = extra_info;

      SendInput(2, input, sizeof(INPUT));
    }
    else
    {
      input[0].type = INPUT_KEYBOARD;
      input[0].ki.dwFlags = KEYEVENTF_KEYUP;
      input[0].ki.wVk = (WORD)keyval;
      input[0].ki.wScan = key_scan;
      input[0].ki.dwExtraInfo = extra_info;

      SendInput(1, input, sizeof(INPUT));
    }
  }
}

static BOOL CALLBACK winGlobalMonitorInfoEnum(HMONITOR handle, HDC handle_dc, LPRECT rect, LPARAM data)
{
  RECT* monitors_rect = (RECT*)data;
  monitors_rect[win_monitor_index] = *rect;
  win_monitor_index++;
  (void)handle_dc;
  (void)handle;
  return TRUE;
}

int iupdrvSetGlobal(const char *name, const char *value)
{
  if (iupStrEqual(name, "LANGUAGE"))
  {
    iupStrMessageUpdateLanguage(value);
    return 1;
  }
  if (iupStrEqual(name, "CURSORPOS"))
  {
    int x, y;
    if (iupStrToIntInt(value, &x, &y, 'x') == 2)
      SetCursorPos(x, y);
    return 0;
  }
  if (iupStrEqual(name, "KEYPRESS"))
  {
    int key;
    if (iupStrToInt(value, &key))
      winGlobalSendKey(key, 0x01);
    return 0;
  }
  if (iupStrEqual(name, "KEYRELEASE"))
  {
    int key;
    if (iupStrToInt(value, &key))
      winGlobalSendKey(key, 0x02);
    return 0;
  }
  if (iupStrEqual(name, "KEY"))
  {
    int key;
    if (iupStrToInt(value, &key))
      winGlobalSendKey(key, 0x03);
    return 0;
  }
  if (iupStrEqual(name, "DLL_HINSTANCE"))
  {
    iupwin_dll_hinstance = (HINSTANCE)value;
    return 0;
  }
  if (iupStrEqual(name, "SINGLEINSTANCE"))
  {
    if (winGlobalSetMutex(value))
    {
      winGlobalFindInstance(value);
      return 0;  /* don't save the attribute, mutex already exist */
    }
    else
      return 1; /* save the attribute, this is the first instance */
  }
  return 1;
}

char *iupdrvGetGlobal(const char *name)
{
  if (iupStrEqual(name, "CURSORPOS"))
  {
    int x, y;
    char* str = iupStrGetMemory(50);
    iupdrvGetCursorPos(&x, &y);
    sprintf(str, "%dx%d", x, y);
    return str;
  }
  if (iupStrEqual(name, "SHIFTKEY"))
  {
    char key[5];
    iupdrvGetKeyState(key);
    if (key[0] == 'S')
      return "ON";
    return "OFF";
  }
  if (iupStrEqual(name, "CONTROLKEY"))
  {
    char key[5];
    iupdrvGetKeyState(key);
    if (key[1] == 'C')
      return "ON";
    return "OFF";
  }
  if (iupStrEqual(name, "MODKEYSTATE"))
  {
    char *str = iupStrGetMemory(5);
    iupdrvGetKeyState(str);
    return str;
  }
  if (iupStrEqual(name, "SCREENSIZE"))
  {
    char *str = iupStrGetMemory(50);
    int w, h;
    iupdrvGetScreenSize(&w, &h);
    sprintf(str, "%dx%d", w, h);
    return str;
  }
  if (iupStrEqual(name, "FULLSIZE"))
  {
    char *str = iupStrGetMemory(50);
    int w, h;
    iupdrvGetFullSize(&w, &h);
    sprintf(str, "%dx%d", w, h);
    return str;
  }
  if (iupStrEqual(name, "SCREENDEPTH"))
  {
    char *str = iupStrGetMemory(50);
    int bpp = iupdrvGetScreenDepth();
    sprintf(str, "%d", bpp);
    return str;
  }
  if (iupStrEqual(name, "VIRTUALSCREEN"))
  {
    char *str = iupStrGetMemory(50);
    int x = GetSystemMetrics(SM_XVIRTUALSCREEN); 
    int y = GetSystemMetrics(SM_YVIRTUALSCREEN);
    int w = GetSystemMetrics(SM_CXVIRTUALSCREEN); 
    int h = GetSystemMetrics(SM_CYVIRTUALSCREEN);
    sprintf(str, "%d %d %d %d", x, y, w, h);
    return str;
  }
  if (iupStrEqual(name, "MONITORSINFO"))
  {
    int i;
    int monitors_count = GetSystemMetrics(SM_CMONITORS);
    RECT* monitors_rect = malloc(monitors_count*sizeof(RECT));
    char *str = iupStrGetMemory(monitors_count*50);
    char* pstr = str;

    win_monitor_index = 0;
    EnumDisplayMonitors(NULL, NULL, winGlobalMonitorInfoEnum, (LPARAM)monitors_rect);

    for (i=0; i < monitors_count; i++)
      pstr += sprintf(pstr, "%d %d %d %d\n", (int)monitors_rect[i].left, (int)monitors_rect[i].top, (int)(monitors_rect[i].right-monitors_rect[i].left), (int)(monitors_rect[i].bottom-monitors_rect[i].top));

    free(monitors_rect);
    return str;
  }
  if (iupStrEqual(name, "TRUECOLORCANVAS"))
  {
    if (iupdrvGetScreenDepth() > 8)
      return "YES";
    return "NO";
  }
  if (iupStrEqual(name, "DLL_HINSTANCE"))
  {
    return (char*)iupwin_dll_hinstance;
  }
  return NULL;
}
