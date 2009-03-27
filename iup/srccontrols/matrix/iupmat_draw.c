/** \file
 * \brief iupmatrix control
 * draw functions
 *
 * See Copyright Notice in "iup.h"
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#undef CD_NO_OLD_INTERFACE

#include "iup.h"
#include "iupcbs.h"

#include <cd.h>

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
#include "iupmat_aux.h"
#include "iupmat_getset.h"
#include "iupmat_mark.h"


/* Color attenuation factor in a marked cell */
#define IMAT_ATENUATION_NO       1.0F
#define IMAT_ATENUATION_FOCUS    0.8F
#define IMAT_ATENUATION_NOFOCUS  1.0F

/* Text alignment that will be draw. Used by iMatrixDrawCellValue */
#define IMAT_T_CENTER  1
#define IMAT_T_LEFT    2
#define IMAT_T_RIGHT   3

/* Used colors to draw the texts. Used by iMatrixDrawCellValue */
#define IMAT_TITLE_COLOR    0    /* Black letters in gray background (for the titles) */
#define IMAT_ELEM_COLOR     1    /* Letters with FGCOLOR and background with BGCOLOR  */
#define IMAT_REVERSE_COLOR  2    /* Letters with BGCOLOR and background with FGCOLOR  */

#define IMAT_CD_INACTIVE_COLOR  0x666666L

#define IMAT_CD_BS  0x666666L  /* Bottom Shadow */
#define IMAT_CD_TS  0xFFFFFFL  /* Top Shadow    */

#define IMAT_COMBOBOX_W 16


typedef int (*IFniiiiiiC)(Ihandle *h, int lin, int col,int x1, int x2, int y1, int y2, cdCanvas* cnv);


/**************************************************************************/
/*  Private functions                                                     */
/**************************************************************************/


static int iMatrixDrawGetColAlignment(Ihandle* ih, int col, char* str)
{
  char* align;
  sprintf(str, "ALIGNMENT%d", col);
  align = iupAttribGet(ih, str);
  if (!align)
    return IMAT_T_LEFT;
  else if (iupStrEqualNoCase(align, "ARIGHT"))
    return IMAT_T_RIGHT;
  else if(iupStrEqualNoCase(align, "ACENTER"))
    return IMAT_T_CENTER;
  else
    return IMAT_T_LEFT;
}

static float iMatrixDrawGetAttenuation(Ihandle* ih)
{
  if(ih->data->mark_mode == IMAT_MARK_NO)
    return IMAT_ATENUATION_NO;
  else
    return IMAT_ATENUATION_FOCUS;
}

static int iMatrixDrawCallDrawCB(Ihandle* ih, int lin, int col, int x1, int x2, int y1, int y2)
{
  IFniiiiiiC cb = (IFniiiiiiC)IupGetCallback(ih, "DRAW_CB");
  if(cb)
  {
    int ret;
    cdCanvas* old_cnv;

    IUPMAT_CLIPAREA(ih, x1, x2, y1, y2);
    cdCanvasClip(ih->data->cddbuffer, CD_CLIPAREA);

    old_cnv = cdActiveCanvas();
    if (old_cnv != ih->data->cddbuffer) /* backward compatibility code */
      cdActivate(ih->data->cddbuffer);

    ret = cb(ih, lin, col, x1, x2, iupMatrixInvertYAxis(ih, y1), iupMatrixInvertYAxis(ih, y2), ih->data->cddbuffer);

    cdCanvasClip(ih->data->cddbuffer, CD_CLIPOFF);

    if (old_cnv && old_cnv != ih->data->cddbuffer) /* backward compatibility code */
    {
      cdActivate(old_cnv);
      cdCanvasActivate(ih->data->cddbuffer);
    }

    if (ret == IUP_DEFAULT)
      return 0;
  }

  return 1;
}

