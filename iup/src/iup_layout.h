/** \file
 * \brief Abstract Layout Management
 *
 * See Copyright Notice in "iup.h"
 */
 
#ifndef __IUP_LAYOUT_H 
#define __IUP_LAYOUT_H

#ifdef __cplusplus
extern "C" {
#endif


/* called only from IupMap */
void iupLayoutUpdate(Ihandle* ih);
void iupLayoutCompute(Ihandle* ih);

/* Other functions declared in <iup.h> and implemented here. 
IupRefresh
*/


#ifdef __cplusplus
}
#endif

#endif
