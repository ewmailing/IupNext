/** \file
 * \brief CD canvas utilities.
 * Header file that contains a function to activate the canvas
 *
 * See Copyright Notice in iup.h
 */
 
#ifndef __TREECD_H 
#define __TREECD_H

#include <cd.h>
#include <cdiup.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CdActivate(ih, err)  do {                                             \
                               if(ih->data->cdcanvas)                         \
                                 err = cdCanvasActivate(ih->data->cddbuffer); \
                               else                                           \
                                 err = CD_ERROR;                              \
                             } while(0)

#ifdef __cplusplus
}
#endif

#endif
