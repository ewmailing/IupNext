/** \file
 * \brief iupmatrix control
 * draw functions
 *
 * See Copyright Notice in iup.h
 * $Id: imdraw.c,v 1.1 2008-10-17 06:20:15 scuri Exp $
 */

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#undef CD_NO_OLD_INTERFACE

#include <iup.h>
#include <iupcpi.h>
#include <cd.h>
#include "istrutil.h"

#include "iupmatrix.h"
#include "matridef.h"
#include "matrixcd.h"
#include "imdraw.h"
#include "imaux.h"
#include "immark.h"
#include "iupcdutil.h"

/* Fator de atenuacao de cor de uma celula marcada */
#define ATENUATION_NO      1.0F
#define ATENUATION_FOCUS   0.8F
#define ATENUATION_NOFOCUS 1.0F

/* Alinhamento do texto que sera desenhado.  Usado por DrawText */
#define T_CENTER  1
#define T_LEFT    2
#define T_RIGHT   3

/* Cores usadas para desenhar os textos. Usado por DrawText */
#define COR_TITULO  0    /* Letras Pretas c/ fundo cinza  (p/ os titulos) */
#define COR_ELEM    1    /* Letras c/ FGCOLOR e fundo c/ BGCOLOR          */
#define COR_REVERSE 2    /* Letras c/ BGCOLOR e fundo c/ FGCOLOR          */


#define CD_INACTIVE_COLOR 0x666666L

#define CD_BS 0x666666L  /* Bottom Shadow */
#define CD_TS 0xFFFFFFL  /* Top Shadow    */

typedef int (*Drawcb)(Ihandle *h, int lin, int col,int x1, int x2, int y1, int y2, cdCanvas* cnv);

/**************************************************************************
***************************************************************************
*
*   Funcoes internas para tratamento de cores
*
***************************************************************************
***************************************************************************/

void iupmatSetCdFrameColor(Ihandle *h)
{
  Tmat *mat=(Tmat*)matrix_data(h);
  unsigned int r,g,b;
  iupGetColor(h, IUP_FRAMECOLOR, &r, &g, &b);
  cdCanvasForeground(mat->cddbuffer,cdEncodeColor((unsigned char) r, (unsigned char) g, (unsigned char) b));
}

static float GetAttenuation(Ihandle *h)
{
  char *mark = IupGetAttribute(h, IUP_MARK_MODE);
	if(mark == NULL || iupStrEqual(mark, "NO"))
		return ATENUATION_NO;
	else
    return ATENUATION_FOCUS;
}

typedef int (*Colorcb )(Ihandle*, int, int, unsigned int *, unsigned int *, unsigned int *);

static int callFgColorCB(Ihandle *h, int lin, int col, unsigned int *r, unsigned int *g, unsigned int *b)
{
  Colorcb cb = (Colorcb)IupGetCallback(h,"FGCOLOR_CB");
  if (cb)
    return cb(h, lin, col, r, g, b);
  else
    return IUP_IGNORE;
}

static int callBgColorCB(Ihandle *h, int lin, int col, unsigned int *r, unsigned int *g, unsigned int *b)
{
  Colorcb cb = (Colorcb)IupGetCallback(h,"BGCOLOR_CB");
  if (cb)
    return cb(h, lin, col, r, g, b);
  else
    return IUP_IGNORE;
}

static int CallDrawCB(Ihandle *h, Tmat *mat, int cor, int lin, int col, int x1, int x2, int y1, int y2)
{
  Drawcb cb = (Drawcb)IupGetCallback(h,"DRAW_CB");
  if (cb)
  {
    cdCanvas* old_cnv = cdActiveCanvas();
    if (cor == COR_TITULO)
    {
      x1+=1;
      x2-=1;
      y1+=1;
      y2-=1;
    }
    else if (iupCheck(h, "HIDEFOCUS")!=YES)
    {
      x1+=2;
      x2-=2;
      y1+=2;
      y2-=2;
    }

    CdClipArea(x1,x2,y1,y2);
    cdCanvasClip(mat->cddbuffer,CD_CLIPAREA);

    if (old_cnv != mat->cddbuffer) /* backward compatibility code */
      cdActivate(mat->cddbuffer);

    if (cb(h, lin, col, x1, x2, INVY(y1), INVY(y2), mat->cddbuffer) == IUP_DEFAULT)
    {
      cdCanvasClip(mat->cddbuffer,CD_CLIPOFF);
      if (old_cnv != mat->cddbuffer)
        cdActivate(old_cnv);
      return 0;
    }
    cdCanvasClip(mat->cddbuffer,CD_CLIPOFF);
    if (old_cnv != mat->cddbuffer)
      cdActivate(old_cnv);
  }

  return 1;
}

static void matDrawGetColor(Ihandle *h, char* attrib, int lin, int col, unsigned int *r, unsigned int *g, unsigned int *b, int parent)
{
  char* value;
  char colorattr[30];

  /* 1 -  check for this cell */
  sprintf(colorattr,"%s%d:%d",attrib,lin,col);
  value = iupGetEnvRaw(h,colorattr);
  if (!value)
  {
    /* 2 - check for this line, if not title col */
    if (col != 0)
    {
      sprintf(colorattr,"%s%d:*",attrib,lin);
      value = iupGetEnvRaw(h,colorattr);
    }

    if (!value)
    {
      /* 3 - check for this column, if not title line */
      if (lin != 0)
      {
        sprintf(colorattr,"%s*:%d",attrib,col);
        value = iupGetEnvRaw(h,colorattr);
      }

      if (!value)
      {
        /* 4 - check for the matrix or parent */
        if (parent)
          value = iupGetParentBgColor(h);
        else
          value = iupGetEnvRaw(h,attrib);
      }
    }
  }

  if (value)
    iupGetRGB(value, r, g, b);
}

