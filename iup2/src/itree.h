/** \file
 * \brief Tree definitions.
 *
 * See Copyright Notice in iup.h
 *  */
 
#ifndef __I_TREE_H 
#define __I_TREE_H

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "iup.h"
#include "iupcpi.h"
#include "itable.h"
#include "ifunc.h"
#include "inames.h"
#include "ilex.h"
#include "imacros.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*Ifindchild_fn) ( Ihandle*,Ihandle*);
Ihandle* iupTreeCreateNode (Ihandle* root);
Ihandle* iupTreeCopyNode   (Ihandle* root, Ihandle *n);
Ihandle* iupTreeAppendNode (Ihandle* root, Ihandle* n);
Ihandle* iupTreeFindNode   (int n);
void     iupTreePrint      (Ihandle* root);
Ihandle* iupTreeGetParent( Ihandle* self );
Ihandle* iupTreeFindChild( Ihandle* self, Ifindchild_fn fn);
Ihandle* iupTreeGetFirstChild(Ihandle* self);
Ihandle* iupTreeGetBrother(Ihandle* self);
void     iupTreeKill           (Ihandle *root);

#ifdef __cplusplus
}
#endif

#endif
