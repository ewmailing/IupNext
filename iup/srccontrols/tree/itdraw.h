/** \file
 * \brief iuptree control
 * Functions used to draw a tree.
 *
 * See Copyright Notice in iup.h
 */
 
#ifndef __ITDRAW_H 
#define __ITDRAW_H

#ifdef __cplusplus
extern "C" {
#endif

#define ITREE_NODE_COLORS  256   /* Number of colors that can be used in a node */
#define ITREE_BOX_COLORS     3   /* Number of colors that can be used in a box  */

#define ITREE_NODE_X       20    /* horizontal distance between nodes */
#define ITREE_NODE_Y       18    /* vertical distance between nodes   */
#define ITREE_NODE_WIDTH   16    /* node width  */ 
#define ITREE_NODE_HEIGHT  16    /* node hieght */
#define ITREE_NODE_SIZE    ITREE_NODE_WIDTH * ITREE_NODE_HEIGHT    /* node size */

#define ITREE_TEXT_BOX_OFFSET    2  /* Text box distance from node */
#define ITREE_TEXT_MARGIN_X      5  /* Text horizontal margin */
#define ITREE_TEXT_MARGIN_Y      2  /* Text horizontal margin */
#define ITREE_TEXT_RIGHT_MARGIN  5  /* Text margin (when the horiz. sb is needed) */ 
#define ITREE_TEXT_BOX_OFFSET_Y  1  /* Text box bottom offset */
#define ITREE_BUTTON_X          -ITREE_NODE_X  /* horizontal distance between branch and button */
#define ITREE_BUTTON_Y          -1  /* vertical distance between branch and button */
#define ITREE_BUTTON_LINE_Y      6  /* vert. pos. of line that links button and branch */
#define ITREE_BUTTON_LINE_X     10  /* horizontal position of line that links buttons  */
#define ITREE_BUTTON_BRANCH_Y    2  /* offset between node and button */
#define ITREE_LEAF_LINE_X1     -13  /* left coordinate of line that links leafs   */
#define ITREE_LEAF_LINE_X2      -2  /* right coordinate of line that links leafs  */
#define ITREE_LEAF_LINE_Y1       7  /* bottom coordinate of line that links leafs */
#define ITREE_LEAF_LINE_Y2      24  /* top coordinate of line that links leafs    */
#define ITREE_BRANCH_LINE_X    -13  /* horizontal position of line that links buttons */
#define ITREE_BRANCH_LINE_Y1    -1  /* bottom coordinate of line that links buttons   */
#define ITREE_BRANCH_LINE_Y2    19  /* top coordinate of line that links buttons      */

#define ITREE_FIRST_LEAF_Y  -8  /* offset of the first leaf */
#define ITREE_LAST_LEAF_Y   -8  /* offset of the last leaf  */       

#define ITREE_TREE_LEFT_MARGIN    2  /* Treeview's left margin  */
#define ITREE_TREE_TOP_MARGIN     1  /* Treeview's top margin   */
#define ITREE_TREE_BOTTOM_MARGIN  1  /* Treeview's down margin  */
#define ITREE_TREE_RIGHT_MARGIN   5  /* Treeview's right margin */
#define ITREE_TREE_HEIGHT       200  /* Treeview's height */
#define ITREE_TREE_WIDTH        400  /* Treeview's width  */

#define ITREE_TEXT_RIGHT_FIX        1  /* Text right correction */
#define ITREE_TREE_BGCOLOR          0xFFFFFFL  /* Treeview's background color */
#define ITREE_TREE_BGCOLORSTRING    "255 255 255"
#define ITREE_TREE_EXPAND_WIDTH     9  /* Treeview's expand width  */ 
#define ITREE_TREE_EXPAND_HEIGHT    9  /* Treeview's expand height */
#define ITREE_TREE_COLLAPSE_WIDTH   9  /* Treeview's expand width  */ 
#define ITREE_TREE_COLLAPSE_HEIGHT  9  /* Treeview's expand height */

#define ITREE_TREE_COLLAPSE_SIZE  ITREE_TREE_COLLAPSE_WIDTH * ITREE_TREE_COLLAPSE_HEIGHT
#define ITREE_TREE_EXPAND_SIZE    ITREE_TREE_EXPAND_WIDTH   * ITREE_TREE_EXPAND_HEIGHT

void iTreeDrawTree(Ihandle* ih);

int  iTreeDrawGetTop(Ihandle* ih);
int  iTreeDrawGetCanvasTop(Ihandle* ih);
int  iTreeDrawGetLeft(Ihandle* ih);
void iTreeDrawSetVisibility(Ihandle* ih);
  
#ifdef __cplusplus
}
#endif

#endif
