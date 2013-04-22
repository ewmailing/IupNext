/** \file
 * \brief Scintilla control.
 *
 * See Copyright Notice in "iup.h"
 */
 
#ifndef __IUPSCI_H 
#define __IUPSCI_H

#ifdef __cplusplus
extern "C" {
#endif


sptr_t iupScintillaSendMessage(Ihandle* ih, unsigned int iMessage, uptr_t wParam, sptr_t lParam);

struct _IcontrolData
{
  int sb;
  int append_newline;
  int last_caret_pos;
  int ignore_change;
};


#ifdef __cplusplus
}
#endif

#endif
