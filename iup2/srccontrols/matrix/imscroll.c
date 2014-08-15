/** \file
 * \brief iupmatrix control
 * scrolling
 *
 * See Copyright Notice in iup.h
 *  */

#include <string.h>
#include <stdio.h>

#include <iup.h>
#include <iupcpi.h>
#include <cd.h>

#include "iupmatrix.h"
#include "matridef.h"
#include "matrixcd.h"
#include "imdraw.h"
#include "imscroll.h"
#include "imfocus.h"
#include "imaux.h"
#include "imkey.h"
#include "immark.h"
#include "imgetset.h"
#include "imedit.h"

/* Tipo para callback de scroll */
typedef int (*Scrollcb)(Ihandle*, int, int);

static void cb_scroll  (Ihandle *h);
static void OpenLineColumn    (Ihandle *h, int m);
static void GetFirstLineColumn(Ihandle *h, int pos, int m);

/**************************************************************************
***************************************************************************
*
*   Funcoes INTERNAS...
*
***************************************************************************
***************************************************************************/

/*
%F Retira colunas/linhas a esquerda/acima da matriz ate que a ultima
   coluna/linha possa ficar totalmente visivel
%i h : handle da matriz
   m : Escolhe se vai operar sobre linhas ou colunas. [MAT_LIN|MAT_COL]
*/
static void OpenLineColumn(Ihandle *h, int m)
{
  Tmat *mat=(Tmat*)matrix_data(h);
  int i=0;
  int old;
  Tlincol *p;

  if (m == MAT_LIN)
    p = &(mat->lin);
  else
    p = &(mat->col);

  old = p->last;  /* Guarda qual a ultima linha/coluna visivel*/

  /*Incrementa a primeira col/lin ate que a ultima esteja completamente visivel*/
  do
  {
    i++;
    p->first++;
    iupmatGetLastWidth(h,m);
  }while(p->last == old &&
         p->last != (p->num - 1) && /* Nao e' a ultima linha/coluna */
         p->lastwh != p->wh[p->last]);

  iupmatGetPos(h,m);
  if(m == MAT_COL)
    iupmatScroll(h,SCROLL_RIGHT,old,i);
  else
    iupmatScroll(h,SCROLL_DOWN,old,i);
}


/*

%F Calcula qual deve ser a primeira coluna/linha visivel da matriz tal que
   a soma das larguras das colunas/linhas invisiveis a esquerda/acima seja
   o mais proximo possivel do valor dado.
%i h : handle da matriz,
   pos : tamanho da parte a ser deixada a esquerda da coluna/linha.
   m : Escolhe se vai operar sobre linhas ou colunas. [MAT_LIN|MAT_COL]

*/
static void GetFirstLineColumn(Ihandle *h, int pos, int m)
{
  Tmat *mat=(Tmat*)matrix_data(h);
  int i;
  Tlincol *p;

  if (m == MAT_LIN)
    p = &(mat->lin);
  else
    p = &(mat->col);


  p->pos = 0;
  for (i=0;i<p->num;i++)
  {
    p->pos+=p->wh[i];
    if (p->pos > pos)
     break;
  }
  if ((p->pos-pos) > (pos - p->pos+p->wh[i]) || (p->num == i) )
  {
    p->pos -=p->wh[i];
    i--;
  }
  p->first = i+1;
}


/*
%F callback para notificar o usuario de que a area de vizualizacao da
   matriz mudou.

%i h - Handle da matriz
*/
static void cb_scroll(Ihandle *h)
{
  Tmat *mat=(Tmat*)matrix_data(h);
  Scrollcb cb = (Scrollcb)IupGetCallback(h,"SCROLLTOP_CB");

  if(cb) 
  {
    cb(h,mat_fl(mat)+1,mat_fc(mat)+1);
  }
}


