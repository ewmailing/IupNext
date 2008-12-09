/** \file
 * \brief iupmatrix column resize
 *
 * See Copyright Notice in "iup.h"
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "iup.h"
#include "iupcbs.h"
#include "iupmatrix.h"
#include "iupkey.h"

#include <cd.h>
#include <cdiup.h>
#include <cddbuf.h>

#include "iup_object.h"
#include "iup_attrib.h"
#include "iup_str.h"
#include "iup_drv.h"
#include "iup_drvfont.h"
#include "iup_stdcontrols.h"
#include "iup_controls.h"
#include "iup_cdutil.h"

#include "iupmat_def.h"
#include "iupmat_cd.h"
#include "iupmat_draw.h"
#include "iupmat_scroll.h"
#include "iupmat_aux.h"
#include "iupmat_mark.h"
#include "iupmat_focus.h"


#define IMAT_TOL       3

static int  Dragging=0,      /* this flag indicates if it is being made a resize  */
            DragCol,         /* column in changed - value is zero-based, 0 if it
                                is being made resize in the first column of data; 
                                -1 if it is being made resize in the title column */
            DragColStartPos; /* position, in pixels, of the start of the column
                                in changed                                        */

static int  Lastxpos;  /* previous position of the feedback line in the screen */

static void iMatrixColResChangeMatrixWH(Ihandle* ih, int col, int largura, int m);

#define IMAT_RESIZE_COLOR  0x666666L

/**************************************************************************/
/* Interactive Column Resize Functions                                    */
/**************************************************************************/

/* Verify if the mouse is in the intersection between two of column titles,
   the resize is started (x,y : mouse coordinates (canvas coordinates))
*/
int iupMatrixColResTry(Ihandle* ih, int x, int y)
{
  int found, width, col;
  int ativo = IupGetInt(ih, "RESIZEMATRIX");

  if(ih->data->lin.titlewh && y < ih->data->lin.titlewh && ativo)
  {
    /* It is in the area of the column titles */
    found = 0;
    width = ih->data->col.titlewh;

    if(abs(width-x) < IMAT_TOL)
    {
      /* interface between the column titles and the first column... */
      col = 0;
      found = 1;
      DragColStartPos = 0;
    }
    else
    {
      /* find to identify the interface */
      DragColStartPos = width;
      for(col = ih->data->col.first; col <= ih->data->col.last && !found; col++)
      {
        width += ih->data->col.wh[col];
        if(abs(width-x) < IMAT_TOL)
          found = 1;
        if(!found)
          DragColStartPos= width;
      }
    }

    if(found)
    {
      Dragging =  1;
      Lastxpos = -1;
      DragCol  = col-1;

      return 1;
    }
  }
  return 0;
}

/* Finish the interactive resize of columns. Call ChangeMatrixWidth to truly change
   the column size (x : x mouse coordinate (canvas coordinate)).
*/
void iupMatrixColResFinish(Ihandle* ih, int x)
{
  int charwidth, charheight, width;
  int y1, y2;

  iupdrvFontGetCharSize(ih, &charwidth, &charheight);
  width = x - DragColStartPos - IMAT_DECOR_X;

  if(width < charwidth)
    width = charwidth ;  /* min size to the cell */

  /* delete feedback */
  if(Lastxpos != -1)
  {
    y1 = ih->data->lin.titlewh;
    y2 = ih->data->YmaxC;

    cdCanvasWriteMode(ih->data->cdcanvas, CD_XOR);
    cdCanvasForeground(ih->data->cdcanvas, IMAT_RESIZE_COLOR);
    cdCanvasLine(ih->data->cdcanvas, Lastxpos, (ih->data->YmaxC - (y1)), Lastxpos, (ih->data->YmaxC - (y2)));
    cdCanvasWriteMode(ih->data->cdcanvas, CD_REPLACE);
  }

  iMatrixColResChangeMatrixWH(ih, DragCol, width + IMAT_DECOR_X, IMAT_MAT_COL);
  Dragging = 0;
}

