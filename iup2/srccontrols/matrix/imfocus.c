/** \file
 * \brief iupmatrix focus control
 *
 * See Copyright Notice in iup.h
 * $Id: imfocus.c,v 1.1 2008-10-17 06:20:15 scuri Exp $
 */

#include <stdio.h>
#include <string.h>

#include <iup.h>
#include <iupcpi.h>
#include <cd.h>

#include "iupmatrix.h"
#include "matrixcd.h"
#include "matridef.h"
#include "imaux.h"
#include "imdraw.h"
#include "imedit.h"
#include "imfocus.h"
#include "immark.h"
#include "imscroll.h"
#include "imgetset.h"

/*
%F Verifica se a celula que vai conter o foco esta numa area
   visivel da matriz.
%i h - Handle da matriz.
*/
int iupmatIsFocusVisible(Ihandle *h)
{
  Tmat *mat=(Tmat*)matrix_data(h);

  /* Verifica se a celula que tem o foco esta visivel */
  if (!iupmatIsCellVisible(h,mat_lin(mat),mat_col(mat)))
    return 0;

  return 1;
}

/*
%F Mostra a representacao do foco na celula que o contem (mat_lin(mat) e
   mat_col(mat)).
   so coloca a representacao do foco, NAO redesenha a celula, isto e'
   trabalho de quem chamou a funcao....
   Seta o foco do IUP para a matriz ou para o campo de edicao.
%i h - Handle da matriz.
*/
void iupmatShowFocus(Ihandle *h)
{
  Tmat *mat=(Tmat*)matrix_data(h);

  if(!iupmatIsFocusVisible(h) || !mat_hasiupfocus(mat))
    return;

  iupmatDrawFocus(h,mat_lin(mat),mat_col(mat),1);
}

/*
%F Esconde a representacao do foco. 
%i h - Handle da matriz.
*/
void iupmatHideFocus(Ihandle *h)
{
  Tmat *mat=(Tmat*)matrix_data(h);
  if(iupmatIsFocusVisible(h)) 
    iupmatDrawFocus(h,mat_lin(mat),mat_col(mat),0);
}

/*
%F Seta qual a celula que contem o foco.  NAO mostra a representacao
   do foco... Isto e' feito por iupmatShowFocus.  NAO deve ser chamada
   com a representacao do foco visivel na tela...

%i h - Handle da matriz
   lin, col - Coordenadas da celula que vai receber o foco.
*/
void iupmatSetFocusPos(Ihandle *h, int lin, int col)
{
  Tmat *mat=(Tmat*)matrix_data(h);

  mat_lin(mat) = lin;
  mat_col(mat) = col;
}

/*
%F Muda o foco de lugar... So uma forma compacta para os inumeros
   casos em que teria que chamar Hide/Set/Show em sequencia

%i h - Handle da matriz
   lin,col - Coordenadas da celula que vai receber o foco.
*/
void iupmatHideSetShowFocus(Ihandle *h, int lin, int col)
{
  iupmatHideFocus(h);
  iupmatSetFocusPos(h,lin,col);
  iupmatShowFocus(h);
}

/*
%F Seta a posicao do foco e mostra. So uma forma compacta para os inumeros
   casos em que teria que chamar Set/Show em sequencia

%i h - Handle da matriz
   lin,col - Coordenadas da celula que vai receber o foco.
*/
void iupmatSetShowFocus(Ihandle *h, int lin, int col)
{
  iupmatSetFocusPos(h,lin,col);
  iupmatShowFocus(h);
}

int iupmatFocusCb(Ihandle *hm, int focus)
{
  Tmat *mat=(Tmat*)matrix_data(hm);
  int err;
  int rc = IUP_DEFAULT;

  /* avoid callback inheritance */
  if (!iupStrEqual(IupGetClassName(hm), "matrix"))
    return IUP_IGNORE;

#ifdef _MOTIF_
  if(focus && IupGetInt(mat_self(mat), "_IUPMAT_DOUBLE_CLICK"))
  {
    IupSetAttribute(mat_self(mat), "_IUPMAT_DOUBLE_CLICK", NULL);
    return IUP_DEFAULT;
  }
#endif

  IsCanvasSet(mat,err);
  if(err == CD_OK)
  {
    if (focus)
    {
      SetSbV;
      SetSbH;

      mat_hasiupfocus(mat) = 1;
      iupmatShowFocus(hm);
    }
    else
    {
      mat_hasiupfocus(mat) = 0;
      iupmatHideFocus(hm);
    }

    if (mat->redraw) { cdCanvasFlush(mat->cddbuffer); mat->redraw = 0; }
  }

  if (focus)
    iupmatCallEntercellCb(hm);
  else
    iupmatCallLeavecellCb(hm);

  return rc;
}
