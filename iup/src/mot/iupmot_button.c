/** \file
 * \brief Button Control
 *
 * See Copyright Notice in "iup.h"
 */

#include <Xm/Xm.h>
#include <Xm/PushB.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <stdarg.h>

#include "iup.h"
#include "iupcbs.h"

#include "iup_object.h"
#include "iup_childtree.h"
#include "iup_attrib.h"
#include "iup_dialog.h"
#include "iup_str.h"
#include "iup_button.h"
#include "iup_drv.h"
#include "iup_image.h"
#include "iup_key.h"

#include "iupmot_drv.h"


void iupdrvButtonAddBorders(int *x, int *y)
{
  int border_size = 2*5;
  (*x) += border_size;
  (*y) += border_size;
}

static int motButtonSetTitleAttrib(Ihandle* ih, const char* value)
{
  if (ih->data->type == IUP_BUTTON_TEXT)
  {
    iupmotSetMnemonicTitle(ih, value);
    return 1;
  }

  return 0;
}

static int motButtonSetAlignmentAttrib(Ihandle* ih, const char* value)
{
  unsigned char align;
  char value1[30]="", value2[30]="";

  iupStrToStrStr(value, value1, value2, ':');   /* value2 is ignored, NOT supported in Motif */

  if (iupStrEqualNoCase(value1, "ARIGHT"))
    align = XmALIGNMENT_END;
  else if (iupStrEqualNoCase(value1, "ACENTER"))
    align = XmALIGNMENT_CENTER;
  else /* "ALEFT" */
    align = XmALIGNMENT_BEGINNING;

  XtVaSetValues (ih->handle, XmNalignment, align, NULL);
  return 1;
}

static int motButtonSetImageAttrib(Ihandle* ih, const char* value)
{
  if (ih->data->type == IUP_BUTTON_IMAGE)
  {
    iupmotSetPixmap(ih, value, XmNlabelPixmap, 0, "IMAGE");

    if (!iupAttribGetStr(ih, "IMINACTIVE"))
    {
      /* if not active and IMINACTIVE is not defined 
         then automaticaly create one based on IMAGE */
      iupmotSetPixmap(ih, value, XmNlabelInsensitivePixmap, 1, "IMINACTIVE"); /* make_inactive */
    }
    return 1;
  }
  else
    return 0;
}

static int motButtonSetImInactiveAttrib(Ihandle* ih, const char* value)
{
  if (ih->data->type == IUP_BUTTON_IMAGE)
  {
    iupmotSetPixmap(ih, value, XmNlabelInsensitivePixmap, 0, "IMINACTIVE");
    return 1;
  }
  else
    return 0;
}

static int motButtonSetImPressAttrib(Ihandle* ih, const char* value)
{
  if (ih->data->type == IUP_BUTTON_IMAGE)
  {
    iupmotSetPixmap(ih, value, XmNarmPixmap, 0, "IMPRESS");
    return 1;
  }
  else
    return 0;
}

static int motButtonSetPaddingAttrib(Ihandle* ih, const char* value)
{
  iupStrToIntInt(value, &ih->data->horiz_padding, &ih->data->vert_padding, 'x');
  if (ih->handle)
  {
    XtVaSetValues(ih->handle, XmNmarginHeight, ih->data->vert_padding,
                              XmNmarginWidth, ih->data->horiz_padding, NULL);
  }
  return 0;
}

static int motButtonSetBgColorAttrib(Ihandle* ih, const char* value)
{
  if (iupAttribGetStr(ih, "IMPRESS"))
  {
    /* ignore given value, must use only from parent */
    value = iupAttribGetStrNativeParent(ih, "BGCOLOR");
    if (!value) value = IupGetGlobal("DLGBGCOLOR");
    if (iupdrvBaseSetBgColorAttrib(ih, value))
      return 1;
  }

  return iupdrvBaseSetBgColorAttrib(ih, value);
}

static int motButtonSetBackgroundAttrib(Ihandle* ih, const char* value)
{
  if (iupAttribGetStr(ih, "IMPRESS"))
  {
    /* ignore given value, must use only from parent */
    value = iupAttribGetStrNativeParent(ih, "BACKGROUND");

    if (iupdrvBaseSetBgColorAttrib(ih, value))
      return 1;
    else
    {
      Pixmap pixmap = (Pixmap)iupImageGetImage(value, ih, 0, "BACKGROUND");
      if (pixmap)
      {
        XtVaSetValues(ih->handle, XmNbackgroundPixmap, pixmap, NULL);
        return 1;
      }
    }
  }
  return 0; 
}

