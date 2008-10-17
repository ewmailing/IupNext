/** \file
 * \brief iupmatrix control
 * auxiliary functions
 *
 * See Copyright Notice in iup.h
 * $Id: imaux.c,v 1.1 2008-10-17 06:20:15 scuri Exp $
 */

/**************************************************************************
***************************************************************************
*
*   Funcoes auxiliares
*
***************************************************************************
***************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <iup.h>
#include <iupcpi.h>
#include <cd.h>
#include "istrutil.h"

#include "imdraw.h"
#include "matridef.h"
#include "imaux.h"
#include "iupmatrix.h"
#include "immem.h"
#include "imfocus.h"
#include "imedit.h"
#include "matrixcd.h"

/**************************************************************************
***************************************************************************
*
*   Funcoes exportadas
*
***************************************************************************
***************************************************************************/

/*
%F Verifica se uma celula esta visivel ou nao.

%i h - Handle da matriz.
   lin,col - Coordenadas da celula.
%o Retorna 1 se a celula estiver visivel, 0 caso contrario.
*/
int iupmatIsCellVisible(Ihandle *h, int lin, int col)
{
  Tmat *mat=(Tmat*)matrix_data(h);

  /* Se nao ha linhas ou colunas, a celula nao e' visivel */
  if ((mat_nc(mat) == 0)||(mat_nl(mat) == 0)) return 0;

  if (((lin >= mat_fl(mat)) &&
       (lin <= mat_ll(mat)) &&
       (col >= mat_fc(mat)) &&
       (col <= mat_lc(mat))))
  {
    return 1;
  }

  return 0;
}

/*
%F Calcula as coordenadas dos cantos inferior esquerdo e superior
   direito de uma celula.

%i h - Handle da matriz
   lin,col - Coordenadas da celula.
%o x,y     - Coordenadas do canto inferior esquerdo
   dx,dy   - Largura e altura da celula.
   Retorna 0 se a celula nao estiver visivel, 1 caso contrario.
*/
int iupmatGetCellDim(Ihandle *h, int lin, int col, int *x, int *y,
                        int *dx, int *dy)
{
  Tmat *mat=(Tmat*)matrix_data(h);
  int i;

  if(!iupmatIsCellVisible(h,lin,col))
    return 0;

  /* acha a posicao do inicio da coluna */
  *x=mat_wt(mat);
  for(i=mat_fc(mat);i<col;i++) *x+=mat_w(mat)[i];

  /* acha o tamanho da coluna */
  *dx = mat_w(mat)[col] - 1;

  /* acha a posicao do inicio da linha */
  *y=mat_ht(mat);
  for(i=mat_fl(mat);i<lin;i++) *y+=mat_h(mat)[i];

  /* acha o tamanho da linha */
  *dy = mat_h(mat)[lin] - 1;

  return 1;
}

/*

%F Calcula o tamanho, em pixels das colunas/linhas invisiveis, a
   esquerda/acima da primeira coluna/linha.
%i h : handle da matriz.
   m : Escolhe se vai operar sobre linhas ou colunas. [MAT_LIN|MAT_COL]

*/
void iupmatGetPos(Ihandle *h, int m)
{
  Tmat *mat=(Tmat*)matrix_data(h);
  int i;
  Tlincol *p;

  if (m == MAT_LIN)
    p = &(mat->lin);
  else
    p = &(mat->col);

  p->pos=0;
  for (i=0;i<p->first;i++)
    p->pos+=p->wh[i];
}


