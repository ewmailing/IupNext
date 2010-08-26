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

#include "iupwin_drv.h"


static int win_monitor_index = 0;
static HANDLE win_singleintance = NULL;
static HWND win_findwindow = NULL;


int iupdrvCheckMainScreen(int *w, int *h)
{
  (void)w;
  (void)h;
  return 0;
}

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
