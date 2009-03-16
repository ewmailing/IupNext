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
#include <stdlib.h>
#include <string.h>

#include "iup.h"

#include <cd.h>

#include "iup_object.h"
#include "iup_attrib.h"
#include "iup_str.h"
#include "iup_stdcontrols.h"

#include "iupmat_def.h"
#include "iupmat_edit.h"
#include "iupmat_mem.h"
#include "iupmat_numlc.h"
#include "iupmat_draw.h"


static void iMatrixUpdateLineAttributes(Ihandle* ih, int base, int count, int add)
{
#define IMAT_NUM_ATTRIB_LINE 6
#define IMAT_ATTRIB_LINE_ONLY 3
  char* attrib_format[IMAT_NUM_ATTRIB_LINE] = {
    "BGCOLOR%d:*",
    "FGCOLOR%d:*",
    "FONT%d:*",
    "BGCOLOR%d:%d",
    "FGCOLOR%d:%d",
    "FONT%d:%d"};
  char* attrib = iupStrGetMemory(100);
  int a, lin, col;
  char* value;

  if (add)  /* ADD */
  {
    /* copy the attributes of the moved cells, from base+count to num */
    /*   do it in reverse order to avoid overlapping */
    /* then clear the new space starting from base to base+count */

    for(a = 0; a < IMAT_NUM_ATTRIB_LINE; a++)
    {
      for(lin = ih->data->lines.num-1; lin >= base+count; lin--)
      {
        /* Update the line attributes */
        if (a < IMAT_ATTRIB_LINE_ONLY)
        {
          sprintf(attrib, attrib_format[a], lin-count);
          value = iupAttribGet(ih, attrib);
          sprintf(attrib, attrib_format[a], lin);
          iupAttribStoreStr(ih, attrib, value);
        }
        /* Update the cell attribute */
        else for(col = 0; col < ih->data->columns.num; col++)
        {
          sprintf(attrib, attrib_format[a], lin-count, col);
          value = iupAttribGet(ih, attrib);
          sprintf(attrib, attrib_format[a], lin, col);
          iupAttribStoreStr(ih, attrib, value);
        }
      }

      for(lin = base; lin < base+count; lin++)
      {
        if (a < IMAT_ATTRIB_LINE_ONLY)
        {
          sprintf(attrib, attrib_format[a], lin);
          iupAttribSetStr(ih, attrib, NULL);
        }
        else for(col = 0; col < ih->data->columns.num; col++)
        {
          sprintf(attrib, attrib_format[a], lin, col);
          iupAttribSetStr(ih, attrib, NULL);
        }
      }
    }
  }
  else  /* DEL */
  {
    /* copy the attributes of the moved cells from base+count to base */
    /* then clear the remaining space starting at num */

    for(a = 0; a < IMAT_NUM_ATTRIB_LINE; a++)
    {
      for(lin = base; lin < ih->data->lines.num; lin++)
      {
        /* Update the line attributes */     
        if (a < IMAT_ATTRIB_LINE_ONLY)      
        {
          sprintf(attrib, attrib_format[a], lin+count);
          value = iupAttribGet(ih, attrib);
          sprintf(attrib, attrib_format[a], lin);
          iupAttribStoreStr(ih, attrib, value);
        }
        /* Update each cell attribute */
        else for(col = 0; col < ih->data->columns.num; col++) 
        {
          sprintf(attrib, attrib_format[a], lin+count, col);
          value = iupAttribGet(ih, attrib);
          sprintf(attrib, attrib_format[a], lin, col);
          iupAttribStoreStr(ih, attrib, value);
        }
      }

      for(lin = ih->data->lines.num; lin < ih->data->lines.num+count; lin++)
      {
        if (a < IMAT_ATTRIB_LINE_ONLY)
        {
          sprintf(attrib, attrib_format[a], lin);
          iupAttribSetStr(ih, attrib, NULL);
        }
        else for(col = 0; col < ih->data->columns.num; col++)
        {
          sprintf(attrib, attrib_format[a], lin, col);
          iupAttribSetStr(ih, attrib, NULL);
        }
      }
    }
  }
}

