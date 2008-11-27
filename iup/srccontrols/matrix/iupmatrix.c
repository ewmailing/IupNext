/** \file
 * \brief iupmatrix control core
 *
 * See Copyright Notice in iup.h
 * $Id: iupmatrix.c,v 1.6 2008-11-27 06:33:30 scuri Exp $
 */

#include <stdio.h>  /*sprintf*/
#include <stdlib.h>
#include <math.h>   /*ceil*/
#include <string.h>
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
#include "iup_dialog.h"
#include "iup_cdutil.h"

#include "matrixdef.h"
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

/***************************************************************************/
/* Functions to set the scrollbar */
/***************************************************************************/

void SetSbH(Ihandle* ih)
{
  if(ih)
  {
    sprintf(ih->data->sb_posicaox, "%.5f",
      (ih->data->col.totalwh ? ((float)ih->data->col.pos/(float)ih->data->col.totalwh) : 0));
    sprintf(ih->data->sb_tamanhox, "%.5f",
      (ih->data->col.totalwh ? ((float)ih->data->col.size/(float)ih->data->col.totalwh) : 0));

    IupSetAttribute(ih, "DX"  , ih->data->sb_tamanhox);
    IupSetAttribute(ih, "POSX", ih->data->sb_posicaox);

    cdCanvasActivate(ih->data->cddbuffer);
  }
}

void SetSbV(Ihandle* ih)
{
  if(ih)
  {
    sprintf(ih->data->sb_posicaoy, "%.5f",
      (ih->data->lin.totalwh ? ((float)ih->data->lin.pos/(float)ih->data->lin.totalwh) : 0));
    sprintf(ih->data->sb_tamanhoy, "%.5f",
    (ih->data->lin.totalwh ? ((float)ih->data->lin.size/(float)ih->data->lin.totalwh) : 0));

    IupSetAttribute(ih, "DY"  , ih->data->sb_tamanhoy);
    IupSetAttribute(ih, "POSY", ih->data->sb_posicaoy);

    cdCanvasActivate(ih->data->cddbuffer);
  }
}

void SetSb(Ihandle* ih, int m)
{
  if((m) == IMATRIX_MAT_COL)
    SetSbH(ih);
  else
    SetSbV(ih);
}


/*****************************************************************************/
/***** SET AND GET ATTRIBUTES ************************************************/
/*****************************************************************************/

static void iMatrixRepaint(Ihandle* ih)
{
  if (ih->data->redraw)
  {
    cdCanvasFlush(ih->data->cddbuffer);
    ih->data->redraw = 0;
  }
}

static int iMatrixSetValueAttrib(Ihandle* ih, const char* value)
{
  if (IupGetInt(ih->data->datah, "VISIBLE"))
    IupStoreAttribute(ih->data->datah, "VALUE", value);
  else 
    iMatrixSetCell(ih, ih->data->lin.active+1, ih->data->col.active+1, value);
  
  iMatrixRepaint(ih);
  return 1;  /* aqui */
}

static char* iMatrixGetValueAttrib(Ihandle* ih)
{
  if (IupGetInt(ih->data->datah, "VISIBLE"))
    return iMatrixEditGetValue(ih);
  else 
    return iMatrixGetCell(ih, ih->data->lin.active+1, ih->data->col.active+1);
}

static int iMatrixSetCaretAttrib(Ihandle* ih, const char* value)
{
  IupStoreAttribute(ih->data->texth, "CARET", value);
  iupAttribStoreStr(ih, "CARET", value);
  return 1;  /* aqui */
}

static char* iMatrixGetCaretAttrib(Ihandle* ih)
{
  return IupGetAttribute(ih->data->texth, "CARET");
}

static int iMatrixSetCheckFrameColorAttrib(Ihandle* ih, const char* value)
{
  if (iupStrEqualNoCase(value, "YES"))
    ih->data->checkframecolor = 1;
  else
    ih->data->checkframecolor = 0;
  return 0;
}

static char* iMatrixGetCheckFrameColorAttrib(Ihandle* ih)
{
  if (ih->data->checkframecolor)
    return "YES";
  else
    return "NO";
}

static int iMatrixSetSelectionAttrib(Ihandle* ih, const char* value)
{
  IupStoreAttribute(ih->data->texth, "SELECTION", value);
  iupAttribStoreStr(ih, "SELECTION", value);
  return 1;  /* aqui */
}

static char* iMatrixGetSelectionAttrib(Ihandle* ih)
{
  return IupGetAttribute(ih->data->texth, "SELECTION");
}

static int iMatrixSetAddLinAttrib(Ihandle* ih, const char* value)
{
  iMatrixNlcAddLin(ih, value);
  return 1;  /* aqui */
}

static int iMatrixSetDelLinAttrib(Ihandle* ih, const char* value)
{
  iMatrixNlcDelLin(ih, value);
  return 1;  /* aqui */
}

static int iMatrixSetAddColAttrib(Ihandle* ih, const char* value)
{
  iMatrixNlcAddCol(ih, value);
  return 1;  /* aqui */
}

static int iMatrixSetDelColAttrib(Ihandle* ih, const char* value)
{
  iMatrixNlcDelCol(ih, value);
  return 1;  /* aqui */
}

