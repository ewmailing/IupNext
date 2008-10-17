/** \file
 * \brief iuptree control core
 *
 * See Copyright Notice in iup.h
 * $Id: iuptree.c,v 1.1 2008-10-17 06:20:41 scuri Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <limits.h>

#include <iup.h>
#include <iupcpi.h>
#include <iupkey.h>
#include <iupcompat.h>
#include <cd.h>
#include <cddbuf.h>

#include "iuptree.h"
#include "treedef.h"
#include "treecd.h"
#include "itgetset.h"
#include "itdraw.h"
#include "itkey.h"
#include "itmouse.h"
#include "itfind.h"
#include "itedit.h"
#include "itscroll.h"
#include "itcallback.h"
#include "itimage.h"
#include "icontrols.h"
#include "iupcdutil.h"

#include "iglobal.h"

#include <string.h>

/* Tree type definition */
static Ihandle*  treecreate        (Iclass* self);
static void  treedestroy       (Ihandle* self);
static int   treesetnaturalsize(Ihandle* self);
static void  treesetattr       (Ihandle*, char* attr, char* value);
static char* treegetattr       (Ihandle*, char* attr);

/* Variable that stores the control treeKey status */
extern int tree_ctrl;

/* Variable that stores the tree_shift treeKey status */
extern int tree_shift;

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

/*

%F Sets structure that describes the tree (TtreePtr)
%i handle of the tree
%i poniter to structure

*/


static void treesetdata(Ihandle* self,TtreePtr tree)
{
   IupSetAttribute(self,"_IUPTREE_DATA",(char*)tree);
} 

/*

%F Recovers structure that describes the tree
%i handle of the tree
%o poniter to structure
 
*/
TtreePtr treeTreeGetData(Ihandle* self)
{
   return (TtreePtr) IupGetAttribute(self,"_IUPTREE_DATA");
}

/*
 *
 * %F verifies which scrolling bars have been defined
 * %i tree's handle
 * %o sbv: =0 doesn't have vertical scrolling bar
 *         =1 has vertical scrolling bar
 *    sbh: =0 doesn't have horizontal scrolling bar
 *         =1 has horizontal scrolling bar
 *
 */
static void treegetsb(Ihandle* self, int *sbh, int *sbv)
{
  char *sb=IupGetAttribute(self,IUP_SCROLLBAR);
  if (sb == NULL) sb = IUP_YES;

  *sbh=*sbv=0;

  if (iupStrEqual (sb, IUP_YES))
   *sbh=*sbv=1;
  else if (iupStrEqual (sb, IUP_ON))
   *sbh=*sbv=1;
  else if (iupStrEqual (sb, IUP_VERTICAL))
   *sbv=1;
  else if (iupStrEqual (sb, IUP_HORIZONTAL))
   *sbh=1;
}


/*

%F Inicializes tree data
%i tree handle
%0 returns 0 if sucessful and 1 if fails

*/
static void InitTreeRoot(TtreePtr tree)
{
 Node root_node;
 root_node = (Node)malloc(sizeof(struct Node_));

 tree_root(tree) = root_node;
 tree_selected(tree) = root_node;
 tree_starting(tree) = root_node;
 tree_addexpanded(tree) = YES;
 tree_ctrl(tree) = NO;
 tree_shift(tree) = NO;

 node_depth(root_node) = 0;
 node_kind(root_node) = BRANCH;
 node_state(root_node) = EXPANDED;
 node_marked(root_node) = YES;
 node_visible(root_node) = YES;
 node_name(root_node) = NULL;
 node_next(root_node) = NULL;
 node_userid(root_node) = NULL;
 node_imageinuse(root_node) = NO;
 node_expandedimageinuse(root_node) = NO;
 node_text_color(root_node) = 0x000000L;
}