static void iMatrixUpdateColumnAttributes(Ihandle* ih, int base, int count, int add)
{
#define IMAT_NUM_ATTRIB_COL 7
#define IMAT_ATTRIB_COL_ONLY 4
  char* attrib_format[IMAT_NUM_ATTRIB_COL] = { 
    "ALIGNMENT%d",
    "BGCOLOR*:%d",
    "FGCOLOR*:%d",
    "FONT*:%d",
    "BGCOLOR%d:%d",
    "FGCOLOR%d:%d",
    "FONT%d:%d"};
  char* attrib = iupStrGetMemory(100);
  int a, col, lin;
  char* value;

  if (add)  /* ADD */
  {
    /* update the attributes of the moved cells, from base+count to num */
    /*   do it in reverse order to avoid overlapping */
    /* then clear the new space starting from base to base+count */

    for(a = 0; a < IMAT_NUM_ATTRIB_COL; a++)
    {
      for(col = ih->data->columns.num-1; col >= base+count; col--)
      {
        /* Update the column attributes */
        if (a < IMAT_ATTRIB_COL_ONLY)
        {
          sprintf(attrib, attrib_format[a], col-count);
          value = iupAttribGet(ih, attrib);
          sprintf(attrib,attrib_format[a],col);
          iupAttribStoreStr(ih, attrib, value);
        }
        /* Update the cell attributes */
        else for(lin = 0; lin < ih->data->lines.num; lin++)
        {
          sprintf(attrib, attrib_format[a], lin, col-count);
          value = iupAttribGet(ih, attrib);
          sprintf(attrib, attrib_format[a], lin, col);
          iupAttribStoreStr(ih, attrib, value);
        }
      }

      for(col = base; col < base+count; col++)
      {
        if (a < IMAT_ATTRIB_COL_ONLY)
        {
          sprintf(attrib, attrib_format[a], col);
          iupAttribSetStr(ih, attrib, NULL);
        }
        else for(lin = 0; lin < ih->data->lines.num; lin++)
        {
          sprintf(attrib, attrib_format[a], lin, col);
          iupAttribSetStr(ih, attrib, NULL);
        }
      }
    }
  }
  else   /* DEL */
  {
    /* copy the attributes of the moved cells from base+count to base */
    /* then clear the remaining space starting at num */

    for(a = 0; a < IMAT_NUM_ATTRIB_COL; a++)
    {
      for(col = base; col < ih->data->columns.num; col++)
      {
        /* Update the column attributes */
        if (a < IMAT_ATTRIB_COL_ONLY)
        {
          sprintf(attrib, attrib_format[a], col+count);
          value = iupAttribGet(ih, attrib);
          sprintf(attrib, attrib_format[a], col);
          iupAttribStoreStr(ih, attrib, value);
        }
        /* Update the cell attributes */
        else for(lin = 0; lin < ih->data->lines.num; lin++)
        {
          sprintf(attrib, attrib_format[a], lin, col+count);
          value = iupAttribGet(ih, attrib);
          sprintf(attrib, attrib_format[a], lin, col);
          iupAttribStoreStr(ih, attrib, value);
        }
      }

      for(col = ih->data->columns.num; col < ih->data->columns.num+count; col++)
      {
        if (a < IMAT_ATTRIB_COL_ONLY)
        {
          sprintf(attrib, attrib_format[a], col);
          iupAttribSetStr(ih, attrib, NULL);
        }
        else for(lin = 0; lin < ih->data->lines.num; lin++)
        {
          sprintf(attrib, attrib_format[a], lin, col);
          iupAttribSetStr(ih, attrib, NULL);
        }
      }
    }
  }
}

static int iMatrixGetStartEnd(const char* value, int *base, int *count, int max, int fit_count)
{
  int ret;

  if (!value)
    return 0;

  *base = 0; 
  *count = 1;

  ret = sscanf(value, "%d-%d", base, count);
  if (ret <= 0 || ret > 2)
    return 0;
  if (ret == 1)
    *count = 1;

  if (*base < 0 || *base > max || *count <= 0)
    return 0;

  if (fit_count)  /* always when DEL */
  {
    if (*base == max-1)
      return 0;

    /* base can NOT be 0 */
    if (*base == 0)
      *base = 1;

    /* count must be inside the existant range */
    if (*base + *count - 1 >= max)
      *count = max-1 - *base + 1;
  }

  return 1;
}


/**************************************************************************/
/* Exported functions                                                     */
/**************************************************************************/

int iupMatrixSetAddLinAttrib(Ihandle* ih, const char* value)
{
  int base, count, lines_num = ih->data->lines.num;

  if (!iMatrixGetStartEnd(value, &base, &count, lines_num, 0))
    return 0;

  /* if the focus cell is after the inserted area */
  if (ih->data->lines.focus_cell >= base)
  {
    /* leave of the edition mode */
    iupMatrixEditForceHidden(ih);

    /* move it to the same cell */
    ih->data->lines.focus_cell += count;
  }

  iupMatrixMemReAllocLines(ih, lines_num, lines_num+count, base);

  ih->data->lines.num += count;
  ih->data->need_calcsize = 1;

  if (base < lines_num)  /* If before the last line. */
    iMatrixUpdateLineAttributes(ih, base, count, 1);

  iupMatrixDraw(ih, 1);
  return 0;
}

