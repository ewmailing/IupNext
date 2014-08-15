/** \file
 * \brief iupmatrix control
 * cel selection
 *
 * See Copyright Notice in iup.h
 *  */

/**************************************************************************
***************************************************************************
*
*   Funcoes usadas para marcacao de celulas
*
***************************************************************************
***************************************************************************/


#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <iup.h>
#include <iupcpi.h>
#include <cd.h>

#include "iupmatrix.h"
#include "matridef.h"
#include "matrixcd.h"
#include "imdraw.h"
#include "imscroll.h"
#include "imaux.h"
#include "immark.h"


#define MINMAX(a,b,min,max) ((a > b)?(min = b,max = a):(min = a,max = b))


/* MarkLin e MarkCol sao usadas para guardar a celula atual quando estou
   marcando um bloco
*/
static int MarkLin, MarkCol;

/* FullLin e FullCol indicam se estou marcando linhas/colunas inteiras */
static int MarkFullLin = 0,
           MarkFullCol = 0,
           LastMarkFullCol = 0,
           LastMarkFullLin = 0;

/* Marcado indica o numero de celulas marcadas */
static int Marcado = 0;


/**************************************************************************
***************************************************************************
*
*   Funcoes para Marcar/Desmarcar linhas e colunas completas
*
***************************************************************************
***************************************************************************/

/*
%F Marca uma coluna completa. Redesenha esta coluna.

%i h      : handle da matriz,
   col    : coluna a ser marcada.
   redraw : Deve ou nao redesenhar as colunas
*/
static void MarcaColuna(Ihandle *h, int col, int redraw)
{
  Tmat *mat=(Tmat*)matrix_data(h);
  int i;

  for(i=0;i<mat_nl(mat);i++,Marcado++)
    iupmatMarkCellSet(mat,i,col,1);

  mat_colmarked(mat)[col]++;

  if (redraw)
  {
    if(mat_colmarked(mat)[col] == 1)
      iupmatDrawColumnTitle(h,col,col);
    iupmatDrawCells(h,mat_fl(mat),col,mat_ll(mat),col);
  }
}

/*
%F Desmarca uma coluna completa. Redesenha esta coluna.
%i h      : handle da matriz,
   col    : coluna a ser desmarcada.
   redraw : Deve ou nao redesenhar as colunas
*/
static void DesmarcaColuna(Ihandle *h, int col, int redraw)
{
  Tmat *mat=(Tmat*)matrix_data(h);
  int i;

  for(i=0;i<mat_nl(mat);i++,Marcado--)
    iupmatMarkCellSet(mat,i,col,0);

  mat_colmarked(mat)[col]--;

  if (redraw)
  {
    if(!mat_colmarked(mat)[col])
      iupmatDrawColumnTitle(h,col,col);
    iupmatDrawCells(h,mat_fl(mat),col,mat_ll(mat),col);
  }
}

/*
%F Marca uma linha completa. Redesenha esta linha.

%i h      : handle da matriz,
   lin    : linha a ser marcada.
   redraw : Deve ou nao redesenhar as linhas
*/
static void MarcaLinha(Ihandle *h, int lin, int redraw)
{
  Tmat *mat=(Tmat*)matrix_data(h);
  int i;

  for(i=0;i<mat_nc(mat);i++,Marcado++)
    iupmatMarkCellSet(mat,lin,i,1);

  mat_linmarked(mat)[lin]++;

  if (redraw)
  {
    if(mat_linmarked(mat)[lin] == 1)
      iupmatDrawLineTitle(h,lin,lin);
    iupmatDrawCells(h,lin,mat_fc(mat),lin,mat_lc(mat));
  }
}

/*
%F Desmarca uma linha completa.  Redesenha esta linha.

%i h : handle da matriz,
   lin : linha a ser desmarcada.
   redraw : Deve ou nao redesenhar as linhas
*/
static void DesmarcaLinha(Ihandle *h, int lin, int redraw)
{
  Tmat *mat=(Tmat*)matrix_data(h);
  int i;

  for(i=0;i<mat_nc(mat);i++,Marcado--)
    iupmatMarkCellSet(mat,lin,i,0);
  mat_linmarked(mat)[lin]--;

  if (redraw)
  {
    if(!mat_linmarked(mat)[lin])
      iupmatDrawLineTitle(h,lin,lin);
    iupmatDrawCells(h,lin,mat_fc(mat),lin,mat_lc(mat));
  }
}


