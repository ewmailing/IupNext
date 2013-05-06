/*
* IupScintilla component
*
* Description : A source code editing component, 
* derived from Scintilla (http://www.scintilla.org/)
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
#include "iup_scintilla.h"
#include "iupcbs.h"
#include "iup_key.h"

#include "iup_class.h"
#include "iup_object.h"
#include "iup_attrib.h"
#include "iup_str.h"
#include "iup_drv.h"
#include "iup_drvfont.h"
#include "iup_register.h"
#include "iup_layout.h"
#include "iup_assert.h"

#ifdef GTK
#include "iupgtk_drv.h"
#else
#include "iupwin_drv.h"
#endif

#include "iupsci_folding.h"
#include "iupsci_markers.h"
#include "iupsci_margin.h"
#include "iupsci_lexer.h"
#include "iupsci_style.h"
#include "iupsci_text.h"
#include "iupsci_selection.h"
#include "iupsci_clipboard.h"
#include "iupsci_overtype.h"
#include "iupsci_scrolling.h"
#include "iupsci_tab.h"
#include "iupsci_wordwrap.h"
#include "iupsci_whitespace.h"
#include "iupsci_cursor.h"
#include "iupsci_bracelight.h"
#include "iupsci.h"


#ifndef GTK
#define WM_IUPCARET WM_APP+1   /* Custom IUP message */
#endif

sptr_t iupScintillaSendMessage(Ihandle* ih, unsigned int iMessage, uptr_t wParam, sptr_t lParam)
{
#ifdef GTK
  return scintilla_send_message(SCINTILLA(ih->handle), iMessage, wParam, lParam);
#else
  return SendMessage(ih->handle, iMessage, wParam, lParam);
#endif
}


/***** AUXILIARY ATTRIBUTES *****/

long iupScintillaEncodeColor(unsigned char r, unsigned char g, unsigned char b)
{
  return (((unsigned long)r) <<  0) |
         (((unsigned long)g) <<  8) |
         (((unsigned long)b) << 16);
}

void iupScintillaDecodeColor(long color, unsigned char *r, unsigned char *g, unsigned char *b)
{
  *r = (unsigned char)(((color) >>  0) & 0xFF);
  *g = (unsigned char)(((color) >>  8) & 0xFF);
  *b = (unsigned char)(((color) >> 16) & 0xFF);
}

void iupScintillaConvertLinColToPos(Ihandle* ih, int lin, int col, int *pos)
{
  *pos = iupScintillaSendMessage(ih, SCI_POSITIONFROMLINE, lin, 0);
    
  if(*pos != -1)
  {
    int line_length = iupScintillaSendMessage(ih, SCI_GETLINEENDPOSITION, lin, 0) - iupScintillaSendMessage(ih, SCI_POSITIONFROMLINE, lin, 0);
    if(col <= line_length)
      *pos += col;
    else
      *pos += line_length;
  }
  else
  {
    /* "lin" is greater than the lines in the document */
    *pos = iupScintillaSendMessage(ih, SCI_GETLINECOUNT, 0, 0);
  }
}

void iupScintillaConvertPosToLinCol(Ihandle* ih, int pos, int *lin, int *col)
{
  *lin = iupScintillaSendMessage(ih, SCI_LINEFROMPOSITION, pos, 0);
  *col = iupScintillaSendMessage(ih, SCI_GETCOLUMN, pos, 0);
}

static int iScintillaConvertXYToPos(Ihandle* ih, int x, int y)
{
  return iupScintillaSendMessage(ih, SCI_POSITIONFROMPOINT, x, y);
}


/***** GENERAL FUNCTIONS *****/

static int iScintillaSetUsePopupAttrib(Ihandle* ih, const char* value)
{
  iupScintillaSendMessage(ih, SCI_USEPOPUP, iupStrBoolean(value), 0);
  return 1;  /* there is no get */
}

static int iScintillaSetAppendNewlineAttrib(Ihandle* ih, const char* value)
{
  if (iupStrBoolean(value))
    ih->data->append_newline = 1;
  else
    ih->data->append_newline = 0;
  return 0;
}

static char* iScintillaGetAppendNewlineAttrib(Ihandle* ih)
{
  if (ih->data->append_newline)
    return "YES";
  else
    return "NO";
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

  return 0;
}


/***** NOTIFICATIONS *****/


