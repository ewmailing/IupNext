/*
* IupScintilla component
*
* Description : A source code editing component, derived from Scintilla (http://www.scintilla.org/)
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
#include "iupcbs.h"
#include "iup_scintilla.h"

#include "iup_class.h"
#include "iup_object.h"
#include "iup_attrib.h"
#include "iup_str.h"
#include "iup_drv.h"
#include "iup_drvfont.h"
#include "iup_register.h"
#include "iup_stdcontrols.h"
#include "iup_layout.h"

#ifdef GTK
#include "iupgtk_drv.h"
#else
#include "iupwin_drv.h"
#endif

#ifdef GTK
#define IUP_SSM(sci, m, w, l) scintilla_send_message((ScintillaObject*)sci, m, w, l)
#else
#define IUP_SSM(hwnd, m, w, l) SendMessage(hwnd, m, w, l)
#endif

struct _IcontrolData
{
#ifdef GTK
  GtkWidget *editor;
#endif
  int sb;  // scrollbar
  // attributes
};

/***** IUP UTILS *****/
static long iScintillaEncodeColor(unsigned char r, unsigned char g, unsigned char b)
{
  return (((unsigned long)r) << 16) |
         (((unsigned long)g) <<  8) |
         (((unsigned long)b) <<  0);
}

static void iScintillaDecodeColor(long color, unsigned char *r, unsigned char *g, unsigned char *b)
{
  *r = (unsigned char)(((color) >> 16) & 0xFF);
  *g = (unsigned char)(((color) >>  8) & 0xFF);
  *b = (unsigned char)(((color) >>  0) & 0xFF);
}

static char* iScintillaGetScrollbarAttrib(Ihandle* ih)
{
  if (ih->data->sb == (IUP_SB_HORIZ | IUP_SB_VERT))
    return "YES";
  if (ih->data->sb & IUP_SB_HORIZ)
    return "HORIZONTAL";
  if (ih->data->sb & IUP_SB_VERT)
    return "VERTICAL";
  
  return "NO";
}

static int iScintillaSetScrollbarAttrib(Ihandle* ih, const char* value)
{
  if (!value)
    value = "YES";    /* default is YES */

  if (iupStrEqualNoCase(value, "YES"))
    ih->data->sb = IUP_SB_HORIZ | IUP_SB_VERT;
  else if (iupStrEqualNoCase(value, "HORIZONTAL"))
    ih->data->sb = IUP_SB_HORIZ;
  else if (iupStrEqualNoCase(value, "VERTICAL"))
    ih->data->sb = IUP_SB_VERT;
  else
    ih->data->sb = IUP_SB_NONE;

  return 1;
}

/***** FOLDING *****
SCI_VISIBLEFROMDOCLINE(int docLine)
SCI_DOCLINEFROMVISIBLE(int displayLine)
SCI_SHOWLINES(int lineStart, int lineEnd)
SCI_HIDELINES(int lineStart, int lineEnd)
SCI_GETLINEVISIBLE(int line)
SCI_GETALLLINESVISIBLE
SCI_GETLASTCHILD(int line, int level)
SCI_GETFOLDPARENT(int line)
SCI_SETFOLDEXPANDED(int line, bool expanded)
SCI_GETFOLDEXPANDED(int line)
SCI_CONTRACTEDFOLDNEXT(int lineStart)
SCI_ENSUREVISIBLE(int line)
SCI_ENSUREVISIBLEENFORCEPOLICY(int line)
*/

static int iScintillaSetFoldFlagsAttrib(Ihandle* ih, const char* value)
{
  if (iupStrEqualNoCase(value, "LEVELNUMBERS"))
    IUP_SSM(ih->handle, SCI_SETFOLDFLAGS, SC_FOLDFLAG_LEVELNUMBERS, 0);
  else if (iupStrEqualNoCase(value, "LINEBEFORE_EXPANDED"))
    IUP_SSM(ih->handle, SCI_SETFOLDFLAGS, SC_FOLDFLAG_LINEBEFORE_EXPANDED, 0);
  else if (iupStrEqualNoCase(value, "LINEBEFORE_CONTRACTED"))
    IUP_SSM(ih->handle, SCI_SETFOLDFLAGS, SC_FOLDFLAG_LINEBEFORE_CONTRACTED, 0);
  else if (iupStrEqualNoCase(value, "LINEAFTER_EXPANDED"))
    IUP_SSM(ih->handle, SCI_SETFOLDFLAGS, SC_FOLDFLAG_LINEAFTER_EXPANDED, 0);
  else  /* LINEAFTER_CONTRACTED */
    IUP_SSM(ih->handle, SCI_SETFOLDFLAGS, SC_FOLDFLAG_LINEAFTER_CONTRACTED, 0);

  return 1;
}

static char* iScintillaGetFoldLevelAttrib(Ihandle* ih, int line)
{
  int level = IUP_SSM(ih->handle, SCI_GETFOLDLEVEL, line, 0);

  if (level & SC_FOLDLEVELWHITEFLAG)
    return "WHITEFLAG";
  else if (level & SC_FOLDLEVELHEADERFLAG)
    return "HEADERFLAG";
  else if (level & SC_FOLDLEVELNUMBERMASK)
    return "NUMBERMASK";
  else  /* SC_FOLDLEVELBASE */
    return "BASE";
}

static int iScintillaSetFoldLevelAttrib(Ihandle* ih, int line, const char* value)
{
  if (iupStrEqualNoCase(value, "WHITEFLAG"))
    IUP_SSM(ih->handle, SCI_SETFOLDLEVEL, line, SC_FOLDLEVELWHITEFLAG);
  else if (iupStrEqualNoCase(value, "HEADERFLAG"))
    IUP_SSM(ih->handle, SCI_SETFOLDLEVEL, line, SC_FOLDLEVELHEADERFLAG);
  else if (iupStrEqualNoCase(value, "NUMBERMASK"))
    IUP_SSM(ih->handle, SCI_SETFOLDLEVEL, line, SC_FOLDLEVELNUMBERMASK);
  else  /* BASE */
    IUP_SSM(ih->handle, SCI_SETFOLDLEVEL, line, SC_FOLDLEVELBASE);

  return 1;
}

static int iScintillaSetToggleFoldAttrib(Ihandle* ih, const char* value)
{
  int line = atoi(value);
  int level = IUP_SSM(ih->handle, SCI_GETFOLDLEVEL, line, 0);

  if (level & SC_FOLDLEVELHEADERFLAG)
  {
    IUP_SSM(ih->handle, SCI_TOGGLEFOLD, line, 0);
    return 1;
  }

  return 0;
}

