#ifndef Canvas_h
#define Canvas_h

#ifndef XmNvisual
#define XmNvisual "visual"
#define XmCvisual "Visual"
#endif

extern WidgetClass xmCanvasWidgetClass;

typedef struct _XmCanvasClassRec *XmCanvasWidgetClass;
typedef struct _XmCanvasRec      *XmCanvasWidget;

#endif
