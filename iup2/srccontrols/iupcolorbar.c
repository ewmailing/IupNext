/** \file
* \brief iupcolorbar control
* @author André Luiz Clinio.
*
* See Copyright Notice in iup.h
* $Id: iupcolorbar.c,v 1.1 2008-10-17 06:19:56 scuri Exp $
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <iup.h>
#include <cd.h>
#include <cdiup.h>
#include <cddbuf.h>
#include <iupcpi.h>
#include <iupcompat.h>
#include "iupcdutil.h"
#include "icontrols.h"

#include "iupcolorbar.h"

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

/** Default number of cells. */
#define DEFAULT_NUM_CELLS 16

/** No color */
#define NO_COLOR 0xff000000

/** preview margin */
#define DELTA 5

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */


/** Colorbar callback type 1 */
typedef int (*TColorCb1)(Ihandle*, int);

/** Colorbar callback type 1s */
typedef char* (*TColorCb1s)(Ihandle*, int);

/** Colorbar callback type 2 */
typedef int (*TColorCb2)(Ihandle*, int, int);

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

/** 
 * This structure is used to store an object (widget) attributes and 
 * data. 
 * @author André Luiz Clinio
 */
typedef struct {
   /** Pointer to IUP handle element */
   Ihandle* self;

   /** size of the canvas */
   int w, h;

   /** CD canvas used by the widget for drawing */
   cdCanvas *cdcanvas, 
            *cddbuffer;

   /** CD color vector */
   long int colors[256];

   /** Number of cells at the widgets */
   int num_cells;

   /** Number of sections used to split the colors cells area */
   int num_parts;

   /** Vertical orientation flag */
   int vertical;

   /** Square cells attribute flag */
   int squared;

   /** 3D shadowed attribute flag */
   int shadowed;

   /** 3D shadowed colors */
   long light_shadow,
        mid_shadow,
        dark_shadow;

   /** Bufferation flag */
   int bufferize;

   /** Transparency color*/
   long int transparency;

   /** Secondary color selction flag */
   int show_secondary;

   /** Preview size (pixels) */
   int preview_size;

   /** Current primary index selected */
   int fgcolor_idx;

   /** Current secondary index selected */
   int bgcolor_idx;
}Tcolorbar;

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

static void fit_square(int* xmin, int* xmax, int* ymin, int* ymax);

static int get_index_color(Tcolorbar* obj, int x, int y);
static int check_preview(Tcolorbar* obj, int x, int y);

static void get_preview_limit(Tcolorbar* obj, 
       int* xmin, int* xmax, int* ymin, int* ymax);
static void get_cell_limit(Tcolorbar* obj, int idx,
       int* xmin, int* xmax, int* ymin, int* ymax);


static void draw_box(Tcolorbar* obj, int xmin, int xmax, 
       int ymin, int ymax, int idx);
static void draw_sunken(Tcolorbar* obj, int xmin, int xmax, 
       int ymin, int ymax);

static void repaint_all(Tcolorbar* obj);
static void repaint_cell(Tcolorbar* obj, int idx);
static void repaint_preview(Tcolorbar* obj);
static void repaint_cells(Tcolorbar* obj);

static Ihandle* colorbar_create(Iclass *ic, void **params);
static void colorbar_setattr(Ihandle *h, char *attr, char *value);
static void colorbar_destroy(Ihandle *h);
static char* colorbar_getattr(Ihandle *h, char *attr);

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

/** Default colors used for a widget.  @see DEFAULT_NUM_CELLS */
  static struct { unsigned int r, g, b; } default_colors[DEFAULT_NUM_CELLS] = {
    { 0,0,0 }, { 128,0,0 }, { 0,128,0 }, { 128,128,0 },    
    { 0,0,128 }, { 128,0,128 }, { 0,128,128 }, { 192,192,192 },    
    { 128,128,128 }, { 255,0,0 }, { 0,255,0 }, { 255,255,0 },
    { 0,0,255 }, { 255,0,255 }, { 0,255,255 }, { 255,255,255 } };

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

