/** \file
 * \brief iupmatrix control
 * keyboard control
 *
 * See Copyright Notice in iup.h
 *  */

/**************************************************************************
***************************************************************************
*
*   Funcoes usadas para controle de teclas na matriz e no texto de edicao.
*
***************************************************************************
***************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <iup.h>
#include <iupkey.h>
#include <iupcpi.h>
#include <cd.h>

#include "iupmatrix.h"
#include "matridef.h"
#include "matrixcd.h"
#include "imdraw.h"
#include "imscroll.h"
#include "imfocus.h"
#include "imaux.h"
#include "imgetset.h"
#include "imkey.h"
#include "immark.h"
#include "imedit.h"


/* keys that are not chars */
#define MAT_ACTIONKEY(c) \
  ((c) == K_LEFT  || (c) == K_sLEFT  || (c) == K_cLEFT  || \
   (c) == K_RIGHT || (c) == K_sRIGHT || (c) == K_cRIGHT || \
   (c) == K_UP    || (c) == K_sUP    || (c) == K_cUP    || \
   (c) == K_DOWN  || (c) == K_sDOWN  || (c) == K_cDOWN  || \
   (c) == K_HOME  || (c) == K_sHOME  || (c) == K_cHOME  || \
   (c) == K_END   || (c) == K_sEND   || (c) == K_cEND   || \
   (c) == K_ESC   || (c) == K_CR     || (c) == K_sCR    || \
   (c) == K_SP    || (c) == K_DEL    || (c) == K_sDEL   || \
   (c) == K_TAB   || (c) == K_sTAB   || \
   (c) == K_PGUP  || (c) == K_PGDN   || \
   (c) == K_sPGUP || (c) == K_sPGDN)

typedef int (*ImatrixActioncb)  (Ihandle *, int, int, int, int, char*);

/* Quantas vezes as teclas home e end foram pressionadas */
static int homekeycount=0, endkeycount=0;


/**************************************************************************
***************************************************************************
*
*   Funcoes internas
*
***************************************************************************
***************************************************************************/

/*
%F Funcao que faz o tratamento de todas as teclas da matriz.
%i h : handle da matriz,
   c : tecla pressionada.
%o Retorna IUP_IGNORE se soube tratar a tecla, caso contrario retorna
   a propria tecla.
*/
int iupmatKey(Ihandle *h, int c)
{
  int ret = IUP_IGNORE;
  Tmat *mat = (Tmat*)matrix_data(h);

  /* Desliga marcacao das celulas se a tecla nao for tab/shift-tab/del */
  if(c!=K_TAB && c!=K_sTAB && c!=K_DEL && c!=K_sDEL)
    iupmatMarkHide(h,0);

  /* Se o foco nao estiver visivel, ocorre um scroll para que fique visivel */
  if (!iupmatIsFocusVisible(h))
  {
    /* this will end edition mode */
    ScrollPosVer(h,(float)mat_lin(mat)/mat_nl(mat));
    ScrollPosHor(h,(float)mat_col(mat)/mat_nc(mat));
  }

  switch (c)
  {
  case K_CR+1000:   /* used by the iupmatTextActionCb */
    if (iupmatCallLeavecellCb(h) == IUP_IGNORE) break;
    ScrollKeyCr(h);
    iupmatCallEntercellCb(h);
    break;

  case K_cHOME:
  case K_sHOME:
  case K_HOME:
    if (iupmatCallLeavecellCb(h) == IUP_IGNORE) break;
    ScrollKeyHome(h);
    homekeycount++;
    iupmatCallEntercellCb(h);
    break;

  case K_cEND:
  case K_sEND:
  case K_END:
    if (iupmatCallLeavecellCb(h) == IUP_IGNORE) break;
    ScrollKeyEnd(h);
    endkeycount++;
    iupmatCallEntercellCb(h);
    break;

  case K_sTAB:
  case K_TAB:
    return IUP_CONTINUE;

  case K_cLEFT:
  case K_sLEFT:
  case K_LEFT:
    if(iupmatCallLeavecellCb(h) == IUP_IGNORE) break;
    ScrollKeyLeft(h);
    iupmatCallEntercellCb(h);
    break;

  case K_cRIGHT:
  case K_sRIGHT:
  case K_RIGHT:
    if(iupmatCallLeavecellCb(h) == IUP_IGNORE) break;
    ScrollKeyRight(h);
    iupmatCallEntercellCb(h);
    break;

  case K_cUP:
  case K_sUP:
  case K_UP:
    if (iupmatCallLeavecellCb(h) == IUP_IGNORE) break;
    ScrollKeyUp(h);
    iupmatCallEntercellCb(h);
    break ;

  case K_cDOWN:
  case K_sDOWN:
  case K_DOWN:
    if (iupmatCallLeavecellCb(h) == IUP_IGNORE) break;
    ScrollKeyDown(h);
    iupmatCallEntercellCb(h);
    break;

  case K_sPGUP:
  case K_PGUP:
    if (iupmatCallLeavecellCb(h) == IUP_IGNORE) break;
    ScrollKeyPgUp(h);
    iupmatCallEntercellCb(h);
    break;

  case K_sPGDN:
  case K_PGDN:
    if (iupmatCallLeavecellCb(h) == IUP_IGNORE) break;
    ScrollKeyPgDown(h);
    iupmatCallEntercellCb(h);
    break;

  case K_SP:
  case K_CR:
  case K_sCR:
    if (iupmatEditShow(h))
    {
      return IUP_IGNORE; /* do not show mark and focus */
    }
    break;

  case K_sDEL:
  case K_DEL:
    {
      int lm,cm;
      for(lm=0;lm<mat_nl(mat);lm++)
      {
        for(cm=0;cm<mat_nc(mat);cm++)
        {
          if(iupmatMarkCellGet(mat,lm,cm))
          {
            if(iupmatCallEditionCbLinCol(h,lm+1,cm+1,1) != IUP_IGNORE)
              iupmatSetCell(h, lm+1, cm+1, "");
          }
        }
      }
    }
    if (mat->redraw) { cdCanvasFlush(mat->cddbuffer); mat->redraw = 0; }
    return IUP_IGNORE; 
  default:
    /* if a char is pressed enter edition mode */
    if (!isxkey(c) && c != K_ESC && isgraph(c))
    {
      static char newval[2] = {0,0};
      newval[0]=c;
      if (iupmatEditShow(h))
      {
        if (mat_edtdatah(mat) == mat_edttexth(mat))
        {
          IupSetAttribute(mat_edtdatah(mat),IUP_VALUE,newval);
          IupSetAttribute(mat_edtdatah(mat),IUP_CARET,"2");
        }
        return IUP_IGNORE; /* do not show mark and focus */
      }
    }
    ret = IUP_DEFAULT; /* unprocessed keys */
    break;
  }

  iupmatShowFocus(h);

  if (mat->redraw) { cdCanvasFlush(mat->cddbuffer); mat->redraw = 0; }
  
  return ret;
}