/* Change the CD foreground color, for the selected color to draw a cell with
   its FOREGROUND COLOR. This involves checking if there is a color attribute
   that cell.  If no, uses a color attribute for the line, else if no for the
   column,  else if no  for the entire matrix.   Finally,  if not find any of
   these, use the default color.
   -> lin, col - cell coordinates, in IUP format - i.e., l,l represents the left
                 top cell of the matrix; lin and col values = 0 represents the
                 title lines and columns.
   -> type - cell type. If it is a title cell (type = IMAT_TITLE_COLOR), then
             only find a own color of the cell, and not in advanced modes to
             define colors.
   -> mark - indicate if a cell is marked. If yes, its color is attenuated.
*/
static unsigned long iMatrixDrawSetFgColor(Ihandle* ih, int lin, int col, int mark)
{
  unsigned char r = 0, g = 0, b = 0;
  iupMatrixGetFgRGB(ih, lin, col, &r, &g, &b);

  if (mark)
  {
    float att = iMatrixDrawGetAttenuation(ih);
    r = (unsigned char)(r*att);
    g = (unsigned char)(g*att);
    b = (unsigned char)(b*att);
  }
  
  return cdCanvasForeground(ih->data->cddbuffer, cdEncodeColor(r, g, b));
}

/* Change the CD foreground color, for the selected color to draw a cell with
   its BACKGROUND COLOR. This involves checking if there is a color attribute
   that cell.  If no, uses a color attribute for the line, else if no for the
   column,  else if no  for the entire matrix.   Finally,  if not find any of
   these, use the default color.
   -> lin, col - cell coordinates, in IUP format - i.e., l,l represents the left
                 top cell of the matrix; lin and col values = 0 represents the
                 title lines and columns.
   -> type - cell type. If it is a title cell (type = IMAT_TITLE_COLOR), then
             only find a own color of the cell, and not in advanced modes to
             define colors.
   -> mark - indicate if a cell is marked. If yes, its color is attenuated.
*/
static unsigned long iMatrixDrawSetBgColor(Ihandle* ih, int lin, int col, int mark)
{
  unsigned char r = 255, g = 255, b = 255;

  iupMatrixGetBgRGB(ih, lin, col, &r, &g, &b);
  
  if (mark)
  {
    float att = iMatrixDrawGetAttenuation(ih);
    r = (unsigned char)(r*att);
    g = (unsigned char)(g*att);
    b = (unsigned char)(b*att);
  }

  return cdCanvasForeground(ih->data->cddbuffer, cdEncodeColor(r, g, b));
}

static void iMatrixDrawFrameHorizLineCell(Ihandle* ih, int lin, int col, int x1, int x2, int y, long framecolor, char* str)
{
  if (ih->data->checkframecolor)
  {
    unsigned char r,g,b;
    sprintf(str, "FRAMEHORIZCOLOR%d:%d", lin, col);
    if (iupStrToRGB(iupAttribGet(ih, str), &r, &g, &b))
      cdCanvasForeground(ih->data->cddbuffer, cdEncodeColor(r, g, b));
    else
      cdCanvasForeground(ih->data->cddbuffer, framecolor);
  }
  else
    cdCanvasForeground(ih->data->cddbuffer, framecolor);

  IUPMAT_LINE(ih, x1, y, x2, y);
}

static void iMatrixDrawFrameVertLineCell(Ihandle* ih, int lin, int col, int x, int y1, int y2, long framecolor, char* str)
{
  if (ih->data->checkframecolor)
  {
    unsigned char r,g,b;
    sprintf(str, "FRAMEVERTCOLOR%d:%d", lin, col);
    if (iupStrToRGB(iupAttribGet(ih, str), &r, &g, &b))
      cdCanvasForeground(ih->data->cddbuffer, cdEncodeColor(r, g, b));
    else
      cdCanvasForeground(ih->data->cddbuffer, framecolor);
  }
  else
    cdCanvasForeground(ih->data->cddbuffer, framecolor);

  IUPMAT_LINE(ih, x, y1, x, y2);
}

