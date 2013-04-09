/** \file
 * \brief Scintilla control: Overtype
 *
 * See Copyright Notice in "iup.h"
 */
 
#ifndef __IUPSCI_OVERTYPE_H 
#define __IUPSCI_OVERTYPE_H

#ifdef __cplusplus
extern "C" {
#endif


char* iupScintillaGetOvertypeAttrib(Ihandle *ih);
int iupScintillaSetOvertypeAttrib(Ihandle *ih, const char *value);


#ifdef __cplusplus
}
#endif

#endif