static void colorbar_createcanvas(Tcolorbar* obj, Ihandle *self)
{
  if (!obj->cdcanvas)
    obj->cdcanvas = cdCreateCanvas(CD_IUP, self);

  if (obj->cdcanvas)
  {
    cdCanvasActivate(obj->cdcanvas);
    obj->cddbuffer = cdCreateCanvas(CD_DBUFFER, obj->cdcanvas);
  }
}

/* ...................................................................... */
/** 
 * This is the IUP repaint callback for the widget's canvas.
 * @param h the IUP handle 
 */
static int colorbar_repaint(Ihandle *h) {
  Tcolorbar* obj = (Tcolorbar*) iupGetEnv(h, "_IUPCOLORBAR_DATA");
  if (!obj) return IUP_DEFAULT;

  if (!obj->cddbuffer)
    colorbar_createcanvas(obj, h);

  if (!obj->cddbuffer)
    return IUP_DEFAULT;

  /* Call to the proper function using the widget data pointer */
  repaint_all(obj);

  return IUP_DEFAULT;
}

/* ...................................................................... */
/** 
 * This is the IUP resize callback for the widget's canvas.
 * @param h the IUP handle 
 */
static int colorbar_resize(Ihandle *h) {
  Tcolorbar* obj = (Tcolorbar*) iupGetEnv(h, "_IUPCOLORBAR_DATA");
  if (!obj) return IUP_DEFAULT;

  if (!obj->cddbuffer)
    colorbar_createcanvas(obj, h);

  if (!obj->cddbuffer)
    return IUP_DEFAULT;

  cdCanvasActivate(obj->cddbuffer);
  cdCanvasGetSize(obj->cddbuffer,&obj->w, &obj->h, NULL, NULL);

  return IUP_DEFAULT;
}

/* ...................................................................... */
/** 
 * This is the IUP/CPI destroy callback for the class.
 * @param h the IUP handle.  
 */
static void colorbar_destroy(Ihandle *h) {
  Tcolorbar* obj = (Tcolorbar*) iupGetEnv(h, "_IUPCOLORBAR_DATA");
  if (obj == NULL) return;

  /* Destruction of the CD canvas */
  if (obj->cddbuffer) cdKillCanvas(obj->cddbuffer);
  if (obj->cdcanvas) cdKillCanvas(obj->cdcanvas);

  iupSetEnv(h, "_IUPCOLORBAR_DATA", NULL);

  free(obj);
}

/* ...................................................................... */
/** 
 * This is the IUP/CPI attribute writting callback for the class
 * @param h the IUP handle.
 * @param attr the attribute name.
 * @param value the value to be set.
 * @see colorbar_resize 
 */

