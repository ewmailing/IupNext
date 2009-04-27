/** \file
 * \brief Tree control
 *
 * See Copyright Notice in iup.h
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "iup.h"
#include "iupcbs.h"

#include "iup_object.h"
#include "iup_attrib.h"
#include "iup_str.h"
#include "iup_drv.h"
#include "iup_drvfont.h"
#include "iup_stdcontrols.h"
#include "iup_layout.h"
#include "iup_tree.h"

#define ITREE_TREE_HEIGHT  200  /* Tree height */
#define ITREE_TREE_WIDTH   400  /* Tree width  */


static void iTreeInitializeImages(void)
{
  Ihandle *image_leaf, *image_blank, *image_paper;  
  Ihandle *image_collapsed, *image_expanded;  

  unsigned char img_leaf[ITREE_NODE_WIDTH*ITREE_NODE_HEIGHT] = 
  {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 4, 4, 5, 5, 5, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 4, 5, 5, 1, 6, 1, 5, 0, 0, 0, 0, 0,
    0, 0, 0, 3, 4, 4, 5, 5, 1, 6, 1, 5, 0, 0, 0, 0,
    0, 0, 0, 3, 4, 4, 4, 5, 5, 1, 1, 5, 0, 0, 0, 0,
    0, 0, 0, 2, 3, 4, 4, 4, 5, 5, 5, 4, 0, 0, 0, 0,
    0, 0, 0, 2, 3, 3, 4, 4, 4, 5, 4, 4, 0, 0, 0, 0,
    0, 0, 0, 0, 2, 3, 3, 4, 4, 4, 4, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 2, 2, 3, 3, 3, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  };

  unsigned char img_collapsed[ITREE_NODE_WIDTH*ITREE_NODE_HEIGHT] =
  {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  
    0, 0, 2, 2, 2, 2, 2, 3, 0, 0, 0, 0, 0, 0, 0, 0,  
    0, 2, 6, 5, 5, 7, 7, 2, 3, 0, 0, 0, 0, 0, 0, 0, 
    2, 4, 4, 4, 4, 4, 4, 4, 2, 2, 2, 2, 2, 2, 0, 0, 
    2, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 7, 4, 3, 0, 
    2, 5, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 1, 4, 3, 0, 
    2, 5, 7, 7, 7, 7, 7, 7, 7, 1, 7, 1, 7, 4, 3, 0, 
    2, 5, 7, 7, 7, 7, 7, 7, 7, 7, 1, 7, 1, 4, 3, 0, 
    2, 5, 7, 7, 7, 7, 7, 1, 7, 1, 7, 1, 7, 4, 3, 0, 
    2, 5, 7, 7, 7, 7, 7, 7, 1, 7, 1, 7, 1, 4, 3, 0, 
    2, 5, 7, 7, 7, 1, 7, 1, 7, 1, 7, 1, 1, 4, 3, 0, 
    2, 5, 1, 7, 1, 7, 1, 7, 1, 7, 1, 1, 1, 4, 3, 0, 
    2, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3, 0,  
    0, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 0, 
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0  
  };

  unsigned char img_expanded[ITREE_NODE_WIDTH*ITREE_NODE_HEIGHT] =
  {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 2, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 
    0, 0, 2, 1, 1, 1, 1, 1, 2, 0, 0, 0, 0, 0, 0, 0, 
    0, 2, 1, 3, 3, 3, 3, 3, 1, 2, 2, 2, 2, 2, 2, 0, 
    0, 2, 1, 3, 3, 3, 3, 3, 3, 3, 1, 1, 1, 1, 6, 4, 
    0, 2, 1, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 6, 4, 
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 6, 3, 6, 4, 
    2, 1, 3, 3, 3, 3, 3, 3, 5, 3, 5, 6, 4, 6, 6, 4, 
    2, 1, 3, 3, 3, 3, 5, 3, 3, 5, 3, 6, 4, 6, 6, 4, 
    0, 2, 0, 3, 3, 3, 3, 3, 5, 3, 5, 5, 2, 4, 2, 4, 
    0, 2, 0, 3, 3, 5, 3, 5, 3, 5, 5, 5, 6, 4, 2, 4, 
    0, 0, 2, 0, 5, 3, 5, 3, 5, 5, 5, 5, 6, 2, 4, 4, 
    0, 0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 4, 4, 
    0, 0, 0, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 
  };

  unsigned char img_blank[ITREE_NODE_WIDTH*ITREE_NODE_HEIGHT] =
  {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
    0, 0, 4, 3, 4, 3, 4, 3, 4, 3, 4, 3, 0, 0, 0, 0,
    0, 0, 3, 1, 1, 1, 1, 1, 1, 1, 1, 5, 4, 0, 0, 0,
    0, 0, 4, 1, 1, 1, 1, 1, 1, 1, 1, 5, 1, 4, 0, 0,
    0, 0, 3, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 0,
    0, 0, 4, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 5, 2, 0,
    0, 0, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 5, 2, 0,
    0, 0, 4, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 5, 2, 0,
    0, 0, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 5, 2, 0,
    0, 0, 4, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 5, 2, 0,
    0, 0, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 5, 2, 0,
    0, 0, 4, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 5, 2, 0,
    0, 0, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 5, 2, 0,
    0, 0, 4, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 5, 2, 0,
    0, 0, 3, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 2, 0,
    0, 0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0
  };

  unsigned char img_paper[ITREE_NODE_WIDTH*ITREE_NODE_HEIGHT] =
  {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
    0, 0, 4, 3, 4, 3, 4, 3, 4, 3, 4, 3, 0, 0, 0, 0,
    0, 0, 3, 1, 1, 1, 1, 1, 1, 1, 1, 5, 4, 0, 0, 0,
    0, 0, 4, 1, 1, 1, 1, 1, 1, 1, 1, 5, 1, 4, 0, 0,
    0, 0, 3, 1, 4, 3, 4, 3, 4, 3, 4, 2, 2, 2, 2, 0,
    0, 0, 4, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 5, 2, 0,
    0, 0, 3, 1, 3, 4, 3, 4, 3, 4, 3, 4, 1, 5, 2, 0,
    0, 0, 4, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 5, 2, 0,
    0, 0, 3, 1, 4, 3, 4, 3, 4, 3, 4, 3, 1, 5, 2, 0,
    0, 0, 4, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 5, 2, 0,
    0, 0, 3, 1, 3, 4, 3, 4, 3, 4, 3, 4, 1, 5, 2, 0,
    0, 0, 4, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 5, 2, 0,
    0, 0, 3, 1, 4, 3, 4, 3, 4, 3, 4, 3, 1, 5, 2, 0,
    0, 0, 4, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 5, 2, 0,
    0, 0, 3, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 2, 0,
    0, 0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0
  };

  image_leaf      = IupImage(ITREE_NODE_WIDTH, ITREE_NODE_HEIGHT, img_leaf);
  image_collapsed = IupImage(ITREE_NODE_WIDTH, ITREE_NODE_HEIGHT, img_collapsed);
  image_expanded  = IupImage(ITREE_NODE_WIDTH, ITREE_NODE_HEIGHT, img_expanded);
  image_blank     = IupImage(ITREE_NODE_WIDTH, ITREE_NODE_HEIGHT, img_blank);
  image_paper     = IupImage(ITREE_NODE_WIDTH, ITREE_NODE_HEIGHT, img_paper);

  IupSetAttribute(image_leaf, "0", "BGCOLOR");
  IupSetAttribute(image_leaf, "1", "192 192 192");
  IupSetAttribute(image_leaf, "2", "56 56 56");
  IupSetAttribute(image_leaf, "3", "99 99 99");
  IupSetAttribute(image_leaf, "4", "128 128 128");
  IupSetAttribute(image_leaf, "5", "161 161 161");
  IupSetAttribute(image_leaf, "6", "222 222 222");

  IupSetAttribute(image_collapsed, "0", "BGCOLOR");
  IupSetAttribute(image_collapsed, "1", "255 206 156");
  IupSetAttribute(image_collapsed, "2", "156 156 0");
  IupSetAttribute(image_collapsed, "3", "0 0 0");
  IupSetAttribute(image_collapsed, "4", "206 206 99");
  IupSetAttribute(image_collapsed, "5", "255 255 206");
  IupSetAttribute(image_collapsed, "6", "247 247 247");
  IupSetAttribute(image_collapsed, "7", "255 255 156");

  IupSetAttribute(image_expanded, "0", "BGCOLOR");
  IupSetAttribute(image_expanded, "1", "255 255 255");
  IupSetAttribute(image_expanded, "2", "156 156 0");
  IupSetAttribute(image_expanded, "3", "255 255 156");
  IupSetAttribute(image_expanded, "4", "0 0 0");
  IupSetAttribute(image_expanded, "5", "255 206 156");
  IupSetAttribute(image_expanded, "6", "206 206 99");

  IupSetAttribute(image_blank, "0", "BGCOLOR");
  IupSetAttribute(image_blank, "1", "255 255 255");
  IupSetAttribute(image_blank, "2", "000 000 000");
  IupSetAttribute(image_blank, "3", "119 119 119");
  IupSetAttribute(image_blank, "4", "136 136 136");
  IupSetAttribute(image_blank, "5", "187 187 187");

  IupSetAttribute(image_paper, "0", "BGCOLOR");
  IupSetAttribute(image_paper, "1", "255 255 255");
  IupSetAttribute(image_paper, "2", "000 000 000");
  IupSetAttribute(image_paper, "3", "119 119 119");
  IupSetAttribute(image_paper, "4", "136 136 136");
  IupSetAttribute(image_paper, "5", "187 187 187");

  IupSetHandle("IMGLEAF",      image_leaf);
  IupSetHandle("IMGCOLLAPSED", image_collapsed);
  IupSetHandle("IMGEXPANDED",  image_expanded);
  IupSetHandle("IMGBLANK",     image_blank);
  IupSetHandle("IMGPAPER",     image_paper);
}

