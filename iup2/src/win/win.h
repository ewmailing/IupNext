/** \file
 * \brief Windows Core functions.
 *
 * See Copyright Notice in iup.h
 * $Id: win.h,v 1.1 2008-10-17 06:19:26 scuri Exp $
 */
 
#ifndef __WIN_H 
#define __WIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* used by wincreat.c, winproc.c */
#define HORZ_SCROLLBAR_SIZE 30000
#define VERT_SCROLLBAR_SIZE 30000

/* winsize.c */
int iupwinDialogDecorX(Ihandle* n);
int iupwinDialogDecorY(Ihandle* n);
int iupwinDialogDecorLeft(Ihandle* n);
int iupwinDialogDecorTop(Ihandle* n);
/* implements
 iupdrvGetCharSize, iupdrvWindowSizeY, iupdrvWindowSizeX
 iupdrvStringSize, iupdrvGetTextLineBreakSize, iupdrvGetBorderSize
*/
                  
/* winget.c implements
  iupdrvGetAttribute, iupdrvGetDefault
*/
                  
/* winkey.c */
int iupwinKeyProcess ( Ihandle* h, int winkey );
int iupwinKeyDecode(int winkey);

/* winset.c */
void   iupwinSetInit (void);
void   iupwinSetFinish (void);
void   iupwinSet (Ihandle* n, const char* a, char* v);
void   iupwinSetCursor (Ihandle *n);
int    iupwinGetColorRef (Ihandle *n, const char *attr, COLORREF *cr);
int    iupwinGetBrushIndex (COLORREF c);
HBRUSH iupwinGetBrush (int i);
void   iupwinSetBgColor (Ihandle *n);
char*  iupwinTransMultiline( char* txt, int* to_free );
void   iupwinUpdateFont(Ihandle *n);
void   iupwinUpdateAttr(Ihandle *n, const char *attr);
/* implements iupdrvSetAttribute, iupdrvStoreAttribute, IupUnMapFont, IupMapFont, 
   iupdrvSetIdleFunction
*/

/* win.c */
void iupwinAdjustPos(Ihandle* n, int* x, int* y, int width, int height);
int iupwinUseComCtl32Ver6(void);
int iupwinSetMouseHook(int v);
/* implements IupFlush, IupLoopStep, IupHide, IupShowXY, IupShow, IupMap, IupMainLoop
IupPopup, IupSetFocus, IupGetFocus, IupOpen, IupClose
*/

/* windraw.c */
void iupwinDrawButton( Ihandle* n, LPDRAWITEMSTRUCT iteminfo);
void iupwinDrawLabel( Ihandle* n, LPDRAWITEMSTRUCT iteminfo);
int iupwinThemeBorder(HWND wnd);

typedef struct IwinFont_
{
  char typeface[33];
  int height;
  BOOL is_bold;
  BOOL is_italic;
  BOOL is_underline;
  BOOL is_strikeout;
  HFONT font;
} IwinFont;

typedef struct IwinFontInfo_
{
  IwinFont *fontlist;

  int num_winfonts;
  int max_num_winfonts;
} IwinFontInfo;

/* global variables */

extern IwinFontInfo iupwin_fontinfo;      /* winset.c   */
extern HINSTANCE    iupwin_hinstance;     /* wincreat.c */

/* winhelp.c implements IupHelp */

/* winglobal.c implements iupdrvSetGlobal */

/* windestroy.c implements iupdrvDestroyHandle */

/* windlg.c implements IupMessage */

/* winresiz.c implements iupdrvResizeObjects */

#ifdef __cplusplus
}
#endif

#endif
