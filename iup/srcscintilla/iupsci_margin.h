/** \file
 * \brief Scintilla control: Margin
 *
 * See Copyright Notice in "iup.h"
 */
 
#ifndef __IUPSCI_MARGIN_H 
#define __IUPSCI_MARGIN_H

#ifdef __cplusplus
extern "C" {
#endif


char* iupScintillaGetMarginTypeNAttrib(Ihandle* ih, int margin);
int iupScintillaSetMarginTypeNAttrib(Ihandle* ih, int margin, const char* value);
char* iupScintillaGetMarginWidthNAttrib(Ihandle* ih, int margin);
int iupScintillaSetMarginWidthNAttrib(Ihandle* ih, int margin, const char* value);
char* iupScintillaGetMarginMaskNAttrib(Ihandle* ih, int margin);
int iupScintillaSetMarginMaskNAttrib(Ihandle* ih, int margin, const char* value);
char* iupScintillaGetMarginSensitiveNAttrib(Ihandle* ih, int margin);
int iupScintillaSetMarginSensitiveNAttrib(Ihandle* ih, int margin, const char* value);



#ifdef __cplusplus
}
#endif

#endif
