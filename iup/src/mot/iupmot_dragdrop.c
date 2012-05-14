/** \file
 * \brief Motif Base Functions
 *
 * See Copyright Notice in "iup.h"
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include <Xm/Xm.h>
#include <Xm/ScrollBar.h>
#include <Xm/DragDrop.h>
#include <X11/cursorfont.h>

#include "iup.h"
#include "iupkey.h"
#include "iupcbs.h"

#include "iup_object.h"
#include "iup_childtree.h"
#include "iup_key.h"
#include "iup_str.h"
#include "iup_class.h"
#include "iup_attrib.h"
#include "iup_focus.h"
#include "iup_key.h"
#include "iup_drv.h"
#include "iup_drvinfo.h"
#include "iup_image.h"

#include "iupmot_color.h"
#include "iupmot_drv.h"


static void motDoNothing(Widget w, XEvent*  evt, String* params, Cardinal* num_params)
{
  (void)w;
  (void)evt;
  (void)params;
  (void)num_params;
}

void iupmotDisableDragSource(Widget w)
{
  char dragTranslations[] = "#override <Btn2Down>: iupDoNothing()";
  static int do_nothing_rec = 0;
  if (!do_nothing_rec)
  {
    XtActionsRec rec = {"iupDoNothing", (XtActionProc)motDoNothing};
    XtAppAddActions(iupmot_appcontext, &rec, 1);
    do_nothing_rec = 1;
  }
  XtOverrideTranslations(w, XtParseTranslationTable(dragTranslations));
}

static void motDropTransferProc(Widget dropContext, XtPointer clientData, Atom *seltype, Atom *type,
                            XtPointer value, unsigned long *length, int format)
{
  Widget dropTarget = (Widget)clientData;
  Atom *dropTypesList = NULL;
  Cardinal numDropTypes = 0;
  Arg args[20];
  int i, num_args = 0;
  Ihandle *ih = NULL;

  if(!value)
    return;

  iupMOT_SETARG(args, num_args, XmNimportTargets, &dropTypesList);
  iupMOT_SETARG(args, num_args, XmNnumImportTargets, &numDropTypes);
  XmDropSiteRetrieve (dropTarget, args, num_args);

  XtVaGetValues(dropTarget, XmNuserData, &ih, NULL);

  for(i = 0; i < (int)numDropTypes; i++)
  {
    if (*type == dropTypesList[i])
    {
      IFnnsii cbDrop = (IFnnsii)IupGetCallback(ih, "DROPTARGET_CB");
      char* type = XGetAtomName(iupmot_display, dropTypesList[i]);
      int x = iupAttribGetInt(ih, "_IUPMOT_DROP_X");
      int y = iupAttribGetInt(ih, "_IUPMOT_DROP_Y");

      if(cbDrop)
        cbDrop(ih, (Ihandle*)value, type, x, y);

      /* Testing... */
      printf("DROPTARGET_CB ==> Ihandle*: ih, Ihandle*: target, Type: %s, X: %d, Y: %d\n", type, x, y);

      iupAttribSetInt(ih, "_IUPMOT_DROP_X", 0);
      iupAttribSetInt(ih, "_IUPMOT_DROP_Y", 0);
    }
  }

  (void)dropContext;
  (void)format;
  (void)type;
  (void)length;
  (void)seltype;
}

