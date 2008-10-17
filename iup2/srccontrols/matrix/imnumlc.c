/** \file
 * \brief iupmatrix control
 * change number of collumns or lines
 *
 * See Copyright Notice in iup.h
 * $Id: imnumlc.c,v 1.1 2008-10-17 06:20:15 scuri Exp $
 */

/**************************************************************************
***************************************************************************
*
*   Funcoes para a mudanca do numero de linhas e colunas da matriz, apos ela
*  ter sido criada.
*
***************************************************************************
***************************************************************************/

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
#include "imaux.h"
#include "immem.h"
#include "imgetset.h"
#include "immark.h"
#include "imnumlc.h"
#include "imedit.h"


#define INSERT    0     /* Adicionando uma linha ou coluna */
#define REMOVE    1     /* Removendo uma linha ou coluna   */

/**************************************************************************
***************************************************************************
*
*   Funcoes internas
*
***************************************************************************
***************************************************************************/

/*

%F Muda o numero de linhas da matriz, faz a alocacao de memoria se
   necessario. Se necessario muda o numero de linhas visiveis para
   refletir o novo tamanho.
%i h : handle da matriz,
   num : novo numero de linhas da matriz.

*/
static void ChangeNumLines(Ihandle *h, int num)
{
  Tmat *mat=(Tmat*)matrix_data(h);
  int  oldnl  = mat_nl(mat);
  int  wt, i;

  mat_nl(mat) = num;

  /* a linha com o maior titulo pode ter sido apagada.... */
  wt  = iupmatGetTitlelineSize(h);
  if (wt != mat_wt(mat))
  {
    mat_sx(mat) = (XmaxCanvas(mat)+1) - wt;
    mat_wt(mat) = wt;
    iupmatGetLastWidth(h,MAT_COL);
  }

  if (num > oldnl) /* aumentando a matriz */
  {
    iupmatMemRealocLines(mat,num-oldnl,oldnl);

    /* recalcula altura total da matrix */
    for(i=oldnl;i<mat_nl(mat);i++)
    {
      mat_h(mat)[i] = iupmatGetLineHeight(h,i);
      if (mat_h(mat)[i] > mat_sy(mat))
        mat_h(mat)[i] = mat_sy(mat);
      mat_toth(mat)+=mat_h(mat)[i];
    }
  }
  else /* diminuindo a matriz */
  {
    /* se nao ha nenhuma linha sendo vista, muda a primeira linha */
    if (mat_fl(mat) > mat_ultimal(mat))
    {
      mat_fl(mat) = 0;
      if (mat_lin(mat) > mat_ultimal(mat))
        mat_lin(mat) = 0;
    }
    /* recalcula largura total da matrix */
    for(i=oldnl-1;i>=mat_nl(mat);i--)
    {
      mat_toth(mat)-=mat_h(mat)[i];
    }
  }
  iupmatGetLastWidth(h,MAT_LIN);
}


/*

%F Muda o numero de colunas da matriz, faz a alocacao de memoria se
necessario. Se necessario muda o numero de colunas visiveis para
refletir o novo tamanho.
%i h : handle da matriz,
num : novo numero de colunas da matriz.

*/
static void ChangeNumCols(Ihandle *h, int num)
{
  Tmat *mat=(Tmat*)matrix_data(h);
  int oldnc = mat_nc(mat),i;

  mat_nc(mat) = num;

  if (num > oldnc)
  {
    /* aumentando a matriz */
    iupmatMemRealocColumns(mat,num-oldnc,oldnc);

    /* recalcula largura total da matrix */
    for(i=oldnc;i<mat_nc(mat);i++)
    {
      mat_w(mat)[i] = iupmatGetColumnWidth(h,i);
      if (mat_w(mat)[i] > mat_sx(mat))
        mat_w(mat)[i] = mat_sx(mat);
      mat_totw(mat)+=mat_w(mat)[i];
    }
  }
  else
  {
    /* diminuindo a matriz */

    /* se nao ha nenhuma coluna sendo vista, muda a primeira coluna */
    if (mat_fc(mat) > mat_ultimac(mat))
    {
      mat_fc(mat) = 0;
      if (mat_col(mat) > mat_ultimac(mat))
        mat_col(mat) = 0;
    }

    /* recalcula largura total da matrix */
    for(i=oldnc-1;i>=mat_nc(mat);i--)
    {
      mat_totw(mat)-=mat_w(mat)[i];
    }
  }
  iupmatGetLastWidth(h,MAT_COL);
}

