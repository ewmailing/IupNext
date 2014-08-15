/** \file
 * \brief Motif Driver Core and Initialization.
 *
 * See Copyright Notice in iup.h
 *  */
 
#ifndef __MOTIF_H 
#define __MOTIF_H

#ifdef __cplusplus
extern "C" {
#endif

#if (XmVERSION == 1 ) && (XmREVISION < 2)
#define XmStringCreateLocalized XmStringCreateSimple
#endif

typedef struct {
  Ihandle *ihandle;
  void *data;
} Iwidgetdata;

/* color.c - gerenciamento de cores */
void iupmotColorInit (void);
void iupmotColorClose (void);

typedef struct {
  Colormap colormap;         /* colormap para todos os canvas */
  XColor color_table[256];   /* tabela de cores do colormap */
  int num_colors;            /* tamanho maximo da tabela de cores  */
  int rshift;                   /* constante red para calculo truecolor */
  int gshift;                   /* constante green para calculo truecolor */
  int bshift;                   /* constante blue para calculo truecolor */
} Icolor;

extern unsigned long (* iupmot_GetPixel)(unsigned char r, unsigned char g, unsigned char b);
extern void (* iupmot_GetRGB)(unsigned long pixel, unsigned char* red, unsigned char* green, unsigned char* blue);

/* dialog states, used as values to 'data' field of Iwidgetdata for DIALOG */
#define HIDE  ((void*)0)
#define POPUP ((void*)1)
#define SHOW  ((void*)2)

#define hidden(d)  ((d) == HIDE)
#define shown(d)   ((((long)(d)) & 3) == (long)SHOW)
#define popuped(d) ((((long)(d)) & 3) == (long)POPUP)
#define iconic(d)  (((long)(d)) < 0)

#define RES_CONVERT( res_name, res_value) \
        XtVaTypedArg, (res_name), XmRString, (res_value), strlen(res_value) + 1

/* global variables */
extern Widget         iupmot_initialwidget;
extern Display      * iupmot_display;
extern Display      * iupmot_auxdisplay;
extern int            iupmot_screen;
extern int            iupmot_using8bitcanvas;
extern Ihandle      * iupmot_focus;
extern char         * iupmot_fontname;
extern Visual       * iupmot_visual;
extern Icolor         iupmot_color;
extern int            iupmot_depth;
extern int            iupmot_incallback;  /* motproc.c */
extern int            iupmot_exitmainloop;
extern int            iupmot_exitmenuloop;
extern Boolean        iupmot_ismwmrunning;
extern Pixel          iupmot_background;
extern Pixel          iupmot_foreground;
extern Pixel          iupmot_shadow;
extern Pixel          iupmot_highlight;
extern Pixel          iupmot_selection;
extern int            iupmot_nvisiblewin;

/* motif.c */
XmFontList iupmoGetFontList(char *v);
XFontStruct* iupmotGetFontStruct(Ihandle *n);
void       iupmotShowDialog(Ihandle *n);
void       iupmotHideDialog(Ihandle *n);
void       iupmotMoveDialog(Ihandle *n, int x, int y);
/* implements global variables and 
 iupdrvSetIdleFunction
 IupOpen, IupClose, IupGetFocus, IupFlush, IupShow
 IupSetFocus, IupPopup IupShowXY, IupMap, IupHide, IupMainLoop, IupLoopStep
*/

/* motsize.c */
/* implements 
 iupdrvGetCharSize, iupdrvWindowSizeY, iupdrvWindowSizeX
 iupdrvGetBorderSize, iupdrvStringSize, iupdrvGetTextLineBreakSize
*/

/* motproc.c */
void     iupmotCallMenu(void);
Ihandle* iupmotFindRadioOption (Ihandle *radio);

void iupmotCBscrollbar  (Widget w, XtPointer client_data, XtPointer call_data);
void iupmotCBinput      (Widget w, XtPointer client_data, XtPointer call_data);
void iupmotCBrepaint    (Widget w, XtPointer client_data, XtPointer call_data);
void iupmotCBdialog     (Widget w, XtPointer client_data, XtPointer call_data);
void iupmotCBmenu       (Widget w, XtPointer client_data, XtPointer call_data);
void iupmotCBarmmenu    (Widget w, XtPointer client_data, XtPointer call_data);
void iupmotCBmapmenu    (Widget w, XtPointer client_data, XtPointer call_data);
void iupmotCBlist       (Widget w, XtPointer client_data, XtPointer call_data);
void iupmotCBtext       (Widget w, XtPointer client_data, XtPointer call_data);
void iupmotCBmask       (Widget w, XtPointer client_data, XtPointer call_data);
void iupmotCBbutton     (Widget w, XtPointer client_data, XtPointer call_data);
void iupmotCBtoggle     (Widget w, XtPointer client_data, XtPointer call_data);
void iupmotCBradio      (Widget w, XtPointer client_data, XtPointer call_data);
void iupmotCBresize     (Widget w, XtPointer client_data, XtPointer call_data);
void iupmotCBclose      (Widget w, XtPointer client_data, XtPointer call_data);
void iupmotCBhelp       (Widget w, XtPointer client_data, XtPointer call_data);
void iupmotCBcaret      (Widget w, XtPointer client_data, XtPointer call_data);

void iupmotCBstructure   (Widget w, XtPointer data, XEvent *ev, Boolean *cont);
void iupmotCBcreatecanvas(Widget w, XtPointer data, XEvent *ev, Boolean *cont);
void iupmotCBbuttoncb    (Widget w, XtPointer data, XEvent *ev, Boolean *cont);
void iupmotCBfocus       (Widget w, XtPointer data, XEvent *ev, Boolean *cont);
void iupmotCBkeypress    (Widget w, XtPointer data, XEvent *ev, Boolean *cont);
void iupmotCBkeyrelease  (Widget w, XtPointer data, XEvent *ev, Boolean *cont);
void iupmotCBenterleave  (Widget w, XtPointer data, XEvent *ev, Boolean *cont);
void iupmotCBmotion      (Widget w, XtPointer data, XEvent *ev, Boolean *cont);

/* motcreat.c */
void iupmotSetParent(Widget w, Ihandle* h);
void iupmotCreateImage (Ihandle *n, Ihandle* parent, int inactive);
void iupmotCreateCursor (Ihandle *n); 
void iupmotCreateCanvasWithClass( Ihandle* n, WidgetClass c, ... );
/* implements iupdrvDestroyHandle, iupdrvCreateNativeObject
   iupdrvCreateObjects, iupdrvCreateObject
*/

/* motset.c */
int   iupmotGetWidgetHandles(Ihandle *n, Widget* w);
void  iupmotSetAttrs(Ihandle *);
Pixel iupmotGetColor(Ihandle *n, char *cor);
void  iupmotSetInitAttTable (void);
Ihandle*   iupmotGetZboxValue( Ihandle *n );
void iupmotUpdateFont(Ihandle *h);
/* implements iupdrvSetAttribute, IupUnMapFont, IupMapFont 
*/

/* motget.c */
void  iupmotGetInitAttTable (void);
void iupmotLincol( char *str, long int pos, long int *lin, long int *col );
/* implements iupdrvStoreAttribute, iupdrvGetDefault, iupdrvGetAttribute
*/

/* motkey.c */
int iupmotKeyGetCode(XKeyEvent *ev);
int iupmotKeyGetMenuKey(char *n);

/* motmsgdlg.c */
void iupmotMessageDlgOpen(void);

/* motfiledlg.c */
void IupFileDlgOpen(void);
/* Other functions from <iup.h> and implemented here :
Ihandle *IupFileDlg(void)
*/

/* motresize.c implements iupdrvResizeObjects 
*/

#ifdef __cplusplus
}
#endif

#endif
