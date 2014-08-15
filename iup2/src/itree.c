/** \file
 * \brief Control tree hierarchy manager.  
 * implements also IupDestroy
 *
 * See Copyright Notice in iup.h
 *  */

#include <stdio.h>  /* NULL */
#include <assert.h> /* ASSERT */

#include "iglobal.h"
#include "idlglist.h"
#include "idrv.h"
#include "itree.h"

static Ihandle *iTreeLastChild (Ihandle *root)
{
  Ihandle *c;
  if ((root == NULL) || (child(root) == NULL))
    return NULL;
  for (c=child(root); brother(c); c=brother(c))
    ; /* empty*/
  return c;
}

Ihandle *iupTreeAppendNode (Ihandle *root, Ihandle *n)
{
  if ((n == NULL) || (root == NULL))
    return NULL;
  parent(n) = root;
  if (child(root) == NULL)
    child(root) = n;
  else
    brother(iTreeLastChild(root)) = n;
  return root;
}

Ihandle *iupTreeCreateNode (Ihandle *root)
{
  Ihandle *n= (Ihandle *) malloc (sizeof(Ihandle));
  if (n == NULL)
    return NULL;
    
  number(n) = -1;
  type(n) = UNKNOWN_;
  hclass(n) = NULL;
  env(n) = iupTableCreate(IUPTABLE_STRINGINDEXED);
  handle(n) = NULL;
  usersize(n) = 0;
  naturalwidth(n) = 0;
  naturalheight(n) = 0;
  currentwidth(n) = 0;
  currentheight(n) = 0;
  posx(n) = 0;
  posy(n) = 0;
  parent(n) = root;
  child(n) = NULL;
  brother(n) = NULL;
  
  if (root)                      /* insert first element */
  {
    if (child(root) == NULL)      /* insert first child */
      child(root) = n;
    else
      brother(iTreeLastChild(root)) = n;
    return root;
  }
  else
    return n;
}

Ihandle* iupTreeGetParent( Ihandle* self )
{
  if (self == NULL)
    return self;
    
  return parent(self);
}

static Ihandle* iTreeFindChild ( Ihandle* root, Ihandle* current, Ifindchild_fn fn )
{
  Ihandle* result = NULL;

  if (fn(root,current))
    return current;
  else if (child(current))
    result = iTreeFindChild(root, child(current), fn);

  if (result == NULL && brother(current))
    result = iTreeFindChild(root, brother(current), fn);

  return result;
}

Ihandle* iupTreeFindChild( Ihandle* self, Ifindchild_fn fn)
{
  if (self == NULL || child(self) == NULL)
    return NULL;
    
  return iTreeFindChild(self, child(self), fn);
}

Ihandle* iupTreeGetFirstChild(Ihandle* self)
{
  if(self == NULL)
    return NULL;

  return child(self);
}

Ihandle* iupTreeGetBrother(Ihandle* self)
{
  if(self == NULL)
    return NULL;

  return brother(self);
}

/* funcao recursiva p/ destruir uma arvore */
void iupTreeKill (Ihandle *root)
{
  if (root == NULL)
    return;

  /* remove from its parent child list */
  IupDetach(root);

  iupCpiDestroy(root);

  /* destroy all its children */
  while (child(root))
    iupTreeKill (child(root));

  /* destroy the native interface element */
  if (handle(root))
    iupdrvDestroyHandle(root);

  /* Frees alocations done inside the commom code */
  if(type(root) == DIALOG_)
  {
    /* Removes dialog from dialog list */
    iupDlgListDelete(root);
  } 
  else if (type(root) == IMAGE_)
  {
    if(image_data(root))
      free(image_data(root));
  }

  { /* destroy all the names */
    char *name;
    while( (name=IupGetName (root)) != NULL)
      IupSetHandle(name,NULL);
  }
  
  iupTableDestroy(env(root));
  env(root) = NULL;
  free(root);
}

void IupDestroy (Ihandle *root)
{
  iupTreeKill(root);
}