/**************************************************************************
***************************************************************************
*
*   Funcoes para Marcar/Desmarcar blocos usando Drag
*
***************************************************************************
***************************************************************************/

/*
%F Marca ou desmarca uma serie de linhas.

%i h : handle da matriz,
   lin : linha final da marcacao.
*/
static void MarcaRegiaoLin(Ihandle *h, int lin)
{
  Tmat *mat=(Tmat*)matrix_data(h);

  if (lin == MarkLin)
    return;

  if ((lin >= mat_lin(mat) && lin < MarkLin) ||
      (lin <= mat_lin(mat) && lin > MarkLin))     /* Desmarcando */
    DesmarcaLinha(h,MarkLin,1);
  else
    MarcaLinha(h,lin,1);

  MarkLin = lin;
  MarkCol = mat_nc(mat) - 1;
}

/*
%F Marca ou desmarca uma serie de colunas.

%i h : handle da matriz,
   col : coluna final da marcacao.
*/
static void MarcaRegiaoCol(Ihandle *h, int col)
{
  Tmat *mat=(Tmat*)matrix_data(h);

  if (col == MarkCol)
    return;

  if ((col >= mat_col(mat) && col < MarkCol) ||
      (col <= mat_col(mat) && col > MarkCol))     /* Desmarcando */
    DesmarcaColuna(h,MarkCol,1);
  else /* Marcando */
    MarcaColuna(h,col,1);

  MarkCol = col;
  MarkLin = mat_nl(mat) - 1;
}

/*
%F Marca/desmarca uma regiao da matriz. O bloco a ser afetado e definido
   pela celula dada como parametro e pela celula dada pelo par
   MarkLin e MarkCol.

%i h : handle da matriz,
   lin,col : celula final da marcacao.
*/
static void MarcaRegiao(Ihandle *h, int lin, int col)
{
  Tmat *mat=(Tmat*)matrix_data(h);
  int RedrawLin= -1, RedrawCol= -1;
  int colstart=0,colend=0,linstart=0,linend=0, i;


  if (MarkFullCol)  /* Estou marcando/desmarcando colunas inteiras */
  {
    MarcaRegiaoCol(h,col);
    return;
  }

  if (MarkFullLin)  /* Estou marcando/desmarcando linhas inteiras */
  {
    MarcaRegiaoLin(h,lin);
    return;
  }

  if (lin != MarkLin)
  {
    MINMAX (mat_col(mat),MarkCol,colstart,colend);  /* Coloca o < de mat_col(mat) e MarkCol em colstart, e o outro em colend */

    if ((lin >= mat_lin(mat) && lin < MarkLin) ||
        (lin <= mat_lin(mat) && lin > MarkLin))     /* Desmarcando */
    {
       /* lin e MarkLin diferem, no maximo, de 1 */

       /* Decrementa o numero de vezes que a celula foi marcada, assim    */
       /* celulas de outra regiao, ja marcada, que esteja superposta pela */
       /* regiao atual nao sao apagadas */
       for (i=colstart;i<=colend;i++)
         iupmatMarkCellSet(mat,MarkLin,i,0);

       Marcado -= (colend-colstart+1);
       RedrawLin = MarkLin;
    }
    else   /* Marcando */
    {
       /* lin e MarkLin diferem, no maximo, de 1 */
       for (i=colstart;i<=colend;i++)
         iupmatMarkCellSet(mat,lin,i,1);

       Marcado += (colend - colstart + 1);
       RedrawLin = lin;
    }
    MarkLin = lin;
  }

  if (col != MarkCol)
  {
    MINMAX (mat_lin(mat),MarkLin,linstart,linend);  /* Coloca o < de mat_lin(mat) e MarkLin em linstart, e o outro em linend */

    if ((col >= mat_col(mat) && col < MarkCol) ||
        (col <= mat_col(mat) && col > MarkCol))     /* Desmarcando */
    {
       /* col e MarkCol diferem, no maximo, de 1 */
       for (i=linstart;i<=linend;i++)
         iupmatMarkCellSet(mat,i,MarkCol,0);

       Marcado -= (linend - linstart + 1);
       RedrawCol = MarkCol;
    }
    else   /* Marcando */
    {
       /* col e MarkCol diferem, no maximo, de 1 */
       for (i=linstart;i<=linend;i++)
         iupmatMarkCellSet(mat,i,col,1);

       Marcado += (linend - linstart + 1);
       RedrawCol = col;
    }
    MarkCol = col;
  }

  if (RedrawLin != -1)
    iupmatDrawCells(h,RedrawLin,colstart,RedrawLin,colend);
  if (RedrawCol != -1)
    iupmatDrawCells(h,linstart,RedrawCol,linend,RedrawCol);
}


