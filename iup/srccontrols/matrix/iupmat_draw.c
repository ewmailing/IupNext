/** \file
 * \brief iupmatrix control
 * draw functions
 *
 * See Copyright Notice in iup.h
 * $Id: iupmat_draw.c,v 1.2 2008-11-28 00:19:04 scuri Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#undef CD_NO_OLD_INTERFACE

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
#include "iup_stdcontrols.h"
#include "iup_controls.h"
#include "iup_cdutil.h"

#include "iupmat_def.h"
#include "iupmat_cd.h"
#include "iupmat_draw.h"
#include "iupmat_aux.h"
#include "iupmat_mark.h"

/* Color attenuation factor in a marked cell */
#define IMATRIX_ATENUATION_NO       1.0F
#define IMATRIX_ATENUATION_FOCUS    0.8F
#define IMATRIX_ATENUATION_NOFOCUS  1.0F

/* Text alignment that will be draw. Used by iMatrixDrawText */
#define IMATRIX_T_CENTER  1
#define IMATRIX_T_LEFT    2
#define IMATRIX_T_RIGHT   3

/* Used colors to draw the texts. Used by iMatrixDrawText */
#define IMATRIX_TITLE_COLOR    0    /* Black letters in gray background (for the titles) */
#define IMATRIX_ELEM_COLOR     1    /* Letters with FGCOLOR and background with BGCOLOR  */
#define IMATRIX_REVERSE_COLOR  2    /* Letters with BGCOLOR and background with FGCOLOR  */

#define IMATRIX_CD_INACTIVE_COLOR  0x666666L

#define IMATRIX_CD_BS  0x666666L  /* Bottom Shadow */
#define IMATRIX_CD_TS  0xFFFFFFL  /* Top Shadow    */

#define IMATRIX_BOXW 16


typedef int (*IFniiiiiiC)(Ihandle *h, int lin, int col,int x1, int x2, int y1, int y2, cdCanvas* cnv);


/**************************************************************************/
/*  Private functions to the processing of colors                         */
/**************************************************************************/

void iMatrixSetCdFrameColor(Ihandle* ih)
{
  unsigned char r, g, b;
  iupStrToRGB(iupAttribGetStrDefault(ih, "FRAMECOLOR"), &r, &g, &b);
  cdCanvasForeground(ih->data->cddbuffer, cdEncodeColor(r, g, b));
}

static float iMatrixGetAttenuation(Ihandle* ih)
{
  char* mark = iupAttribGetStr(ih, "MARK_MODE");
  if(mark == NULL || iupStrEqualNoCase(mark, "NO"))
    return IMATRIX_ATENUATION_NO;
  else
    return IMATRIX_ATENUATION_FOCUS;
}

static int iMatrixCallFgColorCB(Ihandle* ih, int lin, int col, unsigned int* r, unsigned int* g, unsigned int* b)
{
  IFniiIII cb = (IFniiIII)IupGetCallback(ih, "FGCOLOR_CB");
  if(cb)
    return cb(ih, lin, col, (int*)r, (int*)g, (int*)b);
  else
    return IUP_IGNORE;
}

static int iMatrixCallBgColorCB(Ihandle* ih, int lin, int col, unsigned int *r, unsigned int *g, unsigned int *b)
{
  IFniiIII cb = (IFniiIII)IupGetCallback(ih, "BGCOLOR_CB");
  if(cb)
    return cb(ih, lin, col, (int*)r, (int*)g, (int*)b);
  else
    return IUP_IGNORE;
}

static int iMatrixCallDrawCB(Ihandle* ih, int cor, int lin, int col, int x1, int x2, int y1, int y2)
{
  IFniiiiiiC cb = (IFniiiiiiC)IupGetCallback(ih, "DRAW_CB");
  if(cb)
  {
    int ret;

    cdCanvas* old_cnv = cdActiveCanvas();
    if(cor == IMATRIX_TITLE_COLOR)
    {
      x1+=1;
      x2-=1;
      y1+=1;
      y2-=1;
    }
    else if(!iupAttribGetInt(ih, "HIDEFOCUS"))
    {
      x1+=2;
      x2-=2;
      y1+=2;
      y2-=2;
    }

    CdClipArea(x1, x2, y1, y2);
    cdCanvasClip(ih->data->cddbuffer, CD_CLIPAREA);

    if(old_cnv != ih->data->cddbuffer) /* backward compatibility code */
      cdActivate(ih->data->cddbuffer);

    ret = cb(ih, lin, col, x1, x2, (ih->data->YmaxC - (y1)), INVY(y2), ih->data->cddbuffer);

    cdCanvasClip(ih->data->cddbuffer, CD_CLIPOFF);
    if(old_cnv && old_cnv != ih->data->cddbuffer)
    {
      cdActivate(old_cnv);
      cdCanvasActivate(ih->data->cddbuffer);
    }

    if (ret == IUP_DEFAULT)
      return 0;
  }

  return 1;
}