static int InitTree(Ihandle *ht)
{
 TtreePtr tree=(TtreePtr)tree_data(ht);
 int w, h;

 cdCanvasBackground(tree->cddbuffer,TREE_BGCOLOR);
 cdCanvasGetSize(tree->cddbuffer,&w,&h,NULL,NULL);
 XmaxCanvas(tree) = w-1;
 YmaxCanvas(tree) = h-1;

 IupSetAttribute(ht, IUP_BGCOLOR, TREE_BGCOLORSTRING);
 IupSetAttribute(tree_self(tree),IUP_DX,"1.0");
 IupSetAttribute(tree_self(tree),IUP_DY,"1.0");

 return 0;
}

/**************************************************************************
***************************************************************************
*
*   Callbacks for the canvas
*
***************************************************************************
***************************************************************************/

static void tree_createcanvas(TtreePtr tree)
{
  if (!tree->cdcanvas)
    tree->cdcanvas = cdCreateCanvas(CD_IUP, tree_self(tree));

  if (tree->cdcanvas)
  {
    cdCanvasActivate(tree->cdcanvas);
    tree->cddbuffer = cdCreateCanvas(CD_DBUFFER, tree->cdcanvas);
  }

  if (tree->cddbuffer)
    InitTree(tree_self(tree));
}

/*
 * %F Callback called when the tree has its size altered
 *
 * %i hm : tree's handle.
 *    dx,dy : Canvas size, in pixels.
 *    %o Returns IUP_DEFAULT.
 */
static int cbResize(Ihandle *ht, int dx, int dy)
{
  TtreePtr tree = (TtreePtr)tree_data(ht);

  if (!tree->cddbuffer)
    tree_createcanvas(tree);

  if (!tree->cddbuffer)
    return IUP_DEFAULT;

   cdCanvasActivate(tree->cddbuffer);

   XmaxCanvas(tree) = dx-1;
   YmaxCanvas(tree) = dy-1;

   treeEditCheckHidden(ht);
  
  return IUP_DEFAULT;
}

/*

%F Callback called when the tree is scrolled.
%i hm : tree's handle
%o Returns IUP_DEFAULT.

*/
static int cbScroll(Ihandle *ht)
{
  int err;
  TtreePtr tree=(TtreePtr)tree_data(ht);

  CdActivate(tree,err);
  
  if (err == CD_OK)
  {
    treeEditCheckHidden(ht);

    cdCanvasNativeFont(tree->cddbuffer,IupGetAttribute(ht,IUP_FONT));
	  treedrawDrawTree(ht);
    cdCanvasFlush(tree->cddbuffer);
  }

  if(tree && tree_selected(tree) && node_visible(tree_selected(tree)) == NO)
    treegetsetSetValue(ht, IUP_PREVIOUS, 0);
  
  return IUP_DEFAULT;

}

static void safestrcpy(char *dest, char *orig)
{
  if(orig && dest)
    strcpy(dest, orig);
}

/*
 * This function updates the size of the scrollbar or hides it if necessary
 * based on DX
 */
static void updateScrollPos(TtreePtr tree)
{
  char posy[10];
  char posx[10];
  char dx[10], dy[10];
  safestrcpy(dx, IupGetAttribute(tree_self(tree), IUP_DX));
  safestrcpy(dy, IupGetAttribute(tree_self(tree), IUP_DY));

  strcpy(posy,IupGetAttribute(tree_self(tree), IUP_POSY));
  if(atoi(posy) < 0 || atoi(dy) == 1)
    IupSetAttribute(tree_self(tree),IUP_POSY,"0");
  else
    IupStoreAttribute(tree_self(tree),IUP_POSY,posy);

  strcpy(posx,IupGetAttribute(tree_self(tree), IUP_POSX));
  if(atoi(posx) < 0 || atoi(dx) == 1)
    IupSetAttribute(tree_self(tree),IUP_POSX,"0");
  else
    IupStoreAttribute(tree_self(tree),IUP_POSX,posx);
}

