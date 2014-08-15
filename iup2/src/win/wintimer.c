/** \file
 * \brief Timer for the Windows Driver.
 *
 * See Copyright Notice in iup.h
 *  */
 
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <windows.h>

#include "iglobal.h"
#include "icpi.h"
#include "winproc.h"

#define NUM_MAX_TIMERS 64 /* Max of 64 timers in IUP at the same time */

static Ihandle *wintimer_ids[NUM_MAX_TIMERS];
static HWND wintimer_hwnd = NULL;

static int winTimerAssignId(Ihandle *h)
{
  int i;
  for(i=0; i<NUM_MAX_TIMERS; i++)
  {
    if(wintimer_ids[i] == NULL)
    {
      wintimer_ids[i] = h;
      number(h) = i;
      return i;
    }
  }
  /* iupError("id could not be set, more timers than allowed"); */
  return -1;
}

static void CALLBACK winTimerProc(HWND hwnd, UINT msg, UINT_PTR wid, DWORD time)
{
  Ihandle *n = NULL;
  int id = ((int)wid)-1; /* windows timer ID starts at 1 */

  if(hwnd != wintimer_hwnd)
    return; /* This is not IUP */

  if(msg != WM_TIMER)
    return; /* This shouldn't have reached here */

  assert(id < NUM_MAX_TIMERS && id >= 0);
  if(id >= NUM_MAX_TIMERS || id < 0)
  {
    /* iupError("Timer callback called, but no timer created"); */
    return;
  }

  n = wintimer_ids[id];
  if(n)
  {
    IFn cb = (IFn)IupGetCallback(n, "ACTION_CB");
    if(cb)
    {
      if(cb(n) == IUP_CLOSE)
        IupExitLoop();
    }
  }
  else
  {
    /* iupError("Timer callback called, but no timer created"); */
  }
}

static void winTimerRun(Ihandle *n)
{
  unsigned int time_ms;

  if(number(n) >= 0) /* timer already started */
    return;

  time_ms = IupGetInt(n, IUP_TIME);
  if(time_ms > 0)
  {
    int id = winTimerAssignId(n);
    if(id >= 0)
      SetTimer(wintimer_hwnd, (UINT_PTR)(id+1), time_ms, (TIMERPROC)winTimerProc);
  }
}

static void winTimerStop(Ihandle* n)
{
  if(number(n) >= 0)
  {
    KillTimer(wintimer_hwnd, number(n)+1);
    wintimer_ids[number(n)] = NULL;
    number(n) = -1;
  }
}

static void winTimerDestroy(Ihandle* n)
{
  winTimerStop(n);
}

static void winTimerSetAttr(Ihandle *n, char *a, char *v)
{
  if(iupStrEqual(a, IUP_RUN))
  {
    if(!iupStrEqual(v, IUP_YES))
      winTimerStop(n);
    else
      winTimerRun(n);
  }
}

static Ihandle *winTimerCreate(Iclass *ic)
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
  iupCpiSetClassMethod(ICTimer, ICPI_SETATTR, (Imethod) winTimerSetAttr);
  iupCpiSetClassMethod(ICTimer, ICPI_DESTROY, (Imethod) winTimerDestroy);
  iupCpiSetClassMethod(ICTimer, ICPI_CREATE, (Imethod) winTimerCreate);
  
  memset(wintimer_ids, 0, NUM_MAX_TIMERS*sizeof(Ihandle*));
  wintimer_hwnd = CreateWindow("BUTTON", "dummy", WS_DISABLED, 0, 0, 0, 0, NULL, NULL, GetModuleHandle(NULL), NULL);
}

void IupTimerClose(void)
{
  DestroyWindow(wintimer_hwnd); /* FIXME: should we destroy all timers before? */
}

Ihandle* IupTimer(void)
{
	return IupCreate("timer");
}
