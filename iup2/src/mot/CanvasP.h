#ifndef CanvasP_h
#define CanvasP_h

#include <Xm/XmP.h>
#if (XmVersion > 1001)
#include <Xm/ManagerP.h>
#endif
#include <Xm/DrawingAP.h>
#include <X11/ConstrainP.h>
#include <X11/CompositeP.h>
#include <X11/CoreP.h>

#include "Canvas.h"

typedef struct {
  char	dummy;	/* some stupid compilers barf on empty structures */
} XmCanvasClassPart;

typedef struct _XmCanvasClassRec {
  CoreClassPart            core_class;
  CompositeClassPart       composite_class;
  ConstraintClassPart      constraint_class;
  XmManagerClassPart       xm_manager_class;
  XmDrawingAreaClassPart   xm_drawing_area_class;
  XmCanvasClassPart        xm_canvas_class;
} XmCanvasClassRec;

extern XmCanvasClassRec xmCanvasClassRec;

/* new fields */

typedef struct {
  Visual    *visual;
} XmCanvasPart;

typedef struct _XmCanvasRec {
  CorePart          core;
  CompositePart     composite;
  ConstraintPart    constraint;
  XmManagerPart     xm_manager;
  XmDrawingAreaPart xm_drawing_area;
  XmCanvasPart      xm_canvas;
} XmCanvasRec;

#endif