/**************************************************************************
***************************************************************************
*
*   Funcoes EXPORTADAS, usadas para fazer scroll de tela usando get
*  e put image
*
***************************************************************************
***************************************************************************/
/*

%F Faz o scroll de um certo numero de colunas para a esquerda ou direita ou
   linhas para cima e para baixo, usando a primitiva de ScrollImage.
   Se for para a esquerda, move apenas uma coluna, para a direita pode mover
   mais de uma. Da mesma forma, para cima so move uma linha e para baixo
   pode mover mais de uma.
%i h : handle da matriz,
   dir : [SCROLL_LEFT| SCROLL_RIGHT | SCROLL_UP | SCROLL_DOWN].
   ref : Primeira coluna/linha que deve ser redesenhada no caso de movimentacao
         para a direita ou para baixo.
   num : Numero de colunas/linhas que devem ser movidas, no caso de movimentacao
         para a direita ou para baixo.
*/
void iupmatScroll(Ihandle *h, int dir, int ref, int num)
{
  Tmat *mat=(Tmat*)matrix_data(h);
  int x1,x2,y1,y2,xt2,yt2;
  int i;

  x1 = 0;  x2 = XmaxCanvas(mat);
  y1 = 0;  y2 = YmaxCanvas(mat);

  mat->redraw = 1;

  if (dir == SCROLL_LEFT)
  {
    x1 = mat_wt(mat);      /* pula o titulo da linha */
    x2-= mat_w(mat)[mat_fc(mat)];
    if (x1 >= x2)
    {
     iupmatDrawMatrix(h,DRAW_COL);
     return;
    }

    cdCanvasScrollArea(mat->cddbuffer,x1,x2,y1,y2,mat_w(mat)[mat_fc(mat)],0);

    iupmatDrawColumnTitle(h,mat_fc(mat),mat_fc(mat));

    iupmatSetCdFrameColor(h);
    cdCanvasLine(mat->cddbuffer,XmaxCanvas(mat),y1,XmaxCanvas(mat),y2);

    iupmatDrawCells(h, mat_fl(mat), mat_fc(mat), mat_ll(mat), mat_fc(mat));
  }
  else if (dir == SCROLL_UP)
  {
    y2 -= mat_ht(mat);      /* pula o titulo das colunas */
    y1 += mat_h(mat)[mat_fl(mat)];
    if (y1 >= y2)
    {
     iupmatDrawMatrix(h,DRAW_LIN);
     return;
    }

    cdCanvasScrollArea(mat->cddbuffer,x1,x2,y1,y2,0,-mat_h(mat)[mat_fl(mat)]);
    iupmatDrawLineTitle(h,mat_fl(mat),mat_fl(mat));
    iupmatDrawCells(h, mat_fl(mat), mat_fc(mat), mat_fl(mat), mat_lc(mat));
  }
  else if (dir == SCROLL_RIGHT)
  {
    x1 = mat_wt(mat);      /* pula o titulo da linha */
    for (i=1;i<=num;i++)
      x1+= mat_w(mat)[mat_fc(mat)-i];
    if (x1 >= x2)
    {
     iupmatDrawMatrix(h,DRAW_COL);
     return;
    }

    cdCanvasScrollArea(mat->cddbuffer,x1,x2/*-1*/,y1,y2,mat_wt(mat)-x1,0);

    xt2 = iupmatDrawColumnTitle(h,ref,mat_lc(mat));

    if (xt2 < x2)               /* limpa area da direita que nao tem coluna */
    {
      iupmatDrawEmptyArea(h,xt2,x2,y1,y2);
    }
    else
    {
      iupmatSetCdFrameColor(h);
      cdCanvasLine(cdcv(mat),x2,y1,x2,y2);
    }

    /* desenha as celulas */
    iupmatDrawCells(h, mat_fl(mat), ref, mat_ll(mat), mat_lc(mat));
  }
  else if (dir == SCROLL_DOWN)
  {
    y2 -= mat_ht(mat);      /* pula o titulo das colunas */
    for (i=1;i<=num;i++)
      y2-= mat_h(mat)[mat_fl(mat)-i];
    if (y1 >= y2)
    {
     iupmatDrawMatrix(h,DRAW_LIN);
     return;
    }

    cdCanvasScrollArea(mat->cddbuffer,x1,x2/*-1*/,y1,y2,0,YmaxCanvas(mat)-mat_ht(mat)-y2);

    yt2 = iupmatDrawLineTitle(h,ref,mat_ll(mat));

    if (yt2 < y1)               /* limpa area da direita que nao tem coluna */
    {
     iupmatDrawEmptyArea(h,x1,x2,y1,yt2);
    }
    else
    {
     iupmatSetCdFrameColor(h);
     cdCanvasLine(cdcv(mat),x1,y1,x2,y1);
    }

    /* desenha as celulas */
    iupmatDrawCells(h, ref, mat_fc(mat), mat_ll(mat), mat_lc(mat));
  }
}