static int cbRepaint(Ihandle *ht)
{
  TtreePtr tree=(TtreePtr)tree_data(ht);

  if (!tree->cddbuffer)
    tree_createcanvas(tree);

  if (!tree->cddbuffer)
    return IUP_DEFAULT;

  return treeRepaint(ht);
}

/*
%F Callback called when the tree needs to be redrawn.
%i hm : tree's handle
%o Returns IUP_DEFAULT.
*/
int treeRepaint(Ihandle *ht)
{
  int err;
  TtreePtr tree=(TtreePtr)tree_data(ht);

  if (!tree->cddbuffer)
    return IUP_DEFAULT;

  CdActivate(tree,err);
  
  if (err == CD_OK)
  {
    cdCanvasNativeFont(tree->cddbuffer,IupGetAttribute(ht,IUP_FONT));
    treedrawDrawTree(ht);  /* FIXME: split the calcsize from the redraw */
    updateScrollPos(tree);
    CdActivate(tree,err);
    treedrawDrawTree(ht);
    cdCanvasFlush(tree->cddbuffer);
  }

  if(tree && tree_selected(tree) && node_visible(tree_selected(tree)) == NO)
    treegetsetSetValue(ht, IUP_PREVIOUS,1);

  return IUP_DEFAULT;
}

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

Ihandle* IupTree(void)
{
  return IupCreate("tree");
}

static Ihandle* treecreate(Iclass* ic)
{
   TtreePtr tree = (TtreePtr)malloc(sizeof(Ttree));
   Ihandle* self = IupCanvas(NULL);
   memset(tree, 0, sizeof(Ttree)); 
  (void)ic;

   treesetdata(self,tree);
   tree_self(tree) = self;

   /* The tree has scrollbar by default */
   iupSetEnv(self,IUP_SCROLLBAR,IUP_YES);
   iupSetEnv(self,IUP_BORDER,IUP_NO);

   IupSetCallback(self,"ACTION",(Icallback)cbRepaint);
   IupSetCallback(self,"RESIZE_CB",(Icallback)cbResize);
   IupSetCallback(self,"SCROLL_CB",(Icallback)cbScroll);
   IupSetCallback(self,"BUTTON_CB",(Icallback)treeMouseButtonCb);
   IupSetCallback(self,"MOTION_CB",(Icallback)treeMouseMotionCb);
   IupSetCallback(self,"KEYPRESS_CB",(Icallback)treecallKeypressCb);

   /* Creates the rename node text field */
   treeEditCreate(tree);

   InitTreeRoot(tree);

   treegetsetSetImage(IUP_IMGLEAF,tree_image_leaf(tree),tree_color_leaf(tree),tree_marked_color_leaf(tree),TREE,NULL);
   treegetsetSetImage(IUP_IMGCOLLAPSED,tree_image_collapsed(tree),tree_color_collapsed(tree),tree_marked_color_collapsed(tree),TREE,NULL);
   treegetsetSetImage(IUP_IMGEXPANDED,tree_image_expanded(tree),tree_color_expanded(tree),tree_marked_color_expanded(tree),TREE,NULL);
   
   /* Initially marked element is root */
   treegetsetSetValue(self, "0", 0);
   
   return self;
}

/*

%F Destroys the interface element, freeing memory
%i self : Treeview's handle

*/
static void treedestroy (Ihandle* self)
{
  TtreePtr tree=(TtreePtr)tree_data(self);
  Ihandle* itext = NULL;
  Node temp, next;

  itext = tree_texth(tree);

  temp = tree_root(tree);
  while(temp)
  {
    next = node_next(temp);
    if(node_name(temp))
      free(node_name(temp));
    free(temp);
    temp = next;
  }

  if (tree->cddbuffer) cdKillCanvas(tree->cddbuffer);
  if (tree->cdcanvas) cdKillCanvas(tree->cdcanvas);

  free(tree);
  IupDestroy(itext);

  treesetdata(self, NULL);
}

