/** \file
 * \brief Scintilla control: Brace highlighting
 *
 * See Copyright Notice in "iup.h"
 */
 
#ifndef __IUPSCI_BRACELIGHT_H 
#define __IUPSCI_BRACELIGHT_H

#ifdef __cplusplus
extern "C" {
#endif


int iupScintillaSetBraceHighlightAttrib(Ihandle* ih, const char* value);
int iupScintillaSetBraceBadlightAttrib(Ihandle* ih, const char* value);
int iupScintillaSetBraceHighlightIndicatorAttrib(Ihandle* ih, const char* value);
int iupScintillaSetBraceBadlightIndicatorAttrib(Ihandle* ih, const char* value);
char* iupScintillaGetBraceMatchAttribId(Ihandle* ih, int pos);
int iupScintillaSetUseBraceHLIndicatorAttrib(Ihandle *ih, const char *value);
char* iupScintillaGetUseBraceHLIndicatorAttrib(Ihandle* ih);
int iupScintillaSetUseBraceBLIndicatorAttrib(Ihandle *ih, const char *value);
char* iupScintillaGetUseBraceBLIndicatorAttrib(Ihandle* ih);


#ifdef __cplusplus
}
#endif

#endif
