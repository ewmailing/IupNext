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
#include "iupcontrols.h"

#include "iup_array.h"
#include "iup_object.h"
#include "iup_attrib.h"
#include "iup_str.h"
#include "iup_matrixex.h"

#if 0
void Dmatrix::UpdateSortSelection()
{
  int l(1),c(1);
  IupSetAttribute(mtx,"MARKED",NULL);
  std::string mark;
  std::string yes,no;

  const int nl(IupGetInt(mtx,"NUMLIN")), nc(IupGetInt(mtx,"NUMCOL"));
  for (c=1;c<=nc;++c)
  {
    yes.push_back((c==sorting.col)?'1':'0');
    no .push_back('0');
  }

  for (l=1;l<sorting.fline;++l)
    mark.append(no);
  for (l=sorting.fline;l<=sorting.lline;++l)
    mark.append(yes);
  for (l=sorting.lline+1;l<=nl;++l)
    mark.append(no);
  IupStoreAttribute(mtx,"MARKED",(char*)mark.c_str());
  IupUpdate(mtx);
}

static int iMatrixExSortDialogFindNext_CB(Ihandle* ih_button)
{
  Ihandle* text = IupGetDialogChild(ih_button, "FINDTEXT");
  char* find = IupGetAttribute(text, "VALUE");
  if (find && find[0]!=0)
  {
    ImatExData* matex_data = (ImatExData*)IupGetAttribute(ih_button, "MATRIX_EX_DATA");
    int matchcase = IupGetInt(IupGetDialogChild(ih_button, "MATCHCASE"), "VALUE");
    int matchwholecell = IupGetInt(IupGetDialogChild(ih_button, "MATCHWHOLECELL"), "VALUE");
    int searchbyrow = IupGetInt(IupGetDialogChild(ih_button, "SEARCHBYROW"), "VALUE");

    iupAttribSet(matex_data->ih, "FINDMATCHCASE", matchcase? "Yes": "No");
    iupAttribSet(matex_data->ih, "FINDMATCHWHOLECELL", matchwholecell? "Yes": "No");
    iupAttribSet(matex_data->ih, "FINDDIRECTION", searchbyrow? "RIGHTBOTTOM": "BOTTOMRIGHT");  /* Forward */

    IupSetAttribute(matex_data->ih, "FIND", find);
  }
  return IUP_DEFAULT;
}

static int iMatrixExSortDialogFindPrevious_CB(Ihandle* ih_button)
{
  Ihandle* text = IupGetDialogChild(ih_button, "FINDTEXT");
  char* find = IupGetAttribute(text, "VALUE");
  if (find && find[0]!=0)
  {
    ImatExData* matex_data = (ImatExData*)IupGetAttribute(ih_button, "MATRIX_EX_DATA");
    int matchcase = IupGetInt(IupGetDialogChild(ih_button, "MATCHCASE"), "VALUE");
    int matchwholecell = IupGetInt(IupGetDialogChild(ih_button, "MATCHWHOLECELL"), "VALUE");
    int searchbyrow = IupGetInt(IupGetDialogChild(ih_button, "SEARCHBYROW"), "VALUE");

    iupAttribSet(matex_data->ih, "FINDMATCHCASE", matchcase? "Yes": "No");
    iupAttribSet(matex_data->ih, "FINDMATCHWHOLECELL", matchwholecell? "Yes": "No");
    iupAttribSet(matex_data->ih, "FINDDIRECTION", searchbyrow? "LEFTTOP": "TOPLEFT");  /* Backward */

    IupSetAttribute(matex_data->ih, "FIND", find);
  }
  return IUP_DEFAULT;
}
#endif

static int iMatrixExSortDialogClose_CB(Ihandle* ih_button)
{
  (void)ih_button;
  return IUP_CLOSE;
}

