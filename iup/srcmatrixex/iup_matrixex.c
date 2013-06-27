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


void iupMatrixExInitDataAccess(ImatExData* matex_data)
{
  matex_data->value_cb = (sIFnii)IupGetCallback(matex_data->ih, "VALUE_CB");
  matex_data->value_edit_cb = (IFniis) IupGetCallback(matex_data->ih,"VALUE_EDIT_CB");
  matex_data->edition_cb  = (IFniiii)IupGetCallback(matex_data->ih,"EDITION_CB");
}

char* iupMatrixExGetCell(ImatExData* matex_data, int lin, int col)
{
  char* value;
  if (matex_data->value_cb)
    value = matex_data->value_cb(matex_data->ih, lin, col);
  else
    value = IupGetAttributeId2(matex_data->ih, "", lin, col);
  return value;
}

void iupMatrixExSetCell(ImatExData* matex_data, int lin, int col, const char* value)
{
  if (matex_data->edition_cb && matex_data->edition_cb(matex_data->ih,lin,col,1,1)==IUP_IGNORE)
    return;

  if (matex_data->value_edit_cb)
    matex_data->value_edit_cb(matex_data->ih,lin,col,(char*)value);
  else
    IupSetAttributeId2(matex_data->ih,"",lin,col,value);
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
  matex_data->ih = ih;

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

  iupMatrixExRegisterClipboard(ic);
  iupMatrixExRegisterBusy(ic);
  iupMatrixExRegisterVisible(ic);
  iupMatrixExRegisterExport(ic);
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