static void iMatrixDrawFrameRectTitle(Ihandle* ih, int x1, int x2, int y1, int y2, int marked)
{
  /* avoid drawing over the frame of the next title */
  x2--;
  y2--;

  cdCanvasForeground(ih->data->cddbuffer, marked? IMAT_CD_TS: IMAT_CD_BS);
  IUPMAT_LINE(ih, x2, y1, x2, y2);  /* right vertical line */
  IUPMAT_LINE(ih, x1, y2, x2, y2);  /* bottom horizontal line */

  cdCanvasForeground(ih->data->cddbuffer, marked? IMAT_CD_BS: IMAT_CD_TS);
  IUPMAT_LINE(ih, x1, y1, x1, y2-1);  /* left vertical line */
  IUPMAT_LINE(ih, x1, y1, x2-1, y1);  /* top horizontal line */
}

static void iMatrixDrawSortSign(Ihandle* ih, int x2, int y1, int y2, int col, char* str)
{
  int yc;
  char* sort;

  sprintf(str, "SORTSIGN%d", col);
  sort = iupAttribGet(ih, str);
  if (!sort || iupStrEqualNoCase(sort, "NO"))
    return;

  /* Create an space between text and cell margin */
  x2 -= IMAT_DECOR_X/2;

  /* Set the color used to draw the text */
  if(!iupdrvIsActive(ih))
    cdCanvasForeground(ih->data->cddbuffer, IMAT_CD_INACTIVE_COLOR);
  else
    iMatrixDrawSetFgColor(ih, 0, col, 0);

  yc = (int)( (y1 + y2 ) / 2.0 - .5);

  cdCanvasBegin(ih->data->cddbuffer, CD_FILL);

  if (iupStrEqualNoCase(sort, "UP"))
  {
    IUPMAT_VERTEX(ih, x2 - 5, yc + 2);
    IUPMAT_VERTEX(ih, x2 - 1, yc - 2);
    IUPMAT_VERTEX(ih, x2 - 9, yc - 2);
  }
  else
  {
    IUPMAT_VERTEX(ih, x2 - 1, yc + 2);
    IUPMAT_VERTEX(ih, x2 - 9, yc + 2);
    IUPMAT_VERTEX(ih, x2 - 5, yc - 2);
  }

  cdCanvasEnd(ih->data->cddbuffer);
}

static void iMatrixDrawComboFeedback(Ihandle* ih, int x2, int y1, int y2, int lin, int col, int color_type, long framecolor)
{
  int xh2, yh2, x1;

  /* cell background */
  if(color_type == IMAT_ELEM_COLOR)
    iMatrixDrawSetBgColor(ih, lin, col, 0);
  else
    iMatrixDrawSetBgColor(ih, lin, col, 1);
  IUPMAT_BOX(ih, x2 - IMAT_COMBOBOX_W, x2, y1, y2); 

  /* feedback area */
  x2 -= 3;
  x1  = x2 - IMAT_COMBOBOX_W; 
  y1 += 2;
  y2 -= 3;

  /* feedback background */
  iMatrixDrawSetBgColor(ih, 0, 0, 0);
  IUPMAT_BOX(ih, x1, x2, y1, y2);

  /* feedback frame */
  cdCanvasForeground(ih->data->cddbuffer, framecolor);
  IUPMAT_RECT(ih, x1, x2, y1, y2);

  /* feedback arrow */
  xh2 = x2 - IMAT_COMBOBOX_W / 2;
  yh2 = y2 - (y2 - y1) / 2;

  cdCanvasBegin(ih->data->cddbuffer, CD_FILL);
  IUPMAT_VERTEX(ih, xh2, yh2 + 3);
  IUPMAT_VERTEX(ih, xh2 + 4, yh2 - 1);
  IUPMAT_VERTEX(ih, xh2 - 4, yh2 - 1);
  cdCanvasEnd(ih->data->cddbuffer);
}

