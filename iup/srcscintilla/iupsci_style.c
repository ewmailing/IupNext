/** \file
 * \brief Scintilla control: Style Definition
 *
 * See Copyright Notice in "iup.h"
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#undef SCI_NAMESPACE
#include <Scintilla.h>

#include "iup.h"

#include "iup_object.h"
#include "iup_attrib.h"
#include "iup_str.h"

#include "iupsci_style.h"
#include "iupsci.h"


/***** STYLE DEFINITION *****
Attributes not implement:
SCI_STYLESETCHANGEABLE(int styleNumber, bool changeable)
SCI_STYLEGETCHANGEABLE(int styleNumber)  
*/

char* iupScintillaGetCaseStyleAttrib(Ihandle* ih, int style)
{
  int caseSty;

  if(style == IUP_INVALID_ID)
    style = 0;  /* Lexer style default */
  
  caseSty = iupScintillaSendMessage(ih, SCI_STYLEGETCASE, style, 0);
  
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
    iupScintillaSendMessage(ih, SCI_STYLESETCASE, style, SC_CASE_UPPER);
  else if (iupStrEqualNoCase(value, "LOWERCASE"))
    iupScintillaSendMessage(ih, SCI_STYLESETCASE, style, SC_CASE_LOWER);
  else
    iupScintillaSendMessage(ih, SCI_STYLESETCASE, style, SC_CASE_MIXED);  /* default */

  return 0;
}

char* iupScintillaGetVisibleStyleAttrib(Ihandle* ih, int style)
{
  if(style == IUP_INVALID_ID)
    style = 0;  /* Lexer style default */

  if(iupScintillaSendMessage(ih, SCI_STYLEGETVISIBLE, style, 0))
    return "YES";
  else
    return "NO";
}

int iupScintillaSetVisibleStyleAttrib(Ihandle* ih, int style, const char* value)
{
  if(style == IUP_INVALID_ID)
    style = 0;  /* Lexer style default */

  if (iupStrBoolean(value))
    iupScintillaSendMessage(ih, SCI_STYLESETVISIBLE, style, 1);
  else
    iupScintillaSendMessage(ih, SCI_STYLESETVISIBLE, style, 0);

  return 0;
}

char* iupScintillaGetHotSpotStyleAttrib(Ihandle* ih, int style)
{
  if(style == IUP_INVALID_ID)
    style = 0;  /* Lexer style default */

  if(iupScintillaSendMessage(ih, SCI_STYLEGETHOTSPOT, style, 0))
    return "YES";
  else
    return "NO";
}

int iupScintillaSetHotSpotStyleAttrib(Ihandle* ih, int style, const char* value)
{
  if(style == IUP_INVALID_ID)
    style = 0;  /* Lexer style default */

  if (iupStrBoolean(value))
    iupScintillaSendMessage(ih, SCI_STYLESETHOTSPOT, style, 1);
  else
    iupScintillaSendMessage(ih, SCI_STYLESETHOTSPOT, style, 0);

  return 0;
}

char* iupScintillaGetCharSetStyleAttrib(Ihandle* ih, int style)
{
  int charset;
  
  if(style == IUP_INVALID_ID)
    style = 0;  /* Lexer style default */
  
  charset = iupScintillaSendMessage(ih, SCI_STYLEGETCHARACTERSET, style, 0);

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
    iupScintillaSendMessage(ih, SCI_STYLESETCHARACTERSET, style, SC_CHARSET_EASTEUROPE);
  else if (iupStrEqualNoCase(value, "RUSSIAN"))
    iupScintillaSendMessage(ih, SCI_STYLESETCHARACTERSET, style, SC_CHARSET_RUSSIAN);
  else if (iupStrEqualNoCase(value, "GB2312"))  /* Chinese charset */
    iupScintillaSendMessage(ih, SCI_STYLESETCHARACTERSET, style, SC_CHARSET_GB2312);
  else if (iupStrEqualNoCase(value, "HANGUL"))  /* Korean charset */
    iupScintillaSendMessage(ih, SCI_STYLESETCHARACTERSET, style, SC_CHARSET_HANGUL);
  else if (iupStrEqualNoCase(value, "SHIFTJIS"))  /* Japanese charset */
    iupScintillaSendMessage(ih, SCI_STYLESETCHARACTERSET, style, SC_CHARSET_SHIFTJIS);
  else
    iupScintillaSendMessage(ih, SCI_STYLESETCHARACTERSET, style, SC_CHARSET_ANSI);  /* default */

  return 0;
}

