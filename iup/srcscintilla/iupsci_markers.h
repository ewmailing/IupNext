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


#ifdef __cplusplus
}
#endif

#endif
