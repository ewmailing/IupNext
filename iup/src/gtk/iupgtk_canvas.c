/** \file
 * \brief Canvas Control
 *
 * See Copyright Notice in "iup.h"
 */

#undef GSEAL_ENABLE
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


#define GTK_TYPE_IUPDRAWING_AREA            (iup_gtk_drawing_area_get_type ())
#define GTK_IUPDRAWING_AREA(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GTK_TYPE_IUPDRAWING_AREA, GtkIupDrawingArea))

enum {
   PROP_0,
   PROP_HADJUSTMENT,
   PROP_VADJUSTMENT,
   PROP_HSCROLL_POLICY,
   PROP_VSCROLL_POLICY
};

typedef struct _GtkIupDrawingArea
{
  GtkWidget widget;

#if GTK_CHECK_VERSION(3, 0, 0)
  GtkAdjustment *hadjustment;
  GtkAdjustment *vadjustment;

  guint hscroll_policy : 1;
  guint vscroll_policy : 1;
#endif
} GtkIupDrawingArea;

typedef struct _GtkIupDrawingAreaClass
{
  GtkWidgetClass parent_class;

#if !GTK_CHECK_VERSION(3, 0, 0)
  void  (*set_scroll_adjustments)   (GtkLayout	    *layout,
				     GtkAdjustment  *hadjustment,
				     GtkAdjustment  *vadjustment);
#endif
} GtkIupDrawingAreaClass;

#if GTK_CHECK_VERSION(3, 0, 0)
G_DEFINE_TYPE_WITH_CODE (GtkIupDrawingArea, iup_gtk_drawing_area, GTK_TYPE_WIDGET, G_IMPLEMENT_INTERFACE (GTK_TYPE_SCROLLABLE, NULL))
#else
G_DEFINE_TYPE (GtkIupDrawingArea, iup_gtk_drawing_area, GTK_TYPE_WIDGET)
#endif

static void iup_gtk_drawing_area_init (GtkIupDrawingArea *darea)
{
  (void)darea;
}

static void iup_gtk_drawing_area_realize (GtkWidget *widget)
{
  if (!gtk_widget_get_has_window (widget))
  {
    GTK_WIDGET_CLASS (iup_gtk_drawing_area_parent_class)->realize (widget);
  }
  else
  {
    GtkIupDrawingArea *darea = GTK_IUPDRAWING_AREA (widget);
    GtkAllocation allocation;
    GdkWindow *window;
    GdkWindowAttr attributes;
    gint attributes_mask;

    gtk_widget_set_realized (widget, TRUE);

#if GTK_CHECK_VERSION(2, 18, 0)
    gtk_widget_get_allocation (widget, &allocation);
#else
    allocation = widget->allocation;
#endif

    attributes.window_type = GDK_WINDOW_CHILD;
    attributes.x = allocation.x;
    attributes.y = allocation.y;
    attributes.width = allocation.width;
    attributes.height = allocation.height;
    attributes.wclass = GDK_INPUT_OUTPUT;
    attributes.visual = gtk_widget_get_visual (widget);
    attributes.event_mask = gtk_widget_get_events (widget) | GDK_EXPOSURE_MASK;
    attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL;
#if !GTK_CHECK_VERSION(3, 0, 0)
    attributes.colormap = gtk_widget_get_colormap (widget);
    attributes_mask |= GDK_WA_COLORMAP;
#endif

    window = gdk_window_new (gtk_widget_get_parent_window (widget), &attributes, attributes_mask);
    gdk_window_set_user_data (window, darea);
#if GTK_CHECK_VERSION(2, 18, 0)
    gtk_widget_set_window (widget, window);
#else
    widget->window = window;
#endif

#if GTK_CHECK_VERSION(3, 0, 0)
    gtk_style_context_set_background (gtk_widget_get_style_context (widget), window);
#else
    widget->style = gtk_style_attach (widget->style, widget->window);
    gtk_style_set_background (widget->style, widget->window, GTK_STATE_NORMAL);
#endif
  }
}