/*

%F Quando a matriz tem o seu numero de linhas aumentado, e estas linhas sao
inseridas no final da mesma, tenho que olhar se ha novos atributos que
devem ser colocados nas celulas.
%i h - Handle da matriz
oldnumlin - Numero de linhas que havia na matriz.
*/
static void GetNewLineAttributes(Ihandle *h,int oldnumlin)
{
  Tmat *mat = (Tmat*)matrix_data(h);
  int i,j;
  char *valor, attr[100];

  if (mat->valcb)
    return;

  for (i=oldnumlin;i<mat_nl(mat);i++)
  {
    for (j=0;j<mat_nc(mat);j++)
    {
      sprintf(attr,"%d:%d",i+1,j+1);
      valor=(char *)iupGetEnvRaw(h,attr);
      if (valor && *valor)
      {
        iupmatMemAlocCell(mat,i,j,strlen(valor));
        strcpy(mat_v(mat)[i][j].value,valor);
      }
    }
  }
}


/*

%F Quando a matriz tem o seu numero de colunas aumentado, e estas colunas sao
inseridas no final da mesma, tenho que olhar se ha novos atributos que
devem ser colocados nas celulas.
%i h - Handle da matriz
oldnumcol - Numero de colunas que havia na matriz.
*/
static void GetNewColumnAttributes(Ihandle *h,int oldnumcol)
{
  Tmat *mat = (Tmat*)matrix_data(h);
  int i,j;
  char *valor, attr[100];

  if (mat->valcb)
    return;

  for (i=0;i<mat_nl(mat);i++)
  {
    for (j=oldnumcol;j<mat_nc(mat);j++) /* Shouldn't this be <= ? */  
    {
      sprintf(attr,"%d:%d",i+1,j+1);
      valor=(char *)iupGetEnvRaw(h,attr);
      if (valor && *valor)
      {
        iupmatMemAlocCell(mat,i,j,strlen(valor));
        strcpy(mat_v(mat)[i][j].value,valor);
      }
    }
  }
}


