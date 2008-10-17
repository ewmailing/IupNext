/** \file
 * \brief global attributes enviroment
 *
 * See Copyright Notice in iup.h
 * $Id: iglobalenv.h,v 1.1 2008-10-17 06:19:20 scuri Exp $
 */
 
#ifndef __IGLOBALENV_H 
#define __IGLOBALENV_H

#ifdef __cplusplus
extern "C" {
#endif

void iupGlobalEnvInit(void);
void iupGlobalEnvFinish(void);

/* Other functions declared in <iup.h> and implemented here. 
void IupSetGlobal(const char *key, char *value)
void IupStoreGlobal(const char *key, char *value)
char *IupGetGlobal(const char *key)
*/

#ifdef __cplusplus
}
#endif

#endif
