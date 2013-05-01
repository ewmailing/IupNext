/** \file
 * \brief Scintilla control: Tabs and Indentation Guides
 *
 * See Copyright Notice in "iup.h"
 */
 
#ifndef __IUPSCI_TAB_H 
#define __IUPSCI_TAB_H

#ifdef __cplusplus
extern "C" {
#endif


char* iupScintillaGetTabSizeAttrib(Ihandle *ih);
int iupScintillaSetTabSizeAttrib(Ihandle *ih, const char *value);
char* iupScintillaGetHighlightGuideAttrib(Ihandle *ih);
int iupScintillaSetHighlightGuideAttrib(Ihandle *ih, const char *value);

#ifdef __cplusplus
}
#endif

#endif
