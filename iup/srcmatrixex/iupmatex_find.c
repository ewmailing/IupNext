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


void iupMatrixExFindInitDialog(Ihandle *ih)
{
}

static int iMatrixExFindDialogFindNext_CB(Ihandle* ih)
{
  Ihandle* text = IupGetDialogChild(ih, "FINDTEXT");
  char* find = IupGetAttribute(text, "VALUE");
  if (find && find[0]!=0)
  {
    int matchcase = IupGetInt(IupGetDialogChild(ih, "MATCHCASE"), "VALUE");
    int matchwholecell = IupGetInt(IupGetDialogChild(ih, "MATCHWHOLECELL"), "VALUE");
    int searchbyrow = IupGetInt(IupGetDialogChild(ih, "SEARCHBYROW"), "VALUE");

    iupAttribSet(ih, "FINDMATCHCASE", matchcase? "Yes": "No");
    iupAttribSet(ih, "FINDMATCHWHOLECELL", matchwholecell? "Yes": "No");
    iupAttribSet(ih, "FINDDIRECTION", searchbyrow? "RIGHTBOTTOM": "BOTTOMRIGHT");

    IupSetAttribute(ih, "FIND", find);
  }
  return IUP_DEFAULT;
}

static int iMatrixExFindDialogFindPrevious_CB(Ihandle* ih)
{
  Ihandle* text = IupGetDialogChild(ih, "FINDTEXT");
  char* find = IupGetAttribute(text, "VALUE");
  if (find && find[0]!=0)
  {
    int matchcase = IupGetInt(IupGetDialogChild(ih, "MATCHCASE"), "VALUE");
    int matchwholecell = IupGetInt(IupGetDialogChild(ih, "MATCHWHOLECELL"), "VALUE");
    int searchbyrow = IupGetInt(IupGetDialogChild(ih, "SEARCHBYROW"), "VALUE");

    iupAttribSet(ih, "FINDMATCHCASE", matchcase? "Yes": "No");
    iupAttribSet(ih, "FINDMATCHWHOLECELL", matchwholecell? "Yes": "No");
    iupAttribSet(ih, "FINDDIRECTION", searchbyrow? "LEFTTOP": "TOPLEFT");

    IupSetAttribute(ih, "FIND", find);
  }
  return IUP_DEFAULT;
}

static int iMatrixExFindDialogClose_CB(Ihandle* ih)
{
  IupHide(IupGetDialog(ih));
  return IUP_DEFAULT;
}

Ihandle* iupMatrixExFindCreateDialog(Ihandle *ih)
{
  Ihandle *text_box, *options_box, *find_next, *find_prev, 
          *dlg, *close, *dlg_box, *button_box, *parent;

  text_box = IupSetAttributes(IupHbox(
    IupLabel("_@IUP_FIND_WHAT"), 
    IupSetAttributes(IupText(NULL), "EXPAND=HORIZONTAL, NAME=FINDTEXT"),
    NULL), "ALIGNMENT=ACENTER");
  IupSetAttribute(text_box,"MARGIN","0x0");

  options_box = IupHbox(
    IupVbox(
      IupSetAttributes(IupToggle("_@IUP_MATCH_CASE", NULL), "NAME=MATCHCASE"),
      IupSetAttributes(IupToggle("_@IUP_MATCH_WHOLE_CELL", NULL), "NAME=MATCHWHOLECELL"),
      NULL),
    IupSetAttributes(IupFrame(IupRadio(IupHbox(
      IupSetAttributes(IupToggle("_@IUP_BY_ROW", NULL), "NAME=SEARCHBYROW"),
      IupToggle("_@IUP_BY_COL", NULL), 
      NULL))), "TITLE=_@IUP_SEARCH"),
    NULL);
  IupSetAttribute(options_box,"MARGIN","5x5");

  find_prev = IupButton("_@IUP_FIND_PREVIOUS", NULL);
  IupSetAttribute(find_prev,"PADDING" ,"12x2");
  IupSetCallback(find_prev, "ACTION", (Icallback)iMatrixExFindDialogFindNext_CB);

  find_next = IupButton("_@IUP_FIND_NEXT", NULL);
  IupSetAttribute(find_next,"PADDING" ,"12x2");
  IupSetCallback(find_next, "ACTION", (Icallback)iMatrixExFindDialogFindPrevious_CB);

  close = IupButton("_@IUP_CLOSE", NULL);
  IupSetAttribute(close,"PADDING" ,"12x2");
  IupSetCallback(close, "ACTION", (Icallback)iMatrixExFindDialogClose_CB);

  button_box = IupHbox(
    IupFill(), 
    find_prev,
    find_next,
    close,
    NULL);
  IupSetAttribute(button_box,"MARGIN","0x0");
  IupSetAttribute(button_box, "NORMALIZESIZE", "HORIZONTAL");

  dlg_box = IupVbox(
    text_box,
    options_box,
    button_box,
    NULL);
  IupSetAttribute(dlg_box,"MARGIN","10x10");
  IupSetAttribute(dlg_box,"GAP","5");

  dlg = IupDialog(dlg_box);

  parent = IupGetDialog(ih);

  IupSetStrAttribute(dlg,"TITLE","_@IUP_FIND");
  IupSetAttribute(dlg,"MINBOX","NO");
  IupSetAttribute(dlg,"MAXBOX","NO");
  IupSetAttribute(dlg, "TOOLBOX","YES");
  IupSetAttributeHandle(dlg,"DEFAULTENTER", find_next);
  IupSetAttributeHandle(dlg,"DEFAULTESC", close);
  IupSetAttributeHandle(dlg,"PARENTDIALOG", parent);

  if (IupGetAttribute(parent, "ICON"))
    IupSetAttribute(dlg,"ICON", IupGetAttribute(parent, "ICON"));
  else
    IupSetAttribute(dlg,"ICON", IupGetGlobal("ICON"));

  return dlg;
}

