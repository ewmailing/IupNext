/** \file
 * \brief Web control.
 *
 * See Copyright Notice in "iup.h"
 */
 
#ifndef __IUPWEB_H 
#define __IUPWEB_H

#ifdef __cplusplus
extern "C" {
#endif

int IupWebBrowserOpen(void);

#ifndef WIN32
Ihandle *IupWebBrowser(void);
#else
Ihandle *IupWebBrowser(const char *ProgID);
#endif


#ifdef __cplusplus
}
#endif

#endif
