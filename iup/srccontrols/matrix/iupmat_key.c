/** \file
 * \brief iupmatrix control
 * keyboard control
 *
 * See Copyright Notice in iup.h
 * $Id: iupmat_key.c,v 1.2 2008-11-28 00:19:04 scuri Exp $
 */

/**************************************************************************/
/* Functions to control keys in the matrix and in the text edition        */
/**************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

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
#include "iup_stdcontrols.h"
#include "iup_controls.h"
#include "iup_cdutil.h"

#include "iupmat_def.h"
#include "iupmat_cd.h"
#include "iupmat_draw.h"
#include "iupmat_scroll.h"
#include "iupmat_focus.h"
#include "iupmat_aux.h"
#include "iupmat_getset.h"
#include "iupmat_key.h"
#include "iupmat_mark.h"
#include "iupmat_edit.h"


/* keys that are not chars */
#define IMATRIX_ACTIONKEY(c) \
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

/* how many times the keys "home" and "end" were pressed */
static int homekeycount = 0, endkeycount = 0;


/**************************************************************************/
/* Private functions                                                      */
/**************************************************************************/

/* This function handles all the keys in the matrix.
   -> c: pressed key.
*/
int iMatrixKey(Ihandle* ih, int c)
{
  int ret = IUP_IGNORE;

  /* Hide (off) the marked cells if the key is not tab/shift-tab/del */
  if(c != K_TAB && c != K_sTAB && c != K_DEL && c != K_sDEL)
    iMatrixMarkHide(ih, 0);

  /* If the focus is not visible, a scroll is done for that the focus to be visible */
  if(!iMatrixIsFocusVisible(ih))
  {
    /* this will end edition mode */
    ScrollPosVer(ih, (float)ih->data->lin.active / ih->data->lin.num);
    ScrollPosHor(ih, (float)ih->data->col.active / ih->data->col.num);
  }

  switch (c)
  {
    case K_CR + 1000:   /* used by the iMatrixTextActionCb */
      if(iMatrixCallLeavecellCb(ih) == IUP_IGNORE)
        break;
      ScrollKeyCr(ih);
      iMatrixCallEntercellCb(ih);
      break;

    case K_cHOME:
    case K_sHOME:
    case K_HOME:
      if(iMatrixCallLeavecellCb(ih) == IUP_IGNORE)
        break;
      ScrollKeyHome(ih);
      homekeycount++;
      iMatrixCallEntercellCb(ih);
      break;

    case K_cEND:
    case K_sEND:
    case K_END:
      if(iMatrixCallLeavecellCb(ih) == IUP_IGNORE)
        break;
      ScrollKeyEnd(ih);
      endkeycount++;
      iMatrixCallEntercellCb(ih);
      break;

    case K_sTAB:
    case K_TAB:
      return IUP_CONTINUE;

    case K_cLEFT:
    case K_sLEFT:
    case K_LEFT:
      if(iMatrixCallLeavecellCb(ih) == IUP_IGNORE)
        break;
      ScrollKeyLeft(ih);
      iMatrixCallEntercellCb(ih);
      break;

    case K_cRIGHT:
    case K_sRIGHT:
    case K_RIGHT:
      if(iMatrixCallLeavecellCb(ih) == IUP_IGNORE)
        break;
      ScrollKeyRight(ih);
      iMatrixCallEntercellCb(ih);
      break;

    case K_cUP:
    case K_sUP:
    case K_UP:
      if(iMatrixCallLeavecellCb(ih) == IUP_IGNORE)
        break;
      ScrollKeyUp(ih);
      iMatrixCallEntercellCb(ih);
      break ;

    case K_cDOWN:
    case K_sDOWN:
    case K_DOWN:
      if(iMatrixCallLeavecellCb(ih) == IUP_IGNORE)
        break;
      ScrollKeyDown(ih);
      iMatrixCallEntercellCb(ih);
      break;

    case K_sPGUP:
    case K_PGUP:
      if(iMatrixCallLeavecellCb(ih) == IUP_IGNORE)
        break;
      ScrollKeyPgUp(ih);
      iMatrixCallEntercellCb(ih);
      break;

    case K_sPGDN:
    case K_PGDN:
      if(iMatrixCallLeavecellCb(ih) == IUP_IGNORE)
        break;
      ScrollKeyPgDown(ih);
      iMatrixCallEntercellCb(ih);
      break;

    case K_SP:
    case K_CR:
    case K_sCR:
      if(iMatrixEditShow(ih))
      {
        return IUP_IGNORE; /* do not show mark and focus */
      }
      break;

    case K_sDEL:
    case K_DEL:
      {
        int lm, cm;
        for(lm = 0; lm < ih->data->lin.num; lm++)
        {
          for(cm = 0; cm < ih->data->col.num; cm++)
          {
            if(iMatrixMarkCellGet(ih, lm, cm))
            {
              if(iMatrixCallEditionCbLinCol(ih, lm + 1, cm + 1, 1) != IUP_IGNORE)
                iMatrixSetCell(ih, lm + 1, cm + 1, "");
            }
          }
        }
      }
      if(ih->data->redraw)
      {
        cdCanvasFlush(ih->data->cddbuffer);
        ih->data->redraw = 0;
      }
      return IUP_IGNORE; 
    default:
      /* if a char is pressed enter edition mode */
      if(!isxkey(c) && c != K_ESC && isgraph(c))
      {
        static char newval[2] = {0,0};
        newval[0]=c;
        if(iMatrixEditShow(ih))
        {
          if(ih->data->datah == ih->data->texth)
          {
            IupSetAttribute(ih->data->datah, "VALUE", newval);
            IupSetAttribute(ih->data->datah, "CARET", "2");
          }
          return IUP_IGNORE; /* do not show mark and focus */
        }
      }
      ret = IUP_DEFAULT; /* unprocessed keys */
      break;
  }

  iMatrixShowFocus(ih);

  if(ih->data->redraw)
  {
    cdCanvasFlush(ih->data->cddbuffer);
    ih->data->redraw = 0;
  }
  
  return ret;
}