static int iMatrixSetNumLinAttrib(Ihandle* ih, const char* value)
{
  iMatrixNlcNumLin(ih, value);
  return 1;  /* aqui */
}

static char* iMatrixGetNumLinAttrib(Ihandle* ih)
{
  return iMatrixNlcGetNumLin(ih);
}

static int iMatrixSetNumColAttrib(Ihandle* ih, const char* value)
{
  iMatrixNlcNumCol(ih, value);
  return 1;  /* aqui */
}

static char* iMatrixGetNumColAttrib(Ihandle* ih)
{
  return iMatrixNlcGetNumCol(ih);
}

static int iMatrixSetMarkedAttrib(Ihandle* ih, const char* value)
{
  iMatrixMarkSet(ih, value);
  iMatrixRepaint(ih);
  return 1;  /* aqui */
}

static char* iMatrixGetMarkedAttrib(Ihandle* ih)
{
  return iMatrixMarkGet(ih);
}

static int iMatrixSetMarkModeAttrib(Ihandle* ih, const char* value)
{
  iMatrixMarkSetMode(ih, value);
  iMatrixRepaint(ih);
  return 1;
}

static int iMatrixSetAreaAttrib(Ihandle* ih, const char* value)
{
  iMatrixMarkSetMode(ih, value);
  iMatrixRepaint(ih);
  return 1;
}

static int iMatrixSetMultipleAttrib(Ihandle* ih, const char* value)
{
  iMatrixMarkSetMode(ih, value);
  iMatrixRepaint(ih);
  return 1;
}

static int iMatrixSetFocusCellAttrib(Ihandle* ih, const char* value)
{
  iMatrixSetFocusPosition(ih, value, 0);
  iMatrixRepaint(ih);
  return 1;  /* aqui */
}

static char* iMatrixGetFocusCellAttrib(Ihandle* ih)
{
  return iMatrixGetFocusPosition(ih);
}

static int iMatrixSetOriginAttrib(Ihandle* ih, const char* value)
{
  iMatrixSetOrigin(ih, value);
  iMatrixRepaint(ih);
  return 1;  /* aqui */
}

static char* iMatrixGetOriginAttrib(Ihandle* ih)
{
  return iMatrixGetOrigin(ih);
}

static int iMatrixSetEditModeAttrib(Ihandle* ih, const char* value)
{
  if (iupStrEqualNoCase(value, "YES") || iupStrEqualNoCase(value, "ON"))
    iMatrixEditShow(ih);
  else
    iMatrixEditClose(ih);

  iMatrixRepaint(ih);
  return 1;  /* aqui */
}

static char* iMatrixGetEditModeAttrib(Ihandle* ih)
{
  if (iMatrixEditIsVisible(ih))
    return "YES";
  else
    return "NO";
}

static int iMatrixSetActiveAttrib(Ihandle* ih, const char* value)
{
  iupBaseSetActiveAttrib(ih, value);
  iMatrixSetRedraw(ih, "ALL");
  iMatrixRepaint(ih);
  return 1;  /* aqui */
}

static int iMatrixSetRedrawAttrib(Ihandle* ih, const char* value)
{
  iMatrixSetRedraw(ih, value);
  iMatrixRepaint(ih);
  return 1;  /* aqui */
}

static int iMatrixSetWidthAttrib(Ihandle* ih, const char* name_id, const char* value)
{
  int col = 0;
  if (iupStrToInt(name_id, &col))
  {
    iMatrixColResSet(ih, value, col, IMATRIX_MAT_COL, 0);
    iMatrixSetRedraw(ih, "ALL");
    iMatrixRepaint(ih);
  }
  return 1;  /* aqui */
}

static char* iMatrixGetWidthAttrib(Ihandle* ih, const char* name_id)
{
  int col;
  if (iupStrToInt(name_id, &col))
    return iMatrixColResGet(ih, col, IMATRIX_MAT_COL, 0);

  return NULL;
}

static int iMatrixSetHeightAttrib(Ihandle* ih, const char* name_id, const char* value)
{
  int lin = 0;
  if (iupStrToInt(name_id, &lin))
  {
    iMatrixColResSet(ih, value, lin, IMATRIX_MAT_LIN, 0);
    iMatrixSetRedraw(ih, "ALL");
    iMatrixRepaint(ih);
  }
  return 1;  /* aqui */
}

static char* iMatrixGetHeightAttrib(Ihandle* ih, const char* name_id)
{
  int lin;
  if (iupStrToInt(name_id, &lin))
    return iMatrixColResGet(ih, lin, IMATRIX_MAT_LIN, 0);

  return NULL;
}

static int iMatrixSetRasterWidthAttrib(Ihandle* ih, const char* name_id, const char* value)
{
  int col = 0;
  if (iupStrToInt(name_id, &col))
  {
    iMatrixColResSet(ih, value, col, IMATRIX_MAT_COL, 1);
    iMatrixSetRedraw(ih, "ALL");
    iMatrixRepaint(ih);
  }
  return 1;  /* aqui */
}