/*

%F Calcula qual a ultima coluna/linha visivel na matriz  e
   tambem a sua largura/altura.
%i h : handle da matriz
   m : Escolhe se vai operar sobre linhas ou colunas. [MAT_LIN|MAT_COL]

*/
void iupmatGetLastWidth(Ihandle *h, int m)
{
  Tmat *mat=(Tmat*)matrix_data(h);
  int i,soma=0,osoma;
  Tlincol *p;

  if (m == MAT_LIN)
    p = &(mat->lin);
  else
    p = &(mat->col);

  if (p->num == 0)
    return;

  if (soma < p->size)
  {
    /* Procura qual a ultima coluna/linha. Comeca na primeira visivel e vai
       somando as larguras ate atingir o tamanho da matriz
    */
    for(i=p->first,osoma=0;i<p->num;i++)
    {
      osoma  = soma;
      soma  += p->wh[i];
      if (soma >= p->size) break;
    }

    if (i == p->num)
    {
      p->last   = i-1;
      p->lastwh = p->wh[i-1];

      /****/
      /* Acerta a primeira linha/coluna para nao ficarem espacos vazios */
      for(soma=0,i=p->num-1;i>=0;i--)
      {
        soma+=p->wh[i];
        if (soma > p->size)
          break;
      }
      p->first = i+1;
      if (i >= 0)
        soma-=p->wh[i];
      p->pos = p->totalwh-soma;
      /****/

    }
    else
    {
      p->last   = i;
      p->lastwh = p->size-osoma;
    }
  }
  else
  {
    /* Se nao ha espaco para nenhuma coluna, seta a ultima coluna como 0 */
    p->last   = 0;
    p->lastwh = 0;
  }
}


/*

%F Retorna o tamanho da coluna de titulos de linha. Verifica se tem um valor
   setado para WIDTH0. Se tiver, retorna o valor, ja com seu tamanho
   convertido para pixels. Caso contrario retorna o tamanho em pixels do
   maior titulo de linha.
%i h : handle da matriz.

*/
int iupmatGetTitlelineSize(Ihandle *h)
{
  Tmat *mat=(Tmat*)matrix_data(h);
  int i,wt=0;
  char *tline,*larg;
  char aux[100];

  larg = iupGetEnvRaw(h,"WIDTH0");
  if (larg)
  {
    /* Usuario definiu um tamanho para os titulos */
    int charwidth,charheight;

    iupdrvGetCharSize(h,&charwidth,&charheight);
    sscanf(larg,"%d",&wt);
    wt = (int)((wt/4.)*charwidth)+DECOR_X;
  }
  else 
  {
    larg = iupGetEnvRaw(h,"RASTERWIDTH0");
    if (larg)
    {
      sscanf(larg,"%d",&wt);
      wt+=DECOR_X;
    }
    else if (!mat->valcb)
    {
      /* Se valcb estiver definido, nao procura nos atributos, pois quem
      vai ter de fornecer o valor a ser desenhado e' o usuario...

      Procura pelo titulo com maior tamanho.
      */
      wt = 0;
      for(i=0;i<mat_nl(mat);i++)
      {
        sprintf(aux,TITLELIN,i+1);
        tline = iupGetEnvRaw(h,aux);
        if (tline)
        {
          int sizet;
          iupdrvStringSize(h,tline,&sizet,NULL);
          if(sizet > wt)
            wt = sizet;
        }
      }

      if (wt > 0)   /* se tem coluna de titulo, adiciona decoracao */
        wt+=DECOR_X;
    }
  }
  return wt;
}


/*

%F Retorna a altura da linha de titulos de coluna. Verifica se tem um valor
   setado para HEIGHT0. Se tiver, retorna o valor, ja com seu tamanho
   convertido para pixels, caso contrario  retorna a maior altura de um titulo
   de coluna.
%i h : handle da matriz.

*/
int iupmatGetTitlecolumnSize(Ihandle *h)
{
  Tmat *mat=(Tmat*)matrix_data(h);
  char *alt;
  int ht=0;

  alt = iupGetEnvRaw(h,"HEIGHT0");
  if(alt)
  {
    int charwidth,charheight;

    /* Usuario definiu um tamanho para os titulos */
    iupdrvGetCharSize(h,&charwidth,&charheight);
    sscanf(alt,"%d",&ht);
    return (int)((ht/8.)*charheight) + DECOR_Y;
  }
  else
  {
    alt = iupGetEnvRaw(h,"RASTERHEIGHT0");
    if(alt)
    {
      sscanf(alt,"%d",&ht);
      return ht + DECOR_Y;
    }
    else if (!mat->valcb)
    {
      int i;
      char aux[100];
      int max = 0;

      for(i=-1;i<mat_nc(mat);i++)  /* Comeca de -1 para verificar celula 0:0 */
      {
        sprintf(aux,TITLECOL,i+1);
        alt = iupGetEnvRaw(h,aux);
        if(alt)
        {
          iupmatTextHeight(h,alt,&ht,NULL,NULL);
          if (ht > max) max = ht;
        }
      }
      if(max)
        return max + DECOR_Y;
    }
  }
  return 0;
}