static int iTreeSetScrollbarAttrib(Ihandle* ih, const char* value)
{
  /* valid only before map */
  if (ih->handle)
    return 0;

  if (iupStrEqualNoCase(value, "YES"))
    ih->data->sb = IUP_SB_HORIZ | IUP_SB_VERT;
  else if (iupStrEqualNoCase(value, "HORIZONTAL"))
    ih->data->sb = IUP_SB_HORIZ;
  else if (iupStrEqualNoCase(value, "VERTICAL"))
    ih->data->sb = IUP_SB_VERT;
  else
    ih->data->sb = IUP_SB_NONE;

  return 0;
}

static char* iTreeGetScrollbarAttrib(Ihandle* ih)
{
  if (ih->data->sb == (IUP_SB_HORIZ | IUP_SB_VERT))
    return "YES";
  if (ih->data->sb &= IUP_SB_HORIZ)
    return "HORIZONTAL";
  if (ih->data->sb == IUP_SB_VERT)
    return "VERTICAL";
  return "NO";   /* IUP_SB_NONE */
}

/* Retrieves the tree_ctrl iTreeKey state */
static char* iTreeGetCtrlAttrib(Ihandle* ih)
{
  if (ih->data->tree_ctrl)
    return "YES";
  else
    return "NO";
}

