/** \file
 * \brief Motif Driver implementation of iupdrvSetGlobal
 *
 * See Copyright Notice in "iup.h"
 */

#include <Xm/Xm.h>

#include <stdio.h>

#include "iup.h"

#include "iup_str.h"
#include "iup_drv.h"
#include "iup_drvinfo.h"

#include "iupmot_drv.h"


int iupdrvSetGlobal(const char *name, const char *value)
{
  if (iupStrEqual(name, "AUTOREPEAT"))
  {
    XKeyboardControl values;
    if (iupStrBoolean(value))
      values.auto_repeat_mode = 1;
    else
      values.auto_repeat_mode = 0;
    XChangeKeyboardControl(iupmot_display, KBAutoRepeatMode, &values);
    return 0;
  }
  return 1;
}

char* iupdrvGetGlobal(const char *name)
{
  (void)name;
  return NULL;
}