static void iMatrixDrawGetColor(Ihandle* ih, char* attrib, int lin, int col, unsigned int *r, unsigned int *g, unsigned int *b, int parent)
{
  char* value;
  char* colorattr = iupStrGetMemory(30);

  /* 1 -  check for this cell */
  sprintf(colorattr, "%s%d:%d", attrib, lin, col);
  value = iupAttribGetStr(ih, colorattr);
  if(!value)
  {
    /* 2 - check for this line, if not title col */
    if(col != 0)
    {
      sprintf(colorattr, "%s%d:*", attrib, lin);
      value = iupAttribGetStr(ih, colorattr);
    }

    if(!value)
    {
      /* 3 - check for this column, if not title line */
      if(lin != 0)
      {
        sprintf(colorattr,"%s*:%d",attrib,col);
        value = iupAttribGetStr(ih, colorattr);
      }

      if(!value)
      {
        /* 4 - check for the matrix or parent */
        if(parent)
          value = iupControlBaseGetParentBgColor(ih);
        else
          value = iupAttribGetStr(ih, attrib);
      }
    }
  }

  if(value)
    iupStrToRGB(value, (unsigned char*)r, (unsigned char*)g, (unsigned char*)b);
}

/* Change the CD foreground color, for the selected color to draw a cell with
   its FOREGROUND COLOR. This involves checking if there is a color attribute
   that cell.  If no, uses a color attribute for the line, else if no for the
   column,  else if no  for the entire matrix.   Finally,  if not find any of
   these, use the default color.
   -> lin, col - cell coordinates, in IUP format - i.e., l,l represents the left
                 top cell of the matrix; lin and col values = 0 represents the
                 title lines and columns.
   -> type - cell type. If it is a title cell (type = IMATRIX_TITLE_COLOR), then
             only find a own color of the cell, and not in advanced modes to
             define colors.
   -> mark - indicate if a cell is marked. If yes, its color is attenuated.
*/
static unsigned long iMatrixSetFgColor(Ihandle* ih, int lin, int col, int mark)
{
  unsigned int r = 0, g = 0, b = 0;

  if(iMatrixCallFgColorCB(ih, lin, col, &r, &g, &b) == IUP_IGNORE)
    iMatrixDrawGetColor(ih, "FGCOLOR", lin, col, &r, &g, &b, 0);

  if(mark)
  {
    float att = iMatrixGetAttenuation(ih);
    r = (int)(r*att);
    g = (int)(g*att);
    b = (int)(b*att);
  }
  
  return cdCanvasForeground(ih->data->cddbuffer, cdEncodeColor((unsigned char)r, (unsigned char)g, (unsigned char)b));
}

static unsigned long iMatrixSetTitleFgColor(Ihandle* ih, int lin, int col)
{
  return iMatrixSetFgColor(ih, lin, col, 0);
}

/* Change the CD foreground color, for the selected color to draw a cell with
   its BACKGROUND COLOR. This involves checking if there is a color attribute
   that cell.  If no, uses a color attribute for the line, else if no for the
   column,  else if no  for the entire matrix.   Finally,  if not find any of
   these, use the default color.
   -> lin, col - cell coordinates, in IUP format - i.e., l,l represents the left
                 top cell of the matrix; lin and col values = 0 represents the
                 title lines and columns.
   -> type - cell type. If it is a title cell (type = IMATRIX_TITLE_COLOR), then
             only find a own color of the cell, and not in advanced modes to
             define colors.
   -> mark - indicate if a cell is marked. If yes, its color is attenuated.
*/
static unsigned long iMatrixSetBgColor(Ihandle* ih, int lin, int col, int type, int mark)
{
  unsigned int r = 255, g = 255, b = 255;
  int parent = 0;

  if(type == IMATRIX_TITLE_COLOR)
    parent = 1;

  if(iMatrixCallBgColorCB(ih, lin, col, &r, &g, &b) == IUP_IGNORE)
    iMatrixDrawGetColor(ih, "BGCOLOR", lin, col, &r, &g, &b, parent);
  
  if(mark)
  {
    float att = iMatrixGetAttenuation(ih);
    r = (int)(r*att);
    g = (int)(g*att);
    b = (int)(b*att);
  }

  return cdCanvasForeground(ih->data->cddbuffer, cdEncodeColor((unsigned char)r, (unsigned char)g, (unsigned char)b));
}

static unsigned long iMatrixSetTitleBgColor(Ihandle* ih, int lin, int col)
{
  int mark = 0;

  if((lin == 0 && iMatrixMarkColumnMarked(ih,col)) || 
     (col == 0 && iMatrixMarkLineMarked(ih,lin)))
    mark = 1;

  return iMatrixSetBgColor(ih, lin, col, IMATRIX_TITLE_COLOR, mark);
}

static void iMatrixSetEmptyAreaColor(Ihandle* ih)
{
  cdCanvasForeground(ih->data->cddbuffer, cdIupConvertColor(iupControlBaseGetParentBgColor(ih)));
}


/**************************************************************************/
/*  Private functions                                                     */
/**************************************************************************/

