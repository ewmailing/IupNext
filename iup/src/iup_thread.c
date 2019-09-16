/** \file
 * \brief IupThread element
 *
 * See Copyright Notice in "iup.h"
 */
#ifndef WIN32
#include <pthread.h>
#include <sys/time.h>
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


static int iThreadSetLockAttrib(Ihandle* ih, const char* value)
{
  if (iupStrBoolean(value))
  {
#ifndef WIN32	
    pthread_mutex_t* mutex = (pthread_mutex_t*)iupAttribGet(ih, "MUTEX");
    pthread_mutex_lock(mutex);
#else
    HANDLE mutex = (HANDLE)iupAttribGet(ih, "MUTEX");
    WaitForSingleObject(mutex, INFINITE);
#endif	
  }
  else
  {
#ifndef WIN32	
    pthread_mutex_t* mutex = (pthread_mutex_t*)iupAttribGet(ih, "MUTEX");
    pthread_mutex_unlock(mutex);
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
    pthread_t thread;
    pthread_create(&thread, NULL, ClientThreadFunc, ih);
#else
    DWORD threadId;
    HANDLE thread = CreateThread(0, 0, ClientThreadFunc, ih, 0, &threadId);
    HANDLE old_thread = (HANDLE)iupAttribGet(ih, "THREAD");
    CloseHandle(old_thread);
    iupAttribSet(ih, "THREAD", (char*)thread);
#endif
  }

  return 0;
}

static int iThreadCreateMethod(Ihandle* ih, void **params)
{
#ifndef WIN32
  pthread_mutex_t* mutex = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
#else
  HANDLE mutex;
#endif	
  (void)params;

#ifndef WIN32
  pthread_mutex_init(mutex, NULL);
#else
  mutex = CreateMutexA(NULL, FALSE, "mutex");
#endif

  iupAttribSet(ih, "MUTEX", (char*)mutex);

  return IUP_NOERROR;
}

static void iThreadDestroyMethod(Ihandle* ih)
{
#ifndef WIN32
  pthread_mutex_t* mutex = (pthread_mutex_t*)iupAttribGet(ih, "MUTEX");
  pthread_mutex_destroy(mutex);
  free(mutex);
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
  iupClassRegisterAttribute(ic, "START", NULL, iThreadSetStartAttrib, NULL, NULL, IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT | IUPAF_NO_DEFAULTVALUE);
  iupClassRegisterAttribute(ic, "LOCK", NULL, iThreadSetLockAttrib, NULL, NULL, IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT | IUPAF_NO_DEFAULTVALUE);

  return ic;
}

IUP_API Ihandle* IupThread(void)
{
  return IupCreate("thread");
}