/**************************************************************************
***************************************************************************
*
*   Funcoes EXPORTADAS usadas para movimentacao dentro da matriz
*
***************************************************************************
***************************************************************************/

/*
%F Funcao que faz a movimentacao pelas celulas da matriz.
   Recebe como parametro um ponteiro para a funcao que vai executar
   realmente o trabalho.  Isto e' feito para evitar ter de, em cada uma das
   funcoes de manipulacao de cursor, colocar um teste para ver se preciso ou
   nao chamar a callback de scroll.  Ist so e' feito aqui.

%i func - Ponteiro para a funcao que vai fazer o trabalho de movimentacao
   h - Handle da matriz
   modo - Parametro passado para func, tem como objetivo especificar se
          o pedido de movimentacao veio da scrollbar ou do teclado
   pos  - Parametro passado para func, que no caso de func ser a funcao
          de tratamento de posicao de scrollbar, indica a posicao do thumb
          da scrollbar...  Se func for qualquer outra funcao, este parametro
          sera ignorado.
   m    - Parametro passado para func, para dizer se ela esta operando sobre
          linhas ou colunas. [MAT_LIN | MAT_COL]
%o Retorna 1 se mudou a celula de lugar, 0 se nao fez nada.
*/
int iupmatScrollMoveCursor(iupmatScrollMoveF func, Ihandle *h,
                            int modo, float pos, int m)
{
  Tmat *mat = (Tmat*)matrix_data(h);
  int oldfl  = mat_fl(mat);
  int oldfc  = mat_fc(mat);
  int oldlin = mat_lin(mat);
  int oldcol = mat_col(mat);

  iupmatEditCheckHidden(h);

  func(h,modo,pos,m);

  if (mat_fl(mat) != oldfl || mat_fc(mat) != oldfc)
    cb_scroll(h);

  if(mat_lin(mat) != oldlin || mat_col(mat) != oldcol)
    return 1;

  return 0;
}

/*

%F Abre uma celula nao mostrada completamente, da periferia da matriz.
   Chama a callback de Scroll se necessario.
%i h : handle da matriz
   lin,col : Coordenadas da celula que vai conter o foco.
*/
void iupmatScrollOpen(Ihandle *h, int lin, int col)
{
  Tmat *mat = (Tmat*)matrix_data(h);
  int oldfl = mat_fl(mat);
  int oldfc = mat_fc(mat);

  if (col == mat_lc(mat) && mat_wlc(mat) != mat_w(mat)[mat_lc(mat)])
  {
    OpenLineColumn(h,MAT_COL);
    SetSbH;
  }
  if (lin == mat_ll(mat) && mat_hll(mat) != mat_h(mat)[mat_ll(mat)])
  {
    OpenLineColumn(h,MAT_LIN);
    SetSbV;
  }

  if (mat_fl(mat) != oldfl || mat_fc(mat) != oldfc)
    cb_scroll(h);
}