/* Turns on and off the ctrl key function (value: "YES" or "NO") */
static int iTreeSetCtrlAttrib(Ihandle* ih, const char* value)
{
  if (iupStrEqualNoCase(value, "YES"))
    ih->data->tree_ctrl = 1;    
  else 
    ih->data->tree_ctrl = 0;
  return 0;
}

/* Retrieves the tree_shift iTreeKey state */
static char* iTreeGetShiftAttrib(Ihandle* ih)
{
  if (ih->data->tree_shift)
    return "YES";
  else
    return "NO";
}

/* Turns on and off the shift key function (value: "YES" or "NO") */
static int iTreeSetShiftAttrib(Ihandle* ih, const char* value)
{
  if (iupStrEqualNoCase(value, "YES"))
    ih->data->tree_shift = 1;    
  else 
    ih->data->tree_shift = 0;
  return 0;
}

static char* iTreeGetRenameCaretAttrib(Ihandle* ih)
{
  return ih->data->rename_caret;
}

static int iTreeSetRenameCaretAttrib(Ihandle* ih, const char* value)
{
  ih->data->rename_caret = (char*)value;
  return 1;
}

static char* iTreeGetRenameSelectionAttrib(Ihandle* ih)
{
  return ih->data->rename_selection;
}

static int iTreeSetRenameSelectionAttrib(Ihandle* ih, const char* value)
{
  ih->data->rename_selection = (char*)value;
  return 1;
}

