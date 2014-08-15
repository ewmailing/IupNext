/** \file
 * \brief Control hierarchy manager.  
 *
 * See Copyright Notice in iup.h
 *  */

#include <stdio.h>
#include <assert.h>

#include "iglobal.h"
#include "idlglist.h"
#include "idrv.h"
#include "itree.h"

Ihandle *iupGetRadio(Ihandle *n)
{
  Ihandle *p;
  if (n == NULL)
    return NULL;

  for (p=n; parent(p); p=parent(p))
  {
    if (type(p) == RADIO_)
      return p;
  }

  return NULL;
}

Ihandle *IupGetDialog(Ihandle *n)
{
  Ihandle *p;
  if (n == NULL)
    return NULL;

  /* busca o elemento raiz da arvore onde n se encontra */
  for (p=n; parent(p); p=parent(p))
    ; /* empty*/

  /* verifica o tipo do elemento raiz. Existem  */
  /* dois tipos validos: DIALOG_ MENU_.         */
  /*      DIALOG_: e' o elemento que estava     */
  /*               sendo procurado, retorna ele */
  /*      MENU_: e' preciso procurar o dialogo  */
  /*             que o menu esta' associado.    */
  if (type(p) == DIALOG_)
    return p;
  else if (type(p) == MENU_)
  {
    Ihandle *d;
    /* varre todos os dialogos procurando o menu */
    for (d=iupDlgListFirst(); d; d=iupDlgListNext())
    {
      char *menu=IupGetAttribute (d, IUP_MENU);
      if (menu && (IupGetHandle(menu) == p))
        return d;
    }
  }
  return NULL;
}

/* desliga uma sub-arvore da arvore pai */
void IupDetach (Ihandle *chld)
{
  Ihandle *prev=NULL;
  Ihandle *root;
  Ihandle *c;

  if (chld == NULL)
    return;

  root=parent(chld);
  if (root == NULL)
    return;

  /* Cleans the child entry inside the parent's child list */
  foreachchild(c,root)
  {
    if (c == chld) /* Found the right child */
    {
      if (prev == NULL)
        child(root)=brother(chld);
      else
        brother(prev)=brother(chld);
        
      brother(chld)=NULL;
      parent(chld)=NULL;
      return;
    }
    prev=c;
  }
}

Ihandle* IupAppend (Ihandle *box, Ihandle *exp)
{
  return iupTreeAppendNode (box, exp);
}

Ihandle* IupGetChild(Ihandle* ih, int pos)
{
  int i;
  Ihandle* ih_child;

  if (!ih)
    return NULL;

  for (i = 0, ih_child = child(ih); i < pos && ih_child; i++)
    ih_child = brother(ih_child);

  return ih_child;
}

Ihandle *IupGetNextChild(Ihandle *parent, Ihandle *next)
{
  if(next == NULL)
    return iupTreeGetFirstChild(parent);
  else
    return iupTreeGetBrother(next);
}

Ihandle *IupGetBrother(Ihandle *brother)
{
  return iupTreeGetBrother(brother);
}

Ihandle *IupGetParent(Ihandle *child)
{
  return iupTreeGetParent(child);
}