/* Returns which font attribute used to draw a specific cell of the matrix.
   -> lin, col - cell coordinates, in IUP format - i.e., 0,0 represents the
                 cell between titles, and 1,1 represents the cell of the
                 first line, first column of the matrix.
*/
char* iMatrixDrawGetFont(Ihandle* ih, int lin, int col)
{
  char* value;
  char* fontattr = iupStrGetMemory(30);

  /* 1 -  check for this cell */
  sprintf(fontattr, "FONT%d:%d", lin, col);
  value = iupAttribGetStr(ih, fontattr);

  if(!value)
  {
    /* 2 - check for this line, if not title col */
    if(col != 0)
    {
      sprintf(fontattr, "FONT%d:*", lin);
      value = iupAttribGetStr(ih, fontattr);
    }

    if(!value)
    {
      /* 3 - check for this column, if not title line */
      if(lin != 0)
      {
        sprintf(fontattr, "FONT*:%d", col);
        value = iupAttribGetStr(ih, fontattr);
      }

      if(!value)
      {
        /* 4 - check for the matrix or parent */
        value = IupGetAttribute(ih, "FONT");
      }
    }
  }
  return value;
}

static void iMatrixSetFont(Ihandle* ih, int lin, int col)
{
  cdCanvasNativeFont(ih->data->cddbuffer, iMatrixDrawGetFont(ih, lin, col));
}

/* Draw a box, like a button.
   -> px1, px2 - left bottom coordinates
   -> pdx, pdy - width and height
   -> lin, col - cell coordinates (to calculate the color)
*/
static void iMatrixBoxReleased(Ihandle* ih, int px1, int py1, int pdx, int pdy, int lin, int col)
{
  int px2 = px1 + pdx - 1;
  int py2 = py1 + pdy - 1;

  if(pdx < 2 || pdy < 2)
    return;

  cdCanvasForeground(ih->data->cddbuffer, IMATRIX_CD_BS);
  CdLine(px2, py1, px2, py2);
  CdLine(px1, py2, px2, py2);
  cdCanvasForeground(ih->data->cddbuffer, IMATRIX_CD_TS);
  CdLine(px1, py1, px1, py2);
  CdLine(px1, py1, px2, py1);
  iMatrixSetTitleBgColor(ih, lin, col);
  CdBox(px1 + 1, px2 - 1, py1 + 1, py2 - 1);
}

/* Draw a box, like a pressed button.
   -> px1, px2 - left bottom coordinates
   -> pdx, pdy - width and height
   -> lin, col - cell coordinates (to calculate the color)
*/
static void iMatrixBoxPressed(Ihandle* ih, int px1, int py1, int pdx, int pdy, int lin, int col)
{
  int px2 = px1 + pdx - 1;
  int py2 = py1 + pdy - 1 - 1;

  if(pdx < 2 || pdy < 2)
    return;

  cdCanvasForeground(ih->data->cddbuffer, IMATRIX_CD_TS);
  CdLine(px2, py1, px2, py2);
  CdLine(px1, py2, px2, py2);
  cdCanvasForeground(ih->data->cddbuffer, IMATRIX_CD_BS);
  CdLine(px1, py1, px1, py2);
  CdLine(px1, py1, px2, py1);
  iMatrixSetTitleBgColor(ih, lin, col);
  CdBox(px1 + 1, px2 - 1, py1 + 1, py2 - 1);
}

/* Decide if must draw the title as marked or not, and call the
   appropriate function to draw the title box.
   -> lin    - line to draw its title
   -> x1, y1 - left bottom coordinates of the box, in pixels,
               that will have the title
   -> dx, dy - width and height of the text box
*/
static void iMatrixTitleLineBox(Ihandle* ih, int lin, int x1, int y1, int dx, int dy)
{
  if(iMatrixMarkLineMarked(ih, lin))
    iMatrixBoxPressed(ih, x1, y1, dx, dy, lin, 0);
  else
    iMatrixBoxReleased(ih, x1, y1, dx, dy, lin, 0);
}

/* Decide if must draw the title as marked or not, and call the
   appropriate function to draw the title box.
   -> col    - column to draw its title
   -> x1, y1 - left bottom coordinates of the box, in pixels,
               that will have the title
   -> dx, dy - width and height of the text box
*/
static void iMatrixTitleColumnBox(Ihandle* ih, int col, int x1, int y1, int dx, int dy)
{
  if(iMatrixMarkColumnMarked(ih, col))
    iMatrixBoxPressed(ih, x1, y1, dx, dy, 0, col);
  else
    iMatrixBoxReleased(ih, x1, y1, dx, dy, 0, col);
}

static void iMatrixDrawComboFeedback(Ihandle* ih, int x2, int y1, int y2, int lin, int col, int cor)
{
  int xh2, yh2, x1;

  /* cell background */
  if(cor == IMATRIX_ELEM_COLOR)
    iMatrixSetBgColor(ih, lin, col, cor, 0);
  else
    iMatrixSetBgColor(ih, lin, col, cor, 1);
  CdBox(x2 - IMATRIX_BOXW, x2, y1, y2); 

  /* feedback area */
  x2 -= 3;
  x1  = x2 - IMATRIX_BOXW; 
  y1 += 2;
  y2 -= 3;

  /* feedback background */
  iMatrixSetTitleBgColor(ih, 0, 0);
  CdBox(x1, x2, y1, y2);

  /* feedback frame */
  iMatrixSetCdFrameColor(ih);
  CdRect(x1, x2, y1, y2);

  /* feedback arrow */
  xh2 = x2 - IMATRIX_BOXW / 2;
  yh2 = y2 - (y2 - y1) / 2;

  cdCanvasBegin(ih->data->cddbuffer, CD_FILL);
  CdVertex(xh2, yh2 + 3);
  CdVertex(xh2 + 4, yh2 - 1);
  CdVertex(xh2 - 4, yh2 - 1);
  cdCanvasEnd(ih->data->cddbuffer);
}

