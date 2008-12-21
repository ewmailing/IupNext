/** \file
 * \brief initializes dial, gauge, colorbrowser, colorbar controls.
 *
 * See Copyright Notice in "iup.h"
 */
 
#ifndef __IUPCONTROLS_H 
#define __IUPCONTROLS_H

#ifdef __cplusplus
extern "C" {
#endif


int IupControlsOpen(void);
void IupControlsClose(void);   /* for backward compatibility only, does nothing since IUP 3 */

void IupOldValOpen(void);
void IupOldTabsOpen(void);

Ihandle* IupColorbar(void);
Ihandle* IupCells(void);
Ihandle *IupColorBrowser(void);
Ihandle *IupColorBrowser(void);
Ihandle *IupGauge(void);
Ihandle *IupDial(const char *);
Ihandle* IupMatrix(const char *action);
Ihandle* IupTree(void);

/* IupTree utilities */
int   IupTreeSetUserId(Ihandle* ih, int id, void* userid);
void* IupTreeGetUserId(Ihandle* ih, int id);
int   IupTreeGetId(Ihandle* ih, void *userid);
void  IupTreeSetAttribute  (Ihandle* ih, const char* a, int id, char* v);
void  IupTreeStoreAttribute(Ihandle* ih, const char* a, int id, char* v);
char* IupTreeGetAttribute  (Ihandle* ih, const char* a, int id);
int   IupTreeGetInt        (Ihandle* ih, const char* a, int id);
float IupTreeGetFloat      (Ihandle* ih, const char* a, int id);
void  IupTreeSetfAttribute (Ihandle* ih, const char* a, int id, char* f, ...);

/* IupMatrix utilities */
void  IupMatSetAttribute  (Ihandle* ih, const char* a, int l, int c, char* v);
void  IupMatStoreAttribute(Ihandle* ih, const char* a, int l, int c, char* v);
char* IupMatGetAttribute  (Ihandle* ih, const char* a, int l, int c);
int   IupMatGetInt        (Ihandle* ih, const char* a, int l, int c);
float IupMatGetFloat      (Ihandle* ih, const char* a, int l, int c);
void  IupMatSetfAttribute (Ihandle* ih, const char* a, int l, int c, char* f, ...);

/* Used by IupColorbar */
#define IUP_PRIMARY -1
#define IUP_SECONDARY -2


#ifdef __cplusplus
}
#endif

#endif
