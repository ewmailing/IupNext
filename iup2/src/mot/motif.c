/** \file
 * \brief Motif Driver Core and Initialization
 *
 * See Copyright Notice in iup.h
 * $Id: motif.c,v 1.3 2008-10-30 18:10:22 scuri Exp $
 */

#if defined(__STDC__) && defined(ULTRIX)
   #define X$GBLS(x)  #x
#endif

#include <stdio.h>		/* scanf */
#include <string.h>		/* strlen */
#include <assert.h>		/* strlen */
#include <Xm/Xm.h>
#include <Xm/RowColumn.h>
#include <Xm/Form.h>
#include <Xm/Label.h>
#include <Xm/PushB.h>
#include <Xm/TextF.h>
#include <Xm/DrawingA.h>
#include <Xm/Text.h>
#include <Xm/List.h>
#include <X11/Xatom.h>
#include <Xm/AtomMgr.h>
#include <Xm/Protocols.h>
#include <Xm/MwmUtil.h>
#include <sys/utsname.h>
#include <unistd.h>

#include "iglobal.h"
#include "ifunc.h"
#include "iglobalenv.h"
#include "icpi.h"
#include "idrv.h"
#include "motif.h"

void XtMoveWidget( Widget w, int x, int y); /* from ??? */

static XtAppContext mot_context = NULL;
static XFontStruct *mot_default_fontstruct = NULL;
static XmFontList mot_default_fontlist = NULL;
static IFidle mot_idle_cb = NULL;
static XtWorkProcId mot_idle_id;

static char *progname = "Iup";
static struct {
   int dx, dy, dw, dh;
   int title, border;
} wmsizes = {0, 0, 0, 0, 0, 0};  /* FIXME: this is not being initialized. */

/* FIXME: These should be in a global struct */
int           iupmot_nvisiblewin = 0;
Widget        iupmot_initialwidget = 0;
Display    *  iupmot_display       = NULL;
Display    *  iupmot_auxdisplay   = NULL;
int           iupmot_screen        = 0;
int           iupmot_using8bitcanvas = 0;
Ihandle    *  iupmot_focus         = NULL;
char       *  iupmot_fontname      = NULL;/* External font name read from the user's shell or fixed,
                                          * used only when the user has not set a value. */
Visual     *  iupmot_visual        = NULL;
Icolor        iupmot_color;
int           iupmot_depth         = 0;
int           iupmot_exitmainloop; 
int           iupmot_exitmenuloop; 
Boolean       iupmot_ismwmrunning;
Pixel         iupmot_background;
Pixel         iupmot_foreground;
Pixel         iupmot_shadow;
Pixel         iupmot_highlight;
Pixel         iupmot_selection;

/************************************************************************
 *                       Message Loop Management
 ************************************************************************/

static Boolean motIdlecbWorkProc (XtPointer client_data)
{
  if (mot_idle_cb && mot_idle_cb() == IUP_CLOSE)
  {
    iupmot_exitmainloop = 1;
    return True; /* descadastra a working procedure */
  }

  return False; /* faz com que a working procedure continue sendo chamada */
}

void iupdrvSetIdleFunction (Icallback f)
{
  if (mot_idle_cb)
    XtRemoveWorkProc(mot_idle_id);

  mot_idle_cb = (IFidle)f;

  if (mot_idle_cb)
    mot_idle_id = XtAppAddWorkProc(mot_context, motIdlecbWorkProc, NULL);
}

/* esta funcao difere da IupLoopStep exportada porque:
- chama a callback de idle, caso exista alguma
- bloqueia caso nao exista evento na fila
*/
static int motLoopStep(void)
{
  int incb = iupmot_incallback;

  iupmot_incallback = 0;

  XtAppProcessEvent (mot_context, XtIMAll);
  iupmotCallMenu();

  iupmot_incallback = incb;

  return iupmot_exitmainloop ? IUP_CLOSE : IUP_DEFAULT;
}

int IupLoopStep (void)
{
  if (!XtAppPending(mot_context)) return IUP_DEFAULT;

  return motLoopStep();
}

int IupMainLoop (void)
{
  iupmot_exitmainloop = 0;

  while ( !iupmot_exitmainloop )
  {
    if (iupmot_nvisiblewin <= 0)
    {
      char *ll = IupGetGlobal(IUP_LOCKLOOP);
      if(!ll || iupStrEqual(ll, "NO"))
        break;
    }

    motLoopStep();
  }

  iupmot_exitmainloop = 0;

  return IUP_NOERROR;
}

