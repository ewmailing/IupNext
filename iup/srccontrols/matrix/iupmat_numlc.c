/** \file
 * \brief iupmatrix control
 * change number of columns or lines
 *
 * See Copyright Notice in "iup.h"
 */

/**************************************************************************/
/*  Functions to change the number of lines and columns of the matrix,    */
/*  after it has been created.                                            */
/**************************************************************************/

#include <stdio.h>
#include <stdlib.h>  /* malloc, realloc */
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
#include "iup_stdcontrols.h"
#include "iup_controls.h"
#include "iup_cdutil.h"

#include "iupmat_def.h"
#include "iupmat_cd.h"
#include "iupmat_draw.h"
#include "iupmat_scroll.h"
#include "iupmat_focus.h"
#include "iupmat_aux.h"
#include "iupmat_key.h"
#include "iupmat_mark.h"
#include "iupmat_getset.h"
#include "iupmat_edit.h"
#include "iupmat_mem.h"


#define IMAT_INSERT    0     /* Insert a line or column */
#define IMAT_REMOVE    1     /* Remove a line or column */


/**************************************************************************/
/* Private functions                                                      */
/**************************************************************************/

/* Change the number of lines of the matrix and realloc memory, if necessary. 
   It also changes the number of visible lines to show the new size.
   -> num : new number of lines of the matrix.
*/
static void iMatrixNumLCChangeNumLines(Ihandle* ih, int num)
{
  int oldnl = ih->data->lin.num;
  int wt, i;

  ih->data->lin.num = num;

  /* the largest title line may have been erased... */
  wt = iupMatrixAuxGetTitlelineSize(ih);
  if(wt != ih->data->col.titlewh)
  {
    ih->data->col.size = (ih->data->XmaxC+1) - wt;
    ih->data->col.titlewh = wt;
    iupMatrixAuxGetLastWidth(ih, IMAT_MAT_COL);
  }

  if(num > oldnl) /* increasing the matrix */
  {
    iupMatrixMemRealocLines(ih, num-oldnl, oldnl);

    /* recalculating the total height of the matrix */
    for(i = oldnl; i < ih->data->lin.num; i++)
    {
      ih->data->lin.wh[i] = iupMatrixAuxGetLineHeight(ih, i);
/*
      if(ih->data->lin.wh[i] > ih->data->lin.size)  // This condition was changed the height...
        ih->data->lin.wh[i] = ih->data->lin.size;   // ih->data->lin.size was always zero or a negative value
*/
      ih->data->lin.totalwh += ih->data->lin.wh[i];
    }
  }
  else /* decreasing the matrix */
  {
    /* if there is not any visible line, change the first line */
    if(ih->data->lin.first > ih->data->lin.num-1)
    {
      ih->data->lin.first = 0;
      
      if(ih->data->lin.active > ih->data->lin.num-1)
        ih->data->lin.active = 0;
    }
    /* recalculating the total height of the matrix */
    for(i = oldnl-1; i >= ih->data->lin.num; i--)
    {
      ih->data->lin.totalwh -= ih->data->lin.wh[i];
    }
  }
  iupMatrixAuxGetLastWidth(ih, IMAT_MAT_LIN);
}

/* Change the number of columns of the matrix and realloc memory, if necessary. 
   It also changes the number of visible columns to show the new size.
   -> num : new number of columns of the matrix.
*/
static void iMatrixNumLCChangeNumCols(Ihandle* ih, int num)
{
  int oldnc = ih->data->col.num;
  int i;

  ih->data->col.num = num;

  if(num > oldnc)
  {
    /* increasing the matrix */
    iupMatrixMemRealocColumns(ih, num-oldnc, oldnc);

    /* recalculating the total width of the matrix */
    for(i = oldnc; i < ih->data->col.num; i++)
    {
      ih->data->col.wh[i] = iupMatrixAuxGetColumnWidth(ih, i);
/*
      if(ih->data->col.wh[i] > ih->data->col.size)  // This condition was changed the width...
        ih->data->col.wh[i] = ih->data->col.size;   // ih->data->col.size was always zero or a negative value
*/
      ih->data->col.totalwh += ih->data->col.wh[i];
    }
  } 
  else  /* decreasing the matrix */
  {
    /* if there is not any visible column, change the first column */
    if(ih->data->col.first > ih->data->col.num-1)
    {
      ih->data->col.first = 0;
      if(ih->data->col.active > ih->data->col.num-1)
        ih->data->col.active = 0;
    }

    /* recalculating the total width of the matrix */
    for(i = oldnc-1; i >= ih->data->col.num; i--)
    {
      ih->data->col.totalwh -= ih->data->col.wh[i];
    }
  }
  iupMatrixAuxGetLastWidth(ih, IMAT_MAT_COL);
}

