/** \file
 * \brief IUPLua5 Controls internal Functions
 *
 * See Copyright Notice in iup.h
 * $Id: il_controls.h,v 1.2 2008-11-19 03:47:19 scuri Exp $
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
int iuptreelua_open (lua_State * L);
int iupmatrixlua_open (lua_State * L);

#if (IUP_VERSION_NUMBER < 300000)
int iupvallua_open (lua_State * L);
int iuptabslua_open (lua_State * L);
#endif

#ifdef __cplusplus
}
#endif

#endif
