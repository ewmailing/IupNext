/** \file
 * \brief Scintilla control: Folding
 *
 * See Copyright Notice in "iup.h"
 */
 
#ifndef __IUPSCI_FOLDING_H 
#define __IUPSCI_FOLDING_H

#ifdef __cplusplus
extern "C" {
#endif


int iupScintillaSetFoldFlagsAttrib(Ihandle* ih, const char* value);
char* iupScintillaGetFoldLevelAttrib(Ihandle* ih, int line);
int iupScintillaSetFoldLevelAttrib(Ihandle* ih, int line, const char* value);
int iupScintillaSetToggleFoldAttrib(Ihandle* ih, const char* value);


#ifdef __cplusplus
}
#endif

#endif