/* When the matrix has its number of lines increased, and these lines are
   inserted at end of it, it is necessary to search for new attributes that
   must be put in the cells
   -> oldnumlin - number of lines that were in the matrix.
*/
static void iMatrixNumLCGetNewLineAttributes(Ihandle* ih, int oldnumlin)
{
  int i, j;
  char* value;
  char* attr = iupStrGetMemory(100);

  if(ih->data->valcb)
    return;

  for(i = oldnumlin; i < ih->data->lin.num; i++)
  {
    for(j = 0; j < ih->data->col.num; j++)
    {
      sprintf(attr, "%d:%d", i+1, j+1);
      value = (char*)iupAttribGetStr(ih, attr);
      if(value && *value)
      {
        iupMatrixMemAlocCell(ih, i, j, strlen(value));
        strcpy(ih->data->v[i][j].value, value);
      }
    }
  }
}

/* When the matrix has its columns of lines increased, and these columns are
   inserted at end of it, it is necessary to search for new attributes that
   must be put in the cells
   -> oldnumcol - number of columns that were in the matrix.
*/
static void iMatrixNumLCGetNewColumnAttributes(Ihandle* ih, int oldnumcol)
{
  int i, j;
  char* value;
  char* attr = iupStrGetMemory(100);

  if(ih->data->valcb)
    return;

  for(i = 0; i < ih->data->lin.num; i++)
  {
    for(j = oldnumcol; j < ih->data->col.num; j++) /* Shouldn't this be <= ? */  
    {
      sprintf(attr, "%d:%d", i+1, j+1);
      value = (char*)iupAttribGetStr(ih, attr);
      if(value && *value)
      {
        iupMatrixMemAlocCell(ih, i, j, strlen(value));
        strcpy(ih->data->v[i][j].value, value);
      }
    }
  }
}

