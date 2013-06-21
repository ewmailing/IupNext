/** \file
 * \brief IupMatrix Expansion Library.
 *
 * See Copyright Notice in "iup.h"
 */

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>

#include "iup.h"
#include "iupcbs.h"

#include "iup_array.h"
#include "iup_object.h"
#include "iup_attrib.h"
#include "iup_str.h"
#include "iup_matrixex.h"


static void iMatrixExCellMarkedLimits(const char* marked, int num_lin, int num_col, int *lin1, int *lin2, int *col1, int *col2)
{
  /* use only for MARKMODE=CELL */
  int lin, col;

  *lin1 = 0;
  *lin2 = num_lin;
  *col1 = 0;
  *col2 = num_col;

  for(lin = 1; lin <= num_lin; ++lin)
  {
    for(col = 1; col <= num_col; ++col)
    {
      int pos = (lin-1) * num_col + (col-1);  /* marked array does not include titles */
      if(marked[pos] == '1')
      {
        if(lin < *lin1) *lin1 = lin;
        if(lin > *lin2) *lin2 = lin;
        if(col < *col1) *col1 = col;
        if(col > *col2) *col2 = col;
      }
    }
  }
}

static void iMatrixExCellMarkedStart(const char* marked, int num_lin, int num_col, int *lin1, int *col1)
{
  int lin, col;

  if (*marked == 'C')
  {
    marked++;

    *col1 = 1;

    for(lin = 1; lin <= num_lin; ++lin)
    {
      if (marked[lin] == '1')
      {
        *lin1 = lin;
        return;
      }
    }
  }
  else if (*marked == 'L')
  {
    marked++;

    *lin1 = 1;

    for(col = 1; col <= num_col; ++col)
    {
      if (marked[col] == '1')
      {
        *col1 = col;
        return;
      }
    }
  }
  else
  {
    for(lin = 1; lin <= num_lin; ++lin)
    {
      for(col = 1; col <= num_col; ++col)
      {
        int pos = (lin-1) * num_col + (col-1);  /* marked array does not include titles */
        if (marked[pos] == '1')
        {
          *lin1 = lin;
          *col1 = col;
          return;
        }
      }
    }
  }
}

static void iMatrixExArrayAddChar(Iarray* data, char c)
{
  int last_count = iupArrayCount(data);
  char* str_data = (char*)iupArrayInc(data);
  str_data[last_count] = c;
}

static void iMatrixExArrayAddEmpty(Iarray* data)
{
  iMatrixExArrayAddChar(data, ' ');
}

static void iMatrixExReplaceSep(char *txt, int count)
{
  int i;
  for (i=0; i<count; i++)
  {
    if (txt[i]=='\n') txt[i]=' ';
    if (txt[i]=='\t') txt[i]=' ';
  }
}

static void iMatrixExArrayAddStr(Iarray* data, char* str)
{
  int add_count = strlen(str);
  int last_count = iupArrayCount(data);
  char* str_data = (char*)iupArrayAdd(data, add_count);
  memcpy(str_data+last_count, str, add_count);
  iMatrixExReplaceSep(str_data+last_count, add_count);
}

static void iMatrixExArrayAddCell(Ihandle* ih, Iarray* data, int lin, int col, sIFnii value_cb)
{
  char* value = iupMatrixExGetCell(ih, lin, col, value_cb);

  if (value)
    iMatrixExArrayAddStr(data, value);
  else
    iMatrixExArrayAddEmpty(data);
}

static int iMatrixExCellMarkedConsistent(const char* marked, int num_lin, int num_col)
{
  int lin, col, selected;
  const char *l1=NULL, *ll=NULL;

  /* the selected column pattern must be consistent along lines */

  for(lin = 1; lin <= num_lin; ++lin)
  {
    selected = 0;

    for(col = 1; col <= num_col; ++col)
    {
      int pos = (lin-1) * num_col + (col-1);  /* marked array does not include titles */
      if (marked[pos] == '1')
      {
        ll = &marked[(lin-1)*num_col];

        if (l1==NULL)
          l1 = ll;

        selected = 1;
        break;
      }
    }

    if (selected && ll!=l1)
    {
      if (strncmp(l1,ll,(size_t)num_col)!=0)
        return 0;
    }
  }

  return 1;
}

