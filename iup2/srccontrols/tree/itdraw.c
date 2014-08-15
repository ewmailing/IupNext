/** \file
 * \brief iuptree control
 * Functions used to draw a tree
 *
 * See Copyright Notice in iup.h
 *  */

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <iup.h>
#include <iupcpi.h>
#include <iupcompat.h>
#include <cd.h>

#include "iuptree.h"
#include "treedef.h"
#include "itdraw.h"
#include "itfind.h"
#include "iupcdutil.h"

#define MAX(a,b) ((a)>(b)?(a):(b))

/* Images */
extern unsigned char tree_collapse_image[TREE_COLLAPSE_SIZE];
extern long int tree_collapse_colors[BOX_COLORS];

extern unsigned char tree_expand_image[TREE_EXPAND_SIZE];
extern long int tree_expand_colors[BOX_COLORS];

extern unsigned char tree_blank_image[TREE_EXPAND_SIZE];
extern long int tree_blank_colors[BOX_COLORS];

/**************************************************************************
***************************************************************************
*
*   Internal functions
*
***************************************************************************
***************************************************************************/

static void treedrawDrawVerticalDottedLine(cdCanvas *c, int x, int y1, int y2,int first);
static void treedrawDrawVerticalLine(cdCanvas *c, int x, int y1, int y2);
static void treedrawDrawHorizontalDottedLine(cdCanvas *c, int x1, int x2, int y,int first);
static void treedrawDrawHorizontalLine(cdCanvas *c, int x1, int x2, int y);
static void treedrawDrawDottedRect(cdCanvas *c, int x1, int x2, int y1, int y2);
static void treedrawDrawRectangle(cdCanvas *c, int x1, int x2, int y1, int y2);

/*
%F Draws the image pointed by index

%i h - tree's handle
   x, y - Coordinates of the lower-left part of the image
*/
static void treePutImage(cdCanvas *c, int w, int h, unsigned char* index, long int *colors, int x, int y)
{
  cdCanvasPutImageRectMap(c,w,h,index,colors,x,y,0,0,0,0,0,0); 
}
/**************************************************************************
***************************************************************************
*
*   Exported functions
*
***************************************************************************
***************************************************************************/

static void updateScrollDx(Ihandle *h, int maxx)
{
  TtreePtr tree=(TtreePtr)tree_data(h);
  char dx_buffer[10];
  char dy_buffer[10];
  float dy = treefindNumNodesInCanvas(h)/(float)(treefindNumNodes(h)+TREE_BOTTOM_MARGIN);
  float dx = (float)(XmaxCanvas(tree) - NODE_X) / (float)maxx ;

  if(dy>1.0)dy = 1.0;
  if(dx>1.0)dx = 1.0;
 
  sprintf(dy_buffer,"%f",dy);
  sprintf(dx_buffer,"%f",dx);
  IupStoreAttribute(tree_self(tree),IUP_DY,dy_buffer);
  IupStoreAttribute(tree_self(tree),IUP_DX,dx_buffer);
} 

static void treeDrawBorder(TtreePtr tree)
{
  long color_border;
  cdIupCalcShadows(cdIupConvertColor(iupGetBgColor(tree->self)), NULL, &color_border, NULL);
  cdCanvasForeground(tree->cddbuffer,color_border);
  cdCanvasRect(tree->cddbuffer,0, XmaxCanvas(tree), 0, YmaxCanvas(tree));
}