/* Change the title attributes and also the cell values, of each line,
   to show what lines were inserted or removed in the middle of matrix.
   It is necessary to change large lines, instead of base line.
   Note that if this function is called, mat_numlin(ih) already shows
   the new number of lines of the matrix. The space already was allocated
   for the new lines.
   -> base: when this is an insertion, points to the next line where insertions
      will be performed. Note that its value is 1 when it inserts after the first
      line.
      If this is a deletion, points to the first line where will be deleted.
      Note that its value is 1 when it deletes the line 1;
   -> numlin : number of lines to be inserted/ removed;
   -> modo : IMAT_INSERT or IMAT_REMOVE.
*/
static void iMatrixNumLCChangeLineAttributes(Ihandle* ih, int base, int numlin, int modo)
{
  int   i;
  char* aux = iupStrGetMemory(10);
  char* v;
  ImatCell**  tmp = NULL;

  /* Allocate space to store, temporarily, pointers for ImatCell struct,
     allocated in ChangeNumLin. It is necessary to do not
     lose the allocated space, during the move of attributes.
  */
  if(!ih->data->valcb)
    tmp = (ImatCell**) malloc(numlin*sizeof(ImatCell*));

  if(modo == IMAT_INSERT)
  {
    /* Inserting lines */
    if(!ih->data->valcb)
    {
      for(i = 0; i < numlin; i++)
        tmp[i] = ih->data->v[ih->data->lin.num-numlin+i]; /* store the allocated region to attribute new lines */
    }

    /* Moving line attributes down */
    for(i = ih->data->lin.num-1-numlin; i >= base; i--)
    {
      sprintf(aux, IMAT_TITLE_LIN, i+1);        /* Get the current line title and put in the right place */
      v = iupAttribGetStr(ih, aux);                /* The attribute is "i+1:0", because the attributes      */
      sprintf(aux, IMAT_TITLE_LIN, i+1+numlin); /* starts in 1 and internally, starts in zero.           */
      iupAttribStoreStr(ih, aux, v);

      if(!ih->data->valcb)
        ih->data->v[i+numlin] = ih->data->v[i];

      /* Move the indication if the line is or not marked/inactive */
      ih->data->lin.marked[i+numlin]   = ih->data->lin.marked[i];
      ih->data->lin.inactive[i+numlin] = ih->data->lin.inactive[i];
    }

    for(i = 0; i < numlin; i++)
    {
      /* Set the titles of the new lines as "" */
      sprintf(aux, IMAT_TITLE_LIN, base+1+i);
      iupAttribSetStr(ih, aux, "");

      /* Restore the allocated region in the right position */
      if(!ih->data->valcb)
        ih->data->v[base+i] = tmp[i];

      /* New lines are not marked */
      ih->data->lin.marked[base+i]   = 0;
      ih->data->lin.inactive[base+i] = 0;
    }

    /* Adjust the foreground and background color of lines/cells */
    for(i = 0; i < 6; i++)
    {
      char string[6][100] = { "BGCOLOR%d:*",
                              "FGCOLOR%d:*",
                              "FONT%d:*",
                              "BGCOLOR%d:%d",
                              "FGCOLOR%d:%d",
                              "FONT%d:%d"  };
      char* attr = iupStrGetMemory(100);
      char* value;
      int j, k;

      /* Update line/cell attributes after last line added */
      for(j = ih->data->lin.num; j > base+numlin; j--)
      {
        /* Update the line attributes */
        if(i < 3)
        {
          sprintf(attr, string[i], j-numlin);
          value = iupAttribGetStr(ih, attr);
          sprintf(attr, string[i], j);
          iupAttribStoreStr(ih, attr, value);
        }

        /* Update each cell attribute */
        else for(k=0;k<=ih->data->col.num;k++)
        {
          sprintf(attr, string[i], j-numlin, k);
          value = iupAttribGetStr(ih, attr);
          sprintf(attr, string[i], j, k);
          iupAttribStoreStr(ih, attr, value);
        }
      }

      /* The line attributes added are the default */
      for(j = base; j < base+numlin; j++)
      {
        if(i < 2)
        {
          sprintf(attr, string[i], j+1);
          iupAttribSetStr(ih, attr, NULL);
        }
        else for(k = 0; k <= ih->data->col.num; k++)
        {
          sprintf(attr, string[i], j+1, k);
          iupAttribSetStr(ih, attr, NULL);
        }
      }
    }

    if(ih->data->lin.active >= base) /* If the focus is after line where the new line was inserted, plus "numlin" and focus line */
      ih->data->lin.active += numlin;
  }
  else /* modo == IMAT_REMOVE */
  {
    if(!ih->data->valcb)
    {
      for(i = 0; i < numlin; i++)
        tmp[i] = ih->data->v[base-1+i]; /* store allocated region to attribute lines at the end, do not used */
    }

    for(i = base-1; i < ih->data->lin.num; i++)
    {
      sprintf(aux, IMAT_TITLE_LIN, i+1+numlin);  /* Get the next line title */
      v = iupAttribGetStr(ih, aux);
      sprintf(aux, IMAT_TITLE_LIN, i+1);         /* and put in the current line */
      iupAttribStoreStr(ih,aux,v);

      if(!ih->data->valcb)
        ih->data->v[i] = ih->data->v[i+numlin];

      /* Move the indication if the line is or not marked/inactive */
      ih->data->lin.marked[i]   = ih->data->lin.marked[i+numlin];
      ih->data->lin.inactive[i] = ih->data->lin.inactive[i+numlin];
    }

    for(i = 0; i < numlin; i++)
    {
      /* Restore the allocated region in the right position */
      if(!ih->data->valcb)
        ih->data->v[ih->data->lin.num+i] = tmp[i];

      /* Mark the erased lines as not marked */
      ih->data->lin.marked[ih->data->lin.num+i]   = 0;
      ih->data->lin.inactive[ih->data->lin.num+i] = 0;
    }

    /* Adjust the foreground and background color of lines/cells */
    for(i = 0; i < 6; i++)
    {
      char string[6][100] = { "BGCOLOR%d:*",
                              "FGCOLOR%d:*",
                              "FONT%d:*",
                              "BGCOLOR%d:%d",
                              "FGCOLOR%d:%d",
                              "FONT%d:%d"  };
      char* attr = iupStrGetMemory(100);
      char* value;
      int j, k;

      /* Update line/cell attributes after last line erased */
      for(j = base-1; j < ih->data->lin.num; j++)
      {
        /* Update the line attributes */     
        if(i < 3)      
        {
          sprintf(attr, string[i], j+1+numlin);
          value = iupAttribGetStr(ih, attr);
          sprintf(attr, string[i], j+1);
          iupAttribStoreStr(ih, attr, value);
        }

        /* Update each cell attribute */
        else for(k = 0; k <= ih->data->col.num; k++) 
        {
          sprintf(attr, string[i], j+1+numlin, k);
          value = iupAttribGetStr(ih, attr);
          sprintf(attr, string[i], j+1, k);
          iupAttribStoreStr(ih, attr, value);
        }
      }
    }

    if(ih->data->lin.active >= base) /* If the focus is after line that was erased, minus "numlin" and focus line */
      ih->data->lin.active-=numlin;
  }

  {
    /* Adjust the width of the title column, and the visible size of the matrix */
    int wt  = iupMatrixAuxGetTitlelineSize(ih);
    if(wt != ih->data->col.titlewh)
    {
      ih->data->col.size = (ih->data->XmaxC+1) - wt;
      ih->data->col.titlewh = wt;
      iupMatrixAuxGetLastWidth(ih, IMAT_MAT_COL);
    }
  }

  if(tmp)
    free(tmp);
}

