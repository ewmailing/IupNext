/** \file
 * \brief Bindig of iupcontrols to Lua 3.
 *
 * See Copyright Notice in iup.h
 * $Id: luacontrols.c,v 1.3 2009-03-23 15:00:01 scuri Exp $
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
  masklua_open();
  cblua_open();
  colorbarlua_open();
  cellslua_open();
#if (IUP_VERSION_NUMBER < 300000)
  vallua_open();
  tabslua_open();
  gclua_open();
  getparamlua_open();
#endif
  matrixlua_open();  /* Must be after Val, depends on val callbacks initialization */
  diallua_open();  /* Must be after Val, uses val callbacks */
  return 1;
}
