/** \file
 * \brief Scintilla control: Style Definition
 *
 * See Copyright Notice in "iup.h"
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <Scintilla.h>
#include <SciLexer.h>

#ifdef GTK
#include <gtk/gtk.h>
#include <ScintillaWidget.h>
#else
#include <windows.h>
#endif

#include "iup.h"

#include "iup_object.h"
#include "iup_attrib.h"
#include "iup_str.h"
#include "iup_stdcontrols.h"

#include "iupsci_style.h"
#include "iup_scintilla.h"

/***** STYLE DEFINITION *****
Attributes not implement yet:
SCI_STYLESETCHANGEABLE(int styleNumber, bool changeable)
SCI_STYLEGETCHANGEABLE(int styleNumber)  
*/

char* iupScintillaGetCaseStyleAttrib(Ihandle* ih, int style)
{
  int caseSty;

  if(style == IUP_INVALID_ID)
    style = 0;  /* Lexer style default */
  
  caseSty = IUP_SSM(ih->handle, SCI_STYLEGETCASE, style, 0);
  
  if(caseSty == SC_CASE_UPPER)
    return "UPPERCASE";
  else if(caseSty == SC_CASE_LOWER)
    return "LOWERCASE";
  else
    return "SC_CASE_MIXED";
}

int iupScintillaSetCaseStyleAttrib(Ihandle* ih, int style, const char* value)
{
  if(style == IUP_INVALID_ID)
    style = 0;  /* Lexer style default */
  
  if (iupStrEqualNoCase(value, "UPPERCASE"))
    IUP_SSM(ih->handle, SCI_STYLESETCASE, style, SC_CASE_UPPER);
  else if (iupStrEqualNoCase(value, "LOWERCASE"))
    IUP_SSM(ih->handle, SCI_STYLESETCASE, style, SC_CASE_LOWER);
  else
    IUP_SSM(ih->handle, SCI_STYLESETCASE, style, SC_CASE_MIXED);  /* default */

  return 0;
}

char* iupScintillaGetVisibleStyleAttrib(Ihandle* ih, int style)
{
  if(style == IUP_INVALID_ID)
    style = 0;  /* Lexer style default */

  if(IUP_SSM(ih->handle, SCI_STYLEGETVISIBLE, style, 0))
    return "YES";
  else
    return "NO";
}

int iupScintillaSetVisibleStyleAttrib(Ihandle* ih, int style, const char* value)
{
  if(style == IUP_INVALID_ID)
    style = 0;  /* Lexer style default */

  if (iupStrEqualNoCase(value, "YES"))
    IUP_SSM(ih->handle, SCI_STYLESETVISIBLE, style, 1);
  else
    IUP_SSM(ih->handle, SCI_STYLESETVISIBLE, style, 0);

  return 0;
}

char* iupScintillaGetHotSpotStyleAttrib(Ihandle* ih, int style)
{
  if(style == IUP_INVALID_ID)
    style = 0;  /* Lexer style default */

  if(IUP_SSM(ih->handle, SCI_STYLEGETHOTSPOT, style, 0))
    return "YES";
  else
    return "NO";
}

int iupScintillaSetHotSpotStyleAttrib(Ihandle* ih, int style, const char* value)
{
  if(style == IUP_INVALID_ID)
    style = 0;  /* Lexer style default */

  if (iupStrEqualNoCase(value, "YES"))
    IUP_SSM(ih->handle, SCI_STYLESETHOTSPOT, style, 1);
  else
    IUP_SSM(ih->handle, SCI_STYLESETHOTSPOT, style, 0);

  return 0;
}

char* iupScintillaGetCharSetStyleAttrib(Ihandle* ih, int style)
{
  int charset;
  
  if(style == IUP_INVALID_ID)
    style = 0;  /* Lexer style default */
  
  charset = IUP_SSM(ih->handle, SCI_STYLEGETCHARACTERSET, style, 0);

  if(charset == SC_CHARSET_EASTEUROPE)
    return "EASTEUROPE";
  else if(charset == SC_CHARSET_RUSSIAN)
    return "RUSSIAN";
  else if(charset == SC_CHARSET_GB2312)
    return "GB2312";
  else if(charset == SC_CHARSET_HANGUL)
    return "HANGUL";
  else if(charset == SC_CHARSET_SHIFTJIS)
    return "SHIFTJIS";
  else
    return "ANSI";
}