int iupMatrixSetDelLinAttrib(Ihandle* ih, const char* value)
{
  int base, count, lines_num = ih->data->lines.num;

  if (!iMatrixGetStartEnd(value, &base, &count, lines_num, 1))
    return 0;

  /* if the focus cell is after the removed area */
  if (ih->data->lines.focus_cell >= base)
  {
    /* leave of the edition mode */
    iupMatrixEditForceHidden(ih);

    /* if the focus cell is inside the removed area */
    if (ih->data->lines.focus_cell <= base+count-1)
      ih->data->lines.focus_cell = base;   /* move it to the first existant cell */
    else
      ih->data->lines.focus_cell -= count; /* move it to the same cell */
  }

  iupMatrixMemReAllocLines(ih, lines_num, lines_num-count, base);

  ih->data->lines.num -= count;
  ih->data->need_calcsize = 1;

  if (ih->data->lines.focus_cell >= ih->data->lines.num)
    ih->data->lines.focus_cell = ih->data->lines.num-1;
  if (ih->data->lines.focus_cell == 0)
    ih->data->lines.focus_cell = 1;

  if (base < lines_num)  /* If before the last line. */
    iMatrixUpdateLineAttributes(ih, base, count, 0);

  iupMatrixDraw(ih, 1);
  return 0;
}

int iupMatrixSetAddColAttrib(Ihandle* ih, const char* value)
{
  int base, count, columns_num = ih->data->columns.num;

  if (!iMatrixGetStartEnd(value, &base, &count, columns_num, 0))
    return 0;

  /* if the focus cell is after the inserted area */
  if (ih->data->columns.focus_cell >= base)
  {
    /* leave the edition mode */
    iupMatrixEditForceHidden(ih);

    /* move it to the same cell */
    ih->data->columns.focus_cell += count;
  }

  iupMatrixMemReAllocColumns(ih, columns_num, columns_num+count, base);

  ih->data->columns.num += count;
  ih->data->need_calcsize = 1;

  if (base < columns_num)  /* If before the last column. */
    iMatrixUpdateColumnAttributes(ih, base, count, 1);

  iupMatrixDraw(ih, 1);
  return 0;
}

int iupMatrixSetDelColAttrib(Ihandle* ih, const char* value)
{
  int base, count, columns_num = ih->data->columns.num;

  if (!iMatrixGetStartEnd(value, &base, &count, columns_num, 1))
    return 0;

  /* if the focus cell is after the removed area */
  if (ih->data->columns.focus_cell >= base)
  {
    /* leave the edition mode */
    iupMatrixEditForceHidden(ih);

    /* if the focus cell is inside the removed area */
    if (ih->data->columns.focus_cell <= base+count-1)
      ih->data->columns.focus_cell = base;    /* move it to the first existant cell */
    else
      ih->data->columns.focus_cell -= count;  /* move it to the same cell */
  }

  iupMatrixMemReAllocColumns(ih, columns_num, columns_num-count, base);

  ih->data->columns.num -= count;
  ih->data->need_calcsize = 1;

  if (ih->data->columns.focus_cell >= ih->data->columns.num)
    ih->data->columns.focus_cell = ih->data->columns.num-1;
  if (ih->data->columns.focus_cell == 0)
    ih->data->columns.focus_cell = 1;

  if (base < columns_num)  /* If before the last column. */
    iMatrixUpdateColumnAttributes(ih, base, count, 0);

  iupMatrixDraw(ih, 1);
  return 0;
}

int iupMatrixSetNumLinAttrib(Ihandle* ih, const char* value)
{
  int num = 0;
  if (iupStrToInt(value, &num))
  {
    if (num < 0) num = 0;

    /* can be set before map */
    if (ih->handle)
      iupMatrixMemReAllocLines(ih, ih->data->lines.num, num+1, ih->data->lines.num);  /* base is at the end */

    ih->data->lines.num = num+1;  /* add room for title line */
    ih->data->need_calcsize = 1;

    if (ih->handle)
      iupMatrixDraw(ih, 1);
  }

  return 0;
}

int iupMatrixSetNumColAttrib(Ihandle* ih, const char* value)
{
  int num = 0;
  if (iupStrToInt(value, &num))
  {
    if (num < 0) num = 0;

    /* can be set before map */
    if (ih->handle)
      iupMatrixMemReAllocColumns(ih, ih->data->columns.num, num+1, ih->data->columns.num);  /* base is at the end */

    ih->data->columns.num = num+1;  /* add room for title column */
    ih->data->need_calcsize = 1;

    if (ih->handle)
      iupMatrixDraw(ih, 1);
  }

  return 0;
}