/*

%F Muda os atributos de titulo e tambem os valores das celulas, de cada linha,
para refletir insercoes e delecoes de linhas no meio da matriz.
Precisa mudar linhas maiores do que a linha base.
Note que quando esta funcao e chamada, mat_numlin(mat) ja reflete o novo
numero de linhas da matriz. Espaco ja foi alocado para as novas linhas.
%i h : handle da matriz;
base : Se for uma insercao, aponta para a linha apos a qual as insercoes
vao ser feitas. Note que seu valor e 1 quando vamos inserir apos a
primeira linha.
Se for uma delecao, aponta para a primeira linha que vai ser
apagada. Note que seu valor e 1 quando vamos apagar a linha 1;
numlin : Numero de linhas a ser inserido/apagado;
modo : INSERT ou REMOVE.

*/
static void ChangeLineAttributes(Ihandle *h,int base,int numlin, int modo)
{
  Tmat *mat = (Tmat*)matrix_data(h);
  int i;
  char aux[10],*v;
  Tx **tmp = NULL;

  /* Aloca espaco para guardar, temporariamente, ponteiros para estruturas
  do tipo Tx, alocadas em ChangeNumLin. Isto e necessario para nao perder
  o espaco alocado, quando da movimentacao dos atributos.
  */
  if (!mat->valcb)
    tmp = (Tx**) malloc(numlin*sizeof(Tx*));

  if (modo == INSERT)
  {
    /* Inserindo linhas */
    if (!mat->valcb)
    {
      for (i=0;i<numlin;i++)
        tmp[i] = mat_v(mat)[mat_nl(mat)-numlin+i]; /* guarda regiao alocada para atribuir as novas linhas */
    }

    /* Movendo atributos das linhas para baixo */
    for (i=mat_nl(mat)-1-numlin;i>=base;i--)
    {
      sprintf(aux,TITLELIN,i+1);        /* Pega o titulo da linha corrente e coloca no lugar correto */
      v = iupGetEnvRaw(h,aux);             /* O atributo e i+1:0 pois os atributos comecam em 1 e       */
      sprintf(aux,TITLELIN,i+1+numlin); /* na estrutura interna comecam em zero.                     */
      iupStoreEnv(h,aux,v);

      if (!mat->valcb)
        mat_v(mat)[i+numlin] = mat_v(mat)[i];

      /* move a indicacao de se a linha esta ou nao marcada/inativa */
      mat_linmarked(mat)[i+numlin]   = mat_linmarked(mat)[i];
      mat_lininactive(mat)[i+numlin] = mat_lininactive(mat)[i];
    }

    for(i=0;i<numlin;i++)
    {
      /* Seta os titulos das novas linhas como "" */
      sprintf(aux,TITLELIN,base+1+i);
      iupSetEnv(h,aux,"");

      /* Restaura regiao alocada na posicao correta */
      if (!mat->valcb)
        mat_v(mat)[base+i]=tmp[i];

      /* Novas linhas nao estao marcadas */
      mat_linmarked(mat)[base+i]   = 0;
      mat_lininactive(mat)[base+i] = 0;
    }

    /* Ajuste da cor de frente e de fundo das linhas/celulas */
    for (i=0;i<6;i++)
    {
      char string[6][100] = { "BGCOLOR%d:*",
                             "FGCOLOR%d:*",
                             "FONT%d:*",
                             "BGCOLOR%d:%d",
                             "FGCOLOR%d:%d",
                             "FONT%d:%d"};
      char attr[100], *value;
      int j,k;

      /* Atualiza atributos das linhas/celulas apos a ultima linha adicionada */
      for(j=mat_nl(mat);j>base+numlin;j--)
      {
        /* Atualiza os atributos das linhas */      
        if(i<3)
        {
          sprintf(attr,string[i],j-numlin);
          value = iupGetEnvRaw(h,attr);
          sprintf(attr,string[i],j);
          iupStoreEnv(h, attr, value);
        }

        /* Atualiza os atributos de cada celula */
        else for(k=0;k<=mat_nc(mat);k++)
        {
          sprintf(attr,string[i],j-numlin,k);
          value = iupGetEnvRaw(h,attr);
          sprintf(attr,string[i],j,k);
          iupStoreEnv(h, attr, value);
        }
      }

      /* Os atributos das linhas adicionadas os default */
      for(j=base;j<base+numlin;j++)
      {
        if(i<2)
        {
          sprintf(attr,string[i],j+1);
          iupSetEnv(h,attr,NULL);
        }
        else for(k=0;k<=mat_nc(mat);k++)
        {
          sprintf(attr,string[i],j+1,k);
          iupSetEnv(h,attr,NULL);
        }
      }
    }

    if (mat_lin(mat) >= base) /* Se o foco estiver depois da linha em que a nova linha foi inserida soma numlin a linha do foco */
      mat_lin(mat)+=numlin;
  }
  else /* modo == REMOVE */
  {
    if (!mat->valcb)
    {
      for (i=0;i<numlin;i++)
        tmp[i] = mat_v(mat)[base-1+i]; /* guarda regiao alocada para atribuir as linhas no final , nao usadas*/
    }

    for (i=base-1;i<mat_nl(mat);i++)
    {
      sprintf(aux,TITLELIN,i+1+numlin);  /* Pega o titulo da proxima linha */
      v = iupGetEnvRaw(h,aux);
      sprintf(aux,TITLELIN,i+1);         /* e coloca na linha atual */
      iupStoreEnv(h,aux,v);

      if (!mat->valcb)
        mat_v(mat)[i] = mat_v(mat)[i+numlin];

      /* move a indicacao de se a linha esta ou nao marcada/inativa */
      mat_linmarked(mat)[i]   = mat_linmarked(mat)[i+numlin];
      mat_lininactive(mat)[i] = mat_lininactive(mat)[i+numlin];
    }

    for(i=0;i<numlin;i++)
    {
      /* Restaura regiao alocada na posicao correta */
      if (!mat->valcb)
        mat_v(mat)[mat_nl(mat)+i]=tmp[i];

      /* Marca as linhas apagadas como nao marcadas */
      mat_linmarked(mat)[mat_nl(mat)+i]=0;
      mat_lininactive(mat)[mat_nl(mat)+i]=0;
    }

    /* Ajuste da cor de frente e de fundo das linhas/celulas */
    for(i=0;i<6;i++)
    {
      char string[6][100] = { "BGCOLOR%d:*",
                             "FGCOLOR%d:*",
                             "FONT%d:*",
                             "BGCOLOR%d:%d",
                             "FGCOLOR%d:%d",
                             "FONT%d:%d"} ;
      char attr[100], *value;
      int j,k;

      /* Atualiza atributos das linhas/celulas apos a ultima linha apagada */
      for(j=base-1;j<mat_nl(mat);j++)
      {
        /* Atualiza os atributos das linhas */      
        if(i<3)      
        {
          sprintf(attr,string[i],j+1+numlin);
          value = iupGetEnvRaw(h,attr);
          sprintf(attr,string[i],j+1);
          iupStoreEnv(h, attr, value);
        }

        /* Atualiza os atributos de cada celula */
        else for(k=0;k<=mat_nc(mat);k++) 
        {
          sprintf(attr,string[i],j+1+numlin,k);
          value = iupGetEnvRaw(h,attr);
          sprintf(attr,string[i],j+1,k);
          iupStoreEnv(h, attr, value);
        }
      }
    }

    if (mat_lin(mat) >= base) /* Se o foco estiver depois da linha  que foi retirada, diminue numlin da linha do foco */
      mat_lin(mat)-=numlin;
  }

  {
    /* Arruma a largura da coluna de titulos, e tambem o tamanho visivel da matriz */
    int wt  = iupmatGetTitlelineSize(h);
    if (wt != mat_wt(mat))
    {
      mat_sx(mat) = (XmaxCanvas(mat)+1) - wt;
      mat_wt(mat) = wt;
      iupmatGetLastWidth(h,MAT_COL);
    }
  }

  if (tmp) free(tmp);
}