static void iScintillaKeySetStatus(int state, char* status, int doubleclick)
{
  if (state & SCMOD_SHIFT)
    iupKEY_SETSHIFT(status);

  if (state & SCMOD_CTRL)
    iupKEY_SETCONTROL(status); 

  iupKEY_SETBUTTON1(status);

  if (state & SCMOD_ALT)
    iupKEY_SETALT(status);

  if (state & SCMOD_META) /* Apple/Win */
    iupKEY_SETSYS(status);

  if (doubleclick)
    iupKEY_SETDOUBLE(status);
}

static void iScintillaNotify(Ihandle *ih, struct SCNotification* pMsg)
{
  int lin = iupScintillaSendMessage(ih, SCI_LINEFROMPOSITION, pMsg->position, 0);
  int col = iupScintillaSendMessage(ih, SCI_GETCOLUMN, pMsg->position, 0);

  switch(pMsg->nmhdr.code)
  {
  case SCN_SAVEPOINTREACHED:
  case SCN_SAVEPOINTLEFT:
    {
      IFni cb = (IFni)IupGetCallback(ih, "SAVEPOINT_CB");
      if (cb)
        cb(ih, pMsg->nmhdr.code==SCN_SAVEPOINTREACHED? 1: 0);
    }
    break;
  case SCN_MARGINCLICK:
    {
      IFniis cb = (IFniis)IupGetCallback(ih, "MARGINCLICK_CB");
      if (cb)
      {
        char status[IUPKEY_STATUS_SIZE] = IUPKEY_STATUS_INIT;
        iScintillaKeySetStatus(pMsg->modifiers, status, 0);
        cb(ih, pMsg->margin, lin, status);
      }
    }
    break;
  case SCN_HOTSPOTDOUBLECLICK:
  case SCN_HOTSPOTCLICK:
    {
      IFniiis cb = (IFniiis)IupGetCallback(ih, "HOTSPOTCLICK_CB");
      if (cb)
      {
        char status[IUPKEY_STATUS_SIZE] = IUPKEY_STATUS_INIT;
        iScintillaKeySetStatus(pMsg->modifiers, status, pMsg->nmhdr.code==SCN_HOTSPOTDOUBLECLICK? 1: 0);
        cb(ih, pMsg->position, lin, col, status);
      }
    }
    break;
  case SCN_ZOOM:
    {
      IFni cb = (IFni)IupGetCallback(ih, "ZOOM_CB");
      if (cb)
      {
        int points = iupScintillaSendMessage(ih, SCI_GETZOOM, 0, 0);
        cb(ih, points);
      }
    }
    break;
  case SCN_MODIFIED:
    {
      if (ih->data->ignore_change)
      {
#ifndef GTK
        PostMessage(ih->handle, WM_IUPCARET, 0, 0L);
#endif
        break;
      }

      if (pMsg->modificationType&SC_PERFORMED_USER ||
          pMsg->modificationType&SC_PERFORMED_UNDO || 
          pMsg->modificationType&SC_PERFORMED_REDO)
      {
        if (pMsg->modificationType&SC_MOD_BEFOREINSERT ||
            pMsg->modificationType&SC_MOD_BEFOREDELETE)
        {
          IFn value_cb = (IFn)IupGetCallback(ih, "VALUECHANGED_CB");
          IFniiis cb = (IFniiis)IupGetCallback(ih, "ACTION");
          if (cb)
          {
            int insert = 1;
            if (pMsg->modificationType&SC_MOD_BEFOREDELETE)
              insert = 0;

            cb(ih, insert, pMsg->position, pMsg->length, (char*)pMsg->text);
          }

          if (value_cb)
            value_cb(ih);

#ifndef GTK
          PostMessage(ih->handle, WM_IUPCARET, 0, 0L);
#endif
        }
      }
    }
    break;
  }
}