/***** MARGIN *****
SCI_SETMARGINCURSORN(int margin, int cursor)
SCI_GETMARGINCURSORN(int margin)
SCI_SETMARGINLEFT(<unused>, int pixels)
SCI_GETMARGINLEFT
SCI_SETMARGINRIGHT(<unused>, int pixels)
SCI_GETMARGINRIGHT
SCI_SETFOLDMARGINCOLOUR(bool useSetting, int colour)
SCI_SETFOLDMARGINHICOLOUR(bool useSetting, int colour)
SCI_MARGINSETTEXT(int line, char *text)
SCI_MARGINGETTEXT(int line, char *text)
SCI_MARGINSETSTYLE(int line, int style)
SCI_MARGINGETSTYLE(int line)
SCI_MARGINSETSTYLES(int line, char *styles)
SCI_MARGINGETSTYLES(int line, char *styles)
SCI_MARGINTEXTCLEARALL
SCI_MARGINSETSTYLEOFFSET(int style)
SCI_MARGINGETSTYLEOFFSET
SCI_SETMARGINOPTIONS(int marginOptions)
SCI_GETMARGINOPTIONS
*/

static char* iScintillaGetMarginTypeNAttrib(Ihandle* ih, int margin)
{
  int type = IUP_SSM(ih->handle, SCI_GETMARGINTYPEN, margin, 0);

  if (type == SC_MARGIN_NUMBER)
    return "MARGIN_NUMBER";
  if (type == SC_MARGIN_TEXT)
    return "MARGIN_TEXT";
  if (type == SC_MARGIN_RTEXT)
    return "MARGIN_RTEXT";
  if (type == SC_MARGIN_BACK)
    return "MARGIN_BACK";
  if (type == SC_MARGIN_FORE)
    return "MARGIN_FORE";

  return "MARGIN_SYMBOL";
}

static int iScintillaSetMarginTypeNAttrib(Ihandle* ih, int margin, const char* value)
{
  if (iupStrEqualNoCase(value, "MARGIN_NUMBER"))
    IUP_SSM(ih->handle, SCI_SETMARGINTYPEN, margin, SC_MARGIN_NUMBER);
  else if (iupStrEqualNoCase(value, "MARGIN_TEXT"))
    IUP_SSM(ih->handle, SCI_SETMARGINTYPEN, margin, SC_MARGIN_TEXT);
  else if (iupStrEqualNoCase(value, "MARGIN_RTEXT"))
    IUP_SSM(ih->handle, SCI_SETMARGINTYPEN, margin, SC_MARGIN_RTEXT);
  else if (iupStrEqualNoCase(value, "MARGIN_BACK"))
    IUP_SSM(ih->handle, SCI_SETMARGINTYPEN, margin, SC_MARGIN_BACK);
  else if (iupStrEqualNoCase(value, "MARGIN_FORE"))
    IUP_SSM(ih->handle, SCI_SETMARGINTYPEN, margin, SC_MARGIN_FORE);
  else  /* MARGIN_SYMBOL */
    IUP_SSM(ih->handle, SCI_SETMARGINTYPEN, margin, SC_MARGIN_SYMBOL);

  return 1;
}

static char* iScintillaGetMarginWidthNAttrib(Ihandle* ih, int margin)
{
  int pixelWidth = IUP_SSM(ih->handle, SCI_GETMARGINWIDTHN, margin, 0);
  char* str = iupStrGetMemory(15);

  sprintf(str, "%d", pixelWidth);

  return str;
}

static int iScintillaSetMarginWidthNAttrib(Ihandle* ih, int margin, const char* value)
{
  int pixelWidth = atoi(value);
  
  if(pixelWidth < 1)
    pixelWidth = 16;

  IUP_SSM(ih->handle, SCI_SETMARGINWIDTHN, margin, pixelWidth);

  return 1;
}

static char* iScintillaGetMarginMaskNAttrib(Ihandle* ih, int margin)
{
  if(IUP_SSM(ih->handle, SCI_GETMARGINMASKN, margin, 0) == SC_MASK_FOLDERS)
    return "MASK_FOLDERS";
  else
    return "NO_MASK_FOLDERS";
}

static int iScintillaSetMarginMaskNAttrib(Ihandle* ih, int margin, const char* value)
{
  if (iupStrEqualNoCase(value, "MASK_FOLDERS"))
    IUP_SSM(ih->handle, SCI_SETMARGINMASKN, margin, SC_MASK_FOLDERS);
  else
    IUP_SSM(ih->handle, SCI_SETMARGINMASKN, margin, ~SC_MASK_FOLDERS);

  return 1;
}

static char* iScintillaGetMarginSensitiveNAttrib(Ihandle* ih, int margin)
{
  if(IUP_SSM(ih->handle, SCI_SETMARGINSENSITIVEN, margin, 0))
    return "YES";
  else
    return "NO";
}

static int iScintillaSetMarginSensitiveNAttrib(Ihandle* ih, int margin, const char* value)
{
  if (iupStrEqualNoCase(value, "YES"))
    IUP_SSM(ih->handle, SCI_SETMARGINSENSITIVEN, margin, 1);
  else
    IUP_SSM(ih->handle, SCI_SETMARGINSENSITIVEN, margin, 0);

  return 1;
}

/***** TEXT RETRIEVAL AND MODIFICATION *****
Attributes not implement yet:
SCI_SETSAVEPOINT
SCI_ALLOCATE(int bytes, <unused>)
SCI_ADDSTYLEDTEXT(int length, cell *s)
SCI_GETSTYLEDTEXT(<unused>, Sci_TextRange *tr)
SCI_GETSTYLEAT(int position)
SCI_SETSTYLEBITS(int bits)
SCI_GETSTYLEBITS
SCI_RELEASEALLEXTENDEDSTYLES
SCI_ALLOCATEEXTENDEDSTYLES(int numberStyles)
SCI_TARGETASUTF8(<unused>, char *s)  // only GTK
SCI_ENCODEDFROMUTF8(const char *utf8, char *encoded)  // only GTK
SCI_SETLENGTHFORENCODE(int bytes)  // only GTK
*/
static char* iScintillaGetTextAttrib(Ihandle* ih, int len)
{
  char* str = iupStrGetMemory(len);
  IUP_SSM(ih->handle, SCI_GETTEXT, len, (sptr_t)str);
  return str;
}

static int iScintillaSetTextAttrib(Ihandle* ih, int len, const char* value)
{
  (void)len;
  IUP_SSM(ih->handle, SCI_SETTEXT, 0, (sptr_t)value);
  return 1;
}

