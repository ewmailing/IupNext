/** \file
 * \brief GTK Focus
 *
 * See Copyright Notice in "iup.h"
 */

#include <gtk/gtk.h>

#include <stdio.h>

#include "iup.h"

#include "iup_object.h"
#include "iup_focus.h"
#include "iup_attrib.h"
#include "iup_drv.h"
#include "iup_assert.h" 
#include "iup_drv.h" 

#include "iupgtk_drv.h"


void iupdrvSetFocus(Ihandle *ih)
{
  Ihandle* dialog = IupGetDialog(ih);
  if (!gtk_window_is_active((GtkWindow*)dialog->handle))
    gdk_window_focus(dialog->handle->window, gtk_get_current_event_time());
  gtk_widget_grab_focus(ih->handle);
}

gboolean iupgtkFocusInOutEvent(GtkWidget *widget, GdkEventFocus *evt, Ihandle *ih)
{
  (void)widget;

  if (evt->in)
  {
    /* even when ACTIVE=NO the dialog gets this evt */
    if (!iupdrvIsActive(ih))
      return TRUE;

    iupCallGetFocusCb(ih);
  }
  else
    iupCallKillFocusCb(ih);

  return FALSE;
}
