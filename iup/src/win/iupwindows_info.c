/** \file
 * \brief Windows System Information
 *
 * See Copyright Notice in "iup.h"
 */

#include <stdio.h>
#include <stdlib.h> 

/* This module should depend only on IUP core headers 
   and Windows system headers. */

#include <windows.h>
#include <ShlObj.h> /* for SHGetFolderPath */

#include "iup_export.h"
#include "iup_str.h"
#include "iup_drvinfo.h"
#include "iup_varg.h"


#ifdef _MSC_VER
/* warning C4996: 'GetVersionExW': was declared deprecated */
#pragma warning( disable : 4996 )
#endif

IUP_SDK_API char* iupdrvLocaleInfo(void)
{
  CPINFOEXA info;
  GetCPInfoExA(CP_ACP, 0, &info);
  return iupStrReturnStr(info.CodePageName);
}

/* TODO: Since Windows 8.1/Visual Studio 2013 GetVersionEx is deprecated. 
         We can replace it using GetProductInfo. But for now leave it. */

IUP_SDK_API char *iupdrvGetSystemName(void)
{
  OSVERSIONINFOA osvi;
  osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOA);
  GetVersionExA(&osvi);

  if (osvi.dwPlatformId == VER_PLATFORM_WIN32_NT)
  {
    if (osvi.dwMajorVersion <= 4)
      return "WinNT";

    if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0)
      return "Win2K";

    if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion > 0)
      return "WinXP";

    if (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 0)
      return "Vista";

    if (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 1)
      return "Win7";

    if (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 2)
      return "Win8";

    /* IMPORTANT: starting here will work only if the Manifest has been changed 
       to include Windows 8+ support. Otherwise GetVersionEx will report 6.2 */

    if (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 3)
      return "Win81";

    if (osvi.dwMajorVersion == 10 && osvi.dwMinorVersion == 0)
      return "Win10";
  }

  return "Windows";
}

IUP_SDK_API char *iupdrvGetSystemVersion(void)
{
  char *str = iupStrGetMemory(256);
  OSVERSIONINFOEXA osvi;
  SYSTEM_INFO si;

  ZeroMemory(&si, sizeof(SYSTEM_INFO));
  GetSystemInfo(&si);

  ZeroMemory(&osvi, sizeof(OSVERSIONINFOEXA));
  osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXA);
  GetVersionExA((OSVERSIONINFOA*)&osvi);

  sprintf(str, "%d.%d.%d", (int)osvi.dwMajorVersion, (int)osvi.dwMinorVersion, (int)osvi.dwBuildNumber);

  /* Display service pack (if any). */
  if (osvi.szCSDVersion[0] != 0)
  {
    strcat(str, " ");
    strcat(str, osvi.szCSDVersion);
  }

  if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64)
    strcat(str, " (IA64)");
  else if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64)
    strcat(str, " (x64)");
  else
    strcat(str, " (x86)");

  return str;
}

/*
Windows 7 and 10
PreferencePath(0)=C:\Users\Tecgraf\
PreferencePath(1)=C:\Users\Tecgraf\AppData\Roaming\

Windows XP
PreferencePath(0)=C:\Documents and Settings\Tecgraf\
PreferencePath(1)=C:\Documents and Settings\Tecgraf\Application Data\
*/

IUP_SDK_API int iupdrvGetPreferencePath(char *filename, int use_system)
{
  char* homedrive;
  char* homepath;

  if (use_system)
  {
    if (SHGetFolderPathA(NULL, CSIDL_APPDATA | CSIDL_FLAG_CREATE, NULL, SHGFP_TYPE_CURRENT, filename) == S_OK)
    {
      strcat(filename, "\\");
      return 1;
    }
  }

  homedrive = getenv("HOMEDRIVE");
  homepath = getenv("HOMEPATH");
  if (homedrive && homepath)
  {
    strcpy(filename, homedrive);
    strcat(filename, homepath);
    strcat(filename, "\\");
    return 1;
  }

  filename[0] = '\0';
  return 0;

}

IUP_SDK_API int iupdrvSetCurrentDirectory(const char* path)
{
  return SetCurrentDirectoryA(path);
}

IUP_SDK_API char* iupdrvGetCurrentDirectory(void)
{
  char* cur_dir = NULL;

  int len = GetCurrentDirectoryA(0, NULL);
  if (len == 0) return NULL;

  cur_dir = iupStrGetMemory(len + 2);
  GetCurrentDirectoryA(len + 1, cur_dir);
  cur_dir[len] = '\\';
  cur_dir[len + 1] = 0;

  return cur_dir;
}

IUP_API void IupLogV(const char* type, const char* format, va_list arglist)
{
  HANDLE EventSource;
  WORD wtype = 0;

  int size;
  char* value = iupStrGetLargeMem(&size);
  vsnprintf(value, size, format, arglist);

  if (iupStrEqualNoCase(type, "DEBUG"))
  {
    OutputDebugStringA(value);
    return;
  }
  else if (iupStrEqualNoCase(type, "ERROR"))
    wtype = EVENTLOG_ERROR_TYPE;
  else if (iupStrEqualNoCase(type, "WARNING"))
    wtype = EVENTLOG_WARNING_TYPE;
  else if (iupStrEqualNoCase(type, "INFO"))
    wtype = EVENTLOG_INFORMATION_TYPE;

  EventSource = RegisterEventSourceA(NULL, "Application");
  if (EventSource)
  {
    ReportEventA(EventSource, wtype, 0, 0, NULL, 1, 0, &value, NULL);
    DeregisterEventSource(EventSource);
  }
}

IUP_API void IupLog(const char* type, const char* format, ...)
{
  va_list arglist;
  va_start(arglist, format);
  IupLogV(type, format, arglist);
  va_end(arglist);
}
