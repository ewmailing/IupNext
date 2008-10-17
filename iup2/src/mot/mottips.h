/** \file
 * \brief Motif Driver TIPS Control.
 *
 * See Copyright Notice in iup.h
 * $Id: mottips.h,v 1.1 2008-10-17 06:19:26 scuri Exp $
 */
 
#ifndef __MOTTIPS_H 
#define __MOTTIPS_H

#ifdef __cplusplus
extern "C" {
#endif

void iupmotTipEnterNotify ( Widget w, Ihandle *h );
void iupmotTipLeaveNotify ( void );

#ifdef __cplusplus
}
#endif

#endif