static void iup_gtk_drawing_area_size_allocate (GtkWidget *widget, GtkAllocation *allocation)
{
#if GTK_CHECK_VERSION(2, 18, 0)
  gtk_widget_set_allocation (widget, allocation);
#else
  widget->allocation = *allocation;
#endif

  if (gtk_widget_get_realized (widget))
  {
    if (gtk_widget_get_has_window (widget))
    {
#if GTK_CHECK_VERSION(2, 14, 0)
      GdkWindow* window = gtk_widget_get_window (widget);
#else
      GdkWindow* window = widget->window;
#endif
      gdk_window_move_resize (window, allocation->x, allocation->y, allocation->width, allocation->height);
    }
  }
}

#if GTK_CHECK_VERSION(3, 0, 0)
static void iup_gtk_drawing_area_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec  *pspec)
{
  GtkIupDrawingArea *darea = GTK_IUPDRAWING_AREA(object);

  switch (prop_id)
    {
    case PROP_HADJUSTMENT:
      g_value_set_object (value, darea->hadjustment);
      break;
    case PROP_VADJUSTMENT:
      g_value_set_object (value, darea->vadjustment);
      break;
    case PROP_HSCROLL_POLICY:
      g_value_set_enum (value, darea->hscroll_policy);
      break;
    case PROP_VSCROLL_POLICY:
      g_value_set_enum (value, darea->vscroll_policy);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void iup_gtk_drawing_area_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
  GtkIupDrawingArea *darea = GTK_IUPDRAWING_AREA(object);

  switch (prop_id)
    {
    case PROP_HADJUSTMENT:
      darea->hadjustment = (GtkAdjustment*)g_value_get_object(value);
      break;
    case PROP_VADJUSTMENT:
      darea->vadjustment = (GtkAdjustment*)g_value_get_object(value);
      break;
    case PROP_HSCROLL_POLICY:
      darea->hscroll_policy = g_value_get_enum (value);
      gtk_widget_queue_resize (GTK_WIDGET (object));
      break;
    case PROP_VSCROLL_POLICY:
      darea->vscroll_policy = g_value_get_enum (value);
      gtk_widget_queue_resize (GTK_WIDGET (object));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}
#else
static void iup_gtk_drawing_area_set_adjustments(GtkLayout *layout, GtkAdjustment *hadj, GtkAdjustment *vadj)
{
  (void)layout;
  (void)hadj;
  (void)vadj;
}

static void iup_gtk_marshal_VOID__OBJECT_OBJECT (GClosure *closure, GValue *return_value, guint n_param_values, const GValue *param_values, gpointer invocation_hint, gpointer marshal_data)
{
  typedef void (*GMarshalFunc_VOID__OBJECT_OBJECT) (gpointer data1, gpointer arg_1, gpointer arg_2, gpointer data2);
  register GMarshalFunc_VOID__OBJECT_OBJECT callback;
  register GCClosure *cc = (GCClosure*) closure;
  register gpointer data1, data2;
  (void)invocation_hint;
  (void)return_value;

  g_return_if_fail (n_param_values == 3);

  if (G_CCLOSURE_SWAP_DATA (closure))
  {
    data1 = closure->data;
    data2 = g_value_peek_pointer (param_values + 0);
  }
  else
  {
    data1 = g_value_peek_pointer (param_values + 0);
    data2 = closure->data;
  }
  callback = (GMarshalFunc_VOID__OBJECT_OBJECT) (marshal_data ? marshal_data : cc->callback);

  callback (data1,
            g_value_get_object (param_values + 1),
            g_value_get_object (param_values + 2),
            data2);
}
#endif

static void iup_gtk_drawing_area_class_init (GtkIupDrawingAreaClass *_class)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (_class);
  GObjectClass *gobject_class = (GObjectClass*)_class;

  widget_class->realize = iup_gtk_drawing_area_realize;
  widget_class->size_allocate = iup_gtk_drawing_area_size_allocate;

#if GTK_CHECK_VERSION(3, 0, 0)
  gobject_class->set_property = iup_gtk_drawing_area_set_property;
  gobject_class->get_property = iup_gtk_drawing_area_get_property;

  /* Scrollable interface */
  g_object_class_override_property (gobject_class, PROP_HADJUSTMENT,    "hadjustment");
  g_object_class_override_property (gobject_class, PROP_VADJUSTMENT,    "vadjustment");
  g_object_class_override_property (gobject_class, PROP_HSCROLL_POLICY, "hscroll-policy");
  g_object_class_override_property (gobject_class, PROP_VSCROLL_POLICY, "vscroll-policy");
#else
  _class->set_scroll_adjustments = iup_gtk_drawing_area_set_adjustments;

  widget_class->set_scroll_adjustments_signal =
    g_signal_new (g_intern_static_string("set-scroll-adjustments"),
	    G_OBJECT_CLASS_TYPE (gobject_class),
	    G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
	    G_STRUCT_OFFSET (GtkIupDrawingAreaClass, set_scroll_adjustments),
	    NULL, NULL,
	    iup_gtk_marshal_VOID__OBJECT_OBJECT,
	    G_TYPE_NONE, 2,
	    GTK_TYPE_ADJUSTMENT,
	    GTK_TYPE_ADJUSTMENT);
#endif
}

static GtkWidget* iup_gtk_drawing_area_new (void)
{
  return g_object_new (GTK_TYPE_IUPDRAWING_AREA, NULL);
}

/******************************************************************************/

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

static gboolean gtkCanvasScrollEvent(GtkWidget *widget, GdkEventScroll *evt, Ihandle *ih)
{    
  IFnfiis wcb = (IFnfiis)IupGetCallback(ih, "WHEEL_CB");
  if (wcb)
  {
    int delta = evt->direction==GDK_SCROLL_UP||evt->direction==GDK_SCROLL_LEFT? 1: -1;
    char status[IUPKEY_STATUS_SIZE] = IUPKEY_STATUS_INIT;
    int button = evt->direction==GDK_SCROLL_UP||evt->direction==GDK_SCROLL_LEFT? 4: 5;
    iupgtkButtonKeySetStatus(evt->state, button, status, 0);

    wcb(ih, (float)delta, (int)evt->x, (int)evt->y, status);
  }
  else
  {
    IFniff scb = (IFniff)IupGetCallback(ih,"SCROLL_CB");
    int delta = evt->direction==GDK_SCROLL_UP||evt->direction==GDK_SCROLL_LEFT? 1: -1;

    if (evt->direction==GDK_SCROLL_UP || evt->direction==GDK_SCROLL_DOWN)
    {
      float posy = ih->data->posy;
      posy -= delta*iupAttribGetFloat(ih, "DY")/10.0f;
      IupSetfAttribute(ih, "POSY", "%g", posy);
    }
    else
    {
      float posx = ih->data->posx;
      posx -= delta*iupAttribGetFloat(ih, "DX")/10.0f;
      IupSetfAttribute(ih, "POSX", "%g", posx);
    }

    if (scb)
    {
      int scroll_gtk2iup[4] = {IUP_SBUP, IUP_SBDN, IUP_SBLEFT, IUP_SBRIGHT};
      int op = scroll_gtk2iup[evt->direction];
      scb(ih,op,ih->data->posx,ih->data->posy);
    }
  }
  (void)widget;
  return TRUE;
}

static gboolean gtkCanvasButtonEvent(GtkWidget *widget, GdkEventButton *evt, Ihandle *ih)
{
  if (evt->type == GDK_BUTTON_PRESS)
  {
    /* Force focus on canvas click */
    if (iupAttribGetBoolean(ih, "CANFOCUS"))
      gtk_widget_grab_focus(ih->handle);
  }

  return iupgtkButtonEvent(widget, evt, ih);
}

static int gtkCanvasSetBgColorAttrib(Ihandle* ih, const char* value);

#if GTK_CHECK_VERSION(3, 0, 0)
static gboolean gtkCanvasDraw(GtkWidget *widget, cairo_t* cr, Ihandle *ih)
#else
static gboolean gtkCanvasExposeEvent(GtkWidget *widget, GdkEventExpose *evt, Ihandle *ih)
#endif
{
  IFnff cb = (IFnff)IupGetCallback(ih,"ACTION");
  if (cb)
  {
    if (!iupAttribGet(ih, "_IUPGTK_NO_BGCOLOR"))
      gtkCanvasSetBgColorAttrib(ih, iupAttribGetStr(ih, "BGCOLOR"));  /* reset to update window attributes */

#if GTK_CHECK_VERSION(3, 0, 0)
    {
      GdkRectangle rect;
      gdk_cairo_get_clip_rectangle(cr, &rect);
      iupAttribSetStrf(ih, "CLIPRECT", "%d %d %d %d", rect.x, rect.y, rect.x+rect.width-1, rect.y+rect.height-1);
    }
#else
    iupAttribSetStrf(ih, "CLIPRECT", "%d %d %d %d", evt->area.x, evt->area.y, evt->area.x+evt->area.width-1, evt->area.y+evt->area.height-1);
#endif
    cb(ih,ih->data->posx,ih->data->posy);
    iupAttribSetStr(ih, "CLIPRECT", NULL);
  }

  (void)widget;
  return TRUE;  /* stop other handlers */
}

static void gtkCanvasLayoutUpdateMethod(Ihandle *ih)
{
  GdkWindow* window = iupgtkGetWindow(ih->handle);

  iupdrvBaseLayoutUpdateMethod(ih);

  /* Force GdkWindow size update when not visible,
     so when mapped before show GDK returns the correct value. */
  if (!iupdrvIsVisible(ih))
    gdk_window_resize(window, ih->currentwidth, ih->currentheight);
}

static void gtkCanvasSizeAllocate(GtkWidget* widget, GdkRectangle *allocation, Ihandle *ih)
{
  IFnii cb = (IFnii)IupGetCallback(ih, "RESIZE_CB");
  if (cb)
    cb(ih, allocation->width, allocation->height);

  (void)widget;
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

    if (dx >= (xmax-xmin))
      ih->data->posx = 0;

#if GTK_CHECK_VERSION(2, 14, 0)
    {
      double page_size = dx;
      double dvalue = gtk_adjustment_get_value(sb_horiz);
      value_changed = gtkCanvasCheckScroll(xmin, xmax, &page_size, &dvalue);
      gtk_adjustment_configure(sb_horiz, dvalue, xmin, xmax, linex, page_size, dx);
    }
#else
    sb_horiz->lower = xmin;
    sb_horiz->upper = xmax;
    sb_horiz->step_increment = linex;
    sb_horiz->page_size = dx;

    value_changed = gtkCanvasCheckScroll(xmin, xmax, &sb_horiz->page_size, &sb_horiz->value);
    sb_horiz->page_increment = sb_horiz->page_size;

    gtk_adjustment_changed(sb_horiz);
#endif

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

    if (dy >= (ymax-ymin))
      ih->data->posy = 0;

#if GTK_CHECK_VERSION(2, 14, 0)
    {
      double page_size = dy;
      double dvalue = gtk_adjustment_get_value(sb_vert);
      value_changed = gtkCanvasCheckScroll(ymin, ymax, &page_size, &dvalue);
      gtk_adjustment_configure(sb_vert, dvalue, ymin, ymax, liney, page_size, dy);
    }
#else
    sb_vert->lower = ymin;
    sb_vert->upper = ymax;
    sb_vert->step_increment = liney;
    sb_vert->page_size = dy;

    value_changed = gtkCanvasCheckScroll(ymin, ymax, &sb_vert->page_size, &sb_vert->value);
    sb_vert->page_increment = sb_vert->page_size;

    gtk_adjustment_changed(sb_vert);
#endif

    if (value_changed)
      gtk_adjustment_value_changed(sb_vert);
  }
  return 1;
}