/*
%F  Muda a cor de frente do Cd, para a cor necessaria para desenhar uma
    celula com sua FOREGROUND COLOR.  Isto implica em verificar se ha um
    atributo de cor proprio daquela celula. Se nao houver tenta um atributo
    de cor para a linha, senao coluna, senao para a matriz toda.  Finalmente
    se nao encontrar nenhum destes, usa a cor default.

%i  h - Handle da matriz.
    lin,col - Coordenadas da celula, dadas em formato IUP, ou
             seja 1,1 representa a celula superior esquerda da matriz,
             valores de lin e col iguais a 0 se referem as linhas e colunas
             de titulos.
    type - Tipo da celula. Se for uma celula de titulo (type = COR_TITULO), so
           procura por uma cor propria da celula, e nao em formas mais
           abrangentes de definir cores.
    mark - Indica se a celula esta marcada. Se estiver, sua cor e' atenuada.
%o Retorna a antiga cor de frente do CD.
*/
static unsigned long SetFgColor(Ihandle *h, int lin, int col, int mark)
{
  Tmat *mat=(Tmat*)matrix_data(h);
  unsigned int r=0,g=0,b=0;

  if (callFgColorCB(h, lin, col, &r, &g, &b) == IUP_IGNORE)
    matDrawGetColor(h, "FGCOLOR", lin, col, &r, &g, &b, 0);

  if (mark)
  {
		float att = GetAttenuation(h);
    r = (int)(r*att);
    g = (int)(g*att);
    b = (int)(b*att);
  }
  
  return cdCanvasForeground(mat->cddbuffer,cdEncodeColor((unsigned char)r,(unsigned char)g,(unsigned char)b));
}

static unsigned long SetTitleFgColor(Ihandle *h, int lin, int col)
{
  return SetFgColor(h, lin, col, 0);
}


/*
%F  Muda a cor de frente do Cd, para a cor necessaria para desenhar uma
    celula com sua BACKGROUND COLOR.  Isto implica em verificar se ha um
    atributo de cor proprio daquela celula. Se nao houver tenta um atributo
    de cor para a linha, senao coluna, senao para a matriz toda.  Finalmente
    se nao encontrar nenhum destes, usa a cor default.

%i  h - Handle da matriz.
    lin,col - Coordenadas da celula, dadas em formato IUP, ou
             seja 1,1 representa a celula superior esquerda da matriz,
             valores de lin e col iguais a 0 se referem as linhas e colunas
             de titulos.
    type - Tipo da celula. Se for uma celula de titulo (type = COR_TITULO), so
           procura por uma cor propria da celula, e nao em formas mais
           abrangentes de definir cores.
    mark - Indica se a celula esta marcada. Se estiver, sua cor e' atenuada.
%o Retorna a antiga cor de frente do CD.
*/
static unsigned long SetBgColor(Ihandle *h, int lin, int col, int type, int mark)
{
  Tmat *mat=(Tmat*)matrix_data(h);
  unsigned int r=255,g=255,b=255;
  int parent = 0;

  if (type == COR_TITULO)
    parent = 1;

  if (callBgColorCB(h, lin, col, &r, &g, &b) == IUP_IGNORE)
    matDrawGetColor(h, "BGCOLOR", lin, col, &r, &g, &b, parent);
  
  if (mark)
  {
		float att = GetAttenuation(h);
    r = (int)(r*att);
    g = (int)(g*att);
    b = (int)(b*att);
  }

  return cdCanvasForeground(mat->cddbuffer,cdEncodeColor((unsigned char)r,(unsigned char)g,(unsigned char)b));
}

static unsigned long SetTitleBgColor(Ihandle *h, int lin, int col)
{
  int mark = 0;

  if((lin == 0 && iupmatMarkColumnMarked(h,col)) || 
     (col == 0 && iupmatMarkLineMarked(h,lin)))
    mark = 1;

  return SetBgColor(h, lin, col, COR_TITULO, mark);
}

static void SetEmptyAreaColor(Ihandle *h)
{
  Tmat *mat=(Tmat*)matrix_data(h);
  cdCanvasForeground(mat->cddbuffer,cdIupConvertColor(iupGetParentBgColor(h)));
}

/**************************************************************************
***************************************************************************
*
*   Funcoes internas
*
***************************************************************************
***************************************************************************/

/*
%F Retorna qual o atributo de fonte usado para desenhar uma
   determinada celula da matriz.

%i h       - Handle da matriz
   lin,col - Coordenadas da celula a ter sua cor consultada.  As coordenadas
             sao coordenadas IUP, ou seja 0,0 representa a celula entre os
             titulos, e 1,1 representa a celula da primeira linha, e primeira
             coluna da matriz.
%o Retorna uma string com o atributo de fonte usadao para desenhar a celula.
*/
char* iupmatDrawGetFont(Ihandle *h, int lin, int col)
{
  char* value;
  char fontattr[30];

  /* 1 -  check for this cell */
  sprintf(fontattr,"FONT%d:%d",lin,col);
  value = iupGetEnvRaw(h,fontattr);
  if (!value)
  {
    /* 2 - check for this line, if not title col */
    if (col != 0)
    {
      sprintf(fontattr,"FONT%d:*",lin);
      value = iupGetEnvRaw(h,fontattr);
    }

    if (!value)
    {
      /* 3 - check for this column, if not title line */
      if (lin != 0)
      {
        sprintf(fontattr,"FONT*:%d",col);
        value = iupGetEnvRaw(h,fontattr);
      }

      if (!value)
      {
        /* 4 - check for the matrix or parent */
        value = IupGetAttribute(h,"FONT");
      }
    }
  }

  return value;
}

