/** \file
 * \brief Ihandle <-> Name table manager.
 *
 * See Copyright Notice in iup.h
 *  */
 
#ifndef __INAMES_H 
#define __INAMES_H

#ifdef __cplusplus
extern "C" {
#endif

void iupNamesInit(void);
void iupNamesFinish(void);

/* Other functions declared in <iup.h> and implemented here.
char* IupGetName(Ihandle* n);
int IupGetAllDialogs(char *names[], int n);
int IupGetAllNames(char *names[], int n);
Ihandle* IupSetHandle (char *name, Ihandle *h);
Ihandle *IupGetHandle (char *name);
*/

#ifdef __cplusplus
}
#endif

#endif
