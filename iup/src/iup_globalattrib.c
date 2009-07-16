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
#include "iup_drvfont.h"
#include "iup_assert.h"
#include "iup_str.h"


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

  if (iupStrEqual(name, "DEFAULTFONTSIZE"))
  {
    iupSetDefaultFontSizeGlobalAttrib(value);
    return;
  }

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

  if (iupStrEqual(name, "DEFAULTFONTSIZE"))
  {
    iupSetDefaultFontSizeGlobalAttrib(value);
    return;
  }

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

  if (iupStrEqual(name, "DEFAULTFONTSIZE"))
    return iupGetDefaultFontSizeGlobalAttrib();

  value = iupdrvGetGlobal(name);

  if (!value)
    value = (char*)iupTableGet(iglobal_table, name);

  return value;
}

int iupGlobalIsPointer(const char* name)
{
  static struct {
    const char *name;
  } ptr_table[] = {
#ifdef WIN32
    {"HINSTANCE"},
#else
    {"XDISPLAY"},
    {"XSCREEN"},
    {"APPSHELL"},
#endif
  };
#define PTR_TABLE_SIZE ((sizeof ptr_table)/(sizeof ptr_table[0]))

  if (name)
  {
    int i;
    for (i = 0; i < PTR_TABLE_SIZE; i++)
    {
      if (iupStrEqualNoCase(name, ptr_table[i].name))
        return 1;
    }
  }

  return 0;
}