void IupExitLoop(void)
{
  iupmot_exitmainloop = 1;
}

void IupFlush(void)
{
  while ( XPending (iupmot_display) != 0 )
  {
    motLoopStep();
  }
  XFlush( iupmot_display );
}


/************************************************************************
*                       Show/Hide/Popup Management
************************************************************************/


int IupHide (Ihandle* n)
{
  if ((n == NULL)  ||  (handle(n) == NULL))
    return IUP_ERROR;

  IupSetAttribute(n, IUP_VISIBLE, IUP_NO);

  IupFlush();

  return IUP_NOERROR;
}

static int motMapCoordinate(Ihandle *n, int *x, int *y)
{
  Dimension width, height;
  int sw = DisplayWidth(iupmot_display, iupmot_screen);
  int sh = DisplayHeight(iupmot_display, iupmot_screen);
  int maxx = sw-currentwidth(n);
  int maxy = sh-currentheight(n);

  if (*x==IUP_MOUSEPOS || *y==IUP_MOUSEPOS)
  {
    Window root, child;
    int cx, cy;
    unsigned int keys;
    XQueryPointer(iupmot_display, RootWindow(iupmot_display, iupmot_screen),
      &root, &child, x, y, &cx, &cy, &keys); 
    return 1;
  }

  if (*x == IUP_CURRENT) *x = IUP_CENTER;
  if (*y == IUP_CURRENT) *y = IUP_CENTER;

  XtVaGetValues((Widget)handle(n), XmNwidth,  &width,
    XmNheight, &height, NULL );

  if (*x == IUP_LEFT)   *x = 0;
  else if (*x == IUP_RIGHT)  *x = maxx;
  else if (*x == IUP_CENTER) *x = (sw-width+wmsizes.dw)/2;
  else if (*x < 0)           *x = 0;
  else if (*x > maxx)        *x = maxx;

  if (*y == IUP_TOP)    *y = 0;
  else if (*y == IUP_BOTTOM) *y = maxy;
  else if (*y == IUP_CENTER) *y = (sh-height+wmsizes.dh)/2;
  else if (*y < 0)           *y = 0;
  else if (*y > maxy)        *y = maxy; 

  return 1;
}

void iupmotMoveDialog(Ihandle *n, int x, int y)
{
  if (motMapCoordinate(n, &x, &y))
  {
    XSizeHints hints;
    hints.flags = USPosition;
    hints.x = x;
    hints.y = y;
    XSetWMNormalHints(iupmot_display, XtWindow((Widget)handle(n)), &hints);
    XtMoveWidget((Widget)handle(n), x, y);
  }
}

int IupShowXY(Ihandle* n, int x, int y)
{
  if ( !n || IupMap(n)==IUP_ERROR) return IUP_ERROR;

  if (type(n) == DIALOG_) iupmotMoveDialog(n, x, y);

  IupSetAttribute(n, IUP_VISIBLE, IUP_YES);

  /* o IupFlush abaixo e' necessario para garantir que os
  elementos do dialogo sejam desenhados caso seja
  executado um hide sem passar pelo mainloop
  (show e hide na mesma callback). e' preciso obervar
  as aplicacoes pra ver se isso nao as atrapalha, ja'
  que um IupShow pode processar callbacks antes de retornar */

  IupFlush();

  return IUP_NOERROR;
}

int IupShow (Ihandle* n)
{
  assert(n != NULL);
  if(n == NULL)
    return IUP_ERROR;

  if (handle(n))
    return IupShowXY( n, IupGetInt(n, IUP_X), IupGetInt(n, IUP_Y));
  else
    return IupShowXY( n, IUP_CURRENT, IUP_CURRENT);
}

void iupmotHideDialog(Ihandle *n)
{
  Iwidgetdata *wd;
  Widget w = XtNameToWidget((Widget)handle(n),"*dialog_area");
  if (!w) return;
  XtVaGetValues (w, XmNuserData, &wd, NULL);
  if (!wd) return;

  if (hidden(wd->data)) return;

  if (shown(wd->data)) /* aberto com show */
  {
    XtUnmapWidget((Widget)handle(n));
  }
  else if (popuped(wd->data))   /* aberto com popup */
  {
    XtPopdown((Widget)handle(n));
    iupmot_exitmainloop = 1;
  }

  iupmot_nvisiblewin--;
  wd->data = HIDE;

  { /* waits until window gets unmapped */
    XWindowAttributes wa;
    do
    XGetWindowAttributes(XtDisplay((Widget)handle(n)),XtWindow((Widget)handle(n)),&wa);
    while (wa.map_state==IsViewable);
  }
}

