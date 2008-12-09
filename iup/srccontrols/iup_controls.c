/** \file
 * \brief initializes the additional controls.
 *
 * See Copyright Notice in "iup.h"
 */


#include "iup.h"
#include "iupcontrols.h"

#include "iup_object.h"
#include "iup_childtree.h"
#include "iup_register.h"
#include "iup_controls.h"
#include "iup_attrib.h"


static int icontrols_opened = 0;

int IupControlsOpen(void)
{
  if (icontrols_opened)
    return IUP_OPENED;
  icontrols_opened = 1;

  iupRegisterClass(iupDialGetClass());
  iupRegisterClass(iupCellsGetClass());
  iupRegisterClass(iupColorbarGetClass());
  iupRegisterClass(iupColorBrowserGetClass());
  iupRegisterClass(iupMatrixGetClass());
  iupRegisterClass(iupTreeGetClass());
  iupRegisterClass(iupGaugeGetClass());
  iupRegisterClass(iupColorBrowserDlgGetClass());

  return IUP_NOERROR;
}

void IupControlsClose(void)  /* for backward compatibility */
{
}

char *iupControlBaseGetParentBgColor(Ihandle* ih)
{
  return IupGetAttribute(iupChildTreeGetNativeParent(ih), "BGCOLOR");
}

char *iupControlBaseGetBgColorAttrib(Ihandle* ih)
{
  /* check the hash table */
  char *color = iupAttribGetStr(ih, "BGCOLOR");

  /* If not defined check native definition from parent */
  if (!color)
    color = iupControlBaseGetParentBgColor(ih);

  return color;
}

