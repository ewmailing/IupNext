/** \file
 * \brief Motif Driver implementation of iupdrvSetGlobal
 *
 * See Copyright Notice in iup.h
 *  */

#include <Xm/Xm.h>
#include <stdio.h>
#include "iglobal.h"
#include "idrv.h"
#include "motif.h"

int iupdrvSetGlobal(const char *name, const char *value)
{
  if(iupStrEqual(name, "AUTOREPEAT"))
  {
    XKeyboardControl values;
    if(iupStrEqual(value, "YES"))
      values.auto_repeat_mode = 1;
    else
      values.auto_repeat_mode = 0;
    XChangeKeyboardControl(iupmot_auxdisplay, KBAutoRepeatMode, &values);
    return 1;
  }
  else if(iupStrEqual(name, "CURSORPOS"))
  {
    int x, y;
    if(iupStrToIntInt(value, &x, &y, 'x') == 2)
    {
      XEvent ev;
      XWarpPointer(iupmot_display,None,
        RootWindow(iupmot_display, iupmot_screen),0,0,0,0,x,y);
      XNextEvent(iupmot_auxdisplay, &ev);
      XFlush(iupmot_display);
    }
    return 1;
  }
  return 0;
}

char* iupdrvGetGlobal(const char *name)
{
  if(iupStrEqual(name, "CURSORPOS"))
  {
    Window root, child;
    int cx, cy;
    int x, y;
    unsigned int keys;
    char* curpos = iupStrGetMemory(50);

    XQueryPointer(iupmot_display, RootWindow(iupmot_display, iupmot_screen),
         &root, &child, &x, &y, &cx, &cy, &keys);

    sprintf(curpos, "%dx%d", x, y);
    return curpos;
  }
  return NULL;
}
