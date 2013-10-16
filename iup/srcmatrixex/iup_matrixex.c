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
#include "iup_predialogs.h"
#include "iup_matrixex.h"


static void iMatrixListShowLastError(Ihandle* ih)
{
  char* lasterror = iupAttribGet(ih, "LASTERROR");
  if (lasterror)
    iupShowError(IupGetDialog(ih), lasterror);
}

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
  int flin, fcol;

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

  /* clear the previous freeze first */
  flin = IupGetInt(ih,"NUMLIN_NOSCROLL");
  fcol = IupGetInt(ih,"NUMCOL_NOSCROLL");
  if (flin) IupSetAttributeId2(ih,"FRAMEHORIZCOLOR", flin, IUP_INVALID_ID, NULL);
  if (fcol) IupSetAttributeId2(ih,"FRAMEVERTCOLOR", IUP_INVALID_ID, fcol, NULL);

  if (!freeze)
  {
    IupSetAttribute(ih,"NUMLIN_NOSCROLL","0");
    IupSetAttribute(ih,"NUMCOL_NOSCROLL","0");
    IupSetAttribute(ih,"SHOW","1:1");
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

static char* iMatrixExFileDlg(ImatExData* matex_data, int save, const char* filter, const char* info, const char* extfilter)
{
  Ihandle* dlg = IupFileDlg();

  IupSetAttribute(dlg,"DIALOGTYPE", save? "SAVE": "OPEN");
  IupSetAttribute(dlg,"TITLE","Export Table");
  IupSetAttribute(dlg,"FILTER", filter);
  IupSetAttribute(dlg,"FILTERINFO", info);
  IupSetAttribute(dlg,"EXTFILTER", extfilter);  /* Windows and GTK only, but more flexible */
  IupSetAttributeHandle(dlg,"PARENTDIALOG", IupGetDialog(matex_data->ih));

  IupPopup(dlg,IUP_CENTER,IUP_CENTER);
  if (IupGetInt(dlg,"STATUS")!=-1)
  {
    char* value = IupGetAttribute(dlg, "VALUE");
    value = iupStrReturnStr(value);
    IupDestroy(dlg);
    return value;
  }

  IupDestroy(dlg);
  return NULL;
}

static int iMatrixExItemExport_CB(Ihandle* ih_item)
{
  ImatExData* matex_data = (ImatExData*)IupGetAttribute(ih_item, "MATRIX_EX_DATA");
  char *filter, *info, *extfilter, *filename;

  if (iupStrEqual(IupGetAttribute(ih_item, "TEXTFORMAT"), "LaTeX"))
  {
    filter = "*.tex";
    info = "LaTeX file (table format)";
    extfilter = "LaTeX file (table format)|*.tex|All Files|*.*|";
  }
  else if (iupStrEqual(IupGetAttribute(ih_item, "TEXTFORMAT"), "HTML"))
  {
    filter = "*.html;*.htm";
    info = "HTML file (table format)";
    extfilter = "HTML file (table format)|*.html;*.htm|All Files|*.*|";
  }
  else
  {
    filter = "*.txt";
    info = "Text file";
    extfilter = "Text file|*.txt|All Files|*.*|";
  }

  filename = iMatrixExFileDlg(matex_data, 1, filter, info, extfilter);

  IupSetAttribute(matex_data->ih, "TEXTFORMAT", IupGetAttribute(ih_item, "TEXTFORMAT"));
  IupSetAttribute(matex_data->ih, "COPYFILE", filename);

  iMatrixListShowLastError(matex_data->ih);

  return IUP_DEFAULT;
}

static int iMatrixExItemImport_CB(Ihandle* ih_item)
{
  ImatExData* matex_data = (ImatExData*)IupGetAttribute(ih_item, "MATRIX_EX_DATA");
  char *filter, *info, *extfilter, *filename;

  filter = "*.txt";
  info = "Text file";
  extfilter = "Text file|*.txt|All Files|*.*|";

  filename = iMatrixExFileDlg(matex_data, 0, filter, info, extfilter);

  IupSetAttribute(matex_data->ih, "PASTEFILE", filename);

  iMatrixListShowLastError(matex_data->ih);

  return IUP_DEFAULT;
}

static int iMatrixExItemCopy_CB(Ihandle* ih_item)
{
  ImatExData* matex_data = (ImatExData*)IupGetAttribute(ih_item, "MATRIX_EX_DATA");
  IupSetAttribute(matex_data->ih, "COPY", "MARKED");
  iMatrixListShowLastError(matex_data->ih);
  return IUP_DEFAULT;
}

static int iMatrixExItemPaste_CB(Ihandle* ih_item)
{
  ImatExData* matex_data = (ImatExData*)IupGetAttribute(ih_item, "MATRIX_EX_DATA");
  if (IupGetAttribute(matex_data->ih, "MARKED"))
    IupSetAttribute(matex_data->ih, "PASTE", "MARKED");
  else
    IupSetAttribute(matex_data->ih, "PASTE", "FOCUS");
  iMatrixListShowLastError(matex_data->ih);
  return IUP_DEFAULT;
}

static int iMatrixExItemCopyColTo_CB(Ihandle* ih_item)
{
  ImatExData* matex_data = (ImatExData*)IupGetAttribute(ih_item, "MATRIX_EX_DATA");
  char* value = IupGetAttribute(ih_item, "COPYCOLTO");
  int lin, col;

  IupGetIntInt(ih_item, "MENUCONTEXT_CELL", &lin, &col);

  if (iupStrEqual(value, "INTERVAL"))
  {
    char interval[200] = "";
    value = iupAttribGet(matex_data->ih, "_IUP_LAST_COPYCOL_INTERVAL");
    if (value) iupStrCopyN(interval, 200, value);

    if (IupGetParam("Copy To Interval", NULL, NULL, "Intervals: %s\n", interval, NULL))
    {
      IupSetAttributeId2(matex_data->ih, "COPYCOLTO", lin, col, interval);
      iupAttribSetStr(matex_data->ih, "_IUP_LAST_COPYCOL_INTERVAL", interval);
    }
  }
  else
    IupSetAttributeId2(matex_data->ih, "COPYCOLTO", lin, col, value);

  return IUP_DEFAULT;
}

static int iMatrixExItemUndo_CB(Ihandle* ih_item)
{
  ImatExData* matex_data = (ImatExData*)IupGetAttribute(ih_item, "MATRIX_EX_DATA");
  IupSetAttribute(matex_data->ih, "UNDO", NULL);  /* 1 level */
  iMatrixListShowLastError(matex_data->ih);
  return IUP_DEFAULT;
}

static int iMatrixExItemRedo_CB(Ihandle* ih_item)
{
  ImatExData* matex_data = (ImatExData*)IupGetAttribute(ih_item, "MATRIX_EX_DATA");
  IupSetAttribute(matex_data->ih, "REDO", NULL);  /* 1 level */
  iMatrixListShowLastError(matex_data->ih);
  return IUP_DEFAULT;
}

static int iMatrixExItemFind_CB(Ihandle* ih_item)
{
  ImatExData* matex_data = (ImatExData*)IupGetAttribute(ih_item, "MATRIX_EX_DATA");
  iupMatrixExFindInitDialog(matex_data);
  iupMatrixExFindShowDialog(matex_data);
  return IUP_DEFAULT;
}

//  iupClassRegisterAttributeId(ic, "SORTCOLUMN", NULL, iMatrixSetSortColumnAttrib, IUPAF_WRITEONLY|IUPAF_NO_INHERIT);
//  iupClassRegisterAttribute(ic, "SORTCOLUMNORDER", NULL, NULL, IUPAF_SAMEASSYSTEM, "ASCENDING", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
//  iupClassRegisterAttribute(ic, "SORTCOLUMNCASESENSITIVE", NULL, NULL, IUPAF_SAMEASSYSTEM, "YES", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);

static int iMatrixExItemFreeze_CB(Ihandle* ih_item)
{
  ImatExData* matex_data = (ImatExData*)IupGetAttribute(ih_item, "MATRIX_EX_DATA");
  int lin, col, flin, fcol;

  IupGetIntInt(ih_item, "MENUCONTEXT_CELL", &lin, &col);

  IupGetIntInt(matex_data->ih, "FREEZE", &flin, &fcol);
  if (lin!=flin || col!=fcol)
    IupSetfAttribute(matex_data->ih, "FREEZE", "%d:%d", lin, col);
  else
    IupSetAttribute(matex_data->ih, "FREEZE", NULL);
  return IUP_DEFAULT;
}

static Ihandle* iMatrixExCreateMenuContext(Ihandle* ih, int lin, int col)
{
  int readonly = IupGetInt(ih, "READONLY");

  Ihandle* menu = IupMenu(
    IupSubmenu("Export",
      IupMenu(
        IupSetCallbacks(IupSetAttributes(IupItem("Text..." , NULL), "TEXTFORMAT=TXT"), "ACTION", iMatrixExItemExport_CB, NULL),
        IupSetCallbacks(IupSetAttributes(IupItem("LaTeX...", NULL), "TEXTFORMAT=LaTeX"), "ACTION", iMatrixExItemExport_CB, NULL),
        IupSetCallbacks(IupSetAttributes(IupItem("Html..." , NULL), "TEXTFORMAT=HTML"), "ACTION", iMatrixExItemExport_CB, NULL),
        NULL)),
    NULL);

  if (!readonly)
  {
    IupAppend(menu, IupSubmenu("Import",
        IupMenu(
          IupSetCallbacks(IupItem("Text...",  NULL), "ACTION", iMatrixExItemImport_CB, NULL),
          NULL)));
    IupAppend(menu, IupSubmenu("Copy To (Same Column)",
        IupMenu(
          IupSetCallbacks(IupSetAttributes(IupItem("All lines"      , NULL), "COPYCOLTO=ALL"), "ACTION", iMatrixExItemCopyColTo_CB, NULL),     
          IupSetCallbacks(IupSetAttributes(IupItem("Here to top"    , NULL), "COPYCOLTO=TOP"), "ACTION", iMatrixExItemCopyColTo_CB, NULL),     
          IupSetCallbacks(IupSetAttributes(IupItem("Here to bottom" , NULL), "COPYCOLTO=BOTTOM"), "ACTION", iMatrixExItemCopyColTo_CB, NULL),     
          IupSetCallbacks(IupSetAttributes(IupItem("Interval..."    , NULL), "COPYCOLTO=INTERVAL"), "ACTION", iMatrixExItemCopyColTo_CB, NULL),     
          IupSetCallbacks(IupSetAttributes(IupItem("Selected lines" , NULL), "COPYCOLTO=MARKED"), "ACTION", iMatrixExItemCopyColTo_CB, NULL),
          NULL)));
    IupAppend(menu, IupSeparator());
  }

  IupAppend(menu, IupSetCallbacks(IupItem("Copy\tCtrl+C",  NULL), "ACTION", iMatrixExItemCopy_CB, NULL));

  if (!readonly)
  {
    Ihandle *undo, *redo;
    IupAppend(menu, IupSetCallbacks(IupItem("Paste\tCtrl+V", NULL), "ACTION", iMatrixExItemPaste_CB, NULL));
    IupAppend(menu, IupSeparator());
    IupAppend(menu, undo = IupSetCallbacks(IupItem("Undo\tCtrl+Z", NULL), "ACTION", iMatrixExItemUndo_CB, NULL));
    IupAppend(menu, redo = IupSetCallbacks(IupItem("Redo\tCtrl+Y", NULL), "ACTION", iMatrixExItemRedo_CB, NULL));
    //IupAppend(menu, IupItem("Undo List...\tCtrl+U", NULL));
    IupAppend(menu, IupSeparator());
    //IupAppend(menu, IupItem("Sort..."             , NULL));

    if (!IupGetInt(ih, "UNDO"))
      IupSetAttribute(undo, "ACTIVE", "No");
    if (!IupGetInt(ih, "REDO"))
      IupSetAttribute(redo, "ACTIVE", "No");
  }

  IupAppend(menu, IupSetCallbacks(IupItem("Find...\tCtrl+F", NULL), "ACTION", iMatrixExItemFind_CB, NULL));
  IupAppend(menu, IupSeparator());

  {
    int flin, fcol;
    IupGetIntInt(ih, "FREEZE", &flin, &fcol);
    if (lin!=flin || col!=fcol)
      IupAppend(menu, IupSetCallbacks(IupItem("Freeze", NULL), "ACTION", iMatrixExItemFreeze_CB, NULL));
    else
      IupAppend(menu, IupSetCallbacks(IupItem("Unfreeze", NULL), "ACTION", iMatrixExItemFreeze_CB, NULL));
  }

  //IupAppend(menu, IupItem("Hide Column"         , NULL));
  //IupAppend(menu, IupItem("Show Hidden Columns" , NULL));

  //Is Numeric Column and Has Units
  //Unit...
  //Decimals...

  return menu;
}

static IFniiiis iMatrixOriginalButton_CB = NULL;

static int iMatrixExButton_CB(Ihandle* ih, int b, int press, int x, int y, char* r)
{
  if (iMatrixOriginalButton_CB(ih, b, press, x, y, r)==IUP_IGNORE)
    return IUP_IGNORE;

  if (b == IUP_BUTTON3 && press && IupGetInt(ih, "MENUCONTEXT"))
  {
    int pos = IupConvertXYToPos(ih, x, y);
    if (pos >= 0)
    {
      ImatExData* matex_data = (ImatExData*)iupAttribGet(ih, "_IUP_MATEX_DATA");
      int lin, col;
      IFnnii menucontext_cb;
      Ihandle* menu;
      int sx, sy;

      IupTextConvertPosToLinCol(ih, pos, &lin, &col);

      menu = iMatrixExCreateMenuContext(ih, lin, col);
      IupSetAttribute(menu, "MATRIX_EX_DATA", (char*)matex_data);  /* do not use "_IUP_MATEX_DATA" to enable inheritance */
      IupSetfAttribute(menu, "MENUCONTEXT_CELL", "%d:%d", lin, col);

      menucontext_cb = (IFnnii)IupGetCallback(ih, "MENUCONTEXT_CB");
      if (menucontext_cb) menucontext_cb(ih, menu, lin, col);

      IupGetIntInt(ih, "SCREENPOSITION", &sx, &sy);
      IupPopup(menu, sx+x, sy+y);
      IupDestroy(menu);
    }
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

        iMatrixListShowLastError(ih);
      }
      return IUP_IGNORE;
    case K_cC: 
      IupSetAttribute(ih, "COPY", "MARKED");
      iMatrixListShowLastError(ih);
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
//    case K_cU: { D->UndoList() ; return IUP_IGNORE  ;}
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

  if (!iMatrixOriginalButton_CB) iMatrixOriginalButton_CB = (IFniiiis)IupGetCallback(ih, "BUTTON_CB");
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

  iupClassRegisterCallback(ic, "MENUCONTEXT_CB", "nii");
  iupClassRegisterAttribute(ic, "MENUCONTEXT", NULL, NULL, IUPAF_SAMEASSYSTEM, "Yes", IUPAF_NO_INHERIT);

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