static void colorbar_setattr(Ihandle *h, char *attr, char *value) {
  Tcolorbar* obj = (Tcolorbar*) iupGetEnv(h, "_IUPCOLORBAR_DATA");
  if (obj == NULL  || value == NULL) { 
     iupCpiDefaultSetAttr(h, attr, value);
     return;
  }
  if (iupStrEqual(attr, IUP_NUM_PARTS)) { 
     int new_val = atoi(value);
     if (new_val > 0) { 
        obj->num_parts = new_val;
        repaint_all(obj);
     }
  }
  else if (iupStrEqual(attr, IUP_PRIMARY_CELL)) { 
    int new_val = atoi(value);
    if (new_val > 0 && new_val < obj->num_cells) { 
       obj->fgcolor_idx = new_val;
       repaint_all(obj);
    }
  }
  else if (iupStrEqual(attr, IUP_SECONDARY_CELL)) { 
    int new_val = atoi(value);
    if (new_val > 0 && new_val < obj->num_cells) { 
       obj->bgcolor_idx = new_val;
       repaint_all(obj);
    }
  }
  else if (iupStrEqual(attr, IUP_BUFFERIZE)) { 
    if (iupStrEqual(value, IUP_NO)) { 
       obj->bufferize = 0;
       repaint_all(obj);
    }
    else {
       obj->bufferize = 1;
    }
  }
  else if (iupStrEqual(attr, IUP_NUM_CELLS)) { 
    int new_val = atoi(value);
    if (new_val > 0 && new_val <= 256) { 
       obj->num_cells = new_val;
      if (obj->fgcolor_idx >= obj->num_cells) {
         obj->fgcolor_idx = obj->num_cells-1;
      }
      if (obj->bgcolor_idx >= obj->num_cells) {
         obj->bgcolor_idx = obj->num_cells-1;
      }
      repaint_all(obj);
    }
  }
  else if (iupStrEqual(attr, IUP_ORIENTATION)) { 
     if (iupStrEqual(value, IUP_HORIZONTAL)) obj->vertical = 0;
     else obj->vertical = 1;
     repaint_all(obj);
  }
  else if (iupStrEqual(attr, IUP_SQUARED)) { 
     if (iupStrEqual(value, IUP_NO)) obj->squared = 0;
     else obj->squared = 1;
     repaint_all(obj);
  }
  else if (iupStrEqual(attr, IUP_SHADOWED)) { 
     if (iupStrEqual(value, IUP_NO)) obj->shadowed = 0;
     else obj->shadowed = 1;
     repaint_all(obj);
  }
  else if (iupStrEqual(attr, IUP_SHOW_SECONDARY)) { 
     if (iupStrEqual(value, IUP_NO)) obj->show_secondary = 0;
     else obj->show_secondary = 1;
     repaint_all(obj);
  }
  else if (iupStrEqual(attr, IUP_SHOW_PREVIEW)) { 
     if (iupStrEqual(value, IUP_NO)) obj->preview_size = 0;
     else obj->preview_size = -9;
     repaint_all(obj);
  }
  else if (iupStrEqual(attr, IUP_PREVIEW_SIZE)) { 
    int new_val = atoi(value);
    if (new_val > 0) { 
       obj->preview_size = new_val;
       repaint_all(obj);
    }
  }
  else if (iupStrEqualPartial(attr, IUP_CELL)) { 
    int idx = -1;
    sscanf(attr+strlen(IUP_CELL), "%d", &idx);
    if (idx >= 0 || idx < obj->num_cells) { 
       obj->colors[ idx ] = cdIupConvertColor(value);
       repaint_all(obj);
    }
  }
  else if (iupStrEqual(attr, IUP_TRANSPARENCY)) { 
    if (value==NULL) obj->transparency = NO_COLOR;
    else obj->transparency = cdIupConvertColor(value);
    repaint_all(obj);
  }
  else {
    iupCpiDefaultSetAttr(h, attr, value);
  }
}

#define REPNTCL_ALL  -1
#define REPNTCL_NONE -2

static void call_repaint(Tcolorbar* obj, int preview, int idx)
{
  if (preview) repaint_preview(obj);
  if (idx != REPNTCL_NONE)
  {
    if (idx == REPNTCL_ALL) repaint_cells(obj);
    else repaint_cell(obj, idx);
  }
  cdCanvasFlush(obj->cddbuffer);
}

/* ...................................................................... */
/** 
 * This is the IUP button callback for the widget´s canvas
 * @param h the IUP handle.
 * @param b the number of the button.
 * @param m the button state.
 * @param x the x coordinate.
 * @param y the value to be set.
 * @param r the key mask.  
 */