/**************************************************************************/
/* Exported functions                                                     */
/**************************************************************************/

/* Reset the number of times that the "home" and "end" keys were pressed  */
void iMatrixResetKeyCount(void)
{
  homekeycount = endkeycount = 0;
}

/* Return the number of times that the "home" was pressed */
int iMatrixGetHomeKeyCount(void)
{
  return homekeycount;
}

/* Return the number of times that the "end" was pressed */
int iMatrixGetEndKeyCount(void)
{
  return endkeycount;
}

/* This function is called when a key is pressed, being the focus
   with the matrix. Call the user callback.
   -> c : charactere pressed
*/
int iMatrixKeyPressCB(Ihandle* ih, int c, int press)
{
  int oldc = c;
  IFniiiis cb;
  int err;

  ih->data->hasiupfocus = 1;

  if(!press)
    return IUP_DEFAULT;

  /* Call the user callback, registered to ACTION_CB */
  cb = (IFniiiis)IupGetCallback(ih, "ACTION_CB");
  if(cb)
  {
    if(IMATRIX_ACTIONKEY(c))
    {
      c = cb(ih, c, ih->data->lin.active+1, ih->data->col.active+1, 0,
             iMatrixGetCellValue(ih, ih->data->lin.active, ih->data->col.active));
    }
    else
    {
      char future[2]={0,0};
      future[0] = c;
      c = cb(ih, c, ih->data->lin.active+1, ih->data->col.active+1, 0, future);
    }
    if(c == IUP_IGNORE || c == IUP_CLOSE || c == IUP_CONTINUE)
      return c;
    else if(c == IUP_DEFAULT)
      c = oldc;
  }

  if(c != K_HOME && c != K_sHOME)
    homekeycount = 0;
  if(c != K_END && c != K_sEND)
    endkeycount = 0;

  IsCanvasSet(ih, err);

  return iMatrixKey(ih, c);
}