/*
%F Descobre a largura de uma coluna.

%i h   : Handle da matriz.
   col : Numero da coluna (0 = primeira coluna)
%o Retorna a largura da coluna col.
*/
int iupmatGetColumnWidth(Ihandle *h, int col)
{
  int w;
  char aux[100],*larg;
  int charwidth, charheight, pixels=0;

  iupdrvGetCharSize(h,&charwidth,&charheight);

  sprintf(aux, "WIDTH%d",col+1);        /* Tem largura propria definida ?    */
  larg = iupGetEnvRaw(h,aux);
  if (!larg)
  {
    sprintf(aux, "RASTERWIDTH%d",col+1);
    larg = iupGetEnvRaw(h,aux);
    if (larg)
      pixels = 1;
  }

  if (!larg)
    larg = IupGetAttribute(h,IUP_WIDTHDEF); /* Tenta atributo de largura default */

  sscanf(larg,"%d",&w);

  if (w == 0)
    return 0;
  else if (w < 0)
    return charwidth*10+DECOR_X;
  else
  {
    if (pixels)
      return w+DECOR_X;
    else
      return (int)(((w/4.)*charwidth)+DECOR_X);
  }
}

/*
%F Descobre a altura de uma linha em pixels.
   Se tem o atributo HEIGHT definido, retorna este valor, se nao tiver e
   o texto do titulo nao existir ou contiver apenas 1 linha, retorna o valor
   de HEIGHTDEF, senao retorna o tamanho ocupado pelo titulo.

%i h   : Handle da matriz.
   lin : Numero da linha (0 = primeira linha)
%o Retorna a altura da linha lin.
*/
int iupmatGetLineHeight(Ihandle *h, int lin)
{
  int a, numl;
  char aux[100],*alt;
  int charwidth, charheight, pixels = 0;

  iupdrvGetCharSize(h,&charwidth,&charheight);

  sprintf(aux, "HEIGHT%d",lin+1);        /* Tem altura propria definida ?    */
  alt = iupGetEnvRaw(h,aux);
  if (!alt)
  {
    sprintf(aux, "RASTERHEIGHT%d",lin+1);
    alt = iupGetEnvRaw(h,aux);
    if (alt)
      pixels = 1;
  }

  if (!alt)
  {
    sprintf(aux,TITLELIN,lin+1);
    numl = iupmatTextHeight(h,iupGetEnvRaw(h,aux),&a,NULL,NULL);
    if (numl > 1)
      return a + DECOR_Y;

    alt  = IupGetAttribute(h,IUP_HEIGHTDEF); /* Tenta atributo de altura default */
  }
  sscanf(alt,"%d",&a);

  if (a == 0)
    return 0;
  else if (a < 0)
    return charheight+DECOR_Y;
  else
  {
    if (pixels)
      return a + DECOR_Y;
    else
      return (int)((a/8.)*charheight) + DECOR_Y;
  }
}

/*
%F Preenche o vetor mat_w com a largura de todas as colunas.
   Calcula o valor de mat_totw

%i h : Handle da matriz.
*/
void iupmatFillWidthVec(Ihandle *h)
{
  Tmat *mat=(Tmat*)matrix_data(h);
  int i;

  /* calcula largura total da matrix e a largura de cada coluna*/
  mat_totw(mat)=0;
  for(i=0;i<mat_nc(mat);i++)
  {
    mat_w(mat)[i] = iupmatGetColumnWidth(h,i);
    if (mat_w(mat)[i] > mat_sx(mat))
      mat_w(mat)[i] = mat_sx(mat);
    mat_totw(mat)+=mat_w(mat)[i];
  }
}

/*
%F Preenche o vetor mat_h com a altura de todas as linhas.
   Calcula o valor de mat_toth

%i h : Handle da matriz.
*/
void iupmatFillHeightVec(Ihandle *h)
{
  Tmat *mat=(Tmat*)matrix_data(h);
  int i;
  /* calcula altura total da matrix e a altura de cada linha*/
  mat_toth(mat)=0;
  for(i=0;i<mat_nl(mat);i++)
  {
    mat_h(mat)[i] = iupmatGetLineHeight(h,i);
    if (mat_h(mat)[i] > mat_sy(mat))
      mat_h(mat)[i] = mat_sy(mat);
    mat_toth(mat)+=mat_h(mat)[i];
  }
}


