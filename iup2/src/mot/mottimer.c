/** \file
 * \brief Timer for the Motif Driver
 *
 * See Copyright Notice in iup.h
 * $Id: mottimer.c,v 1.1 2008-10-17 06:19:26 scuri Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <Xm/PushB.h>

#include "iglobal.h"
#include "iup.h"
#include "iupcpi.h"
#include "imacros.h"
#include "icpi.h"
#include "motif.h"

static void motTimerRun(Ihandle *n);

static void motTimerProc(XtPointer client_data, XtIntervalId *id)
{
  Ihandle *n = (Ihandle*) client_data;
  IFn cb;
  if(n)
  {
    number(n) = -1;

    cb = (IFn) IupGetCallback(n, "ACTION_CB");
    if(cb)
    {
      /* we have to restart the timer everytime */
      motTimerRun(n);
      
      iupmot_incallback = 1;
      if(cb(n)==IUP_CLOSE) iupmot_exitmainloop = 1;
      iupmot_incallback = 0;
    }
  }
  else
  {
    /* iupError("Timer callback called, but no timer created"); */
  }
}

static void motTimerRun(Ihandle *n)
{
  unsigned int time_ms;

  if(number(n) > 0) /* timer already started */
    return;
  
  time_ms = IupGetInt(n, IUP_TIME);
  if(time_ms > 0)
  {
    XtAppContext context = XtWidgetToApplicationContext(iupmot_initialwidget);
    number(n) = XtAppAddTimeOut(context, time_ms, motTimerProc, (XtPointer) n);
  }
}

static void motTimerDestroy(Ihandle* n)
{
  if(number(n) > 0)
  {
    XtRemoveTimeOut(number(n));
    number(n) = -1;
  }
}

static void motTimerSetAttr(Ihandle *n, char *a, char *v)
{
  if(iupStrEqual(a, IUP_RUN))
  {
    if(!iupStrEqual(v, IUP_YES))
      motTimerDestroy(n);
    else
      motTimerRun(n);
  }
}

static Ihandle *motTimerCreate(Iclass *ic)
{
  Ihandle *timer = NULL;
  assert(ic != NULL);
  if(ic == NULL)
    return NULL;

  timer = IupUser();
  number(timer) = -1;
  return timer;
}

void IupTimerOpen(void)
{
  Iclass* ICTimer = iupCpiCreateNewClass("timer", NULL);
  iupCpiSetClassMethod(ICTimer, ICPI_SETATTR, (Imethod) motTimerSetAttr);
  iupCpiSetClassMethod(ICTimer, ICPI_DESTROY, (Imethod) motTimerDestroy);
  iupCpiSetClassMethod(ICTimer, ICPI_CREATE, (Imethod) motTimerCreate);
}

void IupTimerClose(void)
{
}

Ihandle* IupTimer(void)
{
	return IupCreate("timer");
}