/*
%F Redraws the whole tree
%i h : tree's handle
*/
void treedrawDrawTree(Ihandle *h)
{
 TtreePtr tree=(TtreePtr)tree_data(h);
 Node node=(Node)tree_root(tree);
 int x,y=treedrawGetTop(h);
 int first_leaf = 1 ;
 int maxx = 0;

 treedrawSetVisibility(h);

 cdCanvasClear(tree->cddbuffer);

 while(node)
 { 
   int text_x ;
   int text_y ;
   int kind ;

   text_x = 0 ;
   	 
   x = treedrawGetLeft(h) + NODE_X * node_depth(node) ;

   if( node_visible(node) == YES )
   {
     y-= NODE_Y ;
     maxx = MAX(maxx, NODE_X * node_depth(node) + TREE_RIGHT_MARGIN);
   }

   kind = node_kind(node);

   /* Only draws node if it is visible */
   if((y >= -NODE_Y)&&(y < YmaxCanvas(tree) - TREE_TOP_MARGIN + NODE_Y))
   {
     /* if node has a text associated to it... */
     if(node_name(node))
     {
       /* Calculates its dimensions */
       iupdrvStringSize(tree->self,node_name(node), &text_x, &text_y);
       
       if(node_visible(node) == YES)
       {
         /* offset from the bottom of the box */
         int text_bottom_offset = TEXT_MARGIN_Y ;
         int text_left_fix = 0, left_offset = 0;

         if(NODE_HEIGHT > text_y)
   	       text_bottom_offset += (NODE_HEIGHT - text_y) / 2;

	       if(text_y > 12)
           text_bottom_offset += (text_y - 12) / 2;

         if(text_y < 10) /* It means that it is a small font */ 
           text_left_fix = 2;

         if(kind == BRANCH)
           left_offset = 3;

	       if(node_marked(node) == YES)
         {
           /* Draws the marked nodes text box */	   
           cdCanvasForeground(tree->cddbuffer,node_text_color(node));
	         cdCanvasBox(tree->cddbuffer,x + NODE_WIDTH+left_offset,
	            x + NODE_WIDTH+left_offset + 2*TEXT_MARGIN_X-TEXT_RIGHT_FIX + text_x,
		          y - TEXT_BOX_OFFSET_Y,
		          y + NODE_HEIGHT);
           cdCanvasForeground(tree->cddbuffer,CD_WHITE); 
         }
         else
           cdCanvasForeground(tree->cddbuffer,node_text_color(node));

         cdCanvasText(tree->cddbuffer,x + NODE_WIDTH+left_offset + TEXT_MARGIN_X - text_left_fix, y + text_bottom_offset, node_name(node));
	
         maxx = MAX(maxx, NODE_X * node_depth(node)+ 2*TEXT_MARGIN_X - TEXT_RIGHT_FIX + text_x + TREE_RIGHT_MARGIN+left_offset);
	 
         if(node == tree_selected(tree))
         {
            /* Draws the selection rectangle */
	          treedrawDrawRectangle(tree->cddbuffer,x + NODE_WIDTH+left_offset,
	                         x + NODE_WIDTH+left_offset + 2*TEXT_MARGIN_X-TEXT_RIGHT_FIX + text_x,
                       		 y - TEXT_BOX_OFFSET_Y,
                  		     y + NODE_HEIGHT);
         }
       }
     }
   
     /* if node is a branch */
     if(kind == BRANCH)
     {
       Node temp = node;
       int l ;

       if(node_visible(node) == YES)     
       {
         /* if node is not the root, draws button to collapse/expand */
         if( node!=tree_root(tree) )
         {
           if(!node_next(node) || node_depth(node_next(node)) <= node_depth(node))
           {
             treePutImage(tree->cddbuffer, TREE_COLLAPSE_WIDTH,
                      TREE_COLLAPSE_HEIGHT,
                      tree_blank_image,
                      tree_blank_colors,
                      x+(NODE_WIDTH-TREE_COLLAPSE_WIDTH)/2+BUTTON_X,
  	                  y+BUTTON_LINE_Y - TREE_COLLAPSE_HEIGHT/2+1 ) ;
           }
           else if(node_state(node) == EXPANDED)
           {
             treePutImage(tree->cddbuffer, TREE_COLLAPSE_WIDTH,
                      TREE_COLLAPSE_HEIGHT,
                      tree_collapse_image,
                      tree_collapse_colors,
                      x+(NODE_WIDTH-TREE_COLLAPSE_WIDTH)/2+BUTTON_X,
  	                  y+BUTTON_LINE_Y - TREE_COLLAPSE_HEIGHT/2+1 ) ;
           }
           else
           {
	           treePutImage(tree->cddbuffer, TREE_EXPAND_WIDTH,
                      TREE_EXPAND_HEIGHT,
                      tree_expand_image,
                      tree_expand_colors,
                      x+(NODE_WIDTH-TREE_EXPAND_WIDTH)/2+BUTTON_X,
                      y+BUTTON_LINE_Y - TREE_EXPAND_HEIGHT/2+1 ) ;			
           }
	 
	          /* Draws vertical line that links branch with node */
	          treedrawDrawVerticalLine(tree->cddbuffer,x+BUTTON_X+NODE_WIDTH/2-1,
	            y+TREE_COLLAPSE_HEIGHT+2,
	            y+NODE_Y-BUTTON_BRANCH_Y); 


            /* If the following visible node is a branch, draws the rest of the line */
            {
              int depth = node_depth(node);
              Node temp = node_next(node);
	     
              while(temp && depth >= node_depth(node) && node_visible(temp) == NO)
              {
    	          temp = node_next(temp);
              }

              if(temp && node_depth(temp) == depth && node_visible(temp) == YES)
	              treedrawDrawVerticalLine(tree->cddbuffer,x+BUTTON_X+NODE_WIDTH/2-1,
              y-BUTTON_BRANCH_Y+1, 
              y-BUTTON_BRANCH_Y+3);   
            }
       
	          /* Draws horizontal line that links node with button */
            treedrawDrawHorizontalLine(tree->cddbuffer, x+BUTTON_X+(NODE_WIDTH+TREE_COLLAPSE_WIDTH)/2,
                           x, y+BUTTON_LINE_Y);
	 
            for(l=1;l<node_depth(node);l++)
            {
              temp = node ;
	       
              while(temp)
              { 
                {
	                if(node_depth(temp)+l+1==node_depth(node)&&node_kind(temp)==BRANCH)
                    break;	 
                  if(node_depth(temp)+l==node_depth(node))
                  {
		                /* Draws vertical lines on the left of the branch */	 
                    treedrawDrawVerticalLine(tree->cddbuffer,x+BRANCH_LINE_X-NODE_X*l,
                                             y+BRANCH_LINE_Y1,
                                             y+BRANCH_LINE_Y2);
		   
	                  break;
                  }
                  temp = node_next(temp);
                }
              }
            }
         }

	       if(node_state(node) == COLLAPSED)
	       {
	         if(node_imageinuse(node) == NO)
	         {	   
                   treePutImage(tree->cddbuffer, NODE_WIDTH,
	                    NODE_HEIGHT,
		            tree_image_collapsed(tree),
		            node_marked(node) == NO ?
		              (long*)tree_color_collapsed(tree) : 
		              (long*)tree_marked_color_collapsed(tree),
		            x,
		            y ) ;
	         }
	         else
           {
	           treePutImage(tree->cddbuffer, NODE_WIDTH,
	                    NODE_HEIGHT,
		                  node_image(node),
		                  node_marked(node) == NO ?
		                  (long*)node_color(node) : 
		                  (long*)node_marked_color(node),
		                  x,
		                  y ) ;
           }
         }
	       else
         {
	         if(node_expandedimageinuse(node) == NO)
           {
             treePutImage(tree->cddbuffer, NODE_WIDTH,
    	                NODE_HEIGHT,
		                  tree_image_expanded(tree),
		                  node_marked(node) == NO ?
		                  (long*)tree_color_expanded(tree) :
		                  (long*)tree_marked_color_expanded(tree),
		                  x,
		                  y ) ;
           }
	         else
           {
	            treePutImage(tree->cddbuffer, NODE_WIDTH,
	                     NODE_HEIGHT,
                       node_expandedimage(node),
	                     node_marked(node) == NO ?
	                     (long*)node_expandedcolor(node) : 
	                     (long*)node_expandedmarked_color(node),
		                   x,
		                   y ) ;
           }
         }
       }

       first_leaf = 1 ;
     }
     else /* node is a leaf */
     {
       Node temp = node ;
       int l ;
     
       if(node_visible(node) == YES)
       {
         for(l=0;l<node_depth(node);l++)
         {
           temp = node ;
     
           while(temp)
           {
             if(node_depth(temp)+l+1==node_depth(node)&&node_kind(temp)==BRANCH)
               break;	 
             if(node_depth(temp)+l==node_depth(node))
             {
              /* Draws vertical lines on the left depth and below it */	     
               treedrawDrawVerticalLine(tree->cddbuffer,x+LEAF_LINE_X1-NODE_X*l,
                                        y+LEAF_LINE_Y1+(l!=0)*LAST_LEAF_Y,
                                        y+LEAF_LINE_Y2+first_leaf*FIRST_LEAF_Y);
	             if(l != 0)
	             {
                       treedrawDrawVerticalLine(tree->cddbuffer,x+LEAF_LINE_X1-NODE_X*l,
                                          y+LEAF_LINE_Y2+first_leaf*FIRST_LEAF_Y+1,
                                          y+LEAF_LINE_Y2+first_leaf*FIRST_LEAF_Y+3);
			             
	             }
               break;
             }
             temp = node_next(temp);
           }
         }
 
         if(node_next(node) && node_kind(node_next(node))==BRANCH && node_depth(node_next(node)) >= node_depth(node) )
         { 
           /* Draws vertical line under the leaf */		 
           treedrawDrawVerticalLine(tree->cddbuffer,x+LEAF_LINE_X1,
                                    y+LEAF_LINE_Y1+LAST_LEAF_Y,
                                    y+LEAF_LINE_Y1);
         } 

	       /* Draws horizontal line to link the leaf to the vertical line */
         treedrawDrawHorizontalLine(tree->cddbuffer, x+LEAF_LINE_X1,
                                    x+LEAF_LINE_X2,
                                    y+LEAF_LINE_Y1);

	       if(node_imageinuse(node) == NO)
	       {
           treePutImage(tree->cddbuffer, NODE_WIDTH,
                    NODE_HEIGHT,
	  	              tree_image_leaf(tree),
  	                node_marked(node) == NO ? 
		                (long*)tree_color_leaf(tree) :
		                (long*)tree_marked_color_leaf(tree),
                    x,
                    y ) ;
	       }
         else
         {
	         treePutImage(tree->cddbuffer, NODE_WIDTH,
	                  NODE_HEIGHT,
		                node_image(node),
		                node_marked(node) == NO ?
		                (long*)node_color(node) : 
		                (long*)node_marked_color(node),
		                x,
		                y ) ;
         }
         first_leaf = 0 ;
       }
     }
   }
   
   if(node == tree_selected(tree))
   {
     tree_selected_y(tree) = y ;

     if(node_name(node) == NULL)
     {
       /* Draws the selection rectangle around the node */
       treedrawDrawRectangle(tree->cddbuffer,x-1,
                             x+NODE_WIDTH,
                             y-1,
                             y+NODE_HEIGHT);			   
     }
   }
   
   node = node_next(node);
 }
 
 tree_maxx(tree) = maxx;

 treeDrawBorder(tree);
 
 updateScrollDx(h, maxx);
}