/*

%F Funcao chamada quando a tecla home e pressionada. Se for a primeira vez,
   vai para o inicio da linha, segunda para o inicio da pagina e terceira
   vai para o inicio da matriz.
%i h : handle da matriz.
   mode e pos : NAO USADOS
*/
void iupmatScrollHome(Ihandle *h, int mode, float pos, int m)
{
  Tmat *mat = (Tmat*)matrix_data(h);

  if (iupmatGetHomeKeyCount() == 0)  /* vai p/ comeco da linha */
  {
    if (mat_col(mat) != 0)
    {
      iupmatHideFocus(h);
      if (mat_fc(mat) != 0)
      {
        mat_fc(mat) = 0;
        while(mat_fc(mat) < mat_nc(mat) && mat_w(mat)[mat_fc(mat)]==0)
          mat_fc(mat)++;

        iupmatGetLastWidth(h,MAT_COL);
        iupmatGetPos(h,MAT_COL);
        SetSbH;
        iupmatDrawMatrix(h,DRAW_COL);
      }
      iupmatSetShowFocus(h,mat_lin(mat),mat_fc(mat));
    }
  }
  else if (iupmatGetHomeKeyCount() == 1)   /* vai p/ comeco da pagina */
  {
    if (mat_lin(mat) != mat_fl(mat))
    {
      iupmatHideSetShowFocus(h,mat_fl(mat),mat_fc(mat));
    }
  }
  else if (iupmatGetHomeKeyCount() == 2)   /* vai p/ o comeco da matriz 1:1 */
  {
    if (mat_lin(mat) != 0)
    {
      iupmatHideFocus(h);
      mat_fl(mat)=0;
      while(mat_fl(mat) < mat_nl(mat) && mat_h(mat)[mat_fl(mat)]==0)
        mat_fl(mat)++;
      iupmatGetLastWidth(h,MAT_LIN);
      iupmatGetPos(h,MAT_LIN);
      SetSbV;
      iupmatDrawMatrix(h,DRAW_LIN);
      iupmatSetShowFocus(h,0,mat_fc(mat));
    }
  }
}

/*

%F Funaco chamada quando a tecla end e pressionada. Se for a primeira vez,
   vai para o final da linha, segunda para o final da pagina e terceira
   vai para o final da matriz.
%i h : handle da matriz.
   mode e pos : NAO USADOS
*/
void iupmatScrollEnd(Ihandle *h, int mode, float pos, int m)
{
  Tmat *mat = (Tmat*)matrix_data(h);


  if (iupmatGetEndKeyCount() == 0)  /* vai p/ final da linha */
  {
    if (mat_col(mat) != mat_ultimac(mat))
    {
      iupmatHideFocus(h);
      if (mat_lc(mat) != mat_ultimac(mat) ||
          mat_w(mat)[mat_lc(mat)]!=mat_wlc(mat))
      {
        int i,soma;

        mat_lc(mat) = mat_ultimac(mat);
        while(mat_lc(mat) > 0 && mat_w(mat)[mat_lc(mat)] == 0)
          mat_lc(mat)--;

        mat_wlc(mat)=mat_w(mat)[mat_lc(mat)];

        for(soma=0,i=mat_ultimac(mat);i>=0;i--)
        {
          soma+=mat_w(mat)[i];
          if (soma > mat_sx(mat))
            break;
        }
        mat_fc(mat) = i+1;
        if (i >= 0)
          soma-=mat_w(mat)[i];

        mat_posx(mat) = mat_totw(mat)-soma;
        SetSbH;
        iupmatDrawMatrix(h,DRAW_COL);
      }
      iupmatSetShowFocus(h,mat_lin(mat),mat_lc(mat));
    }
  }
  else if (iupmatGetEndKeyCount() == 1)   /* vai p/ final da pagina */
  {
    if (mat_lin(mat) != mat_ll(mat) ||
        mat_h(mat)[mat_ll(mat)] != mat_hll(mat))
    {
      int old = mat_ll(mat);
      iupmatHideFocus(h);
      if (mat_h(mat)[mat_ll(mat)] != mat_hll(mat))
        OpenLineColumn(h,MAT_LIN);
      iupmatSetShowFocus(h,old,mat_col(mat));
      SetSbV;
    }
  }
  else if (iupmatGetEndKeyCount() == 2)   /* vai p/ o final da matriz  */
  {
    if (mat_lin(mat) != mat_ultimal(mat))
    {
      int i,soma;
      iupmatHideFocus(h);

      mat_ll(mat) = mat_ultimal(mat);
      while(mat_ll(mat) > 0 && mat_h(mat)[mat_ll(mat)] == 0)
        mat_ll(mat)--;

      mat_hll(mat)=mat_h(mat)[mat_ll(mat)];

      for(soma=0,i=mat_ultimal(mat);i>=0;i--)
      {
        soma+=mat_h(mat)[i];
        if (soma > mat_sy(mat))
          break;
      }
      mat_fl(mat) = i+1;
      if (i >= 0)
        soma-=mat_h(mat)[i];

      mat_posy(mat) = mat_toth(mat)-soma;

      SetSbV;
      iupmatDrawMatrix(h,DRAW_LIN);
      iupmatSetShowFocus(h,mat_nl(mat)-1,mat_col(mat));
    }
  }
}

