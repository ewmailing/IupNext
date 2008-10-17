/** \file
* \brief iupmatrix 
* Functions used to edit a node name in place.
*
* See Copyright Notice in iup.h
* $Id: imedit.c,v 1.1 2008-10-17 06:20:15 scuri Exp $
*/

#include <iup.h>
#include <iupcpi.h>
#include <iupkey.h>
#include <cd.h>

#include <stdio.h>  
#include <string.h>
#include <stdlib.h>
#include <math.h>   
#include <ctype.h>
#include <limits.h>
#include <stdarg.h>

#include "matridef.h"

#include "iupmatrix.h"
#include "imscroll.h"
#include "imaux.h"
#include "imfocus.h"
#include "imedit.h"
#include "imkey.h"
#include "matrixcd.h"

typedef int (*ImatrixActioncb)  (Ihandle *, int, int, int, int, char*);
typedef int (*Dropmcb )(Ihandle*, Ihandle*, int, int);
typedef int (*Dropmscb)(Ihandle*, int, int, Ihandle*, char*, int, int);

static int CallDropdownCb(Ihandle *h, int line, int col)
{
  Dropmcb cb = (Dropmcb)IupGetCallback(h,IUP_DROP_CB);
  if(cb)
  {
    int ret;
    Tmat *mat=(Tmat*)matrix_data(h);
    char *a = iupmatGetCellValue(h,line,col);
    if(!a) a = "";

    IupSetAttribute(mat_edtdroph(mat), IUP_PREVIOUSVALUE, a);
    IupSetAttribute(mat_edtdroph(mat), IUP_VALUE, "1");

    ret = cb(h,mat_edtdroph(mat),line+1,col+1);

    /* check if the user set an invalid value */
    if (IupGetInt(mat_edtdroph(mat), IUP_VALUE) == 0)
      IupSetAttribute(mat_edtdroph(mat), IUP_VALUE, "1");

    if(ret == IUP_DEFAULT)
      return 1;
  }

  return 0;
}

static int iupmatDropCb (Ihandle *self, char *t, int i, int v)
{
  Tmat *mat=(Tmat*)matrix_data(self);
  Dropmscb cb = (Dropmscb)IupGetCallback(mat_self(mat),IUP_DROPSELECT_CB);

  if (!IupGetInt(self, "VISIBLE"))
    return IUP_DEFAULT;

  if (cb)
  {
    int ret = cb(mat_self(mat),mat_lin(mat)+1,mat_col(mat)+1,self,t,i,v);

    /* If the user returns IUP_CONTINUE in a dropselect_cb 
    the value is accepted and the matrix leaves edition mode. */
    if(ret == IUP_CONTINUE)
      iupmatEditClose(mat_self(mat));
  }

  return IUP_DEFAULT;
}

static void iupmatEditChooseElement(Ihandle *h, int lin, int col)
{
  Tmat *mat=(Tmat*)matrix_data(h);
  int drop;

  drop = CallDropdownCb(h,lin,col);

  if(drop)
    mat_edtdatah(mat) = mat_edtdroph(mat);
  else
  {
    char *p;

    mat_edtdatah(mat) = mat_edttexth(mat);

    /* dropdown values are set by the user in DROP_CB.
    text value is set here from cell contents. */
    p = iupmatGetCellValue(h,mat_lin(mat),mat_col(mat));
    if (p)
      IupSetAttribute(mat_edttexth(mat),IUP_VALUE,p);
    else
      IupSetAttribute(mat_edttexth(mat),IUP_VALUE,"");
  }
}

static int iupmatCallEditionCb(Ihandle *h, int modo, int update)
{
  int rc = IUP_DEFAULT;
  Tmat *mat=(Tmat*)matrix_data(h);
  IFniii cb;

  /* Se a celula pertencer a uma linha ou coluna inativa, nao permite
  a entrada no modo de edicao... */
  if (modo == 1 && (mat_colinactive(mat)[mat_col(mat)] || mat_lininactive(mat)[mat_lin(mat)]))
    return IUP_IGNORE;

  cb=(IFniii)IupGetCallback(h,IUP_EDITION_CB);
  if (cb)
  {
    rc = cb(h,mat_lin(mat)+1,mat_col(mat)+1, modo);
  }

  if (update && rc == IUP_DEFAULT && modo == 0)
    iupmatUpdateCellValue(h);

  return rc;
}