/**************************************************************************
***************************************************************************
*
*   Funcoes exportadas
*
***************************************************************************
***************************************************************************/

/*
%F Reseta o numero de vezes que as teclas home e end foram pressionadas
*/
void iupmatResetKeyCount(void)
{
  homekeycount = endkeycount = 0;
}

/*
%F Retorna o numero de vezes que a tecla home foi pressionada
*/
int iupmatGetHomeKeyCount(void)
{
  return homekeycount;
}

/*
%F Retorna o numero de vezes que a tecla end foi pressionada
*/
int iupmatGetEndKeyCount(void)
{
  return endkeycount;
}

/*

%F Callback chamada quando uma tecla e pressionada, estando o foco com a
   matriz. Chama a callback do usuario.
%i hm : Handle da matriz ou do text,
   c : caracter digitado.
%o Retorno depende do retorno da callback do usuario

*/
int iupmatKeyPressCb(Ihandle *hm, int c, int press)
{
  int oldc = c;
  Tmat *mat=(Tmat*)matrix_data(hm);
  ImatrixActioncb cb;
  int err;

  mat_hasiupfocus(mat) = 1;

  if (!press)
    return IUP_DEFAULT;

  /* Chama callback do usuario associada a acao ACTION_CB */
  cb = (ImatrixActioncb)IupGetCallback(hm,"ACTION_CB");
  if (cb)
  {
    if(MAT_ACTIONKEY(c))
    {
      c = cb(hm, c, mat_lin(mat)+1, mat_col(mat)+1,0,
              iupmatGetCellValue(hm, mat_lin(mat), mat_col(mat)));
    }
    else
    {
      char future[2]={0,0};
      future[0] = c;
      c = cb(hm, c, mat_lin(mat)+1, mat_col(mat)+1,0,future);
    }
    if (c==IUP_IGNORE || c==IUP_CLOSE || c==IUP_CONTINUE)
      return c;
    else if (c==IUP_DEFAULT)
      c = oldc;
  }

  if (c != K_HOME && c != K_sHOME)
    homekeycount = 0;
  if (c != K_END && c != K_sEND)
    endkeycount = 0;

  IsCanvasSet(mat,err);

  return iupmatKey(hm,c);
}