static int gtkCanvasSetPosXAttrib(Ihandle* ih, const char *value)
{
  if (ih->data->sb & IUP_SB_HORIZ)
  {
    float posx, xmin, xmax, dx;
    GtkAdjustment* sb_horiz = gtk_scrolled_window_get_hadjustment(gtkCanvasGetScrolledWindow(ih));
    if (!sb_horiz) return 1;

    if (!iupStrToFloat(value, &posx))
      return 1;

    xmin = iupAttribGetFloat(ih, "XMIN");
    xmax = iupAttribGetFloat(ih, "XMAX");
    dx = iupAttribGetFloat(ih, "DX");

    if (posx < xmin) posx = xmin;
    if (posx > (xmax - dx)) posx = xmax - dx;
    ih->data->posx = posx;

    gtk_adjustment_set_value(sb_horiz, posx);
  }
  return 1;
}

static int gtkCanvasSetPosYAttrib(Ihandle* ih, const char *value)
{
  if (ih->data->sb & IUP_SB_VERT)
  {
    float posy, ymin, ymax, dy;
    GtkAdjustment* sb_vert = gtk_scrolled_window_get_vadjustment(gtkCanvasGetScrolledWindow(ih));
    if (!sb_vert) return 1;

    if (!iupStrToFloat(value, &posy))
      return 1;

    ymin = iupAttribGetFloat(ih, "YMIN");
    ymax = iupAttribGetFloat(ih, "YMAX");
    dy = iupAttribGetFloat(ih, "DY");

    if (posy < ymin) posy = ymin;
    if (posy > (ymax - dy)) posy = ymax - dy;
    ih->data->posy = posy;

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
#if !GTK_CHECK_VERSION(3, 0, 0)
    gdk_window_set_back_pixmap(iupgtkGetWindow(ih->handle), NULL, FALSE);
#endif
    iupAttribSetStr(ih, "_IUPGTK_NO_BGCOLOR", "1");
    return 1;
  }
}

