/** \file
 * \brief Scintilla control: Text retrieval and modification
 *
 * See Copyright Notice in "iup.h"
 */
 
#ifndef __IUPSCI_TEXT_H 
#define __IUPSCI_TEXT_H

#ifdef __cplusplus
extern "C" {
#endif


char* iupScintillaGetValueAttrib(Ihandle* ih);
int iupScintillaSetValueAttrib(Ihandle* ih, const char* value);
char* iupScintillaGetLineAttrib(Ihandle* ih, int line);
char* iupScintillaGetReadOnlyAttrib(Ihandle* ih);
int iupScintillaSetReadOnlyAttrib(Ihandle* ih, const char* value);
char* iupScintillaGetTextRangeAttrib(Ihandle* ih, int min, int max);
int iupScintillaSetPrependTextAttrib(Ihandle* ih, const char* value);
int iupScintillaSetAppendTextAttrib(Ihandle* ih, const char* value);
int iupScintillaSetInsertTextAttrib(Ihandle* ih, int pos, const char* value);
int iupScintillaSetClearAllAttrib(Ihandle* ih, const char* value);
int iupScintillaSetClearDocumentAttrib(Ihandle* ih, const char* value);
int iupScintillaSetDeleteRangeAttrib(Ihandle* ih, const char* value);
char* iupScintillaGetCharAttrib(Ihandle* ih, int pos);


#ifdef __cplusplus
}
#endif

#endif