static void SetFont(Ihandle *h, int lin, int col)
{
  Tmat *mat=(Tmat*)matrix_data(h);
  cdCanvasNativeFont(mat->cddbuffer, iupmatDrawGetFont(h, lin, col));
}

/*
%F Desenha uma caixa na tela, com a aparencia de um botao.

%i h - Handle da matriz
   px1, px2 - Coordenadas do canto inferior esquerdo da caixa.
   pdx,pdy  - Largura e altura da caixa.
   lin,col  - Coordenadas da celula (para calculo da cor)
*/
static void BoxReleased(Ihandle *h,int px1,int py1,int pdx,int pdy,int lin,int col)
{
  Tmat *mat=(Tmat*)matrix_data(h); /* Usado pela macro CdBox */
  int px2 = px1+pdx-1, py2 = py1+pdy-1;

  if (pdx < 2||pdy <2) return;

  cdCanvasForeground(mat->cddbuffer,CD_BS);
  CdLine(px2,py1,px2,py2);
  CdLine(px1,py2,px2,py2);
  cdCanvasForeground(mat->cddbuffer,CD_TS);
  CdLine(px1,py1,px1,py2);
  CdLine(px1,py1,px2,py1);
  SetTitleBgColor(h,lin,col);
  CdBox(px1+1,px2-1,py1+1,py2-1);
}

/*
%F Desenha uma caixa na tela, com a aparencia de um botao pressionado.

%i h - Handle da matriz
   px1, px2 - Coordenadas do canto inferior esquerdo da caixa.
   pdx,pdy  - Largura e altura da caixa.
   lin,col  - Coordenadas da celula (para calculo da cor)
*/
static void BoxPressed(Ihandle *h,int px1,int py1,int pdx,int pdy,int lin,int col)
{
  Tmat *mat=(Tmat*)matrix_data(h); /* Usado pela macro CdBox */

  int px2 = px1+pdx-1, py2 = py1+pdy-1-1;

  if (pdx < 2||pdy <2) return;

  cdCanvasForeground(mat->cddbuffer,CD_TS);
  CdLine(px2,py1,px2,py2);
  CdLine(px1,py2,px2,py2);
  cdCanvasForeground(mat->cddbuffer,CD_BS);
  CdLine(px1,py1,px1,py2);
  CdLine(px1,py1,px2,py1);
  SetTitleBgColor(h,lin,col);
  CdBox(px1+1,px2-1,py1+1,py2-1);
}

/*
%F Decide se deve desenhar o titulo como marcado ou nao, e chama
   a funcao correspondente para desenhar a caixa do titulo.

%i h     - Handle da matriz
   lin   - Linha que vai ter seu titulo desenhado
   x1,y1 - Coordenadas, em pixel, do canto inferior esquerdo da caixa
           que vai conter o titulo.
   dx,dy - Largura e altura da caixa do texto.
*/
static void TitleLineBox(Ihandle *h, int lin, int x1, int y1,
                         int dx, int dy)
{
  if(iupmatMarkLineMarked(h,lin))
    BoxPressed(h,x1,y1,dx,dy,lin,0);
  else
    BoxReleased(h,x1,y1,dx,dy,lin,0);
}

/*
%F Decide se deve desenhar o titulo como marcado ou nao, e chama
   a funcao correspondente para desenhar a caixa do titulo.

%i h     - Handle da matriz
   lin   - Coluna que vai ter seu titulo desenhado
   x1,y1 - Coordenadas, em pixel, do canto inferior esquerdo da caixa
           que vai conter o titulo.
   dx,dy - Largura e altura da caixa do texto.
*/
static void TitleColumnBox(Ihandle *h, int col, int x1, int y1,
                           int dx, int dy)
{
  if(iupmatMarkColumnMarked(h,col))
    BoxPressed(h,x1,y1,dx,dy,0,col);
  else
    BoxReleased(h,x1,y1,dx,dy,0,col);
}

static void DrawComboFeedback(Ihandle *h, int x2, int y1, int y2, int lin, int col, int cor)
{
#define BOXW 16

  Tmat *mat=(Tmat*)matrix_data(h);
  int xh2, yh2, x1;

  /* cell background */
  if (cor == COR_ELEM)
    SetBgColor(h,lin,col,cor,0);
  else
    SetBgColor(h,lin,col,cor,1);
  CdBox(x2-BOXW,x2,y1,y2); 

  /* feddback area */
  x2 -= 3;
  x1 = x2-BOXW; 
  y1 += 2; y2 -= 3;

  /* feedback background */
  SetTitleBgColor(h,0,0);
  CdBox(x1,x2,y1,y2);

  /* feedback frame */
  iupmatSetCdFrameColor(h);
  CdRect(x1,x2,y1,y2);

  /* feedback arrow */
  xh2 = x2-BOXW/2;
  yh2 = y2-(y2-y1)/2;

  cdCanvasBegin(mat->cddbuffer,CD_FILL);
  CdVertex(xh2,yh2+3);
  CdVertex(xh2+4,yh2-1);
  CdVertex(xh2-4,yh2-1);
  cdCanvasEnd(mat->cddbuffer);
}