static void iMatrixDrawBackground(Ihandle* ih, int x1, int x2, int y1, int y2, int color_type, int lin, int col)
{
  /* avoid drawing over the frame of the next title */
  x2--;
  y2--;

  /* avoid drawing over the frame of the cell */
  x2--;
  y2--;

  if (lin==0 || col==0 || (col==1 && ih->data->columns.sizes[0] == 0) || (lin==1 && ih->data->lines.sizes[0] == 0))
  {
    /* avoid drawing over the frame of the cell */
    x1++; 
    y1++; 
  }

  if (color_type == IMAT_TITLE_COLOR)
    iMatrixDrawSetBgColor(ih, lin, col, 0);
  else if(color_type == IMAT_ELEM_COLOR)
    iMatrixDrawSetBgColor(ih, lin, col, 0);
  else
    iMatrixDrawSetBgColor(ih, lin, col, 1);

  IUPMAT_BOX(ih, x1, x2, y1, y2);
}

/* Put the cell contents in the screen, using the specified color and alignment.
   -> y1, y2 : vertical limits of the cell
   -> x1, x2 : horizontal limits of the complete cell
   -> alignment : alignment type (horizontal) assigned to the text. The options are:
                  [IMAT_T_CENTER,IMAT_T_LEFT,IMAT_T_RIGHT]
   -> color_type : color schema that will be used:
           IMAT_TITLE_COLOR  ->  Black letters in gray background (for the titles)
           IMAT_ELEM_COLOR    -> Letters with FGCOLOR and background with BGCOLOR
           IMAT_REVERSE_COLOR -> Letters with BGCOLOR and background with FGCOLOR
   -> lin, col - cell coordinates */
static void iMatrixDrawCellValue(Ihandle* ih, int x1, int x2, int y1, int y2, int alignment, int color_type, int lin, int col)
{
  char *text;

  /* avoid drawing over the frame of the next title */
  x2--;
  y2--;

  /* avoid drawing over the frame of the cell */
  x2--;
  y2--;

  if (color_type == IMAT_TITLE_COLOR)
  {
    /* avoid drawing over the frame of the cell */
    x1++; 
    y1++; 
  }

  if (!iMatrixDrawCallDrawCB(ih, lin, col, x1, x2, y1, y2))
    return;

  text = iupMatrixCellGetValue(ih, lin, col);

  /* Put the text */
  if (text && *text)
  {
    int num_line, line_height, total_height;
    int charheight, ypos;

    num_line = iupStrLineCount(text);
    iupdrvFontGetCharSize(ih, NULL, &charheight);

    line_height  = charheight;
    total_height = (line_height + IMAT_DECOR_Y/2) * num_line - IMAT_DECOR_Y/2;

    if (lin==0)
    {
      int text_w;
      iupdrvFontGetMultiLineStringSize(ih, text, &text_w, NULL);
      if (text_w > x2 - x1 + 1 - IMAT_DECOR_X)
        alignment = IMAT_T_LEFT;
    }

    /* Set the color used to draw the text */
    if (!iupdrvIsActive(ih))
      cdCanvasForeground(ih->data->cddbuffer, IMAT_CD_INACTIVE_COLOR);
    else if(color_type == IMAT_ELEM_COLOR)
      iMatrixDrawSetFgColor(ih, lin, col, 0);
    else if(color_type == IMAT_REVERSE_COLOR)
      iMatrixDrawSetFgColor(ih, lin, col, 1);
    else
      iMatrixDrawSetFgColor(ih, lin, col, 0);

    /* Set the clip area to the cell region informed, the text maybe greatter than the cell */
    IUPMAT_CLIPAREA(ih, x1, x2, y1, y2);
    cdCanvasClip(ih->data->cddbuffer, CD_CLIPAREA);

    cdCanvasNativeFont(ih->data->cddbuffer, iupMatrixGetFont(ih, lin, col));

    /* Create an space between text and cell margin */
    x1 += IMAT_DECOR_X/2;       x2 -= IMAT_DECOR_X/2;
    y1 += IMAT_DECOR_Y/2;       y2 -= IMAT_DECOR_Y/2;

    if (num_line == 1)
    {
      ypos = (int)((y1 + y2) / 2.0 - 0.5);

      /* Put the text */
      if (alignment == IMAT_T_CENTER)
        IUPMAT_TEXT(ih, (x1 + x2) / 2, ypos, text, CD_CENTER)
      else if(alignment == IMAT_T_LEFT)
        IUPMAT_TEXT(ih, x1, ypos, text, CD_WEST)
      else
        IUPMAT_TEXT(ih, x2, ypos, text, CD_EAST)
    }
    else
    {
      int i;
      char *p, *q, *newtext;

      newtext = iupStrDup(text);
      p = newtext;

      /* Get the position of the first text to be put in the screen */
      ypos = (int)( (y1 + y2) / 2.0 - 0.5) - total_height/2 + line_height/2;

      for(i = 0; i < num_line; i++)
      {
        q = strchr(p, '\n');
        if (q) *q = 0;  /* Cut the string to contain only one line */

        /* Put the text */
        if(alignment == IMAT_T_CENTER)
          IUPMAT_TEXT(ih, (x1 + x2) / 2, ypos, p, CD_CENTER)
        else if(alignment == IMAT_T_LEFT)
          IUPMAT_TEXT(ih, x1, ypos, p, CD_WEST)
        else
          IUPMAT_TEXT(ih, x2, ypos, p, CD_EAST)

        /* Advance the string */
        p = q + 1;

        /* Advance a line */
        ypos += line_height + IMAT_DECOR_Y/2;
      }

      free(newtext);
    }

    cdCanvasClip(ih->data->cddbuffer, CD_CLIPOFF);
  }
}

