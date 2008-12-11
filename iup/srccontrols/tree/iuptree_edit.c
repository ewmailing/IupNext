/** \file
 * \brief iuptree control
 * Functions used to edit a node name in place.
 *
 * See Copyright Notice in "iup.h"
 */

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <stdarg.h>

#include "iup.h"
#include "iupcbs.h"
#include "iuptree.h"
#include "iupkey.h"

#include "iup_object.h"
#include "iup_attrib.h"
#include "iup_globalattrib.h"
#include "iup_str.h"
#include "iup_drv.h"
#include "iup_drvfont.h"
#include "iup_stdcontrols.h"
#include "iup_controls.h"
#include "iup_dialog.h"
#include "iup_childtree.h"

#include <cd.h>

#include "iuptree_draw.h"
#include "iuptree_def.h"
#include "iuptree_edit.h"
#include "iuptree_callback.h"
#include "iuptree_getset.h"


static int iTreeEditTextActionCB(Ihandle* ih, int c, char *after)
{
  if(!IupGetInt(ih, "VISIBLE"))
    return IUP_DEFAULT;

  if(c == K_ESC)
  {
    IupSetAttribute(ih->data->texth, "VISIBLE", "NO");
    IupSetAttribute(ih->data->texth, "ACTIVE",  "NO");
    IupSetFocus(ih);
    return IUP_DEFAULT;
  }

  if(c == K_CR)
  {
    int ret;
    IupSetAttribute(ih, "_IUPTREE_CALL_RENAME", "1");
    ret = iupTreeCallbackRenameCB(ih, after);
    IupSetAttribute(ih, "_IUPTREE_CALL_RENAME", NULL);

    if(ret == IUP_IGNORE)
      return IUP_DEFAULT;

    IupSetAttribute(ih->data->texth, "VISIBLE", "NO");
    IupSetAttribute(ih->data->texth, "ACTIVE",  "NO");
    IupSetFocus(ih);

    return IUP_DEFAULT;
  }

  return IUP_DEFAULT;
}

#ifdef _MOTIF_
static int iTreeEditKeyAnyTextCB(Ihandle* ih, int c)
{
  int ret = IUP_DEFAULT;

  /* In motif, these keys are not handled in IupText ACTION. */
  if(c == K_ESC || c == K_CR)
    ret = iTreeEditTextActionCB(ih, c, IupGetAttribute(ih, "VALUE"));

  return ret;
}
#endif

/* called whenever something happened that 
   the edit control should be closed if visible */
void iupTreeEditCheckHidden(Ihandle* ih)
{
  if(IupGetInt(ih->data->texth, "VISIBLE"))
  {
    char* value;

    /* if the user caused a kill focus during a RENAME_CB just ignore it. */
    if(IupGetInt(ih, "_IUPTREE_CALL_RENAME") == 1)
      return;

    value = iupStrDup(IupGetAttribute(ih->data->texth, "VALUE"));
    iupTreeCallbackRenameCB(ih, value);
    free(value);
    IupSetAttribute(ih->data->texth, "VISIBLE", "NO");
    IupSetAttribute(ih->data->texth, "ACTIVE",  "NO");
  }
}

static int iTreeEditTextKillFocusCB(Ihandle* ih)
{
#ifdef _MOTIF_
  if(IupGetInt(ih, "_IUPTREE_DOUBLE_CLICK"))
  {
    IupSetAttribute(ih, "_IUPTREE_DOUBLE_CLICK", NULL);
    return IUP_DEFAULT;
  }
#endif

  iupTreeEditCheckHidden(ih);
  return IUP_DEFAULT;
}

void iupTreeEditShow(Ihandle* ih, int text_x, int x, int y)
{
  char* s = iupStrGetMemory(30);
  int w, h;
  int charwidth, charheight;

  /* active */
  if(!IupGetInt(ih, "ACTIVE"))
    return;

  /* notify application */
  if(iupTreeCallbackShowRenameCB(ih) == IUP_IGNORE)
    return;

  /* calc size */
  x = x + ITREE_NODE_WIDTH;
  y = y - ITREE_TEXT_BOX_OFFSET_Y - 1;
  w = 2 * ITREE_TEXT_MARGIN_X - ITREE_TEXT_RIGHT_FIX + text_x + 2 + 15 + 15;
  h = ITREE_NODE_HEIGHT + 2;
  y = ih->data->YmaxC - y - ITREE_NODE_Y;

  if(x + w > ih->data->XmaxC)
    w = ih->data->XmaxC - x;

  /* set attributes */
  IupSetAttribute(ih->data->texth, "VALUE", iupTreeGSGetName(ih, iupTreeGSGetValue(ih)));  /* get the current node name selected */
  /* IupSetAttribute(ih->data->texth, "VALUE", IupGetAttribute(ih, "NAME")); TODO: check this */
  IupSetAttribute(ih->data->texth, "FONT",  IupGetAttribute(ih, "FONT"));

  /* set position */
  ih->data->texth->x = x;  /* POSX */
  ih->data->texth->y = y;  /* POSY */

  /* set size */
  IupSetfAttribute(ih->data->texth, "RASTERSIZE", "%dx%d", w, h);

  iupdrvFontGetCharSize(ih, &charwidth, &charheight);
  sprintf (s, "%dx%d", 4*w/charwidth, 8*h/charheight);
  IupSetAttribute(ih->data->texth, "SIZE", s);
  ih->data->texth->currentwidth  = w;
  ih->data->texth->currentheight = h;
  iupClassObjectLayoutUpdate(ih->data->texth);

  /* activate and show */
  IupSetAttribute(ih->data->texth, "ACTIVE",  "YES");
  IupSetAttribute(ih->data->texth, "VISIBLE", "YES");
  IupSetFocus(ih->data->texth);

  /* update text attributes */
  {
    char* selection = IupGetAttribute(ih, "SELECTION");
    if(selection)
    {
      /* this allow the user to set the SELECTION inside the SHOWRENAME_CB */
      iupAttribStoreStr(ih->data->texth, "SELECTION", selection);
      iupAttribSetStr(ih, "SELECTION", NULL);
    }
    else
    {
      char* caret = IupGetAttribute(ih, "CARET");
      if(caret)
      {
        /* this allow the user to set the CARET inside the SHOWRENAME_CB */
        iupAttribStoreStr(ih->data->texth, "CARET", caret);
        iupAttribSetStr(ih, "CARET", NULL);
      }
    }
  }
}

void iupTreeEditCreate(Ihandle* ih)
{
  ih->data->texth = IupText(NULL);
  iupChildTreeAppend(ih, ih->data->texth);

  IupSetCallback(ih->data->texth, "ACTION",       (Icallback)iTreeEditTextActionCB);
  IupSetCallback(ih->data->texth, "KILLFOCUS_CB", (Icallback)iTreeEditTextKillFocusCB);
  IupSetAttribute(ih->data->texth, "VALUE",  "");
  IupSetAttribute(ih->data->texth, "BORDER", "YES");
  IupSetAttribute(ih->data->texth, "VISIBLE", "NO");
  IupSetAttribute(ih->data->texth, "ACTIVE",  "NO");

#ifdef _MOTIF_
  IupSetCallback(ih->data->texth, "K_ANY", (Icallback)iTreeEditKeyAnyTextCB);
#else
  /* avoid callback inheritance */
  IupSetAttribute(ih->data->texth, "K_ANY", "__do_nothing__");

#endif
}
