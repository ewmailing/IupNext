/** \file
 * \brief get/set callback
 *
 * See Copyright Notice in iup.h
 * $Id: icall.c,v 1.1 2008-10-17 06:19:20 scuri Exp $
 */

#include <stdlib.h>              /* NULL */
#include <stdarg.h>
#include <assert.h>

#include "iglobal.h"

Icallback IupGetCallback(Ihandle *ih, const char *name)
{
  Icallback func = NULL;
  enum Itable_Types itemType;

  assert(ih);
  if (!ih) return NULL;
  assert(name);
  if (!name) return NULL;

  func = (Icallback)iupTableGet2(env(ih), name, &itemType);

  if (func && itemType != IUP_RESPOINTER)
  {
    /* if not a REFPOINTER then it is an old fashion name */
    func = IupGetFunction((const char*)func);
  }

  return func;
}

Icallback IupSetCallback(Ihandle *ih, const char *name, Icallback func)
{
  Icallback old_func = NULL;

  assert(ih);
  if (!ih) return NULL;
  assert(name);
  if (!name) return NULL;

  if (!func)
    iupTableRemove(env(ih), name);
  else
  {
    enum Itable_Types itemType;
    old_func = (Icallback)iupTableGet2(env(ih), name, &itemType);
    if (old_func && itemType != IUP_RESPOINTER)
      old_func = IupGetFunction((const char*)old_func);

    iupTableSet(env(ih), name, (void*)func, IUP_RESPOINTER);
  }

  return old_func;
}

Ihandle* IupSetCallbacks(Ihandle* ih, ...)
{
  const char* name;
  Icallback func;

  va_list arglist;
  va_start(arglist, ih);

  name=va_arg(arglist, const char*);
  do
  {
    func=va_arg(arglist, Icallback);
    IupSetCallback(ih, name, func);

    name=va_arg(arglist, const char*);
  } while (name);

  va_end (arglist);
  return ih;
}
