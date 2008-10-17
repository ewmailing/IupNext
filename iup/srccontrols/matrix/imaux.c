/** \file
 * \brief iupmatrix control
 * auxiliary functions
 *
 * See Copyright Notice in iup.h
 * $Id: imaux.c,v 1.1 2008-10-17 06:05:35 scuri Exp $
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

#include "imdraw.h"
#include "matrixdef.h"
#include "imaux.h"
#include "immem.h"
#include "imfocus.h"
#include "imedit.h"
#include "matrixcd.h"

/**************************************************************************/
/*   Exported functions                                                   */
/**************************************************************************/

/* Verify if a cell is visible (1) or not (0)
   -> lin, col: cell coordinates               */
int iMatrixIsCellVisible(Ihandle* ih, int lin, int col)
{
  /* No lines or columns, the cell is not visible */
  if((ih->data->col.num == 0) || (ih->data->lin.num == 0))
    return 0;

  if(((lin >= ih->data->lin.first) &&
      (lin <= ih->data->lin.last) &&
      (col >= ih->data->col.first) &&
      (col <= ih->data->col.last)))
  {
    return 1;
  }

  return 0;
}

/* Calculate the coordinates of the left-bottom and right-top cell corners.
   -> lin, col - cell coordinates
   -> x, y - left-bottom coordinates
   -> dx, dy - width and height of the cell                                
*/
int iMatrixGetCellDim(Ihandle* ih, int lin, int col, int* x, int* y, int* dx, int* dy)
{
  int i;

  if(!iMatrixIsCellVisible(ih, lin, col))
    return 0;

  /* find the position where the column starts */
  *x = ih->data->col.titlewh;
  for(i = ih->data->col.first; i < col; i++)
    *x += ih->data->col.wh[i];

  /* find the column size */
  *dx = ih->data->col.wh[col] - 1;

  /* find the position where the line starts */
  *y = ih->data->lin.titlewh;
  for(i = ih->data->lin.first; i < lin; i++)
    *y += ih->data->lin.wh[i];

  /* find the line size */
  *dy = ih->data->lin.wh[lin] - 1;

  return 1;
}

/* Calculate the size, in pixels, of the invisible columns/lines,
   the left/above of the first column/line.
   -> m : choose will operate on lines or columns [IMATRIX_MAT_LIN|IMATRIX_MAT_COL]
*/
void iMatrixGetPos(Ihandle* ih, int m)
{
  int i;
  Tlincol *p;

  if(m == IMATRIX_MAT_LIN)
    p = &(ih->data->lin);
  else
    p = &(ih->data->col);

  p->pos = 0;
  for(i = 0; i < p->first; i++)
    p->pos += p->wh[i];
}

/* Calculate which is the last visible column/line of the matrix and
   also its width/height.
   -> m : choose will operate on lines or columns [IMATRIX_MAT_LIN|IMATRIX_MAT_COL]
*/
void iMatrixGetLastWidth(Ihandle* ih, int m)
{
  int i, soma = 0, osoma;
  Tlincol *p;

  if(m == IMATRIX_MAT_LIN)
    p = &(ih->data->lin);
  else
    p = &(ih->data->col);

  if(p->num == 0)
    return;

  if(soma < p->size)
  {
    /* Find which is the last column/line
       Start in the first visible and continue adding the widths
       up to the size of the matrix
    */
    for(i = p->first, osoma = 0; i < p->num; i++)
    {
      osoma  = soma;
      soma  += p->wh[i];
      if(soma >= p->size)
        break;
    }

    if(i == p->num)
    {
      p->last   = i-1;
      p->lastwh = p->wh[i-1];

      /* Adjust the first line/column not to be empty spaces */
      for(soma = 0, i = p->num-1; i >= 0; i--)
      {
        soma += p->wh[i];
        if(soma > p->size)
          break;
      }
      p->first = i+1;
      if(i >= 0)
        soma -= p->wh[i];
      p->pos = p->totalwh - soma;
    }
    else
    {
      p->last   = i;
      p->lastwh = p->size - osoma;
    }
  }
  else
  {
    /* There is not space for any column, set the last column as 0 */
    p->last   = 0;
    p->lastwh = 0;
  }
}

/* Return the width of the column of the line titles. Verify if have an value
   set to WIDTH0. If yes, returns the value, already with its size converted
   to pixels. If no, returns the size in pixels of the largest line title        */
