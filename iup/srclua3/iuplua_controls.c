/** \file
 * \brief Bindig of iupcontrols to Lua 3.
 *
 * See Copyright Notice in iup.h
 * $Id: iuplua_controls.c,v 1.1 2008-11-27 23:33:33 scuri Exp $
 */
 
#include <lua.h>

#include "iup.h"
#include "iuplua.h"
#include "iupluacontrols.h"
#include "il.h"
#include "il_controls.h"

int iupcontrolslua_open(void)
{
  gaugelua_open();
  treelua_open();
  matrixlua_open();
  masklua_open();
  diallua_open();
  cblua_open();
  colorbarlua_open();
  cellslua_open();
#if (IUP_VERSION_NUMBER < 300000)
  vallua_open();
  tabslua_open();
  gclua_open();
  getparamlua_open();
#endif
  return 1;
}
