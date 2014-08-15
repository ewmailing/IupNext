/** \file
 * \brief iupmatrix. definitions.
 *
 * See Copyright Notice in iup.h
 *  */
 
#ifndef __MATRIDEF_H 
#define __MATRIDEF_H

#include "imdraw.h"
#include "iupcompat.h"

#ifdef __cplusplus
extern "C" {
#endif


/* Transicoes de estado possiveis para uma celula */
#define EDITA    1
#define NAOEDITA 0

#define MAT_LIN   DRAW_LIN
#define MAT_COL   DRAW_COL


/* Tamanhos da decoracao em pixels */
#define DECOR_X   6

#ifdef WIN32
#define DECOR_Y   6
#else
#define DECOR_Y  10
#endif

/* Macros para facilitar o acesso a atributos de titulos delinha e coluna */
#define TITLELIN   "%d:0"
#define TITLECOL   "0:%d"

/* Estrutura guardada em cada matriz */

typedef struct _Tx
{
  char *value;         /* Valor da celula                          */
  int nba;             /* Numero de bytes alocados para este valor */
  unsigned char mark;  /* A celula esta marcada?                   */
} Tx;

typedef struct _Tlincol
{
  int *wh;      /* Largura/altura das colunas/linhas        */
  int lastwh;   /* Largura/altura da ultima coluna/linha    */
  int titlewh;  /* Largura/altura do titulo de coluna/linha */

  char *marked;  /* Indica se as colunas/linhas estao ou nao marcadas */
  char *inactive;/* Indica se as colunas/linhas estao ou nao inativas */

  int num;      /* Numero de colunas/linhas na matriz */
  int numaloc;  /* Numero de colunas/linhas alocadas  */

  int first;    /* Primeira coluna/linha visivel */
  int last;     /* Ultima coluna/linha visivel   */

  int totalwh;  /* Soma das larguras/alturas das colunas/linhas */
  int pos;      /* Soma das larguras/alturas da parte invisivel a esquerda/acima */
  int size;     /* Largura/altura da janela visivel */

  int active;   /* Coluna/linha da celula ativa */
} Tlincol;

typedef struct _Tmat
{
   Tx **v;             /* Valor das celulas             */

   sIFnii  valcb;     /* Callback para pegar o valor de uma celula */
   IFniis valeditcb;
   IFnii markcb;
   IFniii markeditcb;

   Ihandle  *self;   /* Handle da matrix              */
   Ihandle  *texth;     /* Handle do text                */
   Ihandle  *droph;     /* Handle do dropdown            */
   Ihandle  *datah;     /* Handle do elemento de edicao ativo no    */
                        /* momento, pode ser igual a texth ou droph */

   cdCanvas *cddbuffer;       /* Handle para uso do cd         */
   cdCanvas *cdcanvas;
   int redraw;
   int checkframecolor;

   Tlincol lin;
   Tlincol col;

   short int hasiupfocus; /* Tem o focus do IUP?                   */

   int MarkLinCol;      /* Esta marcando linhas ou colunas ?       */

   char sb_posicaox[10],sb_tamanhox[10]; /* Usadas para guardar os     */
   char sb_posicaoy[10],sb_tamanhoy[10]; /* valores passados par o IUP */

   int YmaxC;  /* Variaveis para guardar o tamanho do canvas atual */
   int XmaxC;  /* setadas toda vez que uma callback e chamada      */
} Tmat;


/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

/* Macros para acessar a estrutura de dados da matriz */


#define matrix_data(_)  (iupmatMatrixGetData(_))

#define YmaxCanvas(n)	  ((n)->YmaxC)
#define XmaxCanvas(n)	  ((n)->XmaxC)

#define mat_v(n)        ((n)->v)

#define mat_edttexth(n) ((n)->texth)
#define mat_edtdroph(n) ((n)->droph)
#define mat_edtdatah(n) ((n)->datah)

#define mat_w(n)        ((n)->col.wh)
#define mat_h(n)        ((n)->lin.wh)

#define mat_wlc(n)      ((n)->col.lastwh)
#define mat_hll(n)      ((n)->lin.lastwh)
#define mat_wt(n)       ((n)->col.titlewh)
#define mat_ht(n)       ((n)->lin.titlewh)

#define mat_nc(n)       ((n)->col.num)
#define mat_nl(n)       ((n)->lin.num)
#define mat_nca(n)      ((n)->col.numaloc)
#define mat_nla(n)      ((n)->lin.numaloc)

#define mat_fl(n)       ((n)->lin.first)
#define mat_fc(n)       ((n)->col.first)
#define mat_ll(n)       ((n)->lin.last)
#define mat_lc(n)       ((n)->col.last)
#define mat_ultimal(_)  ((_)->lin.num-1)
#define mat_ultimac(_)  ((_)->col.num-1)

#define mat_totw(n)     ((n)->col.totalwh)
#define mat_toth(n)     ((n)->lin.totalwh)
#define mat_posx(n)     ((n)->col.pos)
#define mat_posy(n)     ((n)->lin.pos)
#define mat_sx(n)       ((n)->col.size)
#define mat_sy(n)       ((n)->lin.size)

#define mat_self(_)  ((_)->self)

#define mat_col(_)      ((_)->col.active)
#define mat_lin(_)      ((_)->lin.active)

#define mat_hasiupfocus(_) ((_)->hasiupfocus)

#define mat_markLC(_)      ((_)->MarkLinCol)

#define mat_colmarked(_)   ((_)->col.marked)
#define mat_linmarked(_)   ((_)->lin.marked)

#define mat_colinactive(_) ((_)->col.inactive)
#define mat_lininactive(_) ((_)->lin.inactive)



/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

/* Macros para setar a scrollbar */

#define TAMX (mat_totw(mat) ? ((float)mat_sx(mat)/(float)mat_totw(mat)) : 0)
#define TAMY (mat_toth(mat) ? ((float)mat_sy(mat)/(float)mat_toth(mat)) : 0)

#define POSX (mat_totw(mat) ? ((float)mat_posx(mat)/(float)mat_totw(mat)) : 0)
#define POSY (mat_toth(mat) ? ((float)mat_posy(mat)/(float)mat_toth(mat)) : 0)


#define SetSbH  if(mat)                                                   \
                {                                                         \
                  sprintf(mat->sb_posicaox,"%.5f",POSX);                       \
                  sprintf(mat->sb_tamanhox,"%.5f",TAMX);                       \
                  IupSetAttribute(mat_self(mat),IUP_DX  ,mat->sb_tamanhox); \
                  IupSetAttribute(mat_self(mat),IUP_POSX,mat->sb_posicaox); \
                  cdCanvasActivate(cdcv(mat));                                 \
                }


#define SetSbV  if(mat)                                                   \
                {                                                         \
                  sprintf(mat->sb_posicaoy,"%.5f",POSY);                       \
                  sprintf(mat->sb_tamanhoy,"%.5f",TAMY);                       \
                  IupSetAttribute(mat_self(mat),IUP_DY  ,mat->sb_tamanhoy); \
                  IupSetAttribute(mat_self(mat),IUP_POSY,mat->sb_posicaoy); \
                  cdCanvasActivate(cdcv(mat));                                 \
                }


#define SetSb(m) if((m)==MAT_COL) {SetSbH;} else {SetSbV;}

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

Tmat* iupmatMatrixGetData(Ihandle* self);

#define NO      0
#define YES     1

#ifdef __cplusplus
}
#endif

#endif
