/** \file
 * \brief Bindig of iupcontrols to Lua 3.
 *
 * See Copyright Notice in iup.h
 * $Id: luacontrols.c,v 1.1 2008-11-21 03:00:12 scuri Exp $
 */
 
#include <lua.h>

#include "iup.h"
#include "iuplua.h"
#include "iupluacontrols.h"
#include "il.h"
#include "il_controls.h"

int iupcontrolslua_open(void)
{
  vallua_open();
  gaugelua_open();
  tabslua_open();
  treelua_open();
  matrixlua_open();
  masklua_open();
  diallua_open();
  cblua_open();
  gclua_open();
  sboxlua_open();
  spinlua_open();
  cboxlua_open();
  colorbarlua_open();
  cellslua_open();
  getparamlua_open();
  return 1;
}