static char* iMatrixGetRasterWidthAttrib(Ihandle* ih, const char* name_id)
{
  int col;
  if (iupStrToInt(name_id, &col))
    return iMatrixColResGet(ih, col, IMATRIX_MAT_COL, 1);

  return NULL;
}

static int iMatrixSetRasterHeightAttrib(Ihandle* ih, const char* name_id, const char* value)
{
  int lin = 0;
  if (iupStrToInt(name_id, &lin))
  {
    iMatrixColResSet(ih, value, lin, IMATRIX_MAT_LIN, 1);
    iMatrixSetRedraw(ih, "ALL");
    iMatrixRepaint(ih);
  }
  return 1;  /* aqui */
}

static char* iMatrixGetRasterHeightAttrib(Ihandle* ih, const char* name_id)
{
  int lin;
  if (iupStrToInt(name_id, &lin))
    return iMatrixColResGet(ih, lin, IMATRIX_MAT_LIN, 1);

  return NULL;
}

static int iMatrixSetAlignmentAttrib(Ihandle* ih, const char* name_id, const char* value)
{
  int col = 0;

  if (iupStrToInt(name_id, &col))
  {
    char* attrib = iupStrGetMemory(100);
    sprintf(attrib, "ALIGNMENT%s", name_id);
    iupAttribSetStr(ih, attrib, value); /* the attribute must be stored before calling iMatrixSetColAlign */

    iMatrixSetColAlign(ih, col);
    ih->data->redraw = 0; /* leave redraw to REDRAW attribute */
  }

  iMatrixRepaint(ih);
  return 1;
}

static int iMatrixSetFontAttrib(Ihandle* ih, const char* name_id, const char* value)
{
  int lin = -1;
  int col = -1;

  iupSetFontAttrib(ih, value);

  iupStrToIntInt(name_id, &lin, &col, ':');
  if (lin != -1 || col != -1)
  {
    iMatrixSetRedrawCell(ih, lin, col);
    ih->data->redraw = 0; /* leave redraw to REDRAW attribute */
  }
  else
  {
    iMatrixSetRedraw(ih, "ALL");
  }

  iMatrixRepaint(ih);
  return 1;  /* aqui */
}

static char* iMatrixGetFontAttrib(Ihandle* ih, const char* name_id)
{
  int lin = -1;
  int col = -1;

  iupStrToIntInt(name_id, &lin, &col, ':');
  if (lin != -1 || col != -1)
    return iMatrixDrawGetFont(ih, lin, col);

  return NULL;
}

static int iMatrixSetFgColorAttrib(Ihandle* ih, const char* name_id, const char* value)
{
  int lin = -1;
  int col = -1;

  char* attrib = iupStrGetMemory(100);
  sprintf(attrib, "FGCOLOR%s", name_id);
  iupAttribSetStr(ih, attrib, value); /* the attribute must be stored before calling iMatrixSetRedraw */

  iupStrToIntInt(name_id, &lin, &col, ':');
  if (lin != -1 || col != -1)
  {
    iMatrixSetRedrawCell(ih, lin, col);
    ih->data->redraw = 0; /* leave redraw to REDRAW attribute */
  }
  else
  {
    iMatrixSetRedraw(ih, "ALL");
  }

  iMatrixRepaint(ih);
  return 1;
}

static char* iMatrixGetFgColorAttrib(Ihandle* ih, const char* name_id)
{
  int lin = -1;
  int col = -1;

  iupStrToIntInt(name_id, &lin, &col, ':');
  if (lin != -1 || col != -1)
    return iMatrixDrawGetFgColor(ih, lin, col);

  return NULL;
}

static int iMatrixSetBgColorAttrib(Ihandle* ih, const char* name_id, const char* value)
{
  int lin = -1;
  int col = -1;

  char* attrib = iupStrGetMemory(100);
  sprintf(attrib, "BGCOLOR%s", name_id);
  iupAttribSetStr(ih, attrib, value); /* the attribute must be stored before calling iMatrixSetRedraw */

  iupStrToIntInt(name_id, &lin, &col, ':');
  if (lin != -1 || col != -1)
  {
    iMatrixSetRedrawCell(ih, lin, col);
    ih->data->redraw = 0; /* leave redraw to REDRAW attribute */
  }
  else
  {
    iMatrixSetRedraw(ih, "ALL");
  }

  iMatrixRepaint(ih);
  return 1;
}

static char* iMatrixGetBgColorAttrib(Ihandle* ih, const char* name_id)
{
  int lin = -1;
  int col = -1;

  iupStrToIntInt(name_id, &lin, &col, ':');
  if (lin != -1 || col != -1)
    return iMatrixDrawGetBgColor(ih, lin, col);

  return NULL;
}

static int iMatrixSetActiveLinAttrib(Ihandle* ih, const char* name_id, const char* value)
{
  /* ACTIVE - OLD STUFF */
  int lin = 0;
  
  if (iupStrToInt(name_id, &lin))
    iMatrixSetActive(ih, IMATRIX_MAT_LIN, lin, value);

  iMatrixRepaint(ih);
  return 1;  /* aqui */
}

