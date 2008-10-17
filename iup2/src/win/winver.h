/** \file
 * \brief Windows Version Functions.
 *
 * See Copyright Notice in iup.h
 * $Id: winver.h,v 1.1 2008-10-17 06:19:26 scuri Exp $
 */
 
#ifndef __WINVER_H 
#define __WINVER_H

#ifdef __cplusplus
extern "C" {
#endif

void  iupwinVersion(void);
int   iupwinGetSystemMajorVersion(void);
char* iupwinGetSystemLanguage(void);
char* iupwinGetSystemName(void);
char* iupwinGetScreenSize(void);
char* iupwinGetScreenDepth(void);
char* iupwinGetComputerName(void);
char* iupwinGetUserName(void);
char *iupwinGetSystemVersion(void);

#ifdef __cplusplus
}
#endif

#endif
