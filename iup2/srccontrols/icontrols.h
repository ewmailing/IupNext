/** \file
 * \brief controls initialization.
 *
 * See Copyright Notice in iup.h
 * $Id: icontrols.h,v 1.1 2008-10-17 06:19:56 scuri Exp $
 */
 
#ifndef __ICONTROLS_H 
#define __ICONTROLS_H

#ifdef __cplusplus
extern "C" {
#endif

void IupMatrixOpen(void);
void IupMatrixClose(void);

void IupTreeOpen( void );
void IupTreeClose(void);

void IupValOpen(void);
void IupColorBrowserOpen(void);
void IupColorbarOpen(void);
void IupDialOpen(void);
void IupGaugeOpen(void);
void IupGetColorOpen(void);
void IupTabsOpen(void);
void IupCellsOpen(void);

#ifdef __cplusplus
}
#endif

#endif
