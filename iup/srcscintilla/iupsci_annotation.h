/** \file
 * \brief Scintilla control: Annotation
 *
 * See Copyright Notice in "iup.h"
 */
 
#ifndef __IUPSCI_ANNOTATION_H 
#define __IUPSCI_ANNOTATION_H

#ifdef __cplusplus
extern "C" {
#endif


int iupScintillaSetAnnotationTextAttribId(Ihandle* ih, int line, const char* value);
char* iupScintillaGetAnnotationTextAttribId(Ihandle* ih, int line);
int iupScintillaSetAnnotationStyleAttribId(Ihandle* ih, int line, const char* value);
char* iupScintillaGetAnnotationStyleAttribId(Ihandle* ih, int line);
int iupScintillaSetAnnotationStyleOffsetAttrib(Ihandle* ih, const char* value);
char* iupScintillaGetAnnotationStyleOffsetAttrib(Ihandle* ih);
int iupScintillaSetAnnotationVisibleAttrib(Ihandle *ih, const char *value);
char* iupScintillaGetAnnotationVisibleAttrib(Ihandle* ih);
int iupScintillaSetAnnotationClearAllAttrib(Ihandle* ih, const char* value);


#ifdef __cplusplus
}
#endif

#endif
