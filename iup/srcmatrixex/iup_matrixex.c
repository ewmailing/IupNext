/** \file
 * \brief IupMatrix Expansion Library.
 *
 * See Copyright Notice in "iup.h"
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "iup.h"
#include "iupcbs.h"
#include "iupcontrols.h"
#include "iupmatrixex.h"

#include "iup_object.h"
#include "iup_childtree.h"
#include "iup_register.h"
#include "iup_attrib.h"
#include "iup_str.h"
#include "iup_assert.h"
#include "iup_matrixex.h"


static int iMatrixExBusyProgressCancel_CB(Ihandle* ih)
{
  ImatExData* matex_data = (ImatExData*)iupAttribGet(ih, "_IUP_MATEX_DATA");
  matex_data->busy_progress_abort = 1;
  return IUP_DEFAULT;
}

void iupMatrixExBusyStart(Ihandle* ih, int count, const char* busyname)
{
  ImatExData* matex_data = (ImatExData*)iupAttribGet(ih, "_IUP_MATEX_DATA");

  /* can not start a new one if already busy */
  iupASSERT(matex_data->busy);
  if (matex_data->busy)
    return;

  matex_data->busy = 1;
  matex_data->busy_count = 0;

  matex_data->busy_cb = (IFniis)IupGetCallback(ih, "BUSY_CB");
  if (matex_data->busy_cb)
    matex_data->busy_cb(ih, 1, count, (char*)busyname);

  if (iupAttribGetBoolean(ih, "BUSYPROGRESS"))
  {
    int x, y;

    if (!matex_data->busy_progress)
    {
      matex_data->busy_progress = IupProgressDlg();
      IupSetCallback(matex_data->busy_progress, "CANCEL_CB", iMatrixExBusyProgressCancel_CB);
      IupSetAttributeHandle(matex_data->busy_progress, "PARENTDIALOG", IupGetDialog(ih));
      IupSetAttribute(matex_data->busy_progress, "_IUP_MATEX_DATA", (char*)matex_data);

      IupMap(matex_data->busy_progress); /* to compute dialog size */
    }
  
    IupStoreAttribute(matex_data->busy_progress, "DESCRIPTION", busyname);
    IupSetfAttribute(matex_data->busy_progress, "TOTALCOUNT", "%d", count);
    IupSetAttribute(matex_data->busy_progress, "COUNT", "0");

    IupRefresh(matex_data->busy_progress);

    x = IupGetInt(ih, "X") + (ih->currentwidth-matex_data->busy_progress->currentwidth)/2;
    y = IupGetInt(ih, "Y") + (ih->currentheight-matex_data->busy_progress->currentheight)/2;
    IupShowXY(matex_data->busy_progress, x, y);

    matex_data->busy_progress_abort = 0;
    matex_data->busy = 2;
  }
}

int iupMatrixExBusyInc(Ihandle* ih)
{
  ImatExData* matex_data = (ImatExData*)iupAttribGet(ih, "_IUP_MATEX_DATA");
  if (matex_data->busy)
  {
    matex_data->busy_count++;

    if (matex_data->busy_cb)
    {
      int ret = matex_data->busy_cb(ih, 2, matex_data->busy_count, NULL);
      if (ret == IUP_IGNORE)
      {
        iupMatrixExBusyEnd(ih);
        return 0;
      }
    }

    if (matex_data->busy == 2)
    {
      IupSetAttribute(matex_data->busy_progress, "INC", NULL);

      if (matex_data->busy_progress_abort)
      {
        iupMatrixExBusyEnd(ih);
        return 0;
      }
    }
  }
  return 1;
}

void iupMatrixExBusyEnd(Ihandle* ih)
{
  ImatExData* matex_data = (ImatExData*)iupAttribGet(ih, "_IUP_MATEX_DATA");
  if (matex_data->busy)
  {
    if (matex_data->busy_cb)
      matex_data->busy_cb(ih, 0, 0, NULL);

    if (matex_data->busy == 2)
      IupHide(matex_data->busy_progress);

    matex_data->busy_count = 0;
    matex_data->busy_cb = NULL;
    matex_data->busy = 0;
  }
}

static int iMatrixSetBusyAttrib(Ihandle* ih, const char* value)
{
  /* can only be canceled */
  ImatExData* matex_data = (ImatExData*)iupAttribGet(ih, "_IUP_MATEX_DATA");
  if (matex_data->busy && !iupStrBoolean(value))
    iupMatrixExBusyEnd(ih);
  return 0;
}

