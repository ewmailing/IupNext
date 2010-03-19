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

#define ITREE_MARK_SINGLE 0
#define ITREE_MARK_MULTIPLE 1

#define ITREE_UPDATEIMAGE_LEAF      1
#define ITREE_UPDATEIMAGE_COLLAPSED 2
#define ITREE_UPDATEIMAGE_EXPANDED  3

void iupdrvTreeInitClass(Iclass* ic);
void iupTreeUpdateImages(Ihandle *ih);
void iupdrvTreeAddNode(Ihandle* ih, const char* id_string, int kind, const char* title, int add);
void iupdrvTreeUpdateMarkMode(Ihandle *ih);

char* iupTreeGetSpacingAttrib(Ihandle* ih);

#if defined(GTK_MAJOR_VERSION)
typedef void InodeData;
#elif defined(XmVERSION)
typedef struct _WidgetRec InodeData;
#elif defined(WINVER)
typedef struct _TREEITEM InodeData;
#else
typedef struct _InodeData InodeData;
#endif

typedef int (*iupTreeNodeFunc)(Ihandle* ih, InodeData* node, int id, void* userdata);
int iupTreeForEach(Ihandle* ih, iupTreeNodeFunc func, void* userdata);
InodeData* iupTreeGetNode(Ihandle* ih, int id);
InodeData* iupTreeGetNodeFromString(Ihandle* ih, const char* name_id);
int iupTreeFindNodeId(Ihandle* ih, InodeData* node);

InodeData* iupdrvTreeGetFocusNode(Ihandle* ih);
int iupdrvTreeTotalChildCount(Ihandle* ih, InodeData* node);

void iupTreeDelFromCache(Ihandle* ih, int id, int count);
void iupTreeAddToCache(Ihandle* ih, int add, int kindPrev, InodeData* prevNode, InodeData* node);

/* Structure of the tree */
struct _IcontrolData
{
  int mark_mode,
      add_expanded,
      show_dragdrop,
      show_rename,
      stamp,    /* Used by GTK only */
      spacing;

  void* def_image_leaf;       /* Default image leaf */
  void* def_image_collapsed;  /* Default image collapsed */
  void* def_image_expanded;   /* Default image expanded */

  void* def_image_leaf_mask;       /* Motif Only */
  void* def_image_collapsed_mask;  
  void* def_image_expanded_mask;  

  InodeData* *node_cache;
  int node_cache_max, node_count;
};


#ifdef __cplusplus
}
#endif

#endif
