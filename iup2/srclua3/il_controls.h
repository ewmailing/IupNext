/** \file
 * \brief iupLua Controls initialization
 *
 * See Copyright Notice in iup.h
 *  */
 
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
void getparamlua_open (void);
int gclua_open (void);
int vallua_open (void);
int tabslua_open (void);
#endif

#ifdef __cplusplus
}
#endif

#endif