int iMatrixGetTitlelineSize(Ihandle* ih)
{
  int i, wt = 0;
  char* tline;
  char* larg;
  char* aux = iupStrGetMemory(100);

  larg = iupAttribGetStr(ih, "WIDTH0");
  if(larg)
  {
    /* user defined a size to the titles */
    int charwidth, charheight;

    iupdrvFontGetCharSize(ih, &charwidth, &charheight);

    if(iupStrToInt(larg, &wt))
      wt = (int)((wt/4.) * charwidth) + IMATRIX_DECOR_X;
  }
  else 
  {
    larg = iupAttribGetStr(ih, "RASTERWIDTH0");
    if(larg)
    {
      if(iupStrToInt(larg, &wt))
        wt += IMATRIX_DECOR_X;
    }
    else if(!ih->data->valcb)
    {
      /* If valcb will be defined, doesn't find in attributes, because
         the user must provide the value to be draw...

         Find the title with the biggest size
      */
      wt = 0;
      for(i = 0; i < ih->data->lin.num; i++)
      {
        sprintf(aux, IMATRIX_TITLE_LIN, i+1);
        tline = iupAttribGetStr(ih, aux);
        if(tline)
        {
          int sizet = iupdrvFontGetStringWidth(ih, tline);
          if(sizet > wt)
            wt = sizet;
        }
      }

      if(wt > 0)   /* have title column, add decoration */
        wt += IMATRIX_DECOR_X;
    }
  }
  return wt;
}

/* Return the height of the line of the column titles. Verify if have an value
   set to HEIGHT0. If yes, returns the value, already with its size converted
   to pixels. If no, returns the size in pixels of the largest column title       */
int iMatrixGetTitlecolumnSize(Ihandle* ih)
{
  char *alt;
  int ht = 0;

  alt = iupAttribGetStr(ih, "HEIGHT0");
  if(alt)
  {
    int charwidth, charheight;

    /* user defined a size to the titles */
    iupdrvFontGetCharSize(ih, &charwidth, &charheight);
  
    if(iupStrToInt(alt, &ht))
      return (int)((ht/8.) * charheight) + IMATRIX_DECOR_Y;
  }
  else
  {
    alt = iupAttribGetStr(ih, "RASTERHEIGHT0");
    if(alt)
    {
      if(iupStrToInt(alt, &ht))
        return ht + IMATRIX_DECOR_Y;
    }
    else if(!ih->data->valcb)
    {
      int i;
      int max = 0;
      char* aux = iupStrGetMemory(100);

      for(i = -1; i < ih->data->col.num; i++)  /* Comeca de -1 para verificar celula 0:0 */
      {
        sprintf(aux, IMATRIX_TITLE_COL, i+1);
        alt = iupAttribGetStr(ih, aux);
        if(alt)
        {
          iMatrixTextHeight(ih, alt, &ht, NULL, NULL);
          if(ht > max) max = ht;
        }
      }
      if(max)
        return max + IMATRIX_DECOR_Y;
    }
  }
  return 0;
}

/* Find the width of a column.
   col : number of the column (0 = first column)  */
int iMatrixGetColumnWidth(Ihandle* ih, int col)
{
  int w;
  char* aux = iupStrGetMemory(100);
  char* larg;
  int charwidth, charheight, pixels = 0;

  iupdrvFontGetCharSize(ih, &charwidth, &charheight);

  sprintf(aux, "WIDTH%d", col+1);  /* Have a width own defined ?  */
  larg = iupAttribGetStr(ih, aux);
  if(!larg)
  {
    sprintf(aux, "RASTERWIDTH%d", col+1);
    larg = iupAttribGetStr(ih, aux);
    if(larg)
      pixels = 1;
  }

  if(!larg)
    larg = iupAttribGetStrDefault(ih, "WIDTHDEF");  /* Attempts to attribute the default width */

  if(iupStrToInt(larg, &w))
  {
    if(w == 0)
      return 0;
    else if(w < 0)
      return charwidth * 10 + IMATRIX_DECOR_X;
    else
    {
      if(pixels)
        return w + IMATRIX_DECOR_X;
      else
        return (int)(((w/4.) * charwidth) + IMATRIX_DECOR_X);
    }
  }
  return 0;
}

/* Find the height of a line, in pixels.
   If the HEIGHT attribute is set, returns this value. If no, and the text of title 
   does not exist or contains just one line, returns the HEIGHTDEF value. Otherwise,
   returns the size occupied by title.
   -> lin : number of line (0 = first line)
*/
int iMatrixGetLineHeight(Ihandle* ih, int lin)
{
  int a, numl;
  char* aux = iupStrGetMemory(100);
  char* alt;
  int charwidth, charheight, pixels = 0;

  iupdrvFontGetCharSize(ih, &charwidth, &charheight);

  sprintf(aux, "HEIGHT%d", lin+1);        /* Have a width own defined ?  */
  alt = iupAttribGetStr(ih, aux);
  if(!alt)
  {
    sprintf(aux, "RASTERHEIGHT%d", lin+1);
    alt = iupAttribGetStr(ih, aux);
    if(alt)
      pixels = 1;
  }

  if(!alt)
  {
    sprintf(aux, IMATRIX_TITLE_LIN, lin+1);
    numl = iMatrixTextHeight(ih, iupAttribGetStr(ih, aux), &a, NULL, NULL);
    if(numl > 1)
      return a + IMATRIX_DECOR_Y;

    alt  = iupAttribGetStrDefault(ih, "HEIGHTDEF");  /* Attempts to attribute the default height */
  }
  
  if(iupStrToInt(alt, &a))
  {
    if(a == 0)
      return 0;
    else if(a < 0)
      return charheight + IMATRIX_DECOR_Y;
    else
    {
      if(pixels)
        return a + IMATRIX_DECOR_Y;
      else
        return (int)((a/8.) * charheight) + IMATRIX_DECOR_Y;
    }
  }
  return 0;
}

