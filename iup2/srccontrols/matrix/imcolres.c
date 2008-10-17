/** \file
 * \brief iupmatrix column resize
 *
 * See Copyright Notice in iup.h
 * $Id: imcolres.c,v 1.1 2008-10-17 06:20:15 scuri Exp $
 */

/**************************************************************************
***************************************************************************
*
*   Funcoes que tratam da interacao para dar um resize numa coluna.
*   Funcoes para fazer setar ou consultar a largura de uma coluna.
*
***************************************************************************
***************************************************************************/

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <iup.h>
#include <iupcpi.h>
#include <cd.h>

#include "iupmatrix.h"
#include "matrixcd.h"
#include "matridef.h"
#include "imdraw.h"
#include "imaux.h"
#include "imfocus.h"
#include "imcolres.h"

#define TOL       3

static int  Dragging=0,      /* Flag que indica se estou ou nao no meio de
                                um resize
                             */
            DragCol,         /* Coluna que esta sendo alterada, valor e'
                                zero based, 0 se estiver dando resize na
                                primeira coluna de dados, -1 se estiver
                                dando resize na coluna de titulos
                             */
            DragColStartPos; /* Posicao em pixels do comeco da coluna
                                que esta sendo modificada
                             */

static int  Lastxpos;  /* Posicao anterior da linha de feedback na tela */

static void ChangeMatrixWH(Ihandle *h,int col,int largura, int m);

#define RESIZE_COLOR  0x666666L

/**************************************************************************
***************************************************************************
*
*   Funcoes para resize interativo de colunas
*
***************************************************************************
***************************************************************************/


/*

%F Verifica se o mouse esta na intersecao entre dois titulos de coluna. Se
   estiver inicia o resize.
%i h : handle da matriz,
   x,y : coordenadas do mouse (coordenadas do canvas).

%o Retorna 1 se iniciou um resize, 0 caso contrario.
*/
int iupmatColresTry(Ihandle *h, int x, int y)
{
  Tmat *mat=(Tmat*)matrix_data(h);
  int achou,ativo,width,col;
  char *attr;

  attr = IupGetAttribute(h,IUP_RESIZEMATRIX);
  if (attr)
    ativo = !iupStrEqual(attr,IUP_NO);
  else
    ativo = 0;

  if(mat_ht(mat) && y<mat_ht(mat) && ativo)
  {
    /* Esta na area de titulos de coluna*/
    achou = 0;
    width = mat_wt(mat);

    if (abs(width-x) < TOL)
    {
      /* Estou na interface entre a coluna de titulos e a primeira coluna */
      col = 0;
      achou = 1;
      DragColStartPos = 0;
    }
    else
    {
      /* Procura para ver se estou em outra interface */
      DragColStartPos= width;
      for (col=mat_fc(mat);col<=mat_lc(mat)&&!achou;col++)
      {
        width+=mat_w(mat)[col];
        if (abs(width-x) < TOL)
          achou = 1;
        if (!achou)
          DragColStartPos= width;
      }
    }

    if (achou)
    {
      Dragging = 1;
      Lastxpos = -1;
      DragCol = col-1;

      return 1;
    }
  }
  return 0;
}

/*

%F Termina o resize interativo de colunas. Chama a funcao ChangeMatrixWidth para
   mudar realmente o tamanho da coluna.
%i h : handle da matriz,
   x : coordenada x do mouse (coordenadas do canvas).

*/
void iupmatColresFinish(Ihandle *h, int x)
{
  int  charwidth,charheight, width;
  int  y1, y2;
  Tmat *mat=(Tmat*)matrix_data(h);

  iupdrvGetCharSize(h,&charwidth,&charheight);
  width = x - DragColStartPos-DECOR_X;

  if (width < charwidth)
    width = charwidth ;  /* Tamanho minimo para a celula */

  /* Apaga feedback */
  if (Lastxpos != -1)
  {

    y1 = mat_ht(mat);
    y2 = YmaxCanvas(mat);

    cdCanvasWriteMode(mat->cdcanvas,CD_XOR);
    cdCanvasForeground(mat->cdcanvas,RESIZE_COLOR);
    cdCanvasLine(mat->cdcanvas,Lastxpos,INVY(y1),Lastxpos,INVY(y2));
    cdCanvasWriteMode(mat->cdcanvas,CD_REPLACE);

  }

  ChangeMatrixWH(h,DragCol,width+DECOR_X,MAT_COL);
  Dragging = 0;
}

