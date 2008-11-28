/** \file
* \brief iupmatrix 
* Functions used to edit a node name in place.
*
* See Copyright Notice in iup.h
* $Id: imedit.c,v 1.2 2008-11-28 00:02:49 scuri Exp $
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>   
#include <ctype.h>
#include <limits.h>
#include <stdarg.h>

#include "iup.h"
#include "iupcbs.h"
#include "iupmatrix.h"
#include "iupkey.h"

#include <cd.h>
#include <cdiup.h>
#include <cddbuf.h>

#include "iup_object.h"
#include "iup_attrib.h"
#include "iup_str.h"
#include "iup_drv.h"
#include "iup_drvfont.h"
#include "iup_stdcontrols.h"
#include "iup_controls.h"
#include "iup_layout.h"
#include "iup_cdutil.h"

#include "matrixdef.h"
#include "imscroll.h"
#include "imaux.h"
#include "imfocus.h"
#include "imedit.h"
#include "imkey.h"
#include "matrixcd.h"


static int iMatrixCallDropdownCb(Ihandle* ih, int lin, int col)
{
  IFnnii cb = (IFnnii)IupGetCallback(ih, "DROP_CB");
  if(cb)
  {
    int ret;
    char* a = iMatrixGetCellValue(ih, lin, col);
    if(!a) a = "";

    IupSetAttribute(ih->data->droph, "PREVIOUSVALUE", a);
    IupSetAttribute(ih->data->droph, "VALUE", "1");

    ret = cb(ih, ih->data->droph, lin+1, col+1);

    /* check if the user set an invalid value */
    if(IupGetInt(ih->data->droph, "VALUE") == 0)
      IupSetAttribute(ih->data->droph, "VALUE", "1");

    if(ret == IUP_DEFAULT)
      return 1;
  }

  return 0;
}

static int iMatrixDropCb(Ihandle* ih, char* t, int i, int v)
{
  IFniinsii cb = (IFniinsii)IupGetCallback(ih, "DROPSELECT_CB");

  if(!IupGetInt(ih, "VISIBLE"))
    return IUP_DEFAULT;

  if(cb)
  {
    int ret = cb(ih, ih->data->lin.active+1, ih->data->col.active+1, ih, t, i, v);

    /* If the user returns IUP_CONTINUE in a dropselect_cb 
    the value is accepted and the matrix leaves edition mode. */
    if(ret == IUP_CONTINUE)
      iMatrixEditClose(ih);
  }

  return IUP_DEFAULT;
}

static void iMatrixEditChooseElement(Ihandle* ih, int lin, int col)
{
  int drop;

  drop = iMatrixCallDropdownCb(ih, lin, col);

  if(drop)
    ih->data->datah = ih->data->droph;
  else
  {
    char* p;

    ih->data->datah = ih->data->texth;

    /* dropdown values are set by the user in DROP_CB.
    text value is set here from cell contents. */
    p = iMatrixGetCellValue(ih, ih->data->lin.active, ih->data->col.active);
    if(p)
      IupSetAttribute(ih->data->texth, "VALUE", p);
    else
      IupSetAttribute(ih->data->texth, "VALUE", "");
  }
}

static int iMatrixCallEditionCb(Ihandle* ih, int modo, int update)
{
  int rc = IUP_DEFAULT;
  IFniii cb;

  /* If the cell belongs to a inactive line or column, don't allow
     to enter in edit mode */
  if(modo == 1 && (ih->data->col.inactive[ih->data->col.active] || ih->data->lin.inactive[ih->data->lin.active]))
    return IUP_IGNORE;

  cb = (IFniii)IupGetCallback(ih, "EDITION_CB");
  if(cb)
  {
    rc = cb(ih, ih->data->lin.active+1, ih->data->col.active+1, modo);
  }

  if(update && rc == IUP_DEFAULT && modo == 0)
    iMatrixUpdateCellValue(ih);

  return rc;
}