static void iMatrixDrawTitleCorner(Ihandle* ih)
{
  iMatrixDrawFrameRectTitle(ih, 0, ih->data->columns.sizes[0], 0, ih->data->lines.sizes[0], 0);

  iMatrixDrawBackground(ih, 0, ih->data->columns.sizes[0], 0, ih->data->lines.sizes[0], IMAT_TITLE_COLOR, 0, 0);

  iMatrixDrawCellValue(ih, 0, ih->data->columns.sizes[0], 0, ih->data->lines.sizes[0], IMAT_T_CENTER, IMAT_TITLE_COLOR, 0, 0);
}

/* Draw the line titles, visible, between lin and lastlin, include it. 
   Line titles marked will be draw with the appropriate feedback.
   -> start_lin - First line to have its title drawn
   -> end_lin - Last line to have its title drawn */
static void iMatrixDrawLineTitle(Ihandle* ih, int start_lin, int end_lin)
{
  int x1, y1, x2, y2;
  int lin, alignment;
  char str[100];
  long framecolor;

  /* Start the position of the line title */
  x1 = 0;
  x2 = ih->data->columns.sizes[0];

  y1 = ih->data->lines.sizes[0];
  for(lin = ih->data->lines.first; lin < start_lin; lin++)
    y1 += ih->data->lines.sizes[lin];

  framecolor = cdIupConvertColor(iupAttribGetStr(ih, "FRAMECOLOR"));

  alignment = iMatrixDrawGetColAlignment(ih, 0, str);

  /* Draw the titles */
  for(lin = start_lin; lin <= end_lin; lin++)
  {
    /* If it is a hidden line (size = 0), don't draw the title */
    if(ih->data->lines.sizes[lin] == 0)
      continue;

    y2 = y1 + ih->data->lines.sizes[lin];

    /* If it doesn't have title, the loop just calculate the final position */
    if (ih->data->columns.sizes[0])
    {
      iMatrixDrawFrameRectTitle(ih, x1, x2, y1, y2, iupMatrixLineIsMarked(ih, lin));

      iMatrixDrawBackground(ih, x1, x2, y1, y2, IMAT_TITLE_COLOR, lin, 0);

      iMatrixDrawCellValue(ih, x1, x2, y1, y2, alignment, IMAT_TITLE_COLOR, lin, 0);
    }

    y1 = y2;
  }
}

