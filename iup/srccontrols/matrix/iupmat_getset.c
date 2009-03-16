/** \file
 * \brief iupmatrix setget control
 * attributes set and get
 *
 * See Copyright Notice in "iup.h"
 */

#include <stdio.h>
#include <stdlib.h>

#include "iup.h"
#include "iupcbs.h"

#include <cd.h>

#include "iup_object.h"
#include "iup_attrib.h"
#include "iup_drvfont.h"
#include "iup_str.h"
#include "iup_stdcontrols.h"
#include "iup_childtree.h"

#include "iupmat_def.h"
#include "iupmat_getset.h"
#include "iupmat_edit.h"
#include "iupmat_draw.h"
#include "iupmat_aux.h"


int iupMatrixCheckCellPos(Ihandle* ih, int lin, int col)
{
  /* Check if the cell exists */
  if ((lin < 0) || (col < 0) || (lin > ih->data->lines.num-1) || (col > ih->data->columns.num-1))
    return 0;

  return 1;
}

void iupMatrixCellSetValue(Ihandle* ih, int lin, int col, const char* value)
{
  if (ih->data->callback_mode)
    return;

  if (ih->data->cells[lin][col].value)
    free(ih->data->cells[lin][col].value);

  ih->data->cells[lin][col].value = iupStrDup(value);

  if (lin==0 || col==0)
    ih->data->need_calcsize = 1;
}

char* iupMatrixCellGetValue (Ihandle* ih, int lin, int col)
{
  if (!ih->handle)
  {
    char str[100];
    sprintf(str, "%d:%d", lin, col);
    return iupAttribGet(ih, str);
  }
  else
  {
    if (ih->data->callback_mode)
    {
      sIFnii value_cb = (sIFnii)IupGetCallback(ih, "VALUE_CB");
      return value_cb(ih, lin, col);
    }
    else
      return ih->data->cells[lin][col].value;
  }
}

void iupMatrixCellUpdateValue(Ihandle* ih)
{
  IFniis value_edit_cb;
  char *value = iupMatrixEditGetValue(ih);

  iupMatrixCellSetValue(ih, ih->data->lines.focus_cell, ih->data->columns.focus_cell, value);

  value_edit_cb = (IFniis)IupGetCallback(ih, "VALUE_EDIT_CB");
  if (value_edit_cb)
    value_edit_cb(ih, ih->data->lines.focus_cell, ih->data->columns.focus_cell, value);

  iupMatrixDrawCell(ih, ih->data->lines.focus_cell, ih->data->columns.focus_cell);
}

static char* iMatrixGetCellAttrib(Ihandle* ih, const char* attrib, int lin, int col, int native_parent)
{
  char* value = NULL;
  char attrib_id[100];

  /* 1 -  check for this cell */
  sprintf(attrib_id, "%s%d:%d", attrib, lin, col);
  value = iupAttribGet(ih, attrib_id);
  if (!value)
  {
    /* 2 - check for this line, if not title col */
    if (col != 0)
    {
      sprintf(attrib_id, "%s%d:*", attrib, lin);
      value = iupAttribGet(ih, attrib_id);
    }

    if (!value)
    {
      /* 3 - check for this column, if not title line */
      if (lin != 0)
      {
        sprintf(attrib_id,"%s*:%d",attrib,col);
        value = iupAttribGet(ih, attrib_id);
      }

      if (!value)
      {
        /* 4 - check for the matrix or native parent */
        if (native_parent)
          value = IupGetAttribute(iupChildTreeGetNativeParent(ih), attrib);
        else
          value = IupGetAttribute(ih, attrib);
      }
    }
  }

  return value;
}