/*

%F Muda os atributos de titulo e tambem os valores das celulas, de cada coluna,
para refletir insercoes e delecoes de colunas no meio da matriz.
Precisa mudar colunas maiores do que a coluna base.
Note que quando esta funcao e chamada, mat_numcol(mat) ja reflete o novo
numero de colunas da matriz. Espaco ja foi alocado para as novas colunas.
%i h : handle da matriz,
base : Se for uma insercao, aponta para a coluna apos a qual as insercoes
vao ser feitas. Note que seu valor e 1 quando vamos inserir apos a
primeira coluna.
Se for uma delecao, aponta para a primeira coluna que vai ser
apagada. Note que seu valor e 1 quando vamos apagar a coluna 1.
numcol : Numero de colunas a ser inserido/apagado.
modo : INSERT ou REMOVE

*/
static void ChangeColumnAttributes(Ihandle *h,int base, int numcol, int modo)
{
  Tmat *mat = (Tmat*)matrix_data(h);
  int i,j,k;
  char aux[10],*v;
  Tx  *tmp = NULL;
  int *newwidth;

  /* Aloca espaco para guardar, temporariamente, ponteiros para estruturas
  do tipo Tx, alocadas em ChangeNumCol. Isto e necessario para nao perder
  o espaco alocado, quando da movimentacao dos atributos.
  */
  if (!mat->valcb)
    tmp = (Tx*) malloc(numcol*sizeof(Tx));
  newwidth = (int*) malloc(numcol*sizeof(int));

  if (modo == INSERT)
  {
    for(i=0;i<numcol;i++)
      newwidth[i] = mat_w(mat)[mat_nc(mat)-numcol+i]; /* guarda largura calculada anteriormente */

    /* Move titulos das colunas e larguras */
    for (i=mat_nc(mat)-1-numcol; i>=base; i--)
    {
      sprintf(aux,TITLECOL,i+1);        /* Pega o titulo da coluna corrente e coloca no lugar correto*/
      v = iupGetEnvRaw(h,aux);             /* O atributo e 0:i+1 pois os atributos comecam em 1 e       */
      sprintf(aux,TITLECOL,i+1+numcol); /* na estrutura interna comecam em zero.                     */
      iupStoreEnv(h,aux,v);

      mat_w(mat)[i+numcol] = mat_w(mat)[i];   /* Rearruma larguras das colunas */

      /* move a indicacao de se a coluna esta ou nao marcada/inativa */
      mat_colmarked(mat)[i+numcol]   = mat_colmarked(mat)[i];
      mat_colinactive(mat)[i+numcol] = mat_colinactive(mat)[i];
    }

    /* Restaura larguras guardadas e seta novos titulos*/
    for(i=0;i<numcol;i++)
    {
      sprintf(aux,TITLECOL,base+1+i);
      iupSetEnv(h,aux,"");

      mat_w(mat)[base+i]=newwidth[i];

      /* Novas colunas nao estao marcadas/inativas */
      mat_colmarked(mat)[base+i] = 0;
      mat_colinactive(mat)[base+i] = 0;
    }

    /* Rearruma valores das celulas */
    if (!mat->valcb)
    {
      for (i=0;i<mat_nl(mat);i++)
      {
        for(j=0;j<numcol;j++)
          tmp[j] = mat_v(mat)[i][mat_nc(mat)-numcol+j];  /* Guarda regiao alocada, para atribuir a nova coluna */

        for(j=mat_nc(mat)-1-numcol; j>=base; j--)       /* Move valores antigos */
        {
          mat_v(mat)[i][j+numcol]=mat_v(mat)[i][j];
        }
        for(j=0;j<numcol;j++)                           /* Recoloca regiao guardada */
          mat_v(mat)[i][base+j]=tmp[j];
      }
    }

    /* Ajuste do alinhamento e da cor de frente e de fundo das colunas */
    for (i=0;i<7;i++)
    {
      char string[7][100] = { "ALIGNMENT%d",
                             "BGCOLOR*:%d",
                             "FGCOLOR*:%d",
                             "FONT*:%d",
                             "BGCOLOR%d:%d",
                             "FGCOLOR%d:%d",
                             "FONT%d:%d"};
      char attr[100], *value;

      /* Atualiza atributos das colunas/celulas `a direita da ultima coluna adicionada */
      for(j=mat_nc(mat);j>base+numcol;j--)
      {
        /* Atualiza os atributos das colunas */
        if(i<4)
        {
          sprintf(attr,string[i],j-numcol);
          value = iupGetEnvRaw(h,attr);
          sprintf(attr,string[i],j);
          iupStoreEnv(h, attr, value);
        }

        /* Atualiza os atributos das celulas */
        else for(k=0;k<=mat_nl(mat);k++)
        {
          sprintf(attr,string[i],k,j-numcol);
          value = iupGetEnvRaw(h,attr);
          sprintf(attr,string[i],k,j);
          iupStoreEnv(h, attr, value);
        }
      }

      /* Os atributos das colunas/celulas adicionadas sao os default */
      for(j=base;j<base+numcol;j++)
      {
        if(i<3)      
        {
          sprintf(attr,string[i],j+1);
          iupSetEnv(h,attr,NULL);
        }
        else for(k=0;k<=mat_nl(mat);k++)
        {
          sprintf(attr,string[i],k,j+1);
          iupSetEnv(h,attr,NULL);
        }
      }
    }

    if (mat_col(mat) >= base) /* Se o foco estiver depois da coluna em que a nova coluna foi inserida soma numcol a coluna do foco */
      mat_col(mat)+=numcol;
  }
  else /* modo == REMOVE */
  {
    /* Move titulos das colunas e larguras */
    for (i=base-1;i<mat_nc(mat);i++)
    {
      sprintf(aux,TITLECOL,i+1+numcol);  /* Pega o titulo da proxima coluna */
      v = iupGetEnvRaw(h,aux);
      sprintf(aux,TITLECOL,i+1);         /* e coloca na coluna atual */
      iupStoreEnv(h,aux,v);

      mat_w(mat)[i] = mat_w(mat)[i+numcol];  /* Rearruma larguras das colunas */

      /* move a indicacao de se a coluna esta ou nao marcada/inativa */
      mat_colmarked(mat)[i]   = mat_colmarked(mat)[i+numcol];
      mat_colinactive(mat)[i] = mat_colinactive(mat)[i+numcol];
    }

    if (!mat->valcb)
    {
      for(i=0;i<mat_nl(mat);i++)             /* Rearruma valores das celulas  */
      {
        for(j=0;j<numcol;j++)
          tmp[j] = mat_v(mat)[i][base-1+j];  /* Guarda regiao alocada, para atribuir a coluna no final*/

        for (j=base-1;j<mat_nc(mat);j++)
          mat_v(mat)[i][j] = mat_v(mat)[i][j+numcol];

        for(j=0;j<numcol;j++)                           /* Recoloca regiao guardada */
          mat_v(mat)[i][mat_nc(mat)+j]=tmp[j];
      }
    }

    /* Marca todas as colunas apagadas como nao marcads */
    for(j=0;j<numcol;j++)
    {
      mat_colmarked(mat)[mat_nc(mat)+j] = 0;
      mat_colinactive(mat)[mat_nc(mat)+j] = 0;
    }

    /* Ajuste do alinhamento e da cor de frente e de fundo das colunas */
    for(i=0;i<7;i++)
    {
      char string[7][100] = { "ALIGNMENT%d",
                             "BGCOLOR*:%d",
                             "FGCOLOR*:%d",
                             "FONT*:%d",
                             "BGCOLOR%d:%d",
                             "FGCOLOR%d:%d",
                             "FONT%d:%d"};
      char attr[100], *value;

      /* Atualiza atributos das colunas/celulas `a direita da ultima coluna apagada */
      for(j=base-1;j<mat_nc(mat);j++)
      {
        /* Atualiza os atributos das colunas */
        if(i<4)
        {
          sprintf(attr,string[i],j+1+numcol);
          value = iupGetEnvRaw(h,attr);
          sprintf(attr,string[i],j+1);
          iupStoreEnv(h, attr, value);
        }

        /* Atualiza os atributos das celulas */
        else for(k=0;k<=mat_nl(mat);k++)
        {
          sprintf(attr,string[i],k,j+1+numcol);
          value = iupGetEnvRaw(h,attr);
          sprintf(attr,string[i],k,j+1);
          iupStoreEnv(h, attr, value);
        }
      }
    }

    if (mat_col(mat) >= base) /* Se o foco estiver depois da linha  que foi retirada, diminui numcol da linha do foco */
      mat_col(mat)-=numcol;
  }

  iupmatGetLastWidth(h,MAT_COL);
  if (tmp) free(tmp);
  free(newwidth);
}