static char* gtkCanvasGetDrawSizeAttrib(Ihandle *ih)
{
  char* str = iupStrGetMemory(20);
  int w, h;
  GdkWindow* window = iupgtkGetWindow(ih->handle);

  if (window)
  {
#if GTK_CHECK_VERSION(2, 24, 0)
    w = gdk_window_get_width(window);
    h = gdk_window_get_height(window);
#else
    gdk_drawable_get_size(window, &w, &h);
#endif
  }
  else
    return NULL;

  sprintf(str, "%dx%d", w, h);
  return str;
}

static char* gtkCanvasGetDrawableAttrib(Ihandle* ih)
{
  return (char*)iupgtkGetWindow(ih->handle);
}

static int gtkCanvasMapMethod(Ihandle* ih)
{
  GtkScrolledWindow* scrolled_window;
#if !GTK_CHECK_VERSION(3, 0, 0)
  void* visual;
#endif

  if (!ih->parent)
    return IUP_ERROR;

  ih->data->sb = iupBaseGetScrollbar(ih);

#if !GTK_CHECK_VERSION(3, 0, 0)
  visual = (void*)IupGetAttribute(ih, "VISUAL");   /* defined by the OpenGL Canvas in X11 or NULL */
  if (visual)
    iupgtkPushVisualAndColormap(visual, (void*)iupAttribGet(ih, "COLORMAP"));
#endif

  ih->handle = iup_gtk_drawing_area_new();

#if !GTK_CHECK_VERSION(3, 0, 0)
  if (visual)
    gtk_widget_pop_colormap();
#endif

  if (!ih->handle)
      return IUP_ERROR;

#if GTK_CHECK_VERSION(2, 18, 0)
  /* CD will NOT work properly without this, must use always the CD-GDK driver */
  gtk_widget_set_has_window(ih->handle, TRUE);  
#endif

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

#if GTK_CHECK_VERSION(3, 0, 0)
  g_signal_connect(G_OBJECT(ih->handle), "draw",               G_CALLBACK(gtkCanvasDraw), ih);
#else
  g_signal_connect(G_OBJECT(ih->handle), "expose-event",       G_CALLBACK(gtkCanvasExposeEvent), ih);
#endif
  g_signal_connect(G_OBJECT(ih->handle), "button-press-event", G_CALLBACK(gtkCanvasButtonEvent), ih);
  g_signal_connect(G_OBJECT(ih->handle), "button-release-event",G_CALLBACK(gtkCanvasButtonEvent), ih);
  g_signal_connect(G_OBJECT(ih->handle), "motion-notify-event",G_CALLBACK(iupgtkMotionNotifyEvent), ih);
  g_signal_connect(G_OBJECT(ih->handle), "scroll-event",G_CALLBACK(gtkCanvasScrollEvent), ih);

  g_signal_connect(G_OBJECT(ih->handle), "size-allocate", G_CALLBACK(gtkCanvasSizeAllocate), ih);

#if GTK_CHECK_VERSION(2, 8, 0)
  g_signal_connect(G_OBJECT(gtk_scrolled_window_get_hscrollbar(scrolled_window)), "change-value",G_CALLBACK(gtkCanvasHChangeValue), ih);
  g_signal_connect(G_OBJECT(gtk_scrolled_window_get_vscrollbar(scrolled_window)), "change-value",G_CALLBACK(gtkCanvasVChangeValue), ih);
#endif

  /* To receive mouse events on a drawing area, you will need to enable them. */
  gtk_widget_add_events(ih->handle, GDK_EXPOSURE_MASK|
    GDK_POINTER_MOTION_MASK|GDK_POINTER_MOTION_HINT_MASK|
    GDK_BUTTON_PRESS_MASK|GDK_BUTTON_RELEASE_MASK|GDK_BUTTON_MOTION_MASK|
    GDK_KEY_PRESS_MASK|GDK_KEY_RELEASE_MASK|
    GDK_ENTER_NOTIFY_MASK|GDK_LEAVE_NOTIFY_MASK|
    GDK_FOCUS_CHANGE_MASK|GDK_STRUCTURE_MASK);

  /* To receive keyboard events, you will need to set the GTK_CAN_FOCUS flag on the drawing area. */
  if (ih->iclass->is_interactive)
  {
    if (iupAttribGetBoolean(ih, "CANFOCUS"))
      iupgtkSetCanFocus(ih->handle, 1);
  }

  if (iupAttribGetBoolean(ih, "BORDER"))
    gtk_scrolled_window_set_shadow_type(scrolled_window, GTK_SHADOW_IN); 
  else
    gtk_scrolled_window_set_shadow_type(scrolled_window, GTK_SHADOW_NONE);

  gtk_widget_realize((GtkWidget*)scrolled_window);
  gtk_widget_realize(ih->handle);

  /* configure for DRAG&DROP */
  if (IupGetCallback(ih, "DROPFILES_CB"))
    iupAttribSetStr(ih, "DROPFILESTARGET", "YES");

  /* update a mnemonic in a label if necessary */
  iupgtkUpdateMnemonic(ih);

  /* configure scrollbar */
  if (ih->data->sb)
  {
    GtkPolicyType hscrollbar_policy = GTK_POLICY_NEVER, 
                  vscrollbar_policy = GTK_POLICY_NEVER;
    if (ih->data->sb & IUP_SB_HORIZ)
      hscrollbar_policy = GTK_POLICY_AUTOMATIC;
    if (ih->data->sb & IUP_SB_VERT)
      vscrollbar_policy = GTK_POLICY_AUTOMATIC;
    gtk_scrolled_window_set_policy(scrolled_window, hscrollbar_policy, vscrollbar_policy);
  }
  else
    gtk_scrolled_window_set_policy(scrolled_window, GTK_POLICY_NEVER, GTK_POLICY_NEVER);

  /* force the update of BGCOLOR here, to let derived classes ignore it if ACTION is defined */
  gtkCanvasSetBgColorAttrib(ih, iupAttribGetStr(ih, "BGCOLOR"));
    
  return IUP_NOERROR;
}