static char* iMatrixGetBusyAttrib(Ihandle* ih)
{
  ImatExData* matex_data = (ImatExData*)iupAttribGet(ih, "_IUP_MATEX_DATA");
  if (matex_data->busy)
    return "Yes";
  else
    return "No";
}

int iupMatrixExIsColumnVisible(Ihandle* ih, int col)
{
  int width = 0;
  char str[100];
  char* value;

  if (col==0)
    return (IupGetIntId(ih, "RASTERWIDTH", 0) != 0);

  /* to be invisible must exist the attribute and must be set to 0 (zero), 
     or else is visible */

  sprintf(str, "WIDTH%d", col);
  value = iupAttribGet(ih, str);
  if (!value)
  {
    sprintf(str, "RASTERWIDTH%d", col);
    value = iupAttribGet(ih, str);
    if (!value)
      return 1;
  }

  if (iupStrToInt(value, &width)==1)
  {
    if (width==0)
      return 0;
  }

  return 1;
}

int iupMatrixExIsLineVisible(Ihandle* ih, int lin)
{
  int height = 0;
  char str[100];
  char* value;

  if (lin==0)
    return (IupGetIntId(ih, "RASTERHEIGHT", 0) != 0);

  sprintf(str, "HEIGHT%d", lin);
  value = iupAttribGet(ih, str);
  if(!value)
  {
    sprintf(str, "RASTERHEIGHT%d", lin);
    value = iupAttribGet(ih, str);
    if(!value)
      return 1;
  }

  if (iupStrToInt(value, &height)==1)
  {
    if (height==0)
      return 0;
  }

  return 1;
}

char* iupMatrixExGetCell(Ihandle* ih, int lin, int col, sIFnii value_cb)
{
  char* value;
  if (value_cb)
    value = value_cb(ih, lin, col);
  else
    value = IupGetAttributeId2(ih, "", lin, col);
  return value;
}

void iupMatrixExSetCell(Ihandle *ih, int lin, int col, const char* value, IFniiii edition_cb, IFniis value_edit_cb)
{
  if (edition_cb && edition_cb(ih,lin,col,1,1)==IUP_IGNORE)
    return;

  if (value_edit_cb)
    value_edit_cb(ih,lin,col,(char*)value);
  else
    IupSetAttributeId2(ih,"",lin,col,value);
}

#if 0
int Dmatrix::_ACT_mtx   (Ihandle *h,int c,int lin,int col,int active,char *after)
{
  Dmatrix *D = Dmatrix::GetSelf(h);
  if (!active)
  {
    switch (c)
    {
    case K_cA: { D->SelectAll(); return IUP_CONTINUE;}
    case K_cC: { D->Copy()     ; return IUP_IGNORE  ;}
    case K_cV: { D->Paste()    ; return IUP_IGNORE  ;}
    case K_cZ: { Dmatrix::_ACT_it_undo(h)   ; return IUP_IGNORE  ;}
    case K_cY: { Dmatrix::_ACT_it_redo(h)   ; return IUP_IGNORE  ;}
    case K_cU: { D->UndoList() ; return IUP_IGNORE  ;}
    case K_sUP   :{D->selblock.SetEnd(lin-1,col);D->selblock.Select(h);break;}
    case K_sDOWN :{D->selblock.SetEnd(lin+1,col);D->selblock.Select(h);break;}
    case K_sRIGHT:{D->selblock.SetEnd(lin,col+1);D->selblock.Select(h);break;}
    case K_sLEFT :{D->selblock.SetEnd(lin,col-1);D->selblock.Select(h);break;}
    case K_UP    :{D->selblock.SetAnchor(lin-1,col);break;}
    case K_DOWN  :{D->selblock.SetAnchor(lin+1,col);break;}
    case K_RIGHT :{D->selblock.SetAnchor(lin,col+1);break;}
    case K_LEFT  :{D->selblock.SetAnchor(lin,col-1);break;}
    case K_cF: {D->Find(); return IUP_IGNORE;}
    case K_mF3:{D->Find(); return IUP_IGNORE;}
    case K_F3: {D->FindNext();return IUP_IGNORE;}
    case K_sF3:{D->FindPrev();return IUP_IGNORE;}
    case K_ESC:{D->finder.CloseDlg(); return IUP_CONTINUE;}
    }
  }
  if (D->_act_mtx_client!=NULL)
    return ((ActFcnType)D->_act_mtx_client)(h,c,lin,col,active,after);
  return IUP_DEFAULT;
}
#endif