/*

%F Funcao chamda se queremos andar uma celula para a esquerda ou para cima.
%i h    : handle da matriz,
   modo : Indica se o comando veio do teclado ou da scrollbar. Se veio da
          sb, nao muda o foco de lugar.
   pos  : NAO USADO
   m    : Escolhe se vai operar sobre linhas (Up) ou colunas (Left).
          [MAT_LIN|MAT_COL]
*/
void iupmatScrollLeftUp(Ihandle *h,int modo, float pos, int m)
{
  Tmat *mat=(Tmat*)matrix_data(h);
  int nextfirst,nextactive;
  Tlincol *p;

  if (m == MAT_LIN)
    p = &(mat->lin);
  else
    p = &(mat->col);

  nextfirst = p->first - 1;
  while(nextfirst >=0 && p->wh[nextfirst]==0) nextfirst--;

  nextactive = p->active - 1;
  while(nextactive >=0 && p->wh[nextactive]==0) nextactive--;

  /* se o comando veio da scrollbar ou se o foco esta na primeira coluna/linha */
  if (modo == SCROLL || nextactive <  p->first)
  {
    int old = p->first;

    if(nextfirst < 0)
      return;

    iupmatHideFocus(h);

    p->first = nextfirst;
    iupmatGetLastWidth(h,m);
    iupmatGetPos(h,m);

    if (m == MAT_COL)
      iupmatScroll(h,SCROLL_LEFT,old,old-nextfirst);
    else
      iupmatScroll(h,SCROLL_UP,old,old-nextfirst);

    SetSb(m);

    if (m == MAT_COL)
      iupmatSetShowFocus(h,mat_lin(mat),modo==SCROLL?mat_col(mat):nextfirst);
    else
      iupmatSetShowFocus(h,modo==SCROLL?mat_lin(mat):nextfirst,mat_col(mat));
  }
  else
  {
    if (nextactive < 0)
      return;
    if (m == MAT_COL)
      iupmatHideSetShowFocus(h,mat_lin(mat),nextactive);
    else
      iupmatHideSetShowFocus(h,nextactive,mat_col(mat));
  }

}

