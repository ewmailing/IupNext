/** \file
 * \brief iuptree control internal definitions.
 *
 * See Copyright Notice in iup.h
 */

#ifndef __IUP_TREE_H 
#define __IUP_TREE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Kinds of node */
#define ITREE_BRANCH        0
#define ITREE_LEAF          1

#define ITREE_NODE_WIDTH   16
#define ITREE_NODE_HEIGHT  16

void iupdrvTreeInitClass(Iclass* ic);

int iupdrvTreeAddNode(Ihandle* ih, const char* id_string, int kind);

int iupdrvTreeSetNameAttrib(Ihandle* ih, const char* name_id, const char* value);
char* iupdrvTreeGetNameAttrib(Ihandle* ih, const char* name_id);

/* Structure of the tree */
struct _IcontrolData
{
  int tree_ctrl;    /* Defines   tree_ctrl: active or not */
  int tree_shift;   /* Defines  tree_shift: active or not */
  int show_rename;  /* Defines show_rename: active or not (only MOTIF) */

  int image_leaf;       /* Default image leaf */
  int image_collapsed;  /* Default image collapsed */
  int image_expanded;   /* Default image expanded */

  char* rename_caret;      /* caret position */
  char* rename_selection;  /* selection position */

  int sb;  /* scrollbar mode */
};


#ifdef __cplusplus
}
#endif

#endif
