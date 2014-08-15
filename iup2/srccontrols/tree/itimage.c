/** \file
 * \brief iuptree control
 * Stores all the pre-defined images
 *
 * See Copyright Notice in iup.h
 *  */

#include <iup.h>

#include "iuptree.h"
#include "itdraw.h"
#include "itimage.h"

unsigned char tree_collapse_image[] = 
{
   0,  0,  1,  0,  1,  0,  1,  0,  0,
   1,  2,  2,  2,  2,  2,  2,  2,  1,
   0,  2,  2,  2,  2,  2,  2,  2,  0,
   1,  2,  2,  2,  2,  2,  2,  2,  1,
   0,  2,  3,  3,  3,  3,  3,  2,  0,
   1,  2,  2,  2,  2,  2,  2,  2,  1,
   0,  2,  2,  2,  2,  2,  2,  2,  0,
   1,  2,  2,  2,  2,  2,  2,  2,  1,
   0,  1,  0,  1,  0,  1,  0,  1,  0 
} ;

long int tree_collapse_colors[4] =
{
  0x888888L,
  0x777777L,
  TREE_BGCOLOR,
  0x000000L
} ;

char tree_expand_image[] = 
{
   0,  0,  1,  0,  1,  0,  1,  0,  0,
   1,  2,  2,  2,  2,  2,  2,  2,  1,
   0,  2,  2,  2,  3,  2,  2,  2,  0,
   1,  2,  2,  2,  3,  2,  2,  2,  1,
   0,  2,  3,  3,  3,  3,  3,  2,  0,
   1,  2,  2,  2,  3,  2,  2,  2,  1,
   0,  2,  2,  2,  3,  2,  2,  2,  0,
   1,  2,  2,  2,  2,  2,  2,  2,  1,
   0,  1,  0,  1,  0,  1,  0,  1,  0 
} ;

long int tree_expand_colors[4] =
{
  0x888888L,
  0x777777L,
  TREE_BGCOLOR,
  0x000000L
} ;

unsigned char tree_blank_image[] =
{
   0,  0,  1,  0,  1,  0,  1,  0,  0,
   1,  2,  2,  2,  2,  2,  2,  2,  1,
   0,  2,  2,  2,  2,  2,  2,  2,  0,
   1,  2,  2,  2,  2,  2,  2,  2,  1,
   0,  2,  2,  2,  2,  2,  2,  2,  0,
   1,  2,  2,  2,  2,  2,  2,  2,  1,
   0,  2,  2,  2,  2,  2,  2,  2,  0,
   1,  2,  2,  2,  2,  2,  2,  2,  1,
   0,  1,  0,  1,  0,  1,  0,  1,  0 
}; 

long int tree_blank_colors[4] =
{
  0x888888L,
  0x777777L,
  TREE_BGCOLOR,
  0x000000L
} ;

