/** \file
 * \brief Ihandle Class Attribute Management
 *
 * See Copyright Notice in iup.h
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>

#include "iup.h"

#include "iup_object.h"
#include "iup_drv.h"
#include "iup_drvfont.h"
#include "iup_str.h"
#include "iup_attrib.h"
#include "iup_assert.h"


typedef struct _IattribFunc
{
  IattribGetFunc get;
  IattribSetFunc set;
  const char* default_value;
  Imap mapped;
  Iinherit inherit;
  int has_id;
} IattribFunc;


static const char* iClassFindId(const char* name)
{
  while(*name)
  {
    if (*name >= '0' && *name <= '9')
      return name;

    name++;
  }
  return NULL;
}

static const char* iClassCutNameId(const char* name, const char* name_id)
{
  char* str;
  int len = name_id - name;
  if (len == 0)
    return NULL;

  str = iupStrGetMemory(len+1);
  memcpy(str, name, len);
  str[len] = 0;
  return str;
}

int iupClassObjectSetAttribute(Ihandle* ih, const char* name, const char * value, int *inherit)
{
  IattribFunc* afunc;

  if (ih->iclass->has_attrib_id)
  {
    const char* name_id = iClassFindId(name);
    if (name_id)
    {
      IattribFunc* afunc;
      const char* partial_name = iClassCutNameId(name, name_id);
      if (!partial_name)
        partial_name = "IDVALUE";  /* pure numbers are used as attributes in IupList and IupMatrix, 
                                      translate them into IDVALUE. */
      afunc = (IattribFunc*)iupTableGet(ih->iclass->attrib_func, partial_name);
      if (afunc)
      {
        *inherit = 0;       /* id numbered attributes are NON inheritable always */
        if (afunc->set && (ih->handle || !afunc->mapped))
        {
          /* id numbered attributes have default value NULL always */
          IattribSetIdFunc id_set = (IattribSetIdFunc)afunc->set;
          return id_set(ih, name_id, value);
        }
        else
          return 1; /* if the function exists, then must return here */
      }
    }
  }

  /* if not has_attrib_id, or not found an ID, or not found the partial name, check using the full name */

  afunc = (IattribFunc*)iupTableGet(ih->iclass->attrib_func, name);
  *inherit = 1; /* default is inheritable */
  if (afunc)
  {
    *inherit = afunc->inherit;
    if (afunc->set && (ih->handle || !afunc->mapped))
    {
      int ret;
      if (!value)
      {
        /* inheritable attributes when reset must check the parent value */
        if (*inherit && ih->parent && !iupAttribIsInternal(name))   
          value = iupAttribGetStrInherit(ih->parent, name); 

        if (!value)
          value = afunc->default_value;
      }

      if (afunc->has_id)
      {
        IattribSetIdFunc id_set = (IattribSetIdFunc)afunc->set;
        return id_set(ih, "", value);  /* empty Id */
      }
      else
        ret = afunc->set(ih, value);

      if (*inherit)
        return 1;   /* inheritable attributes are always stored in the hash table, */
      else          /* to indicate that they are set at the control.               */
        return ret;
    }
  }
  return 1;
}

char* iupClassObjectGetAttribute(Ihandle* ih, const char* name, char* *def_value, int *inherit)
{
  IattribFunc* afunc;

  if (ih->iclass->has_attrib_id)
  {
    const char* name_id = iClassFindId(name);
    if (name_id)
    {
      IattribFunc* afunc;
      const char* partial_name = iClassCutNameId(name, name_id);
      if (!partial_name)
        partial_name = "IDVALUE";  /* pure numbers are used as attributes in IupList and IupMatrix, 
                                      translate them into IDVALUE. */
      afunc = (IattribFunc*)iupTableGet(ih->iclass->attrib_func, partial_name);
      if (afunc)
      {
        *def_value = NULL;  /* id numbered attributes have default value NULL always */
        *inherit = 0;       /* id numbered attributes are NON inheritable always */
        if (afunc->get && (ih->handle || !afunc->mapped))
        {
          IattribGetIdFunc id_get = (IattribGetIdFunc)afunc->get;
          return id_get(ih, name_id);
        }
        else
          return NULL;      /* if the function exists, then must return here */
      }
    }
  }

  /* if not has_attrib_id, or not found an ID, or not found the partial name, check using the full name */

  afunc = (IattribFunc*)iupTableGet(ih->iclass->attrib_func, name);
  *def_value = NULL;
  *inherit = 1; /* default is inheritable */
  if (afunc)
  {
    *def_value = (char*)afunc->default_value;
    *inherit = afunc->inherit;
    if (afunc->get && (ih->handle || !afunc->mapped))
    {
      if (afunc->has_id)
      {
        IattribGetIdFunc id_get = (IattribGetIdFunc)afunc->get;
        return id_get(ih, "");  /* empty Id */
      }
      else
        return afunc->get(ih);
    }
  }
  return NULL;
}