/**************************************************************************
***************************************************************************
*
*   Funcoes exportadas
*
***************************************************************************
***************************************************************************/


/*

%F Retorna o numero de linhas que a matriz possue.
%i h : handle da matriz
%o retorna uma string contendo o numero de linhas da matriz

*/
char *iupmatNlcGetNumLin(Ihandle *h)
{
  static char num[100];
  Tmat *mat = (Tmat*)matrix_data(h);
  sprintf(num,"%d",mat_nl(mat));
  return num;
}


/*

%F Retorna o numero de colunas que a matriz possue.
%i h : handle da matriz
%o retorna uma string contendo o numero de colunas da matriz

*/
char *iupmatNlcGetNumCol(Ihandle *h)
{
  static char num[100];
  Tmat *mat = (Tmat*)matrix_data(h);
  sprintf(num,"%d",mat_nc(mat));
  return num;
}

/*

%F Insere uma ou mais linhas na matriz.
%i h : handle da matriz,
v : string contendo a linha apos a qual serao inseridas outras linhas,
e possivelmente o numero de linhas a ser inserido.
Dois formatos sao possiveis:
a) "%d-%d" insere apos a linha dada pelo primeiro numero, o numero
de linhas dado pelo segundo numero.
b) "%d" insere apos a linha dada pelo numero.

*/
void iupmatNlcAddLin(Ihandle *h,char *v)
{
  Tmat *mat=(Tmat*)matrix_data(h);
  int base=0,oldnl=mat_nl(mat),numlin;
  Ihandle *d   = IupGetDialog(h);
  int visible = (iupCheck(h,IUP_VISIBLE)==YES) && (iupCheck(d,IUP_VISIBLE)==YES);
  int err;
  int ret;

  if (!v) return;

  ret = sscanf(v,"%d-%d",&base,&numlin);

  if (base < 0 || base > oldnl)  /* Fora dos limites validos para base */
    return;

  if (ret != 2) /* Se so reconheceu um numero, usa o formato b */
    numlin = 1;

  /* Sai do modo de edicao */
  iupmatEditCheckHidden(h);

  IsCanvasSet(mat,err);

  if(visible && err == CD_OK)
    IupSetAttribute(h,IUP_VISIBLE,IUP_NO);

  ChangeNumLines(h,oldnl+numlin);

  if (base != oldnl)                             /* Se nao estou inserindo apos a ultima linha */
    ChangeLineAttributes(h,base,numlin,INSERT);  /* Devo mudar atributos e valores da linha 'base' ate o final... */
  else
    GetNewLineAttributes(h,oldnl);

  if(visible && err == CD_OK)
  {
    SetSbV;
    SetSbH;
    IupSetAttribute(h,IUP_VISIBLE,IUP_YES);
  }
}

