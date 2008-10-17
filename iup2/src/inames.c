/** \file
 * \brief Ihandle <-> Name table manager.
 *
 * See Copyright Notice in iup.h
 * $Id: inames.c,v 1.1 2008-10-17 06:19:20 scuri Exp $
 */

#include <stdlib.h>  /* NULL */

#include "iup.h"

#include "istrutil.h"
#include "itable.h"
#include "inames.h"

#include "iglobal.h"  // FIXME

static Itable *inames_strtable = NULL;        /* table indexed by name containing Ihandle* address */
static Itable *inames_ihtable = NULL;   /* table indexed by Ihandle* address containing names */

void iupNamesInit(void)
{
  inames_strtable = iupTableCreate(IUPTABLE_STRINGINDEXED);
  inames_ihtable = iupTableCreate(IUPTABLE_POINTERINDEXED);
}

void iupNamesFinish(void)
{
  iupTableDestroy(inames_strtable);
  inames_strtable = NULL;

  iupTableDestroy(inames_ihtable);
  inames_ihtable = NULL;
}

Ihandle *IupGetHandle(const char *name)
{
  if(name == NULL)
    return NULL;
  return (Ihandle*)iupTableGet (inames_strtable, name);
}

Ihandle* IupSetHandle(const char *name, Ihandle *h)
{
  Ihandle *oe = iupTableGet (inames_strtable, name);
  if (h != NULL)
  {
    iupTableSet(inames_strtable, name, h, IUP_POINTER);
    iupTableSet(inames_ihtable, (char*) h, (char*)name, IUP_STRING);
  }
  else
  {
    h = iupTableGet(inames_strtable, name);
    iupTableRemove(inames_strtable, name);
    if (h) iupTableRemove(inames_ihtable, (char*)h);
  }
  return oe;
}

int IupGetAllNames(char *names[], int n)
{
  int i = 0;
  names[i]=iupTableFirst(inames_strtable);
  while (names[i])
  {
    i++;
    if (i == n)
      break;
    names[i]=iupTableNext(inames_strtable);
  }
  return i;
}

int IupGetAllDialogs(char *names[], int n)
{
  int i = 0;
  names[i]=iupTableFirst(inames_strtable);
  while (names[i])
  {
    Ihandle *d=(Ihandle*)iupTableGetCurr(inames_strtable);
    if (d && type(d)==DIALOG_)
      i++;
    if (i == n)
      break;
    names[i]=iupTableNext(inames_strtable);
  }
  return i;
}

char* IupGetName(Ihandle* ih)
{
  return iupTableGet(inames_ihtable, (char*)ih);
}
