/** \file
 * \brief IupThread element
 *
 * See Copyright Notice in "iup.h"
 */
#ifndef WIN32
#include <glib.h>
/* #include <gthread.h> */
#else
#include <windows.h>
#endif

#include <stdio.h>              
#include <stdlib.h>
#include <string.h>             

#include "iup.h"
#include "iupcbs.h"

#include "iup_object.h"
#include "iup_attrib.h"
#include "iup_str.h"
#include "iup_register.h"
#include "iup_stdcontrols.h"


static int iThreadSetJoinAttrib(Ihandle* ih, const char* value)
{
#ifndef WIN32
  GThread* thread = (GThread*)iupAttribGet(ih, "THREAD");
  g_thread_join(thread);
#else
  HANDLE thread = (HANDLE)iupAttribGet(ih, "THREAD");
  WaitForSingleObject(thread, INFINITE);
#endif

  (void)value;
  return 0;
}

static int iThreadSetYieldAttrib(Ihandle* ih, const char* value)
{
  (void)ih;
  (void)value;

#ifndef WIN32
  g_thread_yield();
#else
  SwitchToThread();
#endif
  return 0;
}

static char* iThreadGetIsCurrentAttrib(Ihandle* ih)
{
#ifndef WIN32
  GThread* thread = (GThread*)iupAttribGet(ih, "THREAD");
  return iupStrReturnBoolean(thread == g_thread_self());
#else
  HANDLE thread = (HANDLE)iupAttribGet(ih, "THREAD");
  return iupStrReturnBoolean(thread == GetCurrentThread());
#endif
}

static int iThreadSetExitAttrib(Ihandle* ih, const char* value)
{
  int exit_code = 0;
  iupStrToInt(value, &exit_code);

#ifndef WIN32	
  g_thread_exit((gpointer)exit_code);
#else
  ExitThread(exit_code);
#endif	

  (void)ih;
  return 0;
}

static int iThreadSetLockAttrib(Ihandle* ih, const char* value)
{
  if (iupStrBoolean(value))
  {
#ifndef WIN32	
    GMutex* mutex = (GMutex*)iupAttribGet(ih, "MUTEX");
    g_mutex_lock(mutex);
#else
    HANDLE mutex = (HANDLE)iupAttribGet(ih, "MUTEX");
    WaitForSingleObject(mutex, INFINITE);
#endif	
  }
  else
  {
#ifndef WIN32	
    GMutex* mutex = (GMutex*)iupAttribGet(ih, "MUTEX");
    g_mutex_unlock(mutex);
#else
    HANDLE mutex = (HANDLE)iupAttribGet(ih, "MUTEX");
    ReleaseMutex(mutex);
#endif
  }

  return 1;
}

#ifndef WIN32
static void* ClientThreadFunc(void* obj)
#else
static DWORD WINAPI ClientThreadFunc(LPVOID obj)
#endif
{
  Ihandle* ih = (Ihandle*)obj;
  Icallback cb = IupGetCallback(ih, "THREAD_CB");
  if (cb)
    cb(ih);
  return 0;
};

static int iThreadSetStartAttrib(Ihandle* ih, const char* value)
{
  if (iupStrBoolean(value))
  {
#ifndef WIN32      
    GThread* thread, *old_thread;
    char* name = iupAttribGet(ih, "THREADNAME");
    if (!name) name = "IupThread";
    thread = g_thread_new(name, ClientThreadFunc, ih);
    old_thread = (GThread*)iupAttribGet(ih, "THREAD");
    if (old_thread) g_thread_unref(old_thread);
    iupAttribSet(ih, "THREAD", (char*)thread);
#else
    DWORD threadId;
    HANDLE thread = CreateThread(0, 0, ClientThreadFunc, ih, 0, &threadId);
    HANDLE old_thread = (HANDLE)iupAttribGet(ih, "THREAD");
    if (old_thread) CloseHandle(old_thread);
    iupAttribSet(ih, "THREAD", (char*)thread);
#endif
  }

  return 0;
}

static int iThreadCreateMethod(Ihandle* ih, void **params)
{
#ifndef WIN32
  GMutex* mutex = (GMutex*)malloc(sizeof(GMutex));
#else
  HANDLE mutex;
#endif	
  (void)params;

#ifndef WIN32
  g_mutex_init(mutex);
#else
  mutex = CreateMutexA(NULL, FALSE, "mutex");
#endif

  iupAttribSet(ih, "MUTEX", (char*)mutex);

  return IUP_NOERROR;
}

static void iThreadDestroyMethod(Ihandle* ih)
{
#ifndef WIN32
  GMutex* mutex = (GMutex*)iupAttribGet(ih, "MUTEX");
  GThread* thread = (GThread*)iupAttribGet(ih, "THREAD");
  g_mutex_clear(mutex);
  free(mutex);
  if (thread) g_thread_unref(thread);
#else
  HANDLE mutex = (HANDLE)iupAttribGet(ih, "MUTEX");
  HANDLE thread = (HANDLE)iupAttribGet(ih, "THREAD");
  CloseHandle(mutex);
  if (thread) CloseHandle(thread);
#endif
}

Iclass* iupThreadNewClass(void)
{
  Iclass* ic = iupClassNew(NULL);

  ic->name = "thread";
  ic->format = NULL; /* no parameters */
  ic->nativetype = IUP_TYPEVOID;
  ic->childtype = IUP_CHILDNONE;
  ic->is_interactive = 0;

  /* Class functions */
  ic->New = iupThreadNewClass;
  ic->Create = iThreadCreateMethod;
  ic->Destroy = iThreadDestroyMethod;

  /* Callbacks */
  iupClassRegisterCallback(ic, "THREAD_CB", "");

  /* Attributes */
  iupClassRegisterAttribute(ic, "START", NULL, iThreadSetStartAttrib, NULL, NULL, IUPAF_WRITEONLY | IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT | IUPAF_NO_DEFAULTVALUE);
  iupClassRegisterAttribute(ic, "LOCK", NULL, iThreadSetLockAttrib, NULL, NULL, IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT | IUPAF_NO_DEFAULTVALUE);
  iupClassRegisterAttribute(ic, "EXIT", NULL, iThreadSetExitAttrib, NULL, NULL, IUPAF_WRITEONLY | IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT | IUPAF_NO_DEFAULTVALUE);
  iupClassRegisterAttribute(ic, "ISCURRENT", iThreadGetIsCurrentAttrib, NULL, NULL, NULL, IUPAF_READONLY | IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT | IUPAF_NO_DEFAULTVALUE);
  iupClassRegisterAttribute(ic, "YIELD", NULL, iThreadSetYieldAttrib, NULL, NULL, IUPAF_WRITEONLY | IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT | IUPAF_NO_DEFAULTVALUE);
  iupClassRegisterAttribute(ic, "JOIN", NULL, iThreadSetJoinAttrib, NULL, NULL, IUPAF_WRITEONLY | IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT | IUPAF_NO_DEFAULTVALUE);

  return ic;
}

IUP_API Ihandle* IupThread(void)
{
  return IupCreate("thread");
}
