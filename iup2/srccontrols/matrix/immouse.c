/** \file
 * \brief iupmatrix control
 * mouse events
 *
 * See Copyright Notice in iup.h
 *  */

/**************************************************************************
***************************************************************************
*
*   Funcoes usadas para tratar eventos de mouse
*
***************************************************************************
***************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#include <iup.h>
#include <iupkey.h>
#include <iupcpi.h>
#include <cd.h>

#include "iupmatrix.h"
#include "matridef.h"
#include "matrixcd.h"
#include "imdraw.h"
#include "imscroll.h"
#include "imcolres.h"
#include "imaux.h"
#include "imfocus.h"
#include "immouse.h"
#include "imkey.h"
#include "immark.h"
#include "imgetset.h"
#include "imedit.h"

/* O botao da esquerda esta pressionado? */
static int  LeftPressed  = 0;

/* As variaveis abaixo guardam a linha e a coluna da posicao
   em que o botao da direita foi pressionado
*/
static int  RightClickLin;
static int  RightClickCol;

/**************************************************************************
***************************************************************************
*
*   Funcoes internas
*
***************************************************************************
***************************************************************************/

/*

%F Chama a callback do usuario ao evento de movimentacao do mouse sobre a 
   matriz
%i h : handle da matriz
   lin,col : coordenadas da celula.

*/
static void CallMoveCb (Ihandle *h, int lin, int col)
{
   Iitemmousemove cb;

   if(lin < -3 || col < -3)
     return;

   cb=(Iitemmousemove)IupGetCallback(h,IUP_MOUSEMOVE_CB);

   if (cb)
     cb(h,lin+1,col+1);
}


/*

%F Chama a callback do usuario associada ao evento de clique do botao do
   mouse sobre uma celula.
%i h : handle da matriz,
   lin,col : coordenadas da celula.

*/
static int CallClickCb(Ihandle *h, int press, int x, int y, char *r)
{
  IFniis cb;

  if (press)
    cb=(IFniis)IupGetCallback(h,IUP_CLICK_CB);
  else
    cb=(IFniis)IupGetCallback(h,"RELEASE_CB");

  if (cb)
  { 
    int lin, col;
    iupmatGetLineCol(h,x,y,&lin,&col);
    return cb(h,lin+1,col+1,r);  /* internamente : lin = -1 -> linha de titulo */
  }                              /*                col = -1 -> coluna de titulo */
                       
  return IUP_DEFAULT;
}


/*

%F Acao gerada quando o mouse e movido em cima da matriz.
   Vereifica se estou dando um resize numa coluna.
   Muda o formato do cursor caso esteja passando por uma juncao entre dois
   titulos de coluna (Se RESIZEMATRIX for YES ).
   Faz a marcacao de celulas se for o caso.
%i h : handle da matriz,
   x,y : coordenadas do mouse (coordenadas do canvas).

*/
static void MouseMove(Ihandle *h, int x, int y)
{
  int mult = !iupCheck(h,IUP_MULTIPLE)? 0 : 1;

  if (LeftPressed && mult)
  {
    /* the right canvas is activated inside iupmatMarkDrag */
    iupmatMarkDrag(h,x,y);
  }
  else if (iupmatColresResizing())/* Dando um resize no tamanho de uma coluna */
  {
    /* the right canvas is activated inside iupmatColresMove */
    iupmatColresMove(h,x);
  }
  else /* Muda cursor quando passando em cima da divisao de titulos de coluna */
  {
    iupmatColresChangeCursor(h,x,y);
  }
}

