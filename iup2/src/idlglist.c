/** \file
 * \brief list of all created dialogs
 *
 * See Copyright Notice in iup.h
 * $Id: idlglist.c,v 1.1 2008-10-17 06:19:20 scuri Exp $
 */

#include <stdlib.h>

#include "iup.h"

#include "idlglist.h"

typedef struct Idiallst_
{
  Ihandle *ih;
  struct Idiallst_ *next;
} Idiallst;

static Idiallst *idlglist = NULL;  /* list of all created dialogs */

void iupDlgListAdd (Ihandle *ih)
{
  if (ih)
  {
    Idiallst *p=(Idiallst *)malloc(sizeof(Idiallst));
    if (!p)
      return;
    p->ih = ih;
    p->next = idlglist;
    idlglist=p;
  }
}

void iupDlgListDelete (Ihandle *ih)
{
  if (idlglist == NULL)
    return;
  if (idlglist->ih == ih)		/* ih is header */
  {
    Idiallst *p=idlglist->next;
    free(idlglist);
    idlglist=p;
  }
  else
  {
    Idiallst *p;		/* current pointer */
    Idiallst *b;		/* before pointer */
    for (b = idlglist, p = idlglist->next; p; b = p, p = p->next)
    {
      if (p->ih == ih)
      {
        b->next = p->next;
        free (p);
        return;
      }
    }
  }
}

static Idiallst *idlg_first = NULL;

Ihandle *iupDlgListFirst (void)
{
  idlg_first = idlglist;
  return iupDlgListNext();
}

Ihandle *iupDlgListNext (void)
{
  Ihandle *ih = NULL;
  if (idlg_first)
  {
    ih = idlg_first->ih;
    idlg_first = idlg_first->next;
  }
  return ih;
}