static void motButtonActivateCallback(Widget w, Ihandle* ih, XtPointer call_data)
{
  Icallback cb;

  /* Must manually hide the tip if the button is pressed. */
  iupmotTipLeaveNotify();

  cb = IupGetCallback(ih, "ACTION");
  if (cb)
  {
    if (cb(ih) == IUP_CLOSE) 
      IupExitLoop();
  }
  (void)w;
  (void)call_data;
}

static void motButtonEnterLeaveWindowEvent(Widget w, Ihandle* ih, XEvent *evt, Boolean *cont)
{
  iupmotEnterLeaveWindowEvent(w, ih, evt, cont);

  if (evt->type == EnterNotify)
    XtVaSetValues(ih->handle, XmNshadowThickness, 2, NULL);
  else  if (evt->type == LeaveNotify)
    XtVaSetValues(ih->handle, XmNshadowThickness, 0, NULL);
}

static int motButtonMapMethod(Ihandle* ih)
{
  char* value;
  int num_args = 0;
  Arg args[30];

  value = iupAttribGetStr(ih, "IMAGE");
  if (value)
  {
    ih->data->type = IUP_BUTTON_IMAGE;
    iupmotSetArg(args[num_args++], XmNlabelType, XmPIXMAP) 
  }
  else
  {
    ih->data->type = IUP_BUTTON_TEXT;
    iupmotSetArg(args[num_args++], XmNlabelType, XmSTRING) 
  }

  /* Core */
  iupmotSetArg(args[num_args++], XmNmappedWhenManaged, False);  /* not visible when managed */
  iupmotSetArg(args[num_args++], XmNx, 0);  /* x-position */
  iupmotSetArg(args[num_args++], XmNy, 0);  /* y-position */
  iupmotSetArg(args[num_args++], XmNwidth, 10);  /* default width to avoid 0 */
  iupmotSetArg(args[num_args++], XmNheight, 10); /* default height to avoid 0 */
  /* Label */
  iupmotSetArg(args[num_args++], XmNrecomputeSize, False);  /* no automatic resize from text */
  iupmotSetArg(args[num_args++], XmNmarginHeight, 0);  /* default padding */
  iupmotSetArg(args[num_args++], XmNmarginWidth, 0);
  iupmotSetArg(args[num_args++], XmNmarginTop, 0);     /* no extra margins */
  iupmotSetArg(args[num_args++], XmNmarginLeft, 0);
  iupmotSetArg(args[num_args++], XmNmarginBottom, 0);
  iupmotSetArg(args[num_args++], XmNmarginRight, 0);
  /* PushButton */
  iupmotSetArg(args[num_args++], XmNfillOnArm, False);

  /* Primitive */
  if (iupAttribGetIntDefault(ih, "FOCUSONCLICK"))
  {
    if (iupStrBoolean(iupAttribGetStrDefault(ih, "CANFOCUS")))
      iupmotSetArg(args[num_args++], XmNtraversalOn, True)
    else
      iupmotSetArg(args[num_args++], XmNtraversalOn, False)
  }
  else
    iupmotSetArg(args[num_args++], XmNtraversalOn, False)
  iupmotSetArg(args[num_args++], XmNhighlightThickness, 2);
  iupmotSetArg(args[num_args++], XmNnavigationType, XmTAB_GROUP);
  
  ih->handle = XtCreateManagedWidget(
    iupDialogGetChildIdStr(ih),  /* child identifier */
    xmPushButtonWidgetClass,     /* widget class */
    iupChildTreeGetNativeParentHandle(ih), /* widget parent */
    args, num_args);

  if (!ih->handle)
    return IUP_ERROR;

  ih->serial = iupDialogGetChildId(ih); /* must be after using the string */

  XtAddCallback(ih->handle, XmNhelpCallback, (XtCallbackProc)iupmotHelpCallback, (XtPointer)ih);

  value = iupAttribGetStr(ih, "IMPRESS");

  if (iupStrBoolean(iupAttribGetStrInherit(ih, "FLAT")) && !value)
  {
    XtVaSetValues(ih->handle, XmNshadowThickness, 0, NULL);
    XtAddEventHandler(ih->handle, EnterWindowMask, False, (XtEventHandler)motButtonEnterLeaveWindowEvent, (XtPointer)ih);
    XtAddEventHandler(ih->handle, LeaveWindowMask, False, (XtEventHandler)motButtonEnterLeaveWindowEvent, (XtPointer)ih);
  }
  else
  {
    if (value && !iupAttribGetStr(ih, "IMPRESSBORDER"))
      XtVaSetValues(ih->handle, XmNshadowThickness, 0, NULL);
    else
      XtVaSetValues(ih->handle, XmNshadowThickness, 2, NULL);
    XtAddEventHandler(ih->handle, EnterWindowMask, False, (XtEventHandler)iupmotEnterLeaveWindowEvent, (XtPointer)ih);
    XtAddEventHandler(ih->handle, LeaveWindowMask, False, (XtEventHandler)iupmotEnterLeaveWindowEvent, (XtPointer)ih);
  }

  XtAddEventHandler(ih->handle, FocusChangeMask, False, (XtEventHandler)iupmotFocusChangeEvent, (XtPointer)ih);
  XtAddEventHandler(ih->handle, KeyPressMask,    False, (XtEventHandler)iupmotKeyPressEvent,    (XtPointer)ih);

  XtAddCallback(ih->handle, XmNactivateCallback, (XtCallbackProc)motButtonActivateCallback, (XtPointer)ih);
  XtAddEventHandler(ih->handle, ButtonPressMask | ButtonReleaseMask, False, (XtEventHandler)iupmotButtonPressReleaseEvent, (XtPointer)ih);

  /* Disable Drag Source */
  iupmotDisableDragSource(ih->handle);

  /* initialize the widget */
  XtRealizeWidget(ih->handle);

  /* ensure the default values, that are different from the native ones */
  motButtonSetAlignmentAttrib(ih, iupAttribGetStrDefault(ih, "ALIGNMENT"));

  if (ih->data->type == IUP_BUTTON_TEXT)
    iupmotSetString(ih->handle, XmNlabelString, "");

  return IUP_NOERROR;
}