static void treemap (Ihandle* self, Ihandle* parent)
{
  TtreePtr tree=(TtreePtr)tree_data(self);

  /* CANVAS mapping */
  iupCpiDefaultMap(self,parent);

  /* text mapping */
  treeEditMap(tree, self);
}

/* Calculates the natural width of the tree */
static int treeGetNaturalWidth(Ihandle *self)
{
  return TREE_WIDTH;
}

/* Calculates the natural height of the tree */
static int treeGetNaturalHeight(Ihandle *self)
{
  return TREE_HEIGHT;
}

static int treesetnaturalsize(Ihandle *self)
{
  int  width, height, sizex, sizey;
  int  sbh, sbv, sbw = 0;

  /* Gets scrollbar parameters*/
  treegetsb(self,&sbh, &sbv);
  if (sbv || sbh) sbw = iupdrvGetScrollSize();

  /* Gets values associated with SIZE attribute */
  iupGetSize(self, &sizex, &sizey);

  /* Get natural size */
  if(sizex)
   width = sizex;
  else
   width = treeGetNaturalWidth(self) + sbw*sbv;

  /* Gets natural height*/
  if(sizey)
   height = sizey;
  else
   height = treeGetNaturalHeight(self) + sbw*sbh;

  iupSetNaturalWidth (self, width);
  iupSetNaturalHeight(self, height);

  return 3; /* Resize is allowed in all directions */
}