/* Fill the ih->data->col.wh vector with the width of all the columns.
   Calculate the value of ih->data->col.totalwh
*/
void iMatrixFillWidthVec(Ihandle* ih)
{
  int i;

  /* Calculate total width of the matrix and the width of each column */
  ih->data->col.totalwh = 0;
  for(i = 0; i < ih->data->col.num; i++)
  {
    ih->data->col.wh[i] = iMatrixGetColumnWidth(ih, i);
/*
    if(ih->data->col.wh[i] > ih->data->col.size)  // This condition was changed the width...
      ih->data->col.wh[i] = ih->data->col.size;   // ih->data->col.size was always zero or a negative value
*/
    ih->data->col.totalwh += ih->data->col.wh[i];
  }
}

/* Fill the ih->data->lin.wh vector with the height of all the columns.
   Calculate the value of ih->data->lin.totalwh
*/
void iMatrixFillHeightVec(Ihandle* ih)
{
  int i;

  /* Calculate total height of the matrix and the height of each line */
  ih->data->lin.totalwh = 0;
  for(i = 0; i < ih->data->lin.num; i++)
  {
    ih->data->lin.wh[i] = iMatrixGetLineHeight(ih, i);
/*
    if(ih->data->lin.wh[i] > ih->data->lin.size)  // This condition was changed the height...
      ih->data->lin.wh[i] = ih->data->lin.size;   // ih->data->lin.size was always zero or a negative value
*/
    ih->data->lin.totalwh += ih->data->lin.wh[i];
  }
}

/* Given a x, y positon, this function returns the correspondent cell,
   in row and column, being lin = -1 for row title and col = -1 for column title.
   If it is on the vertical scrollbar, col = -2; and on the horizontal scrollbar,
   lin = -2. When the matrix doesn't have columns, col = -2; and doesn't have
   lines, lin = -2.
   If the coordinate doesn't have inside the matrix, returns 0.
   When the cell is valid (inside the matrix), returns 1.
*/
int iMatrixGetLineCol(Ihandle* ih, int x, int y, int* l, int* c)
{
  int width, lin, col;

  /* Get the selected column */
  width = ih->data->col.titlewh;
  if(x < width)
    col = -1;  /* It is in the column titles */
  else
  {
    if(ih->data->col.num == 0)
      col = -2;
    else
    {
      for(col = ih->data->col.first; col <= ih->data->col.last; col++)
      {
        width += (col == ih->data->col.last ? ih->data->col.lastwh : ih->data->col.wh[col]);
        if(width >= x)
          break;
      }
      if(col > ih->data->col.last) /* It is in the empty area after the last column or on the scrollbar... */
        col = -2;
    }
  }

  width = ih->data->lin.titlewh;
  if(y < width)
    lin = -1;  /* It is in the line titles */
  else
  {
    if(ih->data->lin.num == 0)
      lin = -2;
    else
    {
      for(lin = ih->data->lin.first; lin <= ih->data->lin.last; lin++)
      {
        width += (lin == ih->data->lin.last ? ih->data->lin.lastwh : ih->data->lin.wh[lin]);
        if(width >= y)
          break;
      }
      if(lin > ih->data->lin.last)  /* It is in the area on the scrollbar... */
        lin = -2;
    }
  }

  if(col == -2 && lin == -2 ) /* It is in any place between the two scrollbars... */
    return 0;

#if _OLD_CODE_
  if(col == -1 && lin == -1 ) /* It is in the left top corner */
    return 0;
#endif

  *l = lin;
  *c = col;
  return 1;
}