/* Change the column width interactively, just change the line in the screen.
   When the user finishes the drag, the iupMatrixColResFinish function is called
   to truly change the column width (x : x mouse coordinate (canvas coordinate)).
*/
void iupMatrixColResMove(Ihandle* ih, int x)
{
  int y1, y2, charwidth, charheight;

  iupdrvFontGetCharSize(ih, &charwidth, &charheight);

  /* If the size column was tiny, no change the size column */
  if(x < DragColStartPos + charwidth + IMAT_DECOR_X)
    return;

  y1 = ih->data->lin.titlewh;
  y2 = ih->data->YmaxC;

  cdCanvasWriteMode(ih->data->cdcanvas, CD_XOR);
  cdCanvasForeground(ih->data->cdcanvas, IMAT_RESIZE_COLOR);

  /* If it is not the first time, move old line */
  if(Lastxpos != -1)
    cdCanvasLine(ih->data->cdcanvas, Lastxpos, (ih->data->YmaxC - (y1)), Lastxpos, (ih->data->YmaxC - (y2)));

  cdCanvasLine(ih->data->cdcanvas, x, (ih->data->YmaxC - (y1)), x, (ih->data->YmaxC - (y2)));

  Lastxpos = x;
  cdCanvasWriteMode(ih->data->cdcanvas, CD_REPLACE);
}


static void iMatrixColResResetMatrixCursor(Ihandle* ih)
{
  char *cursor = iupAttribGetStr(ih, "_IUPMAT_CURSOR");
  if (cursor)
    IupSetAttribute(ih, "CURSOR", "IupMatrixCrossCursor");
  else
    IupStoreAttribute(ih, "CURSOR", cursor);
}

/* Change the cursor when it passes over a group of the column titles.
   -> x,y : mouse coordinates (canvas coordinates)
*/
void iupMatrixColResChangeCursor(Ihandle* ih, int x, int y)
{
  int ativo = IupGetInt(ih, "RESIZEMATRIX");

  if(ih->data->lin.titlewh && y < ih->data->lin.titlewh && ativo)
  {
    /* It is in the column titles area and the resize mode is on */
    int found = 0, width = ih->data->col.titlewh, col;

    if(abs(width - x) < IMAT_TOL)
      found = 1;    /* line titles */
    else
    {
      for(col = ih->data->col.first; col <= ih->data->col.last && !found; col++)
      {
        width += ih->data->col.wh[col];
        if(abs(width - x) < IMAT_TOL)
          found = 1;
      }
    }

    if (found)
    {
      iupAttribStoreStr(ih, "_IUPMAT_CURSOR", iupAttribGetStr(ih, "CURSOR"));
      IupSetAttribute(ih, "CURSOR", "RESIZE_W");
    }
    else /* It is in the empty area after the last column */
      iMatrixColResResetMatrixCursor(ih); 
  }
  else
    iMatrixColResResetMatrixCursor(ih);
}

/* Return the resize flag */
int iupMatrixColResResizing(void)
{
  return Dragging;
}


/**************************************************************************/
/* Set and get functions - width of a column/ line.                       */
/**************************************************************************/

/* Change the width of a column or the height of a line, repaint if visible
   -> col : column/line width to be modified. This function works with a
         internal representation to the number of column, i.e., col = 0 and
         the first column, col = -1 and the column of titles. The same
         representation is used to lines.
   -> largura: width, in pixels, of the column/line col.
   -> m : indicate if it is acting on a column (IMAT_MAT_COL) or line (IMAT_MAT_LIN).
*/
static void iMatrixColResChangeMatrixWH(Ihandle* ih, int col, int largura, int m)
{
 int visible = IupGetInt(ih, "VISIBLE");
 int err, drawmode = m;
 ImatLinColData *p;

 if(m == IMAT_MAT_LIN)
   p = &(ih->data->lin);
 else
   p = &(ih->data->col);

 IsCanvasSet(ih, err);

 if(col != -1)  /* col -1 indicates the title width */
 {
   if(largura > p->size)   /* width can't be greater than the useful area */
     largura = p->size;

   /* Recalculate the total space occupied by columns/ lines */
   p->totalwh = p->totalwh - p->wh[col] + largura;
   p->wh[col] = largura;
 }
 else if(p->titlewh > 0)      /* Only change the size if there is already title */
 {
   if(largura > ih->data->XmaxC)
     largura = ih->data->XmaxC;
   /* Recalculate the blank space to the other cells */
   p->size    = p->size + p->titlewh - largura;
   p->titlewh = largura;

   drawmode = IMAT_DRAW_ALL; /* Line/column titles also must be repaint */
 }
 iupMatrixAuxGetLastWidth(ih, m);

 /* If the column/line is visible, repaint the matrix */
 if(((col >= p->first) && (col <= p->last)) || col==-1)
  if(visible && err == CD_OK)
  {
    iupMatrixSetSb(ih, m);
    iupMatrixDrawMatrix(ih, drawmode);
    iupMatrixFocusShowFocus(ih);
  }
}

