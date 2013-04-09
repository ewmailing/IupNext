/** \file
 * \brief Scintilla control: Style Definition
 *
 * See Copyright Notice in "iup.h"
 */
 
#ifndef __IUPSCI_STYLE_H 
#define __IUPSCI_STYLE_H

#ifdef __cplusplus
extern "C" {
#endif


char* iupScintillaGetCaseStyleAttrib(Ihandle* ih, int style);
int iupScintillaSetCaseStyleAttrib(Ihandle* ih, int style, const char* value);
char* iupScintillaGetVisibleStyleAttrib(Ihandle* ih, int style);
int iupScintillaSetVisibleStyleAttrib(Ihandle* ih, int style, const char* value);
char* iupScintillaGetHotSpotStyleAttrib(Ihandle* ih, int style);
int iupScintillaSetHotSpotStyleAttrib(Ihandle* ih, int style, const char* value);
char* iupScintillaGetCharSetStyleAttrib(Ihandle* ih, int style);
int iupScintillaSetCharSetStyleAttrib(Ihandle* ih, int style, const char* value);
char* iupScintillaGetEolFilledStyleAttrib(Ihandle* ih, int style);
int iupScintillaSetEolFilledStyleAttrib(Ihandle* ih, int style, const char* value);
char* iupScintillaGetFontSizeFracStyleAttrib(Ihandle* ih, int style);
int iupScintillaSetFontSizeFracStyleAttrib(Ihandle* ih, int style, const char* value);
char* iupScintillaGetFontSizeStyleAttrib(Ihandle* ih, int style);
int iupScintillaSetFontSizeStyleAttrib(Ihandle* ih, int style, const char* value);
char* iupScintillaGetFontStyleAttrib(Ihandle* ih, int style);
int iupScintillaSetFontStyleAttrib(Ihandle* ih, int style, const char* value);
char* iupScintillaGetWeightStyleAttrib(Ihandle* ih, int style);
int iupScintillaSetWeightStyleAttrib(Ihandle* ih, int style, const char* value);
char* iupScintillaGetUnderlineStyleAttrib(Ihandle* ih, int style);
int iupScintillaSetUnderlineStyleAttrib(Ihandle* ih, int style, const char* value);
char* iupScintillaGetItalicStyleAttrib(Ihandle* ih, int style);
int iupScintillaSetItalicStyleAttrib(Ihandle* ih, int style, const char* value);
char* iupScintillaGetBoldStyleAttrib(Ihandle* ih, int style);
int iupScintillaSetBoldStyleAttrib(Ihandle* ih, int style, const char* value);
char* iupScintillaGetFgColorStyleAttrib(Ihandle* ih, int style);
int iupScintillaSetFgColorStyleAttrib(Ihandle* ih, int style, const char* value);
char* iupScintillaGetBgColorStyleAttrib(Ihandle* ih, int style);
int iupScintillaSetBgColorStyleAttrib(Ihandle* ih, int style, const char* value);
int iupScintillaSetClearAllStyleAttrib(Ihandle* ih, const char* value);
int iupScintillaSetResetDefaultStyleAttrib(Ihandle* ih, const char* value);


#ifdef __cplusplus
}
#endif

#endif
