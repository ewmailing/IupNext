/** \file
 * \brief Frame Control
 *
 * See Copyright Notice in "iup.h"
 */

#include <windows.h>
#include <commctrl.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <stdarg.h>

#include "iup.h"
#include "iupcbs.h"

#include "iup_object.h"
#include "iup_attrib.h"
#include "iup_str.h"
#include "iup_progressbar.h"
#include "iup_drv.h"

#include "iupwin_drv.h"
#include "iupwin_handle.h"


#ifndef PBS_MARQUEE             /* it is defined only when _WIN32_WINNT >= 0x0501 */
#define PBS_MARQUEE             0x08
#define PBM_SETMARQUEE          (WM_USER+10)
#endif

#define IUP_PB_MAX 32000


static int winProgressBarSetMarqueeAttrib(Ihandle* ih, const char* value)
{
  /* MARQUEE only works when using XP Styles */
  if (!iupwin_comctl32ver6)
    return 0;

  if (iupStrBoolean(value))
  {
    ih->data->marquee = 1;
    iupwinSetStyle(ih, PBS_MARQUEE, 1);
    SendMessage(ih->handle, PBM_SETMARQUEE, TRUE, 100);
  }
  else
  {
    SendMessage(ih->handle, PBM_SETMARQUEE, FALSE, 0);
    iupwinSetStyle(ih, PBS_MARQUEE, 0);
    ih->data->marquee = 0;
  }

  iupwinRedrawNow(ih);

  return 1;
}

static int winProgressBarSetValueAttrib(Ihandle* ih, const char* value)
{
  if (!value)
    ih->data->value = 0;
  else
    ih->data->value = atof(value);

  iProgressBarCropValue(ih);

  /* Shows when the marquee style is not set */
  if (!ih->data->marquee)
  {
    double factor = (ih->data->value - ih->data->vmin) / (ih->data->vmax - ih->data->vmin);
    int val = (int)(IUP_PB_MAX * factor);
    SendMessage(ih->handle, PBM_SETPOS, (WPARAM)val, 0);
  }

  return 0;
}

static int winProgressBarSetBgColorAttrib(Ihandle* ih, const char* value)
{
  unsigned char r, g, b;

  /* Only works when using Classic style */
  if (iupwin_comctl32ver6)
    return 0;

  if (iupStrToRGB(value, &r, &g, &b))
  {
    COLORREF color = RGB(r,g,b);
    SendMessage(ih->handle, PBM_SETBKCOLOR, 0, (LPARAM)color);
  }
  else
    SendMessage(ih->handle, PBM_SETBKCOLOR, 0, (LPARAM)CLR_DEFAULT);
  return 1;
}

static int winProgressBarSetFgColorAttrib(Ihandle* ih, const char* value)
{
  unsigned char r, g, b;

  /* Only works when using Classic style */
  if (iupwin_comctl32ver6)
    return 0;

  if (iupStrToRGB(value, &r, &g, &b))
  {
    COLORREF color = RGB(r,g,b);
    SendMessage(ih->handle, PBM_SETBARCOLOR, 0, (LPARAM)color);
  }
  else
    SendMessage(ih->handle, PBM_SETBARCOLOR, 0, (LPARAM)CLR_DEFAULT);
  return 1;
}

static int winProgressBarMapMethod(Ihandle* ih)
{
  DWORD dwStyle = WS_CHILD;

  if (!ih->parent)
    return IUP_ERROR;

  if (iupStrEqualNoCase(iupAttribGetStr(ih, "ORIENTATION"), "VERTICAL"))
  {
    dwStyle |= PBS_VERTICAL;

    if (ih->currentheight < ih->currentwidth)
    {
      int tmp = ih->currentheight;
      ih->currentheight = ih->currentwidth;
      ih->currentwidth = tmp;
    }
  }

  if (!iupwin_comctl32ver6 && !iupStrBoolean(iupAttribGetStr(ih, "DASHED")))
    dwStyle |= PBS_SMOOTH;

  if (!iupwinCreateWindowEx(ih, PROGRESS_CLASS, 0, dwStyle))
    return IUP_ERROR;

  /* ensure the default values, that are different from the native ones */
  SendMessage(ih->handle, PBM_SETRANGE, 0, MAKELPARAM(0, IUP_PB_MAX));

  return IUP_NOERROR;
}

void iupdrvProgressBarInitClass(Iclass* ic)
{
  /* Driver Dependent Class functions */
  ic->Map = winProgressBarMapMethod;

  /* Visual */
  iupClassRegisterAttribute(ic, "BGCOLOR", NULL, winProgressBarSetBgColorAttrib, NULL, IUP_MAPPED, IUP_INHERIT);  

  /* Special */
  iupClassRegisterAttribute(ic, "FGCOLOR", NULL, winProgressBarSetFgColorAttrib, NULL, IUP_NOT_MAPPED, IUP_INHERIT);

  /* IupProgressBar only */
  iupClassRegisterAttribute(ic, "VALUE",  iProgressBarGetValueAttrib,  winProgressBarSetValueAttrib,  NULL,  IUP_MAPPED, IUP_NO_INHERIT);
  iupClassRegisterAttribute(ic, "ORIENTATION", NULL, NULL, "HORIZONTAL", IUP_MAPPED, IUP_NO_INHERIT);
  iupClassRegisterAttribute(ic, "MARQUEE",     NULL, winProgressBarSetMarqueeAttrib, NULL, IUP_MAPPED, IUP_NO_INHERIT);
}
