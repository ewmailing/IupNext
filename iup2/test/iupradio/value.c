/*
** ---------------------------------------------------------------
** Includes:
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "iup.h"

/*
** ---------------------------------------------------------------
** Global Variables:
*/

static Ihandle* hMainDialog = NULL; // handle iup do dialogo principal
static Ihandle* hEscDialog = NULL;  // handle iup do dialogo de escaloes
static Ihandle* hEscRadio = NULL;   // handle iup do radio com a lista de escaloes
static Ihandle* hEscalaoTxt = NULL; // handle iup da caixa de texto com o nome do escalo selecionado
static Ihandle* hItemTxt = NULL;    // handle iup da caixa de texto com o numero no item selecionado na lista de escaloes

static char* Esc_Desc = NULL;       // Nome do escalao corrente

/*
** ---------------------------------------------------------------
** Iup Callbacks:
*/

int MainDlgMapCB_(Ihandle* self)
{
  IupSetAttribute(hItemTxt, IUP_VALUE, IupGetAttribute(hEscRadio, IUP_VALUE));
  IupSetAttribute(hEscalaoTxt, IUP_VALUE, Esc_Desc);

  return IUP_DEFAULT;
}

int EscalaoOkBtCB_(Ihandle* self)
{
  IupSetAttribute(IupGetDialog(self), "Canceled", IUP_NO);  
  return IUP_CLOSE;
}

int EscalaoCancelBtCB_(Ihandle* self)
{
  IupSetAttribute(IupGetDialog(self), "Canceled", IUP_YES);
  return IUP_CLOSE;
}

int EscalaoBtCB_(Ihandle* self)
{
  int prev_value;

  if(hEscDialog == NULL)
    return IUP_DEFAULT;

  prev_value = IupGetInt(hItemTxt, IUP_VALUE);  /* <====================== Nao funciona */

  IupPopup(hEscDialog, IUP_CENTER, IUP_CENTER);
  if(IupGetInt(hEscDialog, "Canceled") != 0)
  { /* Botao de Cancel foi clicado, portanto a alteracao de escalao sera cancelada */
    char attr[20];

    sprintf(attr, "%d", prev_value);
    IupStoreAttribute(hEscRadio, IUP_VALUE, attr); /* <====================== Nao funciona */
  }else
  {
    char* attr = IupGetAttribute(hEscRadio, IUP_VALUE); /* <====================== Nao funciona */
    fprintf(stdout, "Item Selecionado %s\n", attr);
    IupSetAttribute(hItemTxt, IUP_VALUE, attr);
    IupSetAttribute(hEscalaoTxt, IUP_VALUE, Esc_Desc);
  }


  return IUP_DEFAULT;
}

int EscalaoTgCB_(Ihandle* self, int v)
{
  printf("Toggle %s alterado: %d\n", IupGetAttribute(self, IUP_TITLE), v);
  if(v == 0)
    return IUP_DEFAULT; 

  if(Esc_Desc == NULL)
    free(Esc_Desc);
  Esc_Desc = strdup(IupGetAttribute(self, IUP_TITLE));  
  
  return IUP_DEFAULT;
}

/*
** ---------------------------------------------------------------
** Functions:
*/

Ihandle *tg1, *tg2, *tg3, *tg4;
Ihandle* hEscaloes;

Ihandle* CreateEscaloesForm()
{
  Ihandle* hEscSet = IupVbox(
                       tg1 = IupToggle("AAAA", "Escalao_TgCB"),
                       tg2 = IupToggle("BBBB", "Escalao_TgCB"),
                       tg3 = IupToggle("CCCC", "Escalao_TgCB"),
                       tg4 = IupToggle("DDDD", "Escalao_TgCB"),
                       NULL
                       );

  IupSetHandle("1", tg1);
  IupSetHandle("2", tg2);
  IupSetHandle("3", tg3);
  IupSetHandle("4", tg4);

  hEscSet = IupRadio(hEscSet);

  IupSetFunction("Escalao_TgCB", (Icallback)EscalaoTgCB_);

  /* selecionar inicialmente o item divisao */
  //IupSetAttribute(hEscSet, IUP_VALUE, "3");
  IupSetAttribute(tg2, IUP_VALUE, IUP_ON);
  //IupSetAttribute(tg4, IUP_VALUE, IUP_ON);
  Esc_Desc = strdup("Divisao");

  hEscRadio = hEscSet;

  return hEscSet;
}

int button_action(Ihandle *self)
{
  printf("radio: %s\n", IupGetAttribute(hEscaloes, IUP_VALUE));
  printf("tg1: %s\n", IupGetAttribute(tg1, IUP_VALUE));
  printf("tg2: %s\n", IupGetAttribute(tg2, IUP_VALUE));
  printf("tg3: %s\n", IupGetAttribute(tg3, IUP_VALUE));
  printf("tg4: %s\n", IupGetAttribute(tg4, IUP_VALUE));
  return IUP_DEFAULT;
}