/*
%F Funcao chamada quando estou fazendo a marcacao de varias celulas.
   Faz o scroll da planilha caso o drag saia fora da area de visualizacao.

%i h : handle da matriz,
   x,y : coordenadas do mouse (coordenadas do canvas).
*/
void iupmatMarkDrag(Ihandle *h, int x, int y)
{
  Tmat *mat=(Tmat*)matrix_data(h);
  int lin,col,i,j,incl,incc;
  int err;
  int  mark_mode = iupmatMarkGetMode(h);

  if (mark_mode == MARK_NO) return;

  /* Pega coordenada (lin,col) do cursor */
  if (!iupmatGetLineCol(h,x,y,&lin,&col))
    return;

  /* Nao existe drag dentro da mesma celula... */
  if (lin == mat_lin(mat) && col == mat_col(mat))
    return;

  /* Critica de acordo com o tipo de marcacao */
  if (mark_mode == MARK_LIN)
  {
    if(col != -1) return;
    if (!mat_linmarked(mat)[mat_lin(mat)])  return;
  }
  else if (mark_mode == MARK_COL)
  {
    if(lin != -1) return;
    if (!mat_colmarked(mat)[mat_col(mat)])  return;
  }
  else if (mark_mode == MARK_LINCOL)
  {
    if ((lin != -1) && (col != -1)) return;
    if ((lin == -1) && (col == -1))
    {
      if ((!mat_colmarked(mat)[mat_col(mat)]) && (!mat_linmarked(mat)[mat_lin(mat)])) return;
    }
    else
    {
      if ((lin == -1) && (!mat_colmarked(mat)[mat_col(mat)])) return;
      if ((col == -1) && (!mat_linmarked(mat)[mat_lin(mat)])) return;
    }
  }

  IsCanvasSet(mat,err);

  if (col == -2)      /* Area da scrollbar vertical */
  {
    if (!MarkFullLin) ScrollRight(h);
    col = mat_lc(mat);
  }
  else if (col == -1) /* Coluna de titulos de linha */
  {
    if (!MarkFullLin) ScrollLeft(h);
    col = mat_fc(mat);
  }

  if (lin == -2)      /* Area da scrollbar horizontal */
  {
    if (!MarkFullCol) ScrollDown(h);
    lin = mat_ll(mat);
  }
  else if (lin == -1) /* Linha de titulos de coluna */
  {
    if (!MarkFullCol) ScrollUp(h);
    lin = mat_fl(mat);
  }

  incl = (lin > MarkLin)?1:-1;
  incc = (col > MarkCol)?1:-1;

  /* Chama MarcaRegiao mais de uma vez, no caso de o mouse ter sido      */
  /* "arremessado", e nenhum evento tenha sido gerado em uma linha/coluna*/
  for (i=MarkLin,j=MarkCol; i!=lin || j !=col;)
  {
    if (i != lin)  i += incl;
    if (j != col)  j += incc;
    MarcaRegiao(h,i,j);
  }

  if (mat->redraw) { cdCanvasFlush(cdcv(mat)); mat->redraw = 0; }
}