static int iMatrixSetActiveColAttrib(Ihandle* ih, const char* name_id, const char* value)
{
  /* ACTIVE - OLD STUFF */
  int col = 0;
  
  if (iupStrToInt(name_id, &col))
    iMatrixSetActive(ih, IMATRIX_MAT_COL, col, value);

  iMatrixRepaint(ih);
  return 1;  /* aqui */
}

static int iMatrixSetMatrixCellAttrib(Ihandle* ih, const char* name_id, const char* value)
{
  int lin = 0, col = 0, ret = 1;

  if ((iupStrToIntInt(name_id, &lin, &col, ':') == 2) && (col == 0))       /* matrix line titles */
    iMatrixSetTitleLine(ih, lin);
  else if ((iupStrToIntInt(name_id, &lin, &col, ':') == 2) && (lin == 0))  /* matrix column titles */
    iMatrixSetTitleColumn(ih, col);
  else if (iupStrToIntInt(name_id, &lin, &col, ':') == 2)                  /* matrix cell */
  {
    iMatrixSetCell(ih, lin, col, value);
    ret = 0;
  }

  ih->data->redraw = 0; /* leave redraw to REDRAW attribute */
  return ret;
}

static char* iMatrixGetMatrixCellAttrib(Ihandle* ih, const char* name_id)
{
  int lin, col;

  if (iupStrToIntInt(name_id, &lin, &col, ':') == 2) /*matrix cell */
    return iMatrixGetCell(ih, lin, col);

  return NULL;
}

static char* iMatrixGetNumColVisibleAttrib(Ihandle* ih)
{
  char* buffer = iupStrGetMemory(50);
  sprintf(buffer, "%d", ih->data->col.last - ih->data->col.first);
  return buffer;
}

static char* iMatrixGetNumLinVisibleAttrib(Ihandle* ih)
{
  char* buffer = iupStrGetMemory(50);
  sprintf(buffer, "%d", ih->data->lin.last - ih->data->lin.first);
  return buffer;
}

static char* iMatrixGetMaskDataAttrib(Ihandle* ih)
{
  /* Used only by the OLD iupmask API */
  if (IupGetInt(ih->data->datah, "VISIBLE"))
    return IupGetAttribute(ih->data->datah,"_IUPMASK_DATA");
  else
    return NULL;
}

/*****************************************************************************/

/* This function verifies which scrollbar have been defined */
/* sbv: =0 has NO vertical scrollbar
        =1        vertical scrollbar
   sbh: =0 has NO horizontal scrollbar
        =1        horizontal scrollbar                      */
static void iMatrixGetSB(Ihandle* ih, int* sbh, int* sbv)
{
 char* sb = IupGetAttribute(ih, "SCROLLBAR");

 if (sb == NULL)
   sb = "YES";

 *sbh = *sbv = 0;

 if (iupStrEqualNoCase(sb, "YES") || iupStrEqualNoCase(sb, "ON"))
  *sbh = *sbv = 1;
 else if (iupStrEqualNoCase (sb, "VERTICAL"))
  *sbv = 1;
 else if (iupStrEqualNoCase (sb, "HORIZONTAL"))
  *sbh = 1;
}

/* This function calculates the size of the matrix */
static void iMatrixSize(Ihandle* ih)
{
  int w, h;

  cdCanvasGetSize(ih->data->cddbuffer, &w, &h, NULL, NULL);

  /* Matrix useful area is the current size minus the title area */
  ih->data->col.size= w - ih->data->col.titlewh;
  ih->data->lin.size= h - ih->data->lin.titlewh;

  ih->data->XmaxC = w - 1;
  ih->data->YmaxC = h - 1;
}

/* This function gets the initial cell values of the matrix, just those
   defined by attributes */
static void iMatrixGetInitialValues(Ihandle* ih)
{
  int i, j;
  char* value;
  char* attr = iupStrGetMemory(100);

  if(ih->data->valcb)
    return;

  for (i = 0; i < ih->data->lin.num; i++)
  {
    for (j = 0; j < ih->data->col.num; j++)
    {
      sprintf(attr, "%d:%d", i+1, j+1);
      value = (char*)IupGetAttribute(ih, attr);
      if (value && *value)
      {
        iMatrixMemAlocCell(ih, i, j, strlen(value));
        strcpy(ih->data->v[i][j].value, value);
      }
    }
  }
}

/* Start the matrix data */
static void iMatrixAllocMatrix(Ihandle* ih)
{
  /* Test if the value callback is defined */
  ih->data->valcb      = (sIFnii)IupGetCallback(ih, "VALUE_CB");
  ih->data->valeditcb  = (IFniis)IupGetCallback(ih, "VALUE_EDIT_CB");
  ih->data->markcb     =  (IFnii)IupGetCallback(ih, "MARK_CB");
  ih->data->markeditcb = (IFniii)IupGetCallback(ih, "MARKEDIT_CB");

  iMatrixMemAloc(ih);

  ih->data->col.first   = 0;
  ih->data->lin.first   = 0;
  ih->data->col.pos = 0;
  ih->data->lin.pos = 0;

  ih->data->col.active  = 0;
  ih->data->lin.active  = 0;

  ih->data->MarkLinCol = -1;

  iMatrixGetLastWidth(ih, IMATRIX_MAT_COL);
  iMatrixGetLastWidth(ih, IMATRIX_MAT_LIN);

  iMatrixGetInitialValues(ih);

  /* default is text */
  ih->data->datah = ih->data->texth;
}

