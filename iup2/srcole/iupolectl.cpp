/*
 * iupolectl.cpp
 *
 *   CPI que implementa um container OLE
 */
// RCS Info
static char *rcsid = "$Id: iupolectl.cpp,v 1.1 2008-10-17 06:21:31 scuri Exp $";
static char *rcsname = "$Name: not supported by cvs2svn $";

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