int iupScintillaSetCharSetStyleAttrib(Ihandle* ih, int style, const char* value)
{
  if(style == IUP_INVALID_ID)
    style = 0;  /* Lexer style default */

  /* These character sets are supported both on Windows as on GTK */
  if (iupStrEqualNoCase(value, "EASTEUROPE"))
    IUP_SSM(ih->handle, SCI_STYLESETCHARACTERSET, style, SC_CHARSET_EASTEUROPE);
  else if (iupStrEqualNoCase(value, "RUSSIAN"))
    IUP_SSM(ih->handle, SCI_STYLESETCHARACTERSET, style, SC_CHARSET_RUSSIAN);
  else if (iupStrEqualNoCase(value, "GB2312"))  /* Chinese charset */
    IUP_SSM(ih->handle, SCI_STYLESETCHARACTERSET, style, SC_CHARSET_GB2312);
  else if (iupStrEqualNoCase(value, "HANGUL"))  /* Korean charset */
    IUP_SSM(ih->handle, SCI_STYLESETCHARACTERSET, style, SC_CHARSET_HANGUL);
  else if (iupStrEqualNoCase(value, "SHIFTJIS"))  /* Japanese charset */
    IUP_SSM(ih->handle, SCI_STYLESETCHARACTERSET, style, SC_CHARSET_SHIFTJIS);
  else
    IUP_SSM(ih->handle, SCI_STYLESETCHARACTERSET, style, SC_CHARSET_ANSI);  /* default */

  return 0;
}

char* iupScintillaGetEolFilledStyleAttrib(Ihandle* ih, int style)
{
  if(style == IUP_INVALID_ID)
    style = 0;  /* Lexer style default */

  if(IUP_SSM(ih->handle, SCI_STYLEGETEOLFILLED, style, 0))
    return "YES";
  else
    return "NO";
}

int iupScintillaSetEolFilledStyleAttrib(Ihandle* ih, int style, const char* value)
{
  if(style == IUP_INVALID_ID)
    style = 0;  /* Lexer style default */

  if (iupStrEqualNoCase(value, "YES"))
    IUP_SSM(ih->handle, SCI_STYLESETEOLFILLED, style, 1);
  else
    IUP_SSM(ih->handle, SCI_STYLESETEOLFILLED, style, 0);

  return 0;
}

char* iupScintillaGetFontSizeFracStyleAttrib(Ihandle* ih, int style)
{
  int size;
  char* str = iupStrGetMemory(15);

  if(style == IUP_INVALID_ID)
    style = 0;  /* Lexer style default */

  size = IUP_SSM(ih->handle, SCI_STYLESETSIZEFRACTIONAL, style, 0);

  sprintf(str, "%f", (double)(size / SC_FONT_SIZE_MULTIPLIER));

  return str;
}

int iupScintillaSetFontSizeFracStyleAttrib(Ihandle* ih, int style, const char* value)
{
  float size;

  if(style == IUP_INVALID_ID)
    style = 0;  /* Lexer style default */

  iupStrToFloat(value, &size);

  IUP_SSM(ih->handle, SCI_STYLESETSIZEFRACTIONAL, style, (int)(size*SC_FONT_SIZE_MULTIPLIER));

  return 0;
}

char* iupScintillaGetFontSizeStyleAttrib(Ihandle* ih, int style)
{
  int size;
  char* str = iupStrGetMemory(15);

  if(style == IUP_INVALID_ID)
    style = 0;  /* Lexer style default */

  size = IUP_SSM(ih->handle, SCI_STYLEGETSIZE, style, 0);

  sprintf(str, "%d", size);

  return str;
}

int iupScintillaSetFontSizeStyleAttrib(Ihandle* ih, int style, const char* value)
{
  int size;

  if(style == IUP_INVALID_ID)
    style = 0;  /* Lexer style default */

  iupStrToInt(value, &size);

  IUP_SSM(ih->handle, SCI_STYLESETSIZE, style, size);

  return 0;
}

char* iupScintillaGetFontStyleAttrib(Ihandle* ih, int style)
{
  char* str = iupStrGetMemory(15);

  if(style == IUP_INVALID_ID)
    style = 0;  /* Lexer style default */

  IUP_SSM(ih->handle, SCI_STYLEGETFONT, style, (sptr_t)str);

  return str;
}

int iupScintillaSetFontStyleAttrib(Ihandle* ih, int style, const char* value)
{
  if(style == IUP_INVALID_ID)
    style = 0;  /* Lexer style default */

  IUP_SSM(ih->handle, SCI_STYLESETFONT, style, (sptr_t)value);

  return 0;
}

char* iupScintillaGetWeightStyleAttrib(Ihandle* ih, int style)
{
  int weight = IUP_SSM(ih->handle, SCI_STYLEGETWEIGHT, style, 0);
  char* str = iupStrGetMemory(15);

  if(style == IUP_INVALID_ID)
    style = 0;  /* Lexer style default */

  weight = IUP_SSM(ih->handle, SCI_STYLEGETWEIGHT, style, 0);

  sprintf(str, "%d", weight);

  return str;
}

int iupScintillaSetWeightStyleAttrib(Ihandle* ih, int style, const char* value)
{
  int weight = 0;

  if(style == IUP_INVALID_ID)
    style = 0;  /* Lexer style default */

  if (iupStrEqualNoCase(value, "NORMAL"))
    weight = 400;
  else if (iupStrEqualNoCase(value, "SEMIBOLD"))
    weight = 600;
  else if (iupStrEqualNoCase(value, "BOLD"))
    weight = 700;
  else
  {
    iupStrToInt(value, &weight);
    if(weight < 1)
      weight = 1;
    if(weight > 999)
      weight = 999;
  }

  IUP_SSM(ih->handle, SCI_STYLESETWEIGHT, style, 0);

  return 0;
}

