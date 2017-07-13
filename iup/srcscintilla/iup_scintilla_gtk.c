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

#include "Scintilla.h"
#include "SciLexer.h"

#include <gtk/gtk.h>
#include <ScintillaWidget.h>

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

#include "iupgtk_drv.h"

#include "iupsci.h"


void iupdrvScintillaRefreshCaret(Ihandle* ih)
{
  (void)ih;
}

int iupdrvScintillaGetBorder(void)
{
  int border_size = 2 * 5;
  return border_size;
}

void iupdrvScintillaOpen(void)
{
}

sptr_t IupScintillaSendMessage(Ihandle* ih, unsigned int iMessage, uptr_t wParam, sptr_t lParam)
{
  return scintilla_send_message(SCINTILLA(ih->handle), iMessage, wParam, lParam);
}

static void gtkScintillaNotify(GtkWidget *w, gint wp, gpointer lp, Ihandle *ih)
{
  SCNotification *pMsg =(SCNotification*)lp;

  iupScintillaNotify(ih, pMsg);

  (void)w;
  (void)wp;
}

static gboolean gtkScintillaKeyReleaseEvent(GtkWidget *widget, GdkEventKey *evt, Ihandle *ih)
{
  iupScintillaCallCaretCb(ih);
  (void)widget;
  (void)evt;
  return FALSE;
}

static gboolean gtkScintillaButtonEvent(GtkWidget *widget, GdkEventButton *evt, Ihandle *ih)
{
  iupScintillaCallCaretCb(ih);
  return iupgtkButtonEvent(widget, evt, ih);
}

/*****************************************************************************/

int idrvScintillaMap(Ihandle* ih)
{
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

  return IUP_NOERROR;
}

void iupdrvScintillaReleaseMethod(Iclass* ic)
{
  (void)ic;
}