/**************************************************************************
***************************************************************************
*
*   Funcoes para Marcar desmarcar blocos de celulas
*
***************************************************************************
***************************************************************************/

/*

%F Desmarca ultimo bloco a ser marcado. Os limites do bloco sao definidos
   pela celula que contem o foco e pela celula dada por MarkLin e MarkCol.
   NAO redesenha a matriz.
%i h : handle da matriz,

*/
void iupmatMarkDesmarcaBloco(Ihandle *h)
{
  Tmat *mat=(Tmat*)matrix_data(h);
  int i,j, inc;

  if (!Marcado) return;

  if (LastMarkFullLin)
  {
    inc = mat_lin(mat) < MarkLin?1:-1;
    for(i=mat_lin(mat);;i+=inc)
    {
      DesmarcaLinha(h,i,0);
      if(i == MarkLin) break;
    }
  }
  else if (LastMarkFullCol)
  {
    inc = mat_col(mat) < MarkCol?1:-1;
    for(i=mat_col(mat);;i+=inc)
    {
      DesmarcaColuna(h,i,0);
      if(i == MarkCol) break;
    }
  }
  else
  {
    int inclin = (mat_lin(mat) < MarkLin)?1:-1;
    int inccol = (mat_col(mat) < MarkCol)?1:-1;

    for (i=mat_lin(mat);;i+=inclin)
    {
      for(j=mat_col(mat);;j+=inccol)
      {
        iupmatMarkCellSet(mat,i,j,0);
        Marcado--;
        if (j == MarkCol) break;
      }
      if (i == MarkLin) break;
    }
  }
}

/*
%F Marca um bloco de celulas. Se a celula ja esta marcada, incrementa seu
   contador de numero de vezes que ela foi marcada. (Quando duas marcacaoes
   se sobrepoe). Marca desde a celula que contem o foco ate a celula dada
   como parametro. Atualiza MarkLin e MarkCol
   NAO redesenha a matriz.

%i h : handle da matriz,
   lin,col : coordenadas da celula que, juntamente com a celula que
             atualmente tem o foco, delimitam o bloco a ser marcado.
*/
void iupmatMarkBloco(Ihandle *h,int lin,int col)
{
  Tmat *mat=(Tmat*)matrix_data(h);
  int i,j,inc;

  if (iupmatMarkFullLin())
  {
    inc = mat_lin(mat) < lin?1:-1;
    for(i=mat_lin(mat);;i+=inc)
    {
      MarcaLinha(h,i,0);
      if(i == lin) break;
    }
    MarkLin = lin;
  }
  else if (iupmatMarkFullCol())
  {
    inc = mat_col(mat) < col?1:-1;
    for(i=mat_col(mat);;i+=inc)
    {
      MarcaColuna(h,i,0);
      if(i == col) break;
    }
    MarkCol = col;
  }
  else
  {
    int inclin = (mat_lin(mat) < lin)?1:-1;
    int inccol = (mat_col(mat) < col)?1:-1;

    for (i=mat_lin(mat);;i+=inclin)
    {
      for(j=mat_col(mat);;j+=inccol)
      {
        iupmatMarkCellSet(mat,i,j,1);
        Marcado++;
        if (j == col) break;
      }
      if (i == lin) break;
    }
    MarkLin = lin;
    MarkCol = col;
  }
}

void iupmatMarkCellSet(Tmat *mat,int lin,int col, int mark)
{
  if (!mat->valcb)
    mat_v(mat)[lin][col].mark = mark;
  else if (mat->markeditcb)
    mat->markeditcb(mat->self,lin+1,col+1, mark);
  else
  {
    char str[100];
    sprintf(str, "MARK%d:%d", lin, col);
    if (mark)
      iupSetEnv(mat->self, str, "1");
    else
      iupSetEnv(mat->self, str, NULL);
  }
}

int iupmatMarkCellGet(Tmat *mat,int lin,int col)
{
  if (!mat->valcb)
    return mat_v(mat)[lin][col].mark;
  else
  {
    if (mat->markcb)
      return mat->markcb(mat->self,lin+1,col+1);
    else
    {
      char str[100];
      sprintf(str, "MARK%d:%d", lin, col);
      return iupGetEnvInt(mat->self, str);
    }
  }
}