/*

%F Coloca um texto na tela, com a cor e alinhamento especificados. Recebe o
   tamanho da celula, e tambem qual a parte visivel da celula, para poder
   clipar contra a parte visivel, mas calcular corretamente a posicao do
   centro e margem direita da celula, usando o tamnaho total dela.
%i h : handle da matriz,
   y1,y2 : limites verticais da celula,
   x1,x2 : limites horizontais da celula completa,
   xc : ponto em que o texto e clipado,
   text  : texto a ser escrito,
   alignment : tipo de alinhamento (horizontal) dado ao texto, pode ser:
               [T_CENTER,T_LEFT,T_RIGHT],
   cor   : esquema de cor que vai ser uasdo :
           COR_TITULO  -> Letras Pretas c/ fundo cinza  (p/ os titulos)
           COR_ELEM    -> Letras c/ FGCOLOR e fundo c/ BGCOLOR
           COR_REVERSE -> Letras c/ BGCOLOR e fundo c/ FGCOLOR
   lin,col - Coordenadas da celula a ser desenhada, dadas em formato IUP, ou
             seja 1,1 representa a celula superior esquerda da matriz,
             valores de lin e col iguais a 0 se referem as linhas e colunas
             de titulos
*/
static void DrawText(Ihandle *h,int x1, int x2, int y1, int y2, char *text, int alignment, int xc, int cor, int lin, int col)
{
  Tmat *mat=(Tmat*)matrix_data(h); /* Usado pela macro CdBox */
  int oldbgc = -1;
  int ypos;

  int oldx1 = x1,                  /* limite completo da celula sem ter */
      oldx2 = x2-1,                /* pulando as decoracoes */
      oldy1 = y1,
      oldy2 = y2,
      oldxc = xc;

  /* Deixa um espaco entre o texto e a margem da celula */
  x1+=DECOR_X/2;  x2-=DECOR_X/2; xc-=DECOR_X/2;
  y1+=DECOR_Y/2;  y2-=DECOR_Y/2;

  /* Limpa a celula */
  if (cor == COR_TITULO)
  {
    oldbgc = SetBgColor(h,lin,col,cor,0);
    CdBox(oldx1+2,oldxc-2,oldy1+2,oldy2-2); /* Limpa caixa da celula mas limitada pelo xc */
  }
  else if (cor == COR_ELEM)
  {
    oldbgc = SetBgColor(h,lin,col,cor,0);
    CdBox(oldx1,oldx2,oldy1,oldy2); /* Limpa caixa da celula */
  }
  else
  {
    oldbgc = SetBgColor(h,lin,col,cor,1);
    /* Limpa a caixa do texto, com a cor atenuada... */
    CdBox(oldx1,oldx2,oldy1,oldy2);
    SetFgColor(h,lin,col,1);
  }

  if (!CallDrawCB(h, mat, cor, lin, col, oldx1, oldx2, oldy1, oldy2))
    return;

  /* Coloca o texto */
  if (text && *text)
  {
    int numl;
    int lineh, totalh, spacing;

    /* Seta a area de clip para a regiao da celula dada */
    CdClipArea(x1,xc,oldy1,oldy2);
    cdCanvasClip(mat->cddbuffer,CD_CLIPAREA);

    /* Seta a cor usada para desenhar o texto */
    if((lin > 0 && mat_lininactive(mat)[lin-1]) ||
       (col > 0 && mat_colinactive(mat)[col-1]) ||
       !iupCheck(h,IUP_ACTIVE))
      cdCanvasForeground(mat->cddbuffer,CD_INACTIVE_COLOR);
    else if (cor == COR_ELEM)
      SetFgColor(h,lin,col,0);
    else if (cor == COR_REVERSE)
      oldbgc = SetFgColor(h,lin,col,1);
    else
      SetFgColor(h,lin,col,0);

    numl = iupmatTextHeight(h,text,&totalh,&lineh,&spacing);

    SetFont(h, lin, col);

    if (numl == 1)
    {
      ypos = (int)((y1+y2)/2.0 - .5);

      /* Coloca o texto */
      if (alignment == T_CENTER)
        CdPutText((x1+x2)/2,ypos,text,CD_CENTER);
      else if (alignment == T_LEFT)
        CdPutText(x1,ypos,text,CD_WEST);
      else
        CdPutText(x2,ypos,text,CD_EAST);
    }
    else
    {
      int   i;
      char *p,*q, *newtext=NULL;

      if(text != NULL)
      {
        p = (char*) iupStrDup(text);
        newtext = p;
      }
      else
        p = NULL;

      /* Pega a posicao do primeiro texto a ser colocado na tela */
      ypos = (int)((y1+y2)/2.0 - .5) - totalh/2 + lineh/2;
      for(i=0;i<numl;i++)
      {
        q = strchr(p,'\n');
        if (q)
          *q = 0;  /* Corta a string para so conter uma linha */

        /* Coloca o texto */
        if (alignment == T_CENTER)
          CdPutText((x1+x2)/2,ypos,p,CD_CENTER);
        else if (alignment == T_LEFT)
          CdPutText(x1,ypos,p,CD_WEST);
        else
          CdPutText(x2,ypos,p,CD_EAST);

        if (q)
          *q = '\n'; /* Restaura a string */
        p = q+1;

        /* Avanca uma linha */
        ypos += lineh + spacing;

      }

      if(newtext)
        free(newtext);
    }

    cdCanvasClip(mat->cddbuffer,CD_CLIPOFF);
  }

  if (cor != COR_TITULO)
    CdRestoreBgColor();
}