char* iupClassObjectGetAttributeDefault(Ihandle* ih, const char* name)
{
  IattribFunc* afunc = (IattribFunc*)iupTableGet(ih->iclass->attrib_func, name);
  if (afunc)
    return (char*)afunc->default_value;
  return NULL;
}

void iupClassRegisterAttribute(Iclass* ic, const char* name, 
                               IattribGetFunc _get, IattribSetFunc _set, 
                               const char* _default_value, Imap _mapped, Iinherit _inherit)
{
  IattribFunc* afunc = (IattribFunc*)iupTableGet(ic->attrib_func, name);
  if (afunc)
    free(afunc);  /* overwrite a previous registration */

  afunc = (IattribFunc*)malloc(sizeof(IattribFunc));
  afunc->get = _get;
  afunc->set = _set;
  afunc->default_value = _default_value;
  afunc->mapped = _mapped;
  afunc->inherit = _inherit;
  afunc->has_id = 0;

  iupTableSet(ic->attrib_func, name, (void*)afunc, IUPTABLE_POINTER);
}

void iupClassRegisterAttributeId(Iclass* ic, const char* name, 
                               IattribGetIdFunc _get, IattribSetIdFunc _set, 
                               const char* _default_value, Imap _mapped, Iinherit _inherit)
{
  IattribFunc* afunc = (IattribFunc*)iupTableGet(ic->attrib_func, name);
  if (afunc)
    free(afunc);  /* overwrite a previous registration */

  afunc = (IattribFunc*)malloc(sizeof(IattribFunc));
  afunc->get = (IattribGetFunc)_get;
  afunc->set = (IattribSetFunc)_set;
  afunc->default_value = _default_value;
  afunc->mapped = _mapped;
  afunc->inherit = _inherit;
  afunc->has_id = 1;

  iupTableSet(ic->attrib_func, name, (void*)afunc, IUPTABLE_POINTER);
}

void iupClassRegisterGetAttribute(Iclass* ic, const char* name, 
                                  IattribGetFunc *_get, IattribSetFunc *_set, 
                                  const char* *_default_value, Imap *_mapped, Iinherit *_inherit, int *_has_id)
{
  IattribFunc* afunc = (IattribFunc*)iupTableGet(ic->attrib_func, name);
  if (afunc)
  {
    if (_get) *_get = afunc->get;
    if (_set) *_set = afunc->set;
    if (_default_value) *_default_value = afunc->default_value;
    if (_mapped) *_mapped = afunc->mapped;
    if (_inherit) *_inherit = afunc->inherit;
    if (_has_id) *_has_id = afunc->has_id;
  }
}

void iupClassRegisterCallback(Iclass* ic, const char* name, const char* format)
{
  /* Since attributes and callbacks do not conflict 
     we can use the same structure to store the callback format using the default_value. */
  iupClassRegisterAttribute(ic, name, NULL, NULL, format, IUP_NOT_MAPPED, IUP_NO_INHERIT);
}

char* iupClassCallbackGetFormat(Iclass* ic, const char* name)
{
  IattribFunc* afunc = (IattribFunc*)iupTableGet(ic->attrib_func, name);
  if (afunc)
    return (char*)afunc->default_value;
  return NULL;
}

int IupGetClassAttributes(Ihandle* ih, char *names[], int n)
{
  Iclass* ic;
  char *name;
  int i = 0;

  iupASSERT(iupObjectCheck(ih));
  if (!iupObjectCheck(ih))
    return 0;

  iupASSERT(names!=NULL);
  if (!names)
    return 0;

  ic = ih->iclass;

  name = iupTableFirst(ic->attrib_func);
  while (name)
  {
    strcpy(names[i], name);
    i++;
    if (i == n)
      break;

    name = iupTableNext(ic->attrib_func);
  }

  return i;
}

void IupSaveClassAttributes(Ihandle* ih)
{
  Iclass* ic;
  char *name;

  iupASSERT(iupObjectCheck(ih));
  if (!iupObjectCheck(ih))
    return;

  ic = ih->iclass;

  name = iupTableFirst(ic->attrib_func);
  while (name)
  {
    if (!iupAttribIsInternal(name) && !iupAttribIsPointer(name))
    {
      int inherit;
      char *def_value;
      char *value = iupClassObjectGetAttribute(ih, name, &def_value, &inherit);
      if (value && value != iupAttribGetStr(ih, name))
        iupAttribStoreStr(ih, name, value);
    }

    name = iupTableNext(ic->attrib_func);
  }
}

int iupClassCurIsInherit(Iclass* ic)
{
  IattribFunc* afunc = (IattribFunc*)iupTableGetCurr(ic->attrib_func);
  if (afunc && afunc->inherit)
    return 1;
  return 0;
}