static void iMatrixInitMatrix(Ihandle* ih)
{
  /* Calculate the size of titles */
  ih->data->col.titlewh = iMatrixGetTitlelineSize(ih);
  ih->data->lin.titlewh = iMatrixGetTitlecolumnSize(ih);

  iMatrixSize(ih);

  iMatrixFillWidthVec(ih);
  iMatrixFillHeightVec(ih);

  iMatrixMarkSet(ih, iupAttribGetStr(ih, "MARKED"));
}

/*****************************************************************************/
/*   Callbacks registered to the Canvas                                      */
/*****************************************************************************/

static void iMatrixCreateCanvas(Ihandle* ih)
{
  /* update canvas size */
  cdCanvasActivate(ih->data->cdcanvas);

  /* this can fail if canvas size is zero */
  ih->data->cddbuffer = cdCreateCanvas(CD_DBUFFER, ih->data->cdcanvas);

  if (!ih->data->cddbuffer)
    return;

  /* update canvas size */
  cdCanvasActivate(ih->data->cddbuffer);

  iMatrixInitMatrix(ih);
}

/* Callback used when the matrix has its size changed
   -> dx, dy : current size of the canvas, in pixels.  */
static int iMatrixResizeCB(Ihandle* ih, int dx, int dy)
{
  if (!ih->data->cddbuffer)
    iMatrixCreateCanvas(ih);

  if (!ih->data->cddbuffer)
    return IUP_DEFAULT;

  /* just update internal canvas properties,
     so we do not need to reactivate the same canvas. */
  cdCanvasActivate(ih->data->cddbuffer);

  ih->data->XmaxC = dx - 1;
  ih->data->YmaxC = dy - 1;

  /* store the size of the matrix useful area, i.e.,
     the size of canvas minus the line/column title area  */
  ih->data->col.size = dx - ih->data->col.titlewh;
  ih->data->lin.size = dy - ih->data->lin.titlewh;

  iMatrixGetLastWidth(ih, IMATRIX_MAT_COL);
  iMatrixGetLastWidth(ih, IMATRIX_MAT_LIN);

  iMatrixEditCheckHidden(ih);

  return IUP_DEFAULT;
}

/* Callback used when the matrix needs to be repaint */
static int iMatrixRepaintCB(Ihandle* ih)
{
  if (!ih->data->cddbuffer)
    return IUP_DEFAULT;

  iMatrixSize(ih);

  SetSbV(ih);
  SetSbH(ih);

  iMatrixDrawMatrix(ih, IMATRIX_DRAW_ALL);
  iMatrixShowFocus(ih);

  cdCanvasFlush(ih->data->cddbuffer);
  ih->data->redraw = 0;    /* flush always here */

  return IUP_DEFAULT;
}

/***************************************************************************/

static int iMatrixCreateMethod(Ihandle* ih, void **params)
{
  if (params && params[0])
  {
    char* action = (char*)params[0];
    iupAttribStoreStr(ih, "ACTION_CB", action);
  }

  /* free the data allocated by IupCanvas */
  if (ih->data) free(ih->data);
  ih->data = iupALLOCCTRLDATA();

  /* IupCanvas callbacks */
  IupSetCallback(ih, "ACTION",      (Icallback)iMatrixRepaintCB);
  IupSetCallback(ih, "RESIZE_CB",   (Icallback)iMatrixResizeCB);
  IupSetCallback(ih, "BUTTON_CB",   (Icallback)iMatrixMouseButtonCB);
  IupSetCallback(ih, "MOTION_CB",   (Icallback)iMatrixMouseMoveCB);
  IupSetCallback(ih, "KEYPRESS_CB", (Icallback)iMatrixKeyPressCB);
  IupSetCallback(ih, "FOCUS_CB",    (Icallback)iMatrixFocusCB);
  IupSetCallback(ih, "SCROLL_CB",   (Icallback)iMatrixScrollCB);

  iMatrixAllocMatrix(ih);

  /* Create the edit fields */
  /* TODO: iMatrixEditCreate(ih); */

  /* change the IupCanvas default values */
  iupAttribSetStr(ih, "SCROLLBAR", "YES");
  iupAttribSetStr(ih, "BORDER", "NO");

  return IUP_NOERROR;
}

static void iMatrixDestroyMethod(Ihandle* ih)
{
  if(ih->data->cddbuffer)
    cdKillCanvas(ih->data->cddbuffer);

  if(ih->data->cdcanvas)
    cdKillCanvas(ih->data->cdcanvas);

  if(ih->data->v)
  {
    int lin, col;
    for(lin = 0; lin < ih->data->lin.numaloc; lin++)
    {
      for(col = 0; col < ih->data->col.numaloc; col++)
        if(ih->data->v[lin][col].value)
          free(ih->data->v[lin][col].value);
      free(ih->data->v[lin]);
    }
    free(ih->data->v);
  }

  if(ih->data->col.wh)
    free(ih->data->col.wh);
  if(ih->data->lin.wh)
    free(ih->data->lin.wh);

  if(ih->data->col.marked)
    free(ih->data->col.marked);
  if(ih->data->lin.marked)
    free(ih->data->lin.marked);

  if(ih->data->lin.inactive) 
    free(ih->data->lin.inactive); 
  if(ih->data->col.inactive) 
    free(ih->data->col.inactive); 
}

