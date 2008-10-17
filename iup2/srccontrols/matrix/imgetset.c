/** \file
 * \brief iupmatrix control
 * attributes set and get
 *
 * See Copyright Notice in iup.h
 * $Id: imgetset.c,v 1.1 2008-10-17 06:20:15 scuri Exp $
 */

#include <stdlib.h> /* malloc, realloc */
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
#include "immem.h"
#include "imfocus.h"
#include "immark.h"
#include "imgetset.h"
#include "imedit.h"


/**************************************************************************
***************************************************************************
*
*   Set e Get do valor das celulas.
*
***************************************************************************
***************************************************************************/

/*

%F Seta o valor de uma celula da matriz, redesenha a celula se
   estiver visivel.
%i n : handle da matriz,
   lin,col : coordenadas da celula a ser modificada, (1,1) corresponde a
             celula do canto superior esquerdo.
   v : string com o novo valor da celula.
%o retorna v.

*/
void *iupmatSetCell (Ihandle *n, int lin ,int col, char *v)
{
 Tmat *mat    = (Tmat*)matrix_data(n);
 Ihandle *d   = IupGetDialog(n);
 int cellvisible;
 int visible = (iupCheck(n,IUP_VISIBLE)==YES) && (iupCheck(d,IUP_VISIBLE)==YES);

 int err;

 if (v==NULL)
   v="";

 IsCanvasSet(mat,err);

 /* Se a celula nao existe, retorna NULL */
 if ((lin < 1) || (col < 1) || (lin > mat_nl(mat)) || (col > mat_nc(mat)))
   return NULL;

 lin--;  /* a celula superior esquerda e 1:1 para o usuario, */
 col--;  /* internamente ela e 0:0                           */

  cellvisible = iupmatIsCellVisible(n,lin,col);

  if (mat->valeditcb)
  {
    mat->valeditcb(n,lin+1,col+1,v);
  }
  else if (!mat->valcb)
  {
    iupmatMemAlocCell(mat,lin,col,strlen(v));
    strcpy(mat_v(mat)[lin][col].value,v);
  }

  if (visible && cellvisible && err == CD_OK)
  {
    iupmatDrawCells(n, lin, col, lin, col);
    iupmatShowFocus(n);
  }

  return v;
}


/*
%F Retorna o valor de uma celula da matriz
%i n : handle da matriz,
   lin,col : coordenadas da celula a ser consultada, (1,1) corresponde a
             celula do canto superior esquerdo.
%o retorna o valor da celula.
   Used only by getattribute method.
*/
char *iupmatGetCell (Ihandle *n, int lin, int col)
{
  Tmat *mat = (Tmat*)matrix_data(n);

  /* Se a celula nao existe, retorna NULL */
  if ((lin < 0) || (col < 0) || (lin > mat_nl(mat)) || (col > mat_nc(mat)))
    return NULL;

  lin--;  /* a celula superior esquerda e´ 1:1 para o usuario, */
  col--;  /* internamente ela e´ 0:0                           */

  if (lin==mat_lin(mat) && col==mat_col(mat) && iupCheck(mat_edtdatah(mat), "VISIBLE")==YES)
    return iupmatEditGetValue(mat);
  else
    return iupmatGetCellValue(n,lin,col);
}



/**************************************************************************
***************************************************************************
*
*   Seta/Consulta a celula que contem o foco.
*
***************************************************************************
***************************************************************************/

