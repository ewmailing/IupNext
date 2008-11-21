/** \file
 * \brief iupLua Controls initialization
 *
 * See Copyright Notice in iup.h
 * $Id: il_controls.h,v 1.2 2008-11-21 05:46:06 scuri Exp $
 */
 
#ifndef __IL_CONTROLS_H 
#define __IL_CONTROLS_H

#ifdef __cplusplus
extern "C" {
#endif

int diallua_open (void);
int gaugelua_open (void);
int masklua_open (void);
int matrixlua_open (void);
int treelua_open (void);
int cblua_open (void);
int colorbarlua_open (void);
int cellslua_open(void);

#if (IUP_VERSION_NUMBER < 300000)
void iupgetparamlua_open (void);
int iupgclua_open (void);
int iupvallua_open (void);
int iuptabslua_open (void);
#endif

#ifdef __cplusplus
}
#endif

#endif
