/** \file
 * \brief Canvas Control
 *
 * See Copyright Notice in "iup.h"
 */

#include <gtk/gtk.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <stdarg.h>
#include <limits.h>

#include "iup.h"
#include "iupcbs.h"

#include "iup_object.h"
#include "iup_layout.h"
#include "iup_attrib.h"
#include "iup_dialog.h"
#include "iup_str.h"
#include "iup_drv.h"
#include "iup_drvinfo.h"
#include "iup_drvfont.h"
#include "iup_canvas.h"
#include "iup_key.h"

#include "iupgtk_drv.h"


static int gtkCanvasScroll2Iup(GtkScrollType scroll, int vert)
{
  switch(scroll)
  {
    case GTK_SCROLL_STEP_UP:
      return IUP_SBUP;
    case GTK_SCROLL_STEP_DOWN:
      return IUP_SBDN;
    case GTK_SCROLL_PAGE_UP:
      return IUP_SBPGUP;
    case GTK_SCROLL_PAGE_DOWN:
      return IUP_SBPGDN;
    case GTK_SCROLL_STEP_LEFT:
      return IUP_SBLEFT;
    case GTK_SCROLL_STEP_RIGHT:
      return IUP_SBRIGHT;
    case GTK_SCROLL_PAGE_LEFT:
      return IUP_SBPGLEFT;
    case GTK_SCROLL_PAGE_RIGHT:
      return IUP_SBPGRIGHT;
    case GTK_SCROLL_STEP_BACKWARD:
      return vert? IUP_SBUP: IUP_SBLEFT;
    case GTK_SCROLL_STEP_FORWARD:
      return vert? IUP_SBDN: IUP_SBRIGHT;
    case GTK_SCROLL_PAGE_BACKWARD:
      return vert? IUP_SBPGUP: IUP_SBPGLEFT;
    case GTK_SCROLL_PAGE_FORWARD:
      return vert? IUP_SBPGDN: IUP_SBPGRIGHT;
    case GTK_SCROLL_JUMP:
    case GTK_SCROLL_START:
    case GTK_SCROLL_END:
      return vert? IUP_SBPOSV: IUP_SBPOSH;
    case GTK_SCROLL_NONE:
      return -1;
  }

  /* No IUP_SBDRAGV or IUP_SBDRAGH support in GTK */

  return -1;
}

static gboolean gtkCanvasHChangeValue(GtkRange *range, GtkScrollType scroll, double value, Ihandle *ih)
{
  double posx, posy;
  IFniff cb;

  double xmin = iupAttribGetFloat(ih, "XMIN");
  double xmax = iupAttribGetFloat(ih, "XMAX");
  double dx = iupAttribGetFloat(ih, "DX");
  if (value < xmin) value = xmin;
  if (value > xmax-dx) value = xmax-dx;

  posx = value;
  ih->data->posx = (float)posx;
  posy = ih->data->posy;

  cb = (IFniff)IupGetCallback(ih,"SCROLL_CB");
  if (cb)
  {
    int op = gtkCanvasScroll2Iup(scroll, 0);
    if (op == -1)
      return FALSE;

    cb(ih, op, (float)posx, (float)posy);
  }
  else
  {
    IFnff cb = (IFnff)IupGetCallback(ih,"ACTION");
    if (cb)
      cb (ih, (float)posx, (float)posy);
  }

  (void)range;
  return FALSE;
}

static gboolean gtkCanvasVChangeValue(GtkRange *range, GtkScrollType scroll, double value, Ihandle *ih)
{
  double posx, posy;
  IFniff cb;

  double ymin = iupAttribGetFloat(ih, "YMIN");
  double ymax = iupAttribGetFloat(ih, "YMAX");
  double dy = iupAttribGetFloat(ih, "DY");
  if (value < ymin) value = ymin;
  if (value > ymax-dy) value = ymax-dy;

  posy = value;
  ih->data->posy = (float)posy;
  posx = ih->data->posx;

  cb = (IFniff)IupGetCallback(ih,"SCROLL_CB");
  if (cb)
  {
    int op = gtkCanvasScroll2Iup(scroll, 1);
    if (op == -1)
      return FALSE;

    cb(ih, op, (float)posx, (float)posy);
  }
  else
  {
    IFnff cb = (IFnff)IupGetCallback(ih,"ACTION");
    if (cb)
      cb (ih, (float)posx, (float)posy);
  }

  (void)range;
  return FALSE;
}

