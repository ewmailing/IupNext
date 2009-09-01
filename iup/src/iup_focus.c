/** \file
 * \brief Keyboard Focus navigation
 *
 * See Copyright Notice in "iup.h"
 */

#include <stdio.h>

#include "iup.h"
#include "iupcbs.h"

#include "iup_object.h"
#include "iup_focus.h"
#include "iup_class.h"
#include "iup_assert.h"
#include "iup_attrib.h"
#include "iup_drv.h"


Ihandle* iupFocusNextInteractive(Ihandle *ih)
{
  Ihandle *c;

  if (!ih)
    return NULL;

  for (c = ih->brother; c; c = c->brother)
  {
    if (c->iclass->is_interactive)
      return c;
  }

  return NULL;
}

int iupFocusCanAccept(Ihandle *ih)
{
  if (ih->iclass->is_interactive &&  /* interactive */
      iupAttribGetBoolean(ih, "CANFOCUS") &&   /* can receive focus */
      ih->handle &&                  /* mapped  */
      IupGetInt(ih, "ACTIVE") &&     /* active  */
      IupGetInt(ih, "VISIBLE"))      /* visible */
    return 1;
  else
    return 0;
}

static Ihandle* iFocusFindAtBrothers(Ihandle *start)
{
  Ihandle *c;
  Ihandle *nf;

  for (c = start; c; c = c->brother)
  {
    /* check itself */
    if (iupFocusCanAccept(c))
      return c;

    /* then check its children */
    nf = iFocusFindAtBrothers(c->firstchild);
    if (nf)
      return nf;
  }

  return NULL;
}

Ihandle* iupGetNextFocus(Ihandle *ih)
{
  Ihandle *nf, *p;

  if (!ih)
    return NULL;

  /* look down in the child tree */
  if (ih->firstchild)
  {
    nf = iFocusFindAtBrothers(ih->firstchild);
    if (nf) return nf;
  }

  /* look in the same level */
  if (ih->brother)
  {
    nf = iFocusFindAtBrothers(ih->brother);
    if (nf) return nf;
  }

  /* look up in the brothers of the parent level */
  if (ih->parent)
  {
    for (p = ih->parent; p; p = p->parent)
    {
      if (p->brother)
      {
        nf = iFocusFindAtBrothers(p->brother);
        if (nf) return nf;
      }
    }
  }

  return NULL;
}

Ihandle* IupNextField(Ihandle *ih)
{
  Ihandle *ih_next;

  iupASSERT(iupObjectCheck(ih));
  if (!iupObjectCheck(ih))
    return NULL;

  ih_next = iupGetNextFocus(ih);
  if (!ih_next)
  {
    /* not found after the element, then start over from the begining,
       at the dialog. */
    ih_next = iupGetNextFocus(IupGetDialog(ih));
    if (ih_next == ih)
      return NULL;
  }

  if (ih_next)
  {
    IupSetFocus(ih_next);
    return ih_next;
  }

  return NULL;
}

static int iupFindPreviousFocus(Ihandle *parent, Ihandle **previous, Ihandle *ih)
{
  Ihandle *c;

  if (!parent)
    return 0;

  for (c = parent->firstchild; c; c = c->brother)
  {
    if (c == ih)
    {
      /* if found child, returns the current previous.
         but if previous is NULL, then keep searching until the last element. */
      if (*previous)
        return 1;
    }
    else
    {
      /* save the possible previous */
      if (iupFocusCanAccept(c))
        *previous = c;
    }

    /* then check its children */
    if (iupFindPreviousFocus(c, previous, ih))
      return 1;
  }

  return 0;
}

Ihandle* IupPreviousField(Ihandle *ih)
{
  Ihandle *previous = NULL;

  iupASSERT(iupObjectCheck(ih));
  if (!iupObjectCheck(ih))
    return NULL;

  /* search from the dialog down to the element */
  iupFindPreviousFocus(IupGetDialog(ih), &previous, ih);
  
  if (previous)
  {
    IupSetFocus(previous);
    return previous;
  }

  return NULL;
}


/* local variables */
static Ihandle* iup_current_focus = NULL;

Ihandle *IupSetFocus(Ihandle *ih)
{
  Ihandle* old_focus = iup_current_focus;

  iupASSERT(iupObjectCheck(ih));
  if (!iupObjectCheck(ih))
    return old_focus;

  /* iup_current_focus is NOT set here, 
     only in the iupCallGetFocusCb */

  if (iupFocusCanAccept(ih))  
    iupdrvSetFocus(ih);

  return old_focus;
}

Ihandle *IupGetFocus(void)
{
  return iup_current_focus;
}

void iupCallGetFocusCb(Ihandle *ih)
{
  Icallback cb;

  if (ih == iup_current_focus)  /* avoid duplicate messages */
    return;

  cb = (Icallback)IupGetCallback(ih, "GETFOCUS_CB");
  if (cb) cb(ih);

  if (ih->iclass->nativetype == IUP_TYPECANVAS)
  {
    IFni cb2 = (IFni)IupGetCallback(ih, "FOCUS_CB");
    if (cb2) cb2(ih, 1);
  }

  iup_current_focus = ih;
}

void iupCallKillFocusCb(Ihandle *ih)
{
  Icallback cb;

  if (ih != iup_current_focus)  /* avoid duplicate messages */
    return;

  cb = IupGetCallback(ih, "KILLFOCUS_CB");
  if (cb) cb(ih);

  if (ih->iclass->nativetype == IUP_TYPECANVAS)
  {
    IFni cb2 = (IFni)IupGetCallback(ih, "FOCUS_CB");
    if (cb2) cb2(ih, 0);
  }

  iup_current_focus = NULL;
}