static int iTreeSetAddLeafAttrib(Ihandle* ih, const char* name_id, const char* value)
{
  char* next = iupStrGetMemory(10);
  int id = 0;

  if (name_id[0])
    id = atoi(name_id) + 1;
  else
    id = atoi(IupGetAttribute(ih, "VALUE")) + 1;

  sprintf(next, "%d", id);

  if (iupdrvTreeAddNode(ih, name_id, ITREE_LEAF))
    iupdrvTreeSetNameAttrib(ih, next, value);

  return 1;
}

static int iTreeSetAddBranchAttrib(Ihandle* ih, const char* name_id, const char* value)
{
  char* next = iupStrGetMemory(10);
  int id = 0;

  if (name_id[0])
    id = atoi(name_id) + 1;
  else
    id = atoi(IupGetAttribute(ih, "VALUE")) + 1;

  sprintf(next, "%d", id);

  if(iupdrvTreeAddNode(ih, name_id, ITREE_BRANCH))
    iupdrvTreeSetNameAttrib(ih, next, value);

  return 1;
}

static char* iTreeGetShowRenameAttrib(Ihandle* ih)
{
  if (ih->data->show_rename)
    return "YES";
  else
    return "NO";
}

static int iTreeSetShowRenameAttrib(Ihandle* ih, const char* value)
{
  /* valid only before map */
  if (ih->handle)
    return 0;

  if (iupStrBoolean(value))
    ih->data->show_rename = 1;
  else
    ih->data->show_rename = 0;

  return 0;
}

static int iTreeCreateMethod(Ihandle* ih, void **params)
{
  (void)params;

  ih->data = iupALLOCCTRLDATA();

  ih->data->sb = IUP_SB_HORIZ | IUP_SB_VERT;

  return IUP_NOERROR;
}

static void iTreeComputeNaturalSizeMethod(Ihandle* ih)
{
  iupBaseContainerUpdateExpand(ih);

  /* always initialize the natural size using the user size */
  ih->naturalwidth = ih->userwidth;
  ih->naturalheight = ih->userheight;

  /* if user size is not defined, then calculate the natural size */
  if (ih->naturalwidth <= 0 || ih->naturalheight <= 0)
  {
    int natural_w = ITREE_TREE_WIDTH;
    int natural_h = ITREE_TREE_HEIGHT;

    /* add scrollbar */
    if (ih->data->sb)
    {
      int sb_size = iupdrvGetScrollbarSize();

      if (ih->data->sb & IUP_SB_HORIZ)
        natural_w += sb_size;
      if (ih->data->sb & IUP_SB_VERT)
        natural_h += sb_size;
    }

    if (ih->naturalwidth <= 0) ih->naturalwidth = natural_w;
    if (ih->naturalheight <= 0) ih->naturalheight = natural_h;
  }
}