static int iupmatEditCancel(Ihandle *h, int focus, int update, int ignore)
{
  Tmat *mat = (Tmat*)matrix_data(h);

  if (iupCheck(mat_edtdatah(mat), "VISIBLE")==YES)
  {
    int ret;

    /* Avoid calling EDITION_CB twice. Usually because a killfocus. */
    if (IupGetInt(mat_self(mat), "_IUPMAT_CALL_EDITION")==1)
      return IUP_DEFAULT;

    iupSetEnv(mat_self(mat), "_IUPMAT_CALL_EDITION", "1");
    ret = iupmatCallEditionCb(mat_self(mat), 0, update);
    iupSetEnv(mat_self(mat), "_IUPMAT_CALL_EDITION", NULL);

    if (ret == IUP_IGNORE && ignore)
      return IUP_IGNORE;

    IupSetAttribute(mat_edtdatah(mat),IUP_VISIBLE,IUP_NO);
    IupSetAttribute(mat_edtdatah(mat),IUP_ACTIVE,IUP_NO);
    if (focus) 
    {
      IupSetFocus(mat_self(mat));
      mat_hasiupfocus(mat) = 1; /* set this so even if getfocus_cb is not called the focus is drawn */
    }
#ifdef SunOS
    IupSetAttribute(h,"REDRAW", "ALL");
#endif
  }

  return IUP_DEFAULT;
}

static int matEditKillFocusCb(Ihandle* self)
{
#ifdef _MOTIF_
  Tmat *mat = (Tmat*)matrix_data(self);
  if(IupGetInt(mat_self(mat), "_IUPMAT_DOUBLE_CLICK"))
    return IUP_DEFAULT;
#endif

  iupmatEditCheckHidden(self);
  return IUP_DEFAULT;
}

int iupmatEditClose(Ihandle *self)
{
  return iupmatEditCancel(self, 1, 1, 1); /* set focus + update + use ignore */
}

void iupmatEditCheckHidden(Ihandle* self)
{
  iupmatEditCancel(self, 0, 1, 0); /* no focus + update + no ignore */
}

int iupmatEditIsVisible(Ihandle *self)
{
  Tmat *mat=(Tmat*)matrix_data(self);

  /* matrix active */
  if (!iupCheck(self,IUP_ACTIVE))
    return 0;

  /* not visible */
  if (iupCheck(mat_edtdatah(mat), "VISIBLE")==YES)
    return 1;

  return 0;
}

int iupmatEditShow(Ihandle *self)
{
  static char s[30];
  Tmat *mat=(Tmat*)matrix_data(self);
  int w, h, x, y;
  int charwidth,charheight;

  /* matrix active */
  if (!iupCheck(self,IUP_ACTIVE))
    return 0;

  /* not visible */
  if (iupCheck(mat_edtdatah(mat), "VISIBLE")==YES)
    return 0;

  /* notify application */
  if (iupmatCallEditionCb(self, 1, 0) == IUP_IGNORE)
    return 0;

  /* select edit control */
  iupmatEditChooseElement(self,mat_lin(mat),mat_col(mat));

  /* position the cell to make it visible */
  iupmatScrollOpen(self,mat_lin(mat),mat_col(mat));

  /* set attributes */
  IupStoreAttribute(mat_edtdatah(mat),IUP_BGCOLOR,
                    iupmatDrawGetBgColor(mat_self(mat),mat_lin(mat)+1, mat_col(mat)+1));
  IupStoreAttribute(mat_edtdatah(mat),IUP_FGCOLOR,
                    iupmatDrawGetFgColor(mat_self(mat),mat_lin(mat)+1, mat_col(mat)+1));
  IupSetAttribute(mat_edtdatah(mat),IUP_FONT,IupGetAttribute(self,IUP_FONT));

  /* calc size */
  iupmatGetCellDim(self,mat_lin(mat),mat_col(mat),&x,&y,&w,&h);

  /* set position */
  iupSetPosX(mat_edtdatah(mat),x);
  iupSetPosY(mat_edtdatah(mat),y);

  /* set size */
  IupSetfAttribute(mat_edtdatah(mat), "RASTERSIZE", "%dx%d", w, h);

  iupdrvGetCharSize(self,&charwidth, &charheight);
  sprintf (s, "%dx%d", (4*w)/charwidth, (8*h)/charheight);
  iupSetEnv(mat_edtdatah(mat), IUP_SIZE, s);
  iupSetCurrentWidth(mat_edtdatah(mat), w);
  iupSetCurrentHeight(mat_edtdatah(mat), h);
  iupdrvResizeObjects(mat_edtdatah(mat));

  /* activate and show */
  IupSetAttribute(mat_edtdatah(mat),IUP_ACTIVE,IUP_YES);
  IupSetAttribute(mat_edtdatah(mat),IUP_VISIBLE,IUP_YES);
  IupSetFocus(mat_edtdatah(mat));

  /* update text attributes */
  if (mat_edtdatah(mat) == mat_edttexth(mat))
  {
    char* selection = iupGetEnv(self, IUP_SELECTION);
    if (selection)
    {
      /* this allow the user to set the SELECTION inside the SHOWRENAME_CB */
      IupStoreAttribute(mat_edttexth(mat),IUP_SELECTION,selection);
      iupSetEnv(self, IUP_SELECTION, NULL);
    }
    else
    {
      char* caret = iupGetEnv(self, IUP_CARET);
      if (caret)
      {
        /* this allow the user to set the CARET inside the SHOWRENAME_CB */
        IupStoreAttribute(mat_edttexth(mat),IUP_CARET,caret);
        iupSetEnv(self, IUP_CARET, NULL);
      }
    }
  }

  return 1;
}

