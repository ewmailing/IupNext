/** \file
 * \brief Text Controls Private Declarations
 *
 * See Copyright Notice in iup.h
 */
 
#ifndef __IUP_TEXT_H 
#define __IUP_TEXT_H

#ifdef __cplusplus
extern "C" {
#endif


void iupdrvTextInitClass(Iclass* ic);
void iupdrvTextAddBorders(int *x, int *y);
void iupdrvTextAddFormatTag(Ihandle* ih, Ihandle* formattag);
void iupdrvTextConvertXYToChar(Ihandle* ih, int x, int y, int *lin, int *col, int *pos);

void iupTextUpdateFormatTags(Ihandle* ih);
char* iupTextGetPaddingAttrib(Ihandle* ih);
char* iupTextGetNCAttrib(Ihandle* ih);

struct _IcontrolData 
{
  int is_multiline,
      has_formatting,
      append_newline,
      nc,
      sb,                           /* scrollbar configuration */
      horiz_padding, vert_padding,  /* button margin */
      last_caret_pos;
  Iarray* formattags;
  Imask* mask;
};


#ifdef __cplusplus
}
#endif

#endif
