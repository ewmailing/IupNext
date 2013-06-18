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


static void iMatrixExReplaceSep(char *txt, int count)
{
  int i;
  for (i=0; i<count; i++)
  {
    if (txt[i]=='\n') txt[i]=' ';
    if (txt[i]=='\t') txt[i]=' ';
  }
}

int iupMatrixExIsColumnVisible(Ihandle* ih, int col)
{
  int width = 0;
  char str[100];
  char* value;

  if (col==0)
    return (IupGetIntId(ih, "RASTERWIDTH", 0) != 0);

  /* to be invisible must exist the attribute and must be set to 0 (zero), 
     or else is visible */

  sprintf(str, "WIDTH%d", col);
  value = iupAttribGet(ih, str);
  if (!value)
  {
    sprintf(str, "RASTERWIDTH%d", col);
    value = iupAttribGet(ih, str);
    if (!value)
      return 1;
  }

  if (iupStrToInt(value, &width)==1)
  {
    if (width==0)
      return 0;
  }

  return 1;
}

int iupMatrixExIsLineVisible(Ihandle* ih, int lin)
{
  int height = 0;
  char str[100];
  char* value;

  if (lin==0)
    return (IupGetIntId(ih, "RASTERHEIGHT", 0) != 0);

  sprintf(str, "HEIGHT%d", lin);
  value = iupAttribGet(ih, str);
  if(!value)
  {
    sprintf(str, "RASTERHEIGHT%d", lin);
    value = iupAttribGet(ih, str);
    if(!value)
      return 1;
  }

  if (iupStrToInt(value, &height)==1)
  {
    if (height==0)
      return 0;
  }

  return 1;
}

static void iMatrixExCellMarkedLimits(const char* marked, int num_lin, int num_col, int *lin1, int *lin2, int *col1, int *col2)
{
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

static void iMatrixExCopyChar(Iarray* data, char c)
{
  int last_count = iupArrayCount(data);
  char* str_data = (char*)iupArrayInc(data);
  str_data[last_count] = c;
}

static void iMatrixExCopyEmpty(Iarray* data)
{
  iMatrixExCopyChar(data, ' ');
}

static void iMatrixExCopyStr(Iarray* data, char* str)
{
  int add_count = strlen(str);
  int last_count = iupArrayCount(data);
  char* str_data = (char*)iupArrayAdd(data, add_count);
  memcpy(str_data+last_count, str, add_count);
  iMatrixExReplaceSep(str_data+last_count, add_count);
}

static void iMatrixExCopyCell(Ihandle* ih, Iarray* data, int lin, int col, sIFnii value_cb)
{
  char* value;
  if (!value_cb)
    value = IupGetAttributeId2(ih, "", lin, col);
  else
    value = value_cb(ih, lin, col);

  if (value)
    iMatrixExCopyStr(data, value);
  else
    iMatrixExCopyEmpty(data);
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

  char *marked = IupGetAttribute(ih,"MARKED");
  if (!marked)  /* no marked cells */
    return 0;

  (void)value;
  iupAttribSetStr(ih, "COPYERROR", NULL);

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
              iMatrixExCopyChar(data, '\t');

            iMatrixExCopyCell(ih, data, lin, col, value_cb);
            add_sep = 1;
          }
        }
      }

      if (add_sep)
        iMatrixExCopyChar(data, '\n');
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
              iMatrixExCopyChar(data, '\t');

            iMatrixExCopyCell(ih, data, lin, col, value_cb);
            add_sep = 1;
          }
        }
      }

      if (add_sep)
        iMatrixExCopyChar(data, '\n');
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
      iupAttribSetStr(ih, "COPYERROR", "MARKEDCONSISTENCY");
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
                iMatrixExCopyChar(data, '\t');
              iMatrixExCopyCell(ih, data, lin, col, value_cb);
              add_sep = 1;
            }
            else if (keep_struct)
            {
              if (add_sep)
                iMatrixExCopyChar(data, '\t');
              iMatrixExCopyEmpty(data);
              add_sep = 1;
            }
          }
        }
      }

      if (add_sep)
        iMatrixExCopyChar(data, '\n');
    }
  }

  if (iupArrayCount(data)!=0)
  {
    Ihandle* clipboard = IupClipboard();
    IupSetAttribute(clipboard, "TEXT", NULL);  /* clear all data from clipboard */
    iMatrixExCopyChar(data, '\0');
    IupSetAttribute(clipboard, "TEXT", (char*)iupArrayGetData(data));
    IupDestroy(clipboard);
  }

  iupArrayDestroy(data);

  return 0;
}

