/** \file
 * \brief iuptree control internal definitions.
 *
 * See Copyright Notice in iup.h
 *  */
 
#ifndef __TREEDEF_H 
#define __TREEDEF_H

#include <iup.h>
#include <cd.h>

#include "itdraw.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Kinds of node */
#define BRANCH               0
#define LEAF                 1

/* States of node */
#define EXPANDED             0
#define COLLAPSED            1

/* Kinds of image operation */
#define TREE                 0
#define NODE                 1
#define NODEEXPANDED         2

/* Kinds of set mode */
#define INVERT               -1

typedef struct Node_ *Node;

/* Structure of each node of the tree */
struct Node_
{
 Node next;    /* Points to next node. NULL if node is the last node */
 int kind;     /* Node kind. (BRANCH or LEAF) */
 int depth;    /* Depth of the node. 0 = root */
 int state;    /* Branch state (EXPANDED or COLLAPSED). Unused if leaf */
 int visible;  /* Node's visibility (YES or NO [default] ) */
 int marked;   /* Is node marked? (YES or NO [default] ) */
 char* name;   /* Node's name */
 void* userid; /* User id that identifies the node */

 int imageinuse;                               /* YES or NO [default] */
 unsigned char image[NODE_SIZE] ;                       /* Node's image */ 
 unsigned long int color[NODE_COLORS] ;        /* Node's color */
 unsigned long int marked_color[NODE_COLORS] ; /* Node's marked color */
 long int text_color;
 
 int expandedimageinuse;                       /* YES or NO [default] */
 unsigned char expandedimage[NODE_SIZE] ;               /* Node's image */ 
 unsigned long int expandedcolor[NODE_COLORS] ;       /* Node's color */
 unsigned long int expandedmarked_color[NODE_COLORS]; /* Node's marked color */
};

/* Structure of the tree */
typedef struct _Ttree
{
  Node root ;      /* Treeview's root */
  Node selected ;  /* Marked node */
  Node starting ;  /* Starting node (when marking with the tree_shift) */

  int addexpanded ; /* Defines if the created branchs will be expanded */
  int tree_ctrl ;        /* Defines if the tree_ctrl will be active or not */
  int tree_shift ;       /* Defines if the tree_shift will be active or not */

  Ihandle  *self; /* Handle of tree */
  Ihandle  *texth;   /* Handle of text to rename node */
  
  cdCanvas *cdcanvas;
  cdCanvas *cddbuffer;

  /* Treeview's images */ 
  unsigned char image_leaf[NODE_SIZE] ;  
  unsigned char image_collapsed[NODE_SIZE] ; 
  unsigned char image_expanded[NODE_SIZE] ;              
  
  /* Treeview's colors */
  unsigned long int color_leaf[NODE_COLORS] ;
  unsigned long int color_collapsed[NODE_COLORS] ;
  unsigned long int color_expanded[NODE_COLORS] ;
  
  /* Treeview's marked colors */
  unsigned long int marked_color_leaf[NODE_COLORS];
  unsigned long int marked_color_collapsed[NODE_COLORS];
  unsigned long int marked_color_expanded[NODE_COLORS];
  
  int YmaxC; /* Variables to store actual canvas size */
  int XmaxC; /* Set each time a callback is called.   */
  int selected_y ; /* Marked node's horizontal pos. in the canvas (in pixels) */
  int maxx ; /* Treeview's maxximum x */
} Ttree;

typedef Ttree* TtreePtr;

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

/* Macros for acessing tree's data structure */


#define tree_data(_)   (treeTreeGetData(_))

#define YmaxCanvas(n)   ((n)->YmaxC)
#define XmaxCanvas(n)   ((n)->XmaxC)

#define tree_self(_)  ((_)->self)
#define tree_texth(_)    ((_)->texth)

#define tree_image_leaf(_)         ((_)->image_leaf)		
#define tree_image_collapsed(_)    ((_)->image_collapsed)		
#define tree_image_expanded(_)     ((_)->image_expanded)		

#define tree_color_leaf(_)         ((_)->color_leaf)		
#define tree_color_collapsed(_)    ((_)->color_collapsed)		
#define tree_color_expanded(_)     ((_)->color_expanded)		

#define tree_marked_color_leaf(_)      ((_)->marked_color_leaf)		
#define tree_marked_color_collapsed(_) ((_)->marked_color_collapsed)		
#define tree_marked_color_expanded(_)  ((_)->marked_color_expanded)		

#define tree_root(_)          ((_)->root)
#define tree_selected(_)        ((_)->selected)
#define tree_starting(_)        ((_)->starting)
#define tree_selected_y(_)      ((_)->selected_y)
#define tree_maxx(_)          ((_)->maxx)	
#define tree_addexpanded(_)   ((_)->addexpanded)
#define tree_ctrl(_)   ((_)->tree_ctrl)
#define tree_shift(_)   ((_)->tree_shift)

#define node_next(_)     ((_)->next)
#define node_kind(_)     ((_)->kind)
#define node_depth(_)    ((_)->depth)   
#define node_name(_)     ((_)->name)
#define node_state(_)    ((_)->state)           
#define node_marked(_)   ((_)->marked)
#define node_visible(_)  ((_)->visible)
#define node_userid(_)   ((_)->userid)

#define node_imageinuse(_)     ((_)->imageinuse)
#define node_image(_)          ((_)->image)
#define node_color(_)          ((_)->color)
#define node_marked_color(_)   ((_)->marked_color)

#define node_expandedimageinuse(_)     ((_)->expandedimageinuse)
#define node_expandedimage(_)          ((_)->expandedimage)
#define node_expandedcolor(_)          ((_)->expandedcolor)
#define node_expandedmarked_color(_)   ((_)->expandedmarked_color)
#define node_text_color(_)   ((_)->text_color)

TtreePtr treeTreeGetData(Ihandle* self);
int treeRepaint(Ihandle *h);

#define NO      0
#define YES     1

#ifdef __cplusplus
}
#endif

#endif