void iupdrvButtonInitClass(Iclass* ic)
{
  /* Driver Dependent Class functions */
  ic->Map = motButtonMapMethod;

  /* Driver Dependent Attribute functions */

  /* Visual */
  iupClassRegisterAttribute(ic, "BGCOLOR", iupmotGetBgColorAttrib, motButtonSetBgColorAttrib, "DLGBGCOLOR", IUP_MAPPED, IUP_INHERIT);
  iupClassRegisterAttribute(ic, "BACKGROUND", NULL, motButtonSetBackgroundAttrib, "DLGBGCOLOR", IUP_MAPPED, IUP_INHERIT);

  /* Special */
  iupClassRegisterAttribute(ic, "FGCOLOR", NULL, iupdrvBaseSetFgColorAttrib, "0 0 0", IUP_MAPPED, IUP_INHERIT);
  iupClassRegisterAttribute(ic, "TITLE", NULL, motButtonSetTitleAttrib, NULL, IUP_MAPPED, IUP_NO_INHERIT);

  /* IupButton only */
  iupClassRegisterAttribute(ic, "ALIGNMENT", NULL, motButtonSetAlignmentAttrib, "ACENTER:ACENTER", IUP_MAPPED, IUP_NO_INHERIT);
  iupClassRegisterAttribute(ic, "IMAGE", NULL, motButtonSetImageAttrib, NULL, IUP_MAPPED, IUP_NO_INHERIT);
  iupClassRegisterAttribute(ic, "IMINACTIVE", NULL, motButtonSetImInactiveAttrib, NULL, IUP_MAPPED, IUP_NO_INHERIT);
  iupClassRegisterAttribute(ic, "IMPRESS", NULL, motButtonSetImPressAttrib, NULL, IUP_MAPPED, IUP_NO_INHERIT);
  iupClassRegisterAttribute(ic, "FOCUSONCLICK", NULL, NULL, "YES", IUP_MAPPED, IUP_NO_INHERIT);

  iupClassRegisterAttribute(ic, "PADDING", iupButtonGetPaddingAttrib, motButtonSetPaddingAttrib, "0x0", IUP_NOT_MAPPED, IUP_INHERIT);
}
