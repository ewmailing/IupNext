/** \file
 * \brief global attributes enviroment
 *
 * See Copyright Notice in "iup.h"
 */

#include <stdlib.h>      

#include "iup.h" 

#include "iup_table.h"
#include "iup_globalattrib.h"
#include "iup_drv.h"
#include "iup_assert.h"


static Itable *iglobal_table = NULL;

void iupGlobalAttribInit(void)
{
  iglobal_table = iupTableCreate(IUPTABLE_STRINGINDEXED);
}

void iupGlobalAttribFinish(void)
{
  iupTableDestroy(iglobal_table);
  iglobal_table = NULL;
}

void IupSetGlobal(const char *name, const char *value)
{
  iupASSERT(name!=NULL);
  if (!name) return;

  if (iupdrvSetGlobal(name, value))
  {
    if (!value)
      iupTableRemove(iglobal_table, name);
    else
      iupTableSet(iglobal_table, name, (void*)value, IUPTABLE_POINTER);
  }
}

void IupStoreGlobal(const char *name, const char *value)
{
  iupASSERT(name!=NULL);
  if (!name) return;

  if (iupdrvSetGlobal(name, value))
  {
    if (!value)
      iupTableRemove(iglobal_table, name);
    else
      iupTableSet(iglobal_table, name, (void*)value, IUPTABLE_STRING);
  }
}

char *IupGetGlobal(const char *name)
{
  char* value;
  
  iupASSERT(name!=NULL);
  if (!name) 
    return NULL;

  value = iupdrvGetGlobal(name);
  if (!value) 
    value = (char*)iupTableGet(iglobal_table, name);

  return value;
}