static void DrawSort(Ihandle *h,int x2, int y1, int y2, int xc, int lin, int col, char* sort)
{
  Tmat *mat=(Tmat*)matrix_data(h); /* Usado pela macro CdBox */
  int yc;

  /* Deixa um espaco entre o texto e a margem da celula */
  x2-=DECOR_X/2; xc-=DECOR_X/2;

  /* Seta a cor usada para desenhar o texto */
  if((lin > 0 && mat_lininactive(mat)[lin-1]) ||
      (col > 0 && mat_colinactive(mat)[col-1]) ||
      !iupCheck(h,IUP_ACTIVE))
    cdCanvasForeground(mat->cddbuffer,CD_INACTIVE_COLOR);
  else
    SetFgColor(h,lin,col,0);

  yc = (int)((y1+y2)/2.0 - .5);

  cdCanvasBegin(mat->cddbuffer,CD_FILL);

  if (iupStrEqual(sort, "UP"))
  {
    CdVertex(x2-5, yc+2);
    CdVertex(x2-1, yc-2);
    CdVertex(x2-9, yc-2);
  }
  else
  {
    CdVertex(x2-1, yc+2);
    CdVertex(x2-9, yc+2);
    CdVertex(x2-5, yc-2);
  }

  cdCanvasEnd(mat->cddbuffer);
}

/*
%F Retorna o alinhamento a ser usado por uma determinada coluna
   da matriz.
*/
static int GetColAlignment(Ihandle* h, int col)
{
  char attr[15], *align;

  sprintf(attr,"ALIGNMENT%d",col);
  align = IupGetAttribute(h,attr);
  assert(align);  /* Sempre tem valor (mecanismo de default attribute) */

  if(iupStrEqual(align,IUP_ALEFT))
    return T_LEFT;
  else if(iupStrEqual(align,IUP_ACENTER))
    return T_CENTER;
  else
    return T_RIGHT;
}

/**************************************************************************
***************************************************************************
*
*   Funcoes exportadas
*
***************************************************************************
***************************************************************************/

/*
%F Desenha os titulos das linhas, visiveis, entre lin e lastlin,
inclusive. Titulos de linhas marcadas vao ser desenhados com o feedback
apropriado.

%i h - Handle da matriz
   lin - Primeira linha a ter seu titulo desenhado
   lastlin - Ultima linha a ter seu titulo desenhado
%o Retorna a posicao(em pixels) do final da ultima linha desenhada
*/
int iupmatDrawLineTitle(Ihandle *h, int lin, int lastlin)
{
 Tmat *mat=(Tmat*)matrix_data(h);
 int x1, y1, x2, y2;
 int j;
 char *str;

 /* Se nao tinha titulo de linha, mesmo assim deve retornar a posicao em pixels
    que seria a posicao vertical do final da ultima linha desenhada se houvesse
    necessidade de desenho.  Isto e' um requisito usado pela funcao de scroll
    de tela.
 */


 /* arruma lin e lastlin para conterem a regiao de linhas que tenho que
    desenhar (estao visiveis)
 */

 if(lin < mat_fl(mat))
   lin = mat_fl(mat);

 if(lastlin > mat_ll(mat))
   lastlin = mat_ll(mat);

 if (lastlin < lin)
   return 0;

 /* Inicializa posicao do primeiro titulo de linha */
 x1 = 0;
 x2 = mat_wt(mat);
 y1 = mat_ht(mat);

 for(j=mat_fl(mat);j<lin;j++)
   y1 += mat_h(mat)[j];

 mat->redraw = 1;

 /* Desenha os titulos */
 for(j=lin;j<=lastlin;j++)
 {
   /* Se e' uma linha escondida (tamanho zero), nao desenha o titulo */
   if(mat_h(mat)[j] == 0)
     continue;

   y2 = y1+mat_h(mat)[j]-1;

   /* Se nao tenho titulo, o loop so calcula a posicao final */
   if(mat_wt(mat))
   {
     TitleLineBox(h,j+1,x1,y1,x2-x1,y2-y1+1);
     str = iupmatGetCellValue(h,j,-1);
     DrawText(h,x1,x2,y1,y2,str,GetColAlignment(h,0),x2,COR_TITULO,j+1,0);
     iupmatSetCdFrameColor(h);
     CdLine(x2-1,y1,x2-1,y2);
   }

   y1=y2+1;
 }

 return y1;
}