static int iupmatTextActionCb(Ihandle *h, int c, char* after)
{
  Tmat *mat = (Tmat*)matrix_data(h);
  ImatrixActioncb cb = (ImatrixActioncb) IupGetCallback(mat_self(mat),"ACTION_CB");

  /* Chama callback do usuario */
  if (cb)
  {
    int oldc = c;
    c = cb(mat_self(mat), c, mat_lin(mat)+1, mat_col(mat)+1, 1, after);
    if (c==IUP_IGNORE || c==IUP_CLOSE || c==IUP_CONTINUE)
      return c;
    else if (c==IUP_DEFAULT)
      c = oldc;
  }

  switch (c)
  {
  case K_UP:
  case K_DOWN:
  case K_cUP:
  case K_cDOWN:
  case K_cLEFT:
  case K_cRIGHT:     
    if (iupmatEditClose(mat_self(mat)) == IUP_DEFAULT)
    {
      iupmatKey(mat_self(mat),c);  
      return IUP_IGNORE;
    }
    break;
  case K_LEFT:
    if (IupGetInt(h,IUP_CARET) == 1)
    {
      /* if at first character */
      if (iupmatEditClose(mat_self(mat)) == IUP_DEFAULT)
        iupmatKey(mat_self(mat),c);
    }
    break;
  case K_RIGHT:
    { 
      char* val = IupGetAttribute(h,IUP_VALUE);
      if (val)
      {
        /* if at last character */
        int vallen = strlen(val);
        if (vallen == (IupGetInt(h,IUP_CARET)-1))
        {
          if (iupmatEditClose(mat_self(mat)) == IUP_DEFAULT)
            iupmatKey(mat_self(mat),c);
        }
      }
    }
    break;
  case K_ESC:
    iupmatEditCancel(h, 1, 0, 0); /* set focus + NO update + NO ignore */
    break;
  case K_CR:
    if (iupmatEditClose(mat_self(mat)) == IUP_DEFAULT)
      iupmatKey(mat_self(mat),c+1000);  
    break;
  }

  return IUP_DEFAULT;
}

/* keys that are not received by iupmatTextActionCb in Motif. */
#define MAT_EDITACTIONKEY(c) \
  ((c) == K_LEFT   || (c) == K_sLEFT || (c) == K_cLEFT || (c) == K_RIGHT || (c) == K_sRIGHT || (c) == K_cRIGHT || \
   (c) == K_UP     || (c) == K_sUP   || (c) == K_cUP   || (c) == K_DOWN  || (c) == K_sDOWN  || (c) == K_cDOWN  || \
   (c) == K_ESC)