/* Change the title attributes and also the cell values, of each column,
   to show what columns were inserted or removed in the middle of matrix.
   It is necessary to change large columns, instead of base column.
   Note that if this function is called, mat_numcol(ih) already shows
   the new number of columns of the matrix. The space already was allocated
   for the new columns.
   -> base: when this is an insertion, points to the next column where insertions
      will be performed. Note that its value is 1 when it inserts after the first
      column.
      If this is a deletion, points to the first column where will be deleted.
      Note that its value is 1 when it deletes the line 1;
   -> numcol : number of column to be inserted/ removed;
   -> modo : IMAT_INSERT or IMAT_REMOVE.
*/
static void iMatrixNumLCChangeColumnAttributes(Ihandle* ih, int base, int numcol, int modo)
{
  int i, j, k;
  char*  aux = iupStrGetMemory(10);
  char*  v;
  ImatCell*    tmp = NULL;
  int*   newwidth;

  /* Allocate space to store, temporarily, pointers for ImatCell structs,
     allocated in ChangeNumCol. It is necessary to do not
     lose the allocated space, during the move of attributes.
  */
  if(!ih->data->valcb)
    tmp = (ImatCell*) malloc(numcol * sizeof(ImatCell));
  
  newwidth = (int*) malloc(numcol * sizeof(int));

  if(modo == IMAT_INSERT)
  {
    for(i = 0; i < numcol; i++)
      newwidth[i] = ih->data->col.wh[ih->data->col.num-numcol+i]; /* store width, calculated previously */

    /* Move column titles and widths */
    for(i = ih->data->col.num-1-numcol; i >= base; i--)
    {
      sprintf(aux, IMAT_TITLE_COL, i+1);        /* Get the current column title and put in the right place */
      v = iupAttribGetStr(ih, aux);                /* The attribute is "0:i+1", because the attributes        */
      sprintf(aux, IMAT_TITLE_COL, i+1+numcol); /* starts in 1 and internally, starts in zero.             */
      iupAttribStoreStr(ih, aux, v);

      ih->data->col.wh[i+numcol] = ih->data->col.wh[i];   /* Reorder the width of columns */

      /* Move the indication if the column is or not marked/inactive */
      ih->data->col.marked[i+numcol]   = ih->data->col.marked[i];
      ih->data->col.inactive[i+numcol] = ih->data->col.inactive[i];
    }

    /* Restore stored widths and set new titles */
    for(i = 0; i < numcol; i++)
    {
      sprintf(aux, IMAT_TITLE_COL, base+1+i);
      iupAttribSetStr(ih, aux, "");

      ih->data->col.wh[base+i] = newwidth[i];

      /* New columns do not marked/inactive */
      ih->data->col.marked[base+i]   = 0;
      ih->data->col.inactive[base+i] = 0;
    }

    /* Reorder the cell values */
    if(!ih->data->valcb)
    {
      for(i = 0; i < ih->data->lin.num; i++)
      {
        for(j = 0; j < numcol; j++)
          tmp[j] = ih->data->v[i][ih->data->col.num-numcol+j];  /* store allocated region to attribute the new column */

        for(j = ih->data->col.num-1-numcol; j >= base; j--)   /* Move old values */
        {
          ih->data->v[i][j+numcol] = ih->data->v[i][j];
        }
        for(j = 0; j < numcol; j++)                    /* Reallocate stored region */
          ih->data->v[i][base+j]=tmp[j];
      }
    }

    /* Adjust the foreground and background color of columns */
    for(i = 0; i < 7; i++)
    {
      char string[7][100] = { "ALIGNMENT%d",
                              "BGCOLOR*:%d",
                              "FGCOLOR*:%d",
                              "FONT*:%d",
                              "BGCOLOR%d:%d",
                              "FGCOLOR%d:%d",
                              "FONT%d:%d"  };
      char* attr = iupStrGetMemory(100);
      char* value;

      /* Update column/cell attributes in the right side of the last column added */
      for(j = ih->data->col.num; j > base+numcol; j--)
      {
        /* Update the column attributes */
        if(i < 4)
        {
          sprintf(attr, string[i], j-numcol);
          value = iupAttribGetStr(ih, attr);
          sprintf(attr,string[i],j);
          iupAttribStoreStr(ih, attr, value);
        }

        /* Update the cell attributes */
        else for(k = 0; k <= ih->data->lin.num; k++)
        {
          sprintf(attr, string[i], k, j-numcol);
          value = iupAttribGetStr(ih, attr);
          sprintf(attr, string[i], k, j);
          iupAttribStoreStr(ih, attr, value);
        }
      }

      /* The column/cell attributes added are the default */
      for(j = base; j < base+numcol; j++)
      {
        if(i < 3)
        {
          sprintf(attr, string[i], j+1);
          iupAttribSetStr(ih, attr, NULL);
        }
        else for(k = 0; k <= ih->data->lin.num; k++)
        {
          sprintf(attr, string[i], k, j+1);
          iupAttribSetStr(ih, attr, NULL);
        }
      }
    }

    if(ih->data->col.active >= base) /* If the focus is after column where the new column was inserted, plus "numcol" and focus column */
      ih->data->col.active += numcol;
  }
  else /* modo == IMAT_REMOVE */
  {
    /* Move column titles and widths */
    for(i = base-1; i < ih->data->col.num; i++)
    {
      sprintf(aux, IMAT_TITLE_COL, i+1+numcol);  /* Get the next column title */     
      v = iupAttribGetStr(ih, aux);                                                   
      sprintf(aux, IMAT_TITLE_COL, i+1);         /* and put in the current line */ 
      iupAttribStoreStr(ih, aux, v);

      ih->data->col.wh[i] = ih->data->col.wh[i+numcol];  /* Reorder the width of columns */

      /* Move the indication if the column is or not marked/inactive */
      ih->data->col.marked[i]   = ih->data->col.marked[i+numcol];
      ih->data->col.inactive[i] = ih->data->col.inactive[i+numcol];
    }

    if(!ih->data->valcb)
    {
      for(i = 0; i < ih->data->lin.num; i++)       /* Reorder the cell values */
      {
        for(j = 0; j < numcol; j++)
          tmp[j] = ih->data->v[i][base-1+j];  /* store allocated region to attribute columns at the end */

        for(j = base-1; j < ih->data->col.num; j++)
          ih->data->v[i][j] = ih->data->v[i][j+numcol];

        for(j = 0; j < numcol; j++)         /* Reallocated stored region */
          ih->data->v[i][ih->data->col.num+j]=tmp[j];
      }
    }

    /* Mark all the erased columns as not marked */
    for(j = 0; j < numcol; j++)
    {
      ih->data->col.marked[ih->data->col.num+j]   = 0;
      ih->data->col.inactive[ih->data->col.num+j] = 0;
    }

    /* Adjust the foreground and background color of columns */
    for(i = 0; i < 7; i++)
    {
      char string[7][100] = { "ALIGNMENT%d",
                              "BGCOLOR*:%d",
                              "FGCOLOR*:%d",
                              "FONT*:%d",
                              "BGCOLOR%d:%d",
                              "FGCOLOR%d:%d",
                              "FONT%d:%d"  };
      char* attr = iupStrGetMemory(100);
      char* value;

      /* Update column/cell attributes in the right side of the last column erased */
      for(j = base-1; j < ih->data->col.num; j++)
      {
        /* Update the column attributes */
        if(i < 4)
        {
          sprintf(attr, string[i], j+1+numcol);
          value = iupAttribGetStr(ih, attr);
          sprintf(attr, string[i], j+1);
          iupAttribStoreStr(ih, attr, value);
        }

        /* Update the cell attributes */
        else for(k = 0; k <= ih->data->lin.num; k++)
        {
          sprintf(attr, string[i], k, j+1+numcol);
          value = iupAttribGetStr(ih, attr);
          sprintf(attr, string[i], k, j+1);
          iupAttribStoreStr(ih, attr, value);
        }
      }
    }

    if(ih->data->col.active >= base) /* If the focus is after column that was erased, minus "numcol" and focus column */
      ih->data->col.active -= numcol;
  }

  iupMatrixAuxGetLastWidth(ih, IMAT_MAT_COL);
  
  if(tmp)
    free(tmp);
  
  free(newwidth);
}