static int iMatrixExGetDataSize(const char* data, int *num_lin, int *num_col, char sep)
{
  *num_lin = iupStrLineCount(data);

  if (sep != 0)
    *num_col = iupStrCountChar(data, (int)sep);
  else
  {
    *num_col = iupStrCountChar(data, (int)'\t');
    if (*num_col == 0)
      *num_col = iupStrCountChar(data, (int)';');
  }

  if (*num_lin == 0 ||
      *num_col == 0)
    return 0;

  if ((*num_col)%(*num_lin)!=0)
    return 0;

  *num_col = (*num_col)/(*num_lin) + 1;
  return 1;
}

static void iMatrixExSetData(Ihandle *ih, const char* data, int data_num_lin, int data_num_col, char sep, int lin, int col)
{
  iMatrixExPushUndoBlock(ih, lin, col, data_num_lin, data_num_col);

  iMatrixExParseText(ih, data, data_num_lin, data_num_col, sep, lin, col);

  IupSetAttribute(ih,"REDRAW","ALL");
#if 0
  int usegauge(0);
  int ncels(undo->_nlins()*undo->_ncols());
  if (busy_callback !=NULL)
    busy_callback(ih,1,ncels,DMATRIX_PASTE,&usegauge);

  if (usegauge)
  {
    gauge.SetTitle("Copy all");
    gauge.Start(dlg,ncels);
  }

  if (busy_callback !=NULL)
    busy_callback(ih,0,undo->_nlins(),undo->_ncols(),&usegauge);
#endif
}

static int iMatrixExSetPasteDataAttrib(Ihandle *ih, const char* data)
{
  int lin=0, col=0, num_lin, num_col,
      data_num_lin, data_num_col;
  char* sep;

  if (!data)
  {
    iupAttribSetStr(ih, "PASTEERROR", "NOTEXT");
    return 0;
  }

  sep = IupGetAttribute(ih, "TEXTSEPARATOR");
  if (!sep) sep = "";

  if (!iMatrixExGetDataSize(data, &data_num_lin, &data_num_col, *sep))
  {
    iupAttribSetStr(ih, "PASTEERROR", "INVALIDMATRIX");
    return 0;
  }

  num_lin = IupGetInt(ih, "NUMLIN");
  num_col = IupGetInt(ih, "NUMCOL");

  IupGetIntInt(ih, "FOCUS_CELL", &lin, &col);

  if (lin+data_num_lin-1>num_lin ||
      col+data_num_col-1>num_col)
  {
    IFnii cb = (IFnii)IupGetCallback(ih, "PASTESIZE_CB");
    if (cb)
    {
      int ret = cb(ih, lin+data_num_lin-1, col+data_num_col-1);
      if (ret == IUP_IGNORE)
        return 0;
    }
  }

  iMatrixExSetData(ih, data, data_num_lin, data_num_col, *sep, lin, col);
  return 0;
}

