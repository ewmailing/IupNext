/** \file
 * \brief iuptree control
 * Functions used to draw a tree
 *
 * See Copyright Notice in iup.h
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "iup.h"
#include "iupcbs.h"
#include "iuptree.h"
#include "iupkey.h"

#include <cd.h>
#include <cdiup.h>
#include <cddbuf.h>

#include "iup_object.h"
#include "iup_attrib.h"
#include "iup_str.h"
#include "iup_drv.h"
#include "iup_drvfont.h"
#include "iup_stdcontrols.h"
#include "iup_controls.h"
#include "iup_cdutil.h"

#include "iuptree_draw.h"
#include "iuptree_def.h"
#include "iuptree_callback.h"
#include "iuptree_find.h"

#define ITREE_MAX(a, b) ((a) > (b) ? (a) : (b))

/* Images */
extern unsigned char tree_collapse_image[ITREE_TREE_COLLAPSE_SIZE];
extern long int tree_collapse_colors[ITREE_BOX_COLORS];

extern unsigned char tree_expand_image[ITREE_TREE_EXPAND_SIZE];
extern long int tree_expand_colors[ITREE_BOX_COLORS];

extern unsigned char tree_blank_image[ITREE_TREE_EXPAND_SIZE];
extern long int tree_blank_colors[ITREE_BOX_COLORS];


/**************************************************************************
*   Internal functions
***************************************************************************/

static void iTreeDrawVerticalDottedLine(cdCanvas* c, int x, int y1, int y2,int first);
static void iTreeDrawVerticalLine(cdCanvas* c, int x, int y1, int y2);
static void iTreeDrawHorizontalDottedLine(cdCanvas* c, int x1, int x2, int y,int first);
static void iTreeDrawHorizontalLine(cdCanvas* c, int x1, int x2, int y);
static void iTreeDrawDottedRect(cdCanvas* c, int x1, int x2, int y1, int y2);
static void iTreeDrawRectangle(cdCanvas* c, int x1, int x2, int y1, int y2);

/* Draws the image pointed by index
   - x, y: Coordinates of the lower-left part of the image
*/
static void iTreeDrawPutImage(cdCanvas* c, int w, int h, unsigned char* index, long int* colors, int x, int y)
{
  cdCanvasPutImageRectMap(c, w, h, index, colors, x, y, 0, 0, 0, 0, 0, 0); 
}


/**************************************************************************
*   Exported functions
***************************************************************************/

static void iTreeDrawUpdateScrollDx(Ihandle* ih, int maxx)
{
  float dy = iTreeFindNumNodesInCanvas(ih) / (float)(iTreeFindNumNodes(ih) + ITREE_TREE_BOTTOM_MARGIN);
  float dx = (float)(ih->data->XmaxC - ITREE_NODE_X) / (float)maxx;

  if(dy > 1.0)
    dy = 1.0;
  
  if(dx > 1.0)
    dx = 1.0;
 
  IupSetfAttribute(ih, "DY", "%f", (double)dy);
  IupSetfAttribute(ih, "DX", "%f", (double)dx);
} 

static void iTreeDrawBorder(Ihandle* ih)
{
  long color_border;

  cdIupCalcShadows(cdIupConvertColor(iupControlBaseGetParentBgColor(ih)), NULL, &color_border, NULL);
  cdCanvasForeground(ih->data->cddbuffer, color_border);
  cdCanvasRect(ih->data->cddbuffer, 0, ih->data->XmaxC, 0, ih->data->YmaxC);
}

