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


void iupMatrixExCheckLimitsOrder(int *v1, int *v2, int min, int max)
{
  if (*v1<min) *v1 = min;
  if (*v2<min) *v2 = min;
  if (*v1>max) *v1 = max;
  if (*v2>max) *v2 = max;
  if (*v1>*v2) {int v=*v1; *v1=*v2; *v2=v;}
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
    IupSetAttributeId2(ih,"FRAMEHORIZCOLOR", lin, IUP_INVALID_ID, NULL);
    IupSetAttributeId2(ih,"FRAMEVERTCOLOR", IUP_INVALID_ID, col, NULL);

    IupSetAttribute(ih,"NUMLIN_NOSCROLL","0");
    IupSetAttribute(ih,"NUMCOL_NOSCROLL","0");
  }
  else
  {
    char* fzcolor = IupGetAttribute(ih, "FREEZECOLOR");

    IupSetInt(ih,"NUMLIN_NOSCROLL",lin);
    IupSetInt(ih,"NUMCOL_NOSCROLL",col);  

    IupSetAttributeId2(ih,"FRAMEHORIZCOLOR", lin, IUP_INVALID_ID, fzcolor);
    IupSetAttributeId2(ih,"FRAMEVERTCOLOR",IUP_INVALID_ID, col, fzcolor);
  }

  IupSetAttribute(ih,"REDRAW","ALL");
  return 1;  /* store freeze state */
}

static int iMatrixExCreateMethod(Ihandle* ih, void **params)
{
  ImatExData* matex_data = (ImatExData*)malloc(sizeof(ImatExData));
  memset(matex_data, 0, sizeof(ImatExData));

  iupAttribSet(ih, "_IUP_MATEX_DATA", (char*)matex_data);
  matex_data->ih = ih;

  (void)params;
  return IUP_NOERROR;
}

static void iMatrixExDestroyMethod(Ihandle* ih)
{
  ImatExData* matex_data = (ImatExData*)iupAttribGet(ih, "_IUP_MATEX_DATA");
  if (matex_data->busy_progress)
    IupDestroy(matex_data->busy_progress);
  if (matex_data->undo_stack)
  {
    iupAttribSetClassObject(ih, "UNDOCLEAR", NULL);
    iupArrayDestroy(matex_data->undo_stack);
  }
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
  iupMatrixExRegisterCopy(ic);
  iupMatrixExRegisterUnits(ic);
  iupMatrixExRegisterUndo(ic);
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