void iupmotShowDialog(Ihandle *n)
{
  Iwidgetdata *wd;
  Widget w = XtNameToWidget((Widget)handle(n),"*dialog_area");
  if (!w) return;
  XtVaGetValues (w, XmNuserData, &wd, NULL);
  if (!wd) return;

  if (!hidden(wd->data)) /* already visible */
  {
    XRaiseWindow( XtDisplay((Widget)handle(n)),XtWindow((Widget)handle(n)) );
    return; 
  }

  wd->data = SHOW;

  XtMapWidget ((Widget)handle (n));
  { /* waits until window get mapped */
    XWindowAttributes wa;
    do
      XGetWindowAttributes(XtDisplay((Widget)handle(n)),XtWindow((Widget)handle(n)),&wa);
    while (wa.map_state!=IsViewable);
  }
}

static int motPopupMenu(Ihandle *n, int x, int y)
{
  XButtonEvent ev;
  motMapCoordinate( n, &x, &y );
  ev.x_root = x;
  ev.y_root = y;
  XmMenuPosition((Widget)handle(n), &ev);
  XtManageChild((Widget)handle(n));
  iupmot_exitmenuloop = 0;
  while ( !iupmot_exitmenuloop )
    motLoopStep();
  iupmot_exitmenuloop = 0;
  return IUP_NOERROR;
}

static int motPopupDialog(Ihandle *n, int x, int y)
{
  int ow;
  Iwidgetdata *wd;
  Widget w = XtNameToWidget((Widget)handle(n),"*dialog_area");

  if (!w) return IUP_ERROR;
  XtVaGetValues (w, XmNuserData, &wd, NULL);
  if (!wd) return IUP_ERROR;

  if ( ! hidden(wd->data) ) return IUP_ERROR; /* already visible */

  iupmotMoveDialog(n, x, y);

  wd->data = POPUP;

  ow = iupmot_nvisiblewin;
  XtPopup( (Widget)handle(n), XtGrabExclusive );
  /* process messagens until window is visible */
  while (ow == iupmot_nvisiblewin) 
    motLoopStep();

  /* interrupt processing here */
  IupMainLoop();

  if (XtIsRealized(w))
  {
    iupmotHideDialog(n);
    iupmot_exitmainloop = 0;
  }

  return IUP_NOERROR;
}

int IupPopup (Ihandle *n, int x, int y)
{
  if (n == NULL)
    return IUP_ERROR;

  if (type(n)!=DIALOG_ && type(n)!=MENU_) return IUP_ERROR;

  if (IupMap(n)==IUP_ERROR) return IUP_ERROR;

  if (type(n)==MENU_)
    motPopupMenu(n, x, y);
  else
    motPopupDialog(n, x, y);

  IupFlush();

  return IUP_NOERROR;
}


/************************************************************************
*                       Open/Close/Map and others
************************************************************************/


