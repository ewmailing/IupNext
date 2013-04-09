/** \file
 * \brief Scintilla control: Selection and information
 *
 * See Copyright Notice in "iup.h"
 */
 
#ifndef __IUPSCI_SELECTION_H 
#define __IUPSCI_SELECTION_H

#ifdef __cplusplus
extern "C" {
#endif


char* iupScintillaGetCaretAttrib(Ihandle* ih);
int iupScintillaSetCaretAttrib(Ihandle* ih, const char* value);
char* iupScintillaGetCaretPosAttrib(Ihandle* ih);
int iupScintillaSetCaretPosAttrib(Ihandle* ih, const char* value);
char* iupScintillaGetCountAttrib(Ihandle* ih);
char* iupScintillaGetLineCountAttrib(Ihandle* ih);
char* iupScintillaGetCurrentLineAttrib(Ihandle* ih);
char* iupScintillaGetSelectedTextAttrib(Ihandle* ih);
int iupScintillaSetSelectedTextAttrib(Ihandle* ih, const char* value);
char* iupScintillaGetSelectionAttrib(Ihandle* ih);
int iupScintillaSetSelectionAttrib(Ihandle* ih, const char* value);
char* iupScintillaGetSelectionPosAttrib(Ihandle* ih);
int iupScintillaSetSelectionPosAttrib(Ihandle* ih, const char* value);


#ifdef __cplusplus
}
#endif

#endif