static int colorbar_button(Ihandle *h, int b, int m, int x, int y, char* r) {
  int idx, ret;
  Tcolorbar* obj = (Tcolorbar*) iupGetEnv(h, "_IUPCOLORBAR_DATA");
  if (obj == NULL) return IUP_IGNORE;
  if (m == 0) return IUP_IGNORE;

  y = cdIupInvertYAxis(y, obj->h);

  if (b == IUP_BUTTON1 && isdouble(r)) 
  { 
    TColorCb1s cell_callback = (TColorCb1s)IupGetCallback(h, IUP_CELL_CB);
    char * returned = NULL;

    idx = get_index_color(obj, x, y); 
    if (idx < 0  || idx >= obj->num_cells) 
    {
      ret = check_preview(obj, x, y);
      if (ret)
      {
        if (ret == 1)
        {
          TColorCb2 switch_callback = (TColorCb2)IupGetCallback(h, IUP_SWITCH_CB);
          if (!obj->show_secondary) return IUP_IGNORE;
          if (switch_callback && switch_callback(h, obj->fgcolor_idx, obj->bgcolor_idx) != IUP_DEFAULT) 
            return IUP_IGNORE;

          /* the application allow to switch the indices */
          idx = obj->fgcolor_idx;
          obj->fgcolor_idx = obj->bgcolor_idx;
          obj->bgcolor_idx = idx;

          call_repaint(obj, 1, REPNTCL_NONE); /* only the preview area is repainted */
        }
        else
        {
          if (ret == IUP_PRIMARY)
            idx = obj->fgcolor_idx;
          else
            idx = obj->bgcolor_idx;

          if (cell_callback) 
            returned = cell_callback(h, idx);  /* the application can change the color */

          if (returned)
          {                        
            obj->colors[ idx ] = cdIupConvertColor(returned);
            call_repaint(obj, 1, idx);        /* the preview and the cell are repainted */
          }
        }
      }

      return IUP_DEFAULT;
    }

    if (cell_callback)
      returned = cell_callback(h, idx);  /* the application can change the color */

    if (returned) 
    {
      int preview = 0;                        /* check if the preview area should be repainted */
      if (idx == obj->fgcolor_idx || idx == obj->bgcolor_idx)
        preview = 1;

      obj->colors[ idx ] = cdIupConvertColor(returned);
      call_repaint(obj, preview, idx);        /* the preview and the cell are repainted */
    }
  }
  else if (b == IUP_BUTTON1) 
  { 
    TColorCb2 select_callback = (TColorCb2)IupGetCallback(h, IUP_SELECT_CB);

    idx = get_index_color(obj, x, y); 
    if (idx < 0  || idx >= obj->num_cells) return IUP_IGNORE;

    if (select_callback && select_callback(h, idx, IUP_PRIMARY) != IUP_DEFAULT) 
      return IUP_IGNORE;

    obj->fgcolor_idx = idx;
    
    call_repaint(obj, 1, REPNTCL_NONE); /* only the preview area is repainted */
  }
  else if (b == IUP_BUTTON3 && isshift(r)) 
  { 
    TColorCb1 extended_callback = (TColorCb1)IupGetCallback(h, IUP_EXTENDED_CB);
    if (extended_callback == NULL) return IUP_IGNORE;

    idx = get_index_color(obj, x, y); 
    if (idx < 0  || idx >= obj->num_cells) return IUP_IGNORE;

    if (extended_callback(h, idx) != IUP_DEFAULT) return IUP_DEFAULT;

    call_repaint(obj, 1, REPNTCL_ALL);  /* the preview and all the cells are repainted */
  }
  else if (b == IUP_BUTTON3) 
  { 
    TColorCb2 select_callback = (TColorCb2)IupGetCallback(h, IUP_SELECT_CB);
    if (!obj->show_secondary) return IUP_IGNORE;

    idx = get_index_color(obj, x, y); 
    if (idx < 0  || idx >= obj->num_cells) return IUP_IGNORE;

    if (select_callback && select_callback(h, idx, IUP_SECONDARY) != IUP_DEFAULT)
      return IUP_IGNORE;

    obj->bgcolor_idx = idx;

    call_repaint(obj, 1, REPNTCL_NONE);  /* only the preview area is repainted */
  }

  return IUP_DEFAULT;
}

/* ...................................................................... */
/** 
 * IUP/CPI get attribute method for the colorbar.
 * @param h the IUP handle
 * @param attr the attribute name
 * @return the requested value
 * @see "_IUPCOLORBAR_DATA" 
 */
