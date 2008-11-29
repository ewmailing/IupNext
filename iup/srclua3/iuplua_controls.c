/** \file
 * \brief Bindig of iupcontrols to Lua 3.
 *
 * See Copyright Notice in iup.h
 * $Id: iuplua_controls.c,v 1.2 2008-11-29 03:55:20 scuri Exp $
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
  vallua_open();
  tabslua_open();
  gclua_open();
  getparamlua_open();
  return 1;
}