/*

%F Seta a celula que contem o focus. Se a celula nao estiver visivel,
   muda a parte visivel da matriz para conter a celula como o foco.
%i h : handle da matriz,
   v : string contendo a celula que vai conter o foco. E da forma "%d:%d",
       onde o primeiro numero indica a linha e o segundo a coluna. "1:1"
       corresponde a celula do canto superior esquerdo.

*/
void iupmatSetFocusPosition(Ihandle *h, char *v, int call_cb)
{
  Tmat *mat=(Tmat*)matrix_data(h);
  int lin,col;
  int redraw = 0;
  Ihandle *d   = IupGetDialog(h);
  int visible = (iupCheck(h,IUP_VISIBLE)==YES) && (iupCheck(d,IUP_VISIBLE)==YES);
  int err;

  IsCanvasSet(mat,err);

  if (!v) return;

  if (iupStrToIntInt(v,&lin,&col,':')!=2)
    return;

  /* Se a celula nao existe, retorna */
  if ((lin < 1) || (col < 1) || (lin > mat_nl(mat)) || (col > mat_nc(mat)))
    return;

  lin--;  /* a celula superior esquerda e 1:1 para o usuario, */
  col--;  /* internamente ela e 0:0                           */

  if (mat_w(mat)[col] == 0)
    return;

  /* se a matriz esta' com focus, pede permissao a aplicacao */
  /* para sair da celula. Quando a matriz nao esta' com o    */
  /* focus, a aplicacao ja' foi avisada na perda do focus    */
  if (mat_hasiupfocus(mat) && call_cb)
  {
    if (iupmatCallLeavecellCb(h) == IUP_IGNORE)
      return;
  }

  /* Se a linha estiver invisivel */
  if (lin<mat_fl(mat) || lin>mat_ll(mat))
  {
    /* faz a primeira linha ser aquela que vai conter o focus */
    mat_fl(mat) = lin;
    iupmatGetLastWidth(h,MAT_LIN);
    redraw = 1;
  }

  /* Se a coluna estiver invisivel */
  if (col<mat_fc(mat) || col>mat_lc(mat))
  {
    /* Faz a primeira coluna ser aquela que vai conter o focus */
    mat_fc(mat) = col;
    iupmatGetLastWidth(h,MAT_COL);
    redraw = 1;
  }

  if(visible && err == CD_OK)
  {
    iupmatHideFocus(h);
    iupmatSetFocusPos(h,lin,col);

    if (redraw)
      iupmatDrawMatrix(h,DRAW_ALL);
    iupmatShowFocus(h);
  }

  /* Se a matriz esta' com focus, avisa a aplicacao da nova
     celula que tem o foco. Caso contrario, quando a matriz
     ganhar o foco entao a aplicacao sera avisada.
  */
  if (mat_hasiupfocus(mat) && call_cb)
   iupmatCallEntercellCb(h);
}

/*

%F Consulta a celula que contem o focus.
%i h : handle da matriz,
%r string contendo a celula esta com o foco, na forma "%d:%d",
       onde o primeiro numero indica a linha e o segundo a coluna. "1:1"
       corresponde a celula do canto superior esquerdo.

*/
char *iupmatGetFocusPosition(Ihandle *h)
{
  static char cell[100];
  Tmat *mat=(Tmat*)matrix_data(h);
  if (mat)
   sprintf(cell,"%d:%d",mat_lin(mat)+1,mat_col(mat)+1);
  else
   return NULL;
  return cell;
}

/**************************************************************************
***************************************************************************
*
*   Muda o alinhamento do texto em uma dada coluna
*
***************************************************************************
***************************************************************************/

/*
%F Muda o alihamento dos textos em uma determinada coluna. Redesenha a
   coluna se estiver visivel.
%i h : handle da matriz,
   col : coluna ater seu alinhamento mudado. col = 1 representa aprimeira
         coluna da matriz.
*/
void iupmatSetColAlign (Ihandle *h, int col)
{
  Tmat *mat=(Tmat*)matrix_data(h);
  Ihandle *d   = IupGetDialog(h);
  int visible = (iupCheck(h,IUP_VISIBLE)==YES) && (iupCheck(d,IUP_VISIBLE)==YES);
  int err;

  IsCanvasSet(mat,err);

  if(col > mat_nc(mat) || col < 0)
    return;

  if(!visible || err!=CD_OK)
    return;

  if(col == 0) /* Alinhamento da coluna de titulos */
  {
    iupmatDrawLineTitle(h, mat_fl(mat), mat_ll(mat));
  }
  else
  {
    col--;  /* a celula super. esq. e 1:1 para o usuario, internamente e 0:0 */
    /* Se a coluna esta na parte visivel, redesenha a matriz */
    if((col >= mat_fc(mat))&&(col <= mat_lc(mat)))
    {
      iupmatDrawCells(h,mat_fl(mat),col,mat_ll(mat),col);
      iupmatShowFocus(h);
    }
  }
}


/**************************************************************************
***************************************************************************
*
*   Funcoes para a mudanca dos titulos de linha e coluna
*
***************************************************************************
***************************************************************************/

/*
%F Chamada quando da mudanca do titulo de uma linha, redesenha o titulo
   se ele for visivel
%i h : handle da matriz,
   linha : Linha que teve seu titulo alterado. linha = 1 indica que o
           titulo da primeira linha foi alterado. Ja linha = 0 indica que
           o titulo da celula que fica no canto entre os titulos de linha
           e coluna foi modificado.

*/
void iupmatSetTitleLine(Ihandle *h, int linha)
{
 Tmat *mat=(Tmat*)matrix_data(h);
 Ihandle *d   = IupGetDialog(h);
 int visible = (iupCheck(h,IUP_VISIBLE)==YES) && (iupCheck(d,IUP_VISIBLE)==YES);
 int err;

 if (linha > mat_nl(mat) || linha < 0)
   return;

 IsCanvasSet(mat,err);
 if (visible && err == CD_OK)
 {
   /* Se e' o titulo do canto, e o canto existe */
   if (linha == 0)
   {
     /* Redesenha o canto entre titulos de linha e coluna
        se ele existir
     */
     iupmatDrawTitleCorner(h);
   }
   else
   {
     linha--;
     /* Redesenha o titulo da linha, se ela estiver visivel */
     iupmatDrawLineTitle(h,linha,linha);
   }
 }
}