/*

%F Funcao chamda se queremos andar uma celula para a direita ou para baixo.
%i h : handle da matriz,
   modo : Indica se o comando veio do teclado ou da scrollbar. Se veio da
          sb, nao muda o foco de lugar.
   pos : NAO USADO
   m   : Escolhe se vai operar sobre linhas (Down) ou colunas (Right).
         [MAT_LIN|MAT_COL]

*/
void iupmatScrollRightDown(Ihandle *h,int modo, float pos, int m)
{
  Tmat *mat=(Tmat*)matrix_data(h);
  int old;
  int next;
  Tlincol *p;

  if (m == MAT_LIN)
    p = &(mat->lin);
  else
    p = &(mat->col);

  old = p->last;

  if (modo == KEY)
  {
    int abre = 0;
    /* Na ultima linha/coluna, mas nao completamente visivel.
       Abre mas mantem na mesma celula
    */
    if(p->active == p->last && p->lastwh != p->wh[p->last])
    {
      next = p->last;
      abre = 1;
    }
    else
    {
      next = p->active+1;
      while(next < p->num && p->wh[next]==0) next++;

      if(next >= p->num) return;

      if((next > p->last) || (next == p->last && p->lastwh != p->wh[p->last]))
        abre = 1;
    }

    /* Se preciso abrir alguma linha/coluna  */
    if (abre)
    {
      iupmatHideFocus(h);
      OpenLineColumn(h,m);
      SetSb(m);
      if (m == MAT_COL)
        iupmatSetShowFocus(h,mat_lin(mat),next);
      else
        iupmatSetShowFocus(h,next,mat_col(mat));
    }
    else
    {
      if (m == MAT_COL)
        iupmatHideSetShowFocus(h,mat_lin(mat),next);
      else
        iupmatHideSetShowFocus(h,next,mat_col(mat));
    }
  }
  else  /* SCROLL */
  {
    int oldf = p->first;

    /* Se a ultima coluna/linha da matriz ja esta completamente visivel, nao
      tenho mais nada para fazer por aqui.  Retorna.
    */
    if ((p->lastwh == p->wh[p->last])&&
        (p->last == (p->num - 1)))
      return;

    p->first++;
    while(p->first < p->num && p->wh[p->first]==0)
      p->first++;

    if (p->first >= p->num)
      return;

    iupmatGetLastWidth(h,m);
    iupmatGetPos(h,m);

    iupmatHideFocus(h);
    if(m == MAT_COL)
      iupmatScroll(h,SCROLL_RIGHT,old,p->first-oldf);
    else
      iupmatScroll(h,SCROLL_DOWN,old,p->first-oldf);

    SetSb(m);

    iupmatShowFocus(h);
  }
}

/*
%F Anda a matriz uma pagina para a esquerda ou para cima
%i h : handle da matriz.
   mode: Indica se o comando veio do teclado ou da scrollbar. Se veio da
         sb, nao muda o foco de lugar.
   pos : NAO USADO
   m   : Escolhe se vai operar sobre linhas (PgUp) ou colunas (PgLeft).
         [MAT_LIN|MAT_COL]
*/
void iupmatScrollPgLeftUp(Ihandle *h, int mode, float pos, int m)
{
  Tmat *mat=(Tmat*)matrix_data(h);
  int soma;
  Tlincol *p;
  int old;

  if (m == MAT_LIN)
    p = &(mat->lin);
  else
    p = &(mat->col);

  if (p->first <= 0)
  {
    /* Se estou na primeira pagina da matriz, e o pedido de scroll
       veio do teclado, entao muda o foco para o comeco da pagina.
    */
    if (mode == KEY)
      iupmatHideSetShowFocus(h,0,mat_col(mat));
    return;
  }

  old  = p->first;
  soma = p->size;

  /* Procura qual vai ser a primeira coluna/linha */
  while((soma > 0)&&(p->first!=0))
  {
   soma -= p->wh[p->first-1];
   if (soma >= 0)
     p->first--;
  }

  iupmatGetLastWidth(h,m);
  iupmatGetPos(h,m);

  iupmatHideFocus(h);
  SetSb(m);
  iupmatDrawMatrix(h,m);

  if(mode == KEY)
  {
    /* Se modo == KEY, m == MAT_LIN com certeza, nao existe PgLeft
       no teclado
    */
    int newl = mat_lin(mat)+(p->first - old);
    if (newl <= 0)
      newl = 0;
    iupmatSetShowFocus(h,newl,mat_col(mat));
  }
  else
    iupmatSetShowFocus(h,mat_lin(mat),mat_col(mat));
}

