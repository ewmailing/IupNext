/** \file
 * \brief global attributes enviroment
 *
 * See Copyright Notice in iup.h
 */
 
#ifndef __IUP_GLOBALATTRIB_H 
#define __IUP_GLOBALATTRIB_H

#ifdef __cplusplus
extern "C" {
#endif

/* called only in IupOpen and IupClose */
void iupGlobalAttribInit(void);
void iupGlobalAttribFinish(void);

/* Other functions declared in <iup.h> and implemented here. 
IupSetGlobal
IupStoreGlobal
IupGetGlobal
*/

#ifdef __cplusplus
}
#endif

#endif
