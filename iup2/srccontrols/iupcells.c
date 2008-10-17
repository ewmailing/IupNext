/**
 * @file iupcells.c
 * Arquivo de implementação do controle IupCells em C (IUP/CPI).
 * @author André Luiz Clinio.
 */

/* ====================================================================== */
/* ====================================================================== */
/* ====================================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#undef CD_NO_OLD_INTERFACE

#include <iup.h>
#include <cd.h>
#include <cdiup.h>
#include <cddbuf.h>
#include <iupcpi.h>
#include <iupcompat.h>

#include "iupcells.h"
#include "iupcdutil.h"

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

/** 
 * Cells internal structure.
 * This struct is used to store the control (object) 
 * data so that it can be allways recovered (with a handle)
 * from iup callbacks 
 * @author André Luiz Clinio.
 */ 
typedef struct {

   /** handle for iup canvas */
   Ihandle* self;          
  
   /** cd canvas for drawing */
   cdCanvas* cdcanvas;     

   /** image canvas for double buffering */
   cdCanvas* cddbuffer;    

   /** control width (pixels) */
   int width;         

   /** control height (pixels) */
   int height;        

   /** cells bounding box clipping activated */
   int clipped;         

   /** draw cells bounding box activated */
   int boxed;         

   /** bufferize on */
   int bufferize;     

   /** number of non-scrollable lines */
   int non_scrollable_lins;  

   /** number of non-scrollable columns */
   int non_scrollable_cols;   

   /** Empty area color */
   long int no_color;
} TCells;

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

/** Mouse click/unclick callback */
typedef int (*TButtonCb)(Ihandle*, int, int, int, int, int, int, char*);

/** Aplication repaint callback */
typedef int (*TDrawCb)(Ihandle*, int, int, int, int, int, int, cdCanvas* cnv);

/** Number of lines callback */
typedef int (*TLinesCb)(Ihandle*);

/** Mouse motion callback */
typedef int (*TMotionCb)(Ihandle*, int, int, int, int, char*);

/** Number of columns callback */
typedef int (*TColsCb)(Ihandle*);

/** Columns width callback */
typedef int (*TWidthCb)(Ihandle*, int);

/** Lines height callback */
typedef int (*THeightCb)(Ihandle*, int);

/** Horizontal span callback */
typedef int (*TSpanHorCb)(Ihandle*, int, int);

/** Vertical span callback */
typedef int (*TSpanVerCb)(Ihandle*, int, int);

/** Scroll bar callback */
typedef int (*TScrollCb)(Ihandle*, int, int);

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

static void set_full_visible(TCells* obj, int i, int j);
static void adjust_scrolls(TCells* obj);

static int get_first_line(TCells* obj);
static int get_first_col(TCells* obj);
static int adjust_origin(TCells* obj, char* attr);

static char* cd_to_iup(long int col);

static void get_virtual_size(TCells* obj, int* wi, int* he);
static void call_apl_draw(TCells* obj, int xmin, int xmax, 
 int ymin, int ymax, int i, int j);

static int get_height(TCells* obj, int i);
static int get_width(TCells* obj, int j);
static int get_nlines(TCells* obj);
static int get_ncols(TCells* obj);
static int get_hspan(TCells* obj, int i, int j);
static int get_vspan(TCells* obj, int i, int j);
static int get_coord(TCells* obj, int x, int y, int* i, int* j);

static int get_ranged_coord(TCells* obj, int x, int y, int* lin, int* col,
  int linfrom, int linto, int colfrom, int colto);

static int get_ranged_width(TCells* obj, int from, int to);
static int get_ranged_height(TCells* obj, int from, int to);

static int get_cell_limit(TCells* obj, int i, int j,
  int* xmin, int* xmax, int* ymin, int* ymax);

static void repaint_all(TCells* obj);
static void repaint_cells(TCells* obj);
static void repaint_ranged_cells(TCells* obj, int linfrom, int linto,
  int colfrom, int colto);

static void repaint_cell_in(TCells* obj, int i, int j, 
       int xmin, int xmax, int ymin, int ymax);

static Ihandle* cells_create(Iclass *ic, void **params);
static void cells_destroy(Ihandle *h);
static char* cells_getattrib(Ihandle *h, char *attr);
static void cells_setattrib(Ihandle *h, char *attr, char *value);

static int cells_button(Ihandle *h, int b, int m, int x, int y, char* r);
static int cells_motion(Ihandle *h, int x, int y, char* r);
static int cells_scroll(Ihandle *h);
static int cells_repaint(Ihandle *h);
static int cells_resize(Ihandle *h);

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

/* ...................................................................... */
/** 
 * Exported function used to create a new IupCells element.
 * @param nothing
 * @return a handle pointer to the new IupCells created 
 * or NULL if an erros has occurred. 
 */ 
Ihandle *IupCells(void) {
  /* Returning the created object */
  return IupCreate("cells");
}

/* ...................................................................... */
/** 
 * Exported function used to create a new IupCells element.
 */