/* Put a text in the screen, using the specified color and alignment.
   Receive the size of cell and also the visible part of the cell,
   because need to clip with this visible part and calculate, correctly,
   the center position and right margin of the cell, using the entire
   size of it.
   -> y1, y2 : vertical limits of the cell
   -> x1, x2 : horizontal limits of the complete cell
   -> xc : point where the text is clipped
   -> alignment : alignment type (horizontal) assigned to the text. The options are:
                  [IMATRIX_T_CENTER,IMATRIX_T_LEFT,IMATRIX_T_RIGHT]
   -> cor : color schema that will be used:
           IMATRIX_TITLE_COLOR  ->  Black letters in gray background (for the titles)
           IMATRIX_ELEM_COLOR    -> Letters with FGCOLOR and background with BGCOLOR
           IMATRIX_REVERSE_COLOR -> Letters with BGCOLOR and background with FGCOLOR
   -> lin, col - cell coordinates, in IUP format - i.e., l,l represents the left
                 top cell of the matrix; lin and col values = 0 represents the
                 title lines and columns.
*/
static void iMatrixDrawText(Ihandle* ih, int x1, int x2, int y1, int y2, char *text, int alignment, int xc, int cor, int lin, int col)
{
  int oldbgc = -1;
  int ypos;

  int oldx1 = x1,                  /* Full limit of the cell,     */
      oldx2 = x2 - 1,              /* considering the decorations */
      oldy1 = y1,
      oldy2 = y2,
      oldxc = xc;

  /* Create an space between text and cell margin */
  x1 += IMATRIX_DECOR_X / 2;       x2 -= IMATRIX_DECOR_X / 2;       xc -= IMATRIX_DECOR_X / 2;          
  y1 += IMATRIX_DECOR_Y / 2;       y2 -= IMATRIX_DECOR_Y / 2;

  /* Clear the cell */
  if(cor == IMATRIX_TITLE_COLOR)
  {
    oldbgc = iMatrixSetBgColor(ih, lin, col, cor, 0);
    CdBox(oldx1 + 2, oldxc - 2, oldy1 + 2, oldy2 - 2); /* Clear cell box, considering the xc */
  }
  else if(cor == IMATRIX_ELEM_COLOR)
  {
    oldbgc = iMatrixSetBgColor(ih, lin, col, cor, 0);
    CdBox(oldx1, oldx2, oldy1, oldy2); /* Clear cell box */
  }
  else
  {
    oldbgc = iMatrixSetBgColor(ih, lin, col, cor, 1);
    /* Clear the text box, with the attenuated color... */
    CdBox(oldx1, oldx2, oldy1, oldy2);
    iMatrixSetFgColor(ih, lin, col, 1);
  }

  if(!iMatrixCallDrawCB(ih, cor, lin, col, oldx1, oldx2, oldy1, oldy2))
    return;

  /* Put the text */
  if(text && *text)
  {
    int numl;
    int lineh, totalh, spacing;

    /* Set the clip area to the cell region informed */
    CdClipArea(x1, xc, oldy1, oldy2);
    cdCanvasClip(ih->data->cddbuffer, CD_CLIPAREA);

    /* Set the color used to draw the text */
    if((lin > 0 && ih->data->lin.inactive[lin-1]) ||
       (col > 0 && ih->data->col.inactive[col-1]) ||
       !iupAttribGetInt(ih, "ACTIVE"))
      cdCanvasForeground(ih->data->cddbuffer, IMATRIX_CD_INACTIVE_COLOR);
    else if(cor == IMATRIX_ELEM_COLOR)
      iMatrixSetFgColor(ih, lin, col, 0);
    else if(cor == IMATRIX_REVERSE_COLOR)
      oldbgc = iMatrixSetFgColor(ih, lin, col, 1);
    else
      iMatrixSetFgColor(ih, lin, col, 0);

    numl = iMatrixTextHeight(ih, text, &totalh, &lineh, &spacing);

    iMatrixSetFont(ih, lin, col);

    if(numl == 1)
    {
      ypos = (int)((y1 + y2) / 2.0 - .5);

      /* Put the text */
      if(alignment == IMATRIX_T_CENTER)
        CdPutText((x1 + x2) / 2, ypos, text, CD_CENTER);
      else if(alignment == IMATRIX_T_LEFT)
        CdPutText(x1, ypos, text, CD_WEST);
      else
        CdPutText(x2, ypos, text, CD_EAST);
    }
    else
    {
      int   i;
      char *p, *q, *newtext = NULL;

      if(text != NULL)
      {
        p = (char*) iupStrDup(text);
        newtext = p;
      }
      else
        p = NULL;

      /* Get the position of the first text to be put in the screen */
      ypos = (int)( (y1 + y2) / 2.0 - .5) - totalh / 2 + lineh / 2;
      for(i = 0; i < numl; i++)
      {
        q = strchr(p, '\n');
        if(q)
          *q = 0;  /* Cut the string to contain a line */

        /* Put the text */
        if(alignment == IMATRIX_T_CENTER)
          CdPutText((x1 + x2) / 2, ypos, p, CD_CENTER);
        else if(alignment == IMATRIX_T_LEFT)
          CdPutText(x1, ypos, p, CD_WEST);
        else
          CdPutText(x2, ypos, p, CD_EAST);

        if(q)
          *q = '\n'; /* Restore the string */
        p = q + 1;

        /* Advance a line */
        ypos += lineh + spacing;
      }

      if(newtext)
        free(newtext);
    }

    cdCanvasClip(ih->data->cddbuffer, CD_CLIPOFF);
  }

  if(cor != IMATRIX_TITLE_COLOR)
    CdRestoreBgColor();
}