static int iMatrixEditCancel(Ihandle* ih, int focus, int update, int ignore)
{
  if(IupGetInt(ih->data->datah, "VISIBLE"))
  {
    int ret;

    /* Avoid calling EDITION_CB twice. Usually because a killfocus. */
    if(IupGetInt(ih, "_IUPMAT_CALL_EDITION") == 1)
      return IUP_DEFAULT;

    IupSetAttribute(ih, "_IUPMAT_CALL_EDITION", "1");

    ret = iMatrixCallEditionCb(ih, 0, update);
    IupSetAttribute(ih, "_IUPMAT_CALL_EDITION", NULL);

    if(ret == IUP_IGNORE && ignore)
      return IUP_IGNORE;

    IupSetAttribute(ih->data->datah, "VISIBLE", "NO");
    IupSetAttribute(ih->data->datah, "ACTIVE",  "NO");

    if(focus)
    {
      IupSetFocus(ih);
      ih->data->hasiupfocus = 1; /* set this so even if getfocus_cb is not called the focus is drawn */
    }
#ifdef SunOS
    IupSetAttribute(ih,"REDRAW", "ALL");
#endif
  }

  return IUP_DEFAULT;
}

static int iMatrixEditKillFocusCb(Ihandle* ih)
{
#ifdef _MOTIF_
  if(IupGetInt(ih, "_IUPMATRIX_DOUBLE_CLICK"))
    return IUP_DEFAULT;
#endif

  iMatrixEditCheckHidden(ih);
  return IUP_DEFAULT;
}

int iMatrixEditClose(Ihandle* ih)
{
  return iMatrixEditCancel(ih, 1, 1, 1); /* set focus + update + use ignore */
}

void iMatrixEditCheckHidden(Ihandle* ih)
{
  iMatrixEditCancel(ih, 0, 1, 0); /* no focus + update + no ignore */
}

int iMatrixEditIsVisible(Ihandle* ih)
{
  if (!IupGetInt(ih, "ACTIVE"))
    return 0;

  if (!IupGetInt(ih->data->datah, "VISIBLE"))
    return 0;

  return 1;
}