static void treesetattr(Ihandle* self, char* attr, char* value)
{
   TtreePtr tree=(TtreePtr)tree_data(self);

   if (!attr) return;
   
   if (self == NULL || attr == NULL)
   {
    iupCpiDefaultSetAttr(self,attr,value);
    return;
   }

   if (iupStrEqual(attr,IUP_ACTIVE))
   {
      iupCpiDefaultSetAttr(self,attr,value);
   }
   else if (iupStrEqual(attr,IUP_ADDEXPANDED))
   {
     treegetsetAddExpanded(self,value);
   }
   else if (iupStrEqualPartial(attr,IUP_NAME))
   { 
     treegetsetSetName(self,&attr[strlen(IUP_NAME)],value);
   }
   else if (iupStrEqualPartial(attr,IUP_COLOR))
   { 
     treegetsetSetColor(self,&attr[strlen(IUP_COLOR)],value);
   }
   else if (iupStrEqualPartial(attr,IUP_MARKED))
   {
     treegetsetMark(self, &attr[strlen(IUP_MARKED)], value);
     iupSetEnv(self, attr, NULL); /* Do not store id dependent attributes in the environment */
   }
   else if (iupStrEqual(attr,IUP_VALUE))
   {
     /* Unmarks all nodes if control is not pressed and value is not INVERT,
      * because INVERT does not depend on the control treeKey status. */
     if(tree_ctrl == NO && value && !iupStrEqualPartial(value,IUP_INVERT))
       treegetsetSetMarked(self,NO,0);

     treegetsetSetValue(self,value,0);

     /* Marks block if tree_shift is pressed and value is not CLEARALL, to guarantee
      * CLEARALL will work even if there is a block marked */
     if(tree_shift == YES && value && !iupStrEqual(value,IUP_CLEARALL))
       treegetsetSetValue(self,IUP_BLOCK,0);
   }
   else if (iupStrEqual(attr,IUP_STARTING))
   {
     treegetsetSetStarting(self,value);
   }
   else if (iupStrEqual(attr,IUP_REDRAW))
   {
     treeRepaint(self);
   }
   else if (iupStrEqual(attr,IUP_REPAINT))
   {
     treeRepaint(self);
   }
   else if (iupStrEqual(attr,IUP_IMAGELEAF))
   {
     treegetsetSetImage(value,tree_image_leaf(tree),tree_color_leaf(tree),tree_marked_color_leaf(tree),TREE,NULL);
   }
   else if (iupStrEqual(attr,IUP_IMAGEBRANCHCOLLAPSED))
   {
     treegetsetSetImage(value,tree_image_collapsed(tree),tree_color_collapsed(tree),tree_marked_color_collapsed(tree),TREE,NULL);
   }
   else if (iupStrEqual(attr,IUP_IMAGEBRANCHEXPANDED))
   {
     treegetsetSetImage(value,tree_image_expanded(tree),tree_color_expanded(tree),tree_marked_color_expanded(tree),TREE,NULL);
   }
   /* IUP_IMAGEEXPANDED *MUST* appear before IUP_IMAGE, or else 
    * IUP_IMAGEEXPANDED will never be called...*/
   else if (iupStrEqualPartial(attr,IUP_IMAGEEXPANDED))
   {
     Node node = treefindNodeFromString(self, &attr[strlen(IUP_IMAGEEXPANDED)]);
       
     treegetsetSetImage(value,node_expandedimage(node),node_expandedcolor(node),node_expandedmarked_color(node),NODEEXPANDED,node);
     iupSetEnv(self, attr, NULL); /* Do not store id dependent attributes in the environment */
   }
   else if (iupStrEqualPartial(attr,IUP_IMAGE))
   {
     Node node = treefindNodeFromString(self, &attr[strlen(IUP_IMAGE)]);
	   
     treegetsetSetImage(value,node_image(node),node_color(node),node_marked_color(node),NODE,node);
     iupSetEnv(self, attr, NULL); /* Do not store id dependent attributes in the environment */
   }
   else if (iupStrEqualPartial(attr,IUP_DEPTH))
   {
     treegetsetSetDepth(self,&attr[strlen(IUP_DEPTH)],value);
   }
   else if (iupStrEqualPartial(attr,IUP_STATE))
   {
     treegetsetSetState(self,&attr[strlen(IUP_STATE)],value);
   }
   else if (iupStrEqual(attr,"RENAME"))
   {
     int x, y, text_x;
     if (treeNodeCalcPos(self, &x, &y, &text_x))
     {
       if (IupGetInt(self, "SHOWRENAME"))
         treeEditShow(self,text_x,x,y);
       else
         treecallRenameNodeCb(self);
     }
   }
   else if (iupStrEqual(attr,IUP_CTRL))
   {
     treegetsetSetCtrl(self,value);
   }
   else if (iupStrEqual(attr,IUP_SHIFT))
   {
     treegetsetSetShift(self,value);
   }
   else if (iupStrEqualPartial(attr,IUP_DELNODE))
   {
     treegetsetDelNode(self,&attr[strlen(IUP_DELNODE)],value);
   }
   else if (iupStrEqualPartial(attr,IUP_ADDLEAF))
   {
     static char next[10];

     if(!iupStrEqual(attr,IUP_ADDLEAF))
       sprintf(next, "%d", atoi(&attr[strlen(IUP_ADDLEAF)]) + 1);
     else
       sprintf(next, "%d", atoi(IupGetAttribute(self,IUP_VALUE)) + 1);
	   
     if(treegetsetAddNode(self,&attr[strlen(IUP_ADDLEAF)],LEAF))
       treegetsetSetName(self,next,value);
   }
   else if (iupStrEqualPartial(attr,IUP_ADDBRANCH))
   {
     static char next[10];
     
     if(!iupStrEqual(attr,IUP_ADDBRANCH))
       sprintf(next, "%d", atoi(&attr[strlen(IUP_ADDBRANCH)]) + 1);
     else
       sprintf(next, "%d", atoi(IupGetAttribute(self,IUP_VALUE)) + 1);

     if(treegetsetAddNode(self,&attr[strlen(IUP_ADDBRANCH)],BRANCH))
       treegetsetSetName(self,next,value);
   }
   else if (iupStrEqual(attr,"RENAMECARET"))
   {
     IupSetAttribute(tree_texth(tree),IUP_CARET,value);
     iupStoreEnv(self, IUP_CARET, value);
   }
   else if (iupStrEqual(attr,"RENAMESELECTION"))
   {
     IupSetAttribute(tree_texth(tree),IUP_SELECTION,value);
     iupStoreEnv(self, IUP_SELECTION, value);
   }
   else
     iupCpiDefaultSetAttr(self,attr,value);
}