static void iMatrixDrawSort(Ihandle* ih, int x2, int y1, int y2, int xc, int lin, int col, char* sort)
{
  int yc;

  /* Create an space between text and cell margin */
  x2 -= IMATRIX_DECOR_X / 2;       xc -= IMATRIX_DECOR_X / 2;

  /* Set the color used to draw the text */
  if((lin > 0 && ih->data->lin.inactive[lin-1]) ||
     (col > 0 && ih->data->col.inactive[col-1]) ||
     !iupAttribGetInt(ih, "ACTIVE"))
    cdCanvasForeground(ih->data->cddbuffer, IMATRIX_CD_INACTIVE_COLOR);
  else
    iMatrixSetFgColor(ih, lin, col, 0);

  yc = (int)( (y1 + y2 ) / 2.0 - .5);

  cdCanvasBegin(ih->data->cddbuffer, CD_FILL);

  if(iupStrEqualNoCase(sort, "UP"))
  {
    CdVertex(x2 - 5, yc + 2);
    CdVertex(x2 - 1, yc - 2);
    CdVertex(x2 - 9, yc - 2);
  }
  else
  {
    CdVertex(x2 - 1, yc + 2);
    CdVertex(x2 - 9, yc + 2);
    CdVertex(x2 - 5, yc - 2);
  }

  cdCanvasEnd(ih->data->cddbuffer);
}

/* Return the alignment to be used by a specific column of the matrix */
static int iMatrixGetColAlignment(Ihandle* ih, int col)
{
  char* attr = iupStrGetMemory(15);
  char* align;

  sprintf(attr, "ALIGNMENT%d", col);
  align = IupGetAttribute(ih, attr);

  if(iupStrEqualNoCase(align, "ALEFT"))
    return IMATRIX_T_LEFT;
  else if(iupStrEqualNoCase(align, "ACENTER"))
    return IMATRIX_T_CENTER;
  else
    return IMATRIX_T_RIGHT;
}


/**************************************************************************/
/* Exported functions                                                     */
/**************************************************************************/

/* Draw the line titles, visibles, between lin and lastlin, include it. 
   Line titles marked will be draw with the appropriate feedback.
   -> lin - First line to have its title drawn
   -> lastlin - Last line to have its title drawn
*/
int iMatrixDrawLineTitle(Ihandle* ih, int lin, int lastlin)
{
  int x1, y1, x2, y2;
  int j;
  char *str;

  /* If no has line title, also must return the position (in pixels).
     This position would be the vertical position in the end of the
     last line drawn, if necessary the draw. This is a requirement
     used by scroll function
  */

  /* Fix lin and lastlin to contain the region of the lines that
     will be draw (visible)
  */

  if(lin < ih->data->lin.first)
    lin = ih->data->lin.first;

  if(lastlin > ih->data->lin.last)
    lastlin = ih->data->lin.last;

  if(lastlin < lin)
    return 0;

  /* Start the position of the line title */
  x1 = 0;
  x2 = ih->data->col.titlewh;
  y1 = ih->data->lin.titlewh;

  for(j = ih->data->lin.first; j < lin; j++)
    y1 += ih->data->lin.wh[j];

  ih->data->redraw = 1;

  /* Draw the titles */
  for(j = lin; j <= lastlin; j++)
  {
    /* If it is an hide line (size = 0), no draw the title */
    if(ih->data->lin.wh[j] == 0)
      continue;

    y2 = y1 + ih->data->lin.wh[j]-1;

    /* If it doesn't have title, the loop just calculate the final position */
    if(ih->data->col.titlewh)
    {
      iMatrixTitleLineBox(ih, j+1, x1, y1, x2-x1, y2-y1+1);
      str = iMatrixGetCellValue(ih, j, -1);
      iMatrixDrawText(ih, x1, x2, y1, y2, str, iMatrixGetColAlignment(ih, 0), x2, IMATRIX_TITLE_COLOR, j+1, 0);
      iMatrixSetCdFrameColor(ih);
      CdLine(x2-1, y1, x2-1, y2);
    }

    y1 = y2 + 1;
  }

  return y1;
}