int IupMap(Ihandle* n)
{
  int createdialog=0;
  if (n==NULL) return IUP_ERROR;

  if(type(n) == DIALOG_)
  {
    if (!handle(n)) createdialog=1;
    if (iupSetSize (n) == IUP_ERROR)
      return IUP_ERROR;
  }
  else if(type(n) == MENU_)
  {
    if (!parent(n))
      iupmotSetParent(NULL, NULL);
    else if (!handle(parent(n)))
      return IUP_ERROR;
    else
      iupmotSetParent((Widget)handle(parent(n)), parent(n));
  }
  else if(type(n) == SUBMENU_ || type(n) == ITEM_)
  {
    if (!parent(n) || !handle(parent(n)))
      return IUP_ERROR;
    iupmotSetParent((Widget)handle(parent(n)), parent(n));
  }
  else
  {
    Ihandle *dial;
    dial = IupGetDialog(n);
    if (dial == NULL || handle(dial) == NULL) return IUP_ERROR;
    iupmotSetParent( XtNameToWidget((Widget)handle(dial), "*dialog_area"), dial);
  }
  iupdrvCreateObjects (n);
  iupmotSetParent(NULL, NULL);
  XtRealizeWidget ((Widget)handle (n));

  if (createdialog)
  {
    Icallback cb = IupGetCallback(n, IUP_MAP_CB);
    XSetWindowAttributes attr;
    attr.bit_gravity = ForgetGravity;
    XChangeWindowAttributes( iupmot_display,
      XtWindow((Widget)handle(n)), CWBitGravity, &attr );
    XChangeWindowAttributes( iupmot_display,
      XtWindow(XtNameToWidget((Widget)handle(n),"*dialog_form")),
      CWBitGravity, &attr );
    XChangeWindowAttributes( iupmot_display,
      XtWindow(XtNameToWidget((Widget)handle(n),"*dialog_area")),
      CWBitGravity, &attr );
    iupdrvSetAttribute(n, IUP_CURSOR, IupGetAttribute(n, IUP_CURSOR));
    iupdrvSetAttribute(n, IUP_DEFAULTENTER, IupGetAttribute(n, IUP_DEFAULTENTER));
    if (cb)
    {
      iupmot_incallback = 1;
      if (cb (n)==IUP_CLOSE) iupmot_exitmainloop = 1;
      iupmot_incallback = 0;
    }
  }

  if (type(n)==DIALOG_)
    iupdrvSetAttribute(n, "PARENTDIALOG", IupGetAttribute(n, "PARENTDIALOG"));

  IupFlush();

  return IUP_NOERROR;
}

Ihandle *IupSetFocus (Ihandle *n)
{
  if (!n || !handle(n)) return iupmot_focus;

  if(iupAcceptFocus(n))  /* Elementos normais que aceitam o foco */
  {
    XmProcessTraversal((Widget)handle(n), XmTRAVERSE_CURRENT);
  }
  else /* Elementos tipo HBOX, VBOX, ZBOX... */
  {
    IupNextField(n);
  }
  return iupmot_focus;

}

Ihandle *IupGetFocus (void)
{
  return iupmot_focus;
}

static int iup_opened = 0;
void IupClose (void)
{ 
  if (!iup_opened)
    return;
  iup_opened = 0;

  iupmot_exitmainloop = 1;

  if (mot_idle_cb)
    XtRemoveWorkProc(mot_idle_id);

  /* free the resources */
  if (iupmot_initialwidget)
    XtDestroyWidget(iupmot_initialwidget);

  /* FIXME: In Cygwin this is causing an error:
  "Error: LTHashTableForEachItem: NULL hash table specified."  */
  if (mot_context)
    XtDestroyApplicationContext(mot_context);

  iupStrGetMemory(-1); /* Frees internal buffer */

  iupmotColorClose();
  IupTimerClose();
  IupSpinClose();

  iupGlobalEnvFinish();
  iupNamesFinish();
  iupLexFinish();
  iupCpiFinish();
  iupFuncFinish();
}

static int motOpenAuxDisplay (void)
{
  XVisualInfo vinfo;
  if (XMatchVisualInfo(iupmot_display, XDefaultScreen(iupmot_display), 24,
    TrueColor, &vinfo) == 0 &&
    XMatchVisualInfo(iupmot_display, XDefaultScreen(iupmot_display), 16,
    TrueColor, &vinfo) == 0)
  {
    iupmot_auxdisplay = XOpenDisplay(NULL);
    if (!iupmot_auxdisplay)
    {
      fprintf (stderr, "IUP error: cannot open auxiliar display.\n");
      iupmot_auxdisplay = iupmot_display;
      return 1;
    }
  }
  else
  {
    iupmot_auxdisplay = iupmot_display;
    iupmot_using8bitcanvas = 0;
    return 1;
  }
  iupmot_using8bitcanvas = 1;
  return 0;
}

static void motReserveColors (void)
{
  unsigned char r, g, b;
  XtVaGetValues(iupmot_initialwidget, XmNbackground, &iupmot_background, NULL);
  XmGetColors(DefaultScreenOfDisplay(iupmot_display), iupmot_color.colormap,
    iupmot_background, &iupmot_foreground, &iupmot_highlight, &iupmot_shadow,
    &iupmot_selection);
  if (iupmot_using8bitcanvas)
  {
    iupmot_GetRGB(iupmot_background, &r, &g, &b);
    iupmot_GetPixel(r, g, b);
    iupmot_GetRGB(iupmot_foreground, &r, &g, &b);
    iupmot_GetPixel(r, g, b);
    iupmot_GetRGB(iupmot_shadow, &r, &g, &b);
    iupmot_GetPixel(r, g, b);
    iupmot_GetRGB(iupmot_highlight, &r, &g, &b);
    iupmot_GetPixel(r, g, b);
    iupmot_GetRGB(iupmot_selection, &r, &g, &b);
    iupmot_GetPixel(r, g, b);
  }
}