static gboolean gtkCanvasButtonEvent(GtkWidget *widget, GdkEventButton *evt, Ihandle *ih)
{
  if (evt->type == GDK_BUTTON_PRESS)
  {
    /* Force focus on canvas click */
    gtk_widget_grab_focus(ih->handle);
  }

  iupgtkButtonEvent(widget, evt, ih);

  if ((evt->type == GDK_BUTTON_PRESS) && (evt->button==4 || evt->button==5))
  {                                             
    IFnfiis wcb = (IFnfiis)IupGetCallback(ih, "WHEEL_CB");
    if (wcb)
    {
      int delta = evt->button==4? 1: -1;
      char status[IUPKEY_STATUS_SIZE] = IUPKEY_STATUS_INIT;
      iupgtkButtonKeySetStatus(evt->state, evt->button, status, 0);

      wcb(ih, (float)delta, (int)evt->x, (int)evt->y, status);
    }
    else
    {
      IFniff scb = (IFniff)IupGetCallback(ih,"SCROLL_CB");
      float posy = ih->data->posy;
      int delta = evt->button==4? 1: -1;
      int op = evt->button==4? IUP_SBUP: IUP_SBDN;
      posy -= delta*iupAttribGetFloat(ih, "DY")/10.0f;
      IupSetfAttribute(ih, "POSY", "%g", posy);
      if (scb)
        scb(ih,op,ih->data->posx,ih->data->posy);
    }
  }

  return FALSE;
}

static void gtkCanvasExposeChild(GtkWidget *child, gpointer client_data)
{
  struct {
    GtkWidget *container;
    GdkEventExpose *evt;
  } *data = client_data;
  
  gtk_container_propagate_expose(GTK_CONTAINER(data->container), child, data->evt);
}

static int gtkCanvasSetBgColorAttrib(Ihandle* ih, const char* value);

static gboolean gtkCanvasExposeEvent(GtkWidget *widget, GdkEventExpose *evt, Ihandle *ih)
{
  IFnff cb = (IFnff)IupGetCallback(ih,"ACTION");
  if (cb)
  {
    if (!iupAttribGet(ih, "_IUPGTK_NO_BGCOLOR"))
      gtkCanvasSetBgColorAttrib(ih, iupAttribGetStr(ih, "BGCOLOR"));  /* reset to update window attributes */

    iupAttribSetStrf(ih, "CLIPRECT", "%d %d %d %d", evt->area.x, evt->area.y, evt->area.x+evt->area.width-1, evt->area.y+evt->area.height-1);
    cb(ih,ih->data->posx,ih->data->posy);
    iupAttribSetStr(ih, "CLIPRECT", NULL);
  }

  if (ih->iclass->childtype != IUP_CHILDNONE) /* canvas can be a container */
  {
    struct {
      GtkWidget *container;
      GdkEventExpose *evt;
    } data;
    data.container = widget;
    data.evt = evt;

    gtk_container_foreach(GTK_CONTAINER (widget), gtkCanvasExposeChild, &data);
    return FALSE;
  }

  return TRUE;  /* stop other handlers */
}

static gboolean gtkCanvasConfigureEvent(GtkWidget *widget, GdkEventConfigure *evt, Ihandle *ih)
{
  IFnii cb = (IFnii)IupGetCallback(ih,"RESIZE_CB");
  if (cb)
    cb(ih,evt->width,evt->height);

  (void)widget;
  return FALSE;
}

static GtkScrolledWindow* gtkCanvasGetScrolledWindow(Ihandle* ih)
{
  return (GtkScrolledWindow*)iupAttribGet(ih, "_IUP_EXTRAPARENT");
}

