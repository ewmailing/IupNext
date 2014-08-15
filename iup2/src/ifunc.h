/** \file
 * \brief Global Function table.
 *
 * See Copyright Notice in iup.h
 *  */
 
#ifndef __IFUNC_H 
#define __IFUNC_H

#ifdef __cplusplus
extern "C" {
#endif

void iupFuncInit(void);
void iupFuncFinish(void);

/* Other functions declared in <iup.h> and implemented here. 
char *IupGetActionName (void);
Icallback IupGetFunction (const char *name);
Icallback IupSetFunction (const char *name, Icallback func);
*/

#ifdef __cplusplus
}
#endif

#endif