/*

%F  Chamada quando da mudanca do titulo de uma coluna, redesenha o titulo
    se ele for visivel
%i  h : handle da matriz,
    col : Coluna que teve seu titulo alterado. col = 1 indica que o
          titulo da primeira coluna foi alterado
*/
void iupmatSetTitleColumn(Ihandle *h, int col)
{
 Tmat *mat =(Tmat*)matrix_data(h);
 Ihandle *d   = IupGetDialog(h);
 int visible = (iupCheck(h,IUP_VISIBLE)==YES) && (iupCheck(d,IUP_VISIBLE)==YES);
 int err;

 if (col > mat_nc(mat) || col < 1)
   return;
 col--;

 IsCanvasSet(mat,err);
 if (visible && err==CD_OK)
 {
   /* Redesenha o titulo da coluna, se estiver visivel */
   iupmatDrawColumnTitle(h,col,col);
 }
}



/**************************************************************************
***************************************************************************
*
*   Funcoes para a mudanca e consulta da area visivel de uma matriz
*
***************************************************************************
***************************************************************************/


/*

%F Funcao para setar qual a area visivel da matriz.  Faz isto a partir
da informacao de qual celula deve ser colocada no canto superior
esquerdo da matriz.

%i h     - Handle da matriz
   value - Informacao de qual a celula que deve ficar no canto superio
           esquerdo da matriz, codificada no formato lin:col.

*/
void  iupmatSetOrigin(Ihandle *h, char *value)
{
 Tmat *mat = (Tmat*)matrix_data(h);
 Ihandle *d   = IupGetDialog(h);
 int visible = (iupCheck(h,IUP_VISIBLE)==YES) && (iupCheck(d,IUP_VISIBLE)==YES);
 int err;
 int lin=-1,col=-1,oldlin,oldcol,num=0;

 if (value == NULL)
   return;

 /* Pega os parametros. Um '*' indica que quero manter a tabela na mesma
   linha ou coluna.
 */
 if (iupStrToIntInt(value,&lin,&col,':') != 2)
 {
   if (lin != -1)
     col = mat_fc(mat) + 1;
   else if(col != -1)
     lin = mat_fl(mat) + 1;
   else
     return;
 }

 /* Se a celula nao existe, retorna NULL */
 if ((lin < 1) || (col < 1) || (lin > mat_nl(mat)) || (col > mat_nc(mat)))
   return;

 /* TODO: allow position based on the title cells also */

 lin--;  /* a celula superior esquerda e 1:1 para o usuario, */
 col--;  /* internamente ela e 0:0                           */

 IsCanvasSet(mat,err);

 oldcol = mat_fc(mat);
 oldlin = mat_fl(mat);

 mat_fc(mat) = col;
 iupmatGetLastWidth(h,MAT_COL);
 iupmatGetPos(h,MAT_COL);

 mat_fl(mat) = lin;
 iupmatGetLastWidth(h,MAT_LIN);
 iupmatGetPos(h,MAT_LIN);

 num = abs(oldcol - mat_fc(mat)) + abs(oldlin - mat_fl(mat));

 /* Se a matriz esta visivel, e houve mudanca na parte visivel da mesma,
   entao redesenha...
 */
 if (visible && (err == CD_OK) && num)
 {
  iupmatHideFocus(h);

  if(num == 1)
  {
    if(oldlin != mat_fl(mat))
      iupmatScroll(h,oldlin < mat_fl(mat)?SCROLL_DOWN:SCROLL_UP,oldlin,1);
    else
      iupmatScroll(h,oldcol < mat_fc(mat)?SCROLL_RIGHT:SCROLL_LEFT,oldcol,1);

    SetSbV;
    SetSbH;
  }
  else
  {
    SetSbV;
    SetSbH;

    iupmatDrawMatrix(h,DRAW_ALL);
  }

  iupmatShowFocus(h);
  mat->redraw = 1;
 }
}

/*

%F Funcao que retorna a coordenada da celula superior esquerda da parte
  atualmente visivel da matrix.

%i h - Handle da matriz.
%o Retorna uma string com a coordenada da celula, no formato lin:col

*/
char *iupmatGetOrigin(Ihandle *h)
{
  Tmat *mat = (Tmat*)matrix_data(h);
  static char val[100];

  sprintf(val,"%d:%d",mat_fl(mat)+1,mat_fc(mat)+1);
  return val;
}


/**************************************************************************
***************************************************************************
*
*   Funcao para tornar uma coluna/linha ativa/inativa
*
***************************************************************************
***************************************************************************/

