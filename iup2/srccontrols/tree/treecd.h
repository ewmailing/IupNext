/** \file
 * \brief CD canvas utilities.
 * Header file that contains a function to activate the canvas
 *
 * See Copyright Notice in iup.h
 * $Id: treecd.h,v 1.1 2008-10-17 06:20:41 scuri Exp $
 */
 
#ifndef __TREECD_H 
#define __TREECD_H

#include <cd.h>
#include <cdiup.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CdActivate(tree,err)  do{if((tree)->cdcanvas) \
                                  err = cdCanvasActivate((tree)->cddbuffer); \
                                else                            \
                                  err = CD_ERROR;               \
                               }while(0)

#ifdef __cplusplus
}
#endif

#endif