/*
%F Funcao chamada por LeftPress para tratar marcacaoes com shift pressionado
   Desmarca bloco anterior, e marca um novo, da posicao clicada ate a
   posicao do foco. Se houver mais de um bloco so desmarca aquele que
   estava sendo marcado por ultimo.
   Nao muda o foco de posicao....
%i h : handle da matriz,
   lin,col : coordenadas da celula.
*/
static void LeftClickShift(Ihandle *h, int lin, int col)
{
  Tmat *mat=(Tmat*)matrix_data(h);
    
  iupmatMarkDesmarcaBloco(h);
  iupmatMarkBloco(h,lin,col);

  /* Redesenha toda a area visivel */
  iupmatDrawMatrix(h,DRAW_ALL);

  /* O redraw apagou a borda do foco */
  iupmatDrawFocus(h,mat_lin(mat),mat_col(mat),1);
}

/*
%F Funcao chamada para tratar o pressionamento do botao. Chamada por LeftPress

%i h : handle da matriz,
   lin,col : coordenadas da celula
   ctrl : status da tecla control. 1 indica tecla pressionada e 0 tecla solta.
   duplo: Indica se foi ou nao um double click.
   mark_mode : modo de marcacao.
*/
static void LeftClick(Ihandle *h, int lin, int col, int ctrl, int duplo, int mark_mode)
{
  Tmat *mat=(Tmat*)matrix_data(h);

  if (duplo)
  {
    LeftPressed = 0;
    iupmatMarkReset();

    /* if a double click NOT in the current cell */
    if (lin != mat_lin(mat) || col != mat_col(mat))
    {
      /* leave the previous cell if the matrix previously had the focus */
      if (mat_hasiupfocus(mat) && iupmatCallLeavecellCb(h) == IUP_IGNORE)
        return;

      iupmatSetFocusPos(h,lin,col);
      iupmatCallEntercellCb(h);
    }
    
    if (iupmatEditShow(h))
    {
      if (mat_edtdatah(mat) == mat_edtdroph(mat)) 
        IupSetAttribute(mat_edtdatah(mat),"SHOWDROPDOWN",IUP_YES);

#ifdef _MOTIF_
      if (atoi(IupGetGlobal("MOTIFNUMBER")) < 2203) /* since OpenMotif version 2.2.3 this is not necessary */
        IupSetAttribute(h, "_IUPMAT_DOUBLE_CLICK", "1");
#endif
    }
  }
  else /* single click */
  {
    int mark_show;
    int oldlin = mat_lin(mat);
    int oldcol = mat_col(mat);

    /* leave the previous cell if the matrix previously had the focus */
    if (mat_hasiupfocus(mat) && iupmatCallLeavecellCb(h) == IUP_IGNORE)
      return;

    iupmatHideFocus(h);

    /* Esconde marcacao de celulas se apropriado... */
    mark_show = iupmatMarkHide(h,ctrl);

    iupmatSetFocusPos(h,lin,col);

    /* Mostra marcacao de celulas se apropriado... */
    if ((mark_mode != MARK_NO) && mark_show)
      iupmatMarkShow(h,ctrl,lin,col,oldlin,oldcol);

    iupmatCallEntercellCb(h);
    iupmatShowFocus(h);
  }
}

static void LeftPress(Ihandle *h, int x, int y, int shift, int ctrl, int duplo)
{
  int lin,col;
  int mark_mode;

  iupmatResetKeyCount();

  /* Se x,y nao correspondem a uma celula da matriz, retorna.
     Se for a celula -1,-1 (canto superior esquerdo) retorna.
  */
  if (!iupmatGetLineCol(h,x,y,&lin,&col) || lin == -2 || col == -2 ||
       (lin == -1 && col == -1))
    return;

  /* Faz um conjunto de criticas pertinentes quando estamos
     com a marcacao ligada. Estas criticas operam mudando os
     valores das variaveis de controle shift, ctrl,...
  */
  mark_mode = iupmatMarkCritica(h,&lin,&col,&shift,&ctrl,&duplo);

  LeftPressed = 1;

  if(shift)
    LeftClickShift(h,lin,col);
  else
    LeftClick(h,lin,col,ctrl,duplo,mark_mode);
}


