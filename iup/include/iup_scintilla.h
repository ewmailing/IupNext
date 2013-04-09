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

#ifdef GTK
#define IUP_SSM(sci, m, w, l) scintilla_send_message((ScintillaObject*)sci, m, w, l)
#else
#define IUP_SSM(hwnd, m, w, l) SendMessage(hwnd, m, w, l)
#endif

void IupScintillaDecodeColor(long color, unsigned char *r, unsigned char *g, unsigned char *b);
long IupScintillaEncodeColor(unsigned char r, unsigned char g, unsigned char b);
void IupScintillaConvertLinColToPos(Ihandle* ih, int lin, int col, int *pos);
void IupScintillaConvertPosToLinCol(Ihandle* ih, int pos, int *lin, int *col);

struct _IcontrolData
{
#ifdef GTK
  GtkWidget *editor;
#endif
  int sb;
  int append_newline;
  int last_caret_pos;
  void* mask;
};

#ifdef __cplusplus
}
#endif

#endif
