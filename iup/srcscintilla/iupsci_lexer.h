/** \file
 * \brief Scintilla control: Lexer
 *
 * See Copyright Notice in "iup.h"
 */
 
#ifndef __IUPSCI_LEXER_H 
#define __IUPSCI_LEXER_H

#ifdef __cplusplus
extern "C" {
#endif


char* iupScintillaGetLexerLanguageAttrib(Ihandle* ih);
int iupScintillaSetLexerLanguageAttrib(Ihandle* ih, const char* value);
int iupScintillaSetKeyWordsAttrib(Ihandle* ih, int keyWordSet, const char* value);
char* iupScintillaGetPropertyAttrib(Ihandle* ih);
int iupScintillaSetPropertyAttrib(Ihandle* ih, const char* value);
char* iupScintillaGetPropertyNamessAttrib(Ihandle* ih);
char* iupScintillaGetDescribeKeywordSetsAttrib(Ihandle* ih);


#ifdef __cplusplus
}
#endif

#endif
