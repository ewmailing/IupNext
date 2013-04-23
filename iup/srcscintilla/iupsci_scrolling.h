/** \file
 * \brief Scintilla control: Scrolling and automatic scrolling
 *
 * See Copyright Notice in "iup.h"
 */
 
#ifndef __IUPSCI_SCROLLING_H 
#define __IUPSCI_SCROLLING_H

#ifdef __cplusplus
extern "C" {
#endif


int iupScintillaSetScrollToAttrib(Ihandle *ih, const char *value);
int iupScintillaSetScrollToPosAttrib(Ihandle *ih, const char *value);
int iupScintillaSetScrollCaretAttrib(Ihandle *ih, const char *value);
char* iupScintillaGetScrollWidthAttrib(Ihandle* ih);
int iupScintillaSetScrollWidthAttrib(Ihandle* ih, const char* value);


#ifdef __cplusplus
}
#endif

#endif
