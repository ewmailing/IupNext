/** \file
 * \brief Calendar Control
 *
 * See Copyright Notice in "iup.h"
 */

#include <windows.h>
#include <commctrl.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <memory.h>
#include <stdarg.h>
#include <limits.h>

#include "iup.h"
#include "iupcbs.h"

#include "iup_object.h"
#include "iup_attrib.h"
#include "iup_str.h"
#include "iup_drv.h"
#include "iup_drvfont.h"

#include "iupwin_drv.h"
#include "iupwin_handle.h"
#include "iupwin_str.h"
#include "iupwin_info.h"


#ifndef MCS_NOSELCHANGEONNAV
#define MCS_NOSELCHANGEONNAV 0x0100
#endif


static int winCalendarSetValueAttrib(Ihandle* ih, const char* value)
{
  int year, month, day;
  if (sscanf(value, "%d/%d/%d", &day, &month, &year) == 3)
  {
    SYSTEMTIME st;

    if (month < 1) month = 1;
    if (month > 12) month = 12;
    if (day < 1) day = 1;
    if (day > 31) day = 31;

    st.wYear = (WORD)year;
    st.wMonth = (WORD)month;
    st.wDay = (WORD)day;

    SendMessage(ih->handle, MCM_SETCURSEL, 0, (LPARAM)&st);
  }
  return 0; /* do not store value in hash table */
}

static char* winCalendarGetValueAttrib(Ihandle* ih)
{
  SYSTEMTIME st;
  SendMessage(ih->handle, MCM_GETCURSEL, 0, (LPARAM)&st);
  return iupStrReturnStrf("%02d/%02d/%d", st.wDay, st.wMonth, st.wYear);
}

//MCM_SETTODAY
//MCM_GETTODAY 
//MCM_SETCOLOR

void iupdrvTextAddBorders(int *w, int *h);

static void winCalendarComputeNaturalSizeMethod(Ihandle* ih, int *w, int *h, int *children_expand)
{
  (void)children_expand; /* unset if not a container */

  if (ih->handle)
  {
    RECT rect;
    SendMessage(ih->handle, MCM_GETMINREQRECT, 0, (LPARAM)&rect);
    *w =  rect.right - rect.left;
    *h = rect.bottom - rect.top;
  }
  else
  {
    iupdrvFontGetMultiLineStringSize(ih, "W8W", w, h);

    (*w) *= 7; /* 7 columns */
    (*h) *= 8; /* 9 lines */

    iupdrvTextAddBorders(w, h);
  }
}


static int winCalendarWmNotify(Ihandle* ih, NMHDR* msg_info, int *result)
{
  if (msg_info->code == MCN_SELECT)
  {
    iupBaseCallValueChangedCb(ih);
  }

  (void)result;
  return 0; /* result not used */
}

static int winCalendarMapMethod(Ihandle* ih)
{
  DWORD dwStyle = WS_CHILD | WS_CLIPSIBLINGS | MCS_NOTODAY;

  if (!ih->parent)
    return IUP_ERROR;

  if (iupwinIsVistaOrNew())
    dwStyle |= MCS_NOSELCHANGEONNAV;

  if (iupAttribGetBoolean(ih, "CANFOCUS"))
    dwStyle |= WS_TABSTOP;

  if (iupAttribGetBoolean(ih, "WEEKNUMBERS"))
    dwStyle |= MCS_WEEKNUMBERS;

  if (!iupwinCreateWindow(ih, MONTHCAL_CLASS, 0, dwStyle, NULL))
    return IUP_ERROR;

  /* Process WM_NOTIFY */
  IupSetCallback(ih, "_IUPWIN_NOTIFY_CB", (Icallback)winCalendarWmNotify);

  return IUP_NOERROR;
}

Iclass* iupCalendarNewClass(void)
{
  Iclass* ic = iupClassNew(NULL);

  ic->name = "calendar";
  ic->format = NULL; /* none */
  ic->nativetype = IUP_TYPECONTROL;
  ic->childtype = IUP_CHILDNONE;
  ic->is_interactive = 1;

  /* Class functions */
  ic->New = iupCalendarNewClass;

  ic->Map = winCalendarMapMethod;
  ic->ComputeNaturalSize = winCalendarComputeNaturalSizeMethod;

  ic->LayoutUpdate = iupdrvBaseLayoutUpdateMethod;
  ic->UnMap = iupdrvBaseUnMapMethod;

  /* Callbacks */
  iupClassRegisterCallback(ic, "VALUECHANGED_CB", "");

  /* Common Callbacks */
  iupBaseRegisterCommonCallbacks(ic);

  /* Common */
  iupBaseRegisterCommonAttrib(ic);

  /* Visual */
  iupBaseRegisterVisualAttrib(ic);

  /* IupCalendar only */
  iupClassRegisterAttribute(ic, "VALUE", winCalendarGetValueAttrib, winCalendarSetValueAttrib, NULL, NULL, IUPAF_NO_DEFAULTVALUE | IUPAF_NO_INHERIT);
  //WEEKNUMBERS

//  iupClassRegisterAttribute(ic, "MIN", iValGetMinAttrib, iValSetMinAttrib, IUPAF_SAMEASSYSTEM, "0", IUPAF_NOT_MAPPED);
//  iupClassRegisterAttribute(ic, "TYPE", iValGetOrientationAttrib, iValSetOrientationAttrib, IUPAF_SAMEASSYSTEM, "HORIZONTAL", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
//  iupClassRegisterAttribute(ic, "ORIENTATION", iValGetOrientationAttrib, iValSetOrientationAttrib, IUPAF_SAMEASSYSTEM, "HORIZONTAL", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
//  iupClassRegisterAttribute(ic, "INVERTED", iValGetInvertedAttrib, iValSetInvertedAttrib, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);

  return ic;
}

Ihandle *IupCalendar(void)
{
  return IupCreate("calendar");
}