/*
%F Desmarca todas as celulas marcadas
   Redesenha as celulas afetadas

%i h : handle da matriz.
*/
static void DesmarcaTodas(Ihandle *h)
{
  Tmat *mat=(Tmat*)matrix_data(h);
  int i,j,old;
  int min_col = mat_nc(mat)+1,
      max_col = -1,
      min_lin = mat_nl(mat)+1,
      max_lin = -1;

  for (i=0;i<mat_nl(mat);i++)
  {
    for (j=0;j<mat_nc(mat);j++)
    {
      old = iupmatMarkCellGet(mat,i,j);
      iupmatMarkCellSet(mat,i,j,0);
      if (old)
      {
        if(i<min_lin) min_lin = i;
        if(i>max_lin) max_lin = i;
        if(j<min_col) min_col = j;
        if(j>max_col) max_col = j;
      }
    }
    old = mat_linmarked(mat)[i];
    mat_linmarked(mat)[i]=0;
    if(old)
      iupmatDrawLineTitle(h,i,i);
  }
  Marcado = 0;
  for (j=0;j<mat_nc(mat);j++)
  {
    old = mat_colmarked(mat)[j];
    mat_colmarked(mat)[j]=0;
    if(old)
      iupmatDrawColumnTitle(h,j,j);
  }

  /* Se preciso redesenhar alguma celula, os valores de min_lin, max_lin*/
  /* min_col e max_col foram alterados, mas so preciso testar um deles */
  if(max_lin != -1)
    iupmatDrawCells(h,min_lin,min_col,max_lin,max_col);
}


/**************************************************************************
***************************************************************************
*
*   Funcoes de consulta.
*
***************************************************************************
***************************************************************************/

int iupmatMarkFullLin(void)
{
  return MarkFullLin;
}

int iupmatMarkFullCol(void)
{
  return MarkFullCol;
}

void iupmatMarkReset(void)
{
  LastMarkFullLin = MarkFullLin;
  LastMarkFullCol = MarkFullCol;

  MarkFullLin = 0;
  MarkFullCol = 0;
}


/*
%F Verifica se a coluna esta ou nao marcada.

%i col - Numero da coluna.  Repare que o numero NAO e' dado na forma
        usual. 0 representa a coluna de titulos e 1 a primeira coluna
        da matriz.
%o Retorna um numero maior que 1 se a coluna estiver marcada, 0 caso contrario.
*/
int iupmatMarkColumnMarked (Ihandle *h, int col)
{
  Tmat *mat=(Tmat*)matrix_data(h);
  if (col == 0)  /* Titulos de linha nunca estao marcados... */
    return 0;
  return mat_colmarked(mat)[col-1];
}

/*
%F Verifica se a linha esta ou nao marcada.

%i lin - Numero da linha.  Repare que o numero NAO e' dado na forma
        usual. 0 representa a linha de titulos e 1 a primeira linha
        da matriz.
%o Retorna um numero maior que 1 se a linha estiver marcada, 0 caso contrario.
*/
int iupmatMarkLineMarked (Ihandle *h, int lin)
{
  Tmat *mat=(Tmat*)matrix_data(h);
  if (lin == 0) /* Titulos de coluna nunca estao marcados... */
    return 0;
  return mat_linmarked(mat)[lin-1];
}


/*
%F Retorna o modo de marcacao setado pelo usuario. Consulta o atributo
   "MARK_MODE" e retorna uma constante indicando o modo escolhido.

%i h : handle da matriz,
%o Retorna o modo de marcacao. Pode ser : MARK_CELL, MARK_LIN, MARK_COL,
   MARK_LINCOL ou MARK_NO.
*/
int iupmatMarkGetMode(Ihandle *h)
{
  char *mode = IupGetAttribute(h,"MARK_MODE");

  if (mode && *mode)
  {
    if (iupStrEqual(mode,"CELL"))
      return MARK_CELL;
    if (iupStrEqual(mode,"LIN"))
      return MARK_LIN;
    if (iupStrEqual(mode,"COL"))
      return MARK_COL;
    if (iupStrEqual(mode,"LINCOL"))
      return MARK_LINCOL;
  }
  return MARK_NO;
}