/* Draw the column titles, visible, between col and lastcol, include it. 
   Column titles marked will be draw with the appropriate feedback.
   -> col - First column to have its title drawn
   -> lastcol - Last column to have its title drawn
*/
int iMatrixDrawColumnTitle(Ihandle* ih, int col, int lastcol)
{
  int x1, y1, x2, y2;
  int j;
  char *str;

  /* If no has column title, also must return the position (in pixels).
     This position would be the horizontal position in the end of the
     last column drawn, if necessary the draw. This is a requirement
     used by scroll function
  */

  /* Fix col and lastcol to contain the region of the columns that
     will be draw (visible)
  */

  if(col < ih->data->col.first)
    col = ih->data->col.first;

  if(lastcol > ih->data->col.last)
    lastcol = ih->data->col.last;

  if(lastcol < col)
    return 0;
 
  /* Start the position of the first column title */
  y1 = 0;
  y2 = ih->data->lin.titlewh - 1;
  x1 = ih->data->col.titlewh;

  for(j = ih->data->col.first; j < col; j++)
    x1 += ih->data->col.wh[j];

  ih->data->redraw = 1;

  /* Draw the titles */
  for(j = col; j <= lastcol; j++)
  {
    int w, alignment;

    /* If it is an hide column (size = 0), no draw the title */
    if(ih->data->col.wh[j] == 0)
      continue;

    /* Decide which alignment must be used.
       If the text can be inside the canvas, then center alignment,
       if no, left alignment
    */
    x2 = x1 + (j == ih->data->col.last ? ih->data->col.lastwh : ih->data->col.wh[j]);

    /* If it doesn't have title, the loop just calculate the final position */
    if(ih->data->lin.titlewh)
    {
      str = iMatrixGetCellValue(ih, -1, j);
      if(str)
        iMatrixTextWidth(ih, str, &w);
      else
        w = 0;
      alignment = (w > x2 - x1 + 1 - IMATRIX_DECOR_X) ? IMATRIX_T_LEFT : IMATRIX_T_CENTER;

      /* draw the title */
      iMatrixTitleColumnBox(ih, j+1, x1, y1, x2-x1, y2-y1+1);
      iMatrixDrawText(ih, x1, x1 + ih->data->col.wh[j], y1, y2, str, alignment, x2, IMATRIX_TITLE_COLOR, 0, j+1);
      iMatrixSetCdFrameColor(ih);
      CdLine(x1, y2, x2-1, y2);
      {
        char* aux = iupStrGetMemory(50);
        char* sort;
        sprintf(aux, "SORTSIGN%d", j+1);
        sort = iupAttribGetStr(ih, aux);
        if(sort && !iupStrEqualNoCase(sort, "NO"))
          iMatrixDrawSort(ih, x1 + ih->data->col.wh[j], y1, y2, x2, 0, j+1, sort);
      }
    }
    x1 = x2;
  }

  return x1;
}

/* Draw the corner between line and column titles */
void iMatrixDrawTitleCorner(Ihandle* ih)
{
  char *str;

  /* If there are lines or columns, and exist column and line titles, */
  /* then draw the left top corner                                    */
  if((ih->data->lin.num != 0 || ih->data->col.num != 0) && ih->data->lin.titlewh && ih->data->col.titlewh)
  {
    ih->data->redraw = 1;

    iMatrixBoxReleased(ih, 0, 0, ih->data->col.titlewh, ih->data->lin.titlewh, 0, 0);
    str = iMatrixGetCellValue(ih, -1, -1);
    iMatrixDrawText(ih, 0, ih->data->col.titlewh, 0, ih->data->lin.titlewh-1, str, IMATRIX_T_CENTER, ih->data->col.titlewh, IMATRIX_TITLE_COLOR, 0, 0);
  }
}

/* Clear the received area with the appropriated color for the matrix area
   that no have cells
   -> x1, x2, y1, y2 - area coordinates to clear.
*/
void iMatrixDrawEmptyArea(Ihandle* ih, int x1, int x2, int y1, int y2)
{
  iMatrixSetEmptyAreaColor(ih);
  CdBox(x1, x2, y1, y2);
  ih->data->redraw = 1;
}

/* Redraw the entire matrix, may or not redraw the line and column titles
   -> modo : this constant specifics the titles will be redraw, with the
             following values:
             IMATRIX_DRAW_ALL -> Redraw the title columns and lines
             IMATRIX_DRAW_COL -> Redraw just the title columns
             IMATRIX_DRAW_LIN -> Redraw just the title lines
     ... always redraw the cells - the "modo" just draw the titles ...
*/
void iMatrixDrawMatrix(Ihandle* ih, int modo)
{
  ih->data->redraw = 1;

  if((ih->data->lin.num == 0) || (ih->data->col.num == 0))
  {
    cdCanvasBackground(ih->data->cddbuffer, cdIupConvertColor(iupControlBaseGetParentBgColor(ih)));
    cdCanvasClear(ih->data->cddbuffer);
  }

  /* Draw the corner between line and column titles, if necessary */
  iMatrixDrawTitleCorner(ih);

  /* If there are columns and must draw them, then draw the titles */
  if((ih->data->col.num != 0) &&
     (modo == IMATRIX_DRAW_ALL || modo == IMATRIX_DRAW_COL) && ih->data->lin.titlewh)
  {
    iMatrixDrawColumnTitle(ih, ih->data->col.first, ih->data->col.last);
  }

  /* If there are lines and must draw them, then draw the titles */
  if((ih->data->lin.num != 0) &&
     (modo == IMATRIX_DRAW_ALL || modo == IMATRIX_DRAW_LIN) &&
     ih->data->col.titlewh)
  {
    iMatrixDrawLineTitle(ih, ih->data->lin.first, ih->data->lin.last);
  }

  /* If there are cells in the matrix, then draw the cells */
  if((ih->data->lin.num != 0) && (ih->data->col.num != 0))
    iMatrixDrawCells(ih, ih->data->lin.first, ih->data->col.first, ih->data->lin.last, ih->data->col.last);
}