#ifdef _MOTIF_
static int iupmatKeyAnyTextCb(Ihandle *h, int c)
{
  int ret = IUP_DEFAULT;

  /* No motif, estas teclas nao geram callbacks em um texto, portanto
  * tem de ser tratadas aqui. */
  if (MAT_EDITACTIONKEY(c))
    ret = iupmatTextActionCb(h, c, IupGetAttribute(h,IUP_VALUE));

  return ret;
}
#endif

static int iupmatKeyAnyDropCb(Ihandle *h, int c)
{
  Tmat *mat=(Tmat*)matrix_data(h);
  ImatrixActioncb cb = (ImatrixActioncb) IupGetCallback(mat_self(mat),"ACTION_CB");

  /* Chama callback do usuario */
  if (cb )
  {
    int oldc = c;
    c = cb(mat_self(mat), c, mat_lin(mat)+1, mat_col(mat)+1, 1, "");
    if (c==IUP_IGNORE || c==IUP_CLOSE  || c==IUP_CONTINUE)
      return c;
    else if (c==IUP_DEFAULT)
      c = oldc;
  }

  switch (c)
  {
  case K_CR:
    if (iupmatEditClose(mat_self(mat)) == IUP_DEFAULT)
      iupmatKey(mat_self(mat),c+1000);  
    break;
  case K_ESC:
    iupmatEditCancel(h, 1, 0, 0); /* set focus + NO update + NO ignore */
    break;
  }

  return IUP_DEFAULT;
}

char* iupmatEditGetValue(Tmat *mat)
{
  char *valor=NULL;
  if(mat_edtdatah(mat) == mat_edtdroph(mat))
  {
    char *val = IupGetAttribute(mat_edtdroph(mat),IUP_VALUE);
    if (val)
      valor = IupGetAttribute(mat_edtdroph(mat),val);
  }
  else
    valor = IupGetAttribute(mat_edttexth(mat),IUP_VALUE);

  return valor;
}

void iupmatEditCreate(Tmat *mat)
{
  mat_edttexth(mat) = IupText(NULL);
  IupSetCallback(mat_edttexth(mat),"ACTION",(Icallback)iupmatTextActionCb);
  IupSetCallback(mat_edttexth(mat),IUP_KILLFOCUS_CB, (Icallback)matEditKillFocusCb);
  iupSetEnv(mat_edttexth(mat),IUP_VALUE, "");
  iupSetEnv(mat_edttexth(mat),IUP_BORDER,IUP_YES);

#ifdef _MOTIF_
  IupSetCallback(mat_edttexth(mat),IUP_K_ANY,(Icallback)iupmatKeyAnyTextCb);
#endif

  /* Cria o campo Dropdown */
  mat_edtdroph(mat) = IupList(NULL);
  IupSetCallback(mat_edtdroph(mat),"ACTION",(Icallback)iupmatDropCb);
  IupSetCallback(mat_edtdroph(mat),IUP_KILLFOCUS_CB, (Icallback)matEditKillFocusCb);
  IupSetCallback(mat_edtdroph(mat),IUP_K_ANY,(Icallback)iupmatKeyAnyDropCb);
  iupSetEnv(mat_edtdroph(mat),IUP_DROPDOWN,IUP_YES);
  iupSetEnv(mat_edtdroph(mat),IUP_MULTIPLE,IUP_NO);
}

void iupmatEditMap(Tmat *mat, Ihandle* self)
{
  /* mapeia o campo de edicao de texto */
  iupCpiSetPosition(mat_edttexth(mat),iupGetPosX(self),iupGetPosY(self));
  iupCpiMap(mat_edttexth(mat),self);

  IupSetAttribute(mat_edttexth(mat),IUP_VISIBLE,IUP_NO);
  IupSetAttribute(mat_edttexth(mat),IUP_ACTIVE,IUP_NO);

  /* mapeia a dropdown list */
  iupCpiSetPosition(mat_edtdroph(mat),iupGetPosX(self),iupGetPosY(self));
  iupCpiMap(mat_edtdroph(mat),self);

  IupSetAttribute(mat_edtdroph(mat),IUP_VISIBLE,IUP_NO);
  IupSetAttribute(mat_edtdroph(mat),IUP_ACTIVE,IUP_NO);
}