char* iupScintillaGetUnderlineStyleAttrib(Ihandle* ih, int style)
{
  if(style == IUP_INVALID_ID)
    style = 0;  /* Lexer style default */

  if(IUP_SSM(ih->handle, SCI_STYLEGETUNDERLINE, style, 0))
    return "YES";
  else
    return "NO";
}

int iupScintillaSetUnderlineStyleAttrib(Ihandle* ih, int style, const char* value)
{
  if(style == IUP_INVALID_ID)
    style = 0;  /* Lexer style default */

  if (iupStrEqualNoCase(value, "YES"))
    IUP_SSM(ih->handle, SCI_STYLESETUNDERLINE, style, 1);
  else
    IUP_SSM(ih->handle, SCI_STYLESETITALIC, style, 0);

  return 0;
}

char* iupScintillaGetItalicStyleAttrib(Ihandle* ih, int style)
{
  if(style == IUP_INVALID_ID)
    style = 0;  /* Lexer style default */

  if(IUP_SSM(ih->handle, SCI_STYLEGETITALIC, style, 0))
    return "YES";
  else
    return "NO";
}

int iupScintillaSetItalicStyleAttrib(Ihandle* ih, int style, const char* value)
{
  if(style == IUP_INVALID_ID)
    style = 0;  /* Lexer style default */

  if (iupStrEqualNoCase(value, "YES"))
    IUP_SSM(ih->handle, SCI_STYLESETITALIC, style, 1);
  else
    IUP_SSM(ih->handle, SCI_STYLESETITALIC, style, 0);

  return 0;
}

char* iupScintillaGetBoldStyleAttrib(Ihandle* ih, int style)
{
  if(style == IUP_INVALID_ID)
    style = 0;  /* Lexer style default */

  if(IUP_SSM(ih->handle, SCI_STYLEGETBOLD, style, 0))
    return "YES";
  else
    return "NO";
}

int iupScintillaSetBoldStyleAttrib(Ihandle* ih, int style, const char* value)
{
  if(style == IUP_INVALID_ID)
    style = 0;  /* Lexer style default */

  if (iupStrEqualNoCase(value, "YES"))
    IUP_SSM(ih->handle, SCI_STYLESETBOLD, style, 1);
  else
    IUP_SSM(ih->handle, SCI_STYLESETBOLD, style, 0);

  return 0;
}

char* iupScintillaGetFgColorStyleAttrib(Ihandle* ih, int style)
{
  long color;
  char* str = iupStrGetMemory(15);
  unsigned char r, g, b;

  if(style == IUP_INVALID_ID)
    style = 0;  /* Lexer style default */

  color = IUP_SSM(ih->handle, SCI_STYLEGETFORE, style, 0);
  IupScintillaDecodeColor(color, &r, &g, &b);
  sprintf(str, "%d %d %d", r, g, b);

  return str;
}

int iupScintillaSetFgColorStyleAttrib(Ihandle* ih, int style, const char* value)
{
  unsigned char r, g, b;

  if (!iupStrToRGB(value, &r, &g, &b))
    return 0;

  if(style == IUP_INVALID_ID)
    style = 0;  /* Lexer style default */

  IUP_SSM(ih->handle, SCI_STYLESETFORE, style, IupScintillaEncodeColor(r, g, b));

  return 0;
}

char* iupScintillaGetBgColorStyleAttrib(Ihandle* ih, int style)
{
  long color;
  char* str = iupStrGetMemory(70);
  unsigned char r, g, b;

  if(style == IUP_INVALID_ID)
    style = 0;  /* Lexer style default */

  color = IUP_SSM(ih->handle, SCI_STYLEGETBACK, style, 0);
  IupScintillaDecodeColor(color, &r, &g, &b);
  sprintf(str, "%d %d %d", r, g, b);

  return str;
}

int iupScintillaSetBgColorStyleAttrib(Ihandle* ih, int style, const char* value)
{
  unsigned char r, g, b;

  if (!iupStrToRGB(value, &r, &g, &b))
    return 0;

  if(style == IUP_INVALID_ID)
    style = 0;  /* Lexer style default */

  IUP_SSM(ih->handle, SCI_STYLESETBACK, style, IupScintillaEncodeColor(r, g, b));

  return 0;
}

int iupScintillaSetClearAllStyleAttrib(Ihandle* ih, const char* value)
{
  (void)value;

  IUP_SSM(ih->handle, SCI_STYLECLEARALL, 0, 0);
  return 0;
}

int iupScintillaSetResetDefaultStyleAttrib(Ihandle* ih, const char* value)
{
  (void)value;

  IUP_SSM(ih->handle, SCI_STYLERESETDEFAULT, 0, 0);
  return 0;
}