static char* treegetattr(Ihandle* self, char* attr)
{
  if (self == NULL || attr == NULL)
    return NULL;
  else if (iupStrEqual(attr,IUP_VALUE))
  {
    return treegetsetGetValue(self);
  }
  else if (iupStrEqual(attr,IUP_STARTING))
  {
    return treegetsetGetStarting(self);
  }
  else if (iupStrEqual(attr,IUP_CTRL))
  {
    return treegetsetGetCtrl(self);
  }
  else if (iupStrEqual(attr,IUP_SHIFT))
  {
    return treegetsetGetShift(self);
  }
  else if (iupStrEqualPartial(attr,IUP_NAME))
  {
   return treegetsetGetName(self,&attr[strlen(IUP_NAME)]);
  }
  else if (iupStrEqualPartial(attr,IUP_KIND))
  {
    return treegetsetGetKind(self,&attr[strlen(IUP_KIND)]);
  }
  else if (iupStrEqualPartial(attr,IUP_STATE))
  {
    return treegetsetGetState(self,&attr[strlen(IUP_STATE)]);
  }
  else if (iupStrEqualPartial(attr,IUP_PARENT))
  {
    return treegetsetGetParent(self,&attr[strlen(IUP_PARENT)]);
  }
  else if (iupStrEqualPartial(attr,IUP_DEPTH))
  {
    return treegetsetGetDepth(self,&attr[strlen(IUP_DEPTH)]);
  }
  else if (iupStrEqualPartial(attr,IUP_MARKED))
  {
    return treegetsetGetMarked(self,&attr[strlen(IUP_MARKED)]);
  }
  else if (iupStrEqualPartial(attr,IUP_COLOR))
  {
    return treegetsetGetColor(self,&attr[strlen(IUP_COLOR)]);
  }
  else if (iupStrEqual(attr,"RENAMECARET"))
  {
    TtreePtr tree=(TtreePtr)tree_data(self);
    return IupGetAttribute(tree_texth(tree),IUP_CARET);
  }
  else if (iupStrEqual(attr,"RENAMESELECTION"))
  {
    TtreePtr tree=(TtreePtr)tree_data(self);
    return IupGetAttribute(tree_texth(tree),IUP_SELECTION);
  }

  return iupCpiDefaultGetAttr(self, attr);
}

static void destroyImage(char* name)
{
  Ihandle *imgcursor = IupGetHandle(name);
  if (imgcursor) IupDestroy(imgcursor);
}

void IupTreeClose(void)
{
  destroyImage("IupTreeDragCursor");

  destroyImage("IMGLEAF");
  destroyImage("IMGCOLLAPSED");
  destroyImage("IMGEXPANDED");
  destroyImage("IMGBLANK");
  destroyImage("IMGPAPER");
}