int iupmatMarkCritica(Ihandle *h, int *lin, int *col, int *shift, int *ctrl, int *duplo)
{
  Tmat *mat = (Tmat*)matrix_data(h);

  int  mark_mode = iupmatMarkGetMode(h);
  int  mult = !iupCheck(h,IUP_MULTIPLE)? 0 : 1;
  int  cont = (iupStrEqual(IupGetAttribute(h,IUP_AREA),"CONTINUOUS"))? 1 : 0;

  /* Se estava apontando para um titulo de coluna... */
  if (*lin == -1)
  {
    *lin = mat_fl(mat);
    *duplo = 0; /* Garante que nao vai interpretar como um double-click */
    if (mark_mode != MARK_NO && ((mark_mode == MARK_CELL && mult) || 
                                  mark_mode == MARK_COL || 
                                  mark_mode == MARK_LINCOL))
      MarkFullCol = 1;
  }
  /* Se estava apontando para um titulo de linha... */
  else if (*col == -1)
  {
    *col = mat_fc(mat);
    *duplo = 0; /* Garante que nao vai interpretar como um double-click */
    if (mark_mode != MARK_NO && ((mark_mode == MARK_CELL && mult) || 
                                  mark_mode == MARK_LIN || 
                                  mark_mode == MARK_LINCOL))
      MarkFullLin = 1;
  }

  /* Verifica se cliquei em um titulo de linha ou coluna (marcacao de
     linhas ou colunas inteiras) e nao estou marcando linhas e colunas
     inteiras
  */
  if (*shift)
  {
    if (MarkFullLin && !LastMarkFullLin)
      MarkFullLin = 0;
    if (MarkFullCol && !LastMarkFullCol)
      MarkFullCol = 0;
  }

  /* Testes de consitencia entre o tipo de marcacao pedido e o tipo
     de marcacao sendo efetuada
  */
  if (mark_mode == MARK_LIN && !MarkFullLin)
    mark_mode = MARK_NO;
  else if (mark_mode == MARK_COL && !MarkFullCol)
    mark_mode = MARK_NO;
  else if (mark_mode == MARK_LINCOL && !(MarkFullCol || MarkFullLin))
    mark_mode = MARK_NO;

  if (mark_mode == MARK_LINCOL)
  {
    if ( (MarkFullCol && (mat_markLC(mat) == MARK_LIN)) ||
         (MarkFullLin && (mat_markLC(mat) == MARK_COL)) )
    {
      *ctrl = 0;
      *shift = 0;
    }
    mat_markLC(mat) = (MarkFullCol)?MARK_COL:MARK_LIN;
  }

  /* Testes de consistencia em relacao a atributo multiplo */
  if (!mult || (mark_mode == MARK_NO))
  {
     *ctrl = 0;
     *shift = 0;
  }

  /* Testes de consistencia em relacao a atributo continuo */
  if (cont) *ctrl = 0;

  return mark_mode;
}

int iupmatMarkHide(Ihandle *h,int ctrl)
{
  /* Se o ctrl nao esta pressionado, desmarca TODAS as celulas marcadas
      ate agora
  */
  if (!ctrl)
    DesmarcaTodas(h);
  return 1;
}


void iupmatMarkShow(Ihandle *h,int ctrl, int lin, int col, int oldlin, int oldcol)
{
  Tmat *mat = (Tmat*)matrix_data(h);

  if (iupmatMarkFullLin())
  {
    MarcaLinha(h,lin,1);
    col = mat_nc(mat) - 1;
  }
  else if (iupmatMarkFullCol())
  {
    MarcaColuna(h,col,1);
    lin = mat_nl(mat) - 1;
  }
  else
  {
    if (iupmatMarkCellGet(mat,lin,col))
    {
      iupmatMarkCellSet(mat,lin,col,0);
      Marcado--;
    }
    else
    {
      iupmatMarkCellSet(mat,lin,col,1);
      Marcado++;
    }
  }

  /* Redesenha a celula anterior se estamos pressionando ctrl
    (para mudar o feedback da celula anterior)...
  */
  if(ctrl)
    iupmatDrawCells(h,oldlin,oldcol,oldlin,oldcol);

  /* Redesenha celula com o foco com o valor da marcacao corrente */
  if(lin >= 0 && col >= 0)
    iupmatDrawCells(h,lin,col,lin,col);

  MarkLin = lin;
  MarkCol = col;
}