/*

%F Anda a matriz uma pagina para a direita ou para baixo
%i h   : handle da matriz.
   mode: Indica se o comando veio do teclado ou da scrollbar. Se veio da
         sb, nao muda o foco de lugar.
   pos : NAO USADO
   m   : Escolhe se vai operar sobre linhas (PgDown) ou colunas (PgRight).
         [MAT_LIN|MAT_COL]
*/
void iupmatScrollPgRightDown(Ihandle *h, int mode, float pos, int m)
{
  Tmat *mat=(Tmat*)matrix_data(h);
  Tlincol *p;
  int old;

  if (m == MAT_LIN)
    p = &(mat->lin);
  else
    p = &(mat->col);


  /* Testa se ha colunas  escondidas a direita/abaixo*/
  if ((p->last >= p->num) ||
      ((p->last==(p->num-1))&&(p->lastwh == p->wh[p->last]))
     )
  {
    /* Se estou na ultima pagina da matriz e ela esta completamente
       visivel, e o pedido de scroll veio do teclado, entao muda o
       foco para a ultima linha.
    */
    if (mode == KEY)
      iupmatHideSetShowFocus(h,p->num-1,mat_col(mat));
    return;
  }

  iupmatHideFocus(h);

  old = p->first;
  /* Proxima primeira coluna = ultima visivel antigamente. Se a ultima coluna
     estava sendo vista completamente (raro) entao soma um a mat_fc.
     A mesma coisa se estivermos trabalhando com linhas.
  */
  p->first = p->last;
  if (p->wh[p->last] == p->lastwh)
    p->first++;
  iupmatGetLastWidth(h,m);
  iupmatGetPos(h,m);
  SetSb(m);
  iupmatDrawMatrix(h,m);

  if(mode == KEY)
  {
    /* Se modo == KEY, m == MAT_LIN com certeza, nao existe PgRight
       no teclado
    */
    int newl = mat_lin(mat)+(p->first - old);
    if (newl > p->num-1)
      newl = p->num -1;
    iupmatSetShowFocus(h,newl,mat_col(mat));
  }
  else
    iupmatSetShowFocus(h,mat_lin(mat),mat_col(mat));
}

void iupmatScrollCr(Ihandle *h, int mode, float pos, int m)
{
  Tmat *mat=(Tmat*)matrix_data(h);
  int redraw = 0;
  int lin=0,col=0; /* random initialization */
  int width;
  int oldlin = mat_lin(mat);
  int oldcol = mat_col(mat);

  iupmatScrollRightDown(h, mode, pos, m);

  if (mat_lin(mat) == oldlin && mat_col(mat) == oldcol)
  {
    /* Nao mudou o foco de lugar, logo estava na ultima linha da coluna.
       Vai para a mesma linha da proxima coluna, se existir.
    */
    col = mat_col(mat)+1;
    while(col <= mat_ultimac(mat) && mat_w(mat)[col]==0) col++;
    if(col <= mat_ultimac(mat))
    {
      if(col > mat_lc(mat) || (col == mat_lc(mat) && mat_w(mat)[col] != mat_wlc(mat)))
      {
        width = mat_w(mat)[col];
        while(width > 0)
        {
          width -= mat_w(mat)[mat_fc(mat)];
          mat_fc(mat)++;
        }
        iupmatGetLastWidth(h,MAT_COL);
        iupmatGetPos(h,MAT_COL);
        redraw = 1;
      }

      lin = mat_lin(mat);
      while(lin <= mat_ultimal(mat) && mat_h(mat)[lin] == 0) lin++;
      if(mat_fl(mat) != lin)
      {
        mat_fl(mat) = lin;
        iupmatGetLastWidth(h,MAT_LIN);
        iupmatGetPos(h,MAT_LIN);
        redraw = 1;
      }

      iupmatHideFocus(h);
      iupmatSetFocusPos(h,lin,col);
      if (redraw)
      {
        SetSbV;
        SetSbH
        iupmatDrawMatrix(h,DRAW_ALL);
      }
    }
  }
}

