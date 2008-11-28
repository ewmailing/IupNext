/** \file
 * \brief iuptree control internal definitions.
 *
 * See Copyright Notice in iup.h
 */
 
#ifndef __TREEDEF_H 
#define __TREEDEF_H

#ifdef __cplusplus
extern "C" {
#endif

/* Kinds of node */
#define ITREE_BRANCH               0
#define ITREE_LEAF                 1

/* States of node */
#define ITREE_EXPANDED             0
#define ITREE_COLLAPSED            1

/* Kinds of image operation */
#define ITREE_TREE                 0
#define ITREE_NODE                 1
#define ITREE_NODEEXPANDED         2

/* Kinds of set mode */
#define ITREE_INVERT              -1


typedef struct Node_ *Node;

/* Structure of each node of the tree */
struct Node_
{
  Node next;     /* Points to next node. NULL if node is the last node */
  int kind;      /* Node kind. (ITREE_BRANCH or ITREE_LEAF) */
  int depth;     /* Depth of the node. 0 = root */
  int state;     /* Branch state (ITREE_EXPANDED or ITREE_COLLAPSED). Unused if leaf */
  int visible;   /* Node's visibility (YES or NO [default] ) */
  int marked;    /* Is node marked? (YES or NO [default] )   */
  char* name;    /* Node's name */
  void* userid;  /* User id that identifies the node */

  int imageinuse;                                     /* YES or NO [default] */
  unsigned char image[ITREE_NODE_SIZE];               /* Node's image */ 
  unsigned long int color[ITREE_NODE_COLORS];         /* Node's color */
  unsigned long int marked_color[ITREE_NODE_COLORS];  /* Node's marked color */
  long int text_color;
 
  int expandedimageinuse;                                     /* YES or NO [default] */
  unsigned char expandedimage[ITREE_NODE_SIZE];               /* Node's image */ 
  unsigned long int expandedcolor[ITREE_NODE_COLORS];         /* Node's color */
  unsigned long int expandedmarked_color[ITREE_NODE_COLORS];  /* Node's marked color */
};

/* Structure of the tree */
struct _IcontrolData
{
  iupCanvas canvas;  /* from IupCanvas (must reserve it) */

  Node root;        /* Treeview's root */
  Node selected;    /* Marked node     */
  Node starting;    /* Starting node (when marking with the tree_shift) */

  int addexpanded;  /* Defines if the created branches will be expanded */
  int tree_ctrl;    /* Defines if the tree_ctrl will be active or not   */
  int tree_shift;   /* Defines if the tree_shift will be active or not  */

  Ihandle*  texth;  /* Handle of text to rename node */
  
  cdCanvas* cdcanvas;
  cdCanvas* cddbuffer;

  /* Treeview's images */ 
  unsigned char image_leaf[ITREE_NODE_SIZE];  
  unsigned char image_collapsed[ITREE_NODE_SIZE]; 
  unsigned char image_expanded[ITREE_NODE_SIZE];              
  
  /* Treeview's colors */
  unsigned long int color_leaf[ITREE_NODE_COLORS];
  unsigned long int color_collapsed[ITREE_NODE_COLORS];
  unsigned long int color_expanded[ITREE_NODE_COLORS];
  
  /* Treeview's marked colors */
  unsigned long int marked_color_leaf[ITREE_NODE_COLORS];
  unsigned long int marked_color_collapsed[ITREE_NODE_COLORS];
  unsigned long int marked_color_expanded[ITREE_NODE_COLORS];
  
  int YmaxC;       /* Variables to store actual canvas size */
  int XmaxC;       /* Set each time a callback is called.   */
  int selected_y;  /* Marked node's horizontal pos. in the canvas (in pixels) */
  int maxx;        /* Treeview's maximum x */
};


int iTreeRepaint(Ihandle* ih);

#define NO      0
#define YES     1

#ifdef __cplusplus
}
#endif

#endif