int text_action(Ihandle *self, int c)
{
  if(c == 13)
    IupSetAttribute(hEscaloes, "VALUE", IupGetAttribute(self, IUP_VALUE));

  return IUP_DEFAULT;
}

Ihandle* CreateEscaloesDialog()
{
  Ihandle* dlg ;
  hEscaloes = CreateEscaloesForm();

  IupSetFunction("text_action", (Icallback) text_action);
  IupSetFunction("button_action", (Icallback) button_action);

  dlg = IupDialog(IupVbox(  
                        IupSetAttributes(IupFrame(hEscaloes), "EXPAND=YES"),
                        IupHbox(
                           IupSetAttributes(IupButton("OK", "FiltroEsc_OkBtCB"), "SIZE=50x15"), 
                           IupFill(), 
                           IupSetAttributes(IupButton("Cancel", "FiltroEsc_CancelBtCB"), "SIZE=50x15"),
                           NULL
                        ),
                        IupVbox(
                          IupText("text_action"),
                          IupButton("Check values", "button_action"),
                          NULL
                        ),
                       IupRadio(IupVbox(
                       IupToggle("111", "Escalao_TgCB"),
                       IupToggle("222", "Escalao_TgCB"),
                       IupSetAttributes(IupToggle("333", "Escalao_TgCB"), "VALUE=ON"),
                       IupToggle("444", "Escalao_TgCB"),
                       NULL
                       )),
                        NULL
                     ));

  IupSetFunction("FiltroEsc_OkBtCB", (Icallback)EscalaoOkBtCB_);
  IupSetFunction("FiltroEsc_CancelBtCB", (Icallback)EscalaoCancelBtCB_);

  IupSetAttribute(dlg, IUP_TITLE, "Filtro de Escalao:");
  IupSetAttribute(dlg, "Canceled", IUP_NO);
  IupSetAttribute(dlg, IUP_MAXBOX, IUP_NO);
  IupSetAttribute(dlg, IUP_RESIZE, IUP_NO);
  IupSetAttribute(dlg, IUP_MINBOX, IUP_NO);
  IupSetAttribute(dlg, IUP_MARGIN, "2x2");
  IupSetAttribute(dlg, IUP_MENUBOX, IUP_NO);

  if(hMainDialog != NULL)
    IupSetAttribute(dlg, IUP_PARENTDIALOG, (char*)hMainDialog);

  hEscDialog = dlg;

  return dlg;
}

Ihandle* CreateInfoBar()
{
  Ihandle* hInfo;

  hInfo = IupHbox(
    IupVbox(
      IupLabel("Escalao:"),  
      hEscalaoTxt = IupText(NULL), 
      NULL
    ),
    IupVbox(IupLabel("Item:"), hItemTxt = IupText(NULL),NULL),
    NULL
  );

  IupSetAttribute(hEscalaoTxt, IUP_READONLY, IUP_YES);
  IupSetAttribute(hItemTxt, IUP_READONLY, IUP_YES);
  IupSetAttribute(hEscalaoTxt, IUP_SIZE, "80x");
  IupSetAttribute(hItemTxt, IUP_SIZE, "40x");

  return hInfo;
}

Ihandle* CreateMainDialog()
{
  Ihandle* dlg = NULL;

  dlg = IupDialog(IupSetAttributes(IupVbox(
                    IupSetAttributes(IupButton("Escalao", "EscalaoBtCB"), "SIZE=x20, EXPAND=HORIZONTAL"),
                    CreateInfoBar(),
                    NULL
                  ), "ALIGNMENT = ACENTER, GAP=5")
                 );

  IupSetAttribute(dlg, IUP_TITLE, "Teste do IupRadio");
  IupSetAttribute(dlg, IUP_RESIZE, IUP_NO);
  IupSetAttribute(dlg, IUP_MAXBOX, IUP_NO);
  IupSetAttribute(dlg, IUP_MINBOX, IUP_NO);
  IupSetAttribute(dlg, IUP_MAP_CB, "MainDlgMapCB");

  IupSetFunction("EscalaoBtCB", (Icallback)EscalaoBtCB_);
  IupSetFunction("MainDlgMapCB", (Icallback)MainDlgMapCB_);

  hMainDialog = dlg;

  return dlg;
}

/*
** ---------------------------------------------------------------
** Main function:
*/

void  main(int argc, char **argv)
{
  IupOpen(&argc, &argv);

  CreateMainDialog();
  CreateEscaloesDialog();

  IupShow(hMainDialog);

  IupMainLoop();

  IupClose();
}
