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


Ihandle *IupWebBrowser(const char* progid);

int IupWebBrowserOpen(void);


#ifdef __cplusplus
}
#endif

#endif