static int iMatrixCallDropDownCheckCb(Ihandle* ih, int line, int col)
{
  IFnii cb = (IFnii)IupGetCallback(ih, "DROPCHECK_CB");
  if(cb)
  {
    int ret = cb(ih, line + 1, col + 1);
    if(ret == IUP_DEFAULT)
      return 1;
  }
  return 0;
}

/* Redraw a block of cells of the matrix. Handle marked cells, change
   automatically the background color of them.
   - l1, c1 : cell coordinates that mark the left top corner of the
              area to be redrawn
   - l2, c2 : cell coordinates that mark the right bottom corner of the
              area to be redrawn
*/
void iMatrixDrawCells(Ihandle* ih, int l1, int c1, int l2, int c2)
{
  int x1, y1, x2, y2, oldx2, oldy1, oldy2;
  int yc1, yc2, xc1, xc2, i, j;
  int align;
  long framecolor;
  char str[30];
  unsigned char r,g,b;

  /* If there are no cells in the matrix, returns */
  if(ih->data->lin.num == 0 || ih->data->col.num == 0)
   return;

  /* Adjust parameters */
  if(c1 < ih->data->col.first)
    c1 = ih->data->col.first;
  if(c2 > ih->data->col.last)
    c2 = ih->data->col.last;

  if(l1 < ih->data->lin.first)
    l1 = ih->data->lin.first;
  if(l2 > ih->data->lin.last)
    l2 = ih->data->lin.last;

  if(c1 > c2 || l1 > l2)
    return;

  ih->data->redraw = 1;

  if(l1 <= l2)
    iMatrixDrawLineTitle(ih, l1, l2);
  if(c1<=c2)
    iMatrixDrawColumnTitle(ih, c1, c2);

  x1 = 0;
  x2 = ih->data->XmaxC;
  y1 = 0;
  y2 = ih->data->YmaxC;

  oldx2 = x2;
  oldy1 = y1;
  oldy2 = y2;

  /* Find the initial position of the first column */
  x1 += ih->data->col.titlewh;
  for(j = ih->data->col.first; j < c1; j++)
    x1 += ih->data->col.wh[j];

  /* Find the final position of the last column */
  x2 = x1;
  for( ; j < c2; j++)
    x2 += ih->data->col.wh[j];
  x2 += (c2 == ih->data->col.last ? ih->data->col.lastwh : ih->data->col.wh[c2]);

  /* Find the initial position of the first line */
  y1 += ih->data->lin.titlewh;
  for(j = ih->data->lin.first; j < l1; j++)
    y1 += ih->data->lin.wh[j];

  /* Find the final position of the last line */
  y2 = y1;
  for( ; j < l2; j++)
    y2 += ih->data->lin.wh[j];
  y2 += (l2 == ih->data->lin.last ? ih->data->lin.lastwh : ih->data->lin.wh[l2]);

  if((c2 == ih->data->col.num-1) && (oldx2 > x2))
  {
    /* If it was drawn until the last column and remains space in the right of it,
       then delete this area with the the background color.
    */
    iMatrixDrawEmptyArea(ih, x2, oldx2, oldy1, oldy2);
  }

  if((l2 == ih->data->lin.num-1) && (oldy2 > y2))
  {
    /* If it was drawn until the last line visible and remains space below it,
       then delete this area with the the background color.
    */
    iMatrixDrawEmptyArea(ih, 0, oldx2, y2, oldy2);
  }

  /***** Show the cell values */
  xc1 = x1;
  yc1 = y1;
  iupStrToRGB(iupAttribGetStrDefault(ih, "FRAMECOLOR"), &r, &g, &b);
  framecolor = cdEncodeColor((unsigned char) r, (unsigned char) g, (unsigned char) b);

  for(j = c1; j <= c2; j++)  /* For all the columns in the region */
  {
    if(ih->data->col.wh[j] == 0)
      continue;

    align = iMatrixGetColAlignment(ih, j + 1);

    xc2 = xc1 + (j == ih->data->col.last ? ih->data->col.lastwh : ih->data->col.wh[j]);

    for(i = l1; i <= l2; i++)     /* For all lines in the region */
    {
      if(ih->data->lin.wh[i] == 0)
        continue;

      yc2 = yc1 + ih->data->lin.wh[i]-1;

      if(!(IupGetInt(ih->data->datah, "VISIBLE") && i == ih->data->lin.active && j == ih->data->col.active))
      {
        int drop = 0;
        int cor  = IMATRIX_ELEM_COLOR;
        char *cell_value;

        if(iMatrixCallDropDownCheckCb(ih, i, j))
          drop = IMATRIX_BOXW;

        /* If the cell is marked, then draw it in reverse color */
        if(iMatrixMarkCellGet(ih, i, j))
          cor = IMATRIX_REVERSE_COLOR;

        cell_value = iMatrixGetCellValue(ih, i, j);
        iMatrixDrawText(ih, xc1, xc1+ih->data->col.wh[j]-1-drop, yc1, yc2-1, cell_value, align, xc2, cor, i+1, j+1);

        if(drop)
          iMatrixDrawComboFeedback(ih, xc1+ih->data->col.wh[j]-1, yc1, yc2, i+1, j+1, cor);
      }

      if (ih->data->checkframecolor)
      {
        sprintf(str, "FRAMEHORIZCOLOR%d:%d", i, j);
        if (iupStrToRGB(iupAttribGetStr(ih, str), &r, &g, &b))
          cdCanvasForeground(ih->data->cddbuffer, cdEncodeColor((unsigned char) r, (unsigned char) g, (unsigned char) b));
        else
          cdCanvasForeground(ih->data->cddbuffer, framecolor);
      }
      else
        cdCanvasForeground(ih->data->cddbuffer, framecolor);
    
      /* horizontal line (only for this column) */
      CdLine(xc1, yc2, xc2-1, yc2);
      
      if (ih->data->checkframecolor)
      {
        sprintf(str, "FRAMEVERTCOLOR%d:%d", i+1, j+1);
        if (iupStrToRGB(iupAttribGetStr(ih, str), &r, &g, &b))
          cdCanvasForeground(ih->data->cddbuffer, cdEncodeColor((unsigned char) r, (unsigned char) g, (unsigned char) b));
        else
          cdCanvasForeground(ih->data->cddbuffer, framecolor);
      }
      else
        cdCanvasForeground(ih->data->cddbuffer, framecolor);

      /* vertical line (only for this line) */
      CdLine(xc2-1,yc1,xc2-1,yc2-1);

      yc1  = yc2+1;
    }

    xc1 = xc2;
    yc1 = y1;
  }
}