static char* iScintillaGetLineAttrib(Ihandle* ih, int line)
{
  int len = IUP_SSM(ih->handle, SCI_LINELENGTH, line, 0);
  char* str = iupStrGetMemory(len); 
  IUP_SSM(ih->handle, SCI_GETLINE, line, (sptr_t)str);
  return str;
}

static int iScintillaSetReplaceSelAttrib(Ihandle* ih, int len, const char* value)
{
  (void)len;
  IUP_SSM(ih->handle, SCI_REPLACESEL, 0, (sptr_t)value);
  return 1;
}

static char* iScintillaGetReadOnlyAttrib(Ihandle* ih)
{
  if(IUP_SSM(ih->handle, SCI_GETREADONLY, 0, 0))
    return "YES";
  else
    return "NO";
}

static int iScintillaSetReadOnlyAttrib(Ihandle* ih, const char* value)
{
  if (iupStrEqualNoCase(value, "YES"))
    IUP_SSM(ih->handle, SCI_SETREADONLY, 1, 0);
  else
    IUP_SSM(ih->handle, SCI_SETREADONLY, 0, 0);

  return 1;
}

static char* iScintillaGetTextRangeAttrib(Ihandle* ih, int min, int max)
{
  Sci_TextRange tr;
  int range;
      
  tr.chrg.cpMin = min;
  tr.chrg.cpMax = max;

  range = max - min + 1;
  if(range <= 0)
    range = IUP_SSM(ih->handle, SCI_GETLENGTH, 0, 0);

  tr.lpstrText = iupStrGetMemory(range);
      
  IUP_SSM(ih->handle, SCI_GETTEXTRANGE, 0, (sptr_t)&tr);
      
  return tr.lpstrText;
}

static int iScintillaSetAddTextAttrib(Ihandle* ih, int len, const char* value)
{
  IUP_SSM(ih->handle, SCI_ADDTEXT, len, (sptr_t)value);
  return 1;
}

static int iScintillaSetAppendTextAttrib(Ihandle* ih, int len, const char* value)
{
  IUP_SSM(ih->handle, SCI_APPENDTEXT, len, (sptr_t)value);
  return 1;
}

static int iScintillaSetInsertTextAttrib(Ihandle* ih, int pos, const char* value)
{
  IUP_SSM(ih->handle, SCI_INSERTTEXT, pos, (sptr_t)value);
  return 1;
}

static int iScintillaSetClearAllAttrib(Ihandle* ih, const char* value)
{
  (void)value;
  if(!ih->handle)
    return 0;

  IUP_SSM(ih->handle, SCI_CLEARALL, 0, 0);
  return 1;
}

static int iScintillaSetClearDocumentStyleAttrib(Ihandle* ih, const char* value)
{
  (void)value;
  if(!ih->handle)
    return 0;

  IUP_SSM(ih->handle, SCI_CLEARDOCUMENTSTYLE, 0, 0);
  return 1;
}

static int iScintillaSetDeleteRangeAttrib(Ihandle* ih, const char* value)
{
  int pos, len;
  iupStrToIntInt(value, &pos, &len, ',');

  IUP_SSM(ih->handle, SCI_DELETERANGE, pos, len);
  return 1;
}

static char* iScintillaGetCharAtAttrib(Ihandle* ih, int pos)
{
  return (char*)IUP_SSM(ih->handle, SCI_GETCHARAT, pos, 0);
}

/***** STYLE DEFINITION *****
Attributes not implement yet:
SCI_STYLESETCHANGEABLE(int styleNumber, bool changeable)
SCI_STYLEGETCHANGEABLE(int styleNumber)  
*/
/*
Lexer = CPP number styles
SCE_C_DEFAULT 0						        SCE_C_COMMENT 1				      SCE_C_COMMENTLINE 2
SCE_C_COMMENTDOC 3					      SCE_C_NUMBER 4				      SCE_C_WORD 5
SCE_C_STRING 6						        SCE_C_CHARACTER 7			      SCE_C_UUID 8
SCE_C_PREPROCESSOR 9				      SCE_C_OPERATOR 10			      SCE_C_IDENTIFIER 11
SCE_C_STRINGEOL 12					      SCE_C_VERBATIM 13			      SCE_C_REGEX 14
SCE_C_COMMENTLINEDOC 15				    SCE_C_WORD2 16				      SCE_C_COMMENTDOCKEYWORD 17
SCE_C_COMMENTDOCKEYWORDERROR 18		SCE_C_GLOBALCLASS 19		    SCE_C_STRINGRAW 20
SCE_C_TRIPLEVERBATIM 21				    SCE_C_HASHQUOTEDSTRING 22	  SCE_C_PREPROCESSORCOMMENT 23

Lexer = LUA number styles
SCE_LUA_DEFAULT 0         SCE_LUA_COMMENT 1			SCE_LUA_COMMENTLINE 2
SCE_LUA_COMMENTDOC 3			SCE_LUA_NUMBER 4			SCE_LUA_WORD 5
SCE_LUA_STRING 6					SCE_LUA_CHARACTER 7		SCE_LUA_LITERALSTRING 8
SCE_LUA_PREPROCESSOR 9		SCE_LUA_OPERATOR 10		SCE_LUA_IDENTIFIER 11
SCE_LUA_STRINGEOL 12			SCE_LUA_WORD2 13			SCE_LUA_WORD3 14
SCE_LUA_WORD4 15					SCE_LUA_WORD5 16			SCE_LUA_WORD6 17
SCE_LUA_WORD7 18					SCE_LUA_WORD8 19			SCE_LUA_LABEL 20
*/
static char* iScintillaGetCaseStyleAttrib(Ihandle* ih, int style)
{
  int caseSty = IUP_SSM(ih->handle, SCI_STYLEGETCASE, style, 0);

  if(caseSty == SC_CASE_UPPER)
    return "UPPERCASE";
  else if(caseSty == SC_CASE_LOWER)
    return "LOWERCASE";
  else
    return "SC_CASE_MIXED";
}

static int iScintillaSetCaseStyleAttrib(Ihandle* ih, int style, const char* value)
{
  if (iupStrEqualNoCase(value, "UPPERCASE"))
    IUP_SSM(ih->handle, SCI_STYLESETCASE, style, SC_CASE_UPPER);
  else if (iupStrEqualNoCase(value, "LOWERCASE"))
    IUP_SSM(ih->handle, SCI_STYLESETCASE, style, SC_CASE_LOWER);
  else
    IUP_SSM(ih->handle, SCI_STYLESETCASE, style, SC_CASE_MIXED);  /* default */

  return 1;
}

static char* iScintillaGetVisibleStyleAttrib(Ihandle* ih, int style)
{
  if(IUP_SSM(ih->handle, SCI_STYLEGETVISIBLE, style, 0))
    return "YES";
  else
    return "NO";
}