/**************************************************************************/
/* Exported functions                                                     */
/**************************************************************************/

/* Return the number of lines that the matrix has. */
char* iupMatrixNumLCGetNumLin(Ihandle* ih)
{
  char* num = iupStrGetMemory(100);
  sprintf(num, "%d", ih->data->lin.num);
  return num;
}

/* Return the number of columns that the matrix has. */
char* iupMatrixNumLCGetNumCol(Ihandle* ih)
{
  char* num = iupStrGetMemory(100);
  sprintf(num, "%d", ih->data->col.num);
  return num;
}

/* Insert one or more lines in the matrix.
   -> v : string that contains the line after it will be inserted other lines,
          and perhaps the number of lines to be inserted.
          There is two formats: 
          a) "%d-%d" insert after the line using the first number, and the 
              number of lines.
          b) "%d" insert after the line using the number.
*/
void iupMatrixNumLCAddLin(Ihandle* ih, const char* v)
{
  int base = 0, oldnl = ih->data->lin.num, numlin;
  int visible = IupGetInt(ih, "VISIBLE");
  int err;
  int ret;

  if(!v)
    return;

  ret = sscanf(v, "%d-%d", &base, &numlin);

  if(base < 0 || base > oldnl)  /* Out of the valid limits for the base */
    return;

  if(ret != 2) /* If it was identified one number, uses (b) format */
    numlin = 1;

  /* leave of the edit mode */
  iupMatrixEditCheckHidden(ih);

  IsCanvasSet(ih, err);

  if(visible && err == CD_OK)
    IupSetAttribute(ih, "VISIBLE", "NO");

  iMatrixNumLCChangeNumLines(ih, oldnl+numlin);

  if(base != oldnl)  /* If there is not insertion after last line... */
    iMatrixNumLCChangeLineAttributes(ih, base, numlin, IMAT_INSERT);  /* change attributes and 'base' line values until the end... */
  else
    iMatrixNumLCGetNewLineAttributes(ih, oldnl);

  if(visible && err == CD_OK)
  {
    iupMatrixSetSbV(ih);
    iupMatrixSetSbH(ih);
    IupSetAttribute(ih, "VISIBLE", "YES");
  }
}

