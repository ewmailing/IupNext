/** \file
* \brief Motif Driver TIPS Control
*
* See Copyright Notice in iup.h
* $Id: mottips.c,v 1.1 2008-10-17 06:19:26 scuri Exp $
*/

#include <Xm/Xm.h>
#include <Xm/Label.h>

#include "iglobal.h"
#include "idrv.h"
#include "motif.h"
#include "mottips.h"

/*
** Definitions
*/
#define TIP_FGCOLOR   "0 0 0"   /* Black */
#define TIP_BGCOLOR   "255 255 0" /* Yellow */
#define TIP_SHOW_INTERVAL 1000    /* 1 second to Show     */
#define TIP_HIDE_INTERVAL 6000    /* 1+5 seconds to Hide */
#define TIP_X_OFFSET    0
#define TIP_Y_OFFSET    15

typedef struct _Imottips
{
  Widget        Dialog;
  Widget        Label;
  XtIntervalId  ShowTimerId;       /* Timer: inactive to active tip    */
  XtIntervalId  HideTimerId;       /* Timer: active to inactive tip    */
  unsigned long ShowInterval;      /* Time for tipPopupTimerId         */
  unsigned long HideInterval;      /* Time for tipActiveTimerId        */
  int           XOffset;           /* X Offset when mapping tip        */
  int           YOffset;           /* Y Offset when mapping tip        */
  int           Visible;
} Imottips;

static Imottips mottips = {0, 0, 0, 0, 0, 0, 0, 0};

static void motTipsShow ( XtPointer cd, XtIntervalId *i )
{
  Window root, child;
  int cx, cy, px, py;
  unsigned int keys;
  XQueryPointer(iupmot_display, RootWindow(iupmot_display, iupmot_screen),
                &root, &child, &px, &py, &cx, &cy, &keys);
  mottips.Visible = TRUE;
  mottips.ShowTimerId = (XtIntervalId) NULL;
  XtVaSetValues( mottips.Dialog, 
                     XmNx, px+mottips.XOffset,
                     XmNy, py+mottips.YOffset, 
                     NULL );
  
  if ( XtIsRealized( mottips.Dialog) )
    XtMapWidget( mottips.Dialog );
  else
    XtRealizeWidget( mottips.Dialog );
}

static void motTipsHide(XtPointer cd, XtIntervalId *i)
{
  mottips.HideTimerId = (XtIntervalId) NULL;
  iupmotTipLeaveNotify();
}

static int motTipsSet(Ihandle *h)
{
  XmString label;
  XmFontList font;
  char* tipText;

  tipText = IupGetAttribute(h,IUP_TIP);
  if (tipText == NULL)
    return FALSE;

  if (mottips.Dialog == NULL)
  {
    mottips.Dialog = XtAppCreateShell (" ", "tip",
      overrideShellWidgetClass, iupmot_display,
      0, 0);

    mottips.Label = XtVaCreateManagedWidget ("label tip",
      xmLabelWidgetClass, mottips.Dialog,
      XmNforeground, iupmotGetColor(0, TIP_FGCOLOR),
      XmNbackground, iupmotGetColor(0, TIP_BGCOLOR),
      NULL);
  }
  else
  {
    int lw = 0, lh = 0;
    iupdrvStringSize(h, tipText, &lw, &lh);

    /* add room for borders */
    lw += 2*(3+4);
    lh += 2*(3+1);  

    XtVaSetValues (mottips.Dialog,
      XmNwidth, (XtArgVal)lw,
      XmNheight, (XtArgVal)lh,
      NULL);
 }
 
  label = XmStringCreateLocalized( tipText );
  XtVaSetValues(mottips.Label, XmNlabelString, label, NULL);
  XmStringFree(label);

  font = iupmoGetFontList(IupGetAttribute(h, IUP_FONT));
  if (font)
    XtVaSetValues(mottips.Label, XmNfontList, font, NULL);

  mottips.ShowTimerId  = (XtIntervalId) NULL;
  mottips.HideTimerId  = (XtIntervalId) NULL;
  mottips.ShowInterval = TIP_SHOW_INTERVAL;
  mottips.HideInterval = TIP_HIDE_INTERVAL;
  mottips.XOffset      = TIP_X_OFFSET;
  mottips.YOffset      = TIP_Y_OFFSET;
  mottips.Visible      = FALSE;
  
  return TRUE;
}

void iupmotTipEnterNotify ( Widget w, Ihandle *h )
{
  iupmotTipLeaveNotify();

  if (motTipsSet(h) == FALSE)
    return;

  mottips.ShowTimerId = XtAppAddTimeOut( /* Inactive to Active */
    XtWidgetToApplicationContext(w),
    mottips.ShowInterval,
    motTipsShow,
    NULL);
  mottips.HideTimerId = XtAppAddTimeOut( /* Active to Inactive */
    XtWidgetToApplicationContext(w),
    mottips.HideInterval,
    motTipsHide,
    NULL);
}

void iupmotTipLeaveNotify(void)
{
  if (mottips.ShowTimerId)
  {
    XtRemoveTimeOut(mottips.ShowTimerId);
    mottips.ShowTimerId = (XtIntervalId) NULL;
  }
  if (mottips.HideTimerId)
  {
    XtRemoveTimeOut(mottips.HideTimerId);
    mottips.HideTimerId = (XtIntervalId) NULL;
  }
  if (mottips.Visible)
  {
    XtUnmapWidget(mottips.Dialog);
    mottips.Visible = FALSE;
  }
}