/**************************************************************************
***************************************************************************
*
*   Funcoes para setar, pegar e mudar o modo de marcacao de celulas
*
***************************************************************************
***************************************************************************/

/*

%F Seta quais celuas estao marcadas. Desmarca todas as que ja estavam marcadas.
%i n : handle da matriz,
   v : String contendo a representacao das celulas a serem marcadas. Ha tres
       representacoes possiveis:
       a) "C...." onde .... corresponde a um numero de 0s e 1s igual ao
          numero de colunas da matriz, representando se cada coluna esta
          marcada ou nao. O primeiro numero corresponde a coluna 1,
          o segundo a coluna 2, etc...
       b) "L...." Analogo ao anterior, so que para as linhas da matriz.
       c) "...." onde .... corresponde a um numero de 0s e 1s igual ao
          numero de celulas na matriz, representando se cada celula esta
          marcada ou nao. Numa matriz de L linhas por C colunas, os C
          primeiros numeros correspondem as colunas da linha 1, os proximos C
          as colunas da linha 2, etc...

%o retorna v.

*/
void *iupmatMarkSet (Ihandle *n, char *v)
{
  Tmat *mat = (Tmat*)matrix_data(n);
  int i,j,old;
  char *p = v;
  Ihandle *d   = IupGetDialog(n);
  int visible = (iupCheck(n,IUP_VISIBLE)==YES) && (iupCheck(d,IUP_VISIBLE)==YES) ;
  int err;

  if (p==NULL)
  {
    DesmarcaTodas(n);
    return NULL;
  }
  else if (*p == 'C' || *p == 'c')
  {
    p++;
    Marcado = 0;
    for (j=0;j<mat_nc(mat);j++)
    {
      /* Seta o contador da celula para 1.  As funcoes de Marca e Desmarca
         Linha, incrementam e decrementam o contador.... Por isso nao posso
         usa-las.
      */
      old = mat_colmarked(mat)[j];
      if (*p++ == '1')
      {
        /* Marca todas as celulas da coluna j */
        for (i=0;i<mat_nl(mat);i++,Marcado++)
          iupmatMarkCellSet(mat,i,j,1);
        mat_colmarked(mat)[j] = 1;
        iupmatDrawColumnTitle(n,j,j);
      }
      else
      {
        /* Desmarca todas as celulas da coluna j */
        for (i=0;i<mat_nl(mat);i++)
          iupmatMarkCellSet(mat,i,j,0);
        mat_colmarked(mat)[j] = 0;
        if(old) iupmatDrawColumnTitle(n,j,j);
      }
    }
  }
  else if (*p == 'L' || *p == 'l')
  {
    p++;
    Marcado = 0;
    for (i=0;i<mat_nl(mat);i++)
    {
      /* Seta o contador da celula para 1.  As funcoes de Marca e Desmarca
         Coluna, incrementam e decrementam o contador.... Por isso nao posso
         usa-las.
      */
      old = mat_linmarked(mat)[i];
      if (*p++ == '1')
      {
        /* Marca todas as celulas da linha i*/
        for (j=0;j<mat_nc(mat);j++,Marcado++)
          iupmatMarkCellSet(mat,i,j,1);
        mat_linmarked(mat)[i] = 1;
        iupmatDrawLineTitle(n,i,i);
      }
      else
      {
        /* Desmarca todas as celulas da linha i */
        for (j=0;j<mat_nc(mat);j++)
          iupmatMarkCellSet(mat,i,j,0);
        mat_linmarked(mat)[i] = 0;
        if(old) iupmatDrawLineTitle(n,i,i);
      }
    }
  }
  else
  {
    Marcado = 0;
    for (i=0;i<mat_nl(mat);i++)
    {
      for (j=0;j<mat_nc(mat);j++)
      {
        if (*p++ == '1')
        {
          iupmatMarkCellSet(mat,i,j,1);
          Marcado++;
        }
        else
          iupmatMarkCellSet(mat,i,j,0);
      }
      old = mat_linmarked(mat)[i];
      mat_linmarked(mat)[i] = 0;
      if (old) iupmatDrawLineTitle(n,i,i);
    }
    for (j=0;j<mat_nc(mat);j++)
    {
      old = mat_colmarked(mat)[j];
      mat_colmarked(mat)[j]=0;
      if(old)
        iupmatDrawColumnTitle(n,j,j);
    }
  }
  /* Redesenha as celulas visiveis */

  IsCanvasSet(mat,err);
  if(visible && err == CD_OK)
    iupmatDrawCells(n,mat_fl(mat),mat_fc(mat),mat_ll(mat),mat_lc(mat));
  return v;
}