/*
%F Desenha os titulos de colunas, visiveis, entre col e lastcol,
inclusive. Titulos de colunas marcadas vao ser desenhados com o feedback
apropriado.

%i h - Handle da matriz
   col - Primeira coluna a ter seu titulo desenhado
   lastcol - Ultima coluna a ter seu titulo desenhado
%o Retorna a posicao(em pixels) do final da ultima coluna desenhada
*/
int iupmatDrawColumnTitle(Ihandle *h, int col, int lastcol)
{
 Tmat *mat=(Tmat*)matrix_data(h);
 int x1, y1, x2, y2;
 int j;
 char *str;

 /* Se nao tinha titulo de coluna, mesmo assim deve retornar a posicao em pixels
    que seria a posicao horizontal do final da ultima coluna desenhada se houvesse
    necessidade de desenho.  Isto e' um requisito usado pela funcao de scroll
    de tela.
 */


 /* arruma col e lastcol para conterem a regiao de colunas que tenho que
    desenhar (estao visiveis)
 */

 if(col < mat_fc(mat))
   col = mat_fc(mat);

 if(lastcol > mat_lc(mat))
   lastcol = mat_lc(mat);

 if (lastcol < col)
   return 0;

 /* Inicializa posicao do primeiro titulo de coluna */
 y1 = 0;
 y2 = mat_ht(mat)-1;
 x1 = mat_wt(mat);

 for(j=mat_fc(mat);j<col;j++)
   x1 += mat_w(mat)[j];

 mat->redraw = 1;

 /* Desenha os titulos */
 for(j=col;j<=lastcol;j++)
 {
   int w,alignment;

   /* Se e' uma coluna escondida (tamanho zero), nao desenha o titulo */
   if(mat_w(mat)[j] == 0)
     continue;

   /* Decide qual alinhamento usar.
      Se o texto cabe inteiro no canvas, alinha pelo centro, se nao
      alinha pela esquerda
   */
   x2 = x1 + (j==mat_lc(mat)?mat_wlc(mat):mat_w(mat)[j]);

   /* Se nao tenho titulo, o loop so calcula a posicao final */
   if(mat_ht(mat))
   {
     str = iupmatGetCellValue(h,-1,j);
     if(str)
       iupmatTextWidth(h,str,&w);
     else
       w = 0;
     alignment = (w > x2-x1+1-DECOR_X) ? T_LEFT : T_CENTER;

     /* desenha o titulo */
     TitleColumnBox(h,j+1,x1,y1,x2-x1,y2-y1+1);
     DrawText(h,x1,x1+mat_w(mat)[j],y1,y2,str,alignment,x2,COR_TITULO,0,j+1);
     iupmatSetCdFrameColor(h);
     CdLine(x1,y2,x2-1,y2);

     {
       char aux[50], *sort;
       sprintf(aux, "SORTSIGN%d", j+1);
       sort = iupGetEnvRaw(h,aux);
       if (sort && !iupStrEqual(sort, "NO"))
         DrawSort(h,x1+mat_w(mat)[j], y1, y2, x2, 0, j+1, sort);
     }
   }
   x1=x2;
 }

 return x1;
}

/*
%F  Desenha o canto entre os titulos de linha e os titulos de coluna

%i h - Handle da matriz
*/
void iupmatDrawTitleCorner(Ihandle *h)
{
 Tmat *mat=(Tmat*)matrix_data(h);
 char *str;

 /* se tem linhas ou colunas e se tem titulo de coluna e de linha, */
 /* entao desenha canto superior esquerdo                          */
 if ((mat_nl(mat) != 0 || mat_nc(mat) != 0) && mat_ht(mat) && mat_wt(mat))
 {
   mat->redraw = 1;

   BoxReleased(h,0,0,mat_wt(mat),mat_ht(mat),0,0);
   str = iupmatGetCellValue(h,-1,-1);
   DrawText(h,0,mat_wt(mat),0,mat_ht(mat)-1,str,T_CENTER,mat_wt(mat),COR_TITULO,0,0);
 }
}

/*
%F Limpa a area recebida com a cor apropriada para a area da matriz que
   nao possue celulas

%i h - Handle da matriz
   x1,x2,y1,y2 - Coordenadas da area a ser limpa.
*/
void iupmatDrawEmptyArea(Ihandle *h, int x1, int x2, int y1, int y2)
{
  Tmat *mat=(Tmat*)matrix_data(h); /* Usado por CdBox que e'uma macro... */
  SetEmptyAreaColor(h);
  CdBox (x1,x2,y1,y2);
  mat->redraw = 1;
}

/*

%F Redesenha a matriz toda, podendo ou nao redesenhar os titulos de
   linhas e colunas.
%i h : handle da matriz,
   modo : constante que especifica que titulos devem ser redesenhados
          pode ter os seguintes valores :
          DRAW_ALL -> Redesenha as colunas e linhas de titulos
          DRAW_COL -> Redesenha somente as colunas de titulos
          DRAW_LIN -> Redesenha somente as linhas de titulos
          Sempre redesenha as celulas, os modos acima so afetam o
          desenho dos titulos....
*/
void iupmatDrawMatrix(Ihandle *h,int modo)
{
 Tmat *mat=(Tmat*)matrix_data(h);
 mat->redraw = 1;

 if ((mat_nl(mat) == 0) || (mat_nc(mat) == 0))
 {
   cdCanvasBackground(mat->cddbuffer,cdIupConvertColor(iupGetParentBgColor(h)));
   cdCanvasClear(mat->cddbuffer);
 }

 /* Desenha o canto entre os titulos de linha e coluna, se necessario */
 iupmatDrawTitleCorner(h);

 /* se ha' colunas e tem que desenhar as colunas, entao desenha os titulos */
 if ((mat_nc(mat) != 0) &&
     (modo == DRAW_ALL || modo == DRAW_COL) && mat_ht(mat))
 {
   iupmatDrawColumnTitle(h,mat_fc(mat),mat_lc(mat));
 }

 /* se ha' linhas e tem que desenhar as linhas , entao desenha os titulos */
 if ((mat_nl(mat) != 0) &&
     (modo == DRAW_ALL || modo == DRAW_LIN) &&
     mat_wt(mat))
 {
   iupmatDrawLineTitle(h,mat_fl(mat),mat_ll(mat));
 }

 /* se ha' celulas na matrix, entao desenha as celulas */
 if ((mat_nl(mat) != 0) && (mat_nc(mat) != 0))
   iupmatDrawCells(h, mat_fl(mat), mat_fc(mat), mat_ll(mat), mat_lc(mat));
}