static int iScintillaSetVisibleStyleAttrib(Ihandle* ih, int style, const char* value)
{
  if (iupStrEqualNoCase(value, "YES"))
    IUP_SSM(ih->handle, SCI_STYLESETVISIBLE, style, 1);
  else
    IUP_SSM(ih->handle, SCI_STYLESETVISIBLE, style, 0);

  return 1;
}

static char* iScintillaGetHotSpotStyleAttrib(Ihandle* ih, int style)
{
  if(IUP_SSM(ih->handle, SCI_STYLEGETHOTSPOT, style, 0))
    return "YES";
  else
    return "NO";
}

static int iScintillaSetHotSpotStyleAttrib(Ihandle* ih, int style, const char* value)
{
  if (iupStrEqualNoCase(value, "YES"))
    IUP_SSM(ih->handle, SCI_STYLESETHOTSPOT, style, 1);
  else
    IUP_SSM(ih->handle, SCI_STYLESETHOTSPOT, style, 0);

  return 1;
}

static char* iScintillaGetCharSetStyleAttrib(Ihandle* ih, int style)
{
  int charset = IUP_SSM(ih->handle, SCI_STYLEGETCHARACTERSET, style, 0);

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

static int iScintillaSetCharSetStyleAttrib(Ihandle* ih, int style, const char* value)
{
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

  return 1;
}

static char* iScintillaGetEolFilledStyleAttrib(Ihandle* ih, int style)
{
  if(IUP_SSM(ih->handle, SCI_STYLEGETEOLFILLED, style, 0))
    return "YES";
  else
    return "NO";
}

static int iScintillaSetEolFilledStyleAttrib(Ihandle* ih, int style, const char* value)
{
  if (iupStrEqualNoCase(value, "YES"))
    IUP_SSM(ih->handle, SCI_STYLESETEOLFILLED, style, 1);
  else
    IUP_SSM(ih->handle, SCI_STYLESETEOLFILLED, style, 0);

  return 1;
}

static char* iScintillaGetFontSizeFracStyleAttrib(Ihandle* ih, int style)
{
  int size = IUP_SSM(ih->handle, SCI_STYLESETSIZEFRACTIONAL, style, 0);
  char* str = iupStrGetMemory(15);

  sprintf(str, "%f", (size / SC_FONT_SIZE_MULTIPLIER));

  return str;
}

static int iScintillaSetFontSizeFracStyleAttrib(Ihandle* ih, int style, const char* value)
{
  double size = atof(value);

  IUP_SSM(ih->handle, SCI_STYLESETSIZEFRACTIONAL, style, (int)(size*SC_FONT_SIZE_MULTIPLIER));

  return 1;
}

static char* iScintillaGetFontSizeStyleAttrib(Ihandle* ih, int style)
{
  int size = IUP_SSM(ih->handle, SCI_STYLEGETSIZE, style, 0);
  char* str = iupStrGetMemory(15);

  sprintf(str, "%d", size);

  return str;
}

static int iScintillaSetFontSizeStyleAttrib(Ihandle* ih, int style, const char* value)
{
  int size = atoi(value);

  IUP_SSM(ih->handle, SCI_STYLESETSIZE, style, size);

  return 1;
}

static char* iScintillaGetFontStyleAttrib(Ihandle* ih, int style)
{
  char* str = iupStrGetMemory(15);

  IUP_SSM(ih->handle, SCI_STYLEGETFONT, style, (sptr_t)str);

  return str;
}

static int iScintillaSetFontStyleAttrib(Ihandle* ih, int style, const char* value)
{
  IUP_SSM(ih->handle, SCI_STYLESETFONT, style, (sptr_t)value);

  return 1;
}

static char* iScintillaGetWeightStyleAttrib(Ihandle* ih, int style)
{
  int weight = IUP_SSM(ih->handle, SCI_STYLEGETWEIGHT, style, 0);
  char* str = iupStrGetMemory(15);

  sprintf(str, "%d", weight);

  return str;
}

static int iScintillaSetWeightStyleAttrib(Ihandle* ih, int style, const char* value)
{
  int weight = 0;

  if (iupStrEqualNoCase(value, "NORMAL"))
    weight = 400;
  else if (iupStrEqualNoCase(value, "SEMIBOLD"))
    weight = 600;
  else if (iupStrEqualNoCase(value, "BOLD"))
    weight = 700;
  else
  {
    weight = atoi(value);
    if(weight < 1)
      weight = 1;
    if(weight > 999)
      weight = 999;
  }

  IUP_SSM(ih->handle, SCI_STYLESETWEIGHT, style, 0);

  return 1;
}

static char* iScintillaGetUnderlineStyleAttrib(Ihandle* ih, int style)
{
  if(IUP_SSM(ih->handle, SCI_STYLEGETUNDERLINE, style, 0))
    return "YES";
  else
    return "NO";
}

static int iScintillaSetUnderlineStyleAttrib(Ihandle* ih, int style, const char* value)
{
  if (iupStrEqualNoCase(value, "YES"))
    IUP_SSM(ih->handle, SCI_STYLESETUNDERLINE, style, 1);
  else
    IUP_SSM(ih->handle, SCI_STYLESETITALIC, style, 0);

  return 1;
}

static char* iScintillaGetItalicStyleAttrib(Ihandle* ih, int style)
{
  if(IUP_SSM(ih->handle, SCI_STYLEGETITALIC, style, 0))
    return "YES";
  else
    return "NO";
}

static int iScintillaSetItalicStyleAttrib(Ihandle* ih, int style, const char* value)
{
  if (iupStrEqualNoCase(value, "YES"))
    IUP_SSM(ih->handle, SCI_STYLESETITALIC, style, 1);
  else
    IUP_SSM(ih->handle, SCI_STYLESETITALIC, style, 0);

  return 1;
}

static char* iScintillaGetBoldStyleAttrib(Ihandle* ih, int style)
{
  if(IUP_SSM(ih->handle, SCI_STYLEGETBOLD, style, 0))
    return "YES";
  else
    return "NO";
}

static int iScintillaSetBoldStyleAttrib(Ihandle* ih, int style, const char* value)
{
  if (iupStrEqualNoCase(value, "YES"))
    IUP_SSM(ih->handle, SCI_STYLESETBOLD, style, 1);
  else
    IUP_SSM(ih->handle, SCI_STYLESETBOLD, style, 0);

  return 1;
}

static char* iScintillaGetFgColorStyleAttrib(Ihandle* ih, int style)
{
  long color = IUP_SSM(ih->handle, SCI_STYLEGETFORE, style, 0);
  char* str = iupStrGetMemory(15);
  unsigned char r, g, b;

  iScintillaDecodeColor(color, &r, &g, &b);
  sprintf(str, "%d %d %d", r, g, b);

  return str;
}

static int iScintillaSetFgColorStyleAttrib(Ihandle* ih, int style, const char* value)
{
  unsigned char r, g, b;

  if (!iupStrToRGB(value, &r, &g, &b))
    return 0;

  IUP_SSM(ih->handle, SCI_STYLESETFORE, style, iScintillaEncodeColor(r, g, b));

  return 1;
}

static char* iScintillaGetBgColorStyleAttrib(Ihandle* ih, int style)
{
  long color = IUP_SSM(ih->handle, SCI_STYLEGETBACK, style, 0);
  char* str = iupStrGetMemory(70);
  unsigned char r, g, b;

  iScintillaDecodeColor(color, &r, &g, &b);
  sprintf(str, "%d %d %d", r, g, b);

  return str;
}

static int iScintillaSetBgColorStyleAttrib(Ihandle* ih, int style, const char* value)
{
  unsigned char r, g, b;

  if (!iupStrToRGB(value, &r, &g, &b))
    return 0;

  IUP_SSM(ih->handle, SCI_STYLESETBACK, style, iScintillaEncodeColor(r, g, b));

  return 1;
}

static int iScintillaSetClearAllStyleAttrib(Ihandle* ih, const char* value)
{
  (void)value;
  if(!ih->handle)
    return 0;

  IUP_SSM(ih->handle, SCI_STYLECLEARALL, 0, 0);
  return 1;
}

static int iScintillaSetResetDefaultAttrib(Ihandle* ih, const char* value)
{
  (void)value;
  if(!ih->handle)
    return 0;

  IUP_SSM(ih->handle, SCI_STYLERESETDEFAULT, 0, 0);
  return 1;
}

/***** MARKERS *****
SCI_MARKERDEFINEPIXMAP(int markerNumber, const char *xpm)
SCI_RGBAIMAGESETWIDTH(int width)
SCI_RGBAIMAGESETHEIGHT(int height)
SCI_RGBAIMAGESETSCALE(int scalePercent)
SCI_MARKERDEFINERGBAIMAGE(int markerNumber, const char *pixels)
SCI_MARKERSYMBOLDEFINED(int markerNumber) 
SCI_MARKERSETFORE(int markerNumber, int colour)
SCI_MARKERSETBACK(int markerNumber, int colour)
SCI_MARKERSETBACKSELECTED(int markerNumber, int colour)
SCI_MARKERENABLEHIGHLIGHT(int enabled)
SCI_MARKERSETALPHA(int markerNumber, int alpha)
SCI_MARKERADD(int line, int markerNumber)
SCI_MARKERADDSET(int line, int markerMask)
SCI_MARKERDELETE(int line, int markerNumber)
SCI_MARKERDELETEALL(int markerNumber)
SCI_MARKERGET(int line)
SCI_MARKERNEXT(int lineStart, int markerMask)
SCI_MARKERPREVIOUS(int lineStart, int markerMask)
SCI_MARKERLINEFROMHANDLE(int handle)
SCI_MARKERDELETEHANDLE(int handle)
*/

static int iScintillaSetMarkerDefineAttrib(Ihandle* ih, const char* value)
{
  char *strNumb = iupStrGetMemory(25);
  char *strSymb = iupStrGetMemory(25);
  int markerNumber, markerSymbol;

  iupStrToStrStr(value, strNumb, strSymb, ',');

  /* Setting marker numbers */
  if (iupStrEqualNoCase(strNumb, "FOLDEREND"))
    markerNumber = SC_MARKNUM_FOLDEREND;
  else if (iupStrEqualNoCase(strNumb, "FOLDEROPENMID"))
    markerNumber = SC_MARKNUM_FOLDEROPENMID;
  else if (iupStrEqualNoCase(strNumb, "FOLDERMIDTAIL"))
    markerNumber = SC_MARKNUM_FOLDERMIDTAIL;
  else if (iupStrEqualNoCase(strNumb, "FOLDERTAIL"))
    markerNumber = SC_MARKNUM_FOLDERTAIL;
  else if (iupStrEqualNoCase(strNumb, "FOLDERSUB"))
    markerNumber = SC_MARKNUM_FOLDERSUB;
  else if (iupStrEqualNoCase(strNumb, "FOLDER"))
    markerNumber = SC_MARKNUM_FOLDER;
  else if (iupStrEqualNoCase(strNumb, "FOLDEROPEN"))
    markerNumber = SC_MARKNUM_FOLDEROPEN;
  else
    return 0;

  /* Setting marker symbols */
  if (iupStrEqualNoCase(strSymb, "CIRCLE"))
    markerSymbol = SC_MARK_CIRCLE;
  else if (iupStrEqualNoCase(strSymb, "ROUNDRECT"))
    markerSymbol = SC_MARK_ROUNDRECT;
  else if (iupStrEqualNoCase(strSymb, "ARROW"))
    markerSymbol = SC_MARK_ARROW;
  else if (iupStrEqualNoCase(strSymb, "SMALLRECT"))
    markerSymbol = SC_MARK_SMALLRECT;
  else if (iupStrEqualNoCase(strSymb, "SHORTARROW"))
    markerSymbol = SC_MARK_SHORTARROW;
  else if (iupStrEqualNoCase(strSymb, "EMPTY"))
    markerSymbol = SC_MARK_EMPTY;
  else if (iupStrEqualNoCase(strSymb, "ARROWDOWN"))
    markerSymbol = SC_MARK_ARROWDOWN;
  else if (iupStrEqualNoCase(strSymb, "MINUS"))
    markerSymbol = SC_MARK_MINUS;
  else if (iupStrEqualNoCase(strSymb, "PLUS"))
    markerSymbol = SC_MARK_PLUS;
  else if (iupStrEqualNoCase(strSymb, "VLINE"))
    markerSymbol = SC_MARK_VLINE;
  else if (iupStrEqualNoCase(strSymb, "LCORNER"))
    markerSymbol = SC_MARK_LCORNER;
  else if (iupStrEqualNoCase(strSymb, "TCORNER"))
    markerSymbol = SC_MARK_TCORNER;
  else if (iupStrEqualNoCase(strSymb, "BOXPLUS"))
    markerSymbol = SC_MARK_BOXPLUS;
  else if (iupStrEqualNoCase(strSymb, "BOXPLUSCONNECTED"))
    markerSymbol = SC_MARK_BOXPLUSCONNECTED;
  else if (iupStrEqualNoCase(strSymb, "BOXMINUS"))
    markerSymbol = SC_MARK_BOXMINUS;
  else if (iupStrEqualNoCase(strSymb, "BOXMINUSCONNECTED"))
    markerSymbol = SC_MARK_BOXMINUSCONNECTED;
  else if (iupStrEqualNoCase(strSymb, "LCORNERCURVE"))
    markerSymbol = SC_MARK_LCORNERCURVE;
  else if (iupStrEqualNoCase(strSymb, "TCORNERCURVE"))
    markerSymbol = SC_MARK_TCORNERCURVE;
  else if (iupStrEqualNoCase(strSymb, "CIRCLEPLUS"))
    markerSymbol = SC_MARK_CIRCLEPLUS;
  else if (iupStrEqualNoCase(strSymb, "CIRCLEPLUSCONNECTED"))
    markerSymbol = SC_MARK_CIRCLEPLUSCONNECTED;
  else if (iupStrEqualNoCase(strSymb, "CIRCLEMINUS"))
    markerSymbol = SC_MARK_CIRCLEMINUS;
  else if (iupStrEqualNoCase(strSymb, "CIRCLEMINUSCONNECTED"))
    markerSymbol = SC_MARK_CIRCLEMINUSCONNECTED;
  else if (iupStrEqualNoCase(strSymb, "BACKGROUND"))
    markerSymbol = SC_MARK_BACKGROUND;
  else if (iupStrEqualNoCase(strSymb, "DOTDOTDOT"))
    markerSymbol = SC_MARK_DOTDOTDOT;
  else if (iupStrEqualNoCase(strSymb, "ARROWS"))
    markerSymbol = SC_MARK_ARROWS;
  else if (iupStrEqualNoCase(strSymb, "PIXMAP"))
    markerSymbol = SC_MARK_PIXMAP;
  else if (iupStrEqualNoCase(strSymb, "FULLRECT"))
    markerSymbol = SC_MARK_FULLRECT;
  else if (iupStrEqualNoCase(strSymb, "LEFTRECT"))
    markerSymbol = SC_MARK_LEFTRECT;
  else if (iupStrEqualNoCase(strSymb, "AVAILABLE"))
    markerSymbol = SC_MARK_AVAILABLE;
  else if (iupStrEqualNoCase(strSymb, "UNDERLINE"))
    markerSymbol = SC_MARK_UNDERLINE;
  else if (iupStrEqualNoCase(strSymb, "RGBAIMAGE"))
    markerSymbol = SC_MARK_RGBAIMAGE;
  else if (iupStrEqualNoCase(strSymb, "CHARACTER"))
    markerSymbol = SC_MARK_CHARACTER;
  else
    return 0;

  IUP_SSM(ih->handle, SCI_MARKERDEFINE, markerNumber, markerSymbol);

  return 1;
}

/***** LEXER *****
Attributes not implement yet:
SCI_SETLEXERLANGUAGE(<unused>, const char *name)
SCI_GETLEXERLANGUAGE(<unused>, char *name)
SCI_LOADLEXERLIBRARY(<unused>, const char *path)
SCI_COLOURISE(int start, int end)
SCI_CHANGELEXERSTATE(int start, int end)
SCI_PROPERTYNAMES(<unused>, char *names)
SCI_PROPERTYTYPE(const char *name)
SCI_DESCRIBEPROPERTY(const char *name, char *description)
SCI_GETPROPERTYEXPANDED(const char *key, char *value)
SCI_GETPROPERTYINT(const char *key, int default)
SCI_DESCRIBEKEYWORDSETS(<unused>, char *descriptions)
SCI_GETSTYLEBITSNEEDED
SCI_GETSUBSTYLEBASES(<unused>, char *styles)
SCI_DISTANCETOSECONDARYSTYLES
SCI_ALLOCATESUBSTYLES(int styleBase, int numberStyles)
SCI_FREESUBSTYLES
SCI_GETSUBSTYLESSTART(int styleBase)
SCI_GETSUBSTYLESLENGTH(int styleBase)
SCI_SETIDENTIFIERS(int style, const char *identifiers)
*/

static char* iScintillaGetLexerAttrib(Ihandle* ih)
{
  int value = IUP_SSM(ih->handle, SCI_GETLEXER, 0, 0);

  if (value == SCLEX_CPP)
    return "CPP";
  else if (value == SCLEX_LUA)
    return "LUA";
  else
    return "NULL";
}

static int iScintillaSetLexerAttrib(Ihandle* ih, const char* value)
{
  if (iupStrEqualNoCase(value, "CPP"))
    IUP_SSM(ih->handle, SCI_SETLEXER, SCLEX_CPP, 0);
  else if (iupStrEqualNoCase(value, "LUA"))
    IUP_SSM(ih->handle, SCI_SETLEXER, SCLEX_LUA, 0);
  else
    IUP_SSM(ih->handle, SCI_SETLEXER, SCLEX_NULL, 0);

  return 1;
}

static int iScintillaSetKeyWordsAttrib(Ihandle* ih, int keyWordSet, const char* value)
{
  /* Note: You can set up to 9 lists of keywords for use by the current lexer */
  if(keyWordSet >= 0 && keyWordSet < 9)
    IUP_SSM(ih->handle, SCI_SETKEYWORDS, keyWordSet, (sptr_t)value);

  return 1;
}

static char* iScintillaGetPropertyAttrib(Ihandle* ih)
{
  char *strKey = iupStrGetMemory(50);
  char *strVal = iupStrGetMemory(50);
  char *str = iupStrGetMemory(101);

  IUP_SSM(ih->handle, SCI_GETPROPERTY, (sptr_t)strKey, (sptr_t)strVal);
  sprintf(str, "%s,%s", strKey, strVal);

  return str;
}

static int iScintillaSetPropertyAttrib(Ihandle* ih, const char* value)
{
  char *strKey = iupStrGetMemory(50);
  char *strVal = iupStrGetMemory(50);

  iupStrToStrStr(value, strKey, strVal, ',');

  IUP_SSM(ih->handle, SCI_SETPROPERTY, (sptr_t)strKey, (sptr_t)strVal);

  return 1;
}

/***** NOTIFICATIONS *****
Mapping callbacks!
Notifications not implemented yet:
SCN_STYLENEEDED
SCN_CHARADDED
SCN_SAVEPOINTREACHED
SCN_SAVEPOINTLEFT
SCN_MODIFYATTEMPTRO
SCN_KEY
SCN_UPDATEUI
SCN_MODIFIED
SCN_MACRORECORD
SCN_NEEDSHOWN
SCN_PAINTED
SCN_USERLISTSELECTION
SCN_URIDROPPED
SCN_ZOOM
SCN_HOTSPOTDOUBLECLICK
SCN_HOTSPOTRELEASECLICK
SCN_INDICATORCLICK
SCN_INDICATORRELEASE
SCN_CALLTIPCLICK
SCN_AUTOCSELECTION
SCN_AUTOCCANCELLED
SCN_AUTOCCHARDELETED
*/
static void iScintillaNotify(Ihandle *ih, struct SCNotification* pMsg)
{
  int lineClick = IUP_SSM(ih->handle, SCI_LINEFROMPOSITION, pMsg->position, 0);

  switch(pMsg->nmhdr.code)
  {
    case SCN_MARGINCLICK:
    {
      IFnii cb = (IFnii)IupGetCallback(ih, "MARGINCLICK_CB");
      if (cb)
        cb(ih, pMsg->margin, lineClick);
    }
    break;
    case SCN_DOUBLECLICK:
    {
      IFnii cb = (IFnii)IupGetCallback(ih, "DOUBLECLICK_CB");
      if (cb)
        cb(ih, pMsg->modifiers, lineClick);
    }
    break;
    case SCN_HOTSPOTCLICK:
    {
      IFnii cb = (IFnii)IupGetCallback(ih, "HOTSPOTCLICK_CB");
      if (cb)
        cb(ih, pMsg->modifiers, lineClick);
    }
    break;
    case SCN_DWELLSTART:
    {
      IFniii cb = (IFniii)IupGetCallback(ih, "DWELLSTART_CB");
      if (cb)
        cb(ih, pMsg->x, pMsg->y, lineClick);
    }
    break;
    case SCN_DWELLEND:
    {
      IFniii cb = (IFniii)IupGetCallback(ih, "DWELLEND_CB");
      if (cb)
        cb(ih, pMsg->x, pMsg->y, lineClick);
    }
    break;
  }
}

#ifdef GTK
static void gtkScintillaNotify(GtkWidget *w, gint wp, gpointer lp, Ihandle *ih)
{
  struct SCNotification *pMsg =(struct SCNotification *)lp;

  iScintillaNotify(ih, pMsg);

  (void)w;
  (void)wp;
}
#else
static int winScintillaWmNotify(Ihandle* ih, NMHDR* msg_info, int *result)
{
  SCNotification *pMsg = (SCNotification*)msg_info;

  iScintillaNotify(ih, pMsg);

  (void)result;
  return 0; /* result not used */
}
#endif

/*****************************************************************************/

static int iScintillaMapMethod(Ihandle* ih)
{
#ifdef GTK
  ih->data->editor = scintilla_new();
  if (!ih->data->editor)
    return IUP_ERROR;

  ih->handle = (InativeHandle*)SCINTILLA(ih->data->editor);
  scintilla_set_id((ScintillaObject*)ih->handle, 0);

  gtk_widget_show(ih->data->editor);
  gtk_widget_grab_focus(ih->data->editor);

  g_signal_connect(ih->data->editor, "sci-notify", G_CALLBACK(gtkScintillaNotify), ih);

  /* add to the parent, all GTK controls must call this. */
  iupgtkAddToParent(ih);

  gtk_widget_realize(ih->data->editor);
#else
  DWORD dwStyle = WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_CLIPSIBLINGS;
  DWORD dwExStyle = WS_EX_CLIENTEDGE;

  if (!ih->parent)
    return IUP_ERROR;

  if (!iupwinCreateWindowEx(ih, "Scintilla", dwExStyle, dwStyle))
    return IUP_ERROR;

  IupSetCallback(ih, "_IUPWIN_NOTIFY_CB", (Icallback)winScintillaWmNotify);
#endif

  /* add scrollbar */
  IUP_SSM(ih->handle, SCI_SETHSCROLLBAR, 0, 0);
  IUP_SSM(ih->handle, SCI_SETVSCROLLBAR, 0, 0);

  if (ih->data->sb & IUP_SB_HORIZ)
    IUP_SSM(ih->handle, SCI_SETHSCROLLBAR, 1, 0);

  if (ih->data->sb & IUP_SB_VERT)
    IUP_SSM(ih->handle, SCI_SETVSCROLLBAR, 1, 0);

  return IUP_NOERROR;
}

static void iScintillaComputeNaturalSizeMethod(Ihandle* ih, int *w, int *h, int *expand)
{
  int natural_w = iupAttribGetInt(ih, "VISIBLECOLUMNS"),
      natural_h = iupAttribGetInt(ih, "VISIBLELINES");
  (void)expand; /* unset if not a container */

  /* compute scrollbar */
  if (ih->data->sb != IUP_SB_NONE)
  {
    int sb_size = iupdrvGetScrollbarSize();
    if (ih->data->sb & IUP_SB_HORIZ)
      natural_h += sb_size;  /* sb horizontal affects vertical size */
    if (ih->data->sb & IUP_SB_VERT)
      natural_w += sb_size;  /* sb vertical affects horizontal size */
  }
  
  *w = natural_w;
  *h = natural_h;
}

static int iScintillaCreateMethod(Ihandle* ih, void **params)
{
  (void)params;
  ih->data = iupALLOCCTRLDATA();
  ih->data->sb = IUP_SB_HORIZ | IUP_SB_VERT;
  return IUP_NOERROR;
}

static void iScintillaReleaseMethod(Iclass* ic)
{
  (void)ic;
#ifndef GTK
  Scintilla_ReleaseResources();
#endif
}

static Iclass* iupScintillaNewClass(void)
{
  Iclass* ic = iupClassNew(NULL);

  ic->name = "scintilla";
  ic->format = NULL; /* no parameters */
  ic->nativetype = IUP_TYPECONTROL;
  ic->childtype  = IUP_CHILDNONE;
  ic->is_interactive = 1;
  ic->has_attrib_id  = 2;   /* has attributes with IDs that must be parsed */

  /* Class functions */
  ic->New     = iupScintillaNewClass;
  ic->Release = iScintillaReleaseMethod;
  ic->Create  = iScintillaCreateMethod;
  ic->Map     = iScintillaMapMethod;
  ic->ComputeNaturalSize = iScintillaComputeNaturalSizeMethod;
  ic->LayoutUpdate = iupdrvBaseLayoutUpdateMethod;

  /* Callbacks */
  iupClassRegisterCallback(ic, "MARGINCLICK_CB", "ii");
  iupClassRegisterCallback(ic, "DOUBLECLICK_CB", "ii");
  iupClassRegisterCallback(ic, "HOTSPOTCLICK_CB", "ii");
  iupClassRegisterCallback(ic, "DWELLSTART_CB", "iii");
  iupClassRegisterCallback(ic, "DWELLEND_CB", "iii");
  
  /* Text retrieval and modification */
  iupClassRegisterAttributeId(ic,  "INSERTTEXT", NULL, iScintillaSetInsertTextAttrib, IUPAF_WRITEONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic,  "ADDTEXT", NULL, iScintillaSetAddTextAttrib, IUPAF_WRITEONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic,  "APPENDTEXT", NULL, iScintillaSetAppendTextAttrib, IUPAF_WRITEONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic,  "TEXT", iScintillaGetTextAttrib, iScintillaSetTextAttrib, IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic,  "LINE", iScintillaGetLineAttrib, NULL, IUPAF_READONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic,  "CHARAT", iScintillaGetCharAtAttrib, NULL, IUPAF_READONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic,  "REPLACESEL", NULL, iScintillaSetReplaceSelAttrib, IUPAF_WRITEONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId2(ic, "TEXTRANGE", iScintillaGetTextRangeAttrib, NULL, IUPAF_READONLY|IUPAF_NO_INHERIT);  
  iupClassRegisterAttribute(ic,    "DELETERANGE", NULL, iScintillaSetDeleteRangeAttrib, NULL, NULL, IUPAF_WRITEONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic,    "READONLY", iScintillaGetReadOnlyAttrib, iScintillaSetReadOnlyAttrib, NULL, NULL, IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic,    "CLEARALL", NULL, iScintillaSetClearAllAttrib, NULL, NULL, IUPAF_WRITEONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic,    "CLEARDOCUMENTSTYLE", NULL, iScintillaSetClearDocumentStyleAttrib, NULL, NULL, IUPAF_WRITEONLY|IUPAF_NO_INHERIT);

  /* Style Definition Attributes */
  iupClassRegisterAttribute(ic,   "RESETDEFAULT", NULL, iScintillaSetResetDefaultAttrib, NULL, NULL, IUPAF_WRITEONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic,   "CLEARALLSTYLE", NULL, iScintillaSetClearAllStyleAttrib, NULL, NULL, IUPAF_WRITEONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "FONT", iScintillaGetFontStyleAttrib, iScintillaSetFontStyleAttrib, IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "FONTSIZE", iScintillaGetFontSizeStyleAttrib, iScintillaSetFontSizeStyleAttrib, IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "FONTSIZEFRAC", iScintillaGetFontSizeFracStyleAttrib, iScintillaSetFontSizeFracStyleAttrib, IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "BOLD", iScintillaGetBoldStyleAttrib, iScintillaSetBoldStyleAttrib, IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "WEIGHT", iScintillaGetWeightStyleAttrib, iScintillaSetWeightStyleAttrib, IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "ITALIC", iScintillaGetItalicStyleAttrib, iScintillaSetItalicStyleAttrib, IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "UNDERLINE", iScintillaGetUnderlineStyleAttrib, iScintillaSetUnderlineStyleAttrib, IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "FGCOLORSTYLE", iScintillaGetFgColorStyleAttrib, iScintillaSetFgColorStyleAttrib, IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "BGCOLORSTYLE", iScintillaGetBgColorStyleAttrib, iScintillaSetBgColorStyleAttrib, IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "EOLFILLED", iScintillaGetEolFilledStyleAttrib, iScintillaSetEolFilledStyleAttrib, IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "CHARSET", iScintillaGetCharSetStyleAttrib, iScintillaSetCharSetStyleAttrib, IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "CASE", iScintillaGetCaseStyleAttrib, iScintillaSetCaseStyleAttrib, IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "VISIBLE", iScintillaGetVisibleStyleAttrib, iScintillaSetVisibleStyleAttrib, IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "HOTSPOT", iScintillaGetHotSpotStyleAttrib, iScintillaSetHotSpotStyleAttrib, IUPAF_NO_INHERIT);

  /* Lexer Attributes */
  iupClassRegisterAttribute(ic,   "LEXER", iScintillaGetLexerAttrib, iScintillaSetLexerAttrib, NULL, NULL, IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic,   "PROPERTY", iScintillaGetPropertyAttrib, iScintillaSetPropertyAttrib, NULL, NULL, IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "KEYWORDS", NULL, iScintillaSetKeyWordsAttrib, IUPAF_WRITEONLY|IUPAF_NO_INHERIT);

  /* Folding Attributes */
  iupClassRegisterAttribute(ic,   "FOLDFLAGS", NULL, iScintillaSetFoldFlagsAttrib, NULL, NULL, IUPAF_WRITEONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic,   "TOGGLEFOLD", NULL, iScintillaSetToggleFoldAttrib, NULL, NULL, IUPAF_WRITEONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "FOLDLEVEL", iScintillaGetFoldLevelAttrib, iScintillaSetFoldLevelAttrib, IUPAF_NO_INHERIT);

  /* Margin Attributes */
  iupClassRegisterAttributeId(ic, "MARGINTYPEN", iScintillaGetMarginTypeNAttrib, iScintillaSetMarginTypeNAttrib, IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "MARGINWIDTHN", iScintillaGetMarginWidthNAttrib, iScintillaSetMarginWidthNAttrib, IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "MARGINMASKN", iScintillaGetMarginMaskNAttrib, iScintillaSetMarginMaskNAttrib, IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "MARGINSENSITIVEN", iScintillaGetMarginSensitiveNAttrib, iScintillaSetMarginSensitiveNAttrib, IUPAF_NO_INHERIT);

  /* Marker Attributes */
  iupClassRegisterAttribute(ic, "MARKERDEFINE", NULL, iScintillaSetMarkerDefineAttrib, NULL, NULL, IUPAF_WRITEONLY|IUPAF_NO_INHERIT);

  /* General */
  iupClassRegisterAttribute(ic, "SCROLLBAR", iScintillaGetScrollbarAttrib, iScintillaSetScrollbarAttrib, IUPAF_SAMEASSYSTEM, "YES", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "VISIBLECOLUMNS", NULL, NULL, IUPAF_SAMEASSYSTEM, "5", IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "VISIBLELINES",   NULL, NULL, IUPAF_SAMEASSYSTEM, "1", IUPAF_NO_INHERIT);

  return ic;
}

void IupScintillaOpen(void)
{
  if (!IupGetGlobal("_IUP_SCINTILLA_OPEN"))
  {
    iupRegisterClass(iupScintillaNewClass());
    IupSetGlobal("_IUP_SCINTILLA_OPEN", "1");

#ifndef GTK
    Scintilla_RegisterClasses(iupwin_hinstance);
#endif
  }
}

Ihandle *IupScintilla(void)
{
  return IupCreate("scintilla");
}