/* Returns the value of the one cell. It is possible to get this value
   through matrix internal struct or through a query from a user callback.
   -> lin, col : cell coordiantes that will have its value returned. (0,0) is
                 the cell coordinate localized in the left top corner of the
                 matrix. The value -1 indicates a line or column title.
*/
char* iMatrixGetCellValue(Ihandle* ih, int lin, int col)
{
  char* aux = iupStrGetMemory(100);

  if(ih->data->valcb)
  {
    return ih->data->valcb(ih, lin+1, col+1);
  }
  else
  {
    if(lin == -1 && col == -1)
    {
      return iupAttribGetStr(ih, "0:0");
    }
    else if(lin == -1)  /* Column title */
    {
      sprintf(aux, IMATRIX_TITLE_COL, col+1);
      return iupAttribGetStr(ih, aux);
    }
    else if(col == -1)  /* Line title */
    {
      sprintf(aux, IMATRIX_TITLE_LIN, lin+1);
      return iupAttribGetStr(ih, aux);
    }
    else
    {
      return ih->data->v[lin][col].value;
    }
  }
}


/* Get the edit value and update in the structure */
void iMatrixUpdateCellValue(Ihandle* ih)
{
  char *valor;
  int  tam=0;

  valor = iMatrixEditGetValue(ih);

  if(ih->data->valeditcb)
  {
    ih->data->valeditcb(ih, ih->data->lin.active+1, ih->data->col.active+1, valor);
    return;
  }
  else if(ih->data->valcb) /* do nothing when in callback mode if valeditcb not defined */
    return;

  if(valor)
    tam = strlen(valor);

  if(tam != 0)
  {
    iMatrixMemAlocCell(ih, ih->data->lin.active, ih->data->col.active, tam);
    strcpy(ih->data->v[ih->data->lin.active][ih->data->col.active].value, valor);
  }
  else if(ih->data->v[ih->data->lin.active][ih->data->col.active].value != NULL)
    ih->data->v[ih->data->lin.active][ih->data->col.active].value[0] = '\0';
}

/* Call the user callback associated with the event of a cell leave the focus */
int iMatrixCallLeavecellCb(Ihandle* ih)
{
  IFnii cb = (IFnii)IupGetCallback(ih, "LEAVEITEM_CB");
  if(cb)
  {
   int ret = cb(ih, ih->data->lin.active+1, ih->data->col.active+1);

   return ret;
  }

  return IUP_DEFAULT;
}

/* Call the user callback associated with the event of a cell enter the focus */

void iMatrixCallEntercellCb(Ihandle* ih)
{
  IFnii cb = (IFnii)IupGetCallback(ih, "ENTERITEM_CB");
  if(cb)
  {
    if((ih->data->col.num == 0) || (ih->data->lin.num == 0))
      return;
    cb(ih, ih->data->lin.active+1, ih->data->col.active+1);
  }
}

int iMatrixCallEditionCbLinCol(Ihandle* ih, int lin, int col, int modo)
{
  int rc = IUP_DEFAULT;
  IFniii cb = (IFniii)IupGetCallback(ih, "EDITION_CB");
  if(cb)
  {
    rc = cb(ih, lin, col, modo);
  }

  return rc;
}

/* Realize the parse of a text, returning the number of lines occupied 
   (the lines end with an '\n') and the height occupied by it
   -> text : text to analysis
   -> totalh  : number of pixels to draw all the lines
   -> lineh   : number of pixels to draw a line
   -> spacing : number of pixels to the spacing between lines
*/
int iMatrixTextHeight(Ihandle* ih, char *text, int* totalh, int* lineh, int* spacing)
{
  int numlines = 1;
  char *c;
  int charwidth, charheight;

  if(spacing) *spacing = 0;
  if(totalh ) *totalh  = 0;
  if(lineh  ) *lineh   = 0;

  if(!text || !*text)
    return 0;

  c = text;
  while(c && *c)
  {
    c = strchr(c, '\n');
    if(c)
    {
      numlines++;
      c++;
    }
  }

  iupdrvFontGetCharSize(ih, &charwidth, &charheight);

  if(spacing) *spacing = IMATRIX_DECOR_Y/2;
  if(lineh  ) *lineh   = charheight;
  if(totalh ) *totalh  = charheight * numlines + (IMATRIX_DECOR_Y/2) * (numlines-1);

  return numlines;
}

/* Realize the parse of a text, returning the width occupied by it
   (the lines end with an '\n')
   -> text : text to analysis
   -> width  : number of pixels to draw, completely, the largest text line
*/
void iMatrixTextWidth(Ihandle* ih, char *text, int* width)
{
  char *p, *q, *newtext;
  int w, maxwidth=0;

  if(!text || !*text)
    return;

  newtext = q = (char*)iupStrDup(text);

  while(*q)
  {
    p = strchr(q, '\n');
    if(p) *p = 0;

    w = iupdrvFontGetStringWidth(ih, q);
    if(w > maxwidth)
      maxwidth = w;

    if(p)
    {
      *p = '\n';
      q = p+1;
    }
    else
      break;
  }

  *width = maxwidth;
  
  if(newtext)
    free(newtext);
}