static int iMatrixExSetCopyAttrib(Ihandle *ih, const char* value)
{
  int lin, col, num_lin, num_col;
  sIFnii value_cb;
  Iarray* data;
  int add_sep;
  char *marked;

  /* unused */
  (void)value;

  /* reset error state */
  iupAttribSetStr(ih, "LASTERROR", NULL);

  marked = IupGetAttribute(ih,"MARKED");
  if (!marked)  /* no marked cells */
  {
    iupAttribSetStr(ih, "LASTERROR", "NOMARKED");
    return 0;
  }

  num_lin = IupGetInt(ih, "NUMLIN");
  num_col = IupGetInt(ih, "NUMCOL");
  value_cb = (sIFnii)IupGetCallback(ih, "VALUE_CB");
  data =  iupArrayCreate(100, sizeof(char));

  if (*marked == 'C')
  {
    marked++;

    for(lin = 1; lin <= num_lin; ++lin)
    {
      add_sep = 0;

      if (iupMatrixExIsLineVisible(ih, lin))
      {
        for(col = 1; col <= num_col; ++col)
        {
          if (marked[col-1] == '1' && iupMatrixExIsColumnVisible(ih, col))
          {
            if (add_sep)
              iMatrixExArrayAddChar(data, '\t');

            iMatrixExArrayAddCell(ih, data, lin, col, value_cb);
            add_sep = 1;
          }
        }
      }

      if (add_sep)
        iMatrixExArrayAddChar(data, '\n');
    }
  }
  else if (*marked == 'L')
  {
    marked++;

    for(lin = 1; lin <= num_lin; ++lin)
    {
      add_sep = 0;

      if (iupMatrixExIsLineVisible(ih, lin))
      {
        for(col = 1; col <= num_col; ++col)
        {
          if (marked[lin-1] == '1' && iupMatrixExIsColumnVisible(ih, col))
          {
            if (add_sep)
              iMatrixExArrayAddChar(data, '\t');

            iMatrixExArrayAddCell(ih, data, lin, col, value_cb);
            add_sep = 1;
          }
        }
      }

      if (add_sep)
        iMatrixExArrayAddChar(data, '\n');
    }
  }
  else
  {
    int lin1=0, lin2=num_lin, 
        col1=0, col2=num_col;
    int keep_struct = IupGetInt(ih, "COPYKEEPSTRUCT");

    iMatrixExCellMarkedLimits(marked, num_lin, num_col, &lin1, &lin2, &col1, &col2);

    /* check consistency only when not keeping structure */
    if (!keep_struct && !iMatrixExCellMarkedConsistent(marked, num_lin, num_col))
    {
      iupAttribSetStr(ih, "LASTERROR", "MARKEDCONSISTENCY");
      iupArrayDestroy(data);
      return 0;
    }

    for(lin = lin1; lin <= lin2; ++lin)
    {
      add_sep = 0;

      if (iupMatrixExIsLineVisible(ih, lin))
      {
        for(col = col1; col <= col2; ++col)
        {
          if (iupMatrixExIsColumnVisible(ih, col))
          {
            if (marked[lin-1] == '1')
            {
              if (add_sep)
                iMatrixExArrayAddChar(data, '\t');
              iMatrixExArrayAddCell(ih, data, lin, col, value_cb);
              add_sep = 1;
            }
            else if (keep_struct)
            {
              if (add_sep)
                iMatrixExArrayAddChar(data, '\t');
              iMatrixExArrayAddEmpty(data);
              add_sep = 1;
            }
          }
        }
      }

      if (add_sep)
        iMatrixExArrayAddChar(data, '\n');
    }
  }

  if (iupArrayCount(data)!=0)
  {
    Ihandle* clipboard = IupClipboard();
    IupSetAttribute(clipboard, "TEXT", NULL);  /* clear all data from clipboard */
    iMatrixExArrayAddChar(data, '\0');
    IupSetAttribute(clipboard, "TEXT", (char*)iupArrayGetData(data));
    IupDestroy(clipboard);
  }

  iupArrayDestroy(data);

  return 0;
}

static int iMatrixExGetDataSize(const char* data, int *num_lin, int *num_col, char *sep)
{
  *num_lin = iupStrLineCount(data);

  if (*sep != 0)
    *num_col = iupStrCountChar(data, *sep);
  else
  {
    *sep = '\t';
    *num_col = iupStrCountChar(data, *sep);
    if (*num_col == 0)
    {
      *sep = ';';
      *num_col = iupStrCountChar(data, *sep);
    }
  }

  if (*num_lin == 0 ||
      *num_col == 0)
    return 0;

  if ((*num_col)%(*num_lin)!=0)
    return 0;

  *num_col = (*num_col)/(*num_lin) + 1;
  return 1;
}

