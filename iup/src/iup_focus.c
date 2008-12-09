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
#include "iup_drv.h"


static Ihandle* iFocusGetChildInteractive(Ihandle *ih)
{
  Ihandle *c;
  Ihandle *nf;

  if (!ih)
    return NULL;

  for (c = ih->firstchild; c; c = c->brother)
  {
    if (c->iclass->is_interactive)
      return c;

    nf = iFocusGetChildInteractive(c);
    if (nf)
      return nf;
  }

  return NULL;
}

Ihandle* iupFocusNextInteractive(Ihandle *ih)
{
  Ihandle *nf, *p;

  if (!ih)
    return NULL;

  /* look down in the child tree */
  nf = iFocusGetChildInteractive(ih);
  if (nf)
    return nf;

  /* look up in the tree */
  for (p = ih->parent; p; p = p->parent)
  {
    Ihandle *c;
    for (c = p->firstchild; c; c = c->brother)
    {
      if (c != ih) /* do not check again the current child tree */
      {
        if (c->iclass->is_interactive)
          return c;

        nf = iFocusGetChildInteractive(c);
        if (nf)
          return nf;
      }
    }
  }

  return NULL;
}

int iupFocusCanAccept(Ihandle *ih)
{
  if (ih->iclass->is_interactive &&  /* interactive */
      ih->handle &&               /* mapped  */
      IupGetInt(ih, "ACTIVE") &&  /* active  */
      IupGetInt(ih, "VISIBLE"))   /* visible */
    return 1;
  else
    return 0;
}

static Ihandle* iFocusGetChild(Ihandle *ih)
{
  Ihandle *c;
  Ihandle *nf;

  if (!ih)
    return NULL;

  for (c = ih->firstchild; c; c = c->brother)
  {
    if (iupFocusCanAccept(c))
      return c;

    nf = iFocusGetChild(c);
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
  nf = iFocusGetChild(ih);
  if (nf)
    return nf;

  /* look up in the tree */
  for (p = ih->parent; p; p = p->parent)
  {
    Ihandle *c;
    for (c = p->firstchild; c; c = c->brother)
    {
      if (c != ih) /* do not check again the current child tree */
      {
        if (iupFocusCanAccept(c))
          return c;

        nf = iFocusGetChild(c);
        if (nf)
          return nf;
      }
    }
  }

  return NULL;
}

Ihandle* IupPreviousField(Ihandle *ih)
{
  Ihandle *ih_previous = NULL;
  Ihandle *ih_next;

  iupASSERT(iupObjectCheck(ih));
  if (!iupObjectCheck(ih))
    return NULL;

  ih_next = IupGetDialog(ih);

  for (;;)
  {
    ih_next = iupGetNextFocus(ih_next);

    if (((ih_next == NULL) || (ih_next == ih)) && ih_previous)
    {
      IupSetFocus(ih_previous);
      return ih_previous;
    }

    ih_previous = ih_next;
  }
}

Ihandle* IupNextField(Ihandle *ih)
{
  Ihandle *ih_next;

  iupASSERT(iupObjectCheck(ih));
  if (!iupObjectCheck(ih))
    return NULL;

  ih_next = iupGetNextFocus(ih);
  if (ih_next)
  {
    IupSetFocus(ih_next);
    return ih_next;
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