static void iScintillaCallCaretCb(Ihandle* ih)
{
  int pos;

  IFniii cb = (IFniii)IupGetCallback(ih, "CARET_CB");
  if (!cb)
    return;

  pos = iupScintillaSendMessage(ih, SCI_GETCURRENTPOS, 0, 0);

  if (pos != ih->data->last_caret_pos)
  {
    int col, lin;
    iupScintillaConvertPosToLinCol(ih, pos, &lin, &col);

    ih->data->last_caret_pos = pos;

    cb(ih, lin, col, pos);
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

static gboolean gtkScintillaKeyReleaseEvent(GtkWidget *widget, GdkEventKey *evt, Ihandle *ih)
{
  iScintillaCallCaretCb(ih);
  (void)widget;
  (void)evt;
  return FALSE;
}

static gboolean gtkScintillaButtonEvent(GtkWidget *widget, GdkEventButton *evt, Ihandle *ih)
{
  iScintillaCallCaretCb(ih);
  return iupgtkButtonEvent(widget, evt, ih);
}

#else

static int winScintillaWmNotify(Ihandle* ih, NMHDR* msg_info, int *result)
{
  struct SCNotification *pMsg = (struct SCNotification*)msg_info;

  iScintillaNotify(ih, pMsg);

  (void)result;
  return 0; /* result not used */
}

static int winScintillaProc(Ihandle* ih, UINT msg, WPARAM wp, LPARAM lp, LRESULT *result)
{
  switch (msg)
  {
  case WM_KEYDOWN:
    {
      PostMessage(ih->handle, WM_IUPCARET, 0, 0L);
      break;
    }
  case WM_KEYUP:
    {
      PostMessage(ih->handle, WM_IUPCARET, 0, 0L);
      break;
    }
  case WM_LBUTTONDBLCLK:
  case WM_MBUTTONDBLCLK:
  case WM_RBUTTONDBLCLK:
  case WM_LBUTTONDOWN:
  case WM_MBUTTONDOWN:
  case WM_RBUTTONDOWN:
    {
      if (iupwinButtonDown(ih, msg, wp, lp)==-1)
      {
        *result = 0;
        return 1;
      }
      PostMessage(ih->handle, WM_IUPCARET, 0, 0L);
      break;
    }
  case WM_MBUTTONUP:
  case WM_RBUTTONUP:
  case WM_LBUTTONUP:
    {
      if (iupwinButtonUp(ih, msg, wp, lp)==-1)
      {
        *result = 0;
        return 1;
      }
      PostMessage(ih->handle, WM_IUPCARET, 0, 0L);
      break;
    }
  case WM_IUPCARET:
    {
      iScintillaCallCaretCb(ih);
      break;
    }
  case WM_MOUSEMOVE:
    {
      iupwinMouseMove(ih, msg, wp, lp);
      break;
    }
  }

  return iupwinBaseProc(ih, msg, wp, lp, result);
}
#endif

/*****************************************************************************/

static int iScintillaMapMethod(Ihandle* ih)
{
#ifdef GTK
  ih->handle = scintilla_new();
  if (!ih->handle)
    return IUP_ERROR;

  gtk_widget_show(ih->handle);

  /* add to the parent, all GTK controls must call this. */
  iupgtkAddToParent(ih);

  if (!iupAttribGetBoolean(ih, "CANFOCUS"))
    iupgtkSetCanFocus(ih->handle, 0);

  g_signal_connect(G_OBJECT(ih->handle), "enter-notify-event", G_CALLBACK(iupgtkEnterLeaveEvent), ih);
  g_signal_connect(G_OBJECT(ih->handle), "leave-notify-event", G_CALLBACK(iupgtkEnterLeaveEvent), ih);
  g_signal_connect(G_OBJECT(ih->handle), "focus-in-event",     G_CALLBACK(iupgtkFocusInOutEvent), ih);
  g_signal_connect(G_OBJECT(ih->handle), "focus-out-event",    G_CALLBACK(iupgtkFocusInOutEvent), ih);
  g_signal_connect(G_OBJECT(ih->handle), "key-press-event",    G_CALLBACK(iupgtkKeyPressEvent), ih);
  g_signal_connect(G_OBJECT(ih->handle), "show-help",          G_CALLBACK(iupgtkShowHelp), ih);

  g_signal_connect_after(G_OBJECT(ih->handle), "key-release-event", G_CALLBACK(gtkScintillaKeyReleaseEvent), ih);
  g_signal_connect(G_OBJECT(ih->handle), "button-press-event", G_CALLBACK(gtkScintillaButtonEvent), ih);  /* if connected "after" then it is ignored */
  g_signal_connect(G_OBJECT(ih->handle), "button-release-event", G_CALLBACK(gtkScintillaButtonEvent), ih);
  g_signal_connect(G_OBJECT(ih->handle), "motion-notify-event", G_CALLBACK(iupgtkMotionNotifyEvent), ih);

  g_signal_connect(G_OBJECT(ih->handle), "sci-notify", G_CALLBACK(gtkScintillaNotify), ih);

  gtk_widget_realize(ih->handle);
#else
  DWORD dwStyle = WS_CHILD | WS_CLIPSIBLINGS;
  DWORD dwExStyle = 0;

  if (!ih->parent)
    return IUP_ERROR;

  if (iupAttribGetBoolean(ih, "CANFOCUS"))
    dwStyle |= WS_TABSTOP;

  if (iupAttribGetBoolean(ih, "BORDER"))
    dwExStyle |= WS_EX_CLIENTEDGE;
  
  if (!iupwinCreateWindowEx(ih, "Scintilla", dwExStyle, dwStyle))
    return IUP_ERROR;

  /* Process Scintilla Notifications */
  IupSetCallback(ih, "_IUPWIN_NOTIFY_CB", (Icallback)winScintillaWmNotify);

  /* Process BUTTON_CB, MOTION_CB and CARET_CB */
  IupSetCallback(ih, "_IUPWIN_CTRLPROC_CB", (Icallback)winScintillaProc);
#endif

  /* configure for DROP of files */
  if (IupGetCallback(ih, "DROPFILES_CB"))
    iupAttribSetStr(ih, "DROPFILESTARGET", "YES");

  /* add scrollbar */
  if (ih->data->sb & IUP_SB_HORIZ)
    iupScintillaSendMessage(ih, SCI_SETHSCROLLBAR, 1, 0);
  else
    iupScintillaSendMessage(ih, SCI_SETHSCROLLBAR, 0, 0);

  if (ih->data->sb & IUP_SB_VERT)
    iupScintillaSendMessage(ih, SCI_SETVSCROLLBAR, 1, 0);
  else
    iupScintillaSendMessage(ih, SCI_SETVSCROLLBAR, 0, 0);

  IupSetCallback(ih, "_IUP_XY2POS_CB", (Icallback)iScintillaConvertXYToPos);
  IupSetCallback(ih, "_IUP_POS2LINCOL_CB", (Icallback)iupScintillaConvertPosToLinCol);
  IupSetCallback(ih, "_IUP_LINCOL2POS_CB", (Icallback)iupScintillaConvertLinColToPos);

  iupScintillaSendMessage(ih, SCI_SETPASTECONVERTENDINGS, 1, 0);
  iupScintillaSendMessage(ih, SCI_SETEOLMODE, SC_EOL_LF, 0);

  return IUP_NOERROR;
}

static void iScintillaComputeNaturalSizeMethod(Ihandle* ih, int *w, int *h, int *expand)
{
  int natural_w = 0, 
      natural_h = 0,
      visiblecolumns = iupAttribGetInt(ih, "VISIBLECOLUMNS"),
      visiblelines = iupAttribGetInt(ih, "VISIBLELINES");
  (void)expand; /* unset if not a container */

  iupdrvFontGetCharSize(ih, NULL, &natural_h);  /* one line height */
  natural_w = iupdrvFontGetStringWidth(ih, "WWWWWWWWWW");
  natural_w = (visiblecolumns*natural_w)/10;
  natural_h = visiblelines*natural_h;

  /* compute the borders space */
  if (iupAttribGetBoolean(ih, "BORDER"))
  {
#ifdef GTK
    int border_size = 2*5;
#else
    int border_size = 2*3;
#endif
    natural_w += border_size;
    natural_h += border_size;
  }

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
  ih->data->append_newline = 1;
  iupAttribSetStr(ih, "_IUP_MULTILINE_TEXT", "1");

  /* unused for now */
  ih->data->useBraceHLIndicator = 1;
  ih->data->useBraceBLIndicator = 1;
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
  iupClassRegisterCallback(ic, "SAVEPOINT_CB", "i");
  iupClassRegisterCallback(ic, "MARGINCLICK_CB", "iis");
  iupClassRegisterCallback(ic, "HOTSPOTCLICK_CB", "iiis");
  iupClassRegisterCallback(ic, "BUTTON_CB", "iiiis");
  iupClassRegisterCallback(ic, "MOTION_CB", "iis");
  iupClassRegisterCallback(ic, "CARET_CB", "iii");
  iupClassRegisterCallback(ic, "VALUECHANGED_CB", "");
  iupClassRegisterCallback(ic, "ACTION", "iiis");
  iupClassRegisterCallback(ic, "ZOOM_CB", "i");

  /* Common Callbacks */
  iupBaseRegisterCommonCallbacks(ic);

  /* Common */
  iupBaseRegisterCommonAttrib(ic);

  /* Visual */
  iupBaseRegisterVisualAttrib(ic);

  /* Drag&Drop */
  iupdrvRegisterDragDropAttrib(ic);

  /* Text retrieval and modification */
  iupClassRegisterAttribute(ic, "APPENDNEWLINE", iScintillaGetAppendNewlineAttrib, iScintillaSetAppendNewlineAttrib, IUPAF_SAMEASSYSTEM, "YES", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic,   "APPEND", NULL, iupScintillaSetAppendTextAttrib, NULL, NULL, IUPAF_WRITEONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic,   "PREPEND", NULL, iupScintillaSetPrependTextAttrib, NULL, NULL, IUPAF_WRITEONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic,   "VALUE", iupScintillaGetValueAttrib, iupScintillaSetValueAttrib, NULL, NULL, IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "INSERT", NULL, iupScintillaSetInsertTextAttrib, IUPAF_WRITEONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "LINE", iupScintillaGetLineAttrib, NULL, IUPAF_READONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "CHAR", iupScintillaGetCharAttrib, NULL, IUPAF_READONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic,   "DELETERANGE", NULL, iupScintillaSetDeleteRangeAttrib, NULL, NULL, IUPAF_WRITEONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic,   "READONLY", iupScintillaGetReadOnlyAttrib, iupScintillaSetReadOnlyAttrib, NULL, NULL, IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic,   "CLEARALL", NULL, iupScintillaSetClearAllAttrib, NULL, NULL, IUPAF_WRITEONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic,   "CLEARDOCUMENTSTYLE", NULL, iupScintillaSetClearDocumentAttrib, NULL, NULL, IUPAF_WRITEONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic,   "SAVEPOINT", NULL, iupScintillaSetSavePointAttrib, NULL, NULL, IUPAF_WRITEONLY|IUPAF_NO_INHERIT);

  /* Selection and information */
  iupClassRegisterAttribute(ic, "CARET", iupScintillaGetCaretAttrib, iupScintillaSetCaretAttrib, NULL, NULL, IUPAF_NO_SAVE|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "CARETPOS", iupScintillaGetCaretPosAttrib, iupScintillaSetCaretPosAttrib, IUPAF_SAMEASSYSTEM, "0", IUPAF_NO_SAVE|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "COUNT", iupScintillaGetCountAttrib, NULL, NULL, NULL, IUPAF_READONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "LINECOUNT", iupScintillaGetLineCountAttrib, NULL, NULL, NULL, IUPAF_READONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "LINEVALUE", iupScintillaGetCurrentLineAttrib, NULL, NULL, NULL, IUPAF_READONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "SELECTEDTEXT", iupScintillaGetSelectedTextAttrib, iupScintillaSetSelectedTextAttrib, NULL, NULL, IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "SELECTION", iupScintillaGetSelectionAttrib, iupScintillaSetSelectionAttrib, NULL, NULL, IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "SELECTIONPOS", iupScintillaGetSelectionPosAttrib, iupScintillaSetSelectionPosAttrib, NULL, NULL, IUPAF_NO_INHERIT);

  /* Cut, Copy and Paste */
  iupClassRegisterAttribute(ic, "CLIPBOARD", iupScintillaGetCanPasteAttrib, iupScintillaSetClipboardAttrib, NULL, NULL, IUPAF_NO_SAVE|IUPAF_NO_INHERIT);

  /* Undo, Redo */
  iupClassRegisterAttribute(ic, "UNDO", iupScintillaGetUndoAttrib, iupScintillaSetUndoAttrib, NULL, NULL, IUPAF_NO_SAVE|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "REDO", iupScintillaGetRedoAttrib, iupScintillaSetRedoAttrib, NULL, NULL, IUPAF_NO_SAVE|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "UNDOCOLLECT", iupScintillaGetUndoCollectAttrib, iupScintillaSetUndoCollectAttrib, IUPAF_SAMEASSYSTEM, "YES", IUPAF_NO_INHERIT);

  /* Overtype */
  iupClassRegisterAttribute(ic, "OVERWRITE", iupScintillaGetOvertypeAttrib, iupScintillaSetOvertypeAttrib, NULL, NULL, IUPAF_NO_INHERIT);

  /* Tabs and Indentation Guides */
  iupClassRegisterAttribute(ic, "TABSIZE", iupScintillaGetTabSizeAttrib, iupScintillaSetTabSizeAttrib, IUPAF_SAMEASSYSTEM, "8", IUPAF_DEFAULT);
  iupClassRegisterAttribute(ic, "INDENTATIONGUIDES", iupScintillaGetIndentationGuidesAttrib, iupScintillaSetIndentationGuidesAttrib, IUPAF_SAMEASSYSTEM, "NONE", IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "HIGHLIGHTGUIDE", iupScintillaGetHighlightGuideAttrib, iupScintillaSetHighlightGuideAttrib, NULL, NULL, IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "USETABS", iupScintillaGetUseTabsAttrib, iupScintillaSetUseTabsAttrib, IUPAF_SAMEASSYSTEM, "Yes", IUPAF_NO_INHERIT);

  /* Line wrapping */
  iupClassRegisterAttribute(ic, "WORDWRAP", iupScintillaGetWordWrapAttrib, iupScintillaSetWordWrapAttrib, NULL, NULL, IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "WORDWRAPVISUALFLAGS", iupScintillaGetWordWrapVisualFlagsAttrib, iupScintillaSetWordWrapVisualFlagsAttrib, IUPAF_SAMEASSYSTEM, "NONE", IUPAF_NO_INHERIT);

  /* Style Definition Attributes */
  iupClassRegisterAttribute(ic,   "STYLERESET", NULL, iupScintillaSetResetDefaultStyleAttrib, NULL, NULL, IUPAF_WRITEONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic,   "STYLECLEARALL", NULL, iupScintillaSetClearAllStyleAttrib, NULL, NULL, IUPAF_WRITEONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "STYLEFONT", iupScintillaGetFontStyleAttrib, iupScintillaSetFontStyleAttrib, IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "STYLEFONTSIZE", iupScintillaGetFontSizeStyleAttrib, iupScintillaSetFontSizeStyleAttrib, IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "STYLEFONTSIZEFRAC", iupScintillaGetFontSizeFracStyleAttrib, iupScintillaSetFontSizeFracStyleAttrib, IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "STYLEBOLD", iupScintillaGetBoldStyleAttrib, iupScintillaSetBoldStyleAttrib, IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "STYLEWEIGHT", iupScintillaGetWeightStyleAttrib, iupScintillaSetWeightStyleAttrib, IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "STYLEITALIC", iupScintillaGetItalicStyleAttrib, iupScintillaSetItalicStyleAttrib, IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "STYLEUNDERLINE", iupScintillaGetUnderlineStyleAttrib, iupScintillaSetUnderlineStyleAttrib, IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "STYLEFGCOLOR", iupScintillaGetFgColorStyleAttrib, iupScintillaSetFgColorStyleAttrib, IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "STYLEBGCOLOR", iupScintillaGetBgColorStyleAttrib, iupScintillaSetBgColorStyleAttrib, IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "STYLEEOLFILLED", iupScintillaGetEolFilledStyleAttrib, iupScintillaSetEolFilledStyleAttrib, IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "STYLECHARSET", iupScintillaGetCharSetStyleAttrib, iupScintillaSetCharSetStyleAttrib, IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "STYLECASE", iupScintillaGetCaseStyleAttrib, iupScintillaSetCaseStyleAttrib, IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "STYLEVISIBLE", iupScintillaGetVisibleStyleAttrib, iupScintillaSetVisibleStyleAttrib, IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "STYLEHOTSPOT", iupScintillaGetHotSpotStyleAttrib, iupScintillaSetHotSpotStyleAttrib, IUPAF_NO_INHERIT);

  /* Lexer Attributes */
  iupClassRegisterAttribute(ic,   "LEXERLANGUAGE", iupScintillaGetLexerLanguageAttrib, iupScintillaSetLexerLanguageAttrib, NULL, NULL, IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic,   "PROPERTYNAME", NULL, NULL, NULL, NULL, IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic,   "PROPERTY", iupScintillaGetPropertyAttrib, iupScintillaSetPropertyAttrib, NULL, NULL, IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "KEYWORDS", NULL, iupScintillaSetKeyWordsAttrib, IUPAF_WRITEONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic,   "PROPERTYNAMES", iupScintillaGetPropertyNamessAttrib, NULL, NULL, NULL, IUPAF_READONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic,   "KEYWORDSETS", iupScintillaGetDescribeKeywordSetsAttrib, NULL, NULL, NULL, IUPAF_READONLY|IUPAF_NO_INHERIT);

  /* Folding Attributes */
  iupClassRegisterAttribute(ic,   "FOLDFLAGS", NULL, iupScintillaSetFoldFlagsAttrib, NULL, NULL, IUPAF_WRITEONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic,   "FOLDTOGGLE", NULL, iupScintillaSetFoldToggleAttrib, NULL, NULL, IUPAF_WRITEONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "FOLDLEVEL", iupScintillaGetFoldLevelAttrib, iupScintillaSetFoldLevelAttrib, IUPAF_NO_INHERIT);

  /* Margin Attributes */
  iupClassRegisterAttributeId(ic, "MARGINTYPE", iupScintillaGetMarginTypeAttribId, iupScintillaSetMarginTypeAttribId, IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "MARGINWIDTH", iupScintillaGetMarginWidthAttribId, iupScintillaSetMarginWidthAttribId, IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "MARGINMASKFOLDERS", iupScintillaGetMarginMaskFoldersAttribId, iupScintillaSetMarginMaskFoldersAttribId, IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "MARGINSENSITIVE", iupScintillaGetMarginSensitiveAttribId, iupScintillaSetMarginSensitiveAttribId, IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic,   "MARGINLEFT", iupScintillaGetMarginLeftAttrib, iupScintillaSetMarginLeftAttrib, IUPAF_SAMEASSYSTEM, "1", IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic,   "MARGINRIGHT", iupScintillaGetMarginRightAttrib, iupScintillaSetMarginRightAttrib, IUPAF_SAMEASSYSTEM, "1", IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "MARGINTEXT", iupScintillaGetMarginTextAttribId, iupScintillaSetMarginTextAttribId, IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "MARGINTEXTSTYLE", iupScintillaGetMarginTextStyleAttribId, iupScintillaSetMarginTextStyleAttribId, IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic,   "MARGINTEXTCLEARALL", NULL, iupScintillaSetMarginTextClearAllAttrib, NULL, NULL, IUPAF_WRITEONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "MARGINCURSOR", iupScintillaGetMarginCursorAttribId, iupScintillaSetMarginCursorAttribId, IUPAF_NO_INHERIT);

  /* Marker Attributes */
  iupClassRegisterAttribute(ic, "MARKERDEFINE", NULL, iupScintillaSetMarkerDefineAttrib, NULL, NULL, IUPAF_WRITEONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "MARKERSYMBOL", iupScintillaGetMarkerSymbolAttribId, iupScintillaSetMarkerSymbolAttribId, IUPAF_NO_INHERIT);

  /* White space Attributes */
  iupClassRegisterAttribute(ic, "EXTRAASCENT",  iupScintillaGetWSExtraDescentAttrib, iupScintillaSetWSExtraDescentAttrib, NULL, NULL, IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "EXTRADESCENT", iupScintillaGetWSExtraAscentAttrib, iupScintillaSetWSExtraAscentAttrib, NULL, NULL, IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "WHITESPACEVIEW", iupScintillaGetViewWSAttrib, iupScintillaSetViewWSAttrib, IUPAF_SAMEASSYSTEM, "INVISIBLE", IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "WHITESPACESIZE", iupScintillaGetWSSizeAttrib, iupScintillaSetWSSizeAttrib, IUPAF_SAMEASSYSTEM, "1", IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "WHITESPACEFGCOLOR", NULL, iupScintillaSetWSFgColorAttrib, NULL, NULL, IUPAF_WRITEONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "WHITESPACEBGCOLOR", NULL, iupScintillaSetWSBgColorAttrib, NULL, NULL, IUPAF_WRITEONLY|IUPAF_NO_INHERIT);

  /* Brace highlighting Attributes */
  iupClassRegisterAttribute(ic, "BRACEHIGHLIGHT", NULL, iupScintillaSetBraceHighlightAttrib, NULL, NULL, IUPAF_WRITEONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "BRACEBADLIGHT",  NULL, iupScintillaSetBraceBadlightAttrib, NULL, NULL, IUPAF_WRITEONLY|IUPAF_NO_INHERIT);
  /* unused until we support Indicators */
/*  iupClassRegisterAttribute(ic, "BRACEHLINDICATOR", NULL, iupScintillaSetBraceHighlightIndicatorAttrib, NULL, NULL, IUPAF_WRITEONLY|IUPAF_NO_INHERIT);  */
/*  iupClassRegisterAttribute(ic, "BRACEBLINDICATOR", NULL, iupScintillaSetBraceBadlightIndicatorAttrib, NULL, NULL, IUPAF_WRITEONLY|IUPAF_NO_INHERIT);   */
/*  iupClassRegisterAttribute(ic, "USEBRACEHLINDICATOR", iupScintillaGetUseBraceHLIndicatorAttrib, iupScintillaSetUseBraceHLIndicatorAttrib, IUPAF_SAMEASSYSTEM, "YES", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);  */
/*  iupClassRegisterAttribute(ic, "USEBRACEBLINDICATOR", iupScintillaGetUseBraceBLIndicatorAttrib, iupScintillaSetUseBraceBLIndicatorAttrib, IUPAF_SAMEASSYSTEM, "YES", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);  */
  iupClassRegisterAttributeId(ic, "BRACEMATCH", iupScintillaGetBraceMatchAttribId, NULL, IUPAF_READONLY|IUPAF_NO_INHERIT);

  /* Cursor and Zooming Attributes */
  iupClassRegisterAttribute(ic, "CURSOR",  iupScintillaGetCursorAttrib, iupScintillaSetCursorAttrib, NULL, NULL, IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "ZOOMIN",  NULL, iupScintillaSetZoomInAttrib, NULL, NULL, IUPAF_WRITEONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "ZOOMOUT", NULL, iupScintillaSetZoomOutAttrib, NULL, NULL, IUPAF_WRITEONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "ZOOM",    iupScintillaGetZoomAttrib, iupScintillaSetZoomAttrib, NULL, NULL, IUPAF_NO_INHERIT);

  /* Scrolling and automatic scrolling */
  iupClassRegisterAttribute(ic, "SCROLLBAR", iScintillaGetScrollbarAttrib, iScintillaSetScrollbarAttrib, IUPAF_SAMEASSYSTEM, "YES", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "SCROLLTO", NULL, iupScintillaSetScrollToAttrib, NULL, NULL, IUPAF_WRITEONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "SCROLLTOPOS", NULL, iupScintillaSetScrollToPosAttrib, NULL, NULL, IUPAF_WRITEONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "SCROLLCARET", NULL, iupScintillaSetScrollCaretAttrib, NULL, NULL, IUPAF_WRITEONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "SCROLLWIDTH", iupScintillaGetScrollWidthAttrib, iupScintillaSetScrollWidthAttrib, IUPAF_SAMEASSYSTEM, "2000", IUPAF_NO_INHERIT);

  /* General */
  iupClassRegisterAttribute(ic, "VISIBLECOLUMNS", NULL, NULL, IUPAF_SAMEASSYSTEM, "30", IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "VISIBLELINES",   NULL, NULL, IUPAF_SAMEASSYSTEM, "10", IUPAF_NO_INHERIT);

  iupClassRegisterAttribute(ic, "BORDER", NULL, NULL, IUPAF_SAMEASSYSTEM, "YES", IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "MULTILINE", NULL, NULL, IUPAF_SAMEASSYSTEM, "YES", IUPAF_READONLY|IUPAF_NO_INHERIT);

  iupClassRegisterAttribute(ic, "USEPOPUP", NULL, iScintillaSetUsePopupAttrib, IUPAF_SAMEASSYSTEM, "YES", IUPAF_WRITEONLY|IUPAF_NO_INHERIT);

  return ic;
}

void IupScintillaOpen(void)
{
  if (!IupGetGlobal("_IUP_SCINTILLA_OPEN"))
  {
    iupRegisterClass(iupScintillaNewClass());
    IupSetGlobal("_IUP_SCINTILLA_OPEN", "1");

#ifndef GTK
    Scintilla_RegisterClasses(IupGetGlobal("HINSTANCE"));
#endif
  }
}

Ihandle *IupScintilla(void)
{
  return IupCreate("scintilla");
}

/*****  TODO
- Search & Replace
- Multiple Selection and Virtual Space
- Macro recording
- Printing
- Long lines
- Call tips
- Caret, selection, and hotspot styles
- Indicators
- Brace Highlighting with Indicators
BRACEHLINDICATOR (non inheritable, write only): defines a specified indicator to highlight matching braces instead of changing their style (See Indicator Styles).
BRACEBLINDICATOR (non inheritable, write only): defines a specified indicator to highlight non matching brace instead of changing its style (See Indicator Styles).
USEBRACEHLINDICATOR (non inheritable): enable or disable the indicator to highlight matching braces. Can be YES or NO. Default: YES.
USEBRACEBLINDICATOR (non inheritable): enable or disable the indicator to highlight non matching brace. Can be YES or NO. Default: YES.

- Annotations
- Autocompletion/User lists
- Markers

- FONT/BGCOLOR/FGCOLOR x STYLE*
- iupsci_selection.c
  iupsci_style.c
*/