/*

%F Dada uma posicao x,y retorna a celula correspondente, em lin e col,
   sendo que faz lin = -1 quando for um titulo de linha e col = -1
   quando for um titulo de coluna, se estiver sobre a scrollbar vertical, faz .
   col = -2, e se estiver sobre a scrollbar horizontal, faz lin = -2.
   Faz col = -2 quando a matriz nao tiver colunas e faz lin = -2 se nao
   tiver linhas.
   Se a coordenada nao estiver dentro da matriz, retorna 0.
%i h : handle da matriz,
   x,y : coordenadas do cvanvas.
%o l,c : linha e coluna da celula apontada,
   Retorna 1 se for uma celula valida, dentro da matriz, caso contrario
   retorna 0.

*/
int iupmatGetLineCol(Ihandle *h, int x, int y, int *l, int *c)
{
  Tmat *mat=(Tmat*)matrix_data(h);
  int width,lin,col;

  /* pega a coluna apontada */
  width=mat_wt(mat);
  if (x < width)
    col = -1;  /* Esta na coluna de titulos */
  else
  {
    if (mat_nc(mat) == 0)
      col = -2;
    else
    {
      for (col=mat_fc(mat);col<=mat_lc(mat);col++)
      {
        width+= (col==mat_lc(mat)?mat_wlc(mat):mat_w(mat)[col]);
        if (width >= x)
          break;
      }
      if (col > mat_lc(mat)) /* Esta na area vazia apos a ultima coluna  ou sobre a scrollbar*/
        col = -2;
    }
  }

  width=mat_ht(mat);
  if (y < width)
    lin = -1;  /* Esta na linha de titulos */
  else
  {
    if (mat_nl(mat) == 0)
      lin = -2;
    else
    {
      for (lin=mat_fl(mat);lin<=mat_ll(mat);lin++)
      {
        width+=(lin==mat_ll(mat)?mat_hll(mat):mat_h(mat)[lin]);
        if (width >= y)
          break;
      }
      if (lin > mat_ll(mat))  /* Esta na area sobre a scrollbar*/
        lin = -2;
    }
  }


  if (col == -2 && lin == -2 ) /* esta no limbo entre as duas scrollbars... */
    return 0;

#if _OLD_CODE_
  if (col == -1 && lin == -1 ) /* esta no canto superior esquerdo */
    return 0;
#endif

  *l = lin;
  *c = col;
  return 1;
}

/*
%F Retorna o valor de uma celula.  Pode pegar este valor da estrutura interna
   da matriz ou atraves de uma consulta a uma callback do usuario.

%i h : Handle da matriz
   lin, col : Coordenadas da celula que tera seu valor retornado. (0,0) e' a
              coordenada da celula no canto superior esquerdo da matriz. O
              valor -1 indica um titulo de linha ou coluna.
%o Retorna o valor retornado pela callback do usuario ou o valor na estrutura
   da matriz.
*/
char* iupmatGetCellValue(Ihandle *h, int lin, int col)
{
  Tmat *mat=(Tmat*)matrix_data(h);
  char aux[100];

  if (mat->valcb)
  {
    return mat->valcb(h,lin+1,col+1);
  }
  else
  {
    if (lin == -1 && col == -1)
    {
      return iupGetEnvRaw(h,"0:0");
    }
    else if (lin == -1)  /* Titulo de coluna */
    {
      sprintf(aux,TITLECOL,col+1);
      return iupGetEnvRaw(h,aux);
    }
    else if (col == -1)  /* Titulo de linha  */
    {
      sprintf(aux,TITLELIN,lin+1);
      return iupGetEnvRaw(h,aux);
    }
    else
    {
      return mat_v(mat)[lin][col].value;
    }
  }
}