static int gtkCanvasSetXAutoHideAttrib(Ihandle* ih, const char *value)
{
  GtkPolicyType vscrollbar_policy;
  gtk_scrolled_window_get_policy(gtkCanvasGetScrolledWindow(ih), NULL, &vscrollbar_policy);

  if (ih->data->sb & IUP_SB_HORIZ)
  {
    GtkPolicyType hscrollbar_policy;

    if (iupStrBoolean(value))
      hscrollbar_policy = GTK_POLICY_AUTOMATIC;
    else
      hscrollbar_policy = GTK_POLICY_ALWAYS;

    gtk_scrolled_window_set_policy(gtkCanvasGetScrolledWindow(ih), hscrollbar_policy, vscrollbar_policy);
  }
  else
    gtk_scrolled_window_set_policy(gtkCanvasGetScrolledWindow(ih), GTK_POLICY_NEVER, vscrollbar_policy);

  return 1;
}

static int gtkCanvasSetYAutoHideAttrib(Ihandle* ih, const char *value)
{
  GtkPolicyType hscrollbar_policy;
  gtk_scrolled_window_get_policy(gtkCanvasGetScrolledWindow(ih), &hscrollbar_policy, NULL);

  if (ih->data->sb & IUP_SB_VERT)
  {
    GtkPolicyType vscrollbar_policy;

    if (iupStrBoolean(value))
      vscrollbar_policy = GTK_POLICY_AUTOMATIC;
    else
      vscrollbar_policy = GTK_POLICY_ALWAYS;

    gtk_scrolled_window_set_policy(gtkCanvasGetScrolledWindow(ih), hscrollbar_policy, vscrollbar_policy);
  }
  else
    gtk_scrolled_window_set_policy(gtkCanvasGetScrolledWindow(ih), hscrollbar_policy, GTK_POLICY_NEVER);

  return 1;
}

static int gtkCanvasCheckScroll(double min, double max, double *page, double *pos)
{
  double old_pos = *pos;
  double range = max-min;
  if (*page > range) *page = range;
  if (*page <= 0) *page = range/10.;

  if (*pos < min) *pos = min;
  if (*pos > (max - *page)) *pos = max - *page;

  if (old_pos == *pos)
    return 0;
  else
    return 1;
}

static int gtkCanvasSetDXAttrib(Ihandle* ih, const char *value)
{
  if (ih->data->sb & IUP_SB_HORIZ)
  {
    double xmin, xmax, linex;
    float dx;
    int value_changed;
    GtkAdjustment* sb_horiz = gtk_scrolled_window_get_hadjustment(gtkCanvasGetScrolledWindow(ih));
    if (!sb_horiz) return 1;

    if (!iupStrToFloat(value, &dx))
      return 1;

    xmin = iupAttribGetFloat(ih, "XMIN");
    xmax = iupAttribGetFloat(ih, "XMAX");

    if (!iupAttribGet(ih,"LINEX"))
    {
      linex = dx/10;
      if (linex==0)
        linex = 1;
    }
    else
      linex = iupAttribGetFloat(ih,"LINEX");

    sb_horiz->lower = xmin;
    sb_horiz->upper = xmax;
    sb_horiz->step_increment = linex;
    sb_horiz->page_size = dx;

    value_changed = gtkCanvasCheckScroll(xmin, xmax, &sb_horiz->page_size, &sb_horiz->value);
    sb_horiz->page_increment = sb_horiz->page_size;

    gtk_adjustment_changed(sb_horiz);

    if (value_changed)
      gtk_adjustment_value_changed(sb_horiz);
  }
  return 1;
}

static int gtkCanvasSetDYAttrib(Ihandle* ih, const char *value)
{
  if (ih->data->sb & IUP_SB_VERT)
  {
    double ymin, ymax, liney;
    float dy;
    int value_changed;
    GtkAdjustment* sb_vert = gtk_scrolled_window_get_vadjustment(gtkCanvasGetScrolledWindow(ih));
    if (!sb_vert) return 1;

    if (!iupStrToFloat(value, &dy))
      return 1;

    ymin = iupAttribGetFloat(ih, "YMIN");
    ymax = iupAttribGetFloat(ih, "YMAX");

    if (!iupAttribGet(ih,"LINEY"))
    {
      liney = dy/10;
      if (liney==0)
        liney = 1;
    }
    else
      liney = iupAttribGetFloat(ih,"LINEY");

    sb_vert->lower = ymin;
    sb_vert->upper = ymax;
    sb_vert->step_increment = liney;
    sb_vert->page_size = dy;

    value_changed = gtkCanvasCheckScroll(ymin, ymax, &sb_vert->page_size, &sb_vert->value);
    sb_vert->page_increment = sb_vert->page_size;

    gtk_adjustment_changed(sb_vert);

    if (value_changed)
      gtk_adjustment_value_changed(sb_vert);
  }
  return 1;
}