/* Draw the column titles, visible, between col and lastcol, include it. 
   Column titles marked will be draw with the appropriate feedback.
   -> start_col - First column to have its title drawn
   -> end_col - Last column to have its title drawn */
static void iMatrixDrawColumnTitle(Ihandle* ih, int start_col, int end_col)
{
  int x1, y1, x2, y2;
  int col;
  char str[100];
  long framecolor;

  /* Start the position of the first column title */
  y1 = 0;
  y2 = ih->data->lines.sizes[0];

  x1 = ih->data->columns.sizes[0];
  for(col = ih->data->columns.first; col < start_col; col++)
    x1 += ih->data->columns.sizes[col];

  framecolor = cdIupConvertColor(iupAttribGetStr(ih, "FRAMECOLOR"));

  /* Draw the titles */
  for(col = start_col; col <= end_col; col++)
  {
    /* If it is an hide column (size = 0), no draw the title */
    if(ih->data->columns.sizes[col] == 0)
      continue;

    x2 = x1 + ih->data->columns.sizes[col];

    /* If it doesn't have title, the loop just calculate the final position */
    if (ih->data->lines.sizes[0])
    {
      iMatrixDrawFrameRectTitle(ih, x1, x2, y1, y2, iupMatrixColumnIsMarked(ih, col));

      iMatrixDrawBackground(ih, x1, x2, y1, y2, IMAT_TITLE_COLOR, 0, col);

      iMatrixDrawCellValue(ih, x1, x2, y1, y2, IMAT_T_CENTER, IMAT_TITLE_COLOR, 0, col);

      iMatrixDrawSortSign(ih, x2, y1, y2, col, str);
    }

    x1 = x2;
  }
}

/* Redraw a block of cells of the matrix. Handle marked cells, change
   automatically the background color of them.
   - lin1, col1 : cell coordinates that mark the left top corner of the area to be redrawn
   - lin2, col2 : cell coordinates that mark the right bottom corner of the area to be redrawn */