/* Change the width/height of a column/line. Call when the WIDTHx or HEIGHTx
   attributes are changed.
   Get the nes size of column/line, pass of the IUP unit to pixels and call the
   iMatrixColResChangeMatrixWH function
   -> col : column/ line that will take its width changed. col = 1 is the first column/line.
            col = 0 is the column/line of titles.
   -> m : indicate if it is acting on a column (IMAT_MAT_COL) or line (IMAT_MAT_LIN).
*/
void iupMatrixColResSet(Ihandle* ih, const char* value, int col, int m, int pixels)
{
 int largura = 0;

 if(iupStrToInt(value, &largura))
 {
   col--;  /* the left top cell is 1:1 to the user, internally is 0:0 */

   if(largura)
   {
     if(pixels)
     {
       if(m == IMAT_MAT_COL)
         largura = (int)(largura + IMAT_DECOR_X);
       else
         largura = (int)(largura + IMAT_DECOR_Y);
     }
     else
     {
       int charwidth, charheight;
       iupdrvFontGetCharSize(ih, &charwidth, &charheight);
       /* Transform in pixels */
       if(m == IMAT_MAT_COL)
         largura = (int)((largura / 4.) * charwidth ) + IMAT_DECOR_X;
       else 
         largura = (int)((largura / 8.) * charheight) + IMAT_DECOR_Y;
     }
   }
   iMatrixColResChangeMatrixWH(ih, col, largura, m);
 }   
}

/* Get the width/height of a column/line. The WIDTH and HEIGHT attributes can't be
   collected because when the width/height of the column/line are changed dinamically,
   the value of the attribute was out-of-date. Moreover, not all columns/lines has
   its WIDTH and HEIGHT attributes defined (widht and height default)
   -> col : column/ line that will take its width collected. col = 1 is the first column/line.
            col = 0 is the column/line of titles.
   -> m : indicate if it is acting on a column (IMAT_MAT_COL) or line (IMAT_MAT_LIN).
*/
char *iupMatrixColResGet(Ihandle* ih, int col, int m, int pixels)
{
  char* width = iupStrGetMemory(100);
  int w;
  ImatLinColData *p;

  if(m == IMAT_MAT_LIN)
    p = &(ih->data->lin);
  else
    p = &(ih->data->col);

  if(col > p->num || col < 0)
    return NULL;

  if(col == 0)
    w = p->titlewh;
  else
  {
    col--;  /* the left top cell is 1:1 to the user, internally is 0:0 */ 
    w = p->wh[col];
  }

  if(w)
  {
    if(pixels)
    {
      if(m == IMAT_MAT_COL)
        sprintf(width, "%d", (int)(w - IMAT_DECOR_X));
      else
        sprintf(width, "%d", (int)(w - IMAT_DECOR_Y));
    }
    else
    {
      int charwidth, charheight;
      iupdrvFontGetCharSize(ih, &charwidth, &charheight);
      if(m == IMAT_MAT_COL)
        sprintf(width, "%d", (int)((w - IMAT_DECOR_X) * 4 / charwidth));
      else
        sprintf(width, "%d", (int)((w - IMAT_DECOR_Y) * 8 / charheight));
    }
  }
  else
    strcpy(width, "0");

  return width;
}