void iupdrvCanvasInitClass(Iclass* ic)
{
  /* Driver Dependent Class functions */
  ic->Map = gtkCanvasMapMethod;
  ic->LayoutUpdate = gtkCanvasLayoutUpdateMethod;

  /* Driver Dependent Attribute functions */

  /* Visual */
  iupClassRegisterAttribute(ic, "BGCOLOR", NULL, gtkCanvasSetBgColorAttrib, "255 255 255", NULL, IUPAF_DEFAULT);  /* force new default value */

  /* IupCanvas only */
  iupClassRegisterAttribute(ic, "CURSOR", NULL, iupdrvBaseSetCursorAttrib, IUPAF_SAMEASSYSTEM, "ARROW", IUPAF_IHANDLENAME|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "DRAWSIZE", gtkCanvasGetDrawSizeAttrib, NULL, NULL, NULL, IUPAF_READONLY|IUPAF_NO_INHERIT);

  iupClassRegisterAttribute(ic, "DX", NULL, gtkCanvasSetDXAttrib, "0.1", NULL, IUPAF_NO_INHERIT);  /* force new default value */
  iupClassRegisterAttribute(ic, "DY", NULL, gtkCanvasSetDYAttrib, "0.1", NULL, IUPAF_NO_INHERIT);  /* force new default value */
  iupClassRegisterAttribute(ic, "POSX", iupCanvasGetPosXAttrib, gtkCanvasSetPosXAttrib, "0", NULL, IUPAF_NO_INHERIT);  /* force new default value */
  iupClassRegisterAttribute(ic, "POSY", iupCanvasGetPosYAttrib, gtkCanvasSetPosYAttrib, "0", NULL, IUPAF_NO_INHERIT);  /* force new default value */
  iupClassRegisterAttribute(ic, "XAUTOHIDE", NULL, gtkCanvasSetXAutoHideAttrib, "YES", NULL, IUPAF_NO_INHERIT);  /* force new default value */
  iupClassRegisterAttribute(ic, "YAUTOHIDE", NULL, gtkCanvasSetYAutoHideAttrib, "YES", NULL, IUPAF_NO_INHERIT);  /* force new default value */

  iupClassRegisterAttribute(ic, "DRAWABLE", gtkCanvasGetDrawableAttrib, NULL, NULL, NULL, IUPAF_NO_STRING);

  /* IupCanvas Windows or X only */
#ifndef GTK_MAC
  #ifdef WIN32                                 
    iupClassRegisterAttribute(ic, "HWND", iupgtkGetNativeWindowHandle, NULL, NULL, NULL, IUPAF_NO_STRING|IUPAF_NO_INHERIT);
  #else
    iupClassRegisterAttribute(ic, "XWINDOW", iupgtkGetNativeWindowHandle, NULL, NULL, NULL, IUPAF_NO_INHERIT|IUPAF_NO_STRING);
    iupClassRegisterAttribute(ic, "XDISPLAY", (IattribGetFunc)iupdrvGetDisplay, NULL, NULL, NULL, IUPAF_READONLY|IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT|IUPAF_NO_STRING);
  #endif
#endif

  /* Not Supported */
  iupClassRegisterAttribute(ic, "BACKINGSTORE", NULL, NULL, "YES", NULL, IUPAF_NOT_SUPPORTED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "TOUCH", NULL, NULL, NULL, NULL, IUPAF_NOT_SUPPORTED|IUPAF_NO_INHERIT);
}
