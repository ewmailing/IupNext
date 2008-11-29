/** \file
 * \brief IUPLua5 Controls internal Functions
 *
 * See Copyright Notice in iup.h
 * $Id: il_controls.h,v 1.3 2008-11-29 03:55:20 scuri Exp $
 */
 
#ifndef __IL_CONTROLS_H 
#define __IL_CONTROLS_H

#ifdef __cplusplus
extern "C" {
#endif

void iupmasklua_open (lua_State * L);
int iupgaugelua_open (lua_State * L);
int iupdiallua_open (lua_State * L);
int iupcolorbrowserlua_open (lua_State * L);
int iupcolorbarlua_open (lua_State * L);
int iupcellslua_open (lua_State * L);
int iuptreelua_open (lua_State * L);
int iupmatrixlua_open (lua_State * L);

#ifdef __cplusplus
}
#endif

#endif
