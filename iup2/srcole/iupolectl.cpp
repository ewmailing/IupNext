/*
 * iupolectl.cpp
 *
 *   CPI que implementa um container OLE
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "iup.h"
#include "iupcpi.h"


#include "iupole.h"
#include "tOleControl.h"


Ihandle *IupOleControl(const char *ProgID)
{
  return tOleControl::CreateFromArray(ProgID);
}

void IupOleControlOpen(void)
{
  tOleControl::Initialize();
}

void *IupOleControl_getclass(void)
{
  return tOleControl::iupclass;
}


