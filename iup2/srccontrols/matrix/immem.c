/** \file
 * \brief iupmatrix control
 * memory allocation
 *
 * See Copyright Notice in iup.h
 *  */

/**************************************************************************
***************************************************************************
*
*   Funcoes para alocacao de memoria
*
***************************************************************************
***************************************************************************/

#include <stdlib.h>
#include <string.h>

#include <iup.h>
#include <cd.h>

#include "matridef.h"
#include "immem.h"

#define BLOCKSIZE 25


/**************************************************************************
***************************************************************************
*
*   Funcoes exportadas
*
***************************************************************************
***************************************************************************/

/*

%F Funcao chamada quando da criacao da matriz para alocar espaco para as
   celulas. Aloca tambem espaco para o vetor de larguras de coluna, altura
   das linhas e marcacao de linhas e colunas.
   Inicializa todas as celulas com a string "".
%i mat : contexto da matriz,

*/
void iupmatMemAloc(Tmat *mat)
{
  int i;

  /* mat_v(mat) e um ponteiro para um array com mat_nl (numero de linhas
     da matriz) ponteiros. Cada um destes ponteiros aponta para um vetor
     com mat_nc (numero de colunas) celulas.
  */

  if (mat_nl(mat) == 0)
    mat_nla(mat) = 5;
  else
    mat_nla(mat) = mat_nl(mat);

  if (mat_nc(mat) == 0)
    mat_nca(mat) = 5;
  else
    mat_nca(mat) = mat_nc(mat);

  if (!mat->valcb)
  {
    mat_v(mat) = (Tx**)calloc(mat_nla(mat),sizeof(Tx*));
    for(i=0;i<mat_nla(mat);i++)
      mat_v(mat)[i] = (Tx*)calloc(mat_nca(mat),sizeof(Tx));
  }

  mat_w(mat)    = (int *)calloc(mat_nca(mat),sizeof(int));
  mat_h(mat)    = (int *)calloc(mat_nla(mat),sizeof(int));
  mat_colmarked(mat)   = (char *)calloc(mat_nca(mat),sizeof(char));
  mat_linmarked(mat)   = (char *)calloc(mat_nla(mat),sizeof(char));
  mat_colinactive(mat) = (char *)calloc(mat_nca(mat),sizeof(char));
  mat_lininactive(mat) = (char *)calloc(mat_nla(mat),sizeof(char));
}


/*

%F Funcao chamada quando queremos aumentar o numero de linhas da matriz.
   Inicializa as celulas das novas linhas alocadas com a string "".
%i mat    : contexto da matriz,
   nlines : numero de linhas a ser alocado,
   nl     : numero antigo de linhas.

*/
void iupmatMemRealocLines(Tmat *mat,int nlines, int nl)
{
  int i,j;

  /* Se nao tenho linhas suficientes alocadas, aloca mais espaco */
  if (nl+nlines > mat_nla(mat))
  {
    mat_nla(mat) = nl + nlines;

    if (!mat->valcb)
    {
      mat_v(mat) = (Tx**)realloc(mat_v(mat),mat_nla(mat)*sizeof(Tx*));
      for (i=0;i<nlines;i++)
        mat_v(mat)[nl+i] = (Tx*)calloc(mat_nca(mat),sizeof(Tx));
    }

    mat_h(mat) = (int*)realloc(mat_h(mat),mat_nla(mat)*sizeof(int));

    mat_linmarked(mat)   = (char*)realloc(mat_linmarked(mat),mat_nla(mat)*sizeof(char));
    mat_lininactive(mat) = (char*)realloc(mat_lininactive(mat),mat_nla(mat)*sizeof(char));
    for(i=0;i<nlines;i++)
    {
      mat_linmarked(mat)[nl+i] = 0;
      mat_lininactive(mat)[nl+i] = 0;
    }
  }
  else if (!mat->valcb)
  {
    /* Inicializa novas celulas que vao ser usadas com NULL */
    for (i=0;i<nlines;i++)
      for (j=0;j<mat_nca(mat);j++)
      {
	     if (mat_v(mat)[nl+i][j].nba)
          mat_v(mat)[nl+i][j].value[0] = 0;  /* String vazia, nao posso colocar NULL senao perco o ponteiro */
        mat_v(mat)[nl+i][j].mark = 0;
      }
  }
}

/*

%F Funcao chamada quando queremos aumentar o numero de colunas da matriz.
   Para todas as linhas realoca o numero de colunas e inicialza as novas
   celulas com a string vazia. Realoca tambem o vetor que guarda a largura
   das colunas.
%i mat : contexto da matriz,
   ncols : numero de colunas a ser alocado,
   nc : numero antigo de colunas.

*/
void iupmatMemRealocColumns(Tmat *mat, int ncols, int nc)
{
  int i,j;

  /* Se nao tenho colunas suficientes alocadas, aloca mais espaco */
  if (nc+ncols > mat_nca(mat))
  {
    mat_nca(mat) = nc+ncols;
    if (!mat->valcb)
    {
      for (i=0;i<mat_nla(mat);i++)
      {
        mat_v(mat)[i] = (Tx*)realloc(mat_v(mat)[i],mat_nca(mat)*sizeof(Tx));
        for(j=0;j<ncols;j++)
        {
          mat_v(mat)[i][nc+j].nba = 0;
          mat_v(mat)[i][nc+j].value = NULL;
          mat_v(mat)[i][nc+j].mark = 0;
        }
      }
    }
    mat_w(mat) = (int*)realloc(mat_w(mat),mat_nca(mat)*sizeof(int));
    mat_colmarked(mat)   = (char*)realloc(mat_colmarked(mat),mat_nca(mat)*sizeof(char));
    mat_colinactive(mat) = (char*)realloc(mat_colinactive(mat),mat_nca(mat)*sizeof(char));
    for(j=0;j<ncols;j++)
    {
      mat_colmarked(mat)[nc+j] = 0;
      mat_colinactive(mat)[nc+j] = 0;
    }
  }
  else if (!mat->valcb)
  {
    /* Inicializa novas celulas que vao ser usadas com NULL */
    for (i=0;i<mat_nla(mat);i++)
    {
      for (j=0;j<ncols;j++)
      {
   	    if (mat_v(mat)[i][nc+j].nba)
          mat_v(mat)[i][nc+j].value[0] = 0; /* String vazia, nao posso colocar NULL senao perco o ponteiro */
        mat_v(mat)[i][nc+j].mark = 0;
      }
    }
  }
}

/*

%F Aloca espaco para guardar o valor de uma celula. Este espaco e
   alocado em pacotes de BLOCKSIZE bytes.
%i mat : contexto da matriz,
   lin,col : coordenadas da celula para a qual vai ser alocado espaco,
   numc : numero de cararcateres (tamanho) da string que vai ser guardada
          na celula.

*/
void iupmatMemAlocCell(Tmat *mat, int lin, int col, int numc)
{
  if (mat_v(mat)[lin][col].nba <= numc)
  {
    int numbytes = BLOCKSIZE*(numc/BLOCKSIZE + 1);

    if (mat_v(mat)[lin][col].nba > 0)
      free (mat_v(mat)[lin][col].value);
    mat_v(mat)[lin][col].value = (char*)malloc(numbytes+1); /* +1 do '\0'*/
    mat_v(mat)[lin][col].nba   = numbytes;
  }
}

