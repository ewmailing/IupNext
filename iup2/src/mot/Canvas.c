#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include "CanvasP.h"

#define offset(field) XtOffset(XmCanvasWidget, xm_canvas.field)

static void CanvasRealize (Widget widget, XtValueMask* mask,
                           XSetWindowAttributes* attrs);

static XtResource resources[] = {
  {
    XtNvisual,                                      /* name */
    XtCVisual,                                      /* class */
    XtRVisual,                                      /* type */
    sizeof(((XmCanvasRec*)NULL)->xm_canvas.visual), /* size */
    offset(visual),                                 /* offset */
    XtRVisual,                                      /* default type */
    (XtPointer)NULL                                 /* default addr */
  }
};

XmCanvasClassRec xmCanvasClassRec = {
  { /* core_class fields ------- */
    /* superclass	  	 */ (WidgetClass) &xmDrawingAreaClassRec,
    /* class_name	  	 */ "XmCanvas",
    /* widget_size	  	 */ sizeof(XmCanvasRec),
    /* class_initialize   	 */ NULL,
    /* class_part_initialize	 */ NULL,
    /* class_inited       	 */ False,
    /* initialize	  	 */ NULL,
    /* initialize_hook		 */ NULL,
    /* realize		  	 */ CanvasRealize,
    /* actions		  	 */ NULL,
    /* num_actions	  	 */ 0,
    /* resources	  	 */ resources,
    /* num_resources	  	 */ XtNumber(resources),
    /* xrm_class	  	 */ NULLQUARK,
    /* compress_motion	  	 */ True,
    /* compress_exposure  	 */ XtExposeCompressMultiple,
    /* compress_enterleave	 */ True,
    /* visible_interest	  	 */ True,
    /* destroy		  	 */ NULL,
    /* resize		  	 */ XtInheritResize,
    /* expose		  	 */ XtInheritExpose,
    /* set_values	  	 */ NULL,
    /* set_values_hook		 */ NULL,
    /* set_values_almost	 */ XtInheritSetValuesAlmost,
    /* get_values_hook		 */ NULL,
    /* accept_focus	 	 */ XtInheritAcceptFocus,
    /* version			 */ XtVersion,
    /* callback_private   	 */ NULL,
    /* tm_table		   	 */ XtInheritTranslations,
    /* query_geometry		 */ XtInheritQueryGeometry,
    /* display_accelerator       */ XtInheritDisplayAccelerator,
    /* extension                 */ NULL
  },
  { /* composite_class fields -- */
    /* geometry_manager		 */ XtInheritGeometryManager,
    /* change_managed		 */ XtInheritChangeManaged,
    /* insert_child		 */ XtInheritInsertChild,
    /* delete_child		 */ XtInheritDeleteChild,
    /* extension		 */ NULL
  },
  /* --- Constraint part copied from ComboBox --- */
  { /* constraint_class fields	 */
    /* resources		 */ NULL,
    /* num_resources		 */ 0,
    /* constraint_size		 */ sizeof(XmManagerConstraintPart),
    /* initialize		 */ NULL,
    /* destroy			 */ NULL,
    /* set_values		 */ NULL,
    /* extension		 */ NULL
  },
  /* --- XmManager part copied from ComboBox --- */
  { /* xm_manager_class fields	 */
    /* translations		 */ XtInheritTranslations,
    /* syn_resources		 */ NULL,
    /* num_syn_resources	 */ 0,
    /* syn_constraint_resources	 */ NULL,
    /* num_syn_constraint_resources */ 0,
    /* parent_process		 */ XmInheritParentProcess,
    /* extension		 */ NULL
  },
  { /* xm_drawing_area_class fields */
    /* extension		 */ NULL
  },
  { /* canvas_class fields ----- */
    /* dummy			 */ 0
  }
};

WidgetClass xmCanvasWidgetClass = (WidgetClass) & xmCanvasClassRec;

static void CanvasRealize (Widget widget, XtValueMask* mask,
                           XSetWindowAttributes* attrs)
{
  XmCanvasWidget cw = (XmCanvasWidget)widget;
  XtCreateWindow(widget, (unsigned int)InputOutput,
                 (Visual *)cw->xm_canvas.visual, *mask, attrs);
}