static int gtkCanvasSetPosXAttrib(Ihandle* ih, const char *value)
{
  if (ih->data->sb & IUP_SB_HORIZ)
  {
    float posx;
    GtkAdjustment* sb_horiz = gtk_scrolled_window_get_hadjustment(gtkCanvasGetScrolledWindow(ih));
    if (!sb_horiz) return 1;

    if (!iupStrToFloat(value, &posx))
      return 1;

    gtk_adjustment_set_value(sb_horiz, posx);
  }
  return 1;
}

static int gtkCanvasSetPosYAttrib(Ihandle* ih, const char *value)
{
  if (ih->data->sb & IUP_SB_VERT)
  {
    float posy;
    GtkAdjustment* sb_vert = gtk_scrolled_window_get_vadjustment(gtkCanvasGetScrolledWindow(ih));
    if (!sb_vert) return 1;

    if (!iupStrToFloat(value, &posy))
      return 1;

    gtk_adjustment_set_value(sb_vert, posy);
  }
  return 1;
}

static int gtkCanvasSetBgColorAttrib(Ihandle* ih, const char* value)
{
  GtkScrolledWindow* scrolled_window = gtkCanvasGetScrolledWindow(ih);
  unsigned char r, g, b;

  /* ignore given value, must use only from parent for the scrollbars */
  char* parent_value = iupBaseNativeParentGetBgColor(ih);

  if (iupStrToRGB(parent_value, &r, &g, &b))
  {
    GtkWidget* sb;

    iupgtkBaseSetBgColor((GtkWidget*)scrolled_window, r, g, b);

#if GTK_CHECK_VERSION(2, 8, 0)
    sb = gtk_scrolled_window_get_hscrollbar(scrolled_window);
    if (sb) iupgtkBaseSetBgColor(sb, r, g, b);
    sb = gtk_scrolled_window_get_vscrollbar(scrolled_window);
    if (sb) iupgtkBaseSetBgColor(sb, r, g, b);
#endif
  }

  if (!IupGetCallback(ih, "ACTION")) 
  {
    /* enable automatic double buffering */
    gtk_widget_set_double_buffered(ih->handle, TRUE);
    gtk_widget_set_double_buffered((GtkWidget*)scrolled_window, TRUE);
    return iupdrvBaseSetBgColorAttrib(ih, value);
  }
  else
  {
    /* disable automatic double buffering */
    gtk_widget_set_double_buffered(ih->handle, FALSE);
    gtk_widget_set_double_buffered((GtkWidget*)scrolled_window, FALSE);
    gdk_window_set_back_pixmap(ih->handle->window, NULL, FALSE);
    iupAttribSetStr(ih, "_IUPGTK_NO_BGCOLOR", "1");
    return 1;
  }
}

static void gtkCanvasLayoutUpdateMethod(Ihandle *ih)
{
  iupdrvBaseLayoutUpdateMethod(ih);

  if (ih->iclass->childtype != IUP_CHILDNONE)
  {
    IFnii cb = (IFnii)IupGetCallback(ih,"RESIZE_CB");
    if (cb)
      cb(ih,ih->currentwidth, ih->currentheight);
  }
}