/* This function can be used instead of the IupOpen function in applications that directly use Motif. 
This allows a Motif application to use IUP’s functionalities to create dialogs, for instance. 
The parameter must be a Widget of the applicationShellWidgetClass class, 
usually returned by function XtAppCreateShell. Using this function, 
the IUP_TRUECOLOR environment variable is ignored. For the driver to work properly, 
we recommend using the following fallback resources when initializing the X toolkit: 
"*background: #aaaaaa", 
"*foreground: #000000", 
"*fontList: -misc-fixed-bold-r-normal-*-13-*", 
"*dragInitiatorProtocolStyle: DRAG_NONE", 
"*dragReceiverProtocolStyle: DRAG_NONE" */
static int motSetShellWidget(Widget w)
{
  IupSetGlobal( "VERSION",  IUP_VERSION );
  IupSetGlobal( "COPYRIGHT",  IUP_COPYRIGHT );
  IupSetGlobal( "DRIVER",   "Motif" );
  {
    char mv[50];
    struct utsname un;
    int major = xmUseVersion/1000;
    int minor = xmUseVersion - major * 1000;
    uname(&un);
    sprintf(mv, "%d.%d", major, minor);
    IupStoreGlobal("MOTIFVERSION", mv);
    sprintf(mv, "%d", (XmVERSION * 1000 + XmREVISION * 100 + XmUPDATE_LEVEL));
    IupStoreGlobal("MOTIFNUMBER", mv);
    IupStoreGlobal("SYSTEMVERSION", un.release);
    IupStoreGlobal("SYSTEM", un.sysname);

    gethostname(mv, 50);
    IupStoreGlobal("COMPUTERNAME", mv);
  }

  IupStoreGlobal("USERNAME", getlogin());

  if(IupGetLanguage() == NULL)
    IupSetLanguage(IUP_PORTUGUESE);

  {
    static int first = 1;
    char* env = getenv("IUP_QUIET");
    if (first && env && strcmp(env, "NO")==0)
    {
      printf("IUP "IUP_VERSION" "IUP_COPYRIGHT"\n");
      first = 0;
    }
  }

  if (!w) return IUP_ERROR;

  if (XtClass(w) != applicationShellWidgetClass) return IUP_ERROR;

  iupmot_initialwidget = w;
  iupmot_display = XtDisplay (iupmot_initialwidget);
  iupmot_screen  = XDefaultScreen (iupmot_display);
  iupmot_fontname= XGetDefault(iupmot_display,progname,"fontList");
  iupmot_ismwmrunning = XmIsMotifWMRunning(iupmot_initialwidget);

  XtVaGetValues(iupmot_initialwidget, XmNvisual,   &iupmot_visual,
    XmNdepth,    &iupmot_depth,
    XmNcolormap, &iupmot_color.colormap,
    NULL);
  if (getenv("IUP_DEBUG"))
    XSynchronize(iupmot_display, 1);

  if (!iupmot_fontname ||
    (mot_default_fontstruct = XLoadQueryFont (iupmot_display, iupmot_fontname)) == NULL)
  {
    iupmot_fontname = "fixed";
    if ( (mot_default_fontstruct = XLoadQueryFont (iupmot_display, iupmot_fontname)) == NULL )
    {
      fprintf (stderr, "IUP error: cannot load fixed font.\n");
      return IUP_ERROR;
    }
  }

  mot_default_fontlist = XmFontListCreate(mot_default_fontstruct, XmSTRING_DEFAULT_CHARSET);

  IupSetGlobal("XDISPLAY", (char*)iupmot_display);
  IupSetGlobal("XSCREEN", (char*)iupmot_screen);

  iupmotMessageDlgOpen();
  IupFileDlgOpen();
  IupTimerOpen();
  IupSpinOpen();
  IupCboxOpen();
  IupSboxOpen();

  {
    static char ss[15];
    sprintf( ss, "%dx%d", DisplayWidth(iupmot_display,iupmot_screen),
      DisplayHeight(iupmot_display,iupmot_screen) );
    IupSetGlobal( IUP_SCREENSIZE, ss );
  }

  {
    static char sd[10];
    sprintf( sd, "%d", iupmot_depth);
    IupSetGlobal( "SCREENDEPTH", sd );
  }

  {
    static char motbgcolor[15];
    unsigned char r, g, b;
    iupmot_GetRGB(iupmot_background, &r, &g, &b);
    sprintf( motbgcolor, "%d %d %d", (int)r, (int)g, (int)b );
    IupSetGlobal( "DLGBGCOLOR", motbgcolor );
  }

  return IUP_NOERROR;
}