/* Delete one or more lines of the matrix.
   -> v : string that contains the line will be deleted,
          and perhaps the number of lines to be deleted.
          There is two formats: 
          a) "%d-%d" delete from the line using the first number, and the 
              number of lines.
          b) "%d" delete the line using the number.
*/
void iupMatrixNumLCDelLin(Ihandle* ih, const char* v)
{
  int base, oldnl=ih->data->lin.num, numlin;
  int visible = IupGetInt(ih, "VISIBLE");
  int err;
  int ret;

  if(!v)
    return;

  ret = sscanf(v, "%d-%d", &base, &numlin);

  if(base <= 0 || base > oldnl)   /* Out of the valid limits for the base */
    return;

  if(ret != 2) /* If it was identifyed one number, uses (b) format */
    numlin = 1;

  /* Verify if the last line to be deleted is inside the spreadsheet.
  If no, change numlin to delete until the last line.
  */
  if(base+numlin-1 > oldnl)
    numlin = oldnl-base+1;

  /* leave of the edit mode */
  iupMatrixEditCheckHidden(ih);

  if(ih->data->lin.active+1 >= base && ih->data->lin.active <= base+numlin-2)
  {
    /* If the first line is not cleared, move the focus to the
       first cell before erased cells */
    if(base != 1)
    {      
      ih->data->lin.active = base - 2;
    }
    /* If no, move the focus to the first cell after erased cells */
    else
    {
      ih->data->lin.active = base + numlin - 1;
    }      

    /* Mark the focused cell */
    iupMatrixMarkShow(ih, 0, ih->data->lin.active, ih->data->col.active, ih->data->lin.active, ih->data->col.active);
  }

  IsCanvasSet(ih, err);

  if(visible && err == CD_OK)
    IupSetAttribute(ih, "VISIBLE", "NO");

  iMatrixNumLCChangeNumLines(ih, oldnl-numlin);

  if(base != oldnl)  /* If there is not deletion the last line... */
    iMatrixNumLCChangeLineAttributes(ih, base, numlin, IMAT_REMOVE);  /* change attributes and 'base' line values until the end... */
  else
    iMatrixNumLCGetNewLineAttributes(ih, oldnl);

  if(visible && err == CD_OK)
  {
    iupMatrixSetSbV(ih);
    iupMatrixSetSbH(ih);
    IupSetAttribute(ih, "VISIBLE", "YES");
  }
}