int iMatrixEditShow(Ihandle* ih)
{
  char* mask;
  char* s = iupStrGetMemory(30);
  int w, h, x, y;
  int charwidth, charheight;

  /* not active */
  if(!IupGetInt(ih, "ACTIVE"))
    return 0;

  /* already visible */
  if(IupGetInt(ih->data->datah, "VISIBLE"))
    return 0;

  /* notify application */
  if(iMatrixCallEditionCb(ih, 1, 0) == IUP_IGNORE)
    return 0;

  /* select edit control */
  iMatrixEditChooseElement(ih, ih->data->lin.active, ih->data->col.active);

  /* position the cell to make it visible */
  iMatrixScrollOpen(ih, ih->data->lin.active, ih->data->col.active);

  /* set attributes */
  IupStoreAttribute(ih->data->datah, "BGCOLOR",
                    iMatrixDrawGetBgColor(ih, ih->data->lin.active+1, ih->data->col.active+1));
  IupStoreAttribute(ih->data->datah, "FGCOLOR",
                    iMatrixDrawGetFgColor(ih, ih->data->lin.active+1, ih->data->col.active+1));
  IupSetAttribute(ih->data->datah, "FONT", IupGetAttribute(ih, "FONT"));
  mask = IupMatGetAttribute(ih,"MASK", ih->data->lin.active+1, ih->data->col.active+1);
  if (mask)
  {
    IupSetAttribute(ih->data->datah, "MASKCASEI", IupMatGetAttribute(ih,"MASKCASEI", ih->data->lin.active+1, ih->data->col.active+1));
    IupSetAttribute(ih->data->datah, "MASK", mask);
  }
  else
  {
    mask = IupMatGetAttribute(ih,"MASKINT", ih->data->lin.active+1, ih->data->col.active+1);
    if (mask)
      IupSetAttribute(ih->data->datah, "MASKINT", mask);
    else
    {
      mask = IupMatGetAttribute(ih,"MASKFLOAT", ih->data->lin.active+1, ih->data->col.active+1);
      if (mask)
        IupSetAttribute(ih->data->datah, "MASKFLOAT", mask);
    }
  }

  /* calc size */
  iMatrixGetCellDim(ih, ih->data->lin.active, ih->data->col.active, &x, &y, &w, &h);

  /* set position */
  ih->data->datah->x = x;  /* POSX */
  ih->data->datah->y = y;  /* POSY */

  /* set size */
  IupSetfAttribute(ih->data->datah, "RASTERSIZE", "%dx%d", w, h);

  iupdrvFontGetCharSize(ih, &charwidth, &charheight);
  sprintf(s, "%dx%d", (4*w)/charwidth, (8*h)/charheight);
  IupSetAttribute(ih->data->datah, "SIZE", s);
  ih->data->datah->currentwidth  = w;
  ih->data->datah->currentheight = h;
  iupClassObjectLayoutUpdate(ih->data->datah);

  /* activate and show */
  IupSetAttribute(ih->data->datah, "ACTIVE",  "YES");
  IupSetAttribute(ih->data->datah, "VISIBLE", "YES");
  IupSetFocus(ih->data->datah);

  /* update text attributes */
  if(ih->data->datah == ih->data->texth)
  {
    char* selection = iupAttribGetStr(ih, "SELECTION");
    if(selection)
    {
      /* this allow the user to set the SELECTION inside the SHOWRENAME_CB */
      iupAttribStoreStr(ih->data->texth, "SELECTION", selection);
      iupAttribSetStr(ih, "SELECTION", NULL);
    }
    else
    {
      char* caret = iupAttribGetStr(ih, "CARET");
      if(caret)
      {
        /* this allow the user to set the CARET inside the SHOWRENAME_CB */
        iupAttribStoreStr(ih->data->texth, "CARET", caret);
        iupAttribSetStr(ih, "CARET", NULL);
      }
    }
  }

  return 1;
}

static int iMatrixTextActionCb(Ihandle* ih, int c, char* after)
{
  IFniiiis cb = (IFniiiis) IupGetCallback(ih, "ACTION_CB");

  /* Call the user callback */
  if(cb)
  {
    int oldc = c;
    c = cb(ih, c, ih->data->lin.active+1, ih->data->col.active+1, 1, after);
    if(c == IUP_IGNORE || c == IUP_CLOSE || c == IUP_CONTINUE)
      return c;
    else if(c == IUP_DEFAULT)
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
      if(iMatrixEditClose(ih) == IUP_DEFAULT)
      {
        iMatrixKey(ih, c);  
        return IUP_IGNORE;
      }
      break;
    case K_LEFT:
      if(IupGetInt(ih, "CARET") == 1)
      {
        /* if at first character */
        if(iMatrixEditClose(ih) == IUP_DEFAULT)
          iMatrixKey(ih,c);
      }
      break;
    case K_RIGHT:
      { 
        char* val = IupGetAttribute(ih, "VALUE");
        if(val)
        {
          /* if at last character */
          int vallen = strlen(val);
          if(vallen == (IupGetInt(ih, "CARET") - 1))
          {
            if(iMatrixEditClose(ih) == IUP_DEFAULT)
              iMatrixKey(ih, c);
          }
        }
      }
      break;
    case K_ESC:
      iMatrixEditCancel(ih, 1, 0, 0); /* set focus + NO update + NO ignore */
      break;
    case K_CR:
      if(iMatrixEditClose(ih) == IUP_DEFAULT)
        iMatrixKey(ih, c + 1000);  
      break;
  }

  return IUP_DEFAULT;
}