static int iMatrixMapMethod(Ihandle* ih)
{
  /* ensure the default values */
  IupSetAttribute(ih, "CURSOR", "IupMatrixCrossCursor");

  ih->data->cdcanvas = cdCreateCanvas(CD_IUP, ih);
  if (!ih->data->cdcanvas)
    return IUP_ERROR;

  /* this can fail if canvas size is zero */
  ih->data->cddbuffer = cdCreateCanvas(CD_DBUFFER, ih->data->cdcanvas);

  iMatrixInitMatrix(ih);

  return IUP_NOERROR;
}

static int iMatrixGetNaturalWidth(Ihandle* ih)
{
  int titlesize, colsize;
  int numcolv, i;

  titlesize = iMatrixGetTitlelineSize(ih);

  numcolv = iupAttribGetIntDefault(ih, "NUMCOL_VISIBLE");
  colsize = 0;

  for(i = 0; i < numcolv; i++)
    colsize += iMatrixGetColumnWidth(ih, i);

  return colsize + titlesize;
}

static int iMatrixGetNaturalHeight(Ihandle* ih)
{
  int titlesize, linsize;
  int numlinv,i;

  titlesize = iMatrixGetTitlecolumnSize(ih);

  numlinv = iupAttribGetIntDefault(ih, "NUMLIN_VISIBLE");
  linsize = 0;

  for(i = 0; i < numlinv; i++)
    linsize += iMatrixGetLineHeight(ih, i);

  return linsize + titlesize;
}

static int iMatrixComputeNaturalSizeMethod(Ihandle* ih)
{
  int width, height, sizex, sizey;
  int sbh, sbv, sbw = 0;

  /* Get scrollbar parameters */
  iMatrixGetSB(ih, &sbh, &sbv);
  if (sbv || sbh)
    sbw = iupdrvGetScrollbarSize();

  /* For that iMatrixGetNaturalWidth and iMatrixGetNaturalHeight functions are
  able to call the calculation of width functions, ih->data->lin.num, ih->data->col.num
  and ih->data->valcb values must be defined... whether the canvas still has not
  been created, these values also have not been initialized...
  */
  if(!ih->data->cddbuffer)
    ih->data->valcb   = (sIFnii)IupGetCallback(ih, "VALUE_CB");

  /* Get the value of the SIZE attribute */
  sizex = ih->userwidth;
  sizey = ih->userheight;

  /* Get the natural width */
  if(sizex)
    width = sizex;
  else
    width = iMatrixGetNaturalWidth(ih) + sbw * sbv;

  /* Get the natural height */
  if(sizey)
    height = sizey;
  else
    height = iMatrixGetNaturalHeight(ih) + sbw * sbh;

  ih->naturalwidth  = width;
  ih->naturalheight = height;

  return 3; /* To resize in all directions */
}

static void iMatrixCreateCursor(void)
{
  Ihandle *imgcursor;
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

  imgcursor = IupImage(15, 15, matrx_img_cur_excel);
  IupSetAttribute(imgcursor, "0", "BGCOLOR"); 
  IupSetAttribute(imgcursor, "1", "0 0 0"); 
  IupSetAttribute(imgcursor, "2", "255 255 255"); 
  IupSetAttribute(imgcursor, "HOTSPOT", "7:7");     /* Centered Hotspot           */
  IupSetHandle("IupMatrixCrossCursor", imgcursor); 
  IupSetHandle("matrx_img_cur_excel",  imgcursor);  /* for backward compatibility */
}

