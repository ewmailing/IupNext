/** \file
 * \brief Scintilla control: Markers
 *
 * See Copyright Notice in "iup.h"
 */
 
#ifndef __IUPSCI_MARKERS_H 
#define __IUPSCI_MARKERS_H

#ifdef __cplusplus
extern "C" {
#endif


char* iupScintillaGetMarkerSymbolAttribId(Ihandle* ih, int markerNumber);
int iupScintillaSetMarkerSymbolAttribId(Ihandle* ih, int markerNumber, const char* value);
int iupScintillaSetMarkerDefineAttrib(Ihandle* ih, const char* value);
int iupScintillaSetMarkerDefineRGBAImageId(Ihandle* ih, int markerNumber, const char* value);
int iupScintillaSetRGBAImageSetScale(Ihandle* ih, const char* value);
int iupScintillaSetMarkerFgColorAttribId(Ihandle* ih, int markerNumber, const char* value);
int iupScintillaSetMarkerBgColorAttribId(Ihandle* ih, int markerNumber, const char* value);
int iupScintillaSetMarkerBgColorSelectedAttribId(Ihandle* ih, int markerNumber, const char* value);
int iupScintillaSetMarkerAlphaAttribId(Ihandle* ih, int markerNumber, const char* value);
int iupScintillaSetMarkerEnableHighlightAttrib(Ihandle *ih, const char *value);
int iupScintillaSetMarkerAddAttribId(Ihandle* ih, int line, const char* value);
int iupScintillaSetMarkerDeleteAttribId(Ihandle* ih, int line, const char* value);
char* iupScintillaGetMarkerGetAttribId(Ihandle* ih, int line);
int iupScintillaSetMarkerDeleteAllAttrib(Ihandle* ih, const char* value);
int iupScintillaSetMarkerNextAttribId(Ihandle* ih, int lineStart, const char* value);
int iupScintillaSetMarkerPreviousAttribId(Ihandle* ih, int lineStart, const char* value);
int iupScintillaSetMarkerLineFromHandleAttrib(Ihandle* ih, const char* value);
int iupScintillaSetMarkerDeleteHandleAttrib(Ihandle* ih, const char* value);
char* iupScintillaGetLastMarkerHandle(Ihandle* ih);
char* iupScintillaGetLastMarkerFound(Ihandle* ih);


#ifdef __cplusplus
}
#endif

#endif