void IupCellsOpen(void) {
  /* Creating a new class for CELLS */
  Iclass* ICCells = iupCpiCreateNewClass("cells", NULL);

  /* Setting class cpi methods */
  iupCpiSetClassMethod(ICCells, ICPI_CREATE, (Imethod) cells_create);
  iupCpiSetClassMethod(ICCells, ICPI_DESTROY, (Imethod) cells_destroy);
  iupCpiSetClassMethod(ICCells, ICPI_SETATTR, (Imethod) cells_setattrib);
  iupCpiSetClassMethod(ICCells, ICPI_GETATTR, (Imethod) cells_getattrib);
}

/* ...................................................................... */
/** 
 * Function used to get an attribute value of a IupCells object
 * @param h handle to the IupCells object 
 * @param attr attribute to be read
 * @return the attribute value 
 */ 
static char* cells_getattrib(Ihandle *h, char *attr) {
  static char buffer[ 80 ];
  TCells* obj = NULL;

  /* Getting cell struct pointer saved in iup handle */
  obj = (TCells*) iupGetEnv(h, "_IUPCELLS_DATA");
  if (obj == NULL) return iupCpiDefaultGetAttr(h, attr);

  /* Sequence of ifs, onde for each attribute */
  if (iupStrEqual(attr, IUP_BOXED)) { 
     if (obj->boxed == 0) return IUP_NO;
     else return IUP_YES;
  }
  else if (iupStrEqual(attr, IUP_CLIPPED)) { 
     if (obj->clipped == 0) return IUP_NO;
     else return IUP_YES;
  }
  else if (iupStrEqual(attr, IUP_NO_COLOR)) { 
     return (char*) cd_to_iup(obj->no_color);
  }
  else if (iupStrEqual(attr, IUP_IMAGE_CANVAS)) { 
     return (char*) obj->cddbuffer;
  }
  else if (iupStrEqual(attr, IUP_CANVAS)) { 
     return (char*) obj->cdcanvas;
  }
  else if (iupStrEqual(attr, IUP_FIRST_LINE)) { 
     sprintf(buffer, "%d", get_first_line(obj));
     return buffer;
  }
  else if (iupStrEqual(attr, IUP_FIRST_COL)) { 
     sprintf(buffer, "%d", get_first_col(obj));
     return buffer;
  }
  else if (iupStrEqual(attr, IUP_NON_SCROLLABLE_COLS)) { 
     sprintf(buffer, "%d", obj->non_scrollable_cols);
     return buffer;
  }
  else if (iupStrEqual(attr, IUP_NON_SCROLLABLE_LINES)) { 
     sprintf(buffer, "%d", obj->non_scrollable_lins);
     return buffer;
  }
  else if (strstr(attr, IUP_LIMITS)) { 
    int xmin, xmax, ymin, ymax;
    int i, j;
    int read = sscanf(attr, "%[^:]:%d:%d", buffer, &i, &j);
    if (read != 3) return NULL;
    get_cell_limit(obj, i, j, &xmin, &xmax, &ymin, &ymax);
    sprintf(buffer, "%d:%d:%d:%d", xmin, xmax, ymin, ymax);
    return buffer;
  }
  /* default case: using default iup search in hash table */
  else return iupCpiDefaultGetAttr(h, attr);
}

/* ...................................................................... */
/** 
 * Function used to set an attribute value of a IupCells object
 * @param h handle to the IupCells object 
 * @param attr attribute to be read
 * @param value the new attribute value
 */