static void motDropProc(Widget dropTarget, XtPointer clientData, XmDropProcCallbackStruct* dropData)
{
  XmDropTransferEntryRec transferList[2];
  Arg args[20];
  int i, j, num_args;
  Widget dropContext;
  Cardinal numDragTypes, numDropTypes;
  Atom *dragTypesList, *dropTypesList;
  Atom atomItem;
  Boolean found = False;
  Ihandle *ih = NULL;

  dropContext = dropData->dragContext;

  /* Getting drop types */
  num_args = 0;
  iupMOT_SETARG(args, num_args, XmNimportTargets, &dropTypesList);
  iupMOT_SETARG(args, num_args, XmNnumImportTargets, &numDropTypes);
  XmDropSiteRetrieve (dropTarget, args, num_args);

  if(!numDropTypes)  /* no type registered */
    return;

  /* Getting drag types */
  num_args = 0;
  iupMOT_SETARG(args, num_args, XmNexportTargets, &dragTypesList);
  iupMOT_SETARG(args, num_args, XmNnumExportTargets, &numDragTypes);
  XtGetValues(dropContext, args, num_args);

  if(!numDragTypes)  /* no type registered */
    return;

  /* Checking the type compatibility */ 
  for (i = 0; i < (int)numDragTypes; i++) 
  {
    for (j = 0; j < (int)numDropTypes; j++) 
    {
      if (dragTypesList[i] == dropTypesList[j])  // TODO: improve this
      {
        atomItem = dropTypesList[j];
        found = True;
        break;
      }
    }

    if(found == True)
      break;
  }

  num_args = 0;
  if ((!found) || (dropData->dropAction != XmDROP) ||  (dropData->operation != XmDROP_COPY && dropData->operation != XmDROP_MOVE)) 
  {
    iupMOT_SETARG(args, num_args, XmNtransferStatus, XmTRANSFER_FAILURE);
    iupMOT_SETARG(args, num_args, XmNnumDropTransfers, 0);
  }
  else 
  {
    XtVaGetValues(dropTarget, XmNuserData, &ih, NULL);
    iupAttribSetInt(ih, "_IUPMOT_DROP_X", (int)dropData->x);
    iupAttribSetInt(ih, "_IUPMOT_DROP_Y", (int)dropData->y);

    /* set up transfer requests for drop site */
    transferList[0].target = atomItem;
    transferList[0].client_data = (XtPointer)dropTarget;
    iupMOT_SETARG(args, num_args, XmNdropTransfers, transferList);
    iupMOT_SETARG(args, num_args, XmNnumDropTransfers, 1);
    iupMOT_SETARG(args, num_args, XmNtransferProc, motDropTransferProc);
  }

  XmDropTransferStart(dropContext, args, num_args);
  
  (void)clientData;
}

static void motDragDropFinishCallback(Widget dropContext, XtPointer clientData, XtPointer callData)
{
  (void)dropContext;
  (void)clientData;
  (void)callData;
}

static Boolean motDragConvertProc(Widget drop_context, Atom *selection, Atom *target, Atom *typeReturn,
                          XtPointer *valueReturn, unsigned long *lengthReturn, int *formatReturn)
{
  Atom atomMotifDrop = XInternAtom(iupmot_display, "_MOTIF_DROP", False);
  Atom *dragTypesList;
  Cardinal numDragTypes;
  Arg args[20];
  int i, num_args = 0;
  Ihandle *ih = NULL;
  Widget w = NULL;

  iupMOT_SETARG(args, num_args, XmNclientData, &w);
  iupMOT_SETARG(args, num_args, XmNexportTargets, &dragTypesList);
  iupMOT_SETARG(args, num_args, XmNnumExportTargets, &numDragTypes);
  XtGetValues(drop_context, args, num_args);

  num_args = 0;
  iupMOT_SETARG(args, num_args, XmNuserData, &ih);
  XtGetValues(w, args, num_args);

  /* check if we are dealing with a drop */
  if (*selection != atomMotifDrop)
    return False;

  for(i = 0; i < (int)numDragTypes; i++)
  {
    if (*target == dragTypesList[i])
    {
      IFnnsi cbDrag = (IFnnsi)IupGetCallback(ih, "DRAGSOURCE_CB");
      char* source = iupAttribGet(ih, "IUP_DRAG_DATA");
      char* type = XGetAtomName(iupmot_display, dragTypesList[i]);
      char key[5];
      int is_ctrl;

      if(!source)
        return False;

      iupdrvGetKeyState(key);
      if (key[1] == 'C')
        is_ctrl = 1;  /* COPY */
      else
        is_ctrl = 0;  /* MOVE */

      /* format the value for transfer */
      *typeReturn = dragTypesList[i];
      *valueReturn = (XtPointer)source;
      *lengthReturn = 1;
      *formatReturn = 8;

      if (cbDrag)
        cbDrag(ih, (Ihandle*)source, type, is_ctrl);

      /* Testing... */
      printf("DRAGSOURCE_CB ==> Ihandle* ih, Ihandle* source, Type: %s, 0=Move/1=Copy: %d\n", type, is_ctrl);

      return True;
    }
  }

  return False;
}