static Ihandle* iMatrixExSortCreateDialog(ImatExData* matex_data)
{
  Ihandle *matrix, *matrix_box, *options_box, *reset, *sort, *invert,
          *dlg, *close, *dlg_box, *button_box, *parent;

  matrix = IupSetAttributes(IupMatrix(NULL),
    "NUMLIN=4, "
    "NUMCOL=1, "
    "NUMLIN_VISIBLE=4, "
    "NUMCOL_VISIBLE=1, "
    "ALIGNMENT0=ALEFT, "
    "ALIGNMENT1=ARIGHT, "
    "SCROLLBAR=NO, "
    "USETITLESIZE=Yes, "
    "NAME=INTERVAL, "
    "HEIGHT0=0, "
    "WIDTH1=40");
  IupSetStrAttributeId2(matrix, "", 1, 0, "_@IUP_COLUMN");
  IupSetStrAttributeId2(matrix, "", 2, 0, "_@IUP_ALLLINES");
  IupSetStrAttributeId2(matrix, "", 3, 0, "_@IUP_FIRSTLINE");
  IupSetStrAttributeId2(matrix, "", 4, 0, "_@IUP_LASTLINE");

  matrix_box = IupVbox(
      matrix,
    NULL);

  options_box = IupVbox(
    IupSetAttributes(IupFrame(IupRadio(IupVbox(
      IupSetAttributes(IupToggle("_@IUP_ASCENDING", NULL), "NAME=ASCENDING"),
      IupSetAttributes(IupToggle("_@IUP_DESCENDING", NULL), "NAME=DESCENDING"),
      NULL))), "TITLE=_@IUP_ORDER, GAP=5, MARGIN=5x5"),
    IupSetAttributes(IupToggle("_@IUP_CASESENSITIVE", NULL), "NAME=CASESENSITIVE"),
    NULL);
  IupSetAttribute(options_box,"GAP","10");

  sort = IupButton("_@IUP_SORT", NULL);
  IupSetAttribute(sort,"PADDING" ,"12x2");
//  IupSetCallback(sort, "ACTION", (Icallback)iMatrixExSortDialogSort_CB);

  reset = IupButton("_@IUP_RESET", NULL);
  IupSetAttribute(reset,"PADDING" ,"12x2");
//  IupSetCallback(reset, "ACTION", (Icallback)iMatrixExSortDialogReset_CB);

  invert = IupButton("_@IUP_INVERT", NULL);
  IupSetAttribute(invert,"PADDING" ,"12x2");
//  IupSetCallback(invert, "ACTION", (Icallback)iMatrixExSortDialogInvert_CB);

  close = IupButton("_@IUP_CLOSE", NULL);
  IupSetAttribute(close,"PADDING" ,"12x2");
  IupSetCallback(close, "ACTION", (Icallback)iMatrixExSortDialogClose_CB);

  button_box = IupHbox(
    IupFill(),
    sort,
    invert,
    reset,
    close,
    NULL);
  IupSetAttribute(button_box,"MARGIN","0x0");
  IupSetAttribute(button_box, "NORMALIZESIZE", "HORIZONTAL");
  IupSetAttribute(button_box,"GAP","5");

  dlg_box = IupVbox(
    IupSetAttributes(IupHbox(
      matrix_box,
      options_box,
      NULL), "MARGIN=0x0, GAP=15"),
    button_box,
    NULL);
  IupSetAttribute(dlg_box,"MARGIN","10x10");
  IupSetAttribute(dlg_box,"GAP","15");

  dlg = IupDialog(dlg_box);

  parent = IupGetDialog(matex_data->ih);

  IupSetStrAttribute(dlg,"TITLE","_@IUP_SORTBYCOLUMN");
  IupSetAttribute(dlg,"MINBOX","NO");
  IupSetAttribute(dlg,"MAXBOX","NO");
  IupSetAttribute(dlg,"BORDER","NO");
  IupSetAttribute(dlg,"RESIZE","NO");
  IupSetAttribute(dlg, "TOOLBOX","YES");
  IupSetAttributeHandle(dlg,"DEFAULTENTER", sort);
  IupSetAttributeHandle(dlg,"DEFAULTESC", close);
  IupSetAttributeHandle(dlg,"PARENTDIALOG", parent);

  IupSetAttribute(dlg, "MATRIX_EX_DATA", (char*)matex_data);  /* do not use "_IUP_MATEX_DATA" to enable inheritance */

  if (IupGetAttribute(parent, "ICON"))
    IupSetAttribute(dlg,"ICON", IupGetAttribute(parent, "ICON"));
  else
    IupSetAttribute(dlg,"ICON", IupGetGlobal("ICON"));

  return dlg;
}

