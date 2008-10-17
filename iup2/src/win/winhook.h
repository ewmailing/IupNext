/** \file
 * \brief Windows Hooks
 *
 * See Copyright Notice in iup.h
 * $Id: winhook.h,v 1.1 2008-10-17 06:19:26 scuri Exp $
 */
 
#ifndef __WINHOOK_H 
#define __WINHOOK_H

#ifdef __cplusplus
extern "C" {
#endif

/* Prototipos de funcoes exportadas */

void iupwinHookInit (void);
void iupwinHookFinish(void);
void iupwinUnhook(void);
void iupwinRehook(void);

#ifdef __cplusplus
}
#endif

#endif