/* Insert one or more columns in the matrix.
   -> v : string that contains the column after it will be inserted other columns,
          and perhaps the number of columns to be inserted.
          There is two formats: 
          a) "%d-%d" insert after the column using the first number, and the 
              number of columns.
          b) "%d" insert after the column using the number.
*/
void iupMatrixNumLCAddCol(Ihandle* ih, const char* v)
{
  int base = 0, oldnc = ih->data->col.num, numcol;
  int visible = IupGetInt(ih, "VISIBLE");
  int err;
  int ret;

  if(!v)
    return;

  ret = sscanf(v, "%d-%d", &base, &numcol);

  if(base < 0 || base > oldnc)  /* Out of the valid limits for the base */
    return;

  if(ret != 2)  /* If it was identified one number, uses (b) format */
    numcol = 1;

  /* leave of the edit mode */
  iupMatrixEditCheckHidden(ih);

  IsCanvasSet(ih, err);
  if(visible && err == CD_OK)
    IupSetAttribute(ih, "VISIBLE", "NO");

  iMatrixNumLCChangeNumCols(ih, oldnc+numcol);

  if(base != oldnc)  /* If there is not insertion last column... */
    iMatrixNumLCChangeColumnAttributes(ih, base, numcol, IMAT_INSERT);  /* change attributes and 'base' column values until the end... */
  else
    iMatrixNumLCGetNewColumnAttributes(ih, oldnc);

  if(visible && err == CD_OK)
  {
    iupMatrixSetSbV(ih);
    iupMatrixSetSbH(ih);
    IupSetAttribute(ih, "VISIBLE", "YES");
  }
}

