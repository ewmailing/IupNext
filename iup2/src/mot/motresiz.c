/** \file
 * \brief Motif Driver element size calculus
 * implements iupdrvResizeObjects
 *
 * See Copyright Notice in iup.h
 * $Id: motresiz.c,v 1.1 2008-10-17 06:19:25 scuri Exp $
 */

#if defined(__STDC__) && defined(ULTRIX)
   #define X$GBLS(x)  #x
#endif

#include <assert.h>
#include <Xm/Xm.h>

#include "iglobal.h"
#include "motif.h"
#include "idrv.h"

static void motResizeBasicElement (Ihandle *n, int x, int y)
{
  if (n  &&  handle(n))
  {
    if(type(n) == FRAME_)
    {
      Iwidgetdata *d;
      Widget *gad;
      int dx = 0, dy = 0;

      XtVaGetValues ((Widget)handle (n), XmNuserData, &d, NULL);
      gad = (Widget*)d->data;

      if (gad[4])
      {
        int w, h;
        char *title = iupGetEnv(n, IUP_TITLE);

        iupdrvStringSize (n, title, &w, &h);
        if (w > currentwidth(n)-15) w = currentwidth(n)-15;
        dx = w + 10;
        dy = h/2;
        XtVaSetValues (gad[4],
          XmNx, (XtArgVal) x + 5,
          XmNy, (XtArgVal) y,
          XmNwidth, (XtArgVal) w + 4,
          XmNheight, (XtArgVal) h,
          XmNleftAttachment, XmATTACH_NONE,
          XmNrightAttachment, XmATTACH_NONE,
          XmNtopAttachment, XmATTACH_NONE,
          XmNbottomAttachment, XmATTACH_NONE,
          NULL);
      }

/*      
          x                   x+w-2
                    3

y+dy      +--------------------+
          |+------------------+|
          ||                  ||
          ||                  ||
 0        ||                  ||   2
          ||                  ||
          ||                  ||
          ||                  ||
y+dy+h-2  |+------------------+|
          +--------------------+

                    1
*/

      XtVaSetValues (gad[3],
        XmNx, (XtArgVal) x + dx + 1,
        XmNy, (XtArgVal) y + dy,
        XmNwidth, (XtArgVal) currentwidth (n) - dx - 1,
        XmNleftAttachment, XmATTACH_NONE,
        XmNrightAttachment, XmATTACH_NONE,
        XmNtopAttachment, XmATTACH_NONE,
        XmNbottomAttachment, XmATTACH_NONE,
        NULL);

      XtVaSetValues (gad[2],
        XmNx, (XtArgVal) x + currentwidth (n) - 2,
        XmNy, (XtArgVal) y + dy + 2,
        XmNheight, (XtArgVal) currentheight (n) - dy - 3,
        XmNleftAttachment, XmATTACH_NONE,
        XmNrightAttachment, XmATTACH_NONE,
        XmNtopAttachment, XmATTACH_NONE,
        XmNbottomAttachment, XmATTACH_NONE,
        NULL);

      XtVaSetValues (gad[1],
        XmNx, (XtArgVal) x + 2,
        XmNy, (XtArgVal) y + currentheight (n) - 2,
        XmNwidth, (XtArgVal) currentwidth (n) - 2,
        XmNleftAttachment, XmATTACH_NONE,
        XmNrightAttachment, XmATTACH_NONE,
        XmNtopAttachment, XmATTACH_NONE,
        XmNbottomAttachment, XmATTACH_NONE,
        NULL);

      XtVaSetValues (gad[0],
        XmNx, (XtArgVal) x,
        XmNy, (XtArgVal) y + dy,
        XmNheight, (XtArgVal) currentheight (n) - dy,
        XmNleftAttachment, XmATTACH_NONE,
        XmNrightAttachment, XmATTACH_NONE,
        XmNtopAttachment, XmATTACH_NONE,
        XmNbottomAttachment, XmATTACH_NONE,
        NULL);
    }
    else if(type(n) == MULTILINE_ || type(n) == CANVAS_)
    {
      XtVaSetValues (XtParent( (Widget)handle(n) ),
        XmNx, (XtArgVal) x,
        XmNy, (XtArgVal) y,
        XmNwidth, (XtArgVal) currentwidth (n),
        XmNheight, (XtArgVal) currentheight (n),
        XmNleftAttachment, XmATTACH_NONE,
        XmNrightAttachment, XmATTACH_NONE,
        XmNtopAttachment, XmATTACH_NONE,
        XmNbottomAttachment, XmATTACH_NONE,
        NULL);
    }
    else if(type(n) == DIALOG_)
    {
      Widget dlg_area = XtNameToWidget((Widget)handle(n), "*dialog_area");
      XtVaSetValues (XtParent(dlg_area),
        XmNresizePolicy, XmRESIZE_ANY,
        NULL);
      XtVaSetValues (dlg_area,
        XmNresizePolicy, XmRESIZE_ANY,
        XmNwidth, (XtArgVal) currentwidth (n),
        XmNheight, (XtArgVal) currentheight (n),
        NULL);
      XtVaSetValues (XtParent(dlg_area),
        XmNresizePolicy, XmRESIZE_NONE,
        NULL);
      XtVaSetValues (dlg_area,
        XmNresizePolicy, XmRESIZE_NONE,
        NULL);
    }
    else if(type(n) == LIST_ && iupCheck(n,IUP_DROPDOWN)!=YES && iupCheck(n,"EDITBOX")!=YES)
    {
      Widget sw;

      /* NOT ComboBox resize, ListBox only */

      sw = XtParent ((Widget)handle(n));

      XtVaSetValues (sw,
        XmNx, (XtArgVal) x,
        XmNy, (XtArgVal) y,
        XmNwidth, (XtArgVal) currentwidth (n),
        XmNheight, (XtArgVal) currentheight (n),
        XmNleftAttachment, XmATTACH_NONE,
        XmNrightAttachment, XmATTACH_NONE,
        XmNtopAttachment, XmATTACH_NONE,
        XmNbottomAttachment, XmATTACH_NONE,
        NULL);
    }
    else
    {
      XtVaSetValues ((Widget)handle(n),
        XmNx, (XtArgVal) x,
        XmNy, (XtArgVal) y,
        XmNwidth, (XtArgVal) currentwidth (n),
        XmNheight, (XtArgVal) currentheight (n),
        XmNleftAttachment, XmATTACH_NONE,
        XmNrightAttachment, XmATTACH_NONE,
        XmNtopAttachment, XmATTACH_NONE,
        XmNbottomAttachment, XmATTACH_NONE,
        NULL);
    }
  }
}

void iupdrvResizeObjects (Ihandle *n)
{
  Ihandle *c;
  assert(n);
  if(n == NULL) return;
  if(type(n) == DIALOG_)
  {
    motResizeBasicElement (n, posx(n), posy(n));
    iupdrvResizeObjects (child(n));
  }
  else if(type(n) == ZBOX_ || type(n) == HBOX_ || type(n) == VBOX_)
  {
    foreachchild(c,n)
      iupdrvResizeObjects (c);
  }
  else if(type(n) == FRAME_)
  {
    motResizeBasicElement (n, posx(n), posy(n));
    iupdrvResizeObjects (child(n));
  }
  else if(type(n) == RADIO_)
  {
    iupdrvResizeObjects (child(n));
  }
  else if(type(n) == CANVAS_ || type(n) == LIST_ || type(n) == TOGGLE_ ||
    type(n) == BUTTON_ || type(n) == LABEL_ || type(n) == TEXT_ ||
    type(n) == MULTILINE_)
  {
    motResizeBasicElement (n, posx(n), posy(n));
  }
}

