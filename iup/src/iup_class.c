/** \file
 * \brief IUP Ihandle Class C Interface
 *
 * See Copyright Notice in "iup.h"
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



/*****************************************************************
                        Method Stubs
*****************************************************************/

static int iClassCreate(Iclass* ic, Ihandle* ih, void** params)
{
  int ret = IUP_NOERROR;
  if (ic->parent)
    ret = iClassCreate(ic->parent, ih, params);

  if (ret == IUP_NOERROR && ic->Create)
    ret = ic->Create(ih, params);

  return ret;
}

static int iClassMap(Iclass* ic, Ihandle* ih)
{
  int ret = IUP_NOERROR;
  if (ic->parent)
    ret = iClassMap(ic->parent, ih);

  if (ret == IUP_NOERROR && ic->Map)
    ret = ic->Map(ih);

  return ret;
}

static void iClassUnMap(Iclass* ic, Ihandle* ih)
{
  /* must be before the parent class */
  if (ic->UnMap)
    ic->UnMap(ih);

  if (ic->parent)
    iClassUnMap(ic->parent, ih);
}

static void iClassDestroy(Iclass* ic, Ihandle* ih)
{
  /* must destroy child class before the parent class */
  if (ic->Destroy)
    ic->Destroy(ih);

  if (ic->parent)
    iClassDestroy(ic->parent, ih);
}

static void iClassComputeNaturalSize(Iclass* ic, Ihandle* ih)
{
  if (ic->parent)
    iClassComputeNaturalSize(ic->parent, ih);

  if (ic->ComputeNaturalSize)
    ic->ComputeNaturalSize(ih);
}

static void iClassSetCurrentSize(Iclass* ic, Ihandle* ih, int w, int h, int shrink)
{
  if (ic->parent)
    iClassSetCurrentSize(ic->parent, ih, w, h, shrink);

  if (ic->SetCurrentSize)
    ic->SetCurrentSize(ih, w, h, shrink);
}

static Ihandle* iClassGetInnerContainer(Iclass* ic, Ihandle* ih)
{
  Ihandle* ih_container = ih;

  if (ic->parent)
    ih_container = iClassGetInnerContainer(ic->parent, ih);

  /* if the class implements the function it will ignore the result of the parent class */

  if (ic->GetInnerContainer)
    ih_container = ic->GetInnerContainer(ih);

  return ih_container;
}

static void* iClassGetInnerNativeContainerHandle(Iclass* ic, Ihandle* ih, Ihandle* child)
{
  void* container_handle = ih->handle;

  if (ic->parent)
    container_handle = iClassGetInnerNativeContainerHandle(ic->parent, ih, child);

  /* if the class implements the function it will ignore the result of the parent class */

  if (ic->GetInnerNativeContainerHandle)
    container_handle = ic->GetInnerNativeContainerHandle(ih, child);

  return container_handle;
}

static void iClassObjectChildAdded(Iclass* ic, Ihandle* ih, Ihandle* child)
{
  if (ic->parent)
    iClassObjectChildAdded(ic->parent, ih, child);

  if (ic->ChildAdded)
    ic->ChildAdded(ih, child);
}

static void iClassObjectChildRemoved(Iclass* ic, Ihandle* ih, Ihandle* child)
{
  if (ic->parent)
    iClassObjectChildRemoved(ic->parent, ih, child);

  if (ic->ChildRemoved)
    ic->ChildRemoved(ih, child);
}

static void iClassSetPosition(Iclass* ic, Ihandle* ih, int x, int y)
{
  if (ic->parent)
    iClassSetPosition(ic->parent, ih, x, y);

  if (ic->SetPosition)
    ic->SetPosition(ih, x, y);
}

static void iClassLayoutUpdate(Iclass* ic, Ihandle *ih)
{
  if (ic->parent)
    iClassLayoutUpdate(ic->parent, ih);

  if (ic->LayoutUpdate)
    ic->LayoutUpdate(ih);
}

