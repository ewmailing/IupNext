/** \file
 * \brief Scintilla control: White space
 *
 * See Copyright Notice in "iup.h"
 */
 
#ifndef __IUPSCI_WHITESPACE_H 
#define __IUPSCI_WHITESPACE_H

#ifdef __cplusplus
extern "C" {
#endif


int iupScintillaSetViewWSAttrib(Ihandle *ih, const char *value);
char* iupScintillaGetViewWSAttrib(Ihandle* ih);
int iupScintillaSetWSFgColorAttrib(Ihandle *ih, const char *value);
int iupScintillaSetWSBgColorAttrib(Ihandle *ih, const char *value);
int iupScintillaSetWSSizeAttrib(Ihandle *ih, const char *value);
char* iupScintillaGetWSSizeAttrib(Ihandle* ih);
int iupScintillaSetWSExtraAscentAttrib(Ihandle *ih, const char *value);
char* iupScintillaGetWSExtraAscentAttrib(Ihandle* ih);
int iupScintillaSetWSExtraDescentAttrib(Ihandle *ih, const char *value);
char* iupScintillaGetWSExtraDescentAttrib(Ihandle* ih);


#ifdef __cplusplus
}
#endif

#endif
