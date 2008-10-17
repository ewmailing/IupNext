/** \file
 * \brief iuptree control
 * Functions used to draw a tree.
 *
 * See Copyright Notice in iup.h
 * $Id: itdraw.h,v 1.1 2008-10-17 06:20:41 scuri Exp $
 */
 
#ifndef __ITDRAW_H 
#define __ITDRAW_H

#ifdef __cplusplus
extern "C" {
#endif

#define NODE_COLORS 256    /* Number of colors that can be used in a node */
#define BOX_COLORS 3       /* Number of colors that can be used in a box */

#define NODE_X 20          /* horizontal distance between nodes */
#define NODE_Y 18          /* vertical distance between nodes */
#define NODE_WIDTH 16      /* node width */ 
#define NODE_HEIGHT 16     /* node hieght */
#define NODE_SIZE (NODE_WIDTH*NODE_HEIGHT) /* node size */

#define TEXT_BOX_OFFSET 2   /* Text box distance from node */
#define TEXT_MARGIN_X 5     /* Text horizontal margin */
#define TEXT_MARGIN_Y 2     /* Text horizontal margin */
#define TEXT_RIGHT_MARGIN 5 /* Text margin (when the horiz. sb is needed) */ 
#define TEXT_BOX_OFFSET_Y 1 /* Text box bottom offset */
#define BUTTON_X - NODE_X  /* horizontal distance between branch and button */
#define BUTTON_Y -1        /* vertical distance between branch and button */
#define BUTTON_LINE_Y 6    /* vert. pos. of line that links button and branch */
#define BUTTON_LINE_X 10   /* horizontal position of line that links buttons */
#define BUTTON_BRANCH_Y 2  /* offset between node and button */
#define LEAF_LINE_X1 -13   /* left coordinate of line that links leafs */
#define LEAF_LINE_X2 -2    /* right coordinate of line that links leafs */
#define LEAF_LINE_Y1 7     /* bottom coordinate of line that links leafs */
#define LEAF_LINE_Y2 24    /* top coordinate of line that links leafs */
#define BRANCH_LINE_X -13  /* horizontal position of line that links buttons */
#define BRANCH_LINE_Y1 -1  /* bottom coordinate of line that links buttons */
#define BRANCH_LINE_Y2 19  /* top coordinate of line that links buttons */

#define FIRST_LEAF_Y -8    /* offset of the first leaf */
#define LAST_LEAF_Y -8     /* offset of the last leaf */       

#define TREE_LEFT_MARGIN 2     /* Treeview's left margin */
#define TREE_TOP_MARGIN  1     /* Treeview's top margin */
#define TREE_BOTTOM_MARGIN 1   /* Treeview's down margin */
#define TREE_RIGHT_MARGIN  5   /* Treeview's right margin */
#define TREE_HEIGHT     200    /* Treeview's height */
#define TREE_WIDTH      400    /* Treeview's width */

#define TEXT_RIGHT_FIX  1      /* Text right correction */
#define TREE_BGCOLOR 0xFFFFFFL /* Treeview's background color */
#define TREE_BGCOLORSTRING "255 255 255"
#define TREE_EXPAND_WIDTH 9    /* Treeview's expand width */ 
#define TREE_EXPAND_HEIGHT 9   /* Treeview's expand height */
#define TREE_COLLAPSE_WIDTH 9  /* Treeview's expand width */ 
#define TREE_COLLAPSE_HEIGHT 9 /* Treeview's expand height */

#define TREE_COLLAPSE_SIZE (TREE_COLLAPSE_WIDTH * TREE_COLLAPSE_HEIGHT)
#define TREE_EXPAND_SIZE (TREE_EXPAND_WIDTH * TREE_EXPAND_HEIGHT)

void treedrawDrawTree(Ihandle *h);

int treedrawGetTop(Ihandle *h);
int treedrawGetCanvasTop(Ihandle *h);
int treedrawGetLeft(Ihandle *h);
void treedrawSetVisibility(Ihandle *h);
	
#ifdef __cplusplus
}
#endif

#endif