int treedrawGetTop(Ihandle *h)
{
  TtreePtr tree=(TtreePtr)tree_data(h);
  float posy = IupGetFloat(tree_self(tree), IUP_POSY);     
  float dy = IupGetFloat(tree_self(tree), IUP_DY);
	
  return (int)((1.0 + posy/dy)*(YmaxCanvas(tree)-TREE_TOP_MARGIN));
}

int treedrawGetCanvasTop(Ihandle *h)
{
  TtreePtr tree=(TtreePtr)tree_data(h);
  float posy = IupGetFloat(tree_self(tree), IUP_POSY);     
  float dy = IupGetFloat(tree_self(tree), IUP_DY);
	
  return (int)((posy/dy)*(YmaxCanvas(tree)-TREE_TOP_MARGIN));
}

int treedrawGetLeft(Ihandle *h)
{
  TtreePtr tree=(TtreePtr)tree_data(h);
  float posx = IupGetFloat(tree_self(tree), IUP_POSX);
  float dx = IupGetFloat(tree_self(tree), IUP_DX);

  return (int)(TREE_LEFT_MARGIN - (XmaxCanvas(tree)-NODE_X)*posx/dx);
}

/* first determines if the first pixel is selection or background */
static void treedrawDrawVerticalDottedLine(cdCanvas *c, int x, int y1, int y2, int first)
{
  int i ;
 	
  for(i = y1; i< y2; i +=2)
  {
    cdCanvasPixel(c,x,i+first,CD_DARK_GRAY); 	  
    cdCanvasPixel(c,x,i+(1-first),TREE_BGCOLOR);
  }
  if(i == y2 && first == 1)cdCanvasPixel(c,x,y2,TREE_BGCOLOR);
}