/* Delete one or more columns of the matrix.
   -> v : string that contains the column will be deleted,
          and perhaps the number of columns to be deleted.
          There is two formats: 
          a) "%d-%d" delete from the column using the first number, and the 
              number of columns.
          b) "%d" delete the column using the number.
*/
void iupMatrixNumLCDelCol(Ihandle* ih, const char* v)
{
  int base = 0, oldnc = ih->data->col.num, numcol;
  int visible = IupGetInt(ih, "VISIBLE");
  int err;
  int ret;

  if(!v)
    return;

  ret = sscanf(v, "%d-%d", &base, &numcol);

  if(base <= 0 || base > oldnc)   /* Out of the valid limits for the base */
    return;

  if(ret != 2)  /* If it was identified one number, uses (b) format */
    numcol = 1;

  /* Verify if the last column to be deleted is inside the spreadsheet.
  If no, change numcol to delete until the last line.
  */
  if(base+numcol-1 > oldnc)
    numcol = oldnc-base+1;

  /* leave of the edit mode */
  iupMatrixEditCheckHidden(ih);

  if(ih->data->col.active+1 >= base && ih->data->col.active <= base+numcol-2)
  {
    /* If the first column is not cleared, move the focus to the
       first cell before erased cells */
    if(base != 1)
    {      
      ih->data->col.active = base - 2;
    }
    /* If no, move the focus to the first cell after erased cells */
    else
    {
      ih->data->col.active = base + numcol - 1;
    }

    /* Mark the focused cell */
    iupMatrixMarkShow(ih, 0, ih->data->lin.active, ih->data->col.active, ih->data->lin.active, ih->data->col.active);
  }

  IsCanvasSet(ih, err);
  if(visible && err == CD_OK)
    IupSetAttribute(ih, "VISIBLE", "NO");

  iMatrixNumLCChangeNumCols(ih, oldnc-numcol);

  if(base != oldnc)  /* If there is not deletion the last column... */
    iMatrixNumLCChangeColumnAttributes(ih, base, numcol, IMAT_REMOVE);  /* change attributes and 'base' column values until the end... */
  else
    iMatrixNumLCGetNewColumnAttributes(ih, oldnc);

  if(visible && err == CD_OK)
  {
    iupMatrixSetSbV(ih);
    iupMatrixSetSbH(ih);
    IupSetAttribute(ih, "VISIBLE", "YES");
  }
}

/* Change the number of lines of the matrix, cut and put lines at end */
void iupMatrixNumLCNumLin(Ihandle* ih, const char* v)
{
  int num = 0, oldnl = ih->data->lin.num;
  int visible = IupGetInt(ih, "VISIBLE");
  int err;

  if(iupStrToInt(v, &num))
  {
    IsCanvasSet(ih, err);
    if(visible && err == CD_OK)
      IupSetAttribute(ih, "VISIBLE", "NO");

    iMatrixNumLCChangeNumLines(ih, num);
    iMatrixNumLCGetNewLineAttributes(ih, oldnl);

    if(visible && err == CD_OK)
    {
      iupMatrixSetSbV(ih);
      iupMatrixSetSbH(ih);
      IupSetAttribute(ih, "VISIBLE", "YES");
    }
  }
}

/* Change the number of columns of the matrix, cut and put lines at end */
void iupMatrixNumLCNumCol(Ihandle* ih, const char* v)
{
  int num = 0, oldnc = ih->data->col.num;
  int visible = IupGetInt(ih, "VISIBLE");
  int err;

  if(iupStrToInt(v, &num))
  {
    IsCanvasSet(ih, err);
    if(visible && err == CD_OK)
      IupSetAttribute(ih, "VISIBLE", "NO");

    iMatrixNumLCChangeNumCols(ih, num);
    iMatrixNumLCGetNewColumnAttributes(ih, oldnc); /* Unnecessary call when adding columns? */

    if(visible && err == CD_OK)
    {
      iupMatrixSetSbV(ih);
      iupMatrixSetSbH(ih);
      IupSetAttribute(ih, "VISIBLE", "YES");
    }
  }
}
