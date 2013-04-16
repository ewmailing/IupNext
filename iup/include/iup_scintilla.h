/** \file
 * \brief Scintilla control.
 *
 * See Copyright Notice in "iup.h"
 */
 
#ifndef __IUP_SCINTILLA_H 
#define __IUP_SCINTILLA_H

#ifdef __cplusplus
extern "C" {
#endif


void IupScintillaOpen(void);

Ihandle *IupScintilla(void);

void IupScintillaConvertLinColToPos(Ihandle* ih, int lin, int col, int *pos);
void IupScintillaConvertPosToLinCol(Ihandle* ih, int pos, int *lin, int *col);


#ifdef __cplusplus
}
#endif

#endif
