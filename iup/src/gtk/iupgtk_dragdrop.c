/** \file
 * \brief GTK Drag&Drop Functions
 *
 * See Copyright Notice in "iup.h"
 */

#include <stdio.h>              
#include <stdlib.h>
#include <string.h>             
#include <limits.h>             

#include <gtk/gtk.h>

#include "iup.h"
#include "iupcbs.h"

#include "iup_object.h"
#include "iup_str.h"
#include "iup_class.h"
#include "iup_attrib.h"
#include "iup_drv.h"

#include "iupgtk_drv.h"


#if 0
static gboolean gtkDragMotion(GtkWidget *widget, GdkDragContext *drag_context, gint x, gint y, guint time, Ihandle* ih)
{
  GdkModifierType mask;
  (void)ih;
  (void)x;
  (void)y;

  gdk_window_get_pointer(gtk_widget_get_window(widget), NULL, NULL, &mask);
  
  if (mask & GDK_CONTROL_MASK)
    gdk_drag_status(drag_context, GDK_ACTION_COPY, time);
  else
    gdk_drag_status(drag_context, GDK_ACTION_MOVE, time);

  return 0;
}

static void gtkDragDataGet(GtkWidget *widget, GdkDragContext *drag_context, GtkSelectionData *data, guint info, guint time, Ihandle* ih)
{
  IFnnsi cbDrag = (IFnnsi)IupGetCallback(ih, "DRAGSOURCE_CB");
  char* type = gdk_atom_name(gtk_selection_data_get_target(data));
  char* source = iupAttribGet(ih, "IUP_DRAG_DATA");
  int is_ctrl;
  GdkModifierType mask;

  if(!source)
    return;

  //TODO: pegar type, data e lenght da callback?

  //TODO: data is copied internally?
  gtk_selection_data_set(data, gtk_selection_data_get_target(data), 8, (guchar*)source, sizeof(source)/sizeof(source[0]));

  gdk_window_get_pointer(gtk_widget_get_window(widget), NULL, NULL, &mask);   // TODO: usar x e y?

  if (mask & GDK_CONTROL_MASK)
    is_ctrl = 1;  /* COPY */
  else
    is_ctrl = 0;  /* MOVE */

  if (cbDrag)
    cbDrag(ih, (Ihandle*)source, type, is_ctrl);

  /* Testing... */
  printf("DRAGSOURCE_CB ==> Ihandle* ih, Ihandle* source, Type: %s, 0=Move/1=Copy: %d\n", type, is_ctrl);

  (void)info;
  (void)drag_context;
  (void)time;
}

static void gtkDragDataReceived(GtkWidget *widget, GdkDragContext *drag_context, gint x, gint y,
                            GtkSelectionData *data, guint info, guint time, Ihandle *ih)
{
  IFnnsii cbDrop = (IFnnsii)IupGetCallback(ih, "DROPTARGET_CB");
  Ihandle* target;
  char* type = gdk_atom_name(gtk_selection_data_get_target(data));

  if(gtk_selection_data_get_length(data) <= 0 || gtk_selection_data_get_format(data) != 8)
  {
    gtk_drag_finish(drag_context, FALSE, FALSE, time);
    return;
  }

  target = (Ihandle*)gtk_selection_data_get_data(data);

  if(cbDrop)
    cbDrop(ih, target, type, x, y);   //TODO: raw data, lenght, type
                                      //TODO: opçoes para tipos pre-definidos?
  /* Testing... */
  printf("DROPTARGET_CB ==> Ihandle*: ih, Ihandle*: target, Type: %s, X: %d, Y: %d\n", type, x, y);

  (void)info;
  (void)widget;
  (void)time;
}

static GtkTargetList* gtkCreateTargetList(const char* value)
{
  GtkTargetList* targetlist = gtk_target_list_new((GtkTargetEntry*)NULL, 0);
  char valueCopy[256];
  char valueTemp[256];

  sprintf(valueCopy, "%s", value);
  while(iupStrToStrStr(valueCopy, valueTemp, valueCopy, ',') > 0)
  {
    gtk_target_list_add(targetlist, gdk_atom_intern(valueTemp, 0), 0, 0);

    if(iupStrEqualNoCase(valueCopy, valueTemp))
      break;
  }

  if (targetlist->ref_count == 0)
  {
    gtk_target_list_unref(targetlist);
    return NULL;
  }

  return targetlist;
}

static int gtkSetDropTypesAttrib(Ihandle* ih, const char* value)
{
  GtkTargetList *targetlist = (GtkTargetList*)iupAttribGet(ih, "_IUPGTK_DROP_TARGETLIST");
  if (targetlist)
  {
    gtk_target_list_unref(targetlist);
    iupAttribSetStr(ih, "_IUPGTK_DROP_TARGETLIST", NULL);
  }

  if(!value)
    return 0;

  targetlist = gtkCreateTargetList(value);
  iupAttribSetStr(ih, "_IUPGTK_DROP_TARGETLIST", (char*)targetlist);
  return 1;
}