static void treedrawDrawVerticalLine(cdCanvas *c, int x, int y1, int y2)
{
  treedrawDrawRectangle(c,x,x,y1,y2);
}

/* first determines if the first pixel is selection or background */
static void treedrawDrawHorizontalDottedLine(cdCanvas *c, int x1, int x2, int y, int first)
{
  int i ;

  for(i = x1; i< x2 ; i +=2)
  {
    cdCanvasPixel(c,i+first,y,CD_DARK_GRAY);
    cdCanvasPixel(c,i+(1-first),y,TREE_BGCOLOR);
  }
  if(i == x2 && first == 1) cdCanvasPixel(c,x2,y,TREE_BGCOLOR);
}

static void treedrawDrawHorizontalLine(cdCanvas *c, int x1, int x2, int y)
{
  treedrawDrawRectangle(c,x1,x2,y,y);
}

static void treedrawDrawDottedRect(cdCanvas *c, int x1, int x2, int y1, int y2)
{
  treedrawDrawHorizontalDottedLine(c,x1,x2,y1,0);
  treedrawDrawVerticalDottedLine(c,x2,y1,y2,x1!=x2 && (float)(y2-y1)/2.0 != 0);
  treedrawDrawHorizontalDottedLine(c,x1,x2,y2,y1!=y2 && (float)(x2-x1)/2.0 != 0);
  treedrawDrawVerticalDottedLine(c,x1,y1,y2,0);
}

static void treedrawDrawRectangle(cdCanvas *c, int x1, int x2, int y1, int y2)
{
  treedrawDrawDottedRect(c,x1,x2,y1,y2);
}

void treedrawSetVisibility(Ihandle *h)
{
  TtreePtr tree=(TtreePtr)tree_data(h);
  Node node=(Node)tree_root(tree);
  int hide = 0 ;
  int visible = YES ;
 
  while(node)
  {
    if(node_depth(node) <= hide)
    {
     hide = 0 ;
     visible = YES ;
    }

    if(!hide && visible == YES && node_state(node) == COLLAPSED)
    {
     hide = node_depth(node);
    }

    node_visible(node) = visible ;

    if(node_state(tree_root(tree)) == COLLAPSED || hide)
      visible = NO ;

    node = node_next(node);
  }
}