/*

%F Apaga uma ou mais linhas da matriz.
%i h : handle da matriz,
v : string contendo a linha que sera apagada, e possivelmente o numero
de linhas a ser apagado.
Dois formatos sao possiveis:
a) "%d-%d" apaga a partir da linha dada pelo primeiro numero, o numero
de linhas dado pelo segundo.
b) "%d" apaga a linha dada pelo numero.

*/
void iupmatNlcDelLin(Ihandle *h, char *v)
{
  Tmat *mat=(Tmat*)matrix_data(h);
  int base,oldnl=mat_nl(mat),numlin;
  Ihandle *d   = IupGetDialog(h);
  int visible = (iupCheck(h,IUP_VISIBLE)==YES) && (iupCheck(d,IUP_VISIBLE)==YES);
  int err;
  int ret;

  if (!v) return;

  ret = sscanf(v,"%d-%d",&base,&numlin);

  if (base <= 0 || base > oldnl)  /* Fora dos limites validos para base */
    return;

  if (ret != 2)  /* Se so reconheceu um numero, usa o formato b */
    numlin = 1;

  /* Verifica se a ultima linha a ser apagada ainda esta dentro da planilha.
  Se nao estiver, muda numlin para apagar ate a ultima linha
  */
  if (base+numlin-1 > oldnl)
    numlin = oldnl-base+1;

  /* Sai do modo de edicao */
  iupmatEditCheckHidden(h);

  if(mat_lin(mat)+1>=base && mat_lin(mat)<=base+numlin-2)
  {
    /* Se a primeira linha nao estiver sendo apagada, move o foco para a 
    * primeira celula antes das celulas apagadas */
    if(base!=1)
    {	    
      mat_lin(mat) = base-2;
    }
    /* Caso contrario, move o foco para a primeira celula apos as celulas 
    * apagadas */
    else
    {
      mat_lin(mat) = base+numlin-1 ;
    }	    

    /* Marca a celula que tem o foco */
    iupmatMarkShow(h,0,mat_lin(mat),mat_col(mat),mat_lin(mat),mat_col(mat));
  }

  IsCanvasSet(mat,err);

  if(visible && err == CD_OK)
    IupSetAttribute(h,IUP_VISIBLE,IUP_NO);

  ChangeNumLines(h,oldnl-numlin);

  if (base != oldnl)                            /* Se nao estou apagando a ultima linha */
    ChangeLineAttributes(h,base,numlin,REMOVE); /* Devo mudar atributos e valores da linha 'base' ate o final... */
  else
    GetNewLineAttributes(h,oldnl);

  if(visible && err == CD_OK)
  {
    SetSbV;
    SetSbH;
    IupSetAttribute(h,IUP_VISIBLE,IUP_YES);
  }
}

