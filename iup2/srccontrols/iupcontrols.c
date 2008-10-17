/** \file
 * \brief initializes iupdial, iupgauge, iuptabs, iupcb, iupgc and iupval controls.
 *
 * See Copyright Notice in iup.h
 * $Id: iupcontrols.c,v 1.1 2008-10-17 06:19:56 scuri Exp $
 */


#include <iup.h>
#include <iupcontrols.h>
#include "icontrols.h"

#include <assert.h>

static int opened = 0;

int IupControlsOpen(void)
{
  if (opened)
    return IUP_OPENED;
  opened = 1;

  IupValOpen();
  IupDialOpen();
  IupGaugeOpen();
  IupTabsOpen();
  IupColorBrowserOpen();
  IupGetColorOpen(); /* this must be after mask and colorbrowser open */
  IupMatrixOpen();
  IupTreeOpen();
  IupColorbarOpen();
  IupCellsOpen();

  return IUP_NOERROR;
}

void IupControlsClose(void)
{
  if (!opened)
    return;
  opened = 0;

  IupMatrixClose();
  IupTreeClose();
}