/* Redraws the whole tree */
void iTreeDrawTree(Ihandle* ih)
{
 Node node = (Node)ih->data->root;
 int x, y = iTreeDrawGetTop(ih);
 int first_leaf = 1;
 int maxx = 0;

 iTreeDrawSetVisibility(ih);

 cdCanvasClear(ih->data->cddbuffer);

 while(node)
 { 
   int text_x;
   int text_y;
   int kind;

   text_x = 0;
      
   x = iTreeDrawGetLeft(ih) + ITREE_NODE_X * node->depth;

   if(node->visible == YES)
   {
     y -= ITREE_NODE_Y;
     maxx = ITREE_MAX(maxx, ITREE_NODE_X * node->depth + ITREE_TREE_RIGHT_MARGIN);
   }

   kind = node->kind;

   /* Only draws node if it is visible */
   if((y >= -ITREE_NODE_Y) && (y < ih->data->YmaxC - ITREE_TREE_TOP_MARGIN + ITREE_NODE_Y))
   {
     /* if node has a text associated to it... */
     if(node->name)
     {
       /* Calculates its dimensions */
       cdCanvasGetTextSize(ih->data->cddbuffer, node->name, &text_x, &text_y);
       
       if(node->visible == YES)
       {
         /* offset from the bottom of the box */
         int text_bottom_offset = ITREE_TEXT_MARGIN_Y;
         int text_left_fix = 0, left_offset = 0;

         if(ITREE_NODE_HEIGHT > text_y)
            text_bottom_offset += (ITREE_NODE_HEIGHT - text_y) / 2;

         if(text_y > 12)
           text_bottom_offset += (text_y - 12) / 2;

         if(text_y < 10) /* It means that it is a small font */ 
           text_left_fix = 2;

         if(kind == ITREE_BRANCH)
           left_offset = 3;

         if(node->marked == YES)
         {
           /* Draws the marked nodes text box */     
           cdCanvasForeground(ih->data->cddbuffer, node->text_color);
           cdCanvasBox(ih->data->cddbuffer, x + ITREE_NODE_WIDTH + left_offset,
              x + ITREE_NODE_WIDTH + left_offset + 2 * ITREE_TEXT_MARGIN_X - ITREE_TEXT_RIGHT_FIX + text_x,
              y - ITREE_TEXT_BOX_OFFSET_Y,
              y + ITREE_NODE_HEIGHT);
           cdCanvasForeground(ih->data->cddbuffer, CD_WHITE); 
         }
         else
           cdCanvasForeground(ih->data->cddbuffer, node->text_color);

         cdCanvasText(ih->data->cddbuffer, x + ITREE_NODE_WIDTH + left_offset + ITREE_TEXT_MARGIN_X - text_left_fix,
             y + text_bottom_offset, node->name);
  
         maxx = ITREE_MAX(maxx, ITREE_NODE_X * node->depth + 2 * ITREE_TEXT_MARGIN_X - ITREE_TEXT_RIGHT_FIX + text_x + ITREE_TREE_RIGHT_MARGIN + left_offset);
   
         if(node == ih->data->selected)
         {
            /* Draws the selection rectangle */
            iTreeDrawRectangle(ih->data->cddbuffer, x + ITREE_NODE_WIDTH + left_offset,
                           x + ITREE_NODE_WIDTH + left_offset + 2 * ITREE_TEXT_MARGIN_X - ITREE_TEXT_RIGHT_FIX + text_x,
                            y - ITREE_TEXT_BOX_OFFSET_Y,
                           y + ITREE_NODE_HEIGHT);
         }
       }
     }
   
     /* if node is a branch */
     if(kind == ITREE_BRANCH)
     {
       Node temp = node;
       int l;

       if(node->visible == YES)     
       {
         /* if node is not the root, draws button to collapse/expand */
         if(node != ih->data->root)
         {
           if(!node->next || node->next->depth <= node->depth)
           {
             iTreeDrawPutImage(ih->data->cddbuffer, ITREE_TREE_COLLAPSE_WIDTH,
                      ITREE_TREE_COLLAPSE_HEIGHT,
                      tree_blank_image,
                      tree_blank_colors,
                      x + (ITREE_NODE_WIDTH - ITREE_TREE_COLLAPSE_WIDTH) / 2 + ITREE_BUTTON_X,
                      y + ITREE_BUTTON_LINE_Y - ITREE_TREE_COLLAPSE_HEIGHT / 2 + 1);
           }
           else if(node->state == ITREE_EXPANDED)
           {
             iTreeDrawPutImage(ih->data->cddbuffer, ITREE_TREE_COLLAPSE_WIDTH,
                      ITREE_TREE_COLLAPSE_HEIGHT,
                      tree_collapse_image,
                      tree_collapse_colors,
                      x + (ITREE_NODE_WIDTH - ITREE_TREE_COLLAPSE_WIDTH) / 2 + ITREE_BUTTON_X,
                      y + ITREE_BUTTON_LINE_Y - ITREE_TREE_COLLAPSE_HEIGHT / 2 + 1);
           }
           else
           {
             iTreeDrawPutImage(ih->data->cddbuffer, ITREE_TREE_EXPAND_WIDTH,
                      ITREE_TREE_EXPAND_HEIGHT,
                      tree_expand_image,
                      tree_expand_colors,
                      x + (ITREE_NODE_WIDTH - ITREE_TREE_EXPAND_WIDTH) / 2 + ITREE_BUTTON_X,
                      y + ITREE_BUTTON_LINE_Y - ITREE_TREE_EXPAND_HEIGHT / 2 + 1);      
           }
           
           /* Draws vertical line that links branch with node */
           iTreeDrawVerticalLine(ih->data->cddbuffer, x + ITREE_BUTTON_X + ITREE_NODE_WIDTH / 2 - 1,
             y + ITREE_TREE_COLLAPSE_HEIGHT + 2,
             y + ITREE_NODE_Y-ITREE_BUTTON_BRANCH_Y); 


           /* If the following visible node is a branch, draws the rest of the line */
           {
             int depth = node->depth;
             Node temp = node->next;
       
             while(temp && depth >= node->depth && temp->visible == NO)
             {
               temp = temp->next;
             }

             if(temp && temp->depth == depth && temp->visible == YES)
               iTreeDrawVerticalLine(ih->data->cddbuffer, x + ITREE_BUTTON_X + ITREE_NODE_WIDTH / 2 - 1,
                  y - ITREE_BUTTON_BRANCH_Y + 1,
                  y - ITREE_BUTTON_BRANCH_Y + 3);
           }
       
            /* Draws horizontal line that links node with button */
           iTreeDrawHorizontalLine(ih->data->cddbuffer, x + ITREE_BUTTON_X + (ITREE_NODE_WIDTH + ITREE_TREE_COLLAPSE_WIDTH) / 2,
                          x, y + ITREE_BUTTON_LINE_Y);
   
           for(l = 1; l < node->depth; l++)
           {
             temp = node;
         
             while(temp)
             { 
               {
                 if(temp->depth + l + 1 == node->depth && temp->kind == ITREE_BRANCH)
                   break;

                 if(temp->depth + l == node->depth)
                 {
                   /* Draws vertical lines on the left of the branch */   
                   iTreeDrawVerticalLine(ih->data->cddbuffer, x + ITREE_BRANCH_LINE_X - ITREE_NODE_X * l,
                                            y + ITREE_BRANCH_LINE_Y1,
                                            y + ITREE_BRANCH_LINE_Y2);
       
                   break;
                 }
                 temp = temp->next;
               }
             }
           }
         }

         if(node->state == ITREE_COLLAPSED)
         {
           if(node->imageinuse == NO)
           {
             iTreeDrawPutImage(ih->data->cddbuffer, ITREE_NODE_WIDTH,
                ITREE_NODE_HEIGHT, ih->data->image_collapsed,
                node->marked == NO ? (long*)ih->data->color_collapsed : (long*)ih->data->marked_color_collapsed,
                x, y);
           }
           else
           {
             iTreeDrawPutImage(ih->data->cddbuffer, ITREE_NODE_WIDTH,
                 ITREE_NODE_HEIGHT, node->image,
                 node->marked == NO ? (long*)node->color : (long*)node->marked_color,
                 x, y);
           }
         }
         else
         {
           if(node->expandedimageinuse == NO)
           {
             iTreeDrawPutImage(ih->data->cddbuffer, ITREE_NODE_WIDTH, 
                 ITREE_NODE_HEIGHT, ih->data->image_expanded, 
                 node->marked == NO ? (long*)ih->data->color_expanded : (long*)ih->data->marked_color_expanded,
                 x, y);
           }
           else
           {
              iTreeDrawPutImage(ih->data->cddbuffer, ITREE_NODE_WIDTH, 
                 ITREE_NODE_HEIGHT, node->expandedimage, 
                 node->marked == NO ? (long*)node->expandedcolor : (long*)node->expandedmarked_color, 
                 x, y);
           }
         }
       }

       first_leaf = 1;
     }
     else /* node is a leaf */
     {
       Node temp = node;
       int l;
     
       if(node->visible == YES)
       {
         for(l = 0; l < node->depth; l++)
         {
           temp = node;
     
           while(temp)
           {
             if(temp->depth + l + 1 == node->depth && temp->kind == ITREE_BRANCH)
               break;

             if(temp->depth + l == node->depth)
             {
               /* Draws vertical lines on the left depth and below it */
               iTreeDrawVerticalLine(ih->data->cddbuffer, x + ITREE_LEAF_LINE_X1 - ITREE_NODE_X * l,
                                        y + ITREE_LEAF_LINE_Y1 + (l != 0) * ITREE_LAST_LEAF_Y,
                                        y + ITREE_LEAF_LINE_Y2 + first_leaf * ITREE_FIRST_LEAF_Y);
               if(l != 0)
               {
                 iTreeDrawVerticalLine(ih->data->cddbuffer, x + ITREE_LEAF_LINE_X1 - ITREE_NODE_X * l,
                                          y + ITREE_LEAF_LINE_Y2 + first_leaf * ITREE_FIRST_LEAF_Y + 1,
                                          y + ITREE_LEAF_LINE_Y2 + first_leaf * ITREE_FIRST_LEAF_Y + 3);
                   
               }
               break;
             }
             temp = temp->next;
           }
         }
 
         if(node->next && node->next->kind == ITREE_BRANCH && node->next->depth >= node->depth)
         { 
           /* Draws vertical line under the leaf */
           iTreeDrawVerticalLine(ih->data->cddbuffer, x + ITREE_LEAF_LINE_X1,
                                    y + ITREE_LEAF_LINE_Y1 + ITREE_LAST_LEAF_Y,
                                    y + ITREE_LEAF_LINE_Y1);
         } 

         /* Draws horizontal line to link the leaf to the vertical line */
         iTreeDrawHorizontalLine(ih->data->cddbuffer, x + ITREE_LEAF_LINE_X1,
                                    x + ITREE_LEAF_LINE_X2,
                                    y + ITREE_LEAF_LINE_Y1);

         if(node->imageinuse == NO)
         {
           iTreeDrawPutImage(ih->data->cddbuffer, ITREE_NODE_WIDTH,
                    ITREE_NODE_HEIGHT, ih->data->image_leaf,
                    node->marked == NO ? (long*)ih->data->color_leaf : (long*)ih->data->marked_color_leaf,
                    x, y);
         }
         else
         {
           iTreeDrawPutImage(ih->data->cddbuffer, ITREE_NODE_WIDTH,
                    ITREE_NODE_HEIGHT, node->image,
                    node->marked == NO ? (long*)node->color : (long*)node->marked_color,
                    x, y);
         }
         first_leaf = 0;
       }
     }
   }
   
   if(node == ih->data->selected)
   {
     ih->data->selected_y = y;

     if(node->name == NULL)
     {
       /* Draws the selection rectangle around the node */
       iTreeDrawRectangle(ih->data->cddbuffer, x - 1, x + ITREE_NODE_WIDTH,
                                                   y - 1, y + ITREE_NODE_HEIGHT);         
     }
   }
   
   node = node->next;
 }
 
 ih->data->maxx = maxx;

 iTreeDrawBorder(ih);
 
 iTreeDrawUpdateScrollDx(ih, maxx);
}