static int CallDropdownCheckCb(Ihandle *h, int line, int col)
{
  IFnii cb = (IFnii)IupGetCallback(h,"DROPCHECK_CB");
  if(cb)
  {
    int ret = cb(h,line+1,col+1);
    if(ret == IUP_DEFAULT)
      return 1;
  }
  return 0;
}

/*

%F Redesenha um bloco de celulas da matriz. Trata celulas marcadas, mudando
   automaticamente a cor de fundo delas.
%i h : handle da matriz,
   l1,c1 : coordenadas da celula que marca o canto superior esquerdo da area
           a ser redesenhada,
   l2,c2 : coordenadas da celula que marca o canto inferior direito da area
           a ser redesenhada.
*/
void iupmatDrawCells(Ihandle *h, int l1, int c1, int l2, int c2)
{
 Tmat *mat=(Tmat*)matrix_data(h);
 int x1, y1, x2, y2,oldx2,oldy1,oldy2;
 int yc1,yc2,xc1,xc2,i,j;
 int align;
 long framecolor;
 char str[30];
 unsigned int r,g,b;


 /* Se nao ha' celulas na matrix, retorna */
 if (mat_nl(mat) == 0 || mat_nc(mat) == 0)
  return;

 /* Critica parametros */
 if (c1 < mat_fc(mat))
   c1 = mat_fc(mat);
 if (c2 > mat_lc(mat))
   c2 = mat_lc(mat);

 if (l1 < mat_fl(mat))
   l1 = mat_fl(mat);
 if (l2 > mat_ll(mat))
   l2 = mat_ll(mat);

 if (c1 > c2 || l1 > l2)
   return;

  mat->redraw = 1;

 if(l1<=l2) iupmatDrawLineTitle(h,l1,l2);
 if(c1<=c2) iupmatDrawColumnTitle(h,c1,c2);

 x1= 0;  x2 = XmaxCanvas(mat);
 y1= 0;  y2 = YmaxCanvas(mat);

 oldx2 = x2;
 oldy1 = y1;
 oldy2 = y2;

 /* acha a posicao do inicio da primeira coluna */
 x1+=mat_wt(mat);
 for(j=mat_fc(mat);j<c1;j++)
   x1+=mat_w(mat)[j];

 /* acha a posicao do fim da ultima coluna */
 x2 = x1;
 for(;j<c2;j++)
  x2+=mat_w(mat)[j];
 x2 += (c2==mat_lc(mat)?mat_wlc(mat):mat_w(mat)[c2]);

 /* acha a posicao do inicio da primeira linha */
 y1+=mat_ht(mat);
 for(j=mat_fl(mat);j<l1;j++)
   y1+=mat_h(mat)[j];

 /* acha a posicao do final da ultima linha */
 y2 = y1;
 for(;j<l2;j++)
  y2+=mat_h(mat)[j];
 y2 += (l2==mat_ll(mat)?mat_hll(mat):mat_h(mat)[l2]);

 if((c2 == mat_ultimac(mat)) && (oldx2 > x2))
 {
   /* Se estou desenhando ate a ultima coluna e sobrou espaco a direita dela,
      apaga este espaco com a cor de fundo.
   */
   iupmatDrawEmptyArea(h,x2,oldx2,oldy1,oldy2);
 }

 if ((l2 == mat_ultimal(mat)) && (oldy2 > y2))
 {
   /* Se estou desenhando ate a ultima linha visivel, e sobrou espaco a baixo
      dela, apaga este espaco com a cor de fundo.
   */
   iupmatDrawEmptyArea(h,0,oldx2,y2,oldy2);
 }


 /***** Exibe os valores das celulas */
 xc1 = x1;
 yc1 = y1;

 iupGetColor(h, IUP_FRAMECOLOR, &r, &g, &b);
 framecolor = cdEncodeColor((unsigned char) r, (unsigned char) g, (unsigned char) b);
// cdCanvasForeground(mat->cddbuffer, framecolor);


 /* primeira linha vertical (inclui todas as linhas na regiao) */
// CdLine(xc1-1,y1,xc1-1,y2-1);
 for(j=c1;j<=c2;j++)  /* Para todas as colunas na regiao */
 {
  if (mat_w(mat)[j] == 0)
    continue;

  align = GetColAlignment(h,j+1);

  xc2 = xc1 + (j==mat_lc(mat)?mat_wlc(mat):mat_w(mat)[j]);

  /* primeira linha horizontal (somente desta coluna) */
  CdLine(xc1,yc1-1,xc2-1,yc1-1);
  for(i=l1;i<=l2;i++)   /* Para todas as linhas na regiao */
  {
   if (mat_h(mat)[i] == 0)
     continue;

   yc2 = yc1 + mat_h(mat)[i]-1;

   if (!(iupCheck(mat_edtdatah(mat), "VISIBLE")==YES && i==mat_lin(mat) && j==mat_col(mat)))
   {
     int drop = 0;
     int cor = COR_ELEM;

     char *str = iupmatGetCellValue(h,i,j);
     if (CallDropdownCheckCb(h,i,j))
       drop = BOXW;

     /* Se a celula esta marcada, quero desenha-la invertida */
     if (iupmatMarkCellGet(mat,i,j))
       cor = COR_REVERSE;

     DrawText(h, xc1, xc1+mat_w(mat)[j]-1-drop, yc1, yc2-1, str, align,xc2,cor,i+1,j+1);

     if (drop)
       DrawComboFeedback(h, xc1+mat_w(mat)[j]-1, yc1, yc2, i+1, j+1, cor);
   }

   if (mat->checkframecolor)
   {
     sprintf(str, "FRAMEHORIZCOLOR%d:%d", i, j);
     if (iupGetColor(h, str, &r, &g, &b))
       cdCanvasForeground(mat->cddbuffer, cdEncodeColor((unsigned char) r, (unsigned char) g, (unsigned char) b));
     else
       cdCanvasForeground(mat->cddbuffer, framecolor);
   }
   else
    cdCanvasForeground(mat->cddbuffer, framecolor);

   /* linha horizontal (somente desta coluna) */
   CdLine(xc1,yc2,xc2-1,yc2);


   if (mat->checkframecolor)
   {
     sprintf(str, "FRAMEVERTCOLOR%d:%d", i+1, j+1);
     if (iupGetColor(h, str, &r, &g, &b))
       cdCanvasForeground(mat->cddbuffer, cdEncodeColor((unsigned char) r, (unsigned char) g, (unsigned char) b));
     else
       cdCanvasForeground(mat->cddbuffer, framecolor);
   }
   else
    cdCanvasForeground(mat->cddbuffer, framecolor);

   /* linha vertical (somente desta linha) */
   CdLine(xc2-1,yc1,xc2-1,yc2-1);

   yc1  = yc2+1;
  }

  xc1 = xc2;
  yc1 = y1;
  /* linha vertical (inclui todas as linhas na regiao) */
//  CdLine(xc1-1,y1,xc1-1,y2-1);
 }
}