/*
%F Funcao chamada para ativar ou desativar uma linha ou coluna da matriz.

%i h      - Handle da matriz
   mode   - Flag que indica se estou operando em linhas ou colunas
            [MAT_LIN|MAT_COL]
   lincol - Numero da linha ou coluna em questao (formato IUP)
   val    - Valor do atributo.
*/
void iupmatSetActive(Ihandle *h, int mode, int lincol, char *val)
{
  Tmat *mat = (Tmat*)matrix_data(h);
  int on = iupStrEqualNoCase(val,"ON") || iupStrEqualNoCase(val,"YES");
  Ihandle *d   = IupGetDialog(h);
  int visible = (iupCheck(h,IUP_VISIBLE)==YES) && (iupCheck(d,IUP_VISIBLE)==YES);
  int err;

  lincol--;

  if (mode == MAT_LIN)
    mat_lininactive(mat)[lincol] = !on;
  else
    mat_colinactive(mat)[lincol] = !on;

  IsCanvasSet(mat,err);
  if (visible && (err == CD_OK))
  {
    if(mode == MAT_LIN)
    {
      iupmatDrawLineTitle(h,lincol,lincol);
      iupmatDrawCells(h,lincol,mat_fc(mat),lincol,mat_lc(mat));
    }
    else
    {
      iupmatDrawColumnTitle(h,lincol,lincol);
      iupmatDrawCells(h,mat_fl(mat),lincol,mat_ll(mat),lincol);
    }
  }
}


/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

void iupmatSetRedraw(Ihandle *h, char *value)
{
 Tmat *mat = (Tmat*)matrix_data(h);
 Ihandle *d   = IupGetDialog(h);
 int visible = (iupCheck(h,IUP_VISIBLE)==YES) && (iupCheck(d,IUP_VISIBLE)==YES);
 int err;
 int type=DRAW_ALL;

 if (value == NULL)
   return;

 IsCanvasSet(mat,err);

 if (visible && (err == CD_OK))
 {
   if (value[0] == 'L' || value[0] == 'l')
     type = DRAW_LIN;
   else if (value[0] == 'C' || value[0] == 'c')
     type = DRAW_COL;

   if(type != DRAW_ALL)
   {
     int min=0,max=0;
     value++;
     if (iupStrToIntInt(value,&min,&max, ':')!=2)
       max = min;

     if(type==DRAW_LIN)
       iupmatDrawCells(h,min-1,mat_fc(mat),max-1,mat_lc(mat));
     else
       iupmatDrawCells(h,mat_fl(mat),min-1,mat_ll(mat),max-1);
   }
   else
   {
     iupmatDrawMatrix(h,DRAW_ALL);
   }

   iupmatShowFocus(h);
 }
}


/**************************************************************************
***************************************************************************
*
*   Funcoes para redraw de celulas.
*
***************************************************************************
***************************************************************************/

void iupmatSetRedrawCell(Ihandle* n, int lin, int col)
{
  Tmat *mat    = (Tmat*)matrix_data(n);
  Ihandle *d   = IupGetDialog(n);
  int visible  = (iupCheck(n,IUP_VISIBLE)==YES) && (iupCheck(d,IUP_VISIBLE)==YES);
  int err;

  IsCanvasSet(mat,err);
  if(!visible || err != CD_OK)
    return;

  if(lin == -1) /* Redesenha uma coluna */
  {
    iupmatHideFocus(n);
    iupmatDrawCells(n,mat_fl(mat),col-1,mat_ll(mat),col-1);
    iupmatShowFocus(n);
  }
  else if(col == -1) /* Redesenha uma linha */
  {
    iupmatHideFocus(n);
    iupmatDrawCells(n,lin-1,mat_fc(mat),lin-1,mat_lc(mat));
    iupmatShowFocus(n);
  }
  else /* Redesenha apenas a celula */
  {
    /* Se a celula nao existe, retorna NULL */
    if((lin < 0) || (col < 0) || (lin > mat_nl(mat)) || (col > mat_nc(mat)))
      return;

    lin--;  /* A celula superior esquerda e 1:1 para o usuario, */
    col--;  /* Internamente ela e 0:0                           */

    if(col == -1 && lin == -1) /* Canto de titulo */
     iupmatDrawTitleCorner(n);
    else if(col == -1) /* Titulo de uma linha */
     iupmatDrawLineTitle(n,lin,lin);
    else if(lin == -1) /* Titulo de uma coluna */
      iupmatDrawColumnTitle(n,col,col);
    else if(iupmatIsCellVisible(n,lin,col))
    {
      iupmatDrawCells(n, lin, col, lin, col);
      iupmatShowFocus(n);
    }
  }
}