int iTreeDrawGetTop(Ihandle* ih)
{
  float posy = IupGetFloat(ih, "POSY");     
  float   dy = IupGetFloat(ih, "DY");
  
  return (int)((1.0 + posy / dy) * (ih->data->YmaxC - ITREE_TREE_TOP_MARGIN));
}

int iTreeDrawGetCanvasTop(Ihandle* ih)
{
  float posy = IupGetFloat(ih, "POSY");
  float   dy = IupGetFloat(ih, "DY");
  
  return (int)((posy / dy) * (ih->data->YmaxC - ITREE_TREE_TOP_MARGIN));
}

int iTreeDrawGetLeft(Ihandle* ih)
{
  float posx = IupGetFloat(ih, "POSX");
  float   dx = IupGetFloat(ih, "DX");

  return (int)(ITREE_TREE_LEFT_MARGIN - (ih->data->XmaxC - ITREE_NODE_X) * posx / dx);
}

/* first determines if the first pixel is selection or background */
static void iTreeDrawVerticalDottedLine(cdCanvas* c, int x, int y1, int y2, int first)
{
  int i;
   
  for(i = y1; i < y2; i += 2)
  {
    cdCanvasPixel(c, x, i + first, CD_DARK_GRAY);     
    cdCanvasPixel(c, x, i + (1 - first), ITREE_TREE_BGCOLOR);
  }
  
  if(i == y2 && first == 1)
    cdCanvasPixel(c, x, y2, ITREE_TREE_BGCOLOR);
}

