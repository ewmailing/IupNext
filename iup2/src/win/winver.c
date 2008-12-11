/** \file
 * \brief Windows Driver version functions
 *
 * See Copyright Notice in iup.h
 * $Id: winver.c,v 1.2 2008-12-11 19:02:57 scuri Exp $
 */

#include <stdio.h>
#include <stdlib.h> 
#include <assert.h>

#include <windows.h>
#include <lmcons.h> /* Defines UNLEN */

#include "iglobal.h"
#include "win.h"
#include "winhandle.h"

static int ok_cb(void)
{
   return IUP_CLOSE;
}

void iupwinVersion(void)
{
   Ihandle* dial, *ok;

   dial = IupDialog(IupVbox(IupFrame(IupVbox(
                        IupLabel(IupVersion()),
                        IupLabel(IUP_VERSION_DATE),
                        IupLabel(IUP_COPYRIGHT),
                        NULL)), 
                      ok = IupButton("Ok", NULL),
                      NULL));

   IupSetCallback(ok, "ACTION", (Icallback)ok_cb);

   IupSetAttribute(dial,IUP_TITLE,"IUP");
   IupSetAttribute(dial,IUP_MENUBOX,IUP_NO);
   IupSetAttribute(dial,IUP_MINBOX,IUP_NO);
   IupSetAttribute(dial,IUP_MAXBOX,IUP_NO);
   IupSetAttribute(dial,IUP_RESIZE,IUP_NO);

   IupSetAttribute(dial,"GAP","5");
   IupSetAttribute(dial,"MARGIN","5");

   IupPopup(dial, IUP_CENTER, IUP_CENTER);
   IupDestroy(dial);
}

int iupwinGetSystemMajorVersion(void)
{
  return GetVersion() & 0x000000ff;
}

/* The max size of szCSDVersion is 128. Adding all others won't pass 200 chars.*/
#define IUP_MAX_SYSTEM_NAME 200
char *iupwinGetSystemName(void)
{
  OSVERSIONINFO osvi;
  osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
  GetVersionEx(&osvi);

  if (osvi.dwPlatformId == VER_PLATFORM_WIN32_NT)
  {
    if (osvi.dwMajorVersion <= 4)
      return "WinNT";

    if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0)
      return "Win2K";

    if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion > 0)
      return "WinXP";

    if (osvi.dwMajorVersion == 6)
      return "Vista";
  }

  return "Windows";
}

char *iupwinGetSystemVersion(void)
{
  char *str;

#ifndef OSVERSIONINFOEX
  OSVERSIONINFO osvi;
  osvi.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
  if (! GetVersionEx ( (OSVERSIONINFO *) &osvi) ) 
    return FALSE;
#else
  OSVERSIONINFOEX osvi;
  BOOL bOsVersionInfoEx;

  ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
  osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

  if( !(bOsVersionInfoEx = GetVersionEx ((OSVERSIONINFO *) &osvi)) )
  {
    /* If OSVERSIONINFOEX doesn't work, try OSVERSIONINFO. */
    osvi.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
    if (! GetVersionEx ( (OSVERSIONINFO *) &osvi) ) 
      return FALSE;
  }
#endif

  str = iupStrGetMemory(sizeof(char)*(IUP_MAX_SYSTEM_NAME+1)); 
  memset(str, 0, IUP_MAX_SYSTEM_NAME);

  sprintf(str, "%d.%d.%d", osvi.dwMajorVersion, osvi.dwMinorVersion, osvi.dwBuildNumber);

  /* Display service pack (if any). */
  if(osvi.szCSDVersion != NULL && !iupStrEqual(osvi.szCSDVersion, ""))
  {
    strcat(str, " ");
    strcat(str, osvi.szCSDVersion);
  }

#ifdef OSVERSIONINFOEX
  if (osvi.dwPlatformId == VER_PLATFORM_WIN32_NT && osvi.dwOSVersionInfoSize == sizeof (OSVERSIONINFOEX))
  {
    OSVERSIONINFOEX* osviex = (OSVERSIONINFOEX*)&osvi;

    if ( osviex->wProductType == VER_NT_WORKSTATION )
    {
      if( osviex->wSuiteMask & VER_SUITE_PERSONAL )
        strcat(str, " Home Edition");
      else
        strcat(str, " Professional" );
    }
    else
      strcat(str, " Server" );
  }
#endif

  return str;
}

