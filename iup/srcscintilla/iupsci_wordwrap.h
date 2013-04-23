/** \file
 * \brief Scintilla control: Line wrapping
 *
 * See Copyright Notice in "iup.h"
 */
 
#ifndef __IUPSCI_WORDWRAP_H 
#define __IUPSCI_WORDWRAP_H

#ifdef __cplusplus
extern "C" {
#endif


char* iupScintillaGetWordWrapAttrib(Ihandle *ih);
int iupScintillaSetWordWrapAttrib(Ihandle *ih, const char *value);
char* iupScintillaGetWordWrapVisualFlagsAttrib(Ihandle *ih);
int iupScintillaSetWordWrapVisualFlagsAttrib(Ihandle *ih, const char *value);


#ifdef __cplusplus
}
#endif

#endif
