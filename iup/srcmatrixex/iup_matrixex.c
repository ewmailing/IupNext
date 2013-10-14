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


static void iMatrixExSelectAll(Ihandle *ih)
{
  char* markmode = IupGetAttribute(ih, "MARKMODE");

  if (iupStrEqualNoCasePartial(markmode, "LIN")) /* matches also "LINCOL" */
  {
    int lin, num_lin = IupGetInt(ih, "NUMLIN");
    char* marked = malloc(num_lin+2);

    marked[0] = 'L';
    for(lin = 1; lin <= num_lin; ++lin)
      marked[lin] = '1';
    marked[lin] = 0;

    IupSetAttribute(ih, "MARKED", marked);

    free(marked);
  }
  else if (iupStrEqualNoCase(markmode, "COL"))
  {
    int col, num_col = IupGetInt(ih, "NUMCOL");
    char* marked = malloc(num_col+2);

    marked[0] = 'C';
    for(col = 1; col <= num_col; ++col)
      marked[col] = '1';
    marked[col] = 0;

    IupSetAttribute(ih, "MARKED", marked);

    free(marked);
  }
  else if (iupStrEqualNoCase(markmode, "CELL"))
  {
    int num_col = IupGetInt(ih, "NUMCOL");
    int num_lin = IupGetInt(ih, "NUMLIN");
    int pos, count = num_lin*num_col;
    char* marked = malloc(count+1);

    for(pos = 0; pos < count; pos++)
      marked[pos] = '1';
    marked[pos] = 0;

    IupSetAttribute(ih, "MARKED", marked);

    free(marked);
  }
}

void iupMatrixExCheckLimitsOrder(int *v1, int *v2, int min, int max)
{
  if (*v1<min) *v1 = min;
  if (*v2<min) *v2 = min;
  if (*v1>max) *v1 = max;
  if (*v2>max) *v2 = max;
  if (*v1>*v2) {int v=*v1; *v1=*v2; *v2=v;}
}

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

static IFniiiis iMatrixOriginalButton_CB = NULL;

static int iMatrixExButton_CB(Ihandle* ih, int b, int press, int x, int y, char* r)
{
  if (iMatrixOriginalButton_CB(ih, b, press, x, y, r)==IUP_IGNORE)
    return IUP_IGNORE;

  if (b == IUP_BUTTON2 && press && IupGetInt(ih, "MENUCONTEXT"))
  {
    Ihandle* menu;
    int lin, col;
    int pos = IupConvertXYToPos(ih, x, y);
    IupTextConvertPosToLinCol(ih, pos, &lin, &col);

    menu = iMatrixExCreateMenuContext(ih, lin, col);
  }

  return IUP_DEFAULT;
}

static IFnii iMatrixOriginalKeyPress_CB = NULL;

static int iMatrixExKeyPress_CB(Ihandle* ih, int c, int press)
{
  if (press)
  {
    switch (c)
    {
    case K_cA: 
      iMatrixExSelectAll(ih); 
      return IUP_CONTINUE;
    case K_cV: 
      if (!IupGetInt(ih, "READONLY"))
      {
        if (IupGetAttribute(ih,"MARKED"))
          IupSetAttribute(ih, "PASTE", "MARKED");
        else
          IupSetAttribute(ih, "PASTE", "FOCUS");
      }
      return IUP_IGNORE;
    case K_cC: 
      IupSetAttribute(ih, "COPY", "MARKED");
      return IUP_IGNORE;
    case K_cZ: 
      IupSetAttribute(ih, "UNDO", NULL);  /* 1 level */
      return IUP_IGNORE;
    case K_cY: 
      IupSetAttribute(ih, "REDO", NULL);  /* 1 level */
      return IUP_IGNORE;
    case K_F3: 
      {
        char* find = IupGetAttribute(ih, "FIND");
        if (find)
        {
          /* invert the direction if not a "forward" one */
          char* direction = iupAttribGet(ih, "FINDDIRECTION");
          if (iupStrEqualNoCase(direction, "LEFTTOP"))
            iupAttribSet(ih, "FINDDIRECTION", "RIGHTBOTTOM");
          else if (iupStrEqualNoCase(direction, "TOPLEFT"))
            iupAttribSet(ih, "FINDDIRECTION", "BOTTOMRIGHT");

          IupSetAttribute(ih, "FIND", find);
        }
        return IUP_IGNORE;
      }
    case K_sF3: 
      {
        char* find = IupGetAttribute(ih, "FIND");
        if (find)
        {
          /* invert the direction if not a "reverse" one */
          char* direction = iupAttribGet(ih, "FINDDIRECTION");
          if (iupStrEqualNoCase(direction, "RIGHTBOTTOM"))
            iupAttribSet(ih, "FINDDIRECTION", "LEFTTOP");
          else if (iupStrEqualNoCase(direction, "BOTTOMRIGHT"))
            iupAttribSet(ih, "FINDDIRECTION", "TOPLEFT");

          IupSetAttribute(ih, "FIND", find);
        }
        return IUP_IGNORE;
      }
    case K_cF: 
    case K_mF3: 
      {
        ImatExData* matex_data = (ImatExData*)iupAttribGet(ih, "_IUP_MATEX_DATA");
        iupMatrixExFindInitDialog(matex_data);
        iupMatrixExFindShowDialog(matex_data);
        return IUP_IGNORE;
      }
    case K_ESC: 
      {
        ImatExData* matex_data = (ImatExData*)iupAttribGet(ih, "_IUP_MATEX_DATA");
        if (matex_data->find_dlg)
          IupHide(matex_data->find_dlg);
        return IUP_CONTINUE;
      }
#if 0
    case K_cU: { D->UndoList() ; return IUP_IGNORE  ;}
#endif
    }
  }

  return iMatrixOriginalKeyPress_CB(ih, c, press);
}

static int iMatrixExCreateMethod(Ihandle* ih, void **params)
{
  ImatExData* matex_data = (ImatExData*)malloc(sizeof(ImatExData));
  memset(matex_data, 0, sizeof(ImatExData));

  iupAttribSet(ih, "_IUP_MATEX_DATA", (char*)matex_data);
  matex_data->ih = ih;

  if (!iMatrixOriginalKeyPress_CB) iMatrixOriginalKeyPress_CB = (IFnii)IupGetCallback(ih, "KEYPRESS_CB");
  IupSetCallback(ih, "KEYPRESS_CB", (Icallback)iMatrixExKeyPress_CB);

  if (!iMatrixOriginalButton_CB) iMatrixOriginalButton_CB = (IFniiiis)IupGetCallback(ih, "KEYPRESS_CB");
  IupSetCallback(ih, "BUTTON_CB", (Icallback)iMatrixExButton_CB);

  (void)params;
  return IUP_NOERROR;
}

static void iMatrixExDestroyMethod(Ihandle* ih)
{
  ImatExData* matex_data = (ImatExData*)iupAttribGet(ih, "_IUP_MATEX_DATA");

  if (matex_data->busy_progress_dlg)
    IupDestroy(matex_data->busy_progress_dlg);

  if (matex_data->find_dlg)
    IupDestroy(matex_data->find_dlg);

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
  iupMatrixExRegisterFind(ic);
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

  iMatrixExCreateMethod(ih, NULL);
  IupSetCallback(ih, "DESTROY_CB", (Icallback)iMatrixExDestroyMethod);
    
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