/* keys that are not received by iMatrixTextActionCb in Motif. */
#define IMATRIX_EDITACTIONKEY(c) \
  ((c) == K_LEFT   || (c) == K_sLEFT || (c) == K_cLEFT || (c) == K_RIGHT || (c) == K_sRIGHT || (c) == K_cRIGHT || \
   (c) == K_UP     || (c) == K_sUP   || (c) == K_cUP   || (c) == K_DOWN  || (c) == K_sDOWN  || (c) == K_cDOWN  || \
   (c) == K_ESC)

#ifdef _MOTIF_
static int iMatrixKeyAnyTextCb(Ihandle* ih, int c)
{
  int ret = IUP_DEFAULT;

  /* In Motif, these keys don't generate callbacks in a text.
     Therefore, they need to be solve here.
  */
  if(IMATRIX_EDITACTIONKEY(c))
    ret = iMatrixTextActionCb(ih, c, IupGetAttribute(ih, "VALUE"));

  return ret;
}
#endif

static int iMatrixKeyAnyDropCb(Ihandle* ih, int c)
{
  IFniiiis cb = (IFniiiis)IupGetCallback(ih, "ACTION_CB");

  /* Call the user callback */
  if(cb)
  {
    int oldc = c;
    c = cb(ih, c, ih->data->lin.active + 1, ih->data->col.active + 1, 1, "");
    if(c == IUP_IGNORE || c == IUP_CLOSE  || c == IUP_CONTINUE)
      return c;
    else if(c == IUP_DEFAULT)
      c = oldc;
  }

  switch (c)
  {
    case K_CR:
      if(iMatrixEditClose(ih) == IUP_DEFAULT)
        iMatrixKey(ih, c + 1000);  
      break;
    case K_ESC:
      iMatrixEditCancel(ih, 1, 0, 0); /* set focus + NO update + NO ignore */
      break;
  }

  return IUP_DEFAULT;
}

char* iMatrixEditGetValue(Ihandle* ih)
{
  char* valor = NULL;

  if(ih->data->datah == ih->data->droph)
  {
    char* val = IupGetAttribute(ih->data->droph, "VALUE");
    if(val)
      valor = IupGetAttribute(ih->data->droph, val);
  }
  else
    valor = IupGetAttribute(ih->data->texth, "VALUE");

  return valor;
}

void iMatrixEditCreate(Ihandle* ih)
{
  ih->data->texth = IupText(NULL);
  ih->firstchild = ih->data->texth;
  ih->data->texth->parent = ih;

  IupSetCallback(ih->data->texth, "ACTION",       (Icallback)iMatrixTextActionCb);
  IupSetCallback(ih->data->texth, "KILLFOCUS_CB", (Icallback)iMatrixEditKillFocusCb);
  IupSetAttribute(ih->data->texth, "VALUE",  "");
  IupSetAttribute(ih->data->texth, "BORDER", "YES");
  IupSetAttribute(ih->data->texth, "VISIBLE", "NO");
  IupSetAttribute(ih->data->texth, "ACTIVE",  "NO");

#ifdef _MOTIF_
  IupSetCallback(ih->data->texth, "K_ANY", (Icallback)iMatrixKeyAnyTextCb);
#endif

  /* Create the Dropdown field */
  ih->data->droph = IupList(NULL);
  ih->firstchild->brother = ih->data->droph;
  ih->data->droph->parent = ih;

  IupSetCallback(ih->data->droph, "ACTION",       (Icallback)iMatrixDropCb);
  IupSetCallback(ih->data->droph, "KILLFOCUS_CB", (Icallback)iMatrixEditKillFocusCb);
  IupSetCallback(ih->data->droph, "K_ANY",        (Icallback)iMatrixKeyAnyDropCb);
  IupSetAttribute(ih->data->droph, "DROPDOWN", "YES");
  IupSetAttribute(ih->data->droph, "MULTIPLE", "NO");
  IupSetAttribute(ih->data->droph, "VISIBLE", "NO");
  IupSetAttribute(ih->data->droph, "ACTIVE",  "NO");
}