/*

%F Insere uma ou mais colunas na matriz.
%i h : handle da matriz,
v : string contendo a coluna apos a qual serao inseridas outras colunas,
e possivelmente o numero de colunas a ser inserido.
Dois formatos sao possiveis:
a) "%d-%d" insere apos a coluna dada pelo primeiro numero, o numero
de colunas dado pelo segundo numero.
b) "%d" insere apos a coluna dada pelo numero.

*/
void iupmatNlcAddCol(Ihandle *h,char *v)
{
  Tmat *mat=(Tmat*)matrix_data(h);
  int base=0,oldnc=mat_nc(mat),numcol;
  Ihandle *d   = IupGetDialog(h);
  int visible = (iupCheck(h,IUP_VISIBLE)==YES) && (iupCheck(d,IUP_VISIBLE)==YES);
  int err;
  int ret;

  if (!v) return;

  ret = sscanf(v,"%d-%d",&base,&numcol);

  if (base < 0 || base > oldnc)  /* Fora dos limites validos para base */
    return;

  if (ret != 2)  /* Se so reconheceu um numero, usa o formato b */
    numcol = 1;

  /* Sai do modo de edicao */
  iupmatEditCheckHidden(h);

  IsCanvasSet(mat,err);
  if(visible && err == CD_OK)
    IupSetAttribute(h,IUP_VISIBLE,IUP_NO);

  ChangeNumCols(h,oldnc+numcol);

  if (base != oldnc)                              /* Se nao estou inserindo a ultima coluna */
    ChangeColumnAttributes(h,base,numcol,INSERT); /* Devo mudar atributos e valores da coluna 'base' ate o final... */
  else
    GetNewColumnAttributes(h,oldnc);

  if(visible && err == CD_OK)
  {
    SetSbV;
    SetSbH;
    IupSetAttribute(h,IUP_VISIBLE,IUP_YES);
  }
}