/*

%F Muda a largura da coluna interativamente, so muda a linha na tela.
   Qando o usuario termina o drag, a funcao iupmatColresFinish e chamada
   para realmente mudar a largura da coluna.
%i h : handle da matriz,
   x : coordenada x do mouse (coordenadas do canvas).

*/
void iupmatColresMove(Ihandle *h, int x)
{
  Tmat *mat=(Tmat*)matrix_data(h);
  int y1,y2, charwidth,charheight;

  iupdrvGetCharSize(h,&charwidth,&charheight);

  /* Se tamanho da coluna ficou muito pequeno, nao muda tamanho da coluna */
  if (x < DragColStartPos+charwidth+DECOR_X)
    return;

  y1 = mat_ht(mat);
  y2 = YmaxCanvas(mat);

  cdCanvasWriteMode(mat->cdcanvas, CD_XOR);
  cdCanvasForeground(mat->cdcanvas, RESIZE_COLOR);

  /* Se nao e a primeira vez, retira linha antiga */
  if (Lastxpos != -1)
    cdCanvasLine(mat->cdcanvas,Lastxpos,INVY(y1),Lastxpos,INVY(y2));

  cdCanvasLine(mat->cdcanvas,x,INVY(y1),x,INVY(y2));

  Lastxpos = x;
  cdCanvasWriteMode(mat->cdcanvas, CD_REPLACE);
}


static void setMatrixCursor(Ihandle *mat)
{
  char *cursor = iupGetEnv(mat, "Iup_matrix_cursor");
  if(cursor)
    IupStoreAttribute(mat,IUP_CURSOR, cursor);
}

/*

%F Muda o cursor quando ele passa sobre uma juncao de titulos de coluna.
%i h : handle da matriz,
   x,y : coordenadas do mouse (coordenadas do canvas).

*/
void iupmatColresChangeCursor(Ihandle *h, int x, int y)
{
  Tmat *mat=(Tmat*)matrix_data(h);
  int ativo;
  char *attr = IupGetAttribute(h,IUP_RESIZEMATRIX);
  if (attr)
    ativo = !iupStrEqual(attr,IUP_NO);
  else
    ativo = 0;

  if(mat_ht(mat) && y<mat_ht(mat) && ativo)
  {
    /* Esta na area de tituos de coluna e o modo de resize esta ligado */
    int achou=0,width=mat_wt(mat),col;

    if(abs(width-x)<TOL)
      achou=1;            /*titulos de linha */
    else
    {
      for (col=mat_fc(mat);col<=mat_lc(mat)&&!achou;col++)
      {
        width+=mat_w(mat)[col];
        if (abs(width-x) < TOL)
          achou = 1;
      }
    }

    if (achou)
      iupCpiDefaultSetAttr(mat_self(mat),IUP_CURSOR,IUP_RESIZE_W);
    else
      setMatrixCursor(mat_self(mat)); /* Esta na area vazia apos a ultima coluna */
  }
  else
    setMatrixCursor(mat_self(mat));
}

/*
%F Retorna se estamos no meio de uma interacao ou nao.
*/
int iupmatColresResizing(void)
{
  return Dragging;
}


/**************************************************************************
***************************************************************************
*
*   Funcoes para setar e pegar a largura de uma coluna/linha.
*
***************************************************************************
***************************************************************************/

/*

%F Muda a largura de uma coluna ou a altura de uma linha, redesenhando se
   estiver visivel
%i col : coluna/linha a ter sua largura modificada, Esta funcao trbalha com a
         representacao interna para o numero de coluna, ou seja col = 0 e
         a primeira coluna, col = -1 e a coluna de titulos. O mesma
         representacao e' usada para linhas
   largura : largura em pixels da coluna/linha col.
   m  : Indica se estou atuando sobre uma coluna (MAT_COL) ou linha (MAT_LIN).

*/
static void ChangeMatrixWH(Ihandle *h,int col,int largura, int m)
{
 Tmat *mat=(Tmat*)matrix_data(h);
 Ihandle *d   = IupGetDialog(h);
 int visible = (iupCheck(h,IUP_VISIBLE)==YES) && (iupCheck(d,IUP_VISIBLE)==YES);
 int err,drawmode = m;
 Tlincol *p;

 if (m == MAT_LIN)
   p = &(mat->lin);
 else
   p = &(mat->col);

 IsCanvasSet(mat,err);

 if (col != -1)  /* col -1 indica largura do titulo */
 {
   if (largura > p->size)   /* largura nao pode ser maior que area util */
     largura = p->size;

   /* Recalcula espaco total ocupado pelas colunas/linhas */
   p->totalwh=p->totalwh - p->wh[col] + largura;
   p->wh[col]=largura;
 }
 else if (p->titlewh > 0)      /* so muda tamanho, se ja' existir titulo */
 {
   if(largura > XmaxCanvas(mat))
     largura = XmaxCanvas(mat);
   /* Recalcula espaco que sobra para as outras celulas */
   p->size=p->size+p->titlewh-largura;
   p->titlewh = largura;

   drawmode = DRAW_ALL; /* Tenho que redesenhar os titulos de linha/coluna tambem */
 }
 iupmatGetLastWidth(h,m);

 /* Se a coluna/linha esta na parte visivel, redesenha a matriz */
 if (((col >= p->first)&&(col <= p->last)) || col==-1)
  if (visible && err == CD_OK)
  {
    SetSb(m);
    iupmatDrawMatrix(h,drawmode);
    iupmatShowFocus(h);
  }
}


