/** \file
 * \brief HWND to ihandle table
 *
 * See Copyright Notice in iup.h
 *  */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>  

#include <windows.h>

#include "iglobal.h"
#include "winhandle.h"

static Itable* winhandle_table; /* table indexed by HWND containing Ihandle* address */

/*
 * Retorna Ihandle associado a uma janela, caso
 * a janela pertenca a aplicacao e seja uma
 * janela IUP
 */
Ihandle* iupwinHandleGet(HWND hwnd)
{
  Ihandle *ret=NULL;

  assert(hwnd); 
  assert(IsWindow(hwnd));
  if(!IsWindow(hwnd))
    return NULL;

  /* checks if the window belogs to this thread */
  if(GetWindowThreadProcessId(hwnd, NULL) != GetCurrentThreadId())
    return NULL;

  ret = iupTableGet(winhandle_table, (char*) hwnd);

  if(ret == NULL)
    return NULL;

  /* checks if it is a valid ihandle */
  assert(type(ret));
  if(type(ret) == NULL)
    return NULL;

  return ret;
}

void iupwinHandleAdd(HWND hwnd, Ihandle *n)
{
  iupTableSet(winhandle_table, (char*) hwnd, n, IUP_POINTER);
}

void iupwinHandleRemove(Ihandle *n)
{
  assert(n);
  if(!n)
    return;
  assert(IsWindow(handle(n)));
  if(!IsWindow(handle(n)))
    return;

  iupTableRemove(winhandle_table, handle(n));
}

void iupwinHandleInit(void)
{
  winhandle_table = iupTableCreate(IUPTABLE_POINTERINDEXED);
}

void iupwinHandleFinish(void)
{
  iupTableDestroy(winhandle_table);
  winhandle_table = NULL;
}
