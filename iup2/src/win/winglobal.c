/** \file
 * \brief Windows Driver iupdrvSetGlobal
 *
 * See Copyright Notice in iup.h
 * $Id: winglobal.c,v 1.2 2013-05-20 20:13:53 scuri Exp $
 */

#include <stdio.h>
#include <string.h>
#include <windows.h>

#include "iglobal.h"
#include "idrv.h"
#include "win.h"

int iupdrvSetGlobal(const char *name, const char *value)
{
  if(iupStrEqual(name, "CURSORPOS"))
  {
    int x, y;
    if (iupStrToIntInt(value, &x, &y, 'x') == 2)
      SetCursorPos(x, y);
    return 1;
  }
  else if(iupStrEqual(name, "MOUSEHOOK"))
  {
    if (iupStrEqualNoCase(value, "ON"))
      iupwinSetMouseHook(1);
    else
      iupwinSetMouseHook(0);
    return 1;
  }
  else if(iupStrEqual(name, "DLL_HINSTANCE"))
  {
    iupwin_hinstance = (HINSTANCE)value;
    return 1;
  }
  
  return 0;
}

char *iupdrvGetGlobal(const char *name)
{
  if(iupStrEqual(name, "CURSORPOS"))
  {
    char *s = iupStrGetMemory(50);
    POINT CursorPoint;
    GetCursorPos( &CursorPoint );
    sprintf(s, "%dx%d", (int)CursorPoint.x, (int)CursorPoint.y);
    return s;
  }
  if(iupStrEqual(name, "SHIFTKEY"))
  {
    if (GetAsyncKeyState(VK_SHIFT) & 0x8000)
      return IUP_ON;
    return IUP_OFF;
  }
  if(iupStrEqual(name, "CONTROLKEY"))
  {
    if (GetAsyncKeyState(VK_CONTROL) & 0x8000)
      return IUP_ON;
    return IUP_OFF;
  }
  if(iupStrEqual(name, "SCREENDEPTH"))
    return iupwinGetScreenDepth();
  if(iupStrEqual(name, "SCREENSIZE"))
    return iupwinGetScreenSize();
  if(iupStrEqual(name, "MOUSEHOOK"))
  {
    if (iupwinSetMouseHook(-1))
      return IUP_ON;
    return IUP_OFF;
  }
  return NULL;
}