/*
%F Pega o valor sendo editado e coloca na estrutura.

%i h : handle da matriz.
*/
void iupmatUpdateCellValue(Ihandle *h)
{
  Tmat *mat = (Tmat*)matrix_data(h);
  char *valor;
  int  tam=0;

  valor = iupmatEditGetValue(mat);

  if (mat->valeditcb)
  {
    mat->valeditcb(h,mat_lin(mat)+1,mat_col(mat)+1,valor);
    return;
  }
  else if (mat->valcb) /* do nothing when in callback mode if valeditcb not defined */
    return;

  if (valor)
    tam = strlen(valor);

  if (tam != 0)
  {
    iupmatMemAlocCell(mat,mat_lin(mat),mat_col(mat),tam);
    strcpy(mat_v(mat)[mat_lin(mat)][mat_col(mat)].value,valor);
  }
  else if (mat_v(mat)[mat_lin(mat)][mat_col(mat)].value != NULL)
    mat_v(mat)[mat_lin(mat)][mat_col(mat)].value[0]='\0';
}

/*
%F Chama a callback do usuario associada ao evento de uma celula perder o
   foco.
%i h : handle da matriz.
%o Retorna o que foi retornado pala callback do usuario.

*/
int iupmatCallLeavecellCb(Ihandle *h)
{
  IFnii cb = (IFnii)IupGetCallback(h,IUP_LEAVEITEM_CB);
  if (cb)
  {
   Tmat *mat=(Tmat*)matrix_data(h);
   int ret = cb(h,mat_lin(mat)+1,mat_col(mat)+1);

   return ret;
  }

  return IUP_DEFAULT;
}

/*

%F Chama a callback do usuario associada ao evento de uma celula ganhar o
   foco.
%i h : handle da matriz.

*/
void iupmatCallEntercellCb(Ihandle *h)
{
  IFnii cb = (IFnii)IupGetCallback(h,IUP_ENTERITEM_CB);
  if (cb)
  {
    Tmat *mat=(Tmat*)matrix_data(h);
    if ((mat_nc(mat) == 0)||(mat_nl(mat) == 0)) return;
    cb(h,mat_lin(mat)+1,mat_col(mat)+1);
  }
}

int iupmatCallEditionCbLinCol(Ihandle *h, int lin, int col, int modo)
{
  int rc = IUP_DEFAULT;
  IFniii cb = (IFniii)IupGetCallback(h,IUP_EDITION_CB);
  if (cb)
  {
    rc = cb(h,lin,col, modo);
  }

  return rc;
}

/*
%F Faz o parse de um texto, retornando o numero de linhas ocupadas pelo mesmo
   (cada linha termina por um \n) e a altura ocupada por este texto.

%i h    : Handle da matriz
   text : Texto em questao
%o totalh  : Numero de pixels a serem necessarios para desenhar todas as linhas.
   lineh   : Numero de pixels necessarios para desenhar uma linha.
   spacing : Numero de pixels para o spacamento entre as linhas
   Retorna o numero de linhas existentes no texto;
*/
int iupmatTextHeight(Ihandle *h, char *text, int *totalh, int *lineh, int *spacing)
{
  int numlines = 1;
  char *c;
  int charwidth,charheight;

  if (spacing) *spacing = 0;
  if (totalh ) *totalh  = 0;
  if (lineh  ) *lineh   = 0;

  if (!text || !*text)
    return 0;

  c = text;
  while(c && *c)
  {
    c = strchr(c,'\n');
    if(c)
    {
      numlines++;
      c++;
    }
  }

  iupdrvGetCharSize(h,&charwidth,&charheight);

  if (spacing) *spacing = DECOR_Y/2;
  if (lineh  ) *lineh   = charheight;
  if (totalh ) *totalh  = charheight * numlines + (DECOR_Y/2) * (numlines-1);

  return numlines;
}

/*
%F Faz o parse de um texto, (cada linha termina por um \n) calculando a largura
   ocupada por este texto.

%i h     : Handle da matriz
   text  : Texto em questao
%o width : Numero de pixels necessarios para desenhar completamente a maior
           linha de texto
*/
void iupmatTextWidth(Ihandle *h, char *text, int *width)
{
  Tmat *mat=(Tmat*)matrix_data(h);
  char *p, *q, *newtext;
  int w,maxwidth=0;

  if(!text || !*text) return;

  newtext = q = (char*)iupStrDup(text);

  while(*q)
  {
    p = strchr(q,'\n');
    if(p) *p = 0;

    iupdrvStringSize(mat->self,q,&w,NULL);
    if (w > maxwidth) maxwidth = w;

    if(p)
    {
      *p = '\n';
      q = p+1;
    }
    else
      break;
  }

  *width = maxwidth;
  
  if(newtext)
    free(newtext);
}
