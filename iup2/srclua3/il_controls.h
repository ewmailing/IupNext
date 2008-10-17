/** \file
 * \brief iupLua Controls initialization
 *
 * See Copyright Notice in iup.h
 * $Id: il_controls.h,v 1.1 2008-10-17 06:21:03 scuri Exp $
 */
 
#ifndef __IL_CONTROLS_H 
#define __IL_CONTROLS_H

#ifdef __cplusplus
extern "C" {
#endif

int diallua_open (void);
int gaugelua_open (void);
int gclua_open (void);
void getparamlua_open (void);
int masklua_open (void);
int matrixlua_open (void);
int sboxlua_open (void);
int spinlua_open (void);
int tabslua_open (void);
int treelua_open (void);
int vallua_open (void);
int cblua_open (void);
int cboxlua_open (void);
int colorbarlua_open (void);
int cellslua_open(void);

#ifdef __cplusplus
}
#endif

#endif
