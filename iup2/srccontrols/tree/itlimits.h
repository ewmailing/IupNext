/** \file
 * \brief iuptree control
 * Functions used to verify limits between specified canvas regions.
 *
 * See Copyright Notice in iup.h
 * $Id: itlimits.h,v 1.1 2008-10-17 06:20:41 scuri Exp $
 */
 
#ifndef __ITLIMITS_H 
#define __ITLIMITS_H

#ifdef __cplusplus
extern "C" {
#endif

int treeInsideRegion(cdCanvas *c, int x, int y, int x0, int y0, int w, int h );

#ifdef __cplusplus
}
#endif

#endif
