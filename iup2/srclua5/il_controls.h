/** \file
 * \brief IUPLua5 Controls internal Functions
 *
 * See Copyright Notice in iup.h
 * $Id: il_controls.h,v 1.1 2008-10-17 06:21:23 scuri Exp $
 */
 
#ifndef __IL_CONTROLS_H 
#define __IL_CONTROLS_H

#ifdef __cplusplus
extern "C" {
#endif

void iupmasklua_open (lua_State * L);
void iupgetparamlua_open (lua_State * L);
int iupgclua_open (lua_State * L);
int iupgaugelua_open (lua_State * L);
int iupdiallua_open (lua_State * L);
int iupcolorbrowserlua_open (lua_State * L);
int iupcolorbarlua_open (lua_State * L);
int iupcellslua_open (lua_State * L);
int iupcboxlua_open (lua_State * L);
int iupvallua_open (lua_State * L);
int iuptreelua_open (lua_State * L);
int iuptabslua_open (lua_State * L);
int iupspinboxlua_open (lua_State * L);
int iupspinlua_open (lua_State * L);
int iupsboxlua_open (lua_State * L);
int iupmatrixlua_open (lua_State * L);

#ifdef __cplusplus
}
#endif

#endif