static int iMatrixMatch(Ihandle *ih, const char* findvalue, int lin, int col, int matchcase, int matchwholecell, int utf8)
{
  char* value = iupMatrixExGetCellValue(ih, lin, col, 1);  /* get displayed value */
  if (!value || value[0] == 0)
    return 0;

  if (matchwholecell)
    return iupStrCompareEqual(value, findvalue, matchcase, utf8, 0);
  else
    return iupStrCompareFind(value, findvalue, matchcase, utf8);
}

static int iMatrixExSetFind(Ihandle *ih, const char* value, int inc, int flip, int matchcase, int matchwholecell, int *lin, int *col)
{
  int utf8 = IupGetInt(NULL, "UTF8MODE");
  int num_lin = IupGetInt(ih, "NUMLIN");
  int num_col = IupGetInt(ih, "NUMCOL");
  int count = (num_lin+1)*(num_col+1);
  int pos, start_pos;

  if (inc == 0)
  {
    /* the FOCUS_CELL is always visible and not a title */
    if (iMatrixMatch(ih, value, *lin, *col, matchcase, matchwholecell, utf8))
      return 1;

    inc = 1;
  }

  if (flip)
    pos = (*col)*(num_lin+1) + *lin;
  else
    pos = (*lin)*(num_col+1) + *col;

  start_pos = pos;
  do 
  {
    pos += inc;

    if (pos < 0) pos = count-1;   /* if less than first cell, go to last */
    if (pos > count-1) pos = 0;   /* if more than last cell, go to first */

    if (flip)
    {
      *lin = pos % (num_lin+1);
      *col = pos / (num_lin+1);
    }
    else
    {
      *lin = pos / (num_col+1);
      *col = pos % (num_col+1);
    }

    if (*lin == 0 || *col == 0)  /* dont't search on titles */
      continue;

    if (pos == start_pos)
      return 0;

    if (!iupMatrixExIsLineVisible(ih, *lin) || !iupMatrixExIsColumnVisible(ih, *col))
      continue;

  } while (!iMatrixMatch(ih, value, *lin, *col, matchcase, matchwholecell, utf8));

  return 1;
}