static void iMatrixDrawCells(Ihandle* ih, int lin1, int col1, int lin2, int col2)
{
  int x1, y1, x2, y2, old_x2, old_y1, old_y2;
  int alignment, lin, col;
  long framecolor, emptyarea_color = -1;
  char str[100];
  IFnii mark_cb;
  IFnii dropcheck_cb;

  x1 = 0;
  x2 = ih->data->w-1;
  y1 = 0;
  y2 = ih->data->h-1;

  old_x2 = x2;
  old_y1 = y1;
  old_y2 = y2;

  /* Find the initial position of the first column */
  x1 += ih->data->columns.sizes[0];
  for(col = ih->data->columns.first; col < col1; col++)
    x1 += ih->data->columns.sizes[col];

  /* Find the final position of the last column */
  x2 = x1;
  for( ; col <= col2; col++)
    x2 += ih->data->columns.sizes[col];

  /* Find the initial position of the first line */
  y1 += ih->data->lines.sizes[0];
  for(lin = ih->data->lines.first; lin < lin1; lin++)
    y1 += ih->data->lines.sizes[lin];

  /* Find the final position of the last line */
  y2 = y1;
  for( ; lin <= lin2; lin++)
    y2 += ih->data->lines.sizes[lin];

  if ((col2 == ih->data->columns.num-1) && (old_x2 > x2))
  {
    emptyarea_color = cdIupConvertColor(iupControlBaseGetParentBgColor(ih));
    cdCanvasForeground(ih->data->cddbuffer, emptyarea_color);

    /* If it was drawn until the last column and remains space in the right of it,
       then delete this area with the the background color. */
    IUPMAT_BOX(ih, x2, old_x2, old_y1, old_y2);
  }

  if ((lin2 == ih->data->lines.num-1) && (old_y2 > y2))
  {
    if (emptyarea_color == -1)
      emptyarea_color = cdIupConvertColor(iupControlBaseGetParentBgColor(ih));
    cdCanvasForeground(ih->data->cddbuffer, emptyarea_color);

    /* If it was drawn until the last line visible and remains space below it,
       then delete this area with the the background color. */
    IUPMAT_BOX(ih, 0, old_x2, y2, old_y2);
  }

  /***** Draw the cell values and frame */
  old_y1 = y1;
  framecolor = cdIupConvertColor(iupAttribGetStr(ih, "FRAMECOLOR"));

  mark_cb = (IFnii)IupGetCallback(ih, "MARK_CB");
  dropcheck_cb = (IFnii)IupGetCallback(ih, "DROPCHECK_CB");

  for(col = col1; col <= col2; col++)  /* For all the columns in the region */
  {
    if (ih->data->columns.sizes[col] == 0)
      continue;

    alignment = iMatrixDrawGetColAlignment(ih, col, str);

    x2 = x1 + ih->data->columns.sizes[col];

    for(lin = lin1; lin <= lin2; lin++)     /* For all lines in the region */
    {
      if (ih->data->lines.sizes[lin] == 0)
        continue;

      y2 = y1 + ih->data->lines.sizes[lin];

      if (col==1 && ih->data->columns.sizes[0] == 0)
      {
        /* If does not have titles then draw the left line of the cell frame */
        iMatrixDrawFrameVertLineCell(ih, lin, col, x1, y1, y2-1-1, framecolor, str);
      }

      if (lin==1 && ih->data->lines.sizes[0] == 0)
      {
        /* If does not have titles then draw the top line of the cell frame */
        iMatrixDrawFrameHorizLineCell(ih, lin, col, x1, x2-1-1, y1, framecolor, str);
      }

      /* draw the cell contents */
      {
        int drop = 0;
        int color_type  = IMAT_ELEM_COLOR;

        if (dropcheck_cb && dropcheck_cb(ih, lin, col) == IUP_DEFAULT)
          drop = IMAT_COMBOBOX_W;

        /* If the cell is marked, then draw it in reverse color */
        if (iupMatrixMarkCellGet(ih, lin, col, mark_cb, str))
          color_type = IMAT_REVERSE_COLOR;

        iMatrixDrawBackground(ih, x1, x2, y1, y2, color_type, lin, col);
        
        iMatrixDrawCellValue(ih, x1, x2-drop, y1, y2, alignment, color_type, lin, col);

        if (drop)
          iMatrixDrawComboFeedback(ih, x2, y1, y2, lin, col, color_type, framecolor);
      }

      /* bottom line */
      iMatrixDrawFrameHorizLineCell(ih, lin, col, x1, x2-1, y2-1, framecolor, str);
      
      /* rigth line */
      iMatrixDrawFrameVertLineCell(ih, lin, col, x2-1, y1, y2-1, framecolor, str);

      y1 = y2;
    }

    x1 = x2;
    y1 = old_y1;  /* must reset also y */
  }
}

static void iMatrixDrawMatrix(Ihandle* ih)
{
  iupMatrixStoreGlobalAttrib(ih);

  /* fill the background because there will be empty cells */
  if ((ih->data->lines.num == 1) || (ih->data->columns.num == 1))
  {
    cdCanvasForeground(ih->data->cddbuffer, cdIupConvertColor(iupControlBaseGetParentBgColor(ih)));
    cdCanvasClear(ih->data->cddbuffer);
  }

  /* Draw the corner between line and column titles, if necessary */
  if (ih->data->lines.sizes[0] && ih->data->columns.sizes[0])
    iMatrixDrawTitleCorner(ih);

  /* If there are columns, then draw their titles */
  if (ih->data->lines.sizes[0])
    iMatrixDrawColumnTitle(ih, ih->data->columns.first, ih->data->columns.last);

  /* If there are lines, then draw their titles */
  if (ih->data->columns.sizes[0])
    iMatrixDrawLineTitle(ih, ih->data->lines.first, ih->data->lines.last);

  /* If there are cells in the matrix, then draw them */
  if ((ih->data->lines.num > 1) && (ih->data->columns.num > 1))
    iMatrixDrawCells(ih, ih->data->lines.first, ih->data->columns.first, 
                         ih->data->lines.last, ih->data->columns.last);
}