/*

%F Muda a largura/altura de uma coluna/linha. Chamada quando o atributo WIDTHx
   ou o atributo HEIGHTx e' alterado.
   Pega o novo tamanho da coluna/linha, transforma da unidade do IUP para pixels
   e chama a funcao ChangeMatrixWH
%i h   : handle da matriz,
   col : coluna/linha a ter sua largura modificada. col = 1 corresponde a
         primeira coluna/linha. col = 0 corresponde a coluna/linha de titulos.
   m   : Indica se estou operando sobre linhas(MAT_LIN) ou colunas(MAT_COL).
*/
void iupmatColresSet(Ihandle *h, char* value, int col, int m, int pixels)
{
 int  largura=0;

 sscanf(value,"%d",&largura);

 col--;  /* a celula super. esq. e 1:1 para o usuario, internamente e 0:0 */

 if (largura)
 {
   if (pixels)
   {
     if (m == MAT_COL)
       largura = (int)(largura+DECOR_X);
     else
       largura = (int)(largura+DECOR_Y);
   }
   else
   {
     int charwidth, charheight;
     iupdrvGetCharSize(h,&charwidth,&charheight);
     /* Transforma p/ pixels */
     if (m == MAT_COL)
       largura = (int)((largura/4.)*charwidth)+DECOR_X;
     else
       largura = (int)((largura/8.)*charheight)+DECOR_Y;
   }
 }
 ChangeMatrixWH(h,col,largura,m);
}



/*

%F Pega a largura/altura de uma coluna/linha. Os atributos WIDTH e HEIGHT nao
   podem ser consultados pois quando a largura/altura da coluna/linha sao
   mudados dinamicamente, o valor do atributo fica desatualizado. Alem disso,
   nem todas as colunas/linhas tem seu atributo WIDTH/HEIGHT definidos.
   (Largura e altura default)
%i h   : handle da matriz,
   col : coluna/linha que tera seua largura/altura consultada. col = 1 indica
         a primeira coluna/linha, col = 0 indica a coluna/linha de titulos.
   m   : Indica se estou operando sobre linhas(MAT_LIN) ou colunas(MAT_COL).

%o retorna uma string com a largura, ja retorna em valores tipo IUP.

*/
char *iupmatColresGet(Ihandle *h, int col, int m, int pixels)
{
  static char width[100];
  Tmat *mat = (Tmat*)matrix_data(h);
  int w;
  Tlincol *p;

  if (m == MAT_LIN)
    p = &(mat->lin);
  else
    p = &(mat->col);

  if (col > p->num || col < 0)
    return NULL;

  if (col == 0)
    w = p->titlewh;
  else
  {
    col--;  /* a celula super. esq. e 1:1 para o usuario, internamente e 0:0 */
    w = p->wh[col];
  }


  if (w)
  {
    if (pixels)
    {
      if (m == MAT_COL)
        sprintf(width,"%d",(int)(w-DECOR_X));
      else
        sprintf(width,"%d",(int)(w-DECOR_Y));
    }
    else
    {
      int charwidth,charheight;
      iupdrvGetCharSize(h,&charwidth,&charheight);
      if (m == MAT_COL)
        sprintf(width,"%d",(int)((w-DECOR_X)*4/charwidth));
      else
        sprintf(width,"%d",(int)((w-DECOR_Y)*8/charheight));
    }
  }
  else
    strcpy(width,"0");

  return width;
}