static int gtkCanvasMapMethod(Ihandle* ih)
{
  GtkScrolledWindow* scrolled_window;
  char *visual;
  GdkColormap* default_colormap = NULL;

  if (!ih->parent)
    return IUP_ERROR;

  ih->data->sb = iupBaseGetScrollbar(ih);

  visual = IupGetAttribute(ih, "VISUAL");   /* defined by the OpenGL Canvas in X11 or NULL */
  if (visual)
  {
    GdkColormap* colormap = (GdkColormap*)iupgtkGetColormapFromVisual(visual, (void*)iupAttribGet(ih, "COLORMAP"));
    default_colormap = gtk_widget_get_default_colormap();
    gtk_widget_set_default_colormap(colormap);
  }

  if (ih->iclass->childtype != IUP_CHILDNONE) /* canvas can be a container */
  {
    ih->handle = gtk_fixed_new();
    gtk_fixed_set_has_window((GtkFixed*)ih->handle, TRUE);
    gtk_widget_set_app_paintable(ih->handle, TRUE);
  }
  else
    ih->handle = gtk_drawing_area_new();

  if (visual)
    gtk_widget_set_default_colormap(default_colormap);

  if (!ih->handle)
      return IUP_ERROR;

  scrolled_window = (GtkScrolledWindow*)gtk_scrolled_window_new(NULL, NULL);
  if (!scrolled_window)
    return IUP_ERROR;

  gtk_container_add((GtkContainer*)scrolled_window, ih->handle);
  gtk_widget_show((GtkWidget*)scrolled_window);

  iupAttribSetStr(ih, "_IUP_EXTRAPARENT", (char*)scrolled_window);

  /* add to the parent, all GTK controls must call this. */
  iupgtkBaseAddToParent(ih);

  g_signal_connect(G_OBJECT(ih->handle), "focus-in-event",     G_CALLBACK(iupgtkFocusInOutEvent), ih);
  g_signal_connect(G_OBJECT(ih->handle), "focus-out-event",    G_CALLBACK(iupgtkFocusInOutEvent), ih);
  g_signal_connect(G_OBJECT(ih->handle), "key-press-event",    G_CALLBACK(iupgtkKeyPressEvent), ih);
  g_signal_connect(G_OBJECT(ih->handle), "key-release-event",  G_CALLBACK(iupgtkKeyReleaseEvent), ih);
  g_signal_connect(G_OBJECT(ih->handle), "enter-notify-event", G_CALLBACK(iupgtkEnterLeaveEvent), ih);
  g_signal_connect(G_OBJECT(ih->handle), "leave-notify-event", G_CALLBACK(iupgtkEnterLeaveEvent), ih);
  g_signal_connect(G_OBJECT(ih->handle), "show-help",          G_CALLBACK(iupgtkShowHelp), ih);

  g_signal_connect(G_OBJECT(ih->handle), "expose-event",       G_CALLBACK(gtkCanvasExposeEvent), ih);
  g_signal_connect(G_OBJECT(ih->handle), "button-press-event", G_CALLBACK(gtkCanvasButtonEvent), ih);
  g_signal_connect(G_OBJECT(ih->handle), "button-release-event",G_CALLBACK(gtkCanvasButtonEvent), ih);
  g_signal_connect(G_OBJECT(ih->handle), "motion-notify-event",G_CALLBACK(iupgtkMotionNotifyEvent), ih);

#if GTK_CHECK_VERSION(2, 8, 0)
  g_signal_connect(G_OBJECT(gtk_scrolled_window_get_hscrollbar(scrolled_window)), "change-value",G_CALLBACK(gtkCanvasHChangeValue), ih);
  g_signal_connect(G_OBJECT(gtk_scrolled_window_get_vscrollbar(scrolled_window)), "change-value",G_CALLBACK(gtkCanvasVChangeValue), ih);
#endif

  /* To receive mouse events on a drawing area, you will need to enable them. */
  gtk_widget_add_events(ih->handle, GDK_EXPOSURE_MASK|
    GDK_POINTER_MOTION_MASK|GDK_BUTTON_MOTION_MASK|
    GDK_BUTTON_PRESS_MASK|GDK_BUTTON_RELEASE_MASK|
    GDK_KEY_PRESS_MASK|GDK_KEY_RELEASE_MASK|
    GDK_ENTER_NOTIFY_MASK|GDK_LEAVE_NOTIFY_MASK|
    GDK_FOCUS_CHANGE_MASK|GDK_STRUCTURE_MASK);

  /* To receive keyboard events, you will need to set the GTK_CAN_FOCUS flag on the drawing area. */
  if (ih->iclass->is_interactive)
  {
    if (iupStrBoolean(iupAttribGetStr(ih, "CANFOCUS")))
      GTK_WIDGET_FLAGS(ih->handle) |= GTK_CAN_FOCUS;
  }

  if (IupGetInt(ih, "BORDER"))              /* Use IupGetInt for inheritance */
    gtk_scrolled_window_set_shadow_type(scrolled_window, GTK_SHADOW_IN); 
  else
    gtk_scrolled_window_set_shadow_type(scrolled_window, GTK_SHADOW_NONE);

  gtk_widget_realize((GtkWidget*)scrolled_window);
  gtk_widget_realize(ih->handle);

  if (ih->iclass->childtype == IUP_CHILDNONE)
  {
    /* must be connected after realize or a RESIZE_CB will happen before MAP_CB
      works only for the GtkDrawingArea. */
    g_signal_connect(G_OBJECT(ih->handle), "configure-event", G_CALLBACK(gtkCanvasConfigureEvent), ih);
  }

  /* ensure the default values, that are different from the native ones */
  gtkCanvasSetXAutoHideAttrib(ih, iupAttribGetStr(ih, "XAUTOHIDE"));
  gtkCanvasSetYAutoHideAttrib(ih, iupAttribGetStr(ih, "YAUTOHIDE"));
  gtkCanvasSetDXAttrib(ih, iupAttribGetStr(ih, "DX"));
  gtkCanvasSetDYAttrib(ih, iupAttribGetStr(ih, "DY"));
  gtkCanvasSetPosXAttrib(ih, iupAttribGetStr(ih, "POSX"));
  gtkCanvasSetPosYAttrib(ih, iupAttribGetStr(ih, "POSY"));
  gtkCanvasSetBgColorAttrib(ih, iupAttribGetStr(ih, "BGCOLOR"));

  /* configure for DRAG&DROP */
  if (IupGetCallback(ih, "DROPFILES_CB"))
    iupAttribSetStr(ih, "DRAGDROP", "YES");

  /* update a mnemonic in a label if necessary */
  iupgtkUpdateMnemonic(ih);
    
  return IUP_NOERROR;
}

