/** \file
 * \brief Scintilla control: Cut, copy and paste
 *
 * See Copyright Notice in "iup.h"
 */
 
#ifndef __IUPSCI_CLIPBOARD_H 
#define __IUPSCI_CLIPBOARD_H

#ifdef __cplusplus
extern "C" {
#endif


int iupScintillaSetClipboardAttrib(Ihandle *ih, const char *value);
char* iupScintillaGetCanPasteAttrib(Ihandle* ih);

int iupScintillaSetUndoAttrib(Ihandle *ih, const char *value);
char* iupScintillaGetUndoAttrib(Ihandle* ih);
int iupScintillaSetRedoAttrib(Ihandle *ih, const char *value);
char* iupScintillaGetRedoAttrib(Ihandle* ih);
int iupScintillaSetUndoCollectAttrib(Ihandle *ih, const char *value);
char* iupScintillaGetUndoCollectAttrib(Ihandle* ih);


#ifdef __cplusplus
}
#endif

#endif