static int iMatrixDrawCallColorCB(Ihandle* ih, const char* cb_name, int lin, int col, unsigned char *r, unsigned char *g, unsigned char *b)
{
  IFniiIII cb = (IFniiIII)IupGetCallback(ih, cb_name);
  if(cb)
  {
    int ir, ig, ib, ret;
    ret = cb(ih, lin, col, &ir, &ig, &ib);
    *r = (unsigned char)(ir<0?0:(ir>255)?255:ir);
    *g = (unsigned char)(ig<0?0:(ig>255)?255:ig);
    *b = (unsigned char)(ib<0?0:(ib>255)?255:ib);
    return ret;
  }
  else
    return IUP_IGNORE;
}

char* iupMatrixGetFgColor(Ihandle* ih, int lin, int col)
{
  unsigned char r = 0, g = 0, b = 0;
  if (iMatrixDrawCallColorCB(ih, "FGCOLOR_CB", lin, col, &r, &g, &b) == IUP_IGNORE)
    return iMatrixGetCellAttrib(ih, "FGCOLOR", lin, col, 0);
  else
  {
    char* buffer = iupStrGetMemory(30);
    sprintf(buffer, "%d %d %d", r, g, b);
    return buffer;
  }
}

void iupMatrixGetFgRGB(Ihandle* ih, int lin, int col, unsigned char *r, unsigned char *g, unsigned char *b)
{
  if (iMatrixDrawCallColorCB(ih, "FGCOLOR_CB", lin, col, r, g, b) == IUP_IGNORE)
    iupStrToRGB(iMatrixGetCellAttrib(ih, "FGCOLOR", lin, col, 0), r, g, b);
}

char* iupMatrixGetBgColor(Ihandle* ih, int lin, int col)
{
  unsigned char r = 0, g = 0, b = 0;
  if (iMatrixDrawCallColorCB(ih, "BGCOLOR_CB", lin, col, &r, &g, &b) == IUP_IGNORE)
  {
    int native_parent = 0;
    if (lin == 0 || col == 0)
      native_parent = 1;
    return iMatrixGetCellAttrib(ih, "BGCOLOR", lin, col, native_parent);
  }
  else
  {
    char* buffer = iupStrGetMemory(30);
    sprintf(buffer, "%d %d %d", r, g, b);
    return buffer;
  }
}

void iupMatrixGetBgRGB(Ihandle* ih, int lin, int col, unsigned char *r, unsigned char *g, unsigned char *b)
{
  if (iMatrixDrawCallColorCB(ih, "BGCOLOR_CB", lin, col, r, g, b) == IUP_IGNORE)
  {
    int native_parent = 0;
    if (lin == 0 || col == 0)
      native_parent = 1;
    iupStrToRGB(iMatrixGetCellAttrib(ih, "BGCOLOR", lin, col, native_parent), r, g, b);
  }
}

char* iupMatrixGetFont(Ihandle* ih, int lin, int col)
{
  return iMatrixGetCellAttrib(ih, "FONT", lin, col, 0);
}

char *iupMatrixGetSize(Ihandle* ih, int index, int m, int pixels)
{
  char* str;
  int size;
  ImatLinColData *lincol_data;

  if(m == IMAT_PROCESS_LIN)
    lincol_data = &(ih->data->lines);
  else
    lincol_data = &(ih->data->columns);

  if (index < 0 || index > lincol_data->num-1)
    return NULL;

  if (m == IMAT_PROCESS_LIN)
    size = iupMatrixAuxGetLineHeight(ih, index);
  else
    size = iupMatrixAuxGetColumnWidth(ih, index);

  str = iupStrGetMemory(100);

  if (size)
  {
    if (m == IMAT_PROCESS_COL)
      size -= IMAT_DECOR_X;
    else
      size -= IMAT_DECOR_Y;

    if (!pixels)
    {
      int charwidth, charheight;
      iupdrvFontGetCharSize(ih, &charwidth, &charheight);
      if (m == IMAT_PROCESS_COL)
        size = iupRASTER2WIDTH(size, charwidth);
      else
        size = iupRASTER2HEIGHT(size, charheight);
    }
  }

  sprintf(str, "%d", size);
  return str;
}