void iupdrvCanvasInitClass(Iclass* ic)
{
  /* Driver Dependent Class functions */
  ic->Map = gtkCanvasMapMethod;
  ic->LayoutUpdate = gtkCanvasLayoutUpdateMethod;

  /* Driver Dependent Attribute functions */

  /* Visual */
  iupClassRegisterAttribute(ic, "BGCOLOR", NULL, gtkCanvasSetBgColorAttrib, "255 255 255", NULL, IUPAF_DEFAULT);

  /* IupCanvas only */
  iupClassRegisterAttribute(ic, "DRAGDROP", NULL, iupgtkSetDragDropAttrib, NULL, NULL, IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "CURSOR", NULL, iupdrvBaseSetCursorAttrib, "ARROW", NULL, IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "DRAWSIZE", iupdrvBaseGetClientSizeAttrib, NULL, NULL, NULL, IUPAF_READONLY|IUPAF_NO_INHERIT);

  iupClassRegisterAttribute(ic, "DX", NULL, gtkCanvasSetDXAttrib, "0.1", NULL, IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "DY", NULL, gtkCanvasSetDYAttrib, "0.1", NULL, IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "POSX", iupCanvasGetPosXAttrib, gtkCanvasSetPosXAttrib, "0.0", NULL, IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "POSY", iupCanvasGetPosYAttrib, gtkCanvasSetPosYAttrib, "0.0", NULL, IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "XAUTOHIDE", NULL, gtkCanvasSetXAutoHideAttrib, "YES", NULL, IUPAF_DEFAULT);
  iupClassRegisterAttribute(ic, "YAUTOHIDE", NULL, gtkCanvasSetYAutoHideAttrib, "YES", NULL, IUPAF_DEFAULT);

  /* IupCanvas Windows or X only */
#ifdef WIN32                                 
  iupClassRegisterAttribute(ic, "HWND", iupgtkGetNativeWindowHandle, NULL, NULL, NULL, IUPAF_NO_INHERIT);
#else
  iupClassRegisterAttribute(ic, "XWINDOW", iupgtkGetNativeWindowHandle, NULL, NULL, NULL, IUPAF_NO_INHERIT|IUPAF_NO_STRING);
  iupClassRegisterAttribute(ic, "XDISPLAY", (IattribGetFunc)iupdrvGetDisplay, NULL, NULL, NULL, IUPAF_READONLY|IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT|IUPAF_NO_STRING);
#endif
}