char* iupScintillaGetEolFilledStyleAttrib(Ihandle* ih, int style)
{
  if(style == IUP_INVALID_ID)
    style = 0;  /* Lexer style default */

  if(iupScintillaSendMessage(ih, SCI_STYLEGETEOLFILLED, style, 0))
    return "YES";
  else
    return "NO";
}

int iupScintillaSetEolFilledStyleAttrib(Ihandle* ih, int style, const char* value)
{
  if(style == IUP_INVALID_ID)
    style = 0;  /* Lexer style default */

  if (iupStrBoolean(value))
    iupScintillaSendMessage(ih, SCI_STYLESETEOLFILLED, style, 1);
  else
    iupScintillaSendMessage(ih, SCI_STYLESETEOLFILLED, style, 0);

  return 0;
}

char* iupScintillaGetFontSizeFracStyleAttrib(Ihandle* ih, int style)
{
  int size;

  if(style == IUP_INVALID_ID)
    style = 0;  /* Lexer style default */

  size = iupScintillaSendMessage(ih, SCI_STYLESETSIZEFRACTIONAL, style, 0);

  return iupStrReturnFloat((float)size / SC_FONT_SIZE_MULTIPLIER);
}

int iupScintillaSetFontSizeFracStyleAttrib(Ihandle* ih, int style, const char* value)
{
  float size;

  if(style == IUP_INVALID_ID)
    style = 0;  /* Lexer style default */

  iupStrToFloat(value, &size);

  iupScintillaSendMessage(ih, SCI_STYLESETSIZEFRACTIONAL, style, (int)(size*SC_FONT_SIZE_MULTIPLIER));

  return 0;
}

char* iupScintillaGetFontSizeStyleAttrib(Ihandle* ih, int style)
{
  int size;

  if(style == IUP_INVALID_ID)
    style = 0;  /* Lexer style default */

  size = iupScintillaSendMessage(ih, SCI_STYLEGETSIZE, style, 0);
  return iupStrReturnInt(size);
}

int iupScintillaSetFontSizeStyleAttrib(Ihandle* ih, int style, const char* value)
{
  int size;

  if(style == IUP_INVALID_ID)
    style = 0;  /* Lexer style default */

  iupStrToInt(value, &size);

  iupScintillaSendMessage(ih, SCI_STYLESETSIZE, style, size);

  return 0;
}

char* iupScintillaGetFontStyleAttrib(Ihandle* ih, int style)
{
  char* str = iupStrGetMemory(15);

  if(style == IUP_INVALID_ID)
    style = 0;  /* Lexer style default */

  iupScintillaSendMessage(ih, SCI_STYLEGETFONT, style, (sptr_t)str);

  return str;
}

int iupScintillaSetFontStyleAttrib(Ihandle* ih, int style, const char* value)
{
  if(style == IUP_INVALID_ID)
    style = 0;  /* Lexer style default */

  iupScintillaSendMessage(ih, SCI_STYLESETFONT, style, (sptr_t)value);

  return 0;
}

char* iupScintillaGetWeightStyleAttrib(Ihandle* ih, int style)
{
  int weight = iupScintillaSendMessage(ih, SCI_STYLEGETWEIGHT, style, 0);

  if(style == IUP_INVALID_ID)
    style = 0;  /* Lexer style default */

  weight = iupScintillaSendMessage(ih, SCI_STYLEGETWEIGHT, style, 0);
  return iupStrReturnInt(weight);
}

int iupScintillaSetWeightStyleAttrib(Ihandle* ih, int style, const char* value)
{
  int weight = 0;

  if(style == IUP_INVALID_ID)
    style = 0;  /* Lexer style default */

  if (!value || value[0]==0 || iupStrEqualNoCase(value, "NORMAL"))
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

  iupScintillaSendMessage(ih, SCI_STYLESETWEIGHT, style, 0);

  return 0;
}

char* iupScintillaGetUnderlineStyleAttrib(Ihandle* ih, int style)
{
  if(style == IUP_INVALID_ID)
    style = 0;  /* Lexer style default */

  if(iupScintillaSendMessage(ih, SCI_STYLEGETUNDERLINE, style, 0))
    return "YES";
  else
    return "NO";
}

int iupScintillaSetUnderlineStyleAttrib(Ihandle* ih, int style, const char* value)
{
  if(style == IUP_INVALID_ID)
    style = 0;  /* Lexer style default */

  if (iupStrBoolean(value))
    iupScintillaSendMessage(ih, SCI_STYLESETUNDERLINE, style, 1);
  else
    iupScintillaSendMessage(ih, SCI_STYLESETUNDERLINE, style, 0);

  return 0;
}