Iclass* iupMatrixGetClass(void)
{
  Iclass* ic = iupClassNew(iupCanvasGetClass());

  ic->name = "matrix";
  ic->format = "A"; /* one optional callback name */
  ic->nativetype = IUP_TYPECANVAS;
  ic->childtype = IUP_CHILDNONE;
  ic->is_interactive = 1;
  ic->has_attrib_id = 1;   /* has attributes with IDs that must be parsed */

  /* Class functions */
  ic->Create  = iMatrixCreateMethod;
  ic->Destroy = iMatrixDestroyMethod;
  ic->Map     = iMatrixMapMethod;
  ic->ComputeNaturalSize = iMatrixComputeNaturalSizeMethod;

  /* Do not need to set base attributes because they are inherited from IupCanvas */

  /* IupMatrix Callbacks */
  /* --- Interaction --- */
  iupClassRegisterCallback(ic, "ACTION_CB",  "iiiic");
  iupClassRegisterCallback(ic, "CLICK_CB",   "iic");
  iupClassRegisterCallback(ic, "RELEASE_CB", "iic");
  iupClassRegisterCallback(ic, "MOUSEMOVE_CB", "ii");
  iupClassRegisterCallback(ic, "ENTERITEM_CB", "ii");
  iupClassRegisterCallback(ic, "LEAVEITEM_CB", "ii");
  iupClassRegisterCallback(ic, "SCROLLTOP_CB", "ii");
  /* --- Drawing --- */
  iupClassRegisterCallback(ic, "BGCOLOR_CB", "iiIII");
  iupClassRegisterCallback(ic, "FGCOLOR_CB", "iiIII");
  iupClassRegisterCallback(ic, "DRAW_CB", "iiiiii");
  iupClassRegisterCallback(ic, "DROPCHECK_CB", "ii");
  /* --- Editing --- */
  iupClassRegisterCallback(ic, "DROP_CB", "nii");
  iupClassRegisterCallback(ic, "DROPSELECT_CB", "iinsii");
  iupClassRegisterCallback(ic, "EDITION_CB", "iii");
  /* --- Callback Mode --- */
  iupClassRegisterCallback(ic, "VALUE_CB", "ii=s");
  iupClassRegisterCallback(ic, "VALUE_EDIT_CB", "iic");
  iupClassRegisterCallback(ic, "MARK_CB", "ii");
  iupClassRegisterCallback(ic, "MARKEDIT_CB", "iii");

  /* IupMatrix Attributes - CELL */
  iupClassRegisterAttributeId(ic, "IDVALUE", iMatrixGetMatrixCellAttrib, iMatrixSetMatrixCellAttrib, NULL, IUP_NOT_MAPPED, IUP_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "ALIGNMENT", NULL, iMatrixSetAlignmentAttrib, "ALEFT", IUP_NOT_MAPPED, IUP_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "FGCOLOR", iMatrixGetFgColorAttrib, iMatrixSetFgColorAttrib, "0 0 0", IUP_NOT_MAPPED, IUP_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "BGCOLOR", iMatrixGetBgColorAttrib, iMatrixSetBgColorAttrib, "255 255 255", IUP_NOT_MAPPED, IUP_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "FONT", iMatrixGetFontAttrib, iMatrixSetFontAttrib, NULL, IUP_NOT_MAPPED, IUP_NO_INHERIT);
  iupClassRegisterAttribute(ic, "FOCUS_CELL", iMatrixGetFocusCellAttrib, iMatrixSetFocusCellAttrib, "1:1", IUP_NOT_MAPPED, IUP_NO_INHERIT);
  iupClassRegisterAttribute(ic, "VALUE", iMatrixGetValueAttrib, iMatrixSetValueAttrib, NULL, IUP_NOT_MAPPED, IUP_NO_INHERIT);

  /* IupMatrix Attributes - SIZE */
  iupClassRegisterAttribute(ic, "NUMLIN", iMatrixGetNumLinAttrib, iMatrixSetNumLinAttrib, "0", IUP_NOT_MAPPED, IUP_NO_INHERIT);
  iupClassRegisterAttribute(ic, "NUMCOL", iMatrixGetNumColAttrib, iMatrixSetNumColAttrib, "0", IUP_NOT_MAPPED, IUP_NO_INHERIT);
  iupClassRegisterAttribute(ic, "NUMLIN_VISIBLE", iMatrixGetNumLinVisibleAttrib, NULL, "3", IUP_NOT_MAPPED, IUP_NO_INHERIT);
  iupClassRegisterAttribute(ic, "NUMCOL_VISIBLE", iMatrixGetNumColVisibleAttrib, NULL, "4", IUP_NOT_MAPPED, IUP_NO_INHERIT);
  iupClassRegisterAttribute(ic, "WIDTHDEF", NULL, NULL, "80", IUP_NOT_MAPPED, IUP_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "WIDTH", iMatrixGetWidthAttrib, iMatrixSetWidthAttrib, NULL, IUP_NOT_MAPPED, IUP_NO_INHERIT);
  iupClassRegisterAttribute(ic, "HEIGHTDEF", NULL, NULL, "8", IUP_NOT_MAPPED, IUP_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "HEIGHT", iMatrixGetHeightAttrib, iMatrixSetHeightAttrib, NULL, IUP_NOT_MAPPED, IUP_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "RASTERWIDTH", iMatrixGetRasterWidthAttrib, iMatrixSetRasterWidthAttrib, NULL, IUP_NOT_MAPPED, IUP_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "RASTERHEIGHT", iMatrixGetRasterHeightAttrib, iMatrixSetRasterHeightAttrib, NULL, IUP_NOT_MAPPED, IUP_NO_INHERIT);

  /* IupMatrix Attributes - MARK */
  iupClassRegisterAttribute(ic, "MARKED", iMatrixGetMarkedAttrib, iMatrixSetMarkedAttrib, NULL, IUP_NOT_MAPPED, IUP_NO_INHERIT);
  iupClassRegisterAttribute(ic, "MARK_MODE", NULL, iMatrixSetMarkModeAttrib, NULL, IUP_NOT_MAPPED, IUP_NO_INHERIT);
  iupClassRegisterAttribute(ic, "AREA", NULL, iMatrixSetAreaAttrib, "CONTINUOUS", IUP_NOT_MAPPED, IUP_NO_INHERIT);
  iupClassRegisterAttribute(ic, "MULTIPLE", NULL, iMatrixSetMultipleAttrib, NULL, IUP_NOT_MAPPED, IUP_NO_INHERIT);

  /* IupMatrix Attributes - ACTION */
  iupClassRegisterAttribute(ic, "ADDLIN", NULL, iMatrixSetAddLinAttrib, NULL, IUP_NOT_MAPPED, IUP_NO_INHERIT);
  iupClassRegisterAttribute(ic, "DELLIN", NULL, iMatrixSetDelLinAttrib, NULL, IUP_NOT_MAPPED, IUP_NO_INHERIT);
  iupClassRegisterAttribute(ic, "ADDCOL", NULL, iMatrixSetAddColAttrib, NULL, IUP_NOT_MAPPED, IUP_NO_INHERIT);
  iupClassRegisterAttribute(ic, "DELCOL", NULL, iMatrixSetDelColAttrib, NULL, IUP_NOT_MAPPED, IUP_NO_INHERIT);
  iupClassRegisterAttribute(ic, "ORIGIN", iMatrixGetOriginAttrib, iMatrixSetOriginAttrib, NULL, IUP_NOT_MAPPED, IUP_NO_INHERIT);
  iupClassRegisterAttribute(ic, "EDIT_MODE", iMatrixGetEditModeAttrib, iMatrixSetEditModeAttrib, NULL, IUP_NOT_MAPPED, IUP_NO_INHERIT);
  iupClassRegisterAttribute(ic, "REDRAW", NULL, iMatrixSetRedrawAttrib, NULL, IUP_NOT_MAPPED, IUP_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "ACTIVELIN", NULL, iMatrixSetActiveLinAttrib, NULL, IUP_NOT_MAPPED, IUP_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "ACTIVECOL", NULL, iMatrixSetActiveColAttrib, NULL, IUP_NOT_MAPPED, IUP_NO_INHERIT);

  /* IupMatrix Attributes - GENERAL */
  iupClassRegisterAttribute(ic, "CARET", iMatrixGetCaretAttrib, iMatrixSetCaretAttrib, NULL, IUP_NOT_MAPPED, IUP_NO_INHERIT);
  iupClassRegisterAttribute(ic, "SELECTION", iMatrixGetSelectionAttrib, iMatrixSetSelectionAttrib, NULL, IUP_NOT_MAPPED, IUP_NO_INHERIT);

  iupClassRegisterAttribute(ic, "CHECKFRAMECOLOR", iMatrixGetCheckFrameColorAttrib, iMatrixSetCheckFrameColorAttrib, NULL, IUP_NOT_MAPPED, IUP_NO_INHERIT);
  iupClassRegisterAttribute(ic, "FRAMECOLOR", NULL, NULL, "100 100 100", IUP_NOT_MAPPED, IUP_NO_INHERIT);

  iupClassRegisterAttribute(ic, "ACTIVE", iupBaseGetActiveAttrib, iMatrixSetActiveAttrib, "YES", IUP_MAPPED, IUP_INHERIT);

  /* IupMatrix Attributes - MASK */
  iupClassRegisterAttribute(ic, "_IUPMASK_DATA", iMatrixGetMaskDataAttrib, iupBaseNoSetAttrib, NULL, IUP_NOT_MAPPED, IUP_NO_INHERIT);

  if (!IupGetHandle("IupMatrixCrossCursor"))
    iMatrixCreateCursor();

  return ic;
}