static char* colorbar_getattr(Ihandle *h, char *attr) {
  Tcolorbar* obj = NULL;
  static char buffer[100];

  obj = (Tcolorbar*) iupGetEnv(h, "_IUPCOLORBAR_DATA");
  if (obj == NULL) return iupCpiDefaultGetAttr(h, attr);

  if (iupStrEqual(attr, IUP_NUM_PARTS)) { 
     sprintf(buffer, "%d", obj->num_parts);
     return buffer;
  }
  else if (iupStrEqual(attr, IUP_NUM_CELLS)) {
     sprintf(buffer, "%d", obj->num_cells);
     return buffer;
  }
  else if (iupStrEqual(attr, IUP_PRIMARY_CELL)) {
     sprintf(buffer, "%d", obj->fgcolor_idx);
     return buffer;
  }
  else if (iupStrEqual(attr, IUP_SECONDARY_CELL)) {
     sprintf(buffer, "%d", obj->bgcolor_idx);
     return buffer;
  }
  else if (iupStrEqual(attr, IUP_SQUARED)) {
     if (obj->squared == 0) return IUP_NO;
     else return IUP_YES;
  }
  else if (iupStrEqual(attr, IUP_SHADOWED)) {
     if (obj->shadowed == 0) return IUP_NO;
     else return IUP_YES;
  }
  else if (iupStrEqual(attr, IUP_SHOW_SECONDARY)) {
     if (obj->show_secondary == 0) return IUP_NO;
     else return IUP_YES;
  }
  else if (iupStrEqual(attr, IUP_ORIENTATION)) {
     if (obj->vertical == 0) return IUP_HORIZONTAL;
     else return IUP_VERTICAL;
  }
  else if (iupStrEqual(attr, IUP_SHOW_PREVIEW)) {
     if (obj->preview_size == 0) return IUP_NO;
     else return IUP_YES;
  }
  else if (iupStrEqual(attr, IUP_PREVIEW_SIZE)) {
     sprintf(buffer, "%d", obj->preview_size);
     return buffer;
  }
  else if (iupStrEqualPartial(attr, IUP_CELL)) {
     int idx = -1;
     long col;
     sscanf(attr+strlen(IUP_CELL), "%d", &idx);
     if (idx < 0 || idx >= obj->num_cells) return NULL;
     col = obj->colors[ idx ];
     sprintf(buffer, "%d %d %d", cdRed(col), cdGreen(col), cdBlue(col));
     return buffer;
  }
  else if (iupStrEqual(attr, IUP_TRANSPARENCY)) {
     if (obj->transparency == NO_COLOR) return NULL;
     else {
       sprintf(buffer, "%d %d %d", cdRed(obj->transparency), cdGreen(obj->transparency), cdBlue(obj->transparency));
       return buffer;
     }
  }
  else {
     return iupCpiDefaultGetAttr(h, attr);
  }
}

/* ...................................................................... */
/** 
 * IUP/CPI creation method for the colorbar.
 * @param ic the class
 * @param params the arguments for the creation
 * @return the IUP handle (canvas) 
 * @see "_IUPCOLORBAR_DATA"
 * @see colorbar_button
 * @see colorbar_resize
 * @see colorbar_repaint 
 */