void IupTreeOpen(void)
{
  unsigned char tree_img_drag_cur[16*24] = 
  {
   1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0
  ,1,2,1,0,0,0,0,0,0,0,0,0,0,0,0,0
  ,1,2,2,1,0,0,0,0,0,0,0,0,0,0,0,0
  ,1,2,2,2,1,0,0,0,0,0,0,0,0,0,0,0
  ,1,2,2,2,2,1,0,0,0,0,0,0,0,0,0,0
  ,1,2,2,2,2,2,1,0,0,0,0,0,0,0,0,0
  ,1,2,2,2,2,2,2,1,0,0,0,0,0,0,0,0
  ,1,2,2,2,2,2,2,2,1,0,0,0,0,0,0,0
  ,1,2,2,2,2,2,2,2,2,1,0,0,0,0,0,0
  ,1,2,2,2,2,2,2,2,2,2,1,0,0,0,0,0
  ,1,2,2,2,2,2,2,1,1,1,1,1,0,0,0,0
  ,1,2,2,2,2,2,2,1,0,0,0,0,0,0,0,0
  ,1,2,2,1,1,2,2,1,0,0,0,0,0,0,0,0
  ,1,2,1,0,0,1,2,2,1,0,0,0,0,0,0,0
  ,1,1,0,0,0,1,2,2,1,0,0,0,0,0,0,0
  ,1,0,0,1,2,0,1,2,2,1,2,1,2,1,2,1
  ,0,0,0,2,1,2,1,2,2,1,0,2,1,2,1,2
  ,0,0,0,1,2,0,0,1,2,2,1,0,0,0,2,1
  ,0,0,0,2,1,0,0,1,2,1,1,0,0,0,1,2
  ,0,0,0,1,2,0,0,0,1,0,0,0,0,0,2,1
  ,0,0,0,2,1,0,0,0,0,0,0,0,0,0,1,2
  ,0,0,0,1,2,0,0,0,0,0,0,0,0,0,2,1
  ,0,0,0,2,1,2,1,2,1,2,1,2,1,2,1,2
  ,0,0,0,1,2,1,2,1,2,1,2,1,2,1,2,1
  };

  Ihandle* imgcursor;
  Iclass* ICTree = iupCpiCreateNewClass("tree", NULL);

  iupCpiSetClassMethod(ICTree, ICPI_SETNATURALSIZE, (Imethod) treesetnaturalsize);
  iupCpiSetClassMethod(ICTree, ICPI_CREATE,         (Imethod) treecreate);
  iupCpiSetClassMethod(ICTree, ICPI_DESTROY,        (Imethod) treedestroy);
  iupCpiSetClassMethod(ICTree, ICPI_SETATTR,        (Imethod) treesetattr);
  iupCpiSetClassMethod(ICTree, ICPI_GETATTR,        (Imethod) treegetattr);
  iupCpiSetClassMethod(ICTree, ICPI_MAP,            (Imethod) treemap);

  imgcursor = IupImage(16,24, tree_img_drag_cur);
  IupSetAttribute (imgcursor, "0", "BGCOLOR"); 
  IupSetAttribute (imgcursor, "1", "0 0 0"); 
  IupSetAttribute (imgcursor, "2", "255 255 255"); 
  IupSetAttribute (imgcursor, IUP_HOTSPOT, "0:0");
  IupSetHandle ("IupTreeDragCursor", imgcursor); 

  treeimageInitializeImages();
}

void IupTreeSetAttribute(Ihandle *n, const char* a, int id, char* v)
{
  char attr[50];
  sprintf(attr, "%s%d", a, id);
  IupSetAttribute(n, attr, v);
}

void IupTreeStoreAttribute(Ihandle *n, const char* a, int id, char* v)
{
  char attr[50];
  sprintf(attr, "%s%d", a, id);
  IupStoreAttribute(n, attr, v);
}

char* IupTreeGetAttribute(Ihandle *n, const char* a, int id)
{
  char attr[50];
  sprintf(attr, "%s%d", a, id);
  return IupGetAttribute(n, attr);
}

int IupTreeGetInt(Ihandle *n, const char* a, int id)
{
  char attr[50];
  sprintf(attr, "%s%d", a, id);
  return IupGetInt(n, attr);
}

float IupTreeGetFloat(Ihandle *n, const char* a, int id)
{
  char attr[50];
  sprintf(attr, "%s%d", a, id);
  return IupGetFloat(n, attr);
}

void IupTreeSetfAttribute(Ihandle *n, const char* a, int id, const char* f, ...)
{
  static char v[SHRT_MAX];
  char attr[50];
  va_list arglist;
  sprintf(attr, "%s%d", a, id);
  va_start(arglist, f);
  vsprintf(v, f, arglist);
  va_end(arglist);
  IupStoreAttribute(n, attr, v);
}
