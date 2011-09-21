#ifndef _MGL_MAKEFONT_H 
#define _MGL_MAKEFONT_H

#include "mgl/mgl_font.h"

class mglMakeFont
{
private:
  void    *fontFT;
  mglFont *fontMGL;
  double   ww;  /* maximal width */

  void mglMakeFontMemAlloc();
  void mglMakeFontMainCopy();
  void mglMakeFontSort();
  void mglMakeFontPrepareFont(mglGraph *mgl);
  void mglMakeFontAddGlyph(wchar_t chr);
  bool mglMakeFontSearchSymbols(const wchar_t *str, int size);

public:
  bool mglMakeLoadFontFT(mglGraph *mgl, const char *filename, int pt_size);
  void mglMakeFontSearchGlyph(mglGraph *mgl, const char* str);
};

#endif