/*
 * Retorna uma string contendo a largura da
 * area de trabalho do windows (exclui taskbar)
 * no formato "widthxheigth"
 */

char *iupwinGetScreenSize(void)
{
  RECT area;
  int result = FALSE;
  static char screensize[20];
  
  result = SystemParametersInfo(SPI_GETWORKAREA, 0, &area, 0);
  
  assert(result != FALSE);
  if(result == FALSE)
    return NULL;

  sprintf(screensize, "%dx%d", 
    (int)(area.right - area.left), 
    (int)(area.bottom - area.top));

  return screensize;
}

char* iupwinGetScreenDepth(void)
{
  int bpp;
  static char str[12];
  HDC hDCDisplay = GetDC(NULL);
  bpp = GetDeviceCaps(hDCDisplay, BITSPIXEL);
  ReleaseDC(NULL, hDCDisplay);
  sprintf(str, "%d", bpp);
  return str;
}

char *iupwinGetSystemLanguage(void)
{
#ifdef GetSystemDefaultUILanguage
  LANGID id = GetSystemDefaultUILanguage();
  char *lang = NULL;
  switch(id)
  {
    case 0x0000: lang = "Language Neutral"; break;
    case 0x007f: lang = "The language for the invariant locale (LOCALE_INVARIANT). See MAKELCID."; break;
    case 0x0400: lang = "Process or User Default Language"; break;
    case 0x0800: lang = "System Default Language"; break;
    case 0x0436: lang = "Afrikaans"; break;
    case 0x041c: lang = "Albanian"; break;
    case 0x0401: lang = "Arabic (Saudi Arabia)"; break;
    case 0x0801: lang = "Arabic (Iraq)"; break;
    case 0x0c01: lang = "Arabic (Egypt)"; break;
    case 0x1001: lang = "Arabic (Libya)"; break;
    case 0x1401: lang = "Arabic (Algeria)"; break;
    case 0x1801: lang = "Arabic (Morocco)"; break;
    case 0x1c01: lang = "Arabic (Tunisia)"; break;
    case 0x2001: lang = "Arabic (Oman)"; break;
    case 0x2401: lang = "Arabic (Yemen)"; break;
    case 0x2801: lang = "Arabic (Syria)"; break;
    case 0x2c01: lang = "Arabic (Jordan)"; break;
    case 0x3001: lang = "Arabic (Lebanon)"; break;
    case 0x3401: lang = "Arabic (Kuwait)"; break;
    case 0x3801: lang = "Arabic (U.A.E.)"; break;
    case 0x3c01: lang = "Arabic (Bahrain)"; break;
    case 0x4001: lang = "Arabic (Qatar)"; break;
    case 0x042b: lang = "Windows 2000/XP: Armenian. This is Unicode only."; break;
    case 0x042c: lang = "Azeri (Latin)"; break;
    case 0x082c: lang = "Azeri (Cyrillic)"; break;
    case 0x042d: lang = "Basque"; break;
    case 0x0423: lang = "Belarusian"; break;
    case 0x0402: lang = "Bulgarian"; break;
    case 0x0455: lang = "Burmese"; break;
    case 0x0403: lang = "Catalan"; break;
    case 0x0404: lang = "Chinese (Taiwan)"; break;
    case 0x0804: lang = "Chinese (PRC)"; break;
    case 0x0c04: lang = "Chinese (Hong Kong SAR, PRC)"; break;
    case 0x1004: lang = "Chinese (Singapore)"; break;
    case 0x1404: lang = "Windows 98/Me, Windows 2000/XP: Chinese (Macau SAR)"; break;
    case 0x041a: lang = "Croatian"; break;
    case 0x0405: lang = "Czech"; break;
    case 0x0406: lang = "Danish"; break;
    case 0x0465: lang = "Windows XP: Divehi. This is Unicode only."; break;
    case 0x0413: lang = "Dutch (Netherlands)"; break;
    case 0x0813: lang = "Dutch (Belgium)"; break;
    case 0x0409: lang = "English (United States)"; break;
    case 0x0809: lang = "English (United Kingdom)"; break;
    case 0x0c09: lang = "English (Australian)"; break;
    case 0x1009: lang = "English (Canadian)"; break;
    case 0x1409: lang = "English (New Zealand)"; break;
    case 0x1809: lang = "English (Ireland)"; break;
    case 0x1c09: lang = "English (South Africa)"; break;
    case 0x2009: lang = "English (Jamaica)"; break;
    case 0x2409: lang = "English (Caribbean)"; break;
    case 0x2809: lang = "English (Belize)"; break;
    case 0x2c09: lang = "English (Trinidad)"; break;
    case 0x3009: lang = "Windows 98/Me, Windows 2000/XP: English (Zimbabwe)"; break;
    case 0x3409: lang = "Windows 98/Me, Windows 2000/XP: English (Philippines)"; break;
    case 0x0425: lang = "Estonian"; break;
    case 0x0438: lang = "Faeroese"; break;
    case 0x0429: lang = "Farsi"; break;
    case 0x040b: lang = "Finnish"; break;
    case 0x040c: lang = "French (Standard)"; break;
    case 0x080c: lang = "French (Belgian)"; break;
    case 0x0c0c: lang = "French (Canadian)"; break;
    case 0x100c: lang = "French (Switzerland)"; break;
    case 0x140c: lang = "French (Luxembourg)"; break;
    case 0x180c: lang = "Windows 98/Me, Windows 2000/XP: French (Monaco)"; break;
    case 0x0456: lang = "Windows XP: Galician"; break;
    case 0x0437: lang = "Windows 2000/XP: Georgian. This is Unicode only."; break;
    case 0x0407: lang = "German (Standard)"; break;
    case 0x0807: lang = "German (Switzerland)"; break;
    case 0x0c07: lang = "German (Austria)"; break;
    case 0x1007: lang = "German (Luxembourg)"; break;
    case 0x1407: lang = "German (Liechtenstein)"; break;
    case 0x0408: lang = "Greek"; break;
    case 0x0447: lang = "Windows XP: Gujarati. This is Unicode only."; break;
    case 0x040d: lang = "Hebrew"; break;
    case 0x0439: lang = "Windows 2000/XP: Hindi. This is Unicode only."; break;
    case 0x040e: lang = "Hungarian"; break;
    case 0x040f: lang = "Icelandic"; break;
    case 0x0421: lang = "Indonesian"; break;
    case 0x0410: lang = "Italian (Standard)"; break;
    case 0x0810: lang = "Italian (Switzerland)"; break;
    case 0x0411: lang = "Japanese"; break;
    case 0x044b: lang = "Windows XP: Kannada. This is Unicode only."; break;
    case 0x0457: lang = "Windows 2000/XP: Konkani. This is Unicode only."; break;
    case 0x0412: lang = "Korean"; break;
    case 0x0812: lang = "Windows 95, Windows NT 4.0 only: Korean (Johab)"; break;
    case 0x0440: lang = "Windows XP: Kyrgyz."; break;
    case 0x0426: lang = "Latvian"; break;
    case 0x0427: lang = "Lithuanian"; break;
    case 0x0827: lang = "Windows 98 only: Lithuanian (Classic)"; break;
    case 0x042f: lang = "FYRO Macedonian"; break;
    case 0x043e: lang = "Malay (Malaysian)"; break;
    case 0x083e: lang = "Malay (Brunei Darussalam)"; break;
    case 0x044e: lang = "Windows 2000/XP: Marathi. This is Unicode only."; break;
    case 0x0450: lang = "Windows XP: Mongolian"; break;
    case 0x0414: lang = "Norwegian (Bokmal)"; break;
    case 0x0814: lang = "Norwegian (Nynorsk)"; break;
    case 0x0415: lang = "Polish"; break;
    case 0x0416: lang = "Portuguese (Brazil)"; break;
    case 0x0816: lang = "Portuguese (Portugal)"; break;
    case 0x0446: lang = "Windows XP: Punjabi. This is Unicode only."; break;
    case 0x0418: lang = "Romanian"; break;
    case 0x0419: lang = "Russian"; break;
    case 0x044f: lang = "Windows 2000/XP: Sanskrit. This is Unicode only."; break;
    case 0x0c1a: lang = "Serbian (Cyrillic)"; break;
    case 0x081a: lang = "Serbian (Latin)"; break;
    case 0x041b: lang = "Slovak"; break;
    case 0x0424: lang = "Slovenian"; break;
    case 0x040a: lang = "Spanish (Spain, Traditional Sort)"; break;
    case 0x080a: lang = "Spanish (Mexican)"; break;
    case 0x0c0a: lang = "Spanish (Spain, International Sort)"; break;
    case 0x100a: lang = "Spanish (Guatemala)"; break;
    case 0x140a: lang = "Spanish (Costa Rica)"; break;
    case 0x180a: lang = "Spanish (Panama)"; break;
    case 0x1c0a: lang = "Spanish (Dominican Republic)"; break;
    case 0x200a: lang = "Spanish (Venezuela)"; break;
    case 0x240a: lang = "Spanish (Colombia)"; break;
    case 0x280a: lang = "Spanish (Peru)"; break;
    case 0x2c0a: lang = "Spanish (Argentina)"; break;
    case 0x300a: lang = "Spanish (Ecuador)"; break;
    case 0x340a: lang = "Spanish (Chile)"; break;
    case 0x380a: lang = "Spanish (Uruguay)"; break;
    case 0x3c0a: lang = "Spanish (Paraguay)"; break;
    case 0x400a: lang = "Spanish (Bolivia)"; break;
    case 0x440a: lang = "Spanish (El Salvador)"; break;
    case 0x480a: lang = "Spanish (Honduras)"; break;
    case 0x4c0a: lang = "Spanish (Nicaragua)"; break;
    case 0x500a: lang = "Spanish (Puerto Rico)"; break;
    case 0x0430: lang = "Sutu"; break;
    case 0x0441: lang = "Swahili (Kenya)"; break;
    case 0x041d: lang = "Swedish"; break;
    case 0x081d: lang = "Swedish (Finland)"; break;
    case 0x045a: lang = "Windows XP: Syriac. This is Unicode only."; break;
    case 0x0449: lang = "Windows 2000/XP: Tamil. This is Unicode only."; break;
    case 0x0444: lang = "Tatar (Tatarstan)"; break;
    case 0x044a: lang = "Windows XP: Telugu. This is Unicode only."; break;
    case 0x041e: lang = "Thai"; break;
    case 0x041f: lang = "Turkish"; break;
    case 0x0422: lang = "Ukrainian"; break;
    case 0x0420: lang = "Windows 98/Me, Windows 2000/XP: Urdu (Pakistan)"; break;
    case 0x0820: lang = "Urdu (India)"; break;
    case 0x0443: lang = "Uzbek (Latin)"; break;
    case 0x0843: lang = "Uzbek (Cyrillic)"; break;
    case 0x042a: lang = "Windows 98/Me, Windows NT 4.0 and later: Vietnamese"; break;
  }
  return lang;
#else
  return NULL;
#endif
}

char *iupwinGetComputerName(void)
{
  BOOL test;
  DWORD size = MAX_COMPUTERNAME_LENGTH + 1;
  LPTSTR v = (LPTSTR) iupStrGetMemory(sizeof(char)*(MAX_COMPUTERNAME_LENGTH + 1)); /* This will not work for Unicode */
  test = GetComputerName((LPTSTR) v, &size);
  if(size == 0 || test == 0) 
  { 
    free(v); 
    return NULL; 
  }
  return (char*)v;
}

char *iupwinGetUserName(void)
{
  BOOL test;
  DWORD size = UNLEN + 1;
  LPTSTR v = (LPTSTR) iupStrGetMemory(sizeof(char)*(UNLEN + 1)); /* This will not work for Unicode */
  test = GetUserName((LPTSTR) v, &size);
  if(size == 0 || test == 0) 
  { 
    free(v); 
    return NULL; 
  }
  assert(test != 0);
  return (char*)v;
}