/* Draw the representation of a cell that have the focus, for cells that
   are not in the edit mode
   -> lin, col : the cell
   -> set: 1 to put the draw of the focus
           0 to remove the draw
*/
void iMatrixDrawFocus(Ihandle* ih, int lin, int col, int set)
{
  int x1, y1, x2, y2, dx, dy, oldbgc;

  if(iupAttribGetInt(ih, "HIDEFOCUS"))
    return;

  if(!iMatrixGetCellDim(ih, lin, col, &x1, &y1, &dx, &dy))
    return;

  ih->data->redraw = 1;

  if(set)          /* put the draw of the focus */
    oldbgc = iMatrixSetFgColor(ih, lin+1, col+1, iMatrixMarkCellGet(ih, lin, col));
  else                /* remove the draw of the focus */
    oldbgc = iMatrixSetBgColor(ih, lin+1, col+1, IMATRIX_ELEM_COLOR, iMatrixMarkCellGet(ih, lin, col));

  x2 = x1 + dx - 1;
  y2 = y1 + dy - 1;

  CdRect(x1, x2, y1, y2);
  CdRect(x1+1, x2-1, y1+1, y2-1);
  
  CdRestoreBgColor();
}

/* Return which the foreground color attribute used to draw a specific cell
   of the matrix.
   -> lin, col - cell coordinates that will have its color localized.
                 IUP format is used - i.e., 0,0 represents the
                 cell between titles, and 1,1 represents the cell of the
                 first line, first column of the matrix.
*/
char* iMatrixDrawGetFgColor(Ihandle* ih, int lin, int col)
{
  char* buffer = iupStrGetMemory(30);
  unsigned int r = 0, g = 0, b = 0;

  if (iMatrixCallFgColorCB(ih, lin, col, &r, &g, &b) == IUP_IGNORE)
    iMatrixDrawGetColor(ih, "FGCOLOR", lin, col, &r, &g, &b, 0);

  sprintf(buffer, "%d %d %d", r, g, b);
  return buffer;
}

/* Return which the background color attribute used to draw a specific cell
   of the matrix.
   -> lin, col - cell coordinates that will have its color localized.
                 IUP format is used - i.e., 0,0 represents the
                 cell between titles, and 1,1 represents the cell of the
                 first line, first column of the matrix.
*/
char* iMatrixDrawGetBgColor(Ihandle* ih, int lin, int col)
{
  char* buffer = iupStrGetMemory(30);
  unsigned int r = 255, g = 255, b = 255;
  int parent = 0;

  if(lin == 0 || col == 0)
    parent = 1;

  if(iMatrixCallBgColorCB(ih, lin, col, &r, &g, &b) == IUP_IGNORE)
    iMatrixDrawGetColor(ih, "BGCOLOR", lin, col, &r, &g, &b, parent);

  sprintf(buffer, "%d %d %d", r, g, b);
  return buffer;
}
