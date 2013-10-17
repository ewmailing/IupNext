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
#endif

static int iMatrixExSortDialogSort_CB(Ihandle* ih_button)
{
  ImatExData* matex_data = (ImatExData*)IupGetAttribute(ih_button, "MATRIX_EX_DATA");
  Ihandle* matrix = IupGetDialogChild(ih_button, "INTERVAL");
  int col = IupGetIntId2(matrix, "", 1, 1);

  int casesensitive = IupGetInt(IupGetDialogChild(ih_button, "CASESENSITIVE"), "VALUE");
  int ascending = IupGetInt(IupGetDialogChild(ih_button, "ASCENDING"), "VALUE");

  iupAttribSet(matex_data->ih, "SORTCOLUMNCASESENSITIVE", casesensitive? "Yes": "No");
  iupAttribSet(matex_data->ih, "SORTCOLUMNORDER", ascending? "ASCENDING": "DESCENDING");

  if (IupGetIntId2(matrix, "TOGGLEVALUE", 2, 1))
    IupSetAttributeId(matex_data->ih, "SORTCOLUMN", col, "ALL");
  else
  {
    int lin1 = IupGetIntId2(matrix, "", 3, 1);
    int lin2 = IupGetIntId2(matrix, "", 4, 1);
    IupSetfAttributeId(matex_data->ih, "SORTCOLUMN", col, "%d-%d", lin1, lin2);
  }

  return IUP_DEFAULT;
}

static int iMatrixExSortDialogReset_CB(Ihandle* ih_button)
{
  ImatExData* matex_data = (ImatExData*)IupGetAttribute(ih_button, "MATRIX_EX_DATA");
  IupSetAttribute(matex_data->ih, "SORTCOLUMN", "RESET");
  return IUP_DEFAULT;
}

static int iMatrixExSortDialogInvert_CB(Ihandle* ih_button)
{
  ImatExData* matex_data = (ImatExData*)IupGetAttribute(ih_button, "MATRIX_EX_DATA");
  IupSetAttribute(matex_data->ih, "SORTCOLUMN", "INVERT");
  return IUP_DEFAULT;
}

static int iMatrixExSortDialogClose_CB(Ihandle* ih_button)
{
  (void)ih_button;
  return IUP_CLOSE;
}

static int iMatrixExSortToggleValue_CB(Ihandle *ih_matrix, int lin, int col, int status)
{
  if (lin!=2 || col!=1)
    return IUP_DEFAULT;

  if (status) /* All Lines? disable lin1 && lin2 */
  {
    int num_lin = IupGetInt(ih_matrix, "NUMLIN");
    IupSetStrAttributeId2(ih_matrix, "BGCOLOR", 3, 1, "220 220 220");
    IupSetStrAttributeId2(ih_matrix, "BGCOLOR", 4, 1, "220 220 220");
    IupSetIntId2(ih_matrix, "", 3, 1, 1);
    IupSetIntId2(ih_matrix, "", 4, 1, num_lin);
  }
  else
  {
    IupSetStrAttributeId2(ih_matrix, "BGCOLOR", 3, 1, NULL);
    IupSetStrAttributeId2(ih_matrix, "BGCOLOR", 4, 1, NULL);
  }

  IupSetAttribute(ih_matrix, "REDRAW", "ALL");

  return IUP_DEFAULT;
}

static int iMatrixExSortDropCheck_CB(Ihandle *ih_matrix, int lin, int col)
{
  (void)ih_matrix;

  if (lin==2 && col==1)
    return IUP_CONTINUE;

  return IUP_IGNORE;
}

static int iMatrixExSortEdition_CB(Ihandle *ih_matrix, int lin, int col, int mode, int update)
{
  if (mode==1)
  {
    if (lin==2 && col==1)
      return IUP_IGNORE;
    else if ((lin==3 && col==1) ||
             (lin==4 && col==1))
    {
      if (IupGetIntId2(ih_matrix, "TOGGLEVALUE", 2, 1))
        return IUP_IGNORE;
    }
  }
  else /* mode==0 */
  {
    if (lin==1 && col==1)
    {
      int num_col = IupGetInt(ih_matrix, "NUMCOL");
      col = IupGetIntId2(ih_matrix, "", 1, 1);
      if (col < 1 || col > num_col)
        return IUP_IGNORE;
    }
    else if ((lin==3 && col==1) ||
             (lin==4 && col==1))
    {
      int lin1 = IupGetIntId2(ih_matrix, "", 3, 1);
      int lin2 = IupGetIntId2(ih_matrix, "", 4, 1);
      int num_lin = IupGetInt(ih_matrix, "NUMLIN");
      if (lin1 < 1 || lin1 > lin2 || lin2 > num_lin)
        return IUP_IGNORE;
    }
  }

  (void)update;
  return IUP_DEFAULT;
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
    "MASK1:1=/d+, "
    "MASK3:1=/d+, "
    "MASK4:1=/d+, "
    "HEIGHT0=0, "
    "WIDTH1=40");
  IupSetStrAttributeId2(matrix, "", 1, 0, "_@IUP_COLUMN");
  IupSetStrAttributeId2(matrix, "", 2, 0, "_@IUP_ALLLINES");
  IupSetStrAttributeId2(matrix, "", 3, 0, "_@IUP_FIRSTLINE");
  IupSetStrAttributeId2(matrix, "", 4, 0, "_@IUP_LASTLINE");
  IupSetCallback(matrix, "TOGGLEVALUE_CB", (Icallback)iMatrixExSortToggleValue_CB);
  IupSetCallback(matrix, "DROPCHECK_CB", (Icallback)iMatrixExSortDropCheck_CB);
  IupSetCallback(matrix, "EDITION_CB", (Icallback)iMatrixExSortEdition_CB);

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
  IupSetCallback(sort, "ACTION", (Icallback)iMatrixExSortDialogSort_CB);

  invert = IupButton("_@IUP_INVERT", NULL);
  IupSetAttribute(invert,"PADDING" ,"12x2");
  IupSetCallback(invert, "ACTION", (Icallback)iMatrixExSortDialogInvert_CB);
  IupSetStrAttribute(invert,"TIP" ,"_@IUP_INVERT_TIP");

  reset = IupButton("_@IUP_RESET", NULL);
  IupSetAttribute(reset,"PADDING" ,"12x2");
  IupSetCallback(reset, "ACTION", (Icallback)iMatrixExSortDialogReset_CB);

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
    IupSetStrAttributeId2(matrix, "TOGGLEVALUE", 2, 1, "Yes");
    IupSetStrAttributeId2(matrix, "BGCOLOR", 3, 1, "220 220 220");
    IupSetStrAttributeId2(matrix, "BGCOLOR", 4, 1, "220 220 220");
  }
  else
    IupSetStrAttributeId2(matrix, "TOGGLEVALUE", 2, 1, "No");
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
    
    IupSetLanguageString("IUP_INVERT_TIP", "Just invert the current sort. New parameters are ignored.");

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
    
    IupSetLanguageString("IUP_INVERT_TIP", "Apenas inverte a ordenação corrente. Novos parâmetros são ignorados.");

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

      IupSetLanguageString("IUP_INVERT_TIP", "Apenas inverte a ordenaÃ§Ã£o corrente. Novos parÃ¢metros sÃ£o ignorados.");
      IupSetLanguageString("IUP_CASESENSITIVE", "Diferenciar maiÃºsculas e minÃºsculas");
      IupSetLanguageString("IUP_LASTLINE", "Ãšltima Linha");
    }
  }

  (void)ic;
}
