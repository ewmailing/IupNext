/** \file
 * \brief function table manager
 *
 * See Copyright Notice in iup.h
 * $Id: ifunc.c,v 1.1 2008-10-17 06:19:20 scuri Exp $
 */

#include <stdlib.h>  /* NULL */

#include "iup.h"

#include "istrutil.h"
#include "itable.h"
#include "ifunc.h"
#include "idrv.h"

static Itable *ifunc_table = NULL;   /* the function hast table indexed by the name string */
static const char *ifunc_action_name = NULL;  /* name of the action being retrieved in IupGetFunction */

void iupFuncInit(void)
{
  ifunc_table = iupTableCreate(IUPTABLE_STRINGINDEXED);
}

void iupFuncFinish(void)
{
  iupTableDestroy(ifunc_table);
  ifunc_table = NULL;
}

const char *IupGetActionName(void)
{
  return ifunc_action_name;
}

Icallback IupGetFunction(const char *name)
{
  Icallback func = NULL;

  ifunc_action_name = name; /* store the retrieved name */

  if (name)
  {
    func = (Icallback)iupTableGet(ifunc_table,name);

    /* if not defined and not the idle, then check for the DEFAULT_ACTION */
    if (!func && !iupStrEqual(name, "IDLE_ACTION"))
      func = (Icallback)iupTableGet(ifunc_table, "DEFAULT_ACTION");
  }

  return func;
}

Icallback IupSetFunction(const char *name, Icallback func)
{
  Icallback old_func = (Icallback)iupTableGet(ifunc_table, name);

  if (!func)
    iupTableRemove(ifunc_table, name);
  else
    iupTableSet(ifunc_table, name, (void*)func, IUP_POINTER);

  /* notifies the driver if changing the Idle */
  if (iupStrEqual(name, "IDLE_ACTION"))
    iupdrvSetIdleFunction(func);

  return old_func;
}