Ihandle* IupTree(void)
{
  return IupCreate("tree");
}

Iclass* iupTreeGetClass(void)
{
  Iclass* ic = iupClassNew(NULL);

  ic->name = "tree";
  ic->format = NULL; /* no parameters */
  ic->nativetype = IUP_TYPECONTROL;
  ic->childtype = IUP_CHILDNONE;
  ic->is_interactive = 1;
  ic->has_attrib_id = 1;   /* has attributes with IDs that must be parsed */

  /* Class functions */
  ic->Create = iTreeCreateMethod;
  ic->ComputeNaturalSize = iTreeComputeNaturalSizeMethod;

  ic->SetCurrentSize = iupBaseSetCurrentSizeMethod;
  ic->SetPosition = iupBaseSetPositionMethod;

  ic->LayoutUpdate = iupdrvBaseLayoutUpdateMethod;
  ic->UnMap = iupdrvBaseUnMapMethod;

  /* Callbacks */
  iupClassRegisterCallback(ic, "SELECTION_CB",      "ii");
  iupClassRegisterCallback(ic, "MULTISELECTION_CB", "Ii");
  iupClassRegisterCallback(ic, "BRANCHOPEN_CB",     "i");
  iupClassRegisterCallback(ic, "BRANCHCLOSE_CB",    "i");
  iupClassRegisterCallback(ic, "EXECUTELEAF_CB",    "i");
  iupClassRegisterCallback(ic, "RENAMENODE_CB",     "is");
  iupClassRegisterCallback(ic, "SHOWRENAME_CB",     "i");
  iupClassRegisterCallback(ic, "RENAME_CB",         "is");
  iupClassRegisterCallback(ic, "DRAGDROP_CB",       "iiii");
  iupClassRegisterCallback(ic, "RIGHTCLICK_CB",     "i");

  /* Common */
  iupBaseRegisterCommonAttrib(ic);

  /* Visual */
  iupBaseRegisterVisualAttrib(ic);

  /* IupTree only */
  iupClassRegisterAttribute(ic, "SCROLLBAR", iTreeGetScrollbarAttrib, iTreeSetScrollbarAttrib, IUPAF_SAMEASSYSTEM, "YES", IUPAF_NOT_MAPPED);

  /* IupTree Attributes - GENERAL */
  iupClassRegisterAttribute(ic, "RENAMECARET",     iTreeGetRenameCaretAttrib,     iTreeSetRenameCaretAttrib,     NULL, NULL, IUPAF_NO_DEFAULTVALUE|IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "RENAMESELECTION", iTreeGetRenameSelectionAttrib, iTreeSetRenameSelectionAttrib, NULL, NULL, IUPAF_NO_DEFAULTVALUE|IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "SHOWRENAME",      iTreeGetShowRenameAttrib,      iTreeSetShowRenameAttrib,      NULL, "NO", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);

  /* IupTree Attributes - MARKS */
  iupClassRegisterAttribute(ic, "CTRL",  iTreeGetCtrlAttrib,  iTreeSetCtrlAttrib,  NULL, "NO", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "SHIFT", iTreeGetShiftAttrib, iTreeSetShiftAttrib, NULL, "NO", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);

  /* IupTree Attributes - NODES */
  iupClassRegisterAttributeId(ic, "NAME", iupdrvTreeGetNameAttrib, iupdrvTreeSetNameAttrib, IUPAF_NO_DEFAULTVALUE|IUPAF_NO_INHERIT);

  /* IupTree Attributes - ACTION */
  iupClassRegisterAttributeId(ic, "ADDLEAF",   NULL, iTreeSetAddLeafAttrib,   IUPAF_WRITEONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "ADDBRANCH", NULL, iTreeSetAddBranchAttrib, IUPAF_WRITEONLY|IUPAF_NO_INHERIT);
  
  /* Default node images */
  iTreeInitializeImages();

  iupdrvTreeInitClass(ic);

  return ic;
}

