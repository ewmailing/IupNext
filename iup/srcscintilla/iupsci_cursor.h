/** \file
 * \brief Scintilla control: Cursor and Zooming
 *
 * See Copyright Notice in "iup.h"
 */
 
#ifndef __IUPSCI_CURSOR_H 
#define __IUPSCI_CURSOR_H

#ifdef __cplusplus
extern "C" {
#endif


char* iupScintillaGetCursorAttrib(Ihandle *ih);
int iupScintillaSetCursorAttrib(Ihandle *ih, const char *value);

int iupScintillaSetZoomInAttrib(Ihandle *ih, const char *value);
int iupScintillaSetZoomOutAttrib(Ihandle *ih, const char *value);
int iupScintillaSetZoomAttrib(Ihandle *ih, const char *value);
char* iupScintillaGetZoomAttrib(Ihandle* ih);

#ifdef __cplusplus
}
#endif

#endif