int IupOpen (int *argc, char ***argv)
{
  String fallback[] = {
    "*background: #aaaaaa",
      "*foreground: #000000",
      "*fontList: -misc-fixed-bold-r-normal-*-13-*",
      "*dragInitiatorProtocolStyle: DRAG_NONE",
      "*dragReceiverProtocolStyle: DRAG_NONE",
      NULL
  };
  int   nargs = 0;
  Arg args[3];
  int    dummy_argc = 0;
  char** dummy_argv = {0};

  if (!argc) argc = &dummy_argc;
  if (!argv) argv = &dummy_argv;

  if (iup_opened)
    return IUP_OPENED;
  iup_opened = 1;

  iupNamesInit();
  iupFuncInit();
  iupLexInit();
  iupCpiInit();
  iupGlobalEnvInit();

  XtToolkitInitialize();
  mot_context = XtCreateApplicationContext();
  XtAppSetFallbackResources(mot_context, fallback);
  iupmot_display = XtOpenDisplay(mot_context,
    NULL,      /* display */
    NULL,      /* app name */
    progname,  /* class name */
    NULL,      /* options* */
    0,         /* num options */
    argc,      /* argc* */
    *argv);    /* argv* */
  if (!iupmot_display)
  {
    fprintf (stderr, "IUP error: cannot open display.\n");
    return IUP_ERROR;
  }
  else
  {
    motOpenAuxDisplay();
  }
  iupmot_visual = DefaultVisual(iupmot_display, XDefaultScreen(iupmot_display));
  iupmot_depth  = DefaultDepth(iupmot_display, XDefaultScreen(iupmot_display));

  iupmotColorInit();
  iupmotSetInitAttTable();
  iupmotGetInitAttTable();

  XtSetArg (args[nargs], XmNvisual, iupmot_visual); nargs++;
  XtSetArg (args[nargs], XmNdepth, iupmot_depth); nargs++;
  XtSetArg (args[nargs], XmNcolormap, iupmot_color.colormap); nargs++;
  iupmot_initialwidget = XtAppCreateShell(NULL,
    progname,
    applicationShellWidgetClass,
    iupmot_display,
    args,
    nargs);
  motReserveColors();

  return motSetShellWidget(iupmot_initialwidget);
}

/***************************************************/

typedef struct _motFont 
{
  char name[100];
  XmFontList fl;
  XFontStruct *fs;
  struct _motFont *next;
} motFont;

static motFont* motRetrieveFont(char *v)
{
  static motFont *first = 0;
  XFontStruct* fs;
  motFont *p = first, *newfont;
  while (p && !iupStrEqual(p->name,v)) { p=p->next; }
  if (p) return p;
  fs = XLoadQueryFont(iupmot_display, v);
  if (!fs) return 0;
  newfont = (motFont*)malloc(sizeof(motFont));
  strncpy(newfont->name, v, 99);
  newfont->fs = fs;
  newfont->fl = XmFontListCreate(newfont->fs, XmSTRING_DEFAULT_CHARSET);
  newfont->next = first;
  first = newfont;
  return newfont;
}

XmFontList iupmoGetFontList(char *v)
{
  if (v)
  {
    motFont *f = motRetrieveFont(v);
    if (f) return f->fl;
  }
  return mot_default_fontlist;
}

XFontStruct* iupmotGetFontStruct(Ihandle *n)
{
  char *font = iupGetEnv(n, "_IUPMOT_FONT");
  if (font)
  {
    motFont *f = motRetrieveFont(font);
    if (f) return f->fs;
  }
  return mot_default_fontstruct;
}

#ifndef IUP_MOTIF_NO_REDISPLAY
#include <Xm/Print.h>
#else
#define XmRedisplayWidget XmUpdateDisplay
#endif

void IupUpdate(Ihandle* ih)
{
  if (ih->handle)
    XmRedisplayWidget(ih->handle);
}