static void iMatrixDrawFocus(Ihandle* ih)
{
  int x1, y1, x2, y2, dx, dy;

  if (iupAttribGetInt(ih, "HIDEFOCUS"))
    return;

  if (!iupMatrixAuxIsCellVisible(ih, ih->data->lines.focus_cell, ih->data->columns.focus_cell))
    return;

  iupMatrixAuxGetVisibleCellDim(ih, ih->data->lines.focus_cell, ih->data->columns.focus_cell, &x1, &y1, &dx, &dy);

  x2 = x1 + dx - 1;
  y2 = y1 + dy - 1;

  cdIupDrawFocusRect(ih, ih->data->cdcanvas, x1, iupMatrixInvertYAxis(ih, y1), x2, iupMatrixInvertYAxis(ih, y2));
}


/**************************************************************************/
/* Exported functions                                                     */
/**************************************************************************/


void iupMatrixDrawCell(Ihandle* ih, int lin, int col)
{
  iMatrixDrawCells(ih, lin, col, lin, col);
}

void iupMatrixDraw(Ihandle* ih, int update)
{
  if (ih->data->need_calcsize)
    iupMatrixAuxCalcSizes(ih);

  iMatrixDrawMatrix(ih);

  if (update)
    iupMatrixDrawUpdate(ih);
}

void iupMatrixDrawUpdate(Ihandle* ih)
{
  cdCanvasFlush(ih->data->cddbuffer);

  if (ih->data->has_focus)
    iMatrixDrawFocus(ih);
}

int iupMatrixDrawSetRedrawAttrib(Ihandle* ih, const char* value)
{
  int type;

  if (value == NULL)
    type = 0;
  else if(value[0] == 'L' || value[0] == 'l')
    type = IMAT_PROCESS_LIN;
  else if(value[0] == 'C' || value[0] == 'c')
    type = IMAT_PROCESS_COL;
  else
    type = 0;

  if (type)
  {
    int min = 0, max = 0;
    value++;

    if(iupStrToIntInt(value, &min, &max, ':') != 2)
      max = min;

    iupMatrixStoreGlobalAttrib(ih);

    if (ih->data->need_calcsize)
      iupMatrixAuxCalcSizes(ih);

    if (ih->data->lines.sizes[0] && ih->data->columns.sizes[0])
      iMatrixDrawTitleCorner(ih);

    if (type == IMAT_PROCESS_LIN)
    {
      if(min < ih->data->lines.first)
        min = ih->data->lines.first;
      if(max > ih->data->lines.last)
        max = ih->data->lines.last;

    if (min > max)
      return 0;

      if (ih->data->columns.sizes[0])
        iMatrixDrawLineTitle(ih, min, max);

      iMatrixDrawCells(ih, min, ih->data->columns.first, max, ih->data->columns.last);
    }
    else
    {
      if(min < ih->data->columns.first)
        min = ih->data->columns.first;
      if(max > ih->data->columns.last)
        max = ih->data->columns.last;

      if(min > max)
        return 0;

      if (ih->data->lines.sizes[0])
        iMatrixDrawColumnTitle(ih, min, max);

      iMatrixDrawCells(ih, ih->data->lines.first, min, ih->data->lines.last, max);
    }
  }
  else
  {
    iupMatrixAuxCalcSizes(ih);
    iMatrixDrawMatrix(ih);
  }

  iupMatrixDrawUpdate(ih);
  return 0;
}