static char* iMatrixExCopyValue(char* value, int *value_max_size, const char* data, int value_len)
{
  if (*value_max_size < value_len)
  {
    *value_max_size = value_len+10;
    value = realloc(value, *value_max_size);
  }
  memcpy(value, data, value_len);
  value[value_len] = 0;
  return value;
}

static void iMatrixExParseText(Ihandle *ih, const char* data, int data_num_lin, int data_num_col, char sep, int start_lin, int start_col, int num_lin, int num_col, const char* busyname)
{
  IFniis value_edit_cb = (IFniis) IupGetCallback(ih,"VALUE_EDIT_CB");
  IFniiii edition_cb  = (IFniiii)IupGetCallback(ih,"EDITION_CB");
  int lin, col, len, l, c;
  char* value = NULL;
  int value_max_size = 0, value_len;

  iupMatrixExBusyStart(ih, data_num_lin*data_num_col, busyname);

  lin = start_lin;
  l = 0;
  while (lin <= num_lin && l<data_num_lin)
  {
    if (iupMatrixExIsLineVisible(ih, lin))
    {
      const char* next_line = iupStrNextLine(data, &len); l++;

      col = start_col;
      c = 0;
      while (col <= num_col && c<data_num_col)
      {
        if (iupMatrixExIsColumnVisible(ih, col))
        {
          const char* next_value = iupStrNextValue(data, len, &value_len, sep);  c++;

          value = iMatrixExCopyValue(value, &value_max_size, data, value_len);
          iupMatrixExSetCell(ih, lin, col, value, edition_cb, value_edit_cb);

          data = next_value;

          if (!iupMatrixExBusyInc(ih))
          {
            if (value) 
              free(value);
            return;
          }
        }

        col++;
      }

      data = next_line;
    }

    lin++;
  }

  iupMatrixExBusyEnd(ih);

  if (value)
    free(value);
}

static void iMatrixExSetData(Ihandle *ih, const char* data, int data_num_lin, int data_num_col, char sep, int lin, int col, int num_lin, int num_col, const char* busyname)
{
//  iupMatrixExPushUndoBlock(ih, lin, col, data_num_lin, data_num_col);

  iMatrixExParseText(ih, data, data_num_lin, data_num_col, sep, lin, col, num_lin, num_col, busyname);

  IupSetAttribute(ih,"REDRAW","ALL");
}

static int iMatrixExGetVisibleNumLin(Ihandle *ih, int start_lin, int data_num_lin)
{
  int lin, vis_num_lin = data_num_lin;
  for (lin=start_lin; lin < start_lin+data_num_lin; lin++)
  {
    if (!iupMatrixExIsLineVisible(ih, lin))
      vis_num_lin++;
  }
  return vis_num_lin;
}

static int iMatrixExGetVisibleNumCol(Ihandle *ih, int start_col, int data_num_col)
{
  int col, vis_num_col = data_num_col;
  for (col=start_col; col < start_col+data_num_col; col++)
  {
    if (!iupMatrixExIsColumnVisible(ih, col))
      vis_num_col++;
  }
  return vis_num_col;
}

static void iMatrixExSetPasteData(Ihandle *ih, const char* data, int lin, int col, const char* busyname)
{
  int num_lin, num_col, skip_lines,
      data_num_lin, data_num_col;
  char sep=0, *str_sep;
  IFnii pastesize_cb;

  /* reset error state */
  iupAttribSetStr(ih, "LASTERROR", NULL);

  if (!data || data[0]==0)
  {
    iupAttribSetStr(ih, "LASTERROR", "NOTEXT");
    return;
  }

  skip_lines = IupGetInt(ih, "TEXTSKIPLINES");
  if (skip_lines)
  {
    int i, len;
    const char *pdata;
    for (i=0; i<skip_lines; i++)
    {
      pdata = iupStrNextLine(data, &len);
      if (pdata==data) /* no next line */ 
      {
        iupAttribSetStr(ih, "LASTERROR", "NOTEXT");
        return;
      }
      data = (char*)pdata;
    }
  }

  str_sep = IupGetAttribute(ih, "TEXTSEPARATOR");
  if (str_sep) sep = *str_sep;

  if (!iMatrixExGetDataSize(data, &data_num_lin, &data_num_col, &sep))
  {
    iupAttribSetStr(ih, "LASTERROR", "INVALIDMATRIX");
    return;
  }

  num_lin = IupGetInt(ih, "NUMLIN");
  num_col = IupGetInt(ih, "NUMCOL");

  pastesize_cb = (IFnii)IupGetCallback(ih, "PASTESIZE_CB");
  if (pastesize_cb)
  {
    int vis_num_lin = iMatrixExGetVisibleNumLin(ih, lin, data_num_lin);
    int vis_num_col = iMatrixExGetVisibleNumCol(ih, col, data_num_col);
    if (lin+vis_num_lin>num_lin ||
        col+vis_num_col>num_col)
    {
      int ret = pastesize_cb(ih, lin+vis_num_lin, col+vis_num_col);
      if (ret == IUP_IGNORE)
        return;
      else if (ret == IUP_CONTINUE)
      {
        if (lin+vis_num_lin>num_lin) IupSetfAttribute(ih, "NUMLIN", "%d", lin+vis_num_lin);
        if (col+vis_num_col>num_col) IupSetfAttribute(ih, "NUMCOL", "%d", col+vis_num_col);
      }
    }
  }

  iMatrixExSetData(ih, data, data_num_lin, data_num_col, sep, lin, col, num_lin, num_col, busyname);
}