static Ihandle *colorbar_create(Iclass *ic, void **params) {
  int i;
  /* Creating the widget struct */
  Tcolorbar* obj = (Tcolorbar*) malloc(sizeof(Tcolorbar));

  /* Creating the IUP canvas for the widget */
  Ihandle*  self = IupCanvas(NULL);
  iupSetEnv(self, IUP_BORDER, IUP_NO); 

  /* Storing the widget data pointer inside the handle */
  iupSetEnv(self, "_IUPCOLORBAR_DATA", (char*)obj); 

  /* Setting the IUP canvas callbacks for the widget's CD canvas */
  IupSetCallback(self, IUP_RESIZE_CB, (Icallback) colorbar_resize);
  IupSetCallback(self, IUP_ACTION, (Icallback) colorbar_repaint);
  IupSetCallback(self, IUP_BUTTON_CB, (Icallback) colorbar_button);

  /* Internal struct initialization */
  obj->num_cells = 16;
  obj->num_parts = 1;
  obj->vertical = 1;
  obj->squared = 1;
  obj->shadowed = 1;
  obj->bufferize = 0;
  obj->show_secondary = 0;
  obj->preview_size = -9;
  obj->fgcolor_idx = 0;  /* black */
  obj->bgcolor_idx = 15; /* white */
  obj->cdcanvas = NULL;
  obj->transparency = NO_COLOR;
  obj->light_shadow = CD_WHITE;
  obj->mid_shadow = CD_GRAY;
  obj->dark_shadow = CD_DARK_GRAY;

  /* Internal struct IUP and CD initialization */
  obj->self = self;

  /* Initialization of the color vector */
  for (i = 0; i < DEFAULT_NUM_CELLS; i++) {
    obj->colors[i] = cdEncodeColor((unsigned char)default_colors[i].r, (unsigned char)default_colors[i].g, (unsigned char)default_colors[i].b);
  }

  return self;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

/* ...................................................................... */
/**
 * Element creation.
 * @returns a IUP handle.
 */
Ihandle *IupColorbar(void) {
  return IupCreate("colorbar");
}

/* ...................................................................... */
/** 
 * Library initialization function 
 */
void IupColorbarOpen(void) {
  /* New class creation */
  Iclass* ICColorbar = iupCpiCreateNewClass("colorbar", NULL);

  /* Adjusting CPI methods */
  iupCpiSetClassMethod(ICColorbar, ICPI_CREATE, (Imethod) colorbar_create);
  iupCpiSetClassMethod(ICColorbar, ICPI_DESTROY, (Imethod) colorbar_destroy);
  iupCpiSetClassMethod(ICColorbar, ICPI_SETATTR, (Imethod) colorbar_setattr);
  iupCpiSetClassMethod(ICColorbar, ICPI_GETATTR, (Imethod) colorbar_getattr);
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

/* ...................................................................... */
/** 
 * This function draw the 3D cell effect.
 * @param obj the widget pointer.
 * @param xmin the x minimum value.
 * @param xmax the x maximum value.
 * @param ymin the y minimum value.
 * @param ymax the y maximum value.  
 */
static void draw_sunken(Tcolorbar* obj, int xmin, int xmax, int ymin, 
int ymax) {
  if (!obj->shadowed) return;
  cdIupDrawSunkenRect(obj->cddbuffer, xmin, ymin, xmax, ymax, obj->light_shadow, obj->mid_shadow, obj->dark_shadow);
}

/* ...................................................................... */
/** 
 * This function is used to draw a box for a cell.
 * @param obj the widget data pointer.
 * @param xmin the x minimum value.
 * @param xmax the x minimum value.
 * @param ymin the y minimum value.
 * @param ymax the y maximum value.
 * @param idx the cell index.  
 */
static void draw_box(Tcolorbar* obj, int xmin, int xmax, int ymin, 
int ymax, int idx) {
  long int col = obj->colors[ idx ];
  cdCanvasInteriorStyle(obj->cddbuffer,CD_SOLID);

  if (col == obj->transparency) { 
     int xm = (xmin+xmax)/2;
     int ym = (ymin+ymax)/2;
     cdCanvasForeground(obj->cddbuffer,0xeeeeee);
     cdCanvasBox(obj->cddbuffer,xmin, xm, ymin, ym);
     cdCanvasBox(obj->cddbuffer,xm, xmax, ym, ymax);
     cdCanvasForeground(obj->cddbuffer,0xcccccc);
     cdCanvasBox(obj->cddbuffer,xmin, xm, ym, ymax);
     cdCanvasBox(obj->cddbuffer,xm, xmax, ymin, ym);
  }
  else {
    cdCanvasForeground(obj->cddbuffer,col);
    cdCanvasBegin(obj->cddbuffer,CD_FILL);
    cdCanvasVertex(obj->cddbuffer,xmin, ymin); cdCanvasVertex(obj->cddbuffer,xmin, ymax);
    cdCanvasVertex(obj->cddbuffer,xmax, ymax); cdCanvasVertex(obj->cddbuffer,xmax, ymin);
    cdCanvasEnd(obj->cddbuffer);
  }

  cdCanvasForeground(obj->cddbuffer,CD_BLACK);
  cdCanvasBegin(obj->cddbuffer,CD_CLOSED_LINES);
  cdCanvasVertex(obj->cddbuffer,xmin, ymin); cdCanvasVertex(obj->cddbuffer,xmin, ymax);
  cdCanvasVertex(obj->cddbuffer,xmax, ymax); cdCanvasVertex(obj->cddbuffer,xmax, ymin);
  cdCanvasEnd(obj->cddbuffer);
}

/* ...................................................................... */
/** 
 * This function is used to repaint the preview area.
 * @param obj the widget data pointer.  
 */
static void repaint_preview(Tcolorbar* obj) {
  int delta = DELTA;
  int xmin, xmax, ymin, ymax;
  int bg = obj->bgcolor_idx;
  int fg = obj->fgcolor_idx;
  get_preview_limit(obj, &xmin, &xmax, &ymin, &ymax);

  if (xmax-xmin < delta || ymax-ymin < delta) delta = 0;
  
  if (obj->show_secondary) { 
     int xhalf = 2*(xmax-xmin-2*delta)/3 + delta;
     int yhalf = 2*(ymax-ymin-2*delta)/3 + delta;
     draw_box(obj, xmax-xhalf, xmax-delta, ymin+delta, ymin+yhalf, bg);
     draw_box(obj, xmin+delta, xmin+xhalf, ymax-yhalf, ymax-delta, fg);
  }
  else { 
     draw_box(obj, xmin+delta, xmax-delta, ymin+delta, ymax-delta, fg);
  }
}

/* ...................................................................... */
/** 
 * This function is used to repaint a cell.
 * @param obj the widget data pointer.
 * @param idx the cell index 
 */

static void repaint_cell(Tcolorbar* obj, int idx) {
  int delta = 2;
  int xmin, xmax, ymin, ymax;
  get_cell_limit(obj, idx, &xmin, &xmax, &ymin, &ymax);
  xmin += delta; xmax -= delta; ymin += delta; ymax -= delta;
  draw_box(obj, xmin, xmax, ymin, ymax, idx);
  draw_sunken(obj, xmin, xmax, ymin, ymax);
}

/* ...................................................................... */
/** 
 * This function is used to repaint the widget.
 * @param obj the widget data pointer.  
 */
static void repaint_all(Tcolorbar* obj) {
  long int colorbg_cd = 0;

  /* Checking if the widget is buffering */
  if (obj->bufferize == 1) return;

  /* Instrumentation */
  if (obj->cddbuffer == NULL) return;
  if (obj->self == NULL) return;

  /* Using the IUP_BGCOLOR attribute the draw the background */
  colorbg_cd = cdIupConvertColor(iupGetBgColor(obj->self));
  cdIupCalcShadows(colorbg_cd, &obj->light_shadow, &obj->mid_shadow, &obj->dark_shadow);

  /* Drawing backgound */
  cdCanvasBackground(obj->cddbuffer,colorbg_cd); cdCanvasForeground(obj->cddbuffer,colorbg_cd);
  cdCanvasClear(obj->cddbuffer);

  /* Drawing the preview area */
  repaint_preview(obj);

  /* Drawing the color cells area */
  repaint_cells(obj);

  cdCanvasFlush(obj->cddbuffer);

}

/* ...................................................................... */
/** 
 * This function loops all cells, repainting them.
 * @param obj the widget data pointer.
 * @see repaint_cell 
 */
static void repaint_cells(Tcolorbar* obj) {
  int i;
  for (i = 0; i < obj->num_cells; i++) repaint_cell(obj, i);
}

/* ...................................................................... */
/** 
 * This function is used to get the index color of a canvas coordinate.
 * @param obj the widget data pointer.
 * @param x the x coordinate
 * @param y the y coordinate
 * @return the cell index of the given coordinate. 
 * @see get_cell_limit 
 */
static int get_index_color(Tcolorbar* obj, int x, int y) {
  int xmin, xmax, ymin, ymax;
  int i, result = -9; 
  for (i = 0; i < obj->num_cells; i++) { 
     get_cell_limit(obj, i, &xmin, &xmax, &ymin, &ymax);
     if (x > xmin && x < xmax && y > ymin && y < ymax) { 
        result = i;
        break;
    }
  }
  return result;
}

static int check_preview(Tcolorbar* obj, int x, int y) {
  int xmin, xmax, ymin, ymax;
  int delta = DELTA;
  get_preview_limit(obj, &xmin, &xmax, &ymin, &ymax);

  if (obj->show_secondary) { 
     int xhalf = 2*(xmax-xmin-2*delta)/3 + delta;
     int yhalf = 2*(ymax-ymin-2*delta)/3 + delta;
     if (x > xmin+delta && x < xmin+xhalf && y > ymax-yhalf && y < ymax-delta)
       return -1;
     if (x > xmax-xhalf && x < xmax-delta && y > ymin+delta && y < ymin+yhalf)
       return -2;
     if (x > xmin && x < xmax && y > ymin && y < ymax)
       return 1; /* switch */
  }
  else { 
     if (x > xmin+delta && x < xmax-delta && y > ymin+delta && y < ymax-delta)
       return -1;
  }

  return 0;
}

/* ...................................................................... */
/** 
 * This function is used to get the preview area bounding box.
 * @param obj the widget pointer.
 * @param xmin the x minimum value (reference).
 * @param xmax the x minimum value (reference).
 * @param ymin the y minimum value (reference).
 * @param ymax the y maximum value (reference).
 * @return written references 
 */

static void get_preview_limit(Tcolorbar* obj, int* xmin, int* xmax, 
int* ymin, int* ymax) {
  int num_itens = obj->num_cells / obj->num_parts + 1; /* include space for preview area */

  *xmin = 0; *ymin = 0;
  if (obj->vertical) { 
     *xmax = obj->w;
     *ymax = obj->h / num_itens;
     if (obj->preview_size > 0) *ymax = *ymin + obj->preview_size;
  }
  else { 
     *ymax = obj->h;
     *xmax = obj->w / num_itens;
     if (obj->preview_size > 0) *xmax = *xmin + obj->preview_size;
  }
  if (obj->squared) fit_square(xmin, xmax, ymin, ymax);
}

/* ...................................................................... */
/** 
 * This function is used to get a cell bounding box.
 * @param obj the widget pointer.
 * @param idx the cell index.
 * @param xmin the x minimum value (reference).
 * @param xmax the x minimum value (reference).
 * @param ymin the y minimum value (reference).
 * @param ymax the y maximum value (reference).
 * @return written references 
 */
static void get_cell_limit(Tcolorbar* obj, int idx, int* xmin, int* xmax, 
int* ymin, int* ymax) {

  int num_itens = obj->num_cells / obj->num_parts;
  int delta;
  int trash, px, py;

  get_preview_limit(obj, &trash, &px, &trash, &py);

  /* Vertical orientation */
  if (obj->vertical) { 
     int wcell = obj->w / obj->num_parts;
     int hcell = (obj->h - py) / num_itens;
     int posx = idx / num_itens;
     int posy = idx % num_itens;
     if (obj->squared) { 
        wcell = wcell < hcell ? wcell : hcell;
        hcell = wcell;
     }
     delta = (obj->w - (obj->num_parts*wcell)) / 2;
     *xmin = delta + (posx+0)*wcell; 
     *xmax = delta + (posx+1)*wcell;
     *ymin = py + (posy+0)*hcell;
     *ymax = py + (posy+1)*hcell;
  }
  /* Horizontal orientation */
  else { 
     int hcell = obj->h / obj->num_parts;
     int wcell = (obj->w - px) / num_itens;
     int posx = idx % num_itens;
     int posy = idx / num_itens;
     if (obj->squared) { 
        wcell = wcell < hcell ? wcell : hcell;
        hcell = wcell;
     }
     delta = (obj->h - (obj->num_parts*hcell)) / 2;
     *xmin = px + (posx+0)*wcell; 
     *xmax = px + (posx+1)*wcell;
     *ymin = delta + (posy+0)*hcell;
     *ymax = delta + (posy+1)*hcell;
  }
}


/* ...................................................................... */
/** 
 * This function is used to get the largest square of a cell bounding box.
 * @param xmin the x minimum value (reference).
 * @param xmax the x minimum value (reference).
 * @param ymin the y minimum value (reference).
 * @param ymax the y maximum value (reference).  
 */
static void fit_square(int* xmin, int* xmax, int* ymin, int* ymax) {
  int mx = (*xmax + *xmin) / 2; 
  int my = (*ymax + *ymin) / 2;
  int dx = (*xmax - *xmin) / 2; 
  int dy = (*ymax - *ymin) / 2;

  if (dx < dy) { 
     *ymin = my - dx; 
     *ymax = my + dx; 
  }
  else { 
     *xmin = mx - dy; 
     *xmax = mx + dy; 
  }
}