char* iupScintillaGetItalicStyleAttrib(Ihandle* ih, int style)
{
  if(style == IUP_INVALID_ID)
    style = 0;  /* Lexer style default */

  if(iupScintillaSendMessage(ih, SCI_STYLEGETITALIC, style, 0))
    return "YES";
  else
    return "NO";
}

int iupScintillaSetItalicStyleAttrib(Ihandle* ih, int style, const char* value)
{
  if(style == IUP_INVALID_ID)
    style = 0;  /* Lexer style default */

  if (iupStrBoolean(value))
    iupScintillaSendMessage(ih, SCI_STYLESETITALIC, style, 1);
  else
    iupScintillaSendMessage(ih, SCI_STYLESETITALIC, style, 0);

  return 0;
}

char* iupScintillaGetBoldStyleAttrib(Ihandle* ih, int style)
{
  if(style == IUP_INVALID_ID)
    style = 0;  /* Lexer style default */

  if(iupScintillaSendMessage(ih, SCI_STYLEGETBOLD, style, 0))
    return "YES";
  else
    return "NO";
}

int iupScintillaSetBoldStyleAttrib(Ihandle* ih, int style, const char* value)
{
  if(style == IUP_INVALID_ID)
    style = 0;  /* Lexer style default */

  if (iupStrBoolean(value))
    iupScintillaSendMessage(ih, SCI_STYLESETBOLD, style, 1);
  else
    iupScintillaSendMessage(ih, SCI_STYLESETBOLD, style, 0);

  return 0;
}

char* iupScintillaGetFgColorStyleAttrib(Ihandle* ih, int style)
{
  long color;
  unsigned char r, g, b;

  if(style == IUP_INVALID_ID)
    style = 0;  /* Lexer style default */

  color = iupScintillaSendMessage(ih, SCI_STYLEGETFORE, style, 0);
  iupScintillaDecodeColor(color, &r, &g, &b);
  return iupStrReturnRGB(r, g, b);
}

int iupScintillaSetFgColorStyleAttrib(Ihandle* ih, int style, const char* value)
{
  unsigned char r, g, b;

  if (!iupStrToRGB(value, &r, &g, &b))
    return 0;

  if(style == IUP_INVALID_ID)
    style = 0;  /* Lexer style default */

  iupScintillaSendMessage(ih, SCI_STYLESETFORE, style, iupScintillaEncodeColor(r, g, b));

  return 0;
}

char* iupScintillaGetBgColorStyleAttrib(Ihandle* ih, int style)
{
  long color;
  unsigned char r, g, b;

  if(style == IUP_INVALID_ID)
    style = 0;  /* Lexer style default */

  color = iupScintillaSendMessage(ih, SCI_STYLEGETBACK, style, 0);
  iupScintillaDecodeColor(color, &r, &g, &b);
  return iupStrReturnRGB(r, g, b);
}

int iupScintillaSetBgColorStyleAttrib(Ihandle* ih, int style, const char* value)
{
  unsigned char r, g, b;

  if (!iupStrToRGB(value, &r, &g, &b))
    return 0;

  if(style == IUP_INVALID_ID)
    style = 0;  /* Lexer style default */

  iupScintillaSendMessage(ih, SCI_STYLESETBACK, style, iupScintillaEncodeColor(r, g, b));

  return 0;
}

int iupScintillaSetClearAllStyleAttrib(Ihandle* ih, const char* value)
{
  (void)value;

  iupScintillaSendMessage(ih, SCI_STYLECLEARALL, 0, 0);
  return 0;
}

int iupScintillaSetResetDefaultStyleAttrib(Ihandle* ih, const char* value)
{
  (void)value;

  iupScintillaSendMessage(ih, SCI_STYLERESETDEFAULT, 0, 0);
  return 0;
}

int iupScintillaSetStartStylingAttrib(Ihandle *ih, const char *value)
{
  int pos;
  if (iupStrToInt(value, &pos))
    iupScintillaSendMessage(ih, SCI_STARTSTYLING, pos, 0x1f);  /* mask=31 */

  return 0;
}

int iupScintillaSetStylingAttrib(Ihandle* ih, int style, const char* value)
{
  int length;
  if (iupStrToInt(value, &length))
    iupScintillaSendMessage(ih, SCI_SETSTYLING, length, style);
  return 0;
}