static int iMatrixExSetFreezeAttrib(Ihandle *ih, const char* value)
{
  int freeze, lin, col;

  if (iupStrBoolean(value))
  {
    IupGetIntInt(ih, "FOCUS_CELL", &lin, &col);
    freeze = 1;
  }
  else
  {
    if (iupStrToIntInt(value, &lin, &col, ':')==2)
      freeze = 1;
    else
      freeze = 0;
  }

  if (!freeze)
  {
    lin = IupGetInt(ih,"NUMLIN_NOSCROLL");
    col = IupGetInt(ih,"NUMCOL_NOSCROLL");
    IupSetAttributeId2(ih,"FRAMEHORIZCOLOR", lin, -1, NULL);
    IupSetAttributeId2(ih,"FRAMEVERTCOLOR", -1, col, NULL);

    IupSetAttribute(ih,"NUMLIN_NOSCROLL","0");
    IupSetAttribute(ih,"NUMCOL_NOSCROLL","0");

//    iupMatrixExPushUndoCmd(ih, "FREEZE=%d:%d", lin, col);
  }
  else
  {
    char* fzcolor = IupGetAttribute(ih, "FREEZECOLOR");

    IupSetfAttribute(ih,"NUMLIN_NOSCROLL","%d",lin);
    IupSetfAttribute(ih,"NUMCOL_NOSCROLL","%d",col);  

    IupSetAttributeId2(ih,"FRAMEHORIZCOLOR", lin, -1, fzcolor);
    IupSetAttributeId2(ih,"FRAMEVERTCOLOR",-1, col, fzcolor);

 //   iupMatrixExPushUndoCmd(ih, "FREEZE=NO");
  }

  IupSetAttribute(ih,"REDRAW","ALL");
  return 1;  /* store freeze state */
}

static int iMatrixExCreateMethod(Ihandle* ih, void **params)
{
  ImatExData* matex_data = (ImatExData*)malloc(sizeof(ImatExData));
  memset(matex_data, 0, sizeof(ImatExData));
  iupAttribSetStr(ih, "_IUP_MATEX_DATA", (char*)matex_data);

  (void)params;
  return IUP_NOERROR;
}

static void iMatrixExDestroyMethod(Ihandle* ih)
{
  ImatExData* matex_data = (ImatExData*)iupAttribGet(ih, "_IUP_MATEX_DATA");
  if (matex_data->busy_progress)
    IupDestroy(matex_data->busy_progress);
  free(matex_data);
}

static void iMatrixExInitAttribCb(Iclass* ic)
{
  iupClassRegisterAttribute(ic, "FREEZE", NULL, iMatrixExSetFreezeAttrib, NULL, NULL, IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "FREEZECOLOR", NULL, NULL, IUPAF_SAMEASSYSTEM, "0 0 255", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);

  iupClassRegisterAttribute(ic, "BUSY", iMatrixGetBusyAttrib, iMatrixSetBusyAttrib, NULL, NULL, IUPAF_NO_SAVE|IUPAF_NO_DEFAULTVALUE|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "BUSYPROGRESS", NULL, NULL, NULL, NULL, IUPAF_NO_INHERIT);

  iupMatrixExRegisterClipboard(ic);
}

static Iclass* iMatrixExNewClass(void)
{
  Iclass* ic = iupClassNew(iupRegisterFindClass("matrix"));

  ic->name = "matrixex";
  ic->format = "";
  ic->nativetype = IUP_TYPECANVAS;
  ic->childtype = IUP_CHILDNONE;
  ic->is_interactive = 1;
  ic->has_attrib_id = 2;   /* has attributes with IDs that must be parsed */

  /* Class functions */
  ic->New = iMatrixExNewClass;
  ic->Create  = iMatrixExCreateMethod;
  ic->Destroy  = iMatrixExDestroyMethod;
  
  iMatrixExInitAttribCb(ic);

  return ic;
}

void IupMatrixExInit(Ihandle* ih)
{
  if (ih->iclass->nativetype != IUP_TYPECANVAS || 
      !IupClassMatch(ih, "matrix"))
    return;
    
  iMatrixExInitAttribCb(ih->iclass);
}

void IupMatrixExOpen(void)
{
  if (!IupGetGlobal("_IUP_MATRIXEX_OPEN"))
  {
    iupRegisterClass(iMatrixExNewClass());
    IupSetGlobal("_IUP_MATRIXEX_OPEN", "1");
  }
}

Ihandle* IupMatrixEx(void)
{
  return IupCreate("matrixex");
}