static void iTreeDrawVerticalLine(cdCanvas* c, int x, int y1, int y2)
{
  iTreeDrawRectangle(c, x, x, y1, y2);
}

/* first determines if the first pixel is selection or background */
static void iTreeDrawHorizontalDottedLine(cdCanvas* c, int x1, int x2, int y, int first)
{
  int i;

  for(i = x1; i< x2; i +=2)
  {
    cdCanvasPixel(c, i + first, y, CD_DARK_GRAY);
    cdCanvasPixel(c, i + (1 - first), y, ITREE_TREE_BGCOLOR);
  }

  if(i == x2 && first == 1)
    cdCanvasPixel(c, x2, y, ITREE_TREE_BGCOLOR);
}

static void iTreeDrawHorizontalLine(cdCanvas* c, int x1, int x2, int y)
{
  iTreeDrawRectangle(c, x1, x2, y, y);
}

static void iTreeDrawDottedRect(cdCanvas* c, int x1, int x2, int y1, int y2)
{
  iTreeDrawHorizontalDottedLine(c, x1, x2, y1, 0);
  iTreeDrawVerticalDottedLine(c, x2, y1, y2, x1 != x2 && (float)(y2 - y1) / 2.0 != 0);
  iTreeDrawHorizontalDottedLine(c, x1, x2, y2, y1 != y2 && (float)(x2 - x1) / 2.0 != 0);
  iTreeDrawVerticalDottedLine(c, x1, y1, y2, 0);
}

static void iTreeDrawRectangle(cdCanvas* c, int x1, int x2, int y1, int y2)
{
  iTreeDrawDottedRect(c, x1, x2, y1, y2);
}

void iTreeDrawSetVisibility(Ihandle* ih)
{
  Node node = (Node)ih->data->root;
  int hide = 0;
  int visible = YES;
 
  while(node)
  {
    if(node->depth <= hide)
    {
     hide = 0;
     visible = YES;
    }

    if(!hide && visible == YES && node->state == ITREE_COLLAPSED)
    {
     hide = node->depth;
    }

    node->visible = visible;

    if(ih->data->root->state == ITREE_COLLAPSED || hide)
      visible = NO;

    node = node->next;
  }
}
