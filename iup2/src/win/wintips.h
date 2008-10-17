/** \file
 * \brief Windows Driver TIPS management.
 *
 * See Copyright Notice in iup.h
 * $Id: wintips.h,v 1.1 2008-10-17 06:19:26 scuri Exp $
 */
 
#ifndef __WINTIPS_H 
#define __WINTIPS_H

#ifdef __cplusplus
extern "C" {
#endif

void iupwinTipsSet(Ihandle* h, char* v);
void iupwinTipsUpdate(Ihandle* h);

#ifdef __cplusplus
}
#endif

#endif