/*

%F Apaga uma ou mais colunas da matriz.
%i h : handle da matriz,
v : string contendo a coluna que sera apagada, e possivelmente o numero
de colunas a ser apagado.
Dois formatos sao possiveis:
a) "%d-%d" apaga a partir da coluna dada pelo primeiro numero, o numero
de colunas dado pelo segundo.
b) "%d" apaga a coluna dada pelo numero.

*/
void iupmatNlcDelCol(Ihandle *h,char *v)
{
  Tmat *mat=(Tmat*)matrix_data(h);
  int base=0,oldnc=mat_nc(mat),numcol;
  Ihandle *d  = IupGetDialog(h);
  int visible = (iupCheck(h,IUP_VISIBLE)==YES) && (iupCheck(d,IUP_VISIBLE)==YES);
  int err;
  int ret;

  if (!v) return;

  ret = sscanf(v,"%d-%d",&base,&numcol);

  if (base <= 0 || base > oldnc)   /* Fora dos limites validos para base */
    return;

  if (ret != 2)  /* Se so reconheceu um numero, usa o formato b */
    numcol = 1;

  /* Verifica se a ultima coluna a ser apagada ainda esta dentro da planilha.
  Se nao estiver, muda numcol para apagar ate a ultima coluna
  */
  if (base+numcol-1 > oldnc)
    numcol = oldnc-base+1;

  /* Sai do modo de edicao */
  iupmatEditCheckHidden(h);

  if(mat_col(mat)+1>=base && mat_col(mat)<=base+numcol-2)
  {
    /* Se a primeira coluna nao estiver sendo apagada, move o foco para a 
    * primeira celula antes das celulas apagadas */
    if(base!=1)
    {	    
      mat_col(mat) = base-2;
    }
    /* Caso contrario, move o foco para a primeira celula apos as celulas 
    * apagadas */
    else
    {
      mat_col(mat) = base + numcol - 1;
    }

    /* Marca a celula que tem o foco */
    iupmatMarkShow(h,0,mat_lin(mat),mat_col(mat),mat_lin(mat),mat_col(mat));
  }

  IsCanvasSet(mat,err);
  if(visible && err == CD_OK)
    IupSetAttribute(h,IUP_VISIBLE,IUP_NO);

  ChangeNumCols(h,oldnc-numcol);

  if (base != oldnc)                              /* Se nao estou apagando a ultima coluna */
    ChangeColumnAttributes(h,base,numcol,REMOVE); /* Devo mudar atributos e valores da coluna 'base' ate o final... */
  else
    GetNewColumnAttributes(h,oldnc);

  if(visible && err == CD_OK)
  {
    SetSbV;
    SetSbH;
    IupSetAttribute(h,IUP_VISIBLE,IUP_YES);
  }
}

/*

%F Muda o numero de linhas da matriz, retirando e colocando linhas no final
%i h : handle da matriz.

*/
void iupmatNlcNumLin(Ihandle *h, char *v)
{
  Tmat *mat=(Tmat*)matrix_data(h);
  int  num=0,oldnl = mat_nl(mat);
  Ihandle *d  = IupGetDialog(h);
  int visible = (iupCheck(h,IUP_VISIBLE)==YES) && (iupCheck(d,IUP_VISIBLE)==YES);
  int err;

  if (!v) return;

  sscanf(v,"%d",&num);

  IsCanvasSet(mat,err);
  if(visible && err == CD_OK)
    IupSetAttribute(h,IUP_VISIBLE,IUP_NO);

  ChangeNumLines(h,num);
  GetNewLineAttributes(h,oldnl);

  if(visible && err == CD_OK)
  {
    SetSbV;
    SetSbH;
    IupSetAttribute(h,IUP_VISIBLE,IUP_YES);
  }
}


/*

%F Muda o numero de colunas da matriz, retirando e colocando colunas no final
%i h : handle da matriz.

*/
void iupmatNlcNumCol(Ihandle *h, char *v)
{
  Tmat *mat=(Tmat*)matrix_data(h);
  int num=0,oldnc=mat_nc(mat);
  Ihandle *d  = IupGetDialog(h);
  int visible = (iupCheck(h,IUP_VISIBLE)==YES) && (iupCheck(d,IUP_VISIBLE)==YES);
  int err;

  if (!v) return;

  IsCanvasSet(mat,err);
  if(visible && err == CD_OK)
    IupSetAttribute(h,IUP_VISIBLE,IUP_NO);

  sscanf(v,"%d",&num);

  ChangeNumCols(h,num);
  GetNewColumnAttributes(h,oldnc); /* Unnecessary call when adding columns? */

  if(visible && err == CD_OK)
  {
    SetSbV;
    SetSbH;
    IupSetAttribute(h,IUP_VISIBLE,IUP_YES);
  }
}