static void motDragStart(Widget dragSource, XButtonEvent* evt, String* params, Cardinal* num_params)
{
  Widget drop_context;
  Arg args[20];
  int num_args = 0;
  Pixel fg, bg;
  Atom *exportList;
  Cardinal numExportList;
  Ihandle* ih = NULL;

  XtVaGetValues(dragSource, XmNbackground, &bg, XmNforeground, &fg, XmNuserData, &ih, NULL);

  exportList = (Atom*)iupAttribGet(ih, "_IUPMOT_DRAG_TARGETLIST");
  numExportList =  (Cardinal)iupAttribGetInt(ih, "_IUPMOT_DRAG_TARGETLIST_COUNT");
  if (!exportList)
    return;

  /* specify resources for DragContext for the transfer */
  num_args = 0;
  iupMOT_SETARG(args, num_args, XmNcursorBackground, bg);  // TODO: are these colors necessary?
  iupMOT_SETARG(args, num_args, XmNcursorForeground, fg);
  iupMOT_SETARG(args, num_args, XmNexportTargets, exportList);
  iupMOT_SETARG(args, num_args, XmNnumExportTargets, numExportList);
  iupMOT_SETARG(args, num_args, XmNdragOperations, XmDROP_MOVE|XmDROP_COPY);
  iupMOT_SETARG(args, num_args, XmNconvertProc, motDragConvertProc);
  iupMOT_SETARG(args, num_args, XmNclientData, dragSource);

  /* start the drag and register a callback to clean up when done */
  drop_context = XmDragStart(dragSource, (XEvent*)evt, args, num_args);
  XtAddCallback(drop_context, XmNdragDropFinishCallback, (XtCallbackProc)motDragDropFinishCallback, NULL);

  (void)params;
  (void)num_params;
}

static Atom* motCreateTargetList(const char *value, int *count)
{
  int count_alloc = 10;
  Atom *targetlist = (Atom*)XtMalloc(sizeof(Atom) * count_alloc);
  char valueCopy[256];
  char valueTemp[256];

  *count = 0;

  sprintf(valueCopy, "%s", value);
  while(iupStrToStrStr(valueCopy, valueTemp, valueCopy, ',') > 0)
  {
    targetlist[*count] = XInternAtom(iupmot_display, (char*)valueTemp, False);
    (*count)++;

    if(iupStrEqualNoCase(valueCopy, valueTemp))
      break;

    if (*count == count_alloc)
    {
      count_alloc += 10;
      targetlist = (Atom*)XtRealloc((char*)targetlist, sizeof(Atom) * count_alloc);
    }
  }

  if (*count == 0)
  {
    XtFree((char*)targetlist);
    return NULL;
  }

  return targetlist;
}

static int motSetDropTypesAttrib(Ihandle* ih, const char* value)
{
  int count = 0;
  Atom *targetlist = (Atom*)iupAttribGet(ih, "_IUPMOT_DROP_TARGETLIST");
  if (targetlist)
  {
    XtFree((char*)targetlist);
    iupAttribSetStr(ih, "_IUPMOT_DROP_TARGETLIST", NULL);
    iupAttribSetStr(ih, "_IUPMOT_DROP_TARGETLIST_COUNT", NULL);
  }

  if(!value)
    return 0;

  targetlist = motCreateTargetList(value, &count);
  if (targetlist)
  {
    iupAttribSetStr(ih, "_IUPMOT_DROP_TARGETLIST", (char*)targetlist);
    iupAttribSetInt(ih, "_IUPMOT_DROP_TARGETLIST_COUNT", count);
  }

  return 1;
}

