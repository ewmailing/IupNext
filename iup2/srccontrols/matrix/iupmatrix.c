/** \file
 * \brief iupmatrix control core
 *
 * See Copyright Notice in iup.h
 *  */

#include <iup.h>
#include <iupcpi.h>
#include <iupkey.h>
#include <cd.h>
#include <cddbuf.h>

#include <stdio.h>  /*sprintf*/
#include <string.h>
#include <stdlib.h>
#include <math.h>   /*ceil*/
#include <ctype.h>
#include <limits.h>
#include <stdarg.h>


#include "matridef.h"

#include "iupmatrix.h"
#include "matrixcd.h"
#include "imgetset.h"
#include "imdraw.h"
#include "imscroll.h"
#include "imaux.h"
#include "immem.h"
#include "immouse.h"
#include "imfocus.h"
#include "imkey.h"
#include "imnumlc.h"
#include "imcolres.h"
#include "immark.h"
#include "imedit.h"
#include "iupcdutil.h"
#include "icontrols.h"

/* Definicao do tipo Matrix */
static Ihandle*  matrixcreate    (Iclass* self, Ihandle **params);
static void  matrixmap           (Ihandle* self, Ihandle* parent);
static void  matrixdestroy       (Ihandle* self);
static int   matrixsetnaturalsize(Ihandle *self);
static void  matrixsetattr       (Ihandle*, char* attr, char* value);
static char* matrixgetattr       (Ihandle*, char* attr);
static char* matrixgetdefaultattr(Ihandle*, char* attr);


/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

/*

%F Set a estrutura de dados que descreve a matriz (Tmat)
%i handle da matriz
%i ponteiro da estrutura de dados

*/
static void matrixsetdata(Ihandle* self,Tmat* mat)
{
   iupSetEnv(self,"_IUPMAT_DATA",(char*)mat);
}

/*

%F Recupera a estrutura de dados que descreve a matriz
%i handle da matriz
%o ponteiro da estrutura de dados

*/
Tmat* iupmatMatrixGetData(Ihandle* self)
{
   return (Tmat*) iupGetEnv(self,"_IUPMAT_DATA");
}


static int iupGetEnvInt2(Ihandle* h, char* attrib)
{
  char* v = iupGetEnv(h, attrib);
  if (v)
    return atoi(v);
  else
  {
    /* default values that must be returned only here */
    if (iupStrEqual(attrib,IUP_NUMLIN_VISIBLE))
      return 3;
    if (iupStrEqual(attrib,IUP_NUMCOL_VISIBLE))
      return 4;
    return 0;
  }
}

/*

%F verifica quais as barra de rolamento que foram definidas
%i handle da matriz
%o sbv: =0 nao tem barra de rolamento vertical
        =1     tem barra de rolamento vertical
   sbh: =0 nao tem barra de rolamento horizontal
        =1     tem barra de rolamento horizontal

*/
static void matrixgetsb(Ihandle* self, int *sbh, int *sbv)
{
 char *sb=IupGetAttribute(self,IUP_SCROLLBAR);
 if (sb == NULL) sb = IUP_YES;

 *sbh=*sbv=0;

 if (iupStrEqual (sb, IUP_YES))
  *sbh=*sbv=1;
 else if (iupStrEqual (sb, IUP_ON))
  *sbh=*sbv=1;
 else if (iupStrEqual (sb, IUP_VERTICAL))
  *sbv=1;
 else if (iupStrEqual (sb, IUP_HORIZONTAL))
  *sbh=1;
}

/*
%F Calcula tamanho da matriz.
%i handle da matriz
*/
static void MatrixSize(Ihandle *hm)
{
  Tmat *mat=(Tmat*)matrix_data(hm);
  int w, h;

  cdCanvasGetSize(mat->cddbuffer,&w,&h,NULL,NULL);

  /* Area util da matrix 'e o tamanho corrente menos a area de titulos*/
  mat_sx(mat)= w - mat_wt(mat);
  mat_sy(mat)= h - mat_ht(mat);

  XmaxCanvas(mat) = w - 1;
  YmaxCanvas(mat) = h - 1;
}