/*

%F Funcao chamada quando fazemos um drag na scrollbar
%i h : handle da matriz,
   x : posicao do thumb na scrollbar, valor entre 0 e 1.
   mode : NAO USADO
   m : Escolhe se vai operar sobre linhas ou colunas. [MAT_LIN|MAT_COL]
*/
void iupmatScrollPos(Ihandle *h, int mode, float x, int m)
{
  Tmat *mat=(Tmat*)matrix_data(h);
  int   pos;
  float dx;
  Tlincol *p;

  if (m == MAT_LIN)
  {
    p = &(mat->lin);
    dx = IupGetFloat(h,IUP_DY);
  }
  else
  {
    p = &(mat->col);
    dx = IupGetFloat(h,IUP_DX);
  }

  pos = (int)(x * p->totalwh + .5);

  iupmatHideFocus(h);
  GetFirstLineColumn(h,pos,m);
  iupmatGetLastWidth(h,m);

  /* A conta feita por GetFirstLineColumn acha a coluna que melhor aproxima a
     posicao da scrollbar.  Mas quando a scrollbar encosta no final, esta
     conta pode nao dar muito certo e a ultima coluna nao ficar toda visivel...
     O trecho abaixo corrige isto.  Ele descobre que a scrollbar encostou no
     lado direito, somando sua posicao com seu tamanho e comparando com 1.0
     (usa uma tolerancia de 0.0005).
  */
  if ((x + dx >= 0.9995) && (p->last  != (p->num-1) ||
                             p->lastwh != p->wh[p->num-1]))
  {
    /*Incrementa a primeira col ate que a ultima esteja completamente visivel*/
    do
    {
      p->first++;
      iupmatGetLastWidth(h,m);
    }while(p->lastwh!= p->wh[p->num-1] ||
           p->last != (p->num-1));

    iupmatGetPos(h,m);
  }

  SetSb(m);
  iupmatDrawMatrix(h,m);
  iupmatSetShowFocus(h,mat_lin(mat),mat_col(mat));
}

/*

%F Callback chamada quando alguma acao e feita sobre a scrollbar.
%i hm : handle da matriz,
   action : tipo da acao que gerou o evento,
   x,y : posicao do thumb da scrollbar, valores de 0 a 1.
%o retorna IUP_DEFAULT

*/
int iupmatScrollCb(Ihandle *hm, int action, float x, float y)
{
  Tmat *mat=(Tmat*)matrix_data(hm);
  int err;

  x = IupGetFloat(hm, IUP_POSX);
  y = IupGetFloat(hm, IUP_POSY);

  IsCanvasSet(mat,err);
  if(err == CD_OK)
  {
    switch(action)
    {
      case IUP_SBUP      : ScrollUp(hm);       break;
      case IUP_SBDN      : ScrollDown(hm);     break;
      case IUP_SBPGUP    : ScrollPgUp(hm);     break;
      case IUP_SBPGDN    : ScrollPgDown(hm);   break;
      case IUP_SBRIGHT   : ScrollRight(hm);    break;
      case IUP_SBLEFT    : ScrollLeft(hm);     break;
      case IUP_SBPGRIGHT : ScrollPgRight(hm);  break;
      case IUP_SBPGLEFT  : ScrollPgLeft(hm);   break;
      case IUP_SBPOSV    : ScrollPosVer(hm,y); break;
      case IUP_SBPOSH    : ScrollPosHor(hm,x); break;
      case IUP_SBDRAGV   : ScrollPosVer(hm,y); break;
      case IUP_SBDRAGH   : ScrollPosHor(hm,x); break;
    }
  }

  { cdCanvasFlush(mat->cddbuffer); mat->redraw = 0; } /* always update */
  return IUP_DEFAULT;
}
