/** \file
 * \brief Timer for the GTK Driver.
 *
 * See Copyright Notice in "iup.h"
 */


#include <stdio.h>
#include <stdlib.h>

#include <gtk/gtk.h>

#include "iup.h"

#include "iup_object.h"
#include "iup_attrib.h"
#include "iup_str.h"
#include "iup_assert.h"
#include "iup_timer.h"


static gboolean gtkTimerProc(gpointer data)
{
  Ihandle *ih = (Ihandle*)data;
  Icallback cb;

  if (!iupObjectCheck(ih))   /* control could be destroyed before timer callback */
    return FALSE;

  cb = IupGetCallback(ih, "ACTION_CB");
  if (cb)
  {
    long long end = g_get_monotonic_time();
    long long start = iupTimerGetLongLong(ih, "STARTCOUNT");
    iupAttribSetInt(ih, "TIMEDELAY", (int)(end - start));

    if (cb(ih) == IUP_CLOSE)
      IupExitLoop();
  }

  return TRUE;
}

void iupdrvTimerRun(Ihandle *ih)
{
  unsigned int time_ms;

  if (ih->serial > 0) /* timer already started */
    return;
  
  time_ms = iupAttribGetInt(ih, "TIME");
  if (time_ms > 0)
  {
    long long start;

    ih->serial = g_timeout_add(time_ms, gtkTimerProc, (gpointer)ih);

    start = (long long)g_get_monotonic_time();
    iupAttribSetStrf(ih, "STARTCOUNT", "%lld", start);
  }
}

void iupdrvTimerStop(Ihandle* ih)
{
  if (ih->serial > 0)
  {
    g_source_remove(ih->serial);
    ih->serial = -1;
  }
}

void iupdrvTimerInitClass(Iclass* ic)
{
  (void)ic;
}