/*

%F Pega os valores iniciais das celulas da matriz, aqueles que sao definidos
   apenas por atributos.
%i h : handle da matriz.

*/
static void PegaValoresIniciais(Ihandle *h)
{
  Tmat *mat = (Tmat*)matrix_data(h);
  int i,j;
  char *valor, attr[100];

  if(mat->valcb)
    return;

  for (i=0;i<mat_nl(mat);i++)
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

%F Inicializa os dados da matriz
%i handle da matriz

*/
static void AllocMatrix(Tmat *mat)
{
  Ihandle* h = mat->self;

  /* Verifica se a callback de value esta definida */
  mat->valcb = (sIFnii)IupGetCallback(h,IUP_VALUE_CB);
  mat->valeditcb = (IFniis)IupGetCallback(h,IUP_VALUE_EDIT_CB);
  mat->markcb = (IFnii)IupGetCallback(h,"MARK_CB");
  mat->markeditcb = (IFniii)IupGetCallback(h,"MARKEDIT_CB");

  /* Pega numero de linhas e colunas */
  mat_nl(mat) = iupGetEnvInt(h,IUP_NUMLIN);
  mat_nc(mat) = iupGetEnvInt(h,IUP_NUMCOL);

  iupmatMemAloc(mat);

  mat_fc(mat) = 0;
  mat_fl(mat) = 0;
  mat_posx(mat) = 0;
  mat_posy(mat) = 0;

  mat_col(mat) = 0;
  mat_lin(mat) = 0;

  mat_markLC(mat) = -1;

  iupmatGetLastWidth(h,MAT_COL);
  iupmatGetLastWidth(h,MAT_LIN);

  PegaValoresIniciais(h);

  /* default is text */
  mat_edtdatah(mat) = mat_edttexth(mat);
}

static void InitMatrix(Tmat *mat)
{
  Ihandle* h = mat->self;

  /* calcula tamanho dos titulos */
  mat_wt(mat) = iupmatGetTitlelineSize(h);
  mat_ht(mat) = iupmatGetTitlecolumnSize(h);

  MatrixSize(h);

  iupmatFillWidthVec(h);
  iupmatFillHeightVec(h);

  iupmatMarkSet(h, iupGetEnv(h, "MARKED"));
}


/**************************************************************************
***************************************************************************
*
*   Callbacks cadastradas para o canvas
*
***************************************************************************
***************************************************************************/

static void mat_createcanvas(Tmat *mat)
{
  if (!mat->cdcanvas)
    mat->cdcanvas = cdCreateCanvas(CD_IUP, mat_self(mat));

  if (mat->cdcanvas)
  {
    cdCanvasActivate(mat->cdcanvas);
    mat->cddbuffer = cdCreateCanvas(CD_DBUFFER, mat->cdcanvas);
  }

  if (mat->cddbuffer)
    InitMatrix(mat);
}

/*
%F Callback chamada quando matriz tem seu tamanho alterado

%i hm : handle da matrix.
   dx,dy : Tamanho atual do canvas, em pixels.
%o Retorna IUP_DEFAULT.
*/
static int cbResize(Ihandle *hm, int dx, int dy)
{
  Tmat *mat = (Tmat*)matrix_data(hm);

  if (!mat->cddbuffer)
    mat_createcanvas(mat);

  if (!mat->cddbuffer)
    return IUP_DEFAULT;

  /* just update internal canvas properties,
     so we do not need to reactivate the same canvas. */
  cdCanvasActivate(cdcv(mat));

  XmaxCanvas(mat) = dx-1;
  YmaxCanvas(mat) = dy-1;

  /* Guarda o tamanho da area util da matriz, que e' o tamanho do canvas,
     descontando-se a area dos titulos de linha/coluna
  */
  mat_sx(mat)= dx - mat_wt(mat);
  mat_sy(mat)= dy - mat_ht(mat);

  iupmatGetLastWidth(hm,MAT_COL);
  iupmatGetLastWidth(hm,MAT_LIN);

  iupmatEditCheckHidden(hm);

  return IUP_DEFAULT;
}

/*

%F Callback chamada quando a matriz precisa ser redesenhado
%i hm : handle da matriz.
%o Retorna IUP_DEFAULT.

*/
static int cbRepaint(Ihandle *hm)
{
  Tmat *mat=(Tmat*)matrix_data(hm);

  if (!mat->cddbuffer)
    mat_createcanvas(mat);

  if (!mat->cddbuffer)
    return IUP_DEFAULT;

  MatrixSize(hm);

  SetSbV;
  SetSbH;

  iupmatDrawMatrix(hm,DRAW_ALL);
  iupmatShowFocus(hm);

  cdCanvasFlush(mat->cddbuffer); mat->redraw = 0;  /* flush always here */

  return IUP_DEFAULT;
}


/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
Ihandle* IupMatrix(char* action)
{
   return IupCreatep("matrix", (void*) action, NULL);
}

static Ihandle* matrixcreate(Iclass* ic, Ihandle **params)
{
   Tmat* mat = (Tmat*)calloc(1,sizeof(Tmat));
   Ihandle* self = IupCanvas(NULL);
   char *action = params? (char*) params[0] : NULL;
  (void)ic;

   matrixsetdata(self,mat);
   mat_self(mat) = self;
   iupSetEnv(self, "ACTION_CB", action);

   /* default da matrix e' com scrollbar, diferente do canvas */
   iupSetEnv(self,IUP_SCROLLBAR,IUP_YES);

   IupSetCallback(self,"ACTION",(Icallback)cbRepaint);
   IupSetCallback(self,"RESIZE_CB",(Icallback)cbResize);
   IupSetCallback(self,"BUTTON_CB",(Icallback)iupmatMouseButtonCb);
   IupSetCallback(self,"MOTION_CB",(Icallback)iupmatMouseMoveCb);
   IupSetCallback(self,"KEYPRESS_CB",(Icallback)iupmatKeyPressCb);
   IupSetCallback(self,"FOCUS_CB",(Icallback)iupmatFocusCb);
   IupSetCallback(self,"SCROLL_CB",(Icallback)iupmatScrollCb);

   iupSetEnv(self,IUP_BORDER,IUP_NO);

   /* Cria os campos de edicao */
   iupmatEditCreate(mat);

   iupSetEnv(self, IUP_FRAMECOLOR, "100 100 100");

   /* Matrix internal cursor */
   iupSetEnv(self, "Iup_matrix_cursor", "IupMatrixCrossCursor");

   return self;
}

/*

%F Destroi o elemento de interface, liberando memoria
%i self : Handle da matriz.

*/
static void matrixdestroy (Ihandle* self)
{
 Ihandle* itext = NULL;
 Ihandle* idrop = NULL;

 Tmat *mat=(Tmat*)matrix_data(self);
 itext = mat_edttexth(mat);
 idrop = mat_edtdroph(mat);
 
 if (mat->cddbuffer) cdKillCanvas(mat->cddbuffer);
 if (mat->cdcanvas) cdKillCanvas(mat->cdcanvas);

 if (mat_v(mat))
 {
   int lin,col;
   for (lin=0;lin<mat_nla(mat);lin++)
   {
     for (col=0;col<mat_nca(mat);col++)
       if (mat_v(mat)[lin][col].value) free (mat_v(mat)[lin][col].value);
     free (mat_v(mat)[lin]);
   }
   free (mat_v(mat));
 }

 if (mat_w(mat)) free (mat_w(mat));
 if (mat_h(mat)) free (mat_h(mat));

 if (mat_colmarked(mat)) free (mat_colmarked(mat));
 if (mat_linmarked(mat)) free (mat_linmarked(mat));
 
 if (mat_lininactive(mat)) 
 { 
   free (mat_lininactive(mat)); 
   mat_lininactive(mat) = NULL; 
 }
 if (mat_colinactive(mat)) 
 { 
   free (mat_colinactive(mat)); 
   mat_colinactive(mat) = NULL; 
 }
 
 free(mat);

 IupDestroy(itext);
 IupDestroy(idrop);          

 matrixsetdata(self, NULL);
}

void IupMatrixClose(void)
{
  IupDestroy(IupGetHandle("IupMatrixCrossCursor"));
}

static void matrixmap (Ihandle* self, Ihandle* parent)
{
  Tmat* mat = (Tmat*)matrix_data(self);

  /* Faz o mapeamento do CANVAS */
  iupCpiDefaultMap(self,parent);

  /* mapeia os campos de edicao */
  iupmatEditMap(mat, self);

  AllocMatrix(mat);
}

static int matGetNaturalWidth(Ihandle *self)
{
  int titlesize, colsize;
  int numcolv, i;

  titlesize = iupmatGetTitlelineSize(self);

  numcolv = iupGetEnvInt2(self,IUP_NUMCOL_VISIBLE);
  colsize = 0;
  for(i=0;i<numcolv;i++)
    colsize += iupmatGetColumnWidth(self,i);

  return colsize + titlesize;
}

static int matGetNaturalHeight(Ihandle *self)
{
  int titlesize, linsize;
  int numlinv,i;

  titlesize = iupmatGetTitlecolumnSize(self);

  numlinv = iupGetEnvInt2(self,IUP_NUMLIN_VISIBLE);
  linsize = 0;
  for(i=0;i<numlinv;i++)
    linsize += iupmatGetLineHeight(self,i);

  return linsize + titlesize;
}

static int matrixsetnaturalsize(Ihandle *self)
{
  Tmat* mat = (Tmat*)matrix_data(self);
  int  width, height, sizex, sizey;
  int  sbh, sbv, sbw=0;

  /* Pega parametros da scrollbar */
  matrixgetsb(self,&sbh, &sbv);
  if (sbv || sbh)
    sbw = iupdrvGetScrollSize();


  /* Para as funcoes matGetNaturalWidth e matGetNaturalHeight serem capazes
     de chamar as funcoes de calculo de larguras, os valores de mat_nl(mat),
     mat_nc(mat) e mat->valcb tem de estar definidos... Se o canvas ainda nao foi
     criado, entao estes valores ainda nao foram inicializados...
  */
  if(!mat->cddbuffer)
  {
    mat_nl(mat) = iupGetEnvInt(self,IUP_NUMLIN);
    mat_nc(mat) = iupGetEnvInt(self,IUP_NUMCOL);
    mat->valcb = (sIFnii)IupGetCallback(self,IUP_VALUE_CB);
  }

  /* Pega  valor associado ao atributo SIZE */
  iupGetSize(self, &sizex, &sizey);

  /* Pega a largura natural */
  if(sizex)
    width = sizex;
  else
    width = matGetNaturalWidth(self) + sbw*sbv;

  /* Pega a altura natural */
  if(sizey)
    height = sizey;
  else
    height = matGetNaturalHeight(self) + sbw*sbh;

  iupSetNaturalWidth (self, width);
  iupSetNaturalHeight(self, height);

  return 3; /* Pode dar resize em todas as direcoes */
}


static void matrixsetattr(Ihandle* self, char* attr, char* value)
{
   Tmat* mat;
   int lin = 0,col = 0,ret, old_redraw;
   if (!attr) return;

   if (iupStrEqual(attr,IUP_CURSOR))
   {
     if(value)
       iupStoreEnv (self, "Iup_matrix_cursor", value);
     else
       iupSetEnv(self, "Iup_matrix_cursor", "IupMatrixCrossCursor");
   }

   mat = (Tmat*)matrix_data(self);
   if (!mat)
   {
    iupCpiDefaultSetAttr(self,attr,value);
    return;
   }

   if (iupStrEqual(attr,"CHECKFRAMECOLOR"))
   {
     if (iupStrEqualNoCase(value, "YES"))
       mat->checkframecolor = 1;
     else
       mat->checkframecolor = 0;
     return;
   }

   if (self == NULL || IupGetAttribute(self,IUP_WID) == NULL || attr == NULL)
   {
    iupCpiDefaultSetAttr(self,attr,value);
    return;
   }

   old_redraw = mat->redraw;

   if (iupStrEqual(attr,IUP_VALUE))
   {
     if (iupCheck(mat_edtdatah(mat), "VISIBLE")==YES)
       IupStoreAttribute(mat_edtdatah(mat),IUP_VALUE, value);
     else
       iupmatSetCell(self,mat_lin(mat)+1,mat_col(mat)+1,value);
   }
   else if (iupStrEqual(attr,IUP_CARET))
   {
     IupStoreAttribute(mat_edttexth(mat),IUP_CARET,value);
     iupStoreEnv(self, IUP_CARET, value);
   }
   else if (iupStrEqual(attr,IUP_SELECTION))
   {
     IupStoreAttribute(mat_edttexth(mat),IUP_SELECTION,value);
     iupStoreEnv(self, IUP_SELECTION, value);
   }
   else if (iupStrEqual(attr,IUP_ADDLIN))
      iupmatNlcAddLin(self,value);
   else if (iupStrEqual(attr,IUP_DELLIN))
      iupmatNlcDelLin(self,value);
   else if (iupStrEqual(attr,IUP_ADDCOL))
      iupmatNlcAddCol(self,value);
   else if (iupStrEqual(attr,IUP_DELCOL))
      iupmatNlcDelCol(self,value);
   else if (iupStrEqual(attr,IUP_NUMLIN))
      iupmatNlcNumLin(self,value);
   else if (iupStrEqual(attr,IUP_NUMCOL))
      iupmatNlcNumCol(self,value);
   else if (iupStrEqual(attr,IUP_MARKED))
      iupmatMarkSet(self,value);
   else if (iupStrEqual(attr,IUP_MARK_MODE) || iupStrEqual(attr,IUP_AREA) ||
            iupStrEqual(attr,IUP_MULTIPLE))
      iupmatMarkSetMode(self,value);
   else if (iupStrEqual(attr,IUP_FOCUS_CELL))
      iupmatSetFocusPosition(self,value, 0);
   else if (iupStrEqual(attr,IUP_ORIGIN))
      iupmatSetOrigin(self,value);
   else if (iupStrEqual(attr,IUP_EDIT_MODE))
   {
     if (iupStrEqualNoCase(value, IUP_YES)||iupStrEqualNoCase(value, IUP_ON))
       iupmatEditShow(self);
     else
       iupmatEditClose(self);
   }
   else if (iupStrEqual(attr,IUP_ACTIVE))
   {
      iupCpiDefaultSetAttr(self,attr,value);
      iupmatSetRedraw(self,"ALL");
   }
   else if (iupStrEqual(attr,IUP_REDRAW))
     iupmatSetRedraw(self,value);
   else if (iupStrEqualPartial(attr,"WIDTH"))
   {
     if (sscanf(attr+strlen("WIDTH"),"%d",&col)==1)
     {
       iupmatColresSet(self, value, col, MAT_COL, 0);
       iupmatSetRedraw(self,"ALL");
     }
   }
   else if (iupStrEqualPartial(attr,"HEIGHT"))
   {
     if (sscanf(attr+strlen("HEIGHT"),"%d",&lin)==1)
     {
       iupmatColresSet(self, value, lin, MAT_LIN, 0);
       iupmatSetRedraw(self,"ALL");
     }
   }
   else if (iupStrEqualPartial(attr,"RASTERWIDTH"))
   {
     if (sscanf(attr+strlen("RASTERWIDTH"),"%d",&col)==1)
     {
       iupmatColresSet(self, value, col, MAT_COL, 1);
       iupmatSetRedraw(self,"ALL");
     }
   }
   else if (iupStrEqualPartial(attr,"RASTERHEIGHT"))
   {
     if (sscanf(attr+strlen("RASTERHEIGHT"),"%d",&lin)==1)
     {
       iupmatColresSet(self, value, lin, MAT_LIN, 1);
       iupmatSetRedraw(self,"ALL");
     }
   }
   else if (iupStrEqualPartial(attr,"ALIGNMENT"))
   {
     if (sscanf(attr+strlen("ALIGNMENT"),"%d",&col)==1)
     {
       iupmatSetColAlign(self, col);
       mat->redraw = 0; /* leave redraw to REDRAW attribute */
     }
   }
   else if (iupStrEqualPartial(attr,"FONT"))
   {
      lin = -1; col = -1;
      iupStrToIntInt(attr+strlen("FONT"),&lin,&col,':');
      if (lin != -1 || col != -1)
      {
        iupmatSetRedrawCell(self,lin,col);
        mat->redraw = 0; /* leave redraw to REDRAW attribute */
      }
      else
        iupmatSetRedraw(self,"ALL");
   }
   else if (iupStrEqualPartial(attr,"FGCOLOR"))
   {
      lin = -1; col = -1;
      iupStrToIntInt(attr+strlen("FGCOLOR"),&lin,&col,':');
      if (lin != -1 || col != -1)
      {
        iupmatSetRedrawCell(self,lin,col);
        mat->redraw = 0; /* leave redraw to REDRAW attribute */
      }
      else
        iupmatSetRedraw(self,"ALL");
   }
   else if (iupStrEqualPartial(attr,"BGCOLOR"))
   {
      lin = -1; col = -1;
      iupStrToIntInt(attr+strlen("BGCOLOR"),&lin,&col,':');
      if (lin != -1 || col != -1)
      {
        iupmatSetRedrawCell(self,lin,col);
        mat->redraw = 0; /* leave redraw to REDRAW attribute */
      }
      else
        iupmatSetRedraw(self,"ALL");
   }
   else if (iupStrEqualPartial(attr,"ACTIVELIN"))  /* ACTIVE - OLD STUFF */
   {
     if (sscanf(attr+strlen("ACTIVELIN"),"%d",&lin)==1)
        iupmatSetActive(self,MAT_LIN,lin,value);
   }
   else if (iupStrEqualPartial(attr,"ACTIVECOL"))  /* ACTIVE - OLD STUFF */
   {
     if (sscanf(attr+strlen("ACTIVECOL"),"%d",&col)==1)
        iupmatSetActive(self,MAT_COL,col,value);
   }
   else
   {
      ret = iupStrToIntInt(attr,&lin,&col,':');

      if ((ret == 2) && (col == 0))       /* matrix line titles */
         iupmatSetTitleLine(self, lin);
      else if ((ret == 2) && (lin == 0))  /* matrix column titles */
         iupmatSetTitleColumn(self, col);
      else if (ret == 2)                  /* matrix cell */
         iupmatSetCell(self,lin,col,value);

      mat->redraw = 0; /* leave redraw to REDRAW attribute */
   }

   iupCpiDefaultSetAttr(self,attr,value);

   if (mat->redraw) { cdCanvasFlush(cdcv(mat)); mat->redraw = 0; }
   else if (old_redraw) mat->redraw = old_redraw;
}

static char* matrixgetattr(Ihandle* self, char* attr)
{
  int lin,col;

  if (self == NULL || iupCpiDefaultGetAttr(self,IUP_WID) == NULL || attr == NULL)
      return NULL;

  if (iupStrToIntInt(attr,&lin,&col,':') == 2) /*matrix cell */
    return iupmatGetCell(self,lin,col);
  if (iupStrEqual(attr,IUP_VALUE))
  {
    Tmat* mat = (Tmat*)matrix_data(self);
    if (iupCheck(mat_edtdatah(mat), "VISIBLE")==YES)
      return iupmatEditGetValue(mat);
    else
      return iupmatGetCell(self,mat_lin(mat)+1,mat_col(mat)+1);
  }
  if (iupStrEqual(attr,IUP_CARET))
  {
    Tmat* mat = (Tmat*)matrix_data(self);
    return IupGetAttribute(mat_edttexth(mat),IUP_CARET);
  }
  if (iupStrEqual(attr,IUP_SELECTION))
  {
    Tmat* mat = (Tmat*)matrix_data(self);
    return IupGetAttribute(mat_edttexth(mat),IUP_SELECTION);
  }
  if (iupStrEqual(attr,IUP_NUMLIN))
     return iupmatNlcGetNumLin(self);
  if (iupStrEqual(attr,IUP_NUMCOL))
     return iupmatNlcGetNumCol(self);
  if (iupStrEqual(attr,IUP_EDIT_MODE))
  {
    if (iupmatEditIsVisible(self))
      return "YES";
    else
      return "NO";
  }
  else if (iupStrEqual(attr,"CHECKFRAMECOLOR"))
  {
    Tmat* mat = (Tmat*)matrix_data(self);
    if (mat->checkframecolor)
     return "YES";
   else
     return "NO";
  }
  if (iupStrEqual(attr,IUP_NUMCOL_VISIBLE))
  {
    static char buffer[50];
    Tmat* mat = (Tmat*)matrix_data(self);
    sprintf(buffer, "%d", mat_lc(mat) - mat_fc(mat));
    return buffer;
  }
  if (iupStrEqual(attr,IUP_NUMLIN_VISIBLE))
  {
    static char buffer[50];
    Tmat* mat = (Tmat*)matrix_data(self);
    sprintf(buffer, "%d", mat_ll(mat) - mat_fl(mat));
    return buffer;
  }
  if (iupStrEqual(attr,IUP_MARKED))
     return iupmatMarkGet(self);
  if (iupStrEqual(attr,IUP_FOCUS_CELL))
     return iupmatGetFocusPosition(self);
  if (iupStrEqual(attr,IUP_ORIGIN))
     return iupmatGetOrigin(self);
  if (iupStrEqualPartial(attr,"WIDTH"))
  {
     if (sscanf(attr+strlen("WIDTH"),"%d",&col)==1)
       return iupmatColresGet(self,col,MAT_COL, 0);
  }
  if (iupStrEqualPartial(attr,"HEIGHT"))
  {
     if (sscanf(attr+strlen("HEIGHT"),"%d",&lin)==1)
       return iupmatColresGet(self,lin,MAT_LIN, 0);
  }
  if (iupStrEqualPartial(attr,"RASTERWIDTH"))
  {
     if (sscanf(attr+strlen("RASTERWIDTH"),"%d",&col)==1)
       return iupmatColresGet(self,col,MAT_COL, 1);
  }
  if (iupStrEqualPartial(attr,"RASTERHEIGHT"))
  {
     if (sscanf(attr+strlen("RASTERHEIGHT"),"%d",&lin)==1)
       return iupmatColresGet(self,lin,MAT_LIN, 1);
  }
  if (iupStrEqualPartial(attr,"FGCOLOR"))
   {
      lin = -1; col = -1;
      iupStrToIntInt(attr+strlen("FGCOLOR"),&lin,&col,':');
      if (lin != -1 || col != -1)
        return iupmatDrawGetFgColor(self,lin,col);
   }
  if (iupStrEqualPartial(attr,"BGCOLOR"))
   {
      lin = -1; col = -1;
      iupStrToIntInt(attr+strlen("BGCOLOR"),&lin,&col,':');
      if (lin != -1 || col != -1)
        return iupmatDrawGetBgColor(self,lin,col);
   }
  if (iupStrEqualPartial(attr,"FONT"))
   {
      lin = -1; col = -1;
      iupStrToIntInt(attr+strlen("FONT"),&lin,&col,':');
      if (lin != -1 || col != -1)
        return iupmatDrawGetFont(self,lin,col);
   }

  return iupCpiDefaultGetAttr(self,attr);
}


static char* matrixgetdefaultattr(Ihandle* self, char* attr)
{
   int col;
   if (self == NULL || attr == NULL)
      return NULL;
   else if (iupStrEqual (attr, IUP_AREA))
      return "CONTINUOUS";
   else if (iupStrEqual (attr, IUP_MARK_MODE))
      return IUP_NO;
   else if (sscanf (attr,"ALIGNMENT%d",&col)==1)
      return col == 0 ? IUP_ALEFT : IUP_ACENTER;
   else if (iupStrEqual(attr,IUP_WIDTHDEF))
     return "80";  /* 20 caracteres */
   else if (iupStrEqual(attr,IUP_HEIGHTDEF))
     return "8";  /* 1 caracter */
   else if (iupStrEqual(attr,IUP_RESIZEMATRIX))
      return IUP_NO;
   else
      return iupCpiDefaultGetDefaultAttr(self,attr);
}

void IupMatrixOpen(void)
{
  unsigned char matrx_img_cur_excel[15*15] = 
  {
    0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,
    0,0,0,0,1,2,2,2,2,1,1,0,0,0,0,
    0,0,0,0,1,2,2,2,2,1,1,0,0,0,0,
    0,0,0,0,1,2,2,2,2,1,1,0,0,0,0,
    1,1,1,1,1,2,2,2,2,1,1,1,1,1,0,
    1,2,2,2,2,2,2,2,2,2,2,2,2,1,1,
    1,2,2,2,2,2,2,2,2,2,2,2,2,1,1,
    1,2,2,2,2,2,2,2,2,2,2,2,2,1,1,
    1,1,1,1,1,2,2,2,2,1,1,1,1,1,1,
    0,1,1,1,1,2,2,2,2,1,1,1,1,1,1,
    0,0,0,0,1,2,2,2,2,1,1,0,0,0,0,
    0,0,0,0,1,2,2,2,2,1,1,0,0,0,0,
    0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,
    0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
  };

  Ihandle *imgcursor;
  Iclass* ICMatrix = iupCpiCreateNewClass("matrix" ,"n");

  iupCpiSetClassMethod(ICMatrix, ICPI_SETNATURALSIZE, (Imethod) matrixsetnaturalsize);
  iupCpiSetClassMethod(ICMatrix, ICPI_CREATE,         (Imethod) matrixcreate);
  iupCpiSetClassMethod(ICMatrix, ICPI_DESTROY,        (Imethod) matrixdestroy);
  iupCpiSetClassMethod(ICMatrix, ICPI_MAP,            (Imethod) matrixmap);
  iupCpiSetClassMethod(ICMatrix, ICPI_SETATTR,        (Imethod) matrixsetattr);
  iupCpiSetClassMethod(ICMatrix, ICPI_GETATTR,        (Imethod) matrixgetattr);
  iupCpiSetClassMethod(ICMatrix, ICPI_GETDEFAULTATTR, (Imethod) matrixgetdefaultattr);

  imgcursor = IupImage(15,15, matrx_img_cur_excel);
  IupSetAttribute (imgcursor, "0", "BGCOLOR"); 
  IupSetAttribute (imgcursor, "1", "0 0 0"); 
  IupSetAttribute (imgcursor, "2", "255 255 255"); 
  IupSetAttribute (imgcursor, IUP_HOTSPOT, "7:7"); /* Centered Hotspot    */
  IupSetHandle ("IupMatrixCrossCursor", imgcursor); 
  IupSetHandle ("matrx_img_cur_excel", imgcursor); /* for backward compatibility */
}

void IupMatSetAttribute(Ihandle *n, const char* a, int l, int c, char* v)
{
  char attr[100];
  sprintf(attr, "%s%d:%d", a, l, c);
  IupSetAttribute(n, attr, v);
}

void IupMatStoreAttribute(Ihandle *n, const char* a, int l, int c, char* v)
{
  char attr[100];
  sprintf(attr, "%s%d:%d", a, l, c);
  IupStoreAttribute(n, attr, v);
}

char* IupMatGetAttribute(Ihandle *n, const char* a, int l, int c)
{
  char attr[100];
  sprintf(attr, "%s%d:%d", a, l, c);
  return IupGetAttribute(n, attr);
}

int IupMatGetInt(Ihandle *n, const char* a, int l, int c)
{
  char attr[100];
  sprintf(attr, "%s%d:%d", a, l, c);
  return IupGetInt(n, attr);
}

float IupMatGetFloat(Ihandle *n, const char* a, int l, int c)
{
  char attr[100];
  sprintf(attr, "%s%d:%d", a, l, c);
  return IupGetFloat(n, attr);
}

void IupMatSetfAttribute(Ihandle *n, const char* a, int l, int c, const char* f, ...)
{
  static char v[SHRT_MAX];
  char attr[100];
  va_list arglist;
  sprintf(attr, "%s%d:%d", a, l, c);
  va_start(arglist, f);
  vsprintf(v, f, arglist);
  va_end(arglist);
  IupStoreAttribute(n, attr, v);
}