static int iMatrixExSetFindAttrib(Ihandle *ih, const char* value)
{
  int lin=1, col=1;
  int inc, flip, matchcase, matchwholecell;
  char* direction;

  if (!value || value[0]==0)
    return 0;

  IupGetIntInt(ih, "FOCUS_CELL", &lin, &col);

  direction = iupAttribGet(ih, "FINDDIRECTION");
  if (iupStrEqualNoCase(direction, "LEFTTOP"))
  {
    flip = 0;
    inc = -1;
  }
  else if (iupStrEqualNoCase(direction, "TOPLEFT"))
  {
    flip = 1;
    inc = -1;
  }
  else if (iupStrEqualNoCase(direction, "BOTTOMRIGHT"))
  {
    flip = 1;
    inc = +1;
  }
  else  /* RIGHTBOTTOM */
  {
    flip = 0;
    inc = +1;
  }
  matchcase = iupAttribGetInt(ih, "FINDMATCHCASE");
  matchwholecell = iupAttribGetInt(ih, "FINDMATCHWHOLECELL");

  if (inc == 1)
  {
    int last_lin=0, last_col=0;
    IupGetIntInt(ih, "_IUP_LAST_FOUND", &last_lin, &last_col);
    if (last_lin==lin && last_col==col)
      inc = 0;  /* search in the current cell */
  }

  if (iMatrixExSetFind(ih, value, inc, flip, matchcase, matchwholecell, &lin, &col))
  {
    IupSetfAttribute(ih,"FOCUS_CELL", "%d:%d", lin, col);
//    IupSetfAttribute(ih,"SHOW", "%d:%d", lin, col);
    //  IupSetAttribute(ih,"MARKED",NULL);
    //  IupSetAttributeId2(ih,"MARK",lin,col,"1");
    IupSetfAttribute(ih,"_IUP_LAST_FOUND", "%d:%d", lin, col);
  }
  else
    IupSetAttribute(ih,"_IUP_LAST_FOUND", NULL);

  return 1;
}

void iupMatrixExRegisterFind(Iclass* ic)
{
  iupClassRegisterAttribute(ic, "FIND", NULL, iMatrixExSetFindAttrib, NULL, NULL, IUPAF_NO_INHERIT);

  iupClassRegisterAttribute(ic, "FINDDIRECTION", NULL, NULL, IUPAF_SAMEASSYSTEM, "RIGHTBOTTOM", IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "FINDMATCHCASE", NULL, NULL, IUPAF_SAMEASSYSTEM, "Yes", IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "FINDMATCHWHOLECELL", NULL, NULL, IUPAF_SAMEASSYSTEM, "Yes", IUPAF_NO_INHERIT);

  if (iupStrEqualNoCase(IupGetGlobal("LANGUAGE"), "ENGLISH"))
  {
    IupSetLanguageString("IUP_FIND", "Find");
    IupSetLanguageString("IUP_FIND_WHAT", "Find what:");
    IupSetLanguageString("IUP_FIND_NEXT", "&Find Next");
    IupSetLanguageString("IUP_FIND_PREVIOUS", "Find &Previous");

    IupSetLanguageString("IUP_CLOSE", "Close");
    IupSetLanguageString("IUP_SEARCH", "Search");

    IupSetLanguageString("IUP_MATCH_CASE", "Match Case");
    IupSetLanguageString("IUP_MATCH_WHOLE_CELL", "Match Whole Cell");
    IupSetLanguageString("IUP_BY_ROW", "by Rows");
    IupSetLanguageString("IUP_BY_COL", "by Columns");
  }
  else if (iupStrEqualNoCase(IupGetGlobal("LANGUAGE"), "PORTUGUESE"))
  {
    IupSetLanguageString("IUP_FIND", "Localizar");
    IupSetLanguageString("IUP_FIND_WHAT", "Localizar o que:");
    IupSetLanguageString("IUP_FIND_NEXT", "Localizar &Próximo");
    IupSetLanguageString("IUP_FIND_PREVIOUS", "Localizar &Anterior");

    IupSetLanguageString("IUP_CLOSE", "Fechar");
    IupSetLanguageString("IUP_SEARCH", "Pesquisar");

    IupSetLanguageString("IUP_MATCH_CASE", "Diferenciar maiúsculas e minúsculas");
    IupSetLanguageString("IUP_MATCH_WHOLE_CELL", "Coindidir célula inteira");
    IupSetLanguageString("IUP_BY_ROW", "por Linhas");
    IupSetLanguageString("IUP_BY_COL", "por Colunas");

    if (IupGetInt(NULL, "UTF8MODE"))
    {
      /* When seeing this file assuming ISO8859-1 encoding, above will appear correct.
         When seeing this file assuming UTF-8 encoding, bellow will appear correct. */

      IupSetLanguageString("IUP_FIND_NEXT", "Localizar &PrÃ³ximo");
      IupSetLanguageString("IUP_MATCH_CASE", "Diferenciar maiÃºsculas e minÃºsculas");
      IupSetLanguageString("IUP_MATCH_WHOLE_CELL", "Coindidir cÃ©lula inteira");
    }
  }
}