static void cells_setattrib(Ihandle *h, char *attr, char *value) {

  /* Getting cell struct pointer saved in iup handle */
  TCells* obj = (TCells*) iupGetEnv(h, "_IUPCELLS_DATA");

  if (obj == NULL || attr == NULL) return;
 
  if (iupStrEqual(attr, IUP_BUFFERIZE)) { 
     if (value == NULL || iupStrEqualNoCase(value, IUP_NO)) { 
        obj->bufferize = 0;
        adjust_scrolls(obj);
        repaint_all(obj);
     }
     else obj->bufferize = 1;
  }
  else if (iupStrEqual(attr, IUP_REPAINT)) { 
     obj->bufferize = 0;
     adjust_scrolls(obj);
     repaint_all(obj); 
  }
  else if (iupStrEqual(attr, IUP_NO_COLOR)) { 
     if (value != NULL) obj->no_color = cdIupConvertColor(value);
     repaint_all(obj); 
  }
  else if (iupStrEqual(attr, IUP_ORIGIN)) { 
     if (adjust_origin(obj, value)) repaint_all(obj);
  }
  else if (iupStrEqual(attr, IUP_NON_SCROLLABLE_COLS)) { 
     obj->non_scrollable_cols = atoi(value);
     repaint_all(obj);
  }
  else if (iupStrEqual(attr, IUP_NON_SCROLLABLE_LINES)) { 
     obj->non_scrollable_lins = atoi(value);
     repaint_all(obj);
  }
  else if (iupStrEqual(attr, IUP_BOXED)) { 
     if (value != NULL && iupStrEqualNoCase(value, IUP_NO)) obj->boxed = 0;
     else obj->boxed = 1;
     repaint_all(obj);
  }
  else if (iupStrEqual(attr, IUP_CLIPPED)) { 
     if (value != NULL && iupStrEqualNoCase(value, IUP_NO)) obj->clipped = 0;
     else obj->clipped = 1;
     repaint_all(obj);
  }
  else if (iupStrEqual(attr, IUP_FULL_VISIBLE)) { 
     int i, j;
     if (iupStrToIntInt(value, &i, &j, ':') != 2) return;
     set_full_visible(obj, i, j);
     repaint_all(obj);
  }
  /* default case: using default iup adition in hash table */
  else iupCpiDefaultSetAttr(h, attr, value);
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

/* ...................................................................... */

/** 
 * IUP canvas callback for mouse press/unpress. 
 */
static int cells_scroll(Ihandle *h) {
  TScrollCb func = NULL;
  int value = IUP_DEFAULT;
  int fline = -999;
  int fcol = -999;

  /* Getting cell struct pointer saved in iup handle */
  TCells* obj = (TCells*) iupGetEnv(h, "_IUPCELLS_DATA");
  if (obj == NULL) return IUP_IGNORE;
  fline = get_first_line(obj);
  fcol = get_first_col(obj);

  /* Checking the existence of a scroll bar callback. If the application
   * has set one, it will be called now. If the application returns
   * IUP_DEFAULT, the cells will be repainted. */
  func = (TScrollCb) IupGetCallback(h, "SCROLLING_CB");
  if (func != NULL) value = (*func)(obj->self, fline, fcol);
  if (value == IUP_DEFAULT) repaint_all(obj);

  /* Returning the callback */
  return value;
}

/* ...................................................................... */

/** 
 * IUP canvas callback for mouse press/unpress. 
 */
static int cells_motion(Ihandle *h, int x, int y, char* r) {
  int returned_value = IUP_DEFAULT;

  /* Getting cell struct pointer saved in iup handle */
  TCells* obj = (TCells*) iupGetEnv(h, "_IUPCELLS_DATA");
  if (obj == NULL) return IUP_IGNORE;

  /* Checking the existence of a motion bar callback. If the application
   * has set one, it will be called now. However, before calling the
   * callback, we need to find out which cell is under the mouse
   * position. */
  
  { int i, j; 
    TMotionCb func = (TMotionCb)IupGetCallback(h, "MOUSEMOTION_CB");
    if (func == NULL) return IUP_DEFAULT;
    get_coord(obj, x, y, &i, &j); 
    returned_value = (*(func))(h, i, j, x, y, r);
  }
  return returned_value;
}

/* ...................................................................... */

static void cells_createcanvas(TCells* obj, Ihandle *self)
{
  if (!obj->cdcanvas)
    obj->cdcanvas = cdCreateCanvas(CD_IUP, self);

  if (obj->cdcanvas)
  {
    cdCanvasActivate(obj->cdcanvas);
    obj->cddbuffer = cdCreateCanvas(CD_DBUFFER, obj->cdcanvas);
  }
}

/** 
 * IUP canvas callback for mouse press/unpress. 
 */
static int cells_repaint(Ihandle *h) {
  /* Getting cell struct pointer saved in iup handle */
  TCells* obj = (TCells*) iupGetEnv(h, "_IUPCELLS_DATA");
  if (!obj) return IUP_DEFAULT;

  if (!obj->cddbuffer)
    cells_createcanvas(obj, h);

  if (!obj->cddbuffer)
    return IUP_DEFAULT;

  /* Just repainting all cells */
  repaint_all(obj);

  return IUP_DEFAULT;
}

/* ...................................................................... */

/** 
 * IUP canvas callback for resize 
 */
static int cells_resize(Ihandle *h) {
  /* Getting cell struct pointer saved in iup handle */
  TCells* obj = (TCells*) iupGetEnv(h, "_IUPCELLS_DATA");
  if (!obj) return IUP_DEFAULT;

  if (!obj->cddbuffer)
    cells_createcanvas(obj, h);

  if (!obj->cddbuffer)
    return IUP_DEFAULT;

  cdCanvasActivate(obj->cddbuffer);
  cdCanvasGetSize(obj->cddbuffer,&obj->width, &obj->height, NULL, NULL);

  /* As the size has changed, we need to recalculate scrollbars
   * positions and size */
  adjust_scrolls(obj);

  return IUP_DEFAULT;
}

/* ...................................................................... */
/** 
 * IUP/CPI method for destroying an object  
 */
static void cells_destroy(Ihandle *h) {
  /* Getting cell struct pointer saved in iup handle */
  TCells* obj = (TCells*) iupGetEnv(h, "_IUPCELLS_DATA");
  if (obj == NULL) return;

  /* Destroying cd canvases */
  if (obj->cdcanvas != NULL) 
  { 
    cdKillCanvas(obj->cdcanvas);
    obj->cdcanvas = NULL;
  }

  if (obj->cddbuffer != NULL) 
  { 
    cdKillCanvas(obj->cddbuffer);
    obj->cddbuffer = NULL;
  }

  iupSetEnv(h, "_IUPCELLS_DATA", NULL);

  free(obj);
}

/* ...................................................................... */
/** 
 * IUP canvas callback for mouse press/unpress. 
 */
static int cells_button(Ihandle *h, int b, int m, int x, int y, char* r) {
  int i, j;
    
  /* Getting cell struct pointer saved in iup handle */
  TCells* obj = (TCells*) iupGetEnv(h, "_IUPCELLS_DATA");
  if (obj == NULL) return IUP_DEFAULT;

  cdCanvasUpdateYAxis(obj->cdcanvas, &y);  

  /* Treating the button event. The application will receive
   * a button press callback. */
    
  /* Checking if there is a callback and calling it. */
  { 
     TButtonCb func = (TButtonCb)IupGetCallback(h, "MOUSECLICK_CB");
     if (func == NULL) return IUP_DEFAULT;
     get_coord(obj, x, y, &i, &j);       /* Getting cells coordinates. */
     (*(func))(h, b, m, i, j, x, y, r);  /* Calling the callback */
  }

  return IUP_DEFAULT;
}

/* ...................................................................... */
/** 
 * IUP/CPI method for creating an object  
 */
static Ihandle* cells_create(Iclass *ic, void **params) {
  /* Allocating cell struct pointer saved in iup handle */
  TCells* obj = (TCells*) malloc(sizeof(TCells));
  
  /* Creating iup canvas for the object */
  Ihandle* self = IupCanvas(NULL);

  (void)ic;
  (void)params;

  iupSetEnv(self, "_IUPCELLS_DATA", (char*)obj); 
  iupSetEnv(self, IUP_SCROLLBAR, IUP_YES);
  iupSetEnv(self, IUP_BORDER, IUP_NO);

  /* Setting object's iup callbacks */
  IupSetCallback(self, IUP_RESIZE_CB, (Icallback) cells_resize);
  IupSetCallback(self, IUP_ACTION,    (Icallback) cells_repaint);
  IupSetCallback(self, IUP_BUTTON_CB, (Icallback) cells_button);
  IupSetCallback(self, IUP_MOTION_CB, (Icallback) cells_motion);
  IupSetCallback(self, IUP_SCROLL_CB, (Icallback) cells_scroll);

  /* Initializing object with no cd canvases */
  obj->cddbuffer = NULL;
  obj->cdcanvas = NULL;

  /* Default values for object stored attributes */
  obj->boxed = 1;
  obj->clipped = 1;
  obj->bufferize = 0;
  obj->non_scrollable_lins = 0;
  obj->non_scrollable_cols = 0;
  obj->no_color = -1;

  iupSetEnv(self, IUP_XMIN, "0");
  iupSetEnv(self, IUP_YMIN, "0");
  iupSetEnv(self, IUP_YMAX, "1");
  iupSetEnv(self, IUP_XMAX, "1");
  iupSetEnv(self, IUP_DY,   "1");
  iupSetEnv(self, IUP_DX,   "1");
  iupSetEnv(self, IUP_POSY, "0");
  iupSetEnv(self, IUP_POSX, "0");

  /* Storing iup handle inside the object structure */
  obj->self = self;
  return self;
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

/* ...................................................................... */
/** 
 * Function used to call the client; is used when a cell must be repainted.
 */
static void call_apl_draw(TCells* obj, int xmin, int xmax, 
int ymin, int ymax, int i, int j) {
  int cxmin, cxmax, cymin, cymax;
  int oldxmin, oldxmax, oldymin, oldymax;
  int w = obj->width;
  int h = obj->height;

  /* Getting clipping area for post restore */
  int oldclip = cdCanvasClip(obj->cddbuffer,CD_QUERY);
  cdCanvasGetClipArea(obj->cddbuffer,&oldxmin, &oldxmax, &oldymin, &oldymax);

  /* Clipoing the cell area */
  if (obj->clipped) { 
     cdCanvasClip(obj->cddbuffer, CD_CLIPAREA);
     cxmin = xmin < 0 ? 0 : xmin;
     cymin = ymin < 0 ? 0 : ymin;
     cxmax = xmax > w ? w : xmax;
     cymax = ymax > h ? h : ymax;
     cdCanvasClipArea(obj->cddbuffer, xmin, xmax, ymin, ymax);
  }

  /* Calling the application draw callback. */
  { 
     TDrawCb func = (TDrawCb)IupGetCallback(obj->self, IUP_DRAW_CB);
     if (func)
     {
       cdCanvas* old_cnv = cdActiveCanvas();
       if (old_cnv != obj->cddbuffer) /* backward compatibility code */
       cdActivate(obj->cddbuffer);

       func(obj->self, i, j, xmin, xmax, ymin, ymax, obj->cddbuffer);

       if (old_cnv != obj->cddbuffer)
         cdActivate(old_cnv);
     }
  }

  /* Restoring clipping attributes */
  cdCanvasClip(obj->cddbuffer,oldclip);
  cdCanvasClipArea(obj->cddbuffer, oldxmin, oldxmax, oldymin, oldymax);
}

/* ...................................................................... */
/** 
 * Repaint function for one cell in a given coordinate;
 *  (assume that the canvas is already activated). 
 */
static void repaint_cell_in(TCells* obj, int i, int j, 
int xmin, int xmax, int ymin, int ymax) {
  int k;
  int w = obj->width;
  int h = obj->height;
  int hspan = 1;
  int vspan = 1;

  /* Checking if the cells is out of range. (no span will affect it!) */
  if (xmin > w || ymax < 0) return;
 
  /* Calculating cell spans */
  hspan = get_hspan(obj, i, j);
  vspan = get_vspan(obj, i, j);

  /* if any span is set to zero, then another cell invaded its space and
   * the cell does not need to draw itself */
  if (hspan == 0 || vspan == 0) return;

  /* Increasing cell's width and height according to its spans */
  for(k = 1; k < hspan; k++) xmax += get_width(obj, j+k);
  for(k = 1; k < vspan; k++) ymin -= get_height(obj, i+k);

  /* Checking if the cell expanded enough to appear inside the canvas */
  if (xmax < 0 || ymin > h) return;

  /* Calling application's draw callback */
  call_apl_draw(obj, xmin, xmax, ymin, ymax, i, j);

  /* Drawing a box in cell's area */
  if (obj->boxed) { 
     cdCanvasForeground(obj->cddbuffer,CD_BLACK);
     cdCanvasRect(obj->cddbuffer,xmin, xmax, ymin, ymax);
  }
}

/* ...................................................................... */

/** 
 * Repaint function, but it is the only one that activates the cd canvas 
 */
static void repaint_all(TCells* obj) {

  /* If object is buffering, it will be not drawn */
  if (obj->bufferize == 1) return;

  /* Checking erros or not initialized conditions */
  if (obj->self == NULL) return;
  if (obj->cddbuffer == NULL) return;

  if (obj->no_color == -1)
    obj->no_color = cdIupConvertColor(iupGetBgColor(obj->self));

  /* Getting object background color and repainting it.*/
  cdCanvasBackground(obj->cddbuffer, obj->no_color); 
  cdCanvasClear(obj->cddbuffer);

  /* Repaiting all cells */
  repaint_cells(obj);

  /* Flushing drawing to cd double buffer canvas */
  cdCanvasFlush(obj->cddbuffer);
}

/* ...................................................................... */

/** Repaint function for all cells in a given range 
 *  (assume that the canvas is already activated) 
 */
static void repaint_ranged_cells(TCells* obj, int linfrom, int linto,
int colfrom, int colto) {
  int i, j;
  int xmin, xmax, ymin, ymax;
  int refxmin, refxmax;

  /* Getting first cell limits: this function is slow to be called everytime */
  get_cell_limit(obj, linfrom, colfrom, &xmin, &xmax, &ymin, &ymax);

  /* Initializing current reference position */
  refxmin = xmin; 
  refxmax = xmax;

  /* Looping through the cells adding next cell width and height */
  for (i = linfrom; i <= linto; i++) { 
      xmin = refxmin; 
      xmax = refxmax;
      for (j = colfrom; j <= colto; j++) { 
          repaint_cell_in(obj, i, j, xmin, xmax, ymin, ymax);
          xmin = xmax;
          xmax = xmin + (j == colto ? 0 : get_width(obj, j+1));
      }
      ymax = ymin;
      ymin = ymax - (i == linto ? 0 : get_height(obj, i+1));
  }
}

/* ...................................................................... */

/** 
 * Function used to calculate a group of columns height
 * @param obj pointer to internal object handle
 * @param from initial line
 * @param to last line
 * @return line group height (pixels) 
 */
static int get_ranged_height(TCells* obj, int from, int to) {
  int i;
  int result = 0;
  
  /* Looping through a column adding the line height */
  for (i = from; i <= to; i++) result += get_height(obj, i);
  return result;
}

/* ...................................................................... */
/** Function used to calculate a group of lines width 
 * @param obj pointer to internal object handle
 * @param from initial column
 * @param to last column
 * @return column group width (pixels) 
 */
static int get_ranged_width(TCells* obj, int from, int to) {
  int j;
  int result = 0;

  /* Looping through a column adding the column width */
  for (j = from; j <= to; j++) result += get_width(obj, j);
  return result;
}

/* ...................................................................... */

/** 
 * Repaint function for all cells 
 * (assume that the canvas is already activated) 
 */
static void repaint_cells(TCells* obj) {
  int sline = obj->non_scrollable_lins;
  int scol = obj->non_scrollable_cols;
  int nlines = get_nlines(obj);
  int ncols = get_ncols(obj);

  /* Repainting the four parts of the cells: common cells, non-scrollable
   * columns, non-scrollable lines, and non-scrollable margin 
   * (line and column) */
  repaint_ranged_cells(obj, sline+1, nlines, scol+1, ncols);

  repaint_ranged_cells(obj, sline+1, nlines, 1, scol);
  repaint_ranged_cells(obj, 1, sline, scol+1, ncols);

  repaint_ranged_cells(obj, 1, sline, 1, scol);
}

/* ...................................................................... */

/** 
 * Function used to turn a cell visible 
 * @param i cell line number.
 * @param j cell column number.
 */
static void set_full_visible(TCells* obj, int i, int j) {
  char buffer[ 32 ];
  int xmin, xmax, ymin, ymax;
  int posx = IupGetInt(obj->self, IUP_POSX);
  int posy = IupGetInt(obj->self, IUP_POSY);
  int dx = 0, dy = 0;
  
  /* Getting the frontiers positions for the visible cell */
  int min_x = get_ranged_width(obj, 1, obj->non_scrollable_cols);
  int max_y = obj->height - get_ranged_height(obj, 1, obj->non_scrollable_lins); 

  /* Getting the cell's area limit */
  get_cell_limit(obj, i, j, &xmin, &xmax, &ymin, &ymax);

  /* Adjusting the diference of the scrollbars' position (horizontal) */
  if (xmax > obj->width) dx = xmax - obj->width;
  /* Giving priority to xmin position. This can be seen by the usage
   * of dx at the left part of the expression (using the last dx).
   * This is the case wher the cell cannot be fitted. */
  if (xmin - dx < min_x) dx = - (min_x - xmin); 

  /* Adjusting the diference of the scrollbars' position (horizontal) */
  if (ymax > max_y) dy = - (ymax - max_y);
  if (ymin < 0) dy = -ymin;

  /* Adding the diference to scrollbars' position */
  posx += dx; posy += dy;

  /* Setting iup scrollbars' attributes */
  sprintf(buffer, "%.1f", (float)posx);
  IupStoreAttribute(obj->self, IUP_POSX, buffer); 
  sprintf(buffer, "%.1f", (float)posy);
  IupStoreAttribute(obj->self, IUP_POSY, buffer); 
}

/* ...................................................................... */
/** 
 * Function used to calculate the cell coordinates 
 */
static int get_coord(TCells* obj, int x, int y, int* lin, int* col) {
  int pck = 0;
  int sline = obj->non_scrollable_lins;
  int scol = obj->non_scrollable_cols;
  int nlines = get_nlines(obj);
  int ncols = get_ncols(obj);

  /* Trying to pick a cell (raster coordinates) at the four 
   * parts of the cells (reverse order of the repainting):
   * non-scrollable margin (line and column), non-scrollable 
   * columns, non-scrollable lines, and common cells. */
  pck = get_ranged_coord(obj, x, y, lin, col, 1, sline, 1, scol);
  if (pck) return 1;

  pck = get_ranged_coord(obj, x, y, lin, col, 1, sline, scol+1, ncols);
  if (pck) return 1;

  pck = get_ranged_coord(obj, x, y, lin, col, sline+1, nlines, 1, scol);
  if (pck) return 1;

  pck = get_ranged_coord(obj, x, y, lin, col, 1, nlines, 1, ncols);
  return pck;
}

/* ...................................................................... */
/** 
 * Function used to calculate the cell coordinates limited by a hint 
 */
static int get_ranged_coord(TCells* obj, int x, int y, int* lin, int* col,
int linfrom, int linto, int colfrom, int colto) {
  int i, j, k;
  int xmin, xmax, ymin, ymax;
  int refxmin, refxmax;
  int w = obj->width;
 
  /* Getting the first cell's  limit -- based on the range */
  get_cell_limit(obj, linfrom, colfrom, &xmin, &xmax, &ymin, &ymax);

  /* Initializing reference position */
  refxmin = xmin; 
  refxmax = xmax;
  
  /* Looping through the cells adding next cell width and height */
  for (i = linfrom; i <= linto; i++) { 
      xmin = refxmin; 
      xmax = refxmax;
      if (!(ymax < 0)) { 
         for (j = colfrom; j <= colto; j++) { 
             int hspan = 1;
             int vspan = 1;
             if (!(xmin > w)) { 
                hspan = get_hspan(obj, i, j);
                vspan = get_vspan(obj, i, j);
                if (hspan != 0 && vspan != 0) { 
                   int rxmax = xmax; 
                   int rymin = ymin;
                   for(k = 1; k < hspan; k++) rxmax += get_width(obj, j+k);
                   for(k = 1; k < vspan; k++) rymin -= get_height(obj, i+k);
  
                   /* A cell was found */
                   if (x >= xmin && x <= rxmax && y >= rymin && y <= ymax) { 
                      *lin = i; 
                      *col = j; 
                      return 1; 
                   }
                }
                xmin = xmax;
                xmax = xmin + (j == colto ? 0 : get_width(obj, j+1));
            }
        }
    }
    ymax = ymin;
    ymin = ymax - (i == linto ? 0 : get_height(obj, i+1));
  }

  /* No cell selected... */
  *lin = IUP_OUT;
  *col = IUP_OUT;
  return 0;
}

/* ...................................................................... */
/** 
 * Function to inquire application cell data (vertical span). 
 */
static int get_vspan(TCells* obj, int i, int j) {
  int result = 1; /* default value */

  /* If there is a vspan callback, the application will answer it */
  { 
     TSpanVerCb func = (TSpanVerCb)IupGetCallback(obj->self, "VSPAN_CB");
     if (func != NULL) result = (*(func))(obj->self, i, j);
  }
  if (result < 0) result = 1;
  return result;
}

/* ...................................................................... */
/** 
 * Function to inquire application cell data (horizontal span). 
 * @param obj Internal object handler.
 * @param i line number.
 * @param j column number.
 * @return the span horizontal value.
 */
static int get_hspan(TCells* obj, int i, int j) {
  int result = 1; /* default value */
  
  /* If there is a hspan callback, the application will answer it */
  { 
    TSpanHorCb func = (TSpanHorCb)IupGetCallback(obj->self, "HSPAN_CB");
    if (func != NULL) result = (*(func))(obj->self, i, j);
  }
  if (result < 0) result = 1;
  return result;
}

/* ...................................................................... */
/** 
 * Function to inquire application cell data (#columns). 
 */
static int get_ncols(TCells* obj) {
  int result = 10; /* default value */

  /* If there is a ncols callback, the application will answer it */
  { 
    TColsCb func = (TColsCb)IupGetCallback(obj->self, "NCOLS_CB");
    if (func != NULL) result = (*(func))(obj->self);
  }
  if (result < 0) result = 0;
  return result;
}

/* ...................................................................... */
/** 
 * Function to inquire application cell data (# lines). 
 */
static int get_nlines(TCells* obj) {
  int result = 10; /* default value */

  /* If there is a nlines callback, the application will answer it */
  { 
     TLinesCb func = (TLinesCb)IupGetCallback(obj->self, "NLINES_CB");
     if (func != NULL) result = (*(func))(obj->self);
  }
  if (result < 0) result = 0;
  return result;
}

/* ...................................................................... */

/** 
 * Recalculation of first visible line.
 * @param obj then iupcells internal struct object.
 * @return the number of lines.
 */
static int get_first_line(TCells* obj) {
  int i, j;
  int nlines = get_nlines(obj);
  int ncols = get_ncols(obj);

  if (obj->non_scrollable_lins >= nlines) return 1;

  /* Looping the lines until a visible one is found */
  for (i = 1; i <= nlines; i++) { 
     for (j = 1; j <= ncols; j++) { 
        if (get_cell_limit(obj, i, j, NULL, NULL, NULL, NULL)) return i; 
     }
  }
  return IUP_OUT;
}

/* ...................................................................... */

/** 
 * Recalculation of first visible column.
 * @param obj then iupcells internal struct object.
 * @return column number
 */
static int get_first_col(TCells* obj) {
  int i, j;
  int ncols = get_ncols(obj);
  int nlines = get_nlines(obj);
  if (obj->non_scrollable_cols >= ncols) return 1;

  /* Looping the columns until a visible one is found */
  for (j = 1; j <= ncols; j++) { 
     for (i = 1; i <= nlines; i++) { 
        if (get_cell_limit(obj, i, j, NULL, NULL, NULL, NULL)) return j;
     }
  }
  return IUP_OUT;
}

/* ...................................................................... */
/** 
 * Function to inquire application cell data (line height). 
 */
static int get_height(TCells* obj, int i) {
  int size = 30; /* defualt value */

  /* If there is a height callback, the application will answer it */
  { 
     THeightCb func = (THeightCb)IupGetCallback(obj->self, "HEIGHT_CB");
     if (func == NULL) return 30;
     size = (*(func))(obj->self, i);
     if (size < 0) size = 0;
  }
  return size;
}

/* ...................................................................... */
/** 
 * Function to inquire application cell data (column width). 
 */
static int get_width(TCells* obj, int j) {
  int size = 60; /* default value */

  /* If there is a width callback, the application will answer it */
  { 
     TWidthCb func = (TWidthCb)IupGetCallback(obj->self, "WIDTH_CB");
     if (func == NULL) return 60;
     size = (*(func))(obj->self, j);
     if (size < 0) size = 0;
  }
  return size;
}

/* ...................................................................... */

/** Function used to get the cells groups virtual size 
  * @param obj pointer to internal object handle
  * @param wi a reference to virtual width in pixels 
  * @param he a reference to virtual height in pixels 
  * @return references written [wi, he] 
  */
static void get_virtual_size(TCells* obj, int* wi, int* he) {
  int i, j;

  /* Initializing the return values */
  *wi = 0; *he = 0;

  /* Looping through all lines and columns, adding its width and heights
   * to the return values. So, the cells virtual size is computed */
  for (i = 1; i <= get_nlines(obj); i++) *he = *he + get_height(obj, i);
  for (j = 1; j <= get_ncols(obj); j++) *wi = *wi + get_width(obj, j);
}

/* ...................................................................... */

/** 
 * Function used to make a cell the first visible one. 
 */
static int adjust_origin(TCells* obj, char* attr) { 
  char buffer[ 15 ];
  int lin = -9, col = -9;

  /* Scanning the origin */
  iupStrToIntInt(attr, &lin, &col, ':');

  /* If the origin line is a non-scrollable one, the scrollbar position is
   * set to zero. Otherwise, the sum of the previous widths will be
   * set to the scrollbar position. This algorithm is applied to both
   * scrollbars */
  if (lin <= obj->non_scrollable_lins) {
     IupSetAttribute(obj->self, IUP_POSY, "0"); 
  }
  else if (lin <= get_nlines(obj)) { 
     int ymin_sum = get_ranged_height(obj, obj->non_scrollable_lins+1, lin-1);
     sprintf(buffer, "%d", ymin_sum);
     IupStoreAttribute(obj->self, IUP_POSY, buffer); 
  }

  /* As said before... */
  if (col <= obj->non_scrollable_cols) {
     IupSetAttribute(obj->self, IUP_POSX, "0"); 
  }
  else if (col <= get_ncols(obj)) { 
     int xmin_sum = get_ranged_width(obj, obj->non_scrollable_cols+1, col-1);
     sprintf(buffer, "%d", xmin_sum);
     IupStoreAttribute(obj->self, IUP_POSX, buffer); 
  }

  return 1;
}

/* ...................................................................... */
/** 
 * Function used for the scrollbar's update; usually needed when the
 * object has modified its size or the cells sizes has changed.
 */
static void adjust_scrolls(TCells* obj) { 
  int virtual_height, virtual_width;

  /* Getting the current scrollbars' position */
  int posx = IupGetInt(obj->self, IUP_POSX);
  int posy = IupGetInt(obj->self, IUP_POSY);

  /* Getting the virtual size */
  get_virtual_size(obj, &virtual_width, &virtual_height); 

  /* All cells can be drawn inside the object. So, the virtual size
   * is equal to the object size */
  if (virtual_height < obj->height) virtual_height = obj->height;
  if (virtual_width < obj->width) virtual_width = obj->width;

  /* Adjusting the scrollbar position to the new object size */
  if (posx + obj->width > virtual_width) posx = virtual_width - obj->width;
  if (posy + obj->height > virtual_height) posy = virtual_height - obj->height;

  /* Setting the object scrollbar position */
  IupSetfAttribute(obj->self, IUP_YMAX, "%.1f", (float)virtual_height);
  IupSetfAttribute(obj->self, IUP_XMAX, "%.1f", (float)virtual_width);
  IupSetfAttribute(obj->self, IUP_DY,   "%.1f", (float)obj->height);
  IupSetfAttribute(obj->self, IUP_DX,   "%.1f", (float)obj->width);
  IupSetfAttribute(obj->self, IUP_POSY, "%.1f", (float)posy);
  IupSetfAttribute(obj->self, IUP_POSX, "%.1f", (float)posx);
}

/* ...................................................................... */
/** 
 * Function used to calculate a cell limits
 * @param obj pointer to internal object handle
 * @param i cell line
 * @param j cell column
 * @return references written [xmin, xmax, ymin, ymax] 
 */
static int get_cell_limit(TCells* obj, int i, int j,
int* xmin, int* xmax, int* ymin, int* ymax) {
  int result = 1;
  int xmin_sum = 0;
  int ymin_sum = 0;
  int w = obj->width, h = obj->height;
  int _xmin, _xmax, _ymin, _ymax;

  /* Adjusting the inital position according to the cell's type. If it
   * is non-scrollable, the origin is always zero, otherwise the origin 
   * is the scrollbar position */
  int posx = j <= obj->non_scrollable_cols ? 0 : IupGetInt(obj->self, IUP_POSX);
  int posy = i <= obj->non_scrollable_lins ? 0 : IupGetInt(obj->self, IUP_POSY);
  int idx;

  /* Adding to the origin, the cells' width and height */
  for (idx = 1; idx < j; idx++) xmin_sum += get_width(obj, idx);
  for (idx = 1; idx < i; idx++) ymin_sum += get_height(obj, idx);

  /* Finding the cell origin */
  _xmin = xmin_sum - posx;
  _ymax = h - (ymin_sum - posy) - 1;

  /* Computing the cell limit, based on its origin and size */
  _xmax = _xmin + get_width(obj, j);
  _ymin = _ymax - get_height(obj, i);

  /* Checking if the cell is visible */
  if (_xmax < 0 || _xmin > w || _ymin > h || _ymax < 0) result = 0;

  if (xmin != NULL) *xmin = _xmin;
  if (xmax != NULL) *xmax = _xmax;
  if (ymin != NULL) *ymin = _ymin;
  if (ymax != NULL) *ymax = _ymax;

  return result;
}

/* ...................................................................... */
/** 
 * Utility function to convert a cd color to a iup color.
 * @param col CD color.
 * @return IUP color.
 */
static char* cd_to_iup(long int col) {
  static char buffer[32];
  unsigned char r = (unsigned char)((col & 0x00FF0000) >> 16);
  unsigned char g = (unsigned char)((col & 0x0000FF00) >> 8);
  unsigned char b = (unsigned char)((col & 0x000000FF));
  
  /* Dunmping the iup color components */
  sprintf(buffer, "%d %d %d", r, g, b);
  return buffer;
}