static int motSetDragTypesAttrib(Ihandle* ih, const char* value)
{
  int count = 0;
  Atom *targetlist = (Atom*)iupAttribGet(ih, "_IUPMOT_DRAG_TARGETLIST");
  if (targetlist)
  {
    XtFree((char*)targetlist);
    iupAttribSetStr(ih, "_IUPMOT_DRAG_TARGETLIST", NULL);
    iupAttribSetStr(ih, "_IUPMOT_DRAG_TARGETLIST_COUNT", NULL);
  }

  if(!value)
    return 0;

  targetlist = motCreateTargetList(value, &count);
  if (targetlist)
  {
    iupAttribSetStr(ih, "_IUPMOT_DRAG_TARGETLIST", (char*)targetlist);
    iupAttribSetInt(ih, "_IUPMOT_DRAG_TARGETLIST_COUNT", count);
  }

  return 1;
}

static int motSetDropTargetAttrib(Ihandle* ih, const char* value)
{
  /* Are there defined drop types? */
  if(!iupAttribGet(ih, "_IUPMOT_DROP_TARGETLIST"))
    return 0;

  if(iupStrBoolean(value))
  {
    Atom *importList = (Atom*)iupAttribGet(ih, "_IUPMOT_DROP_TARGETLIST");
    Cardinal numimportList = (Cardinal)iupAttribGetInt(ih, "_IUPMOT_DROP_TARGETLIST_COUNT");
    Arg args[20];
    int num_args = 0;

    iupMOT_SETARG(args, num_args, XmNimportTargets, importList);
    iupMOT_SETARG(args, num_args, XmNnumImportTargets, numimportList);
    iupMOT_SETARG(args, num_args, XmNdropSiteOperations, XmDROP_MOVE|XmDROP_COPY);
    iupMOT_SETARG(args, num_args, XmNdropProc, motDropProc);
    XmDropSiteUpdate(ih->handle, args, num_args);

    XtVaSetValues(ih->handle, XmNuserData, ih, NULL);
  }
  else
    XmDropSiteUnregister(ih->handle);

  return 1;
}

static int motSetDragSourceAttrib(Ihandle* ih, const char* value)
{
  /* Are there defined drag types? */
  if(!iupAttribGet(ih, "_IUPMOT_DRAG_TARGETLIST"))
    return 0;

  if(iupStrBoolean(value))
  {
    char dragTranslations[] = "#override <Btn2Down>: iupStartDrag()";
    static int do_rec = 0;

    if (!do_rec)
    {
      XtActionsRec rec = {"iupStartDrag", (XtActionProc)motDragStart};
      XtAppAddActions(iupmot_appcontext, &rec, 1);
      do_rec = 1;
    }
    XtOverrideTranslations(ih->handle, XtParseTranslationTable(dragTranslations));

    XtVaSetValues(ih->handle, XmNuserData, ih, NULL);
  }
  else
    iupmotDisableDragSource(ih->handle);

  return 1;
}

void iupdrvRegisterDragDropAttrib(Iclass* ic)
{
  /* Not Supported */
  /* iupClassRegisterCallback(ic, "DROPFILES_CB", "siii"); */

  iupClassRegisterCallback(ic, "DRAGSOURCE_CB", "hsi");
  iupClassRegisterCallback(ic, "DROPTARGET_CB", "hsii");

  iupClassRegisterAttribute(ic, "DRAGTYPES",  NULL, motSetDragTypesAttrib, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "DROPTYPES",  NULL, motSetDropTypesAttrib, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "DRAGSOURCE", NULL, motSetDragSourceAttrib, NULL, NULL, IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "DROPTARGET", NULL, motSetDropTargetAttrib, NULL, NULL, IUPAF_NO_INHERIT);

  /* Not Supported */
  iupClassRegisterAttribute(ic, "DRAGDROP", NULL, NULL, NULL, NULL, IUPAF_NOT_SUPPORTED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "DROPFILESTARGET", NULL, NULL, NULL, NULL, IUPAF_NOT_SUPPORTED|IUPAF_NO_INHERIT);
}

/* TODO: 
Estudar se devemos usar
extern void XmDragCancel(Widget dragContext) ;
extern Boolean XmTargetsAreCompatible( 
*/