/*****************************************************************************************************/


Ihandle* IupMatrix(const char* action)
{
  void *params[2];
  params[0] = (void*)action;
  params[1] = NULL;
  return IupCreatev("matrix", params);
}

void IupMatSetAttribute(Ihandle* ih, const char* a, int l, int c, char* v)
{
  char* attr = iupStrGetMemory(100);
  sprintf(attr, "%s%d:%d", a, l, c);
  IupSetAttribute(ih, attr, v);
}

void IupMatStoreAttribute(Ihandle* ih, const char* a, int l, int c, char* v)
{
  char* attr = iupStrGetMemory(100);
  sprintf(attr, "%s%d:%d", a, l, c);
  IupStoreAttribute(ih, attr, v);
}

char* IupMatGetAttribute(Ihandle* ih, const char* a, int l, int c)
{
  char* attr = iupStrGetMemory(100);
  sprintf(attr, "%s%d:%d", a, l, c);
  return IupGetAttribute(ih, attr);
}

int IupMatGetInt(Ihandle* ih, const char* a, int l, int c)
{
  char* attr = iupStrGetMemory(100);
  sprintf(attr, "%s%d:%d", a, l, c);
  return IupGetInt(ih, attr);
}

float IupMatGetFloat(Ihandle* ih, const char* a, int l, int c)
{
  char* attr = iupStrGetMemory(100);
  sprintf(attr, "%s%d:%d", a, l, c);
  return IupGetFloat(ih, attr);
}

void IupMatSetfAttribute(Ihandle* ih, const char* a, int l, int c, char* f, ...)
{
  static char v[SHRT_MAX];
  char* attr = iupStrGetMemory(100);
  va_list arglist;
  sprintf(attr, "%s%d:%d", a, l, c);
  va_start(arglist, f);
  vsprintf(v, f, arglist);
  va_end(arglist);
  IupStoreAttribute(ih, attr, v);
}