/*

%F Retorna as celulas marcadas.
%i h : handle da matriz.
%o retorna string contendo a representacao das celulas marcadas. O formato
   da string depende do modo aual de marcacao. Os formatos estao descritos
   no cabecalho da funcao iupmatSetMarked.

*/
char *iupmatMarkGet (Ihandle *h)
{
  static int numbaloc=0;
  static char *marked=NULL;

  Tmat *mat = (Tmat*)matrix_data(h);
  int i,j,numbneeded;
  char *p;
  int exist_mark = 0;           /* indica se tem alguem marcado */
  int mark_mode = iupmatMarkGetMode(h);

  if (mark_mode == MARK_NO)
   return NULL;

  numbneeded = mat_nl(mat)*mat_nc(mat) + 2 ; /* Mais 2 por causa do L ou C  e do '\0'*/

  if (numbneeded  > numbaloc)  /* Aloca espaco */
  {
    if (numbaloc)
      marked = realloc (marked,numbneeded *sizeof(char));
    else
      marked = malloc(numbneeded *sizeof(char));
    numbaloc = numbneeded ;
  }

  p = marked;

  if (mark_mode == MARK_LINCOL)
    mark_mode = mat_markLC(mat);

  exist_mark = 0;

  /* decide qual forma de string vai retornar, baseado em mark mode */
  switch(mark_mode)
  {
    case MARK_CELL:   for (i=0;i<mat_nl(mat);i++)
                        for (j=0;j<mat_nc(mat);j++)
                          if (iupmatMarkCellGet(mat,i,j))
                          {
                            exist_mark = 1;
                            *p++ = '1';
                          }
                          else
                            *p++ = '0';
                      *p = 0;
                      break;

    case MARK_LIN:    *p++ = 'L';
                      for(i=0;i<mat_nl(mat);i++)
                        if (iupmatMarkCellGet(mat,i,0))
                        {
                          exist_mark = 1;
                          *p++ = '1';
                        }
                        else
                          *p++ = '0';
                      *p = 0;
                      break;

    case MARK_COL:    *p++ = 'C';
                      for(j=0;j<mat_nc(mat);j++)
                        if (iupmatMarkCellGet(mat,0,j))
                        {
                          exist_mark = 1;
                          *p++ = '1';
                        }
                        else
                          *p++ = '0';
                      *p = 0;
                      break;
  }

  return exist_mark ? marked : NULL;
}


/*

%F Chamada quando da alteracao do modo de marcacao. Desmarca todas as
   celulas que estao marcadas.
%i h : handle da matriz.
   v : novo tipo de marcacao. Atualmente nao e usado para nada.
%o retorna v.

*/
void *iupmatMarkSetMode(Ihandle *n, char *v)
{
  Tmat *mat = (Tmat*)matrix_data(n);
  Ihandle *d   = IupGetDialog(n);
  int visible = (iupCheck(n,IUP_VISIBLE)==YES) && (iupCheck(d,IUP_VISIBLE)==YES);
  int err;

  IsCanvasSet(mat,err);

  if (visible &&err == CD_OK)
    DesmarcaTodas(n);
  return v;
}