static int iMatrixExSetPasteAttrib(Ihandle *ih, const char* value)
{
  int lin=0, col=0, num_lin, num_col,
      data_num_lin, data_num_col;
  char* sep;

  Ihandle* clipboard = IupClipboard();
  char* data = IupGetAttribute(clipboard, "TEXT");
  IupDestroy(clipboard);

  if (!data)
  {
    iupAttribSetStr(ih, "PASTEERROR", "NOTEXT");
    return 0;
  }

  sep = IupGetAttribute(ih, "TEXTSEPARATOR");
  if (!sep) sep = "";

  if (!iMatrixExGetDataSize(data, &data_num_lin, &data_num_col, *sep))
  {
    iupAttribSetStr(ih, "PASTEERROR", "INVALIDMATRIX");
    return 0;
  }

  num_lin = IupGetInt(ih, "NUMLIN");
  num_col = IupGetInt(ih, "NUMCOL");

  if (iupStrBoolean(value))
    IupGetIntInt(ih, "FOCUS_CELL", &lin, &col);
  else if (iupStrEqualNoCase(value, "SELECTION"))
  {
    int lin2, col2;
    char *marked = IupGetAttribute(ih,"MARKED");
    if (marked)
    {
      iMatrixExCellMarkedLimits(marked, num_lin, num_col, &lin, &lin2, &col, &col2);
      if (lin2-lin+1!=data_num_lin ||
          col2-col+1!=data_num_col)
      {
        iupAttribSetStr(ih, "PASTEERROR", "INVALIDSELECTION");
        return 0;
      }
    }
  }
  else
  {
    if (iupStrToIntInt(value, &lin, &col, ':')!=2)
      return 0;
  }

  if (lin+data_num_lin-1>num_lin ||
      col+data_num_col-1>num_col)
  {
    IFnii cb = (IFnii)IupGetCallback(ih, "PASTESIZE_CB");
    if (cb)
    {
      int ret = cb(ih, lin+data_num_lin-1, col+data_num_col-1);
      if (ret == IUP_IGNORE)
        return 0;
    }
  }

  iMatrixExSetData(ih, data, data_num_lin, data_num_col, *sep, lin, col);
  return 0;
}

static int iMatrixExSetImportTextAttrib(Ihandle *ih, const char* value)
{
  int skip_lines;
  size_t size;
  char* data;

  FILE *file = fopen(value, "rb");
  if (!file)
  {
    iupAttribSetStr(ih, "IMPORTTEXTERROR", "INVALIDFILENAME");
    return 0;
  }

  fseek(file, 0, SEEK_END);
  size = (size_t)ftell(file); 
  fseek(file, 0, SEEK_SET);

  data = (char*)malloc(size+1);
  fread(data, size, 1, file);
  data[size] = 0;
  fclose(file);

  skip_lines = IupGetInt(ih, "IMPORTTEXTSKIPLINES");
  if (skip_lines)
  {
    int i, len;
    const char *pdata;
    for (i=0; i<skip_lines; i++)
    {
      pdata = iupStrNextLine(data, &len);
      if (pdata==data) /* no next line */ 
        return 0;
      data = (char*)pdata;
    }
  }

  iMatrixExSetPasteDataAttrib(ih, data);

  free(data);
  return 0;
}

void iupMatrixExRegisterClipboard(Iclass* ic)
{
  iupClassRegisterCallback(ic, "PASTESIZE_CB", "ii");

  iupClassRegisterAttribute(ic, "IMPORTTEXT", NULL, iMatrixExSetImportTextAttrib, NULL, NULL, IUPAF_WRITEONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "IMPORTTEXTERROR", NULL, NULL, NULL, NULL, IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "IMPORTTEXTSKIPLINES", NULL, NULL, NULL, NULL, IUPAF_NO_INHERIT);

  iupClassRegisterAttribute(ic, "PASTE", NULL, iMatrixExSetPasteAttrib, NULL, NULL, IUPAF_WRITEONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "PASTEDATA", NULL, iMatrixExSetPasteDataAttrib, NULL, NULL, IUPAF_WRITEONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "PASTEERROR", NULL, NULL, NULL, NULL, IUPAF_NO_INHERIT);

  iupClassRegisterAttribute(ic, "COPY", NULL, iMatrixExSetCopyAttrib, NULL, NULL, IUPAF_WRITEONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "COPYKEEPSTRUCT", NULL, NULL, NULL, NULL, IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "COPYERROR", NULL, NULL, NULL, NULL, IUPAF_NO_INHERIT);

  iupClassRegisterAttribute(ic, "TEXTSEPARATOR", NULL, NULL, NULL, NULL, IUPAF_NO_INHERIT);
}