static int iClassDlgPopup(Iclass* ic, Ihandle* ih, int x, int y)
{
  int ret = IUP_INVALID;  /* IUP_INVALID means it is not implemented */
  if (ic->parent)
    ret = iClassDlgPopup(ic->parent, ih, x, y);

  if (ret != IUP_ERROR && ic->DlgPopup)
    ret = ic->DlgPopup(ih, x, y);

  return ret;
}

int iupClassObjectCreate(Ihandle* ih, void** params)
{
  return iClassCreate(ih->iclass, ih, params);
}

int iupClassObjectMap(Ihandle* ih)
{
  return iClassMap(ih->iclass, ih);
}

void iupClassObjectUnMap(Ihandle* ih)
{
  iClassUnMap(ih->iclass, ih);
}

void iupClassObjectDestroy(Ihandle* ih)
{
  iClassDestroy(ih->iclass, ih);
}

void iupClassObjectComputeNaturalSize(Ihandle* ih)
{
  iClassComputeNaturalSize(ih->iclass, ih);
}

void iupClassObjectSetCurrentSize(Ihandle* ih, int w, int h, int shrink)
{
  iClassSetCurrentSize(ih->iclass, ih, w, h, shrink);
}

Ihandle* iupClassObjectGetInnerContainer(Ihandle* ih)
{
  return iClassGetInnerContainer(ih->iclass, ih);
}

void* iupClassObjectGetInnerNativeContainerHandle(Ihandle* ih, Ihandle* child)
{
  return iClassGetInnerNativeContainerHandle(ih->iclass, ih, child);
}

void iupClassObjectChildAdded(Ihandle* ih, Ihandle* child)
{
  iClassObjectChildAdded(ih->iclass, ih, child);
}

void iupClassObjectChildRemoved(Ihandle* ih, Ihandle* child)
{
  iClassObjectChildRemoved(ih->iclass, ih, child);
}

void iupClassObjectSetPosition(Ihandle* ih, int x, int y)
{
  iClassSetPosition(ih->iclass, ih, x, y);
}

void iupClassObjectLayoutUpdate(Ihandle *ih)
{
  iClassLayoutUpdate(ih->iclass, ih);
}

int iupClassObjectDlgPopup(Ihandle* ih, int x, int y)
{
  return iClassDlgPopup(ih->iclass, ih, x, y);
}


/*****************************************************************
                        Class Definition
*****************************************************************/


static void iClassReleaseAttribFuncTable(Iclass* ic)
{
  char* name = iupTableFirst(ic->attrib_func);
  while (name)
  {
    void* afunc = iupTableGetCurr(ic->attrib_func);
    free(afunc);

    name = iupTableNext(ic->attrib_func);
  }

  iupTableDestroy(ic->attrib_func);
}

Iclass* iupClassNew(Iclass* parent)
{
  Iclass* ic = malloc(sizeof(Iclass));
  memset(ic, 0, sizeof(Iclass));

  if (parent)
    ic->attrib_func = parent->attrib_func;
  else
    ic->attrib_func = iupTableCreate(IUPTABLE_STRINGINDEXED);

  ic->parent = parent;

  return ic;
}

void iupClassRelease(Iclass* ic)
{
  /* must release only the child class */
  if (ic->Release)
    ic->Release(ic);

  /* attributes functions table is released only on root classes */
  if (!ic->parent)
    iClassReleaseAttribFuncTable(ic);
}


/*****************************************************************
                        Main API
*****************************************************************/


char* IupGetClassName(Ihandle *ih)
{
  iupASSERT(iupObjectCheck(ih));
  if (!iupObjectCheck(ih))
    return NULL;

  return ih->iclass->name;
}

char* IupGetClassType(Ihandle *ih)
{
  static char* type2str[] = {"void", "control", "canvas", "dialog", "image", "menu"};
  iupASSERT(iupObjectCheck(ih));
  if (!iupObjectCheck(ih))
    return NULL;

  return type2str[ih->iclass->nativetype];
}