/*

%F Desenha a representacao de que uma celula tem o foco, para celulas que
   nao estao no modo de edicao.
%i h : handle da matriz,
   lin,col : Celula em questao.
   coloca : 1 se devemos colocar o desenho do foco, 0 se devemos retirar o
            desenho

*/
void iupmatDrawFocus(Ihandle *h, int lin, int col, int coloca)
{
  Tmat *mat=(Tmat*)matrix_data(h);
  int x1,y1,x2,y2,dx,dy,oldbgc;

  if (iupCheck(h, "HIDEFOCUS")==YES)
    return;

  if (!iupmatGetCellDim(h,lin,col,&x1,&y1,&dx,&dy))
    return;

  mat->redraw = 1;

  if (coloca)          /* Coloca o desenho do foco */
    oldbgc = SetFgColor(h,lin+1,col+1,iupmatMarkCellGet(mat,lin,col));
  else                 /* Retira o desenho do foco */
    oldbgc = SetBgColor(h,lin+1,col+1,COR_ELEM,iupmatMarkCellGet(mat,lin,col));

  x2 = x1 + dx - 1;
  y2 = y1 + dy - 1;

  CdRect(x1,x2,y1,y2);
  CdRect(x1+1,x2-1,y1+1,y2-1);
  
  CdRestoreBgColor();
}


/*
%F Retorna qual o atributo de cor de frente usado para desenhar uma
   determinada celula da matriz.

%i h       - Handle da matriz
   lin,col - Coordenadas da celula a ter sua cor consultada.  As coordenadas
             sao coordenadas IUP, ou seja 0,0 representa a celula entre os
             titulos, e 1,1 representa a celula da primeira linha, e primeira
             coluna da matriz.
%o Retorna uma string com o atributo de cor usadao para desenhar a celula.
*/
char* iupmatDrawGetFgColor(Ihandle *h, int lin, int col)
{
  static char buffer[30];
  unsigned int r=0,g=0,b=0;

  if (callFgColorCB(h, lin, col, &r, &g, &b) == IUP_IGNORE)
    matDrawGetColor(h, "FGCOLOR", lin, col, &r, &g, &b, 0);

  sprintf(buffer, "%d %d %d", r, g, b);
  return buffer;
}

/*
%F Retorna qual o atributo de cor de fundo usado para desenhar uma
   determinada celula da matriz.

%i h       - Handle da matriz
   lin,col - Coordenadas da celula a ter sua cor consultada.  As coordenadas
             sao coordenadas IUP, ou seja 0,0 representa a celula entre os
             titulos, e 1,1 representa a celula da primeira linha, e primeira
             coluna da matriz.
%o Retorna uma string com o atributo de cor usadao para desenhar a celula.
*/
char* iupmatDrawGetBgColor(Ihandle *h, int lin, int col)
{
  static char buffer[30];
  unsigned int r=255,g=255,b=255;
  int parent = 0;

  if (lin == 0 || col == 0)
    parent = 1;

  if (callBgColorCB(h, lin, col, &r, &g, &b) == IUP_IGNORE)
    matDrawGetColor(h, "BGCOLOR", lin, col, &r, &g, &b, parent);

  sprintf(buffer, "%d %d %d", r, g, b);
  return buffer;
}