void treeimageInitializeImages(void)
{
  Ihandle *image_leaf;	
  Ihandle *image_collapsed;	
  Ihandle *image_expanded;	
  Ihandle *image_blank;	
  Ihandle *image_paper;	
  
  static unsigned char img_leaf[] = 
  {
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 4, 4, 5, 5, 5, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 4, 5, 5, 6, 7, 6, 5, 1, 1, 1, 1, 1,
    1, 1, 1, 3, 4, 4, 5, 5, 6, 7, 6, 5, 1, 1, 1, 1,
    1, 1, 1, 3, 4, 4, 4, 5, 5, 6, 6, 5, 1, 1, 1, 1,
    1, 1, 1, 2, 3, 4, 4, 4, 5, 5, 5, 4, 1, 1, 1, 1,
    1, 1, 1, 2, 3, 3, 4, 4, 4, 5, 4, 4, 1, 1, 1, 1,
    1, 1, 1, 1, 2, 3, 3, 4, 4, 4, 4, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 2, 2, 3, 3, 3, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
  } ;
  
  static unsigned char img_collapsed[] =
  {
     1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  
     1, 1, 2, 2, 2, 2, 2, 3, 1, 1, 1, 1, 1, 1, 1, 1,  
     1, 2, 8, 5, 5, 7, 7, 2, 3, 1, 1, 1, 1, 1, 1, 1, 
     2, 4, 4, 4, 4, 4, 4, 4, 2, 2, 2, 2, 2, 2, 1, 1, 
     2, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 7, 4, 3, 1, 
     2, 5, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 6, 4, 3, 1, 
     2, 5, 7, 7, 7, 7, 7, 7, 7, 6, 7, 6, 7, 4, 3, 1, 
     2, 5, 7, 7, 7, 7, 7, 7, 7, 7, 6, 7, 6, 4, 3, 1, 
     2, 5, 7, 7, 7, 7, 7, 6, 7, 6, 7, 6, 7, 4, 3, 1, 
     2, 5, 7, 7, 7, 7, 7, 7, 6, 7, 6, 7, 6, 4, 3, 1, 
     2, 5, 7, 7, 7, 6, 7, 6, 7, 6, 7, 6, 6, 4, 3, 1, 
     2, 5, 6, 7, 6, 7, 6, 7, 6, 7, 6, 6, 6, 4, 3, 1, 
     2, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3, 1,  
     1, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 1, 
     1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  
     1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1  
  };

  static unsigned char img_expanded[] =
  {
   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
   1, 1, 1, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 
   1, 1, 2, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 
   1, 2, 1, 3, 3, 3, 3, 3, 1, 2, 2, 2, 2, 2, 2, 1, 
   1, 2, 1, 3, 3, 3, 3, 3, 3, 3, 1, 1, 1, 1, 6, 4, 
   1, 2, 1, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 6, 4, 
   2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 6, 3, 6, 4, 
   2, 1, 3, 3, 3, 3, 3, 3, 5, 3, 5, 6, 4, 6, 6, 4, 
   2, 1, 3, 3, 3, 3, 5, 3, 3, 5, 3, 6, 4, 6, 6, 4, 
   1, 2, 1, 3, 3, 3, 3, 3, 5, 3, 5, 5, 2, 4, 2, 4, 
   1, 2, 1, 3, 3, 5, 3, 5, 3, 5, 5, 5, 6, 4, 2, 4, 
   1, 1, 2, 1, 5, 3, 5, 3, 5, 5, 5, 5, 6, 2, 4, 4, 
   1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 4, 4, 
   1, 1, 1, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 
   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 
} ;

  static unsigned char img_blank[] =
  {
   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
   1, 1, 4, 3, 4, 3, 4, 3, 4, 3, 4, 3, 1, 1, 1, 1,
   1, 1, 3, 6, 6, 6, 6, 6, 6, 6, 6, 5, 4, 1, 1, 1,
   1, 1, 4, 6, 6, 6, 6, 6, 6, 6, 6, 5, 6, 4, 1, 1,
   1, 1, 3, 6, 6, 6, 6, 6, 6, 6, 6, 2, 2, 2, 2, 1,
   1, 1, 4, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 5, 2, 1,
   1, 1, 3, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 5, 2, 1,
   1, 1, 4, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 5, 2, 1,
   1, 1, 3, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 5, 2, 1,
   1, 1, 4, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 5, 2, 1,
   1, 1, 3, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 5, 2, 1,
   1, 1, 4, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 5, 2, 1,
   1, 1, 3, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 5, 2, 1,
   1, 1, 4, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 5, 2, 1,
   1, 1, 3, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 2, 1,
   1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1
} ;

  static unsigned char img_paper[] =
  {
   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
   1, 1, 4, 3, 4, 3, 4, 3, 4, 3, 4, 3, 1, 1, 1, 1,
   1, 1, 3, 6, 6, 6, 6, 6, 6, 6, 6, 5, 4, 1, 1, 1,
   1, 1, 4, 6, 6, 6, 6, 6, 6, 6, 6, 5, 6, 4, 1, 1,
   1, 1, 3, 6, 4, 3, 4, 3, 4, 3, 4, 2, 2, 2, 2, 1,
   1, 1, 4, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 5, 2, 1,
   1, 1, 3, 6, 3, 4, 3, 4, 3, 4, 3, 4, 6, 5, 2, 1,
   1, 1, 4, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 5, 2, 1,
   1, 1, 3, 6, 4, 3, 4, 3, 4, 3, 4, 3, 6, 5, 2, 1,
   1, 1, 4, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 5, 2, 1,
   1, 1, 3, 6, 3, 4, 3, 4, 3, 4, 3, 4, 6, 5, 2, 1,
   1, 1, 4, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 5, 2, 1,
   1, 1, 3, 6, 4, 3, 4, 3, 4, 3, 4, 3, 6, 5, 2, 1,
   1, 1, 4, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 5, 2, 1,
   1, 1, 3, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 2, 1,
   1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1
} ;

  image_leaf = IupImage(NODE_WIDTH, NODE_HEIGHT, img_leaf);
  image_collapsed = IupImage(NODE_WIDTH, NODE_HEIGHT, img_collapsed);
  image_expanded = IupImage(NODE_WIDTH, NODE_HEIGHT, img_expanded);
  image_blank = IupImage(NODE_WIDTH, NODE_HEIGHT, img_blank);
  image_paper = IupImage(NODE_WIDTH, NODE_HEIGHT, img_paper);
  
  IupSetAttribute(image_leaf, "1", TREE_BGCOLORSTRING) ;
  IupSetAttribute(image_leaf, "2", "56 56 56");
  IupSetAttribute(image_leaf, "3", "99 99 99");
  IupSetAttribute(image_leaf, "4", "128 128 128");
  IupSetAttribute(image_leaf, "5", "161 161 161");
  IupSetAttribute(image_leaf, "6", "192 192 192");
  IupSetAttribute(image_leaf, "7", "222 222 222");

  IupSetAttribute(image_collapsed, "1", TREE_BGCOLORSTRING) ;
  IupSetAttribute(image_collapsed, "2", "156 156 0");
  IupSetAttribute(image_collapsed, "3", "0 0 0");
  IupSetAttribute(image_collapsed, "4", "206 206 99");
  IupSetAttribute(image_collapsed, "5", "255 255 206");
  IupSetAttribute(image_collapsed, "6", "255 206 156");
  IupSetAttribute(image_collapsed, "7", "255 255 156");
  IupSetAttribute(image_collapsed, "8", "247 247 247");
  
  IupSetAttribute(image_expanded, "1", TREE_BGCOLORSTRING) ;
  IupSetAttribute(image_expanded, "2", "156 156 0");
  IupSetAttribute(image_expanded, "3", "255 255 156");
  IupSetAttribute(image_expanded, "4", "0 0 0");
  IupSetAttribute(image_expanded, "5", "255 206 156");
  IupSetAttribute(image_expanded, "6", "206 206 99");
  
  IupSetAttribute(image_blank, "1", TREE_BGCOLORSTRING) ;
  IupSetAttribute(image_blank, "2", "000 000 000");
  IupSetAttribute(image_blank, "3", "119 119 119");
  IupSetAttribute(image_blank, "4", "136 136 136");
  IupSetAttribute(image_blank, "5", "187 187 187");
  IupSetAttribute(image_blank, "6", "255 255 255");
  
  IupSetAttribute(image_paper, "1", TREE_BGCOLORSTRING) ;
  IupSetAttribute(image_paper, "2", "000 000 000");
  IupSetAttribute(image_paper, "3", "119 119 119");
  IupSetAttribute(image_paper, "4", "136 136 136");
  IupSetAttribute(image_paper, "5", "187 187 187");
  IupSetAttribute(image_paper, "6", "255 255 255");

  IupSetHandle("IMGLEAF",image_leaf);
  IupSetHandle("IMGCOLLAPSED",image_collapsed);
  IupSetHandle("IMGEXPANDED",image_expanded);
  IupSetHandle("IMGBLANK",image_blank);
  IupSetHandle("IMGPAPER",image_paper);
}
