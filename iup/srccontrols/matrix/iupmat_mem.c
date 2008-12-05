/** \file
 * \brief iupmatrix control
 * memory allocation
 *
 * See Copyright Notice in iup.h
 */

/**************************************************************************/
/* Functions to allocate memory                                           */
/**************************************************************************/

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
#include "iup_stdcontrols.h"
#include "iup_controls.h"
#include "iup_cdutil.h"

#include "iupmat_def.h"
#include "iupmat_mem.h"


#define IMAT_BLOCKSIZE 25


/**************************************************************************/
/* Exported functions                                                     */
/**************************************************************************/

/* Allocate space to the cells, when the matrix is created. It also allocate
   space for the vectors of: column widths, line heights and line and selection
   of lines and columns. Initialize all the cells with "".
*/
void iupMatrixMemAloc(Ihandle* ih)
{
  int i;

  /* ih->data->v is a pointer for an array with ih->data->lin.num
     pointers (number of lines). Each one points for a vector with 
     ih->data->col.num cells (number of columns).
  */

  if(ih->data->lin.num == 0)
    ih->data->lin.numaloc = 5;
  else
    ih->data->lin.numaloc = ih->data->lin.num;

  if(ih->data->col.num == 0)
    ih->data->col.numaloc = 5;
  else
    ih->data->col.numaloc = ih->data->col.num;

  if(!ih->data->valcb)
  {
    ih->data->v = (ImatCell**)calloc(ih->data->lin.numaloc, sizeof(ImatCell*));
    for(i = 0; i < ih->data->lin.numaloc; i++)
      ih->data->v[i] = (ImatCell*)calloc(ih->data->col.numaloc, sizeof(ImatCell));
  }

  ih->data->col.wh    = (int*)calloc(ih->data->col.numaloc, sizeof(int));
  ih->data->lin.wh    = (int*)calloc(ih->data->lin.numaloc, sizeof(int));
  ih->data->col.marked   = (char*)calloc(ih->data->col.numaloc, sizeof(char));
  ih->data->lin.marked   = (char*)calloc(ih->data->lin.numaloc, sizeof(char));
  ih->data->col.inactive = (char*)calloc(ih->data->col.numaloc, sizeof(char));
  ih->data->lin.inactive = (char*)calloc(ih->data->lin.numaloc, sizeof(char));
}

/* Reallocate function for lines (add lines in the matrix). Initialize all
   the new cells with "".
   -> nlines : number of lines to be allocate
   -> nl     : old number of lines
*/
void iupMatrixMemRealocLines(Ihandle* ih,int nlines, int nl)
{
  int i, j;

  /* If it doesn't have enough lines allocated, then allocate more space */
  if(nl + nlines > ih->data->lin.numaloc)
  {
    ih->data->lin.numaloc = nl + nlines;

    if(!ih->data->valcb)
    {
      ih->data->v = (ImatCell**)realloc(ih->data->v, ih->data->lin.numaloc*sizeof(ImatCell*));
      for(i = 0; i < nlines; i++)
        ih->data->v[nl+i] = (ImatCell*)calloc(ih->data->col.numaloc, sizeof(ImatCell));
    }

    ih->data->lin.wh = (int*)realloc(ih->data->lin.wh, ih->data->lin.numaloc*sizeof(int));

    ih->data->lin.marked   = (char*)realloc(ih->data->lin.marked, ih->data->lin.numaloc*sizeof(char));
    ih->data->lin.inactive = (char*)realloc(ih->data->lin.inactive, ih->data->lin.numaloc*sizeof(char));
    for(i = 0; i < nlines; i++)
    {
      ih->data->lin.marked[nl+i]   = 0;
      ih->data->lin.inactive[nl+i] = 0;
    }
  }
  else if(!ih->data->valcb)
  {
    /* Initialize new cells to be used with NULL */
    for(i = 0; i < nlines; i++)
      for(j = 0; j < ih->data->col.numaloc; j++)
      {
       if(ih->data->v[nl+i][j].nba)
          ih->data->v[nl+i][j].value[0] = 0;  /* empty string, can not put NULL because the pointer will be lost */
        ih->data->v[nl+i][j].mark = 0;
      }
  }
}

/* Reallocate function for columns (add columns in the matrix). For all the lines,
   reallocate the number of columns and initialize the new cells with the empty
   string. It also reallocate the vector that stores the column widths.
   -> ncols : number of columns to be allocate
   -> nc    : old number of columns
*/
void iupMatrixMemRealocColumns(Ihandle* ih, int ncols, int nc)
{
  int i, j;

  /* If it doesn't have enough columns allocated, then allocate more space */
  if(nc + ncols > ih->data->col.numaloc)
  {
    ih->data->col.numaloc = nc + ncols;
    if(!ih->data->valcb)
    {
      for(i = 0; i < ih->data->lin.numaloc; i++)
      {
        ih->data->v[i] = (ImatCell*)realloc(ih->data->v[i], ih->data->col.numaloc*sizeof(ImatCell));
        for(j = 0; j < ncols; j++)
        {
          ih->data->v[i][nc+j].nba   = 0;
          ih->data->v[i][nc+j].value = NULL;
          ih->data->v[i][nc+j].mark  = 0;
        }
      }
    }
    ih->data->col.wh = (int*)realloc(ih->data->col.wh, ih->data->col.numaloc*sizeof(int));
    ih->data->col.marked   = (char*)realloc(ih->data->col.marked, ih->data->col.numaloc*sizeof(char));
    ih->data->col.inactive = (char*)realloc(ih->data->col.inactive, ih->data->col.numaloc*sizeof(char));
    for(j = 0; j < ncols; j++)
    {
      ih->data->col.marked[nc+j]   = 0;
      ih->data->col.inactive[nc+j] = 0;
    }
  }
  else if(!ih->data->valcb)
  {
    /* Initialize new cells to be used with NULL */
    for(i = 0; i < ih->data->lin.numaloc; i++)
    {
      for(j = 0; j < ncols; j++)
      {
         if(ih->data->v[i][nc+j].nba)
          ih->data->v[i][nc+j].value[0] = 0;  /* empty string, can not put NULL because the pointer will be lost */
        ih->data->v[i][nc+j].mark = 0;
      }
    }
  }
}

/* Allocate space to store the cell values. This space is allocated
   in packages of the IMAT_BLOCKSIZE bytes.
   -> lin, col : cell coordinates to which will be allocated space
   -> numc : number of characteres (size) of the string that will be
             store in the cell
*/
void iupMatrixMemAlocCell(Ihandle* ih, int lin, int col, int numc)
{
  if(ih->data->v[lin][col].nba <= numc)
  {
    int numbytes = IMAT_BLOCKSIZE * (numc / IMAT_BLOCKSIZE + 1);

    if(ih->data->v[lin][col].nba > 0)
      free (ih->data->v[lin][col].value);

    ih->data->v[lin][col].value = (char*)malloc(numbytes + 1); /* +1 do '\0'*/
    ih->data->v[lin][col].nba   = numbytes;
  }
}