/*

%F Funcao chamada quando o botao da esquerda e solto.
%i h : handle da matriz,
   x,y : coordenadas do mouse (relativas ao canvas).

*/
static void LeftRelease(Ihandle *h, int x)
{
  LeftPressed = 0;   /* O botao da esquerda esta solto */

  iupmatMarkReset();

  if (iupmatColresResizing())/*Se estava fazendo um resize de colunas,termina*/
    iupmatColresFinish(h,x);
}

/*
%F Funcao chamada quando o botao do mouse e pressionado. Guarda a celula em
   que foi pressionado para comparacao quando o botao for solto.
%i h : handle da matriz,
   x,y : coordenadas do mouse (relativas ao canvas)

*/
static void RightPress(Ihandle *h, int x, int y)
{
  int lin,col;

  if (iupmatGetLineCol(h,x,y,&lin,&col) && (lin!= -2) && (col != -2))
  {
    RightClickLin = lin;
    RightClickCol = col;
  }
  else
  {
    RightClickLin = -9; /* Valor que nunca sera retornado por iupmatGetLineCol*/
    RightClickCol = -9;
  }
}

/*

%F Funcao chamada quando o botao direito do mouse e solto. Chama uma callback
   do usuario se o botao foi solto na mesma celula em que foi pressionado.
%i h : handle da matriz,
   x,y : coordenadas do mouse (relativas ao canvas)

*/
static void RightRelease(Ihandle *h, int x, int y)
{
  int lin,col;

  if (iupmatGetLineCol(h,x,y,&lin,&col) &&
      (lin == RightClickLin) && (col == RightClickCol));
}


/**************************************************************************
***************************************************************************
*
*   Funcoes exportadas
*
***************************************************************************
***************************************************************************/

/*

%F Callback chamada quando um botao do mouse e pressionado ou solto
%i hm : handle da matriz,
   b : identificador do botao do mouse [IUP_BUTTON1,IUP_BUTTON2 ou IUP_BUTTON3],
   press : 1 se for evento de pressionar o botao 0, se for de soltar,
   x,y : posicao do mouse,
   r : string contendo quais teclas [SHIFT,CTRL e ALT] estao pressionadas
%o Retorna IUP_DEFAULT.

*/
int iupmatMouseButtonCb (Ihandle *hm, int b, int press, int x, int y, char *r)
{
  int ret = IUP_DEFAULT;
  Tmat *mat=(Tmat*)matrix_data(hm);

  if (press)
  {
    /* The edit Kill Focus is not called when the user clicks in the parent canvas. 
       so we have to compensate that. */
    iupmatEditCheckHidden(hm);

    mat_hasiupfocus(matrix_data(hm)) = 1;
  }

  if (b==IUP_BUTTON1)
  {
    if (press)
    {
      int duplo = isdouble(r);

      if (iupmatColresTry(hm,x,y))
        return ret;            /* Resize da largura de uma coluna iniciado */

      LeftPress(hm,x,y,isshift(r),iscontrol(r),duplo);
    }
    else
      LeftRelease(hm,x);
  }
  else if (b==IUP_BUTTON3)
  {
    if (press)
      RightPress(hm,x,y);
    else
      RightRelease(hm,x,y);
  }

  ret = CallClickCb(hm,press,x,y,r);
  if (ret == IUP_IGNORE)
    return ret;

  if (mat->redraw)
  {
    cdCanvasFlush(cdcv(mat)); 
    mat->redraw = 0; 
  }
  return ret;
}

/*
%F callback chamada quando o mouse e movido.
%i hm : handle da matriz,
   x,y : posicao do mouse.
%o Retorna IUP_DEFAULT.
*/
int iupmatMouseMoveCb(Ihandle *hm, int x, int y)
{
  int lin, col;

  MouseMove(hm,x,y);

  iupmatGetLineCol(hm, x, y, &lin, &col);
  CallMoveCb(hm, lin, col);

  return IUP_DEFAULT;
}
