/** \file
 * \brief Motif Driver motMessageDlg
 *
 * See Copyright Notice in iup.h
 * $Id: motmsgdlg.c,v 1.1 2008-10-17 06:19:25 scuri Exp $
 */

#if defined(__STDC__) && defined(ULTRIX)
#define X$GBLS(x)  #x
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#ifdef _VMS_
#include <unixlib.h>
#endif
#include <Xm/Text.h>
#include <Xm/DialogS.h>
#include <Xm/Xm.h>
#include <Xm/MessageB.h>

#include "iglobal.h"
#include "idrv.h"
#include "motif.h"

static void motMessageDlgOkCb(Widget w,
                             void* client_data,
                             XmAnyCallbackStruct* call_data)
{
  Ihandle* h = (Ihandle*)client_data;
  IupHide(h);
}

static int motMessageDlgMap(Ihandle *h)
{
  Widget parent;
  Widget MsgBox;
  Pixel fg, bg;
  XmString caption, message;
  Arg args[10];
  int ac = 0;

  if (child(h) != NULL && handle(child(h)) != NULL)
    return IUP_NOERROR;

  iupCpiDefaultMap( h, 0 );

  parent = XtNameToWidget( (Widget)handle(h), "*dialog_area");
  XtVaSetValues( parent, XmNresizePolicy, XmRESIZE_ANY, NULL );
  XtVaGetValues( parent, XmNforeground, &fg, XmNbackground, &bg, NULL );

  caption = XmStringCreateLocalized(IupGetAttribute(h, IUP_TITLE));
  message = XmStringCreateLocalized(IupGetAttribute(h, IUP_VALUE));
  XtSetArg(args[ac], XmNmessageString, message); ac++;
  XtSetArg(args[ac], XmNdialogTitle, caption); ac++;
  XtSetArg(args[ac], XmNforeground, fg ); ac++;
  XtSetArg(args[ac], XmNbackground, bg ); ac++;
  XtSetArg(args[ac], XmNresizePolicy, XmRESIZE_NONE ); ac++;

  MsgBox = XtCreateWidget("msgbox",
    xmMessageBoxWidgetClass, parent,
    args, ac );

  if (!MsgBox)
    return IUP_ERROR;

  XmStringFree( caption );
  XmStringFree( message );
  XtUnmanageChild(XmMessageBoxGetChild(MsgBox, XmDIALOG_CANCEL_BUTTON));
  XtUnmanageChild(XmMessageBoxGetChild(MsgBox, XmDIALOG_HELP_BUTTON));

  XtAddCallback(MsgBox, XmNokCallback,
    (XtCallbackProc)motMessageDlgOkCb, (XtPointer)h);

  XtManageChild(MsgBox);
  handle(child(h)) = MsgBox;

  return IUP_NOERROR;
}

static Ihandle* motMessageDlgCreate(Iclass *ic)
{
  Ihandle* user = IupUser();
  Ihandle* ih   = IupDialog( user );

  IupSetAttribute( user, IUP_EXPAND, IUP_YES );
  IupSetAttribute( ih,   IUP_MINBOX, IUP_NO );
  IupSetAttribute( ih,   IUP_MAXBOX, IUP_NO );
  IupSetAttribute( ih,   IUP_RESIZE, IUP_NO );

  return ih;
}

static int motMessageDlgGetSize(Ihandle* n, int *w, int *h)
{  
  return 0;
}

/*
* Funcoes exportadas
*/

static Ihandle *motMessageDlg(void)
{
  return IupCreate("messagedlg");
}

void iupmotMessageDlgOpen(void)
{
  Iclass *ICFileDlg = iupCpiCreateNewClass("messagedlg", NULL);

  iupCpiSetClassMethod(ICFileDlg, ICPI_MAP,     (Imethod) motMessageDlgMap);
  iupCpiSetClassMethod(ICFileDlg, ICPI_CREATE,  (Imethod) motMessageDlgCreate);
  iupCpiSetClassMethod(ICFileDlg, ICPI_GETSIZE, (Imethod) motMessageDlgGetSize);
}

void IupMessage(const char* title, const char* msg)
{
  Ihandle* msgdlg = motMessageDlg();
  IupSetAttribute(msgdlg, IUP_TITLE, title);
  IupSetAttribute(msgdlg, IUP_VALUE, msg);
  IupSetAttribute(msgdlg, IUP_PARENTDIALOG, IupGetGlobal(IUP_PARENTDIALOG));
  IupSetAttribute(msgdlg, IUP_ICON, IupGetGlobal(IUP_ICON));
  IupPopup( msgdlg, IUP_CENTER, IUP_CENTER );
  IupDestroy(msgdlg);
}