static int gtkSetDropTargetAttrib(Ihandle* ih, const char* value)
{
  if(iupStrBoolean(value))
  {
    GtkTargetList *targetlist = (GtkTargetList*)iupAttribGet(ih, "_IUPGTK_DROP_TARGETLIST");
    GtkTargetEntry *drop_types_entry;
    int size;

    if(!targetlist)
      return 0;

    drop_types_entry = gtk_target_table_new_from_list(targetlist, &size);

    gtk_drag_dest_set(ih->handle, GTK_DEST_DEFAULT_ALL, drop_types_entry, size, GDK_ACTION_MOVE|GDK_ACTION_COPY);

    g_signal_connect(ih->handle, "drag_motion", G_CALLBACK(gtkDragMotion), ih);
    g_signal_connect(ih->handle, "drag_data_received", G_CALLBACK(gtkDragDataReceived), ih);

    //TODO: release drop_types_entry?
  }
  else
    gtk_drag_dest_unset(ih->handle);

  return 1;
}

static int gtkSetDragTypesAttrib(Ihandle* ih, const char* value)
{
  GtkTargetList *targetlist = (GtkTargetList*)iupAttribGet(ih, "_IUPGTK_DRAG_TARGETLIST");
  if (targetlist)
  {
    gtk_target_list_unref(targetlist);
    iupAttribSetStr(ih, "_IUPGTK_DRAG_TARGETLIST", NULL);
  }

  if (!value)
    return 0;

  targetlist = gtkCreateTargetList(value);
  iupAttribSetStr(ih, "_IUPGTK_DRAG_TARGETLIST", (char*)targetlist);
  return 1;
}

static int gtkSetDragSourceAttrib(Ihandle* ih, const char* value)
{
  if (iupStrBoolean(value))
  {
    GtkTargetList *targetlist = (GtkTargetList*)iupAttribGet(ih, "_IUPGTK_DRAG_TARGETLIST");
    GtkTargetEntry *drag_types_entry;
    int size;

    if(!targetlist)
      return 0;

    drag_types_entry = gtk_target_table_new_from_list(targetlist, &size);

    gtk_drag_source_set(ih->handle, GDK_BUTTON1_MASK, drag_types_entry, size, GDK_ACTION_MOVE|GDK_ACTION_COPY);

    g_signal_connect(ih->handle, "drag_data_get", G_CALLBACK(gtkDragDataGet), ih);

    //TODO: release drag_types_entry?
  }
  else
    gtk_drag_source_unset(ih->handle);

  return 1;
}
#endif

static void gtkDropFileDragDataReceived(GtkWidget* w, GdkDragContext* context, int x, int y,
                                        GtkSelectionData* seldata, guint info, guint time, Ihandle* ih)
{
  gchar **uris = NULL, *data = NULL;
  int i, count;

  IFnsiii cb = (IFnsiii)IupGetCallback(ih, "DROPFILES_CB");
  if (!cb) return; 

#if GTK_CHECK_VERSION(2, 6, 0)
#if GTK_CHECK_VERSION(2, 14, 0)
  data = (char*)gtk_selection_data_get_data(seldata);
#else
  data = (char*)seldata->data;
#endif
  uris = g_uri_list_extract_uris(data);
#endif

  if (!uris)
    return;

  count = 0;
  while (uris[count])
    count++;

  for (i=0; i<count; i++)
  {
    char* filename = uris[i];
    if (iupStrEqualPartial(filename, "file://"))
    {
      filename += strlen("file://");
      if (filename[2] == ':') /* in Windows there is an extra '/' at the beginning. */
        filename++;
    }
    if (cb(ih, filename, count-i-1, x, y) == IUP_IGNORE)
      break;
  }

  g_strfreev (uris);
  (void)time;
  (void)info;
  (void)w;
  (void)context;
}

static int gtkSetDropFilesTargetAttrib(Ihandle* ih, const char* value)
{
  if (iupStrBoolean(value))
  {
    GtkTargetEntry dragtypes[] = { { "text/uri-list", 0, 0 } };
    gtk_drag_dest_set(ih->handle, GTK_DEST_DEFAULT_ALL, dragtypes,
                      sizeof(dragtypes) / sizeof(dragtypes[0]), GDK_ACTION_COPY);
    g_signal_connect(G_OBJECT(ih->handle), "drag_data_received", G_CALLBACK(gtkDropFileDragDataReceived), ih);
  }
  else
    gtk_drag_dest_unset(ih->handle);

  return 1;
}

void iupdrvRegisterDragDropAttrib(Iclass* ic)
{
  iupClassRegisterCallback(ic, "DROPFILES_CB", "siii");

  // iupClassRegisterCallback(ic, "DRAGSOURCE_CB", "hsi");
  // iupClassRegisterCallback(ic, "DROPTARGET_CB", "hsii");

  // iupClassRegisterAttribute(ic, "DRAGTYPES",  NULL, gtkSetDragTypesAttrib, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  // iupClassRegisterAttribute(ic, "DROPTYPES",  NULL, gtkSetDropTypesAttrib, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  // iupClassRegisterAttribute(ic, "DRAGSOURCE", NULL, gtkSetDragSourceAttrib, NULL, NULL, IUPAF_NO_INHERIT);
  // iupClassRegisterAttribute(ic, "DROPTARGET", NULL, gtkSetDropTargetAttrib, NULL, NULL, IUPAF_NO_INHERIT);

  iupClassRegisterAttribute(ic, "DRAGDROP", NULL, gtkSetDropFilesTargetAttrib, NULL, NULL, IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "DROPFILESTARGET", NULL, gtkSetDropFilesTargetAttrib, NULL, NULL, IUPAF_NO_INHERIT);
}

/* TODO:
   controle de copy/move
   default target types: text, image, uri
   dragicon
*/