void iupMatrixExSortShowDialog(ImatExData* matex_data)
{
  int x, y, col, lin1, lin2, num_lin;
  Ihandle* matrix;
  Ihandle* dlg_sort = iMatrixExSortCreateDialog(matex_data);
           
  IupSetAttribute(IupGetDialogChild(dlg_sort, "CASESENSITIVE"), "VALUE", iupAttribGetStr(matex_data->ih, "SORTCOLUMNCASESENSITIVE"));

  if (iupStrEqualNoCase(iupAttribGetStr(matex_data->ih, "SORTCOLUMNORDER"), "DESCENDING"))
    IupSetAttribute(IupGetDialogChild(dlg_sort, "DESCENDING"), "VALUE", "Yes");
  else
    IupSetAttribute(IupGetDialogChild(dlg_sort, "ASCENDING"), "VALUE", "Yes");

  matrix = IupGetDialogChild(dlg_sort, "INTERVAL");

  col = IupGetInt2(matex_data->ih, "FOCUS_CELL");
  IupSetIntId2(matrix, "", 1, 1, col);
  num_lin = IupGetInt(matex_data->ih, "NUMLIN");
  lin1 = 1;
  lin2 = num_lin;
  IupGetIntInt(matex_data->ih, "SORTCOLUMNINTERVAL", &lin1, &lin2);
  if (lin1 < 1) lin1 = 1;
  if (lin2 > num_lin) lin2 = num_lin;
  if (lin1 > lin2) lin1 = lin2;
  if (lin1==1 && lin2==num_lin)
  {
    IupSetStrAttributeId2(matrix, "", 2, 1, "_@IUP_YES");
    IupSetStrAttributeId2(matrix, "TOGGLEVALUE", 2, 1, "Yes");
  }
  else
  {
    IupSetStrAttributeId2(matrix, "", 2, 1, "_@IUP_NO");
    IupSetStrAttributeId2(matrix, "TOGGLEVALUE", 2, 1, "No");
  }
  IupSetIntId2(matrix, "", 3, 1, lin1);
  IupSetIntId2(matrix, "", 4, 1, lin2);
  
  iupMatrixExGetDialogPosition(matex_data, &x, &y);
  IupPopup(dlg_sort, x, y);
  IupDestroy(dlg_sort);
}

void iupMatrixExRegisterSort(Iclass* ic)
{
  /* Defined in IupMatrix - Exported
    SORTCOLUMN   (RESET, INVERT, lin1-lin2)
    SORTCOLUMNORDER  (ASCENDING, DESCENDING)
    SORTCOLUMNCASESENSITIVE (Yes, No) */
  
  if (iupStrEqualNoCase(IupGetGlobal("LANGUAGE"), "ENGLISH"))
  {
    IupSetLanguageString("IUP_SORTBYCOLUMN", "Sort Lines by Column");
    IupSetLanguageString("IUP_INVERT", "Invert");
    IupSetLanguageString("IUP_RESET", "Reset");
    IupSetLanguageString("IUP_SORT", "Sort");

    IupSetLanguageString("IUP_ORDER", "Order");
    IupSetLanguageString("IUP_ASCENDING", "Ascending");
    IupSetLanguageString("IUP_DESCENDING", "Descending");

    IupSetLanguageString("IUP_ALLLINES", "All Lines");
    IupSetLanguageString("IUP_CASESENSITIVE", "Case Sensitive");
    IupSetLanguageString("IUP_COLUMN", "Column");
    IupSetLanguageString("IUP_FIRSTLINE", "First Line");
    IupSetLanguageString("IUP_LASTLINE", "Last Line");
  }
  else if (iupStrEqualNoCase(IupGetGlobal("LANGUAGE"), "PORTUGUESE"))
  {
    IupSetLanguageString("IUP_SORTBYCOLUMN", "Classificar Linhas por Coluna");
    IupSetLanguageString("IUP_INVERT", "Inverter");
    IupSetLanguageString("IUP_RESET", "Reiniciar");
    IupSetLanguageString("IUP_SORT", "Ordenar");

    IupSetLanguageString("IUP_ORDER", "Ordem");
    IupSetLanguageString("IUP_ASCENDING", "Ascendente");
    IupSetLanguageString("IUP_DESCENDING", "Descendente");

    IupSetLanguageString("IUP_ALLLINES", "Todas as Linhas");
    IupSetLanguageString("IUP_CASESENSITIVE", "Diferenciar maiúsculas e minúsculas");
    IupSetLanguageString("IUP_COLUMN", "Coluna");
    IupSetLanguageString("IUP_FIRSTLINE", "Primeira Linha");
    IupSetLanguageString("IUP_LASTLINE", "Última Linha");

    if (IupGetInt(NULL, "UTF8MODE"))
    {
      /* When seeing this file assuming ISO8859-1 encoding, above will appear correct.
         When seeing this file assuming UTF-8 encoding, bellow will appear correct. */

      IupSetLanguageString("IUP_CASESENSITIVE", "Diferenciar maiÃºsculas e minÃºsculas");
      IupSetLanguageString("IUP_LASTLINE", "Ãšltima Linha");
    }
  }

  (void)ic;
}