static int iMatrixExSetPasteAttrib(Ihandle *ih, const char* value)
{
  int lin=0, col=0;

  Ihandle* clipboard = IupClipboard();
  char* data = IupGetAttribute(clipboard, "TEXT");
  IupDestroy(clipboard);

  if (iupStrEqualNoCase(value, "FOCUS"))
    IupGetIntInt(ih, "FOCUS_CELL", &lin, &col);
  else if (iupStrEqualNoCase(value, "SELECTION"))
  {
    char *marked = IupGetAttribute(ih,"MARKED");
    if (marked)
    {
      int num_lin = IupGetInt(ih, "NUMLIN");
      int num_col = IupGetInt(ih, "NUMCOL");
      iMatrixExCellMarkedStart(marked, num_lin, num_col, &lin, &col);
    }
  }
  else
  {
    if (iupStrToIntInt(value, &lin, &col, ':')!=2)
      return 0;
  }

  iMatrixExSetPasteData(ih, data, lin, col, "PASTE");
  return 0;
}

static int iMatrixExSetPasteDataAttrib(Ihandle *ih, const char* data)
{
  int lin=0, col=0;
  IupGetIntInt(ih, "FOCUS_CELL", &lin, &col);
  iMatrixExSetPasteData(ih, data, lin, col, "PASTEDATA");
  return 0;
}

static int iMatrixExSetPasteFileAttrib(Ihandle *ih, const char* value)
{
  size_t size;
  char* data;

  FILE *file = fopen(value, "rb");
  if (!file)
  {
    iupAttribSetStr(ih, "LASTERROR", "INVALIDFILENAME");
    return 0;
  }

  fseek(file, 0, SEEK_END);
  size = (size_t)ftell(file); 
  fseek(file, 0, SEEK_SET);

  data = (char*)malloc(size+1);
  fread(data, size, 1, file);
  data[size] = 0;
  fclose(file);

  iMatrixExSetPasteData(ih, data, 0, 0, "PASTEFILE");

  free(data);
  return 0;
}

void iupMatrixExRegisterClipboard(Iclass* ic)
{
  iupClassRegisterCallback(ic, "PASTESIZE_CB", "ii");

  iupClassRegisterAttribute(ic, "PASTE", NULL, iMatrixExSetPasteAttrib, NULL, NULL, IUPAF_WRITEONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "PASTEDATA", NULL, iMatrixExSetPasteDataAttrib, NULL, NULL, IUPAF_WRITEONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "PASTEFILE", NULL, iMatrixExSetPasteFileAttrib, NULL, NULL, IUPAF_WRITEONLY|IUPAF_NO_INHERIT);

  iupClassRegisterAttribute(ic, "COPY", NULL, iMatrixExSetCopyAttrib, NULL, NULL, IUPAF_WRITEONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "COPYKEEPSTRUCT", NULL, NULL, NULL, NULL, IUPAF_NO_INHERIT);

  iupClassRegisterAttribute(ic, "LASTERROR", NULL, NULL, NULL, NULL, IUPAF_READONLY|IUPAF_NO_INHERIT);

  iupClassRegisterAttribute(ic, "TEXTSEPARATOR", NULL, NULL, NULL, NULL, IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "TEXTSKIPLINES", NULL, NULL, NULL, NULL, IUPAF_NO_INHERIT);
}
