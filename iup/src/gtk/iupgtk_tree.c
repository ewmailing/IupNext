/** \file
 * \brief Tree Control
 *
 * See Copyright Notice in iup.h
 */

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <stdarg.h>

#include "iup.h"
#include "iupcbs.h"

#include "iup_object.h"
#include "iup_layout.h"
#include "iup_attrib.h"
#include "iup_str.h"
#include "iup_drv.h"
#include "iup_drvfont.h"
#include "iup_stdcontrols.h"
#include "iup_key.h"
#include "iup_image.h"
#include "iup_array.h"
#include "iup_tree.h"

#include "iupgtk_drv.h"

enum
{
  IUPGTK_TREE_IMAGE_LEAF,
  IUPGTK_TREE_IMAGE_COLLAPSED,
  IUPGTK_TREE_IMAGE_EXPANDED,
  IUPGTK_TREE_TITLE,
  IUPGTK_TREE_KIND,
  IUPGTK_TREE_COLOR,
  IUPGTK_TREE_USERDATA
};

/*****************************************************************************/
/* COPYING ITEMS (Branches and its children)                                 */
/*****************************************************************************/
/* Insert the copied item in a new location. Returns the new item. */
static GtkTreeIter gtkTreeCopyItem(Ihandle* ih, GtkTreeIter hItem, GtkTreeIter htiNewParent)
{
  GtkTreeStore* store = GTK_TREE_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(ih->handle)));
  GtkTreeIter  iterNewItem;
  int kind;
  char* title;
  void* userdata;
  GdkColor color = {0L,0,0,0};
  GdkPixbuf* image_leaf, *image_collapsed, *image_expanded;

  gtk_tree_model_get(GTK_TREE_MODEL(store), &hItem, IUPGTK_TREE_IMAGE_LEAF,      &image_leaf,
                                                    IUPGTK_TREE_IMAGE_COLLAPSED, &image_collapsed,
                                                    IUPGTK_TREE_IMAGE_EXPANDED,  &image_expanded,
                                                    IUPGTK_TREE_TITLE,  &title,
                                                    IUPGTK_TREE_KIND,  &kind,
                                                    IUPGTK_TREE_COLOR, &color, 
                                                    IUPGTK_TREE_USERDATA, &userdata,
                                                    -1);

  gtk_tree_store_append(store, &iterNewItem, &htiNewParent);

  gtk_tree_store_set(store, &iterNewItem, IUPGTK_TREE_IMAGE_LEAF,      image_leaf,
                                          IUPGTK_TREE_IMAGE_COLLAPSED, image_collapsed,
                                          IUPGTK_TREE_IMAGE_EXPANDED,  image_expanded,
                                          IUPGTK_TREE_TITLE,  title,
                                          IUPGTK_TREE_KIND,  kind,
                                          IUPGTK_TREE_COLOR, &color, 
                                          IUPGTK_TREE_USERDATA, userdata,
                                          -1);

  return iterNewItem;
}

/* Copies all items in a branch to a new location. Returns the new branch node. */
static GtkTreeIter gtkTreeCopyBranch(Ihandle* ih, GtkTreeIter iterBranch, GtkTreeIter iterNewParent)
{
  GtkTreeModel* modelTree = gtk_tree_view_get_model(GTK_TREE_VIEW(ih->handle));
  GtkTreeIter iterNewItem = gtkTreeCopyItem(ih, iterBranch, iterNewParent);
  GtkTreeIter iterChild;
  int hasItem = gtk_tree_model_iter_children(modelTree, &iterChild, &iterBranch);  /* get the firstchild */

  while(hasItem)
  {
    /* Recursively transfer all the items */
    gtkTreeCopyBranch(ih, iterChild, iterNewItem);  

    /* Go to next sibling item */
    hasItem = gtk_tree_model_iter_next(modelTree, &iterChild);
  }
  return iterNewItem;
}

/*****************************************************************************/
/* FINDING ITEMS                                                             */
/*****************************************************************************/
static void gtkTreeFindParentNode(GtkTreeModel* modelTree, GtkTreeIter iterItem, GtkTreeIter iterCandidate, GtkTreeIter *iterFoundParent)
{
  GtkTreePath* pathItem = gtk_tree_model_get_path(modelTree, &iterItem);
  GtkTreePath* pathCandidate;
  GtkTreeIter iterChild;
  int hasItem = TRUE;

  while(hasItem)
  {
    pathCandidate = gtk_tree_model_get_path(modelTree, &iterCandidate);

    if (gtk_tree_path_is_descendant(pathItem, pathCandidate))
    {
      *iterFoundParent = iterCandidate;
      gtk_tree_model_iter_children(modelTree, &iterChild, &iterCandidate);  /* get the firstchild */
      gtkTreeFindParentNode(modelTree, iterItem, iterChild, iterFoundParent); /* iterChild is the new candidate */
    }

    gtk_tree_path_free(pathCandidate);

    /* Go to next sibling item */
    hasItem = gtk_tree_model_iter_next(modelTree, &iterCandidate);
  }

  gtk_tree_path_free(pathItem);
}

/* Recursively, find a new brother for the item
that will have its depth changed. Returns the new brother. */
static GtkTreeIter gtkTreeFindNewBrother(Ihandle* ih, GtkTreeModel* modelTree, GtkTreeIter iterBrother)
{
  GtkTreeIter iterChild;
  int hasItem = TRUE;

  while(hasItem)
  {
    if(ih->data->id_control < 0)
      return iterBrother;

    /* Check whether we have child items */
    if(gtk_tree_model_iter_has_child(modelTree, &iterBrother))
    {
      ih->data->id_control--;

      gtk_tree_model_iter_children(modelTree, &iterChild, &iterBrother);  /* get the firstchild */
      iterChild = gtkTreeFindNewBrother(ih, modelTree, iterChild);

      if(ih->data->id_control < 0)
        return iterChild;
    }

    /* Go to next sibling item */
    hasItem = gtk_tree_model_iter_next(modelTree, &iterBrother);
  }

  return iterBrother;
}

static void gtkTreeInvertAllNodeMarking(Ihandle* ih, GtkTreeSelection* selectTree, GtkTreeIter iterItem)
{
  GtkTreeModel* modelTree = gtk_tree_view_get_model(GTK_TREE_VIEW(ih->handle));
  GtkTreeIter iterChild;
  int hasItem = TRUE;

  while(hasItem)
  {
    if(gtk_tree_selection_iter_is_selected(selectTree, &iterItem))
      gtk_tree_selection_unselect_iter(selectTree, &iterItem);
    else
      gtk_tree_selection_select_iter(selectTree, &iterItem);

    /* Check whether we have child items */
    if(gtk_tree_model_iter_has_child(modelTree, &iterItem))
    {
      gtk_tree_model_iter_children(modelTree, &iterChild, &iterItem);  /* get the firstchild */
      gtkTreeInvertAllNodeMarking(ih, selectTree, iterChild);
    }

    /* Go to next sibling item */
    hasItem = gtk_tree_model_iter_next(modelTree, &iterItem);
  }
}

static GtkTreeIter gtkTreeFindCurrentVisibleNode(Ihandle* ih, GtkTreeModel* modelTree, GtkTreeIter iterItem, GtkTreeIter iterNode)
{
  GtkTreeIter iterChild;
  GtkTreePath* path;
  int hasItem = TRUE;

  while(hasItem)
  {
    /* ID control to traverse items */
    ih->data->id_control++;

    /* StateID founded! */
    if(iterItem.user_data == iterNode.user_data)
      return iterItem;

    path = gtk_tree_model_get_path(modelTree, &iterItem);

    /* Check whether we have child items */
    if (gtk_tree_model_iter_has_child(modelTree, &iterItem) && gtk_tree_view_row_expanded(GTK_TREE_VIEW(ih->handle), path))
    {
      gtk_tree_model_iter_children(modelTree, &iterChild, &iterItem);  /* get the firstchild */
      iterChild = gtkTreeFindCurrentVisibleNode(ih, modelTree, iterChild, iterNode);

      /* StateID founded! */
      if(iterChild.user_data == iterNode.user_data)
      {
        gtk_tree_path_free(path);
        return iterChild;
      }
    }

    gtk_tree_path_free(path);

    /* Go to next sibling item */
    hasItem = gtk_tree_model_iter_next(modelTree, &iterItem);
  }

  return iterItem;
}

static GtkTreeIter gtkTreeFindNewVisibleNode(Ihandle* ih, GtkTreeModel* modelTree, GtkTreeIter iterItem)
{
  GtkTreeIter iterChild;
  GtkTreePath* path;
  int hasItem = TRUE;

  while(hasItem)
  {
    /* ID control to traverse items */
    ih->data->id_control--;

    /* StateID founded! */
    if(ih->data->id_control < 0)
      return iterItem;

    path = gtk_tree_model_get_path(modelTree, &iterItem);

    /* Check whether we have child items */
    if(gtk_tree_model_iter_has_child(modelTree, &iterItem) && gtk_tree_view_row_expanded(GTK_TREE_VIEW(ih->handle), path))
    {
      gtk_tree_model_iter_children(modelTree, &iterChild, &iterItem);  /* get the firstchild */
      iterChild = gtkTreeFindNewVisibleNode(ih, modelTree, iterChild);

      /* StateID founded! */
      if(ih->data->id_control < 0)
      {
        gtk_tree_path_free(path);
        return iterChild;
      }
    }

    gtk_tree_path_free(path);

    /* Go to next sibling item */
    hasItem = gtk_tree_model_iter_next(modelTree, &iterItem);
  }

  return iterItem;
}

static GtkTreeIter gtkTreeFindNodeFromID(Ihandle* ih, GtkTreeModel* modelTree, GtkTreeIter iterItem, GtkTreeIter iterNode)
{
  GtkTreeIter iterChild;
  int hasItem = TRUE;

  while(hasItem)
  {
    /* ID control to traverse items */
    ih->data->id_control++;

    /* StateID founded! */
    if(iterItem.user_data == iterNode.user_data)
      return iterItem;

    /* Check whether we have child items */
    if(gtk_tree_model_iter_has_child(modelTree, &iterItem))
    {
      gtk_tree_model_iter_children(modelTree, &iterChild, &iterItem);  /* get the firstchild */
      iterChild = gtkTreeFindNodeFromID(ih, modelTree, iterChild, iterNode);

      /* StateID founded! */
      if(iterChild.user_data == iterNode.user_data)
        return iterChild;
    }

    /* Go to next sibling item */
    hasItem = gtk_tree_model_iter_next(modelTree, &iterItem);
  }

  return iterItem;
}

static GtkTreeIter gtkTreeFindNode(Ihandle* ih, GtkTreeModel* modelTree, GtkTreeIter iterItem)
{
  GtkTreeIter iterChild;
  int hasItem = TRUE;

  while(hasItem)
  {
    /* ID control to traverse items */
    ih->data->id_control--;

    /* StateID founded! */
    if(ih->data->id_control < 0)
      return iterItem;

    /* Check whether we have child items */
    if(gtk_tree_model_iter_has_child(modelTree, &iterItem))
    {
      gtk_tree_model_iter_children(modelTree, &iterChild, &iterItem);  /* get the firstchild */
      iterChild = gtkTreeFindNode(ih, modelTree, iterChild);

      /* StateID founded! */
      if(ih->data->id_control < 0)
        return iterChild;
    }

    /* Go to next sibling item */
    hasItem = gtk_tree_model_iter_next(modelTree, &iterItem);
  }

  return iterItem;
}

static GtkTreeIter gtkTreeFindNodeFromString(Ihandle* ih, const char* id_string)
{
  GtkTreeModel* model = gtk_tree_view_get_model(GTK_TREE_VIEW(ih->handle));
  GtkTreeIter iterItem;
  iterItem.stamp = 0;

  if (id_string[0])
  {
     iupStrToInt(id_string, &ih->data->id_control);
 
     gtk_tree_model_get_iter_first(model, &iterItem);

     return gtkTreeFindNode(ih, model, iterItem);
  }
  else
  {
    GtkTreeSelection* selected = gtk_tree_view_get_selection(GTK_TREE_VIEW(ih->handle));
    gtk_tree_selection_get_selected(selected, &model, &iterItem);
    return iterItem;
  }
}

/*****************************************************************************/
/* MANIPULATING IMAGES                                                       */
/*****************************************************************************/
static void gtkTreeUpdateImages(GtkTreeModel* modelTree, GtkTreeIter iterItem, GdkPixbuf* pixImage, int mode)
{
  GtkTreeIter iterChild;
  int hasItem = TRUE;
  int kind;

  while(hasItem)
  {
    gtk_tree_model_get(modelTree, &iterItem, IUPGTK_TREE_KIND, &kind, -1);

    /* Check whether we have child items */
    if(gtk_tree_model_iter_has_child(modelTree, &iterItem))
    {
      if(mode == IUPGTK_TREE_IMAGE_COLLAPSED)
        gtk_tree_store_set(GTK_TREE_STORE(modelTree), &iterItem, IUPGTK_TREE_IMAGE_COLLAPSED, pixImage, -1);
      else if(mode == IUPGTK_TREE_IMAGE_EXPANDED)
        gtk_tree_store_set(GTK_TREE_STORE(modelTree), &iterItem, IUPGTK_TREE_IMAGE_EXPANDED, pixImage, -1);

      gtk_tree_model_iter_children(modelTree, &iterChild, &iterItem);  /* get the firstchild */

      gtkTreeUpdateImages(modelTree, iterChild, pixImage, mode);
    }
    else if(mode == IUPGTK_TREE_IMAGE_LEAF && kind == ITREE_LEAF)
    {
      gtk_tree_store_set(GTK_TREE_STORE(modelTree), &iterItem, IUPGTK_TREE_IMAGE_LEAF, pixImage, -1);
    }
    else if(kind == ITREE_BRANCH) /* branches has no children */
    {
      if(mode == IUPGTK_TREE_IMAGE_COLLAPSED)
      {
        gtk_tree_store_set(GTK_TREE_STORE(modelTree), &iterItem, IUPGTK_TREE_IMAGE_COLLAPSED, pixImage, -1);
        gtk_tree_store_set(GTK_TREE_STORE(modelTree), &iterItem,      IUPGTK_TREE_IMAGE_LEAF, pixImage, -1);
      }
      else
        gtk_tree_store_set(GTK_TREE_STORE(modelTree), &iterItem, IUPGTK_TREE_IMAGE_EXPANDED, pixImage, -1);
    }

    /* Go to next sibling item */
    hasItem = gtk_tree_model_iter_next(modelTree, &iterItem);
  }
}

int iupgtkGetColor(const char* value, GdkColor *color)
{
  unsigned char r, g, b;
  if (iupStrToRGB(value, &r, &g, &b))
  {
    iupgdkColorSet(color, r, g, b);
    return 1;
  }
  return 0;
}

/*****************************************************************************/
/* ADDING ITEMS                                                              */
/*****************************************************************************/
void iupdrvTreeAddNode(Ihandle* ih, const char* id_string, int kind, const char* title, int add)
{
  GtkTreeStore* store = GTK_TREE_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(ih->handle)));
  GtkTreeIter iterPrev = gtkTreeFindNodeFromString(ih, id_string);
  GtkTreeIter iterNewItem, iterParent;
  GtkTreePath* path;
  GdkColor color = {0L,0,0,0};
  int kindPrev;

  if (!iterPrev.stamp)
    return;

  gtk_tree_model_get(GTK_TREE_MODEL(store), &iterPrev, IUPGTK_TREE_KIND, &kindPrev, -1);

  if (kindPrev == ITREE_BRANCH && add)
    gtk_tree_store_insert(store, &iterNewItem, &iterPrev, 0);  /* iterPrev is parent of the new item (firstchild of it) */
  else
    gtk_tree_store_insert_after(store, &iterNewItem, NULL, &iterPrev);  /* iterPrev is sibling of the new item */

  iupgtkGetColor(iupAttribGetStr(ih, "FGCOLOR"), &color);

  /* set the attributes of the new node */
  if (kind == ITREE_LEAF)
    gtk_tree_store_set(store, &iterNewItem, IUPGTK_TREE_IMAGE_LEAF, iupImageGetImage("IMGLEAF", ih, 0, "TREEIMAGELEAF"),
                                                  IUPGTK_TREE_KIND, ITREE_LEAF,
                                                  IUPGTK_TREE_TITLE, title,
                                                 IUPGTK_TREE_COLOR, &color, -1);
  else
    gtk_tree_store_set(store, &iterNewItem, IUPGTK_TREE_IMAGE_COLLAPSED, iupImageGetImage("IMGCOLLAPSED", ih, 0, "TREEIMAGECOLLAPSED"),
                                                 IUPGTK_TREE_IMAGE_LEAF, iupImageGetImage("IMGCOLLAPSED", ih, 0, "TREEIMAGECOLLAPSED"),
                                             IUPGTK_TREE_IMAGE_EXPANDED, iupImageGetImage("IMGEXPANDED",  ih, 0, "TREEIMAGEEXPANDED"),
                                                       IUPGTK_TREE_KIND, ITREE_BRANCH,
                                                       IUPGTK_TREE_TITLE, title,
                                                      IUPGTK_TREE_COLOR, &color, -1);

  if (kindPrev == ITREE_BRANCH && add)
    iterParent = iterPrev;
  else
    gtk_tree_model_iter_parent(GTK_TREE_MODEL(store), &iterParent, &iterNewItem);

  /* If this is the first child of the parent, then handle the ADDEXPANDED attribute */
  if (gtk_tree_model_iter_n_children(GTK_TREE_MODEL(store), &iterParent) == 1)
  {
    int depth;
    path = gtk_tree_model_get_path(GTK_TREE_MODEL(store), &iterParent);
    depth = gtk_tree_path_get_depth(path)-1;
    if (ih->data->add_expanded || depth==0)  /* if this is the first child of the root, expand always */
      gtk_tree_view_expand_row(GTK_TREE_VIEW(ih->handle), path, FALSE);
    else
      gtk_tree_view_collapse_row(GTK_TREE_VIEW(ih->handle), path);
    gtk_tree_path_free(path);
  }
}

static void gtkTreeAddRootNode(Ihandle* ih)
{
  GtkTreeStore* store = GTK_TREE_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(ih->handle)));
  GtkTreePath* path;
  GtkTreeIter  iterRoot;
  GdkColor color = {0L,0,0,0};

  iupgtkGetColor(iupAttribGetStr(ih, "FGCOLOR"), &color);

  gtk_tree_store_append(store, &iterRoot, NULL);  /* root node */
  gtk_tree_store_set(store, &iterRoot, IUPGTK_TREE_IMAGE_COLLAPSED, iupImageGetImage("IMGCOLLAPSED", ih, 0, "TREEIMAGECOLLAPSED"),
                                            IUPGTK_TREE_IMAGE_LEAF, iupImageGetImage("IMGCOLLAPSED", ih, 0, "TREEIMAGECOLLAPSED"),
                                        IUPGTK_TREE_IMAGE_EXPANDED, iupImageGetImage("IMGEXPANDED",  ih, 0, "TREEIMAGEEXPANDED"),
                                                  IUPGTK_TREE_KIND, ITREE_BRANCH,
                                                 IUPGTK_TREE_COLOR, &color, -1);

  path = gtk_tree_model_get_path(GTK_TREE_MODEL(store), &iterRoot);
  /* Starting node */
  iupAttribSetStr(ih, "_IUPTREE_STARTINGITEM", (char*)path);   // TODO - gtk_tree_path_free(path);

  gtk_tree_view_set_cursor(GTK_TREE_VIEW(ih->handle), path, NULL, FALSE);
}

/*****************************************************************************/
/* AUXILIAR FUNCTIONS                                                        */
/*****************************************************************************/
static void gtkTreeOpenCloseEvent(Ihandle* ih)
{
  GtkTreeModel* model = gtk_tree_view_get_model(GTK_TREE_VIEW(ih->handle));
  GtkTreeIter iter = gtkTreeFindNodeFromString(ih, "");
  GtkTreePath* path;
  int kind;

  if (!iter.stamp)
    return;

  path = gtk_tree_model_get_path(model, &iter);
  if (path)
  {
    gtk_tree_model_get(model, &iter, IUPGTK_TREE_KIND, &kind, -1);

    if(!gtk_tree_model_iter_has_child(model, &iter) && kind == ITREE_LEAF)  /* leafs */
    {
      gtk_tree_view_row_activated(GTK_TREE_VIEW(ih->handle), path, (GtkTreeViewColumn*)iupAttribGet(ih, "_IUPGTK_COLUMN"));     
    }
    else  /* branches -> has and has no child */
    {
      if(gtk_tree_view_row_expanded(GTK_TREE_VIEW(ih->handle), path))
        gtk_tree_view_collapse_row(GTK_TREE_VIEW(ih->handle), path);
      else
        gtk_tree_view_expand_row(GTK_TREE_VIEW(ih->handle), path, FALSE);
    }

    gtk_tree_path_free(path);
  }
}

static GtkTreePath* gtkTreeGetDropPath(GtkTreeView *widget, gint x, gint y)
{
  GtkTreePath *path;
  gint cx, cy;

  gdk_window_get_geometry (gtk_tree_view_get_bin_window (widget), &cx, &cy, NULL, NULL, NULL);

  if (gtk_tree_view_get_path_at_pos (widget, x -= cx, y -= cy, &path, NULL, &cx, &cy))
  {
    GdkRectangle rect;

    /* in lower 1/3 of row? use next row as target */
    gtk_tree_view_get_background_area (widget, path, gtk_tree_view_get_column(widget, 0), &rect);

    if (cy >= rect.height * 2 / 3.0)
    {
      gtk_tree_path_free (path);
      gtk_tree_view_get_path_at_pos (widget, x, y + rect.height, &path, NULL, &cx, &cy);
    }
  }

  return path;
}

static gboolean gtkTreeSelected_Foreach_Func(GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, GList **rowref_list)
{
  GtkTreeRowReference *rowref;

  rowref = gtk_tree_row_reference_new(model, path);
  *rowref_list = g_list_append(*rowref_list, rowref);

  (void)iter;
  return FALSE; /* do not stop walking the store, call us with next row */
}

/*****************************************************************************/
/* CALLBACKS                                                                 */
/*****************************************************************************/
static int gtkTreeMultiSelection_CB(Ihandle* ih)
{
  IFnIi cbMulti = (IFnIi)IupGetCallback(ih, "MULTISELECTION_CB");

  if(cbMulti)
  {
    GtkTreeModel* model = gtk_tree_view_get_model(GTK_TREE_VIEW(ih->handle));
    GtkTreeSelection* selected = gtk_tree_view_get_selection(GTK_TREE_VIEW(ih->handle));
    GtkTreeIter iterRoot;
    GList *rr_list = NULL;
    GList *node;
    int* id_rowItem;
    int count_selected_rows, i = 0;

    gtk_tree_model_get_iter_first(model, &iterRoot);

    gtk_tree_selection_selected_foreach(selected, (GtkTreeSelectionForeachFunc)gtkTreeSelected_Foreach_Func, &rr_list);
    count_selected_rows = g_list_length(rr_list);
    id_rowItem = malloc(sizeof(int) * count_selected_rows);

    for(node = rr_list; node != NULL; node = node->next)
    {
      GtkTreePath* path = gtk_tree_row_reference_get_path(node->data);
      if (path)
      {
        GtkTreeIter iterItem;
        gtk_tree_model_get_iter(model, &iterItem, path);

        ih->data->id_control = -1;
        gtkTreeFindNodeFromID(ih, model, iterRoot, iterItem);
        id_rowItem[i] = ih->data->id_control;
        i++;
      }
      gtk_tree_path_free(path);
    }

    g_list_foreach(rr_list, (GFunc) gtk_tree_row_reference_free, NULL);
    g_list_free(rr_list);

    cbMulti(ih, id_rowItem, count_selected_rows);
    free(id_rowItem);

    return IUP_DEFAULT;
  }

  return IUP_IGNORE;
}

static int gtkTreeRightClick_CB(Ihandle* ih)
{
  IFni cbRightClick  = (IFni)IupGetCallback(ih, "RIGHTCLICK_CB");

  if(cbRightClick)
  {
    cbRightClick(ih, IupGetInt(ih, "VALUE"));
    return IUP_DEFAULT;
  }    

  return IUP_IGNORE;
}

static int gtkTreeShowRename_CB(Ihandle* ih)
{
  IFni cbShowRename = (IFni)IupGetCallback(ih, "SHOWRENAME_CB");

  if(cbShowRename)
  {
    GtkCellRenderer* renderer =   (GtkCellRenderer*)iupAttribGet(ih, "_IUPGTK_RENDERER_TEXT");
    GtkTreeViewColumn* column = (GtkTreeViewColumn*)iupAttribGet(ih, "_IUPGTK_COLUMN");
    GtkTreeModel* model = gtk_tree_view_get_model(GTK_TREE_VIEW(ih->handle));
    GtkTreeIter iter = gtkTreeFindNodeFromString(ih, "");
    GtkTreePath* path;

    if (!iter.stamp)
      return IUP_IGNORE;

    cbShowRename(ih, IupGetInt(ih, "VALUE"));

    path = gtk_tree_model_get_path(model, &iter);
    gtk_tree_view_set_cursor_on_cell(GTK_TREE_VIEW(ih->handle), path, column, renderer, TRUE);
    gtk_tree_path_free(path);

    return IUP_DEFAULT;
  }

  return IUP_IGNORE;
}

static int gtkTreeRenameNode_CB(Ihandle* ih)
{
  IFnis cbRenameNode = (IFnis)IupGetCallback(ih, "RENAMENODE_CB");

  if(cbRenameNode)
  {
    GtkCellRenderer* renderer =   (GtkCellRenderer*)iupAttribGet(ih, "_IUPGTK_RENDERER_TEXT");
    GtkTreeViewColumn* column = (GtkTreeViewColumn*)iupAttribGet(ih, "_IUPGTK_COLUMN");
    GtkTreeModel* model = gtk_tree_view_get_model(GTK_TREE_VIEW(ih->handle));
    GtkTreeIter iter = gtkTreeFindNodeFromString(ih, "");
    GtkTreePath* path;

    if (!iter.stamp)
      return IUP_IGNORE;

    cbRenameNode(ih, IupGetInt(ih, "VALUE"), IupGetAttribute(ih, "NAME"));  

    path = gtk_tree_model_get_path(model, &iter);
    gtk_tree_view_set_cursor_on_cell(GTK_TREE_VIEW(ih->handle), path, column, renderer, TRUE);
    gtk_tree_path_free(path);

    return IUP_DEFAULT;
  }

  return IUP_IGNORE;
}

static int gtkTreeRename_CB(Ihandle* ih, gchar *path_string, gchar* new_text)
{
  IFnis cbRename = (IFnis)IupGetCallback(ih, "RENAME_CB");

  if(new_text)
  {
    GtkTreeModel* model = gtk_tree_view_get_model(GTK_TREE_VIEW(ih->handle));
    GtkTreeIter iter;

    if(cbRename)
      cbRename(ih, IupGetInt(ih, "VALUE"), new_text);

    gtk_tree_model_get_iter_from_string(model, &iter, path_string);
    if (!iter.stamp)
      return IUP_IGNORE;

    gtk_tree_store_set(GTK_TREE_STORE(model), &iter, IUPGTK_TREE_TITLE, new_text, -1);

    return IUP_DEFAULT;
  }

  return IUP_IGNORE;
}


/*****************************************************************************/
/* GET AND SET ATTRIBUTES                                                    */
/*****************************************************************************/


static char* gtkTreeGetIndentationAttrib(Ihandle* ih)
{
  char* str = iupStrGetMemory(255);
  int indent = gtk_tree_view_get_level_indentation(GTK_TREE_VIEW(ih->handle));
  sprintf(str, "%d", indent);
  return str;
}

static int gtkTreeSetIndentationAttrib(Ihandle* ih, const char* value)
{
  int indent;
  if (iupStrToInt(value, &indent))
    gtk_tree_view_set_level_indentation(GTK_TREE_VIEW(ih->handle), indent);
  return 0;
}

static int gtkTreeSetShowDragDropAttrib(Ihandle* ih, const char* value)
{
  if (iupStrBoolean(value))
    gtk_tree_view_set_reorderable(GTK_TREE_VIEW(ih->handle), TRUE);
  else
    gtk_tree_view_set_reorderable(GTK_TREE_VIEW(ih->handle), FALSE);

  return 0;
}

static int gtkTreeSetExpandAllAttrib(Ihandle* ih, const char* value)
{
  if (iupStrBoolean(value))
    gtk_tree_view_expand_all(GTK_TREE_VIEW(ih->handle));
  else
  {
    GtkTreeModel* model = gtk_tree_view_get_model(GTK_TREE_VIEW(ih->handle));
    GtkTreeIter  iterRoot;
    GtkTreePath* pathRoot;

    gtk_tree_view_collapse_all(GTK_TREE_VIEW(ih->handle));

    /* The root node is always expanded */
    gtk_tree_model_get_iter_first(model, &iterRoot);
    pathRoot = gtk_tree_model_get_path(model, &iterRoot);
    gtk_tree_view_expand_row(GTK_TREE_VIEW(ih->handle), pathRoot, FALSE);
    gtk_tree_path_free(pathRoot);
  }

  return 0;
}

static char* gtkTreeGetDepthAttrib(Ihandle* ih, const char* name_id)
{
  char* depth;
  GtkTreeStore* store = GTK_TREE_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(ih->handle)));
  GtkTreeIter iterItem = gtkTreeFindNodeFromString(ih, name_id);
  if (!iterItem.stamp)
    return NULL;

  depth = iupStrGetMemory(10);
  sprintf(depth, "%d", gtk_tree_store_iter_depth(store, &iterItem));
  return depth;
}

static int gtkTreeSetMoveNodeAttrib(Ihandle* ih, const char* name_id, const char* value)
{
  GtkTreeModel* model = gtk_tree_view_get_model(GTK_TREE_VIEW(ih->handle));
  int curDepth, newDepth;
  GtkTreeIter iterBrotherItem, iterRoot;
  GtkTreeIter iterItem = gtkTreeFindNodeFromString(ih, name_id);
  GtkTreeIter iterFoundParent;

  if (!iterItem.stamp)
    return 0;

  curDepth = gtk_tree_store_iter_depth(GTK_TREE_STORE(model), &iterItem);
  iupStrToInt(value, &newDepth);

  /* just the root node has depth = 0 */
  if(newDepth <= 0)
    return 0;

  gtk_tree_model_get_iter_first(model, &iterRoot);

  if(curDepth < newDepth)  /* Top -> Down */
  {
    ih->data->id_control = newDepth - curDepth - 1;  /* subtract 1 (one) to reach the level of its new parent */

    /* Firstly, the node will be child of your brother - this avoids circular reference */
    iterBrotherItem = iterItem;     /* preserve the current iterItem */ 
    gtk_tree_model_iter_next(model, &iterBrotherItem);
    iterBrotherItem = gtkTreeFindNewBrother(ih, model, iterBrotherItem);

    /* Setting the new parent (iterFoundParent) */
    gtkTreeFindParentNode(model, iterBrotherItem, iterRoot, &iterFoundParent);
  }
  else if (curDepth > newDepth)  /* Bottom -> Up */
  {
    /* When the new depth is less than the current depth, 
    simply define a new parent to the node */
    ih->data->id_control = curDepth - newDepth + 1;  /* add 1 (one) to reach the level of its new parent */

    /* Starting the search by the parent of the current node */
    iterFoundParent = iterItem;
    while(ih->data->id_control != 0)
    {
      /* Setting the new parent */
      gtkTreeFindParentNode(model, iterFoundParent, iterRoot, &iterFoundParent);
      ih->data->id_control--;
    }
  }
  else /* same depth, nothing to do */
    return 0;

  /* without parent, nothing to do */
  if(iterFoundParent.user_data == NULL)
    return 0;

  /* Copying the node and its children to the new position */
  gtkTreeCopyBranch(ih, iterItem, iterFoundParent);

  /* Deleting the node of its old position */
  gtk_tree_store_remove(GTK_TREE_STORE(model), &iterItem);

  return 0;
}

static char* gtkTreeGetColorAttrib(Ihandle* ih, const char* name_id)
{
  char* str;
  GtkTreeModel* model = gtk_tree_view_get_model(GTK_TREE_VIEW(ih->handle));
  GtkTreeIter iterItem = gtkTreeFindNodeFromString(ih, name_id);
  GdkColor color = {0L,0,0,0};
  if (!iterItem.stamp)
    return NULL;

  gtk_tree_model_get(model, &iterItem, IUPGTK_TREE_COLOR, &color, -1);

  str = iupStrGetMemory(20);
  sprintf(str, "%d %d %d", iupCOLOR16TO8(color.red),
                           iupCOLOR16TO8(color.green),
                           iupCOLOR16TO8(color.blue));
  return str;
}

static int gtkTreeSetColorAttrib(Ihandle* ih, const char* name_id, const char* value)
{
  GtkTreeStore* store = GTK_TREE_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(ih->handle)));
  GtkTreeIter iterItem = gtkTreeFindNodeFromString(ih, name_id);
  GdkColor color;
  unsigned char r, g, b;

  if (!iterItem.stamp)
    return 0;

  if (!iupStrToRGB(value, &r, &g, &b))
    return 0;

  iupgdkColorSet(&color, r, g, b);
  gtk_tree_store_set(store, &iterItem, IUPGTK_TREE_COLOR, &color, -1);

  return 0;
}

static char* gtkTreeGetParentAttrib(Ihandle* ih, const char* name_id)
{
  GtkTreeModel* model = gtk_tree_view_get_model(GTK_TREE_VIEW(ih->handle));
  GtkTreeIter iterItem = gtkTreeFindNodeFromString(ih, name_id);
  GtkTreeIter iterRoot;
  GtkTreeIter iterParent;
  char* id;

  if (!iterItem.stamp)
    return NULL;

  if (!gtk_tree_model_iter_parent(model, &iterParent, &iterItem))
    return NULL;

  gtk_tree_model_get_iter_first(model, &iterRoot);

  ih->data->id_control = -1;
  gtkTreeFindNodeFromID(ih, model, iterRoot, iterParent);

  id = iupStrGetMemory(10);
  sprintf(id, "%d", ih->data->id_control);
  return id;
}

static char* gtkTreeGetChildCountAttrib(Ihandle* ih, const char* name_id)
{
  GtkTreeModel* model = gtk_tree_view_get_model(GTK_TREE_VIEW(ih->handle));
  GtkTreeIter iterItem = gtkTreeFindNodeFromString(ih, name_id);
  char* str;

  if (!iterItem.stamp)
    return NULL;

  str = iupStrGetMemory(10);
  sprintf(str, "%d", gtk_tree_model_iter_n_children(model, &iterItem));
  return str;
}

static int gtkTreeCount(GtkTreeModel* modelTree, GtkTreeIter iterBranch)
{
  GtkTreeIter iterChild;
  int count = 0;
  int hasItem = gtk_tree_model_iter_children(modelTree, &iterChild, &iterBranch);  /* get the firstchild */
  count++;
  while(hasItem)
  {
    count += gtkTreeCount(modelTree, iterChild);

    /* Go to next sibling item */
    hasItem = gtk_tree_model_iter_next(modelTree, &iterChild);
  }

  return count;
}

static char* gtkTreeGetCountAttrib(Ihandle* ih)
{
  GtkTreeIter iterRoot;
  GtkTreeModel* model = gtk_tree_view_get_model(GTK_TREE_VIEW(ih->handle));
  char* str = iupStrGetMemory(10);
  gtk_tree_model_get_iter_first(model, &iterRoot);
  sprintf(str, "%d", gtkTreeCount(model, iterRoot));
  return str;
}

static char* gtkTreeGetKindAttrib(Ihandle* ih, const char* name_id)
{
  GtkTreeModel* model = gtk_tree_view_get_model(GTK_TREE_VIEW(ih->handle));
  GtkTreeIter iterItem = gtkTreeFindNodeFromString(ih, name_id);
  int kind;

  if (!iterItem.stamp)
    return NULL;

  gtk_tree_model_get(model, &iterItem, IUPGTK_TREE_KIND, &kind, -1);

  if(!kind)
    return "BRANCH";
  else
    return "LEAF";
}

static char* gtkTreeGetStateAttrib(Ihandle* ih, const char* name_id)
{
  GtkTreeModel* model = gtk_tree_view_get_model(GTK_TREE_VIEW(ih->handle));
  GtkTreeIter iterItem = gtkTreeFindNodeFromString(ih, name_id);

  if (!iterItem.stamp)
    return NULL;

  if (gtk_tree_model_iter_has_child(model, &iterItem))
  {
    GtkTreePath* path = gtk_tree_model_get_path(model, &iterItem);
    int expanded = gtk_tree_view_row_expanded(GTK_TREE_VIEW(ih->handle), path);
    gtk_tree_path_free(path);

    if (expanded)
      return "EXPANDED";
    else
      return "COLLAPSED";
  }

  return NULL;
}

static int gtkTreeSetStateAttrib(Ihandle* ih, const char* name_id, const char* value)
{
  GtkTreeModel* model = gtk_tree_view_get_model(GTK_TREE_VIEW(ih->handle));
  GtkTreeIter iterItem = gtkTreeFindNodeFromString(ih, name_id);
  GtkTreePath* path;

  if (!iterItem.stamp)
    return 0;

  path = gtk_tree_model_get_path(model, &iterItem);
  if(iupStrEqualNoCase(value, "COLLAPSED"))
    gtk_tree_view_collapse_row(GTK_TREE_VIEW(ih->handle), path);
  else if(iupStrEqualNoCase(value, "EXPANDED"))
    gtk_tree_view_expand_row(GTK_TREE_VIEW(ih->handle), path, FALSE);

  gtk_tree_path_free(path);

  return 0;
}

static char* gtkTreeGetTitleAttrib(Ihandle* ih, const char* name_id)
{
  GtkTreeModel* model = gtk_tree_view_get_model(GTK_TREE_VIEW(ih->handle));
  GtkTreeIter iterItem = gtkTreeFindNodeFromString(ih, name_id);
  char* title;
  if (!iterItem.stamp)
    return NULL;
  gtk_tree_model_get(model, &iterItem, IUPGTK_TREE_TITLE, &title, -1);
  return title;
}

static int gtkTreeSetTitleAttrib(Ihandle* ih, const char* name_id, const char* value)
{
  GtkTreeStore* store = GTK_TREE_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(ih->handle)));
  GtkTreeIter iterItem = gtkTreeFindNodeFromString(ih, name_id);
  if (!iterItem.stamp)
    return 0;
  gtk_tree_store_set(store, &iterItem, IUPGTK_TREE_TITLE, value, -1);
  return 0;
}

static char* gtkTreeGetUserDataAttrib(Ihandle* ih, const char* name_id)
{
  GtkTreeModel* model = gtk_tree_view_get_model(GTK_TREE_VIEW(ih->handle));
  GtkTreeIter iterItem = gtkTreeFindNodeFromString(ih, name_id);
  char* userdata;
  if (!iterItem.stamp)
    return NULL;
  gtk_tree_model_get(model, &iterItem, IUPGTK_TREE_USERDATA, &userdata, -1);
  return userdata;
}

static int gtkTreeSetUserDataAttrib(Ihandle* ih, const char* name_id, const char* value)
{
  GtkTreeStore* store = GTK_TREE_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(ih->handle)));
  GtkTreeIter iterItem = gtkTreeFindNodeFromString(ih, name_id);
  if (!iterItem.stamp)
    return 0;
  gtk_tree_store_set(store, &iterItem, IUPGTK_TREE_USERDATA, value, -1);
  return 0;
}

static char* gtkTreeGetValueAttrib(Ihandle* ih)
{
  GtkTreeModel* model = gtk_tree_view_get_model(GTK_TREE_VIEW(ih->handle));
  GtkTreeSelection* selected = gtk_tree_view_get_selection(GTK_TREE_VIEW(ih->handle));
  GtkTreeIter iterRoot, iterSelected;
  char* id = iupStrGetMemory(16);

  gtk_tree_model_get_iter_first(model, &iterRoot);

  if(!gtk_tree_selection_get_selected(selected, &model, &iterSelected))
    return 0;

  ih->data->id_control = -1;
  gtkTreeFindNodeFromID(ih, model, iterRoot, iterSelected);
  sprintf(id, "%d", ih->data->id_control);

  return id;
}

/* Changes the selected node, starting from current branch
- value: "ROOT", "LAST", "NEXT", "PREVIOUS", "INVERT",
"BLOCK", "CLEARALL", "MARKALL", "INVERTALL"
or id of the node that will be the current  */
static int gtkTreeSetValueAttrib(Ihandle* ih, const char* value)
{
  GtkTreeModel* model = gtk_tree_view_get_model(GTK_TREE_VIEW(ih->handle));
  GtkTreeSelection* selected = gtk_tree_view_get_selection(GTK_TREE_VIEW(ih->handle));
  GtkTreeIter iterRoot, iterItem;
  GtkTreePath* path;

  gtk_tree_model_get_iter_first(model, &iterRoot);

  if(iupStrEqualNoCase(value, "ROOT"))
  {
    gtk_tree_selection_set_mode(selected, GTK_SELECTION_SINGLE);
    gtk_tree_selection_select_iter(selected, &iterRoot);
  }
  else if(iupStrEqualNoCase(value, "LAST"))
  {
    GtkTreeIter iterLast;
    gtk_tree_model_iter_children(model, &iterLast, &iterRoot);

    /* iterLast and iterRoot will be never compared (so, function always return the last visible node)
    This was done because GTK does not accepted NULL iterators */
    iterLast = gtkTreeFindNodeFromID(ih, model, iterLast, iterRoot);

    /* set the new selection */
    gtk_tree_selection_set_mode(selected, GTK_SELECTION_SINGLE);
    gtk_tree_selection_select_iter(selected, &iterLast);
  }
  else if(iupStrEqualNoCase(value, "PGUP"))
  {
    GtkTreeIter iterPrev;
    GtkTreePath* pathPrev;

    /* looking for the previous visible node */
    gtk_tree_selection_get_selected(selected, &model, &iterPrev);

    ih->data->id_control = -1;
    gtkTreeFindCurrentVisibleNode(ih, model, iterRoot, iterPrev);

    ih->data->id_control -= 10;  /* Up 10 lines */

    if(ih->data->id_control < 0)
      ih->data->id_control = 0;

    iterPrev = gtkTreeFindNewVisibleNode(ih, model, iterRoot);
    pathPrev = gtk_tree_model_get_path(model, &iterPrev);

    gtk_tree_selection_set_mode(selected, GTK_SELECTION_SINGLE);
    gtk_tree_selection_select_path(selected, pathPrev);
    gtk_tree_path_free(pathPrev);
  }
  else if(iupStrEqualNoCase(value, "PGDN"))
  {
    GtkTreeIter iterNext;
    GtkTreePath* pathNext;

    /* looking for the next visible node */
    gtk_tree_selection_get_selected(selected, &model, &iterNext);

    ih->data->id_control = -1;
    gtkTreeFindCurrentVisibleNode(ih, model, iterRoot, iterNext);

    ih->data->id_control += 10;  /* Down 10 lines */

    iterNext = gtkTreeFindNewVisibleNode(ih, model, iterRoot);
    pathNext = gtk_tree_model_get_path(model, &iterNext);

    gtk_tree_selection_set_mode(selected, GTK_SELECTION_SINGLE);
    gtk_tree_selection_select_path(selected, pathNext);
    gtk_tree_path_free(pathNext);
  }
  else if(iupStrEqualNoCase(value, "NEXT"))
  {
    GtkTreeIter iterNext;
    GtkTreePath* pathNext;

    /* looking for the next visible node */
    gtk_tree_selection_get_selected(selected, &model, &iterNext);

    ih->data->id_control = -1;
    gtkTreeFindCurrentVisibleNode(ih, model, iterRoot, iterNext);
    ih->data->id_control++;

    iterNext = gtkTreeFindNewVisibleNode(ih, model, iterRoot);
    pathNext = gtk_tree_model_get_path(model, &iterNext);

    gtk_tree_selection_set_mode(selected, GTK_SELECTION_SINGLE);
    gtk_tree_selection_select_path(selected, pathNext);
    gtk_tree_path_free(pathNext);
  }
  else if(iupStrEqualNoCase(value, "PREVIOUS"))
  {
    GtkTreeIter iterPrev;
    GtkTreePath* pathPrev;

    /* looking for the previous visible node */
    gtk_tree_selection_get_selected(selected, &model, &iterPrev);

    ih->data->id_control = -1;
    gtkTreeFindCurrentVisibleNode(ih, model, iterRoot, iterPrev);
    ih->data->id_control--;

    if(ih->data->id_control < 0)
      ih->data->id_control = 0;

    iterPrev = gtkTreeFindNewVisibleNode(ih, model, iterRoot);
    pathPrev = gtk_tree_model_get_path(model, &iterPrev);

    gtk_tree_selection_set_mode(selected, GTK_SELECTION_SINGLE);
    gtk_tree_selection_select_path(selected, pathPrev);
    gtk_tree_path_free(pathPrev);
  }
  else if(iupStrEqualNoCase(value, "BLOCK"))
  {
    GtkTreeIter iterSelected, iterStarting;
    GtkTreePath* pathSelected;

    gtk_tree_selection_get_selected(selected, &model, &iterSelected);
    pathSelected = gtk_tree_model_get_path(model, &iterSelected);
    gtk_tree_model_get_iter(model, &iterStarting, (GtkTreePath*)iupAttribGet(ih, "_IUPTREE_STARTINGITEM"));

    gtk_tree_selection_set_mode(selected, GTK_SELECTION_MULTIPLE);
    gtk_tree_selection_select_range(selected, (GtkTreePath*)iupAttribGet(ih, "_IUPTREE_STARTINGITEM"), pathSelected);
  }
  else if(iupStrEqualNoCase(value, "CLEARALL"))
  {
    gtk_tree_selection_unselect_all(selected);
    gtk_tree_selection_set_mode(selected, GTK_SELECTION_SINGLE);
  }
  else if(iupStrEqualNoCase(value, "MARKALL"))
  {
    GtkTreeIter iterLast;

    gtk_tree_model_iter_children(model, &iterLast, &iterRoot);
    iterLast = gtkTreeFindNodeFromID(ih, model, iterLast, iterRoot);

    gtk_tree_selection_set_mode(selected, GTK_SELECTION_MULTIPLE);
    gtk_tree_selection_select_all(selected);
  }
  else if(iupStrEqualNoCase(value, "INVERTALL"))
  {
    /* INVERTALL *MUST* appear before INVERT, or else INVERTALL will never be called. */
    gtk_tree_selection_set_mode(selected, GTK_SELECTION_MULTIPLE);
    gtkTreeInvertAllNodeMarking(ih, selected, iterRoot);
  }
  else if(iupStrEqualPartial(value, "INVERT"))
  {
    /* iupStrEqualPartial allows the use of "INVERTid" form */
    GtkTreeIter iterSelected = gtkTreeFindNodeFromString(ih, &value[strlen("INVERT")]);
    GtkTreeSelection* selected = gtk_tree_view_get_selection(GTK_TREE_VIEW(ih->handle));

    if (!iterSelected.stamp)
      return 0;

    gtk_tree_selection_set_mode(selected, GTK_SELECTION_SINGLE);

    if(gtk_tree_selection_iter_is_selected(selected, &iterSelected))
      gtk_tree_selection_unselect_iter(selected, &iterSelected);
    else
      gtk_tree_selection_select_iter(selected, &iterSelected);
  }
  else
  {
    GtkTreeIter iterNewSel = gtkTreeFindNodeFromString(ih, value);
    if (!iterNewSel.stamp)
      return 0;

    gtk_tree_selection_set_mode(selected, GTK_SELECTION_SINGLE);
    gtk_tree_selection_select_iter(selected, &iterNewSel);
  }

  /* get the selected item updated */
  gtk_tree_selection_get_selected(selected, &model, &iterItem);
  path = gtk_tree_model_get_path(model, &iterItem);

  gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(ih->handle), path, NULL, FALSE, 0, 0);
  gtk_tree_view_set_cursor(GTK_TREE_VIEW(ih->handle), path, NULL, FALSE);
  gtk_tree_path_free(path);

  iupAttribSetInt(ih, "_IUPTREE_OLDVALUE", IupGetInt(ih, "VALUE"));

  return 0;
} 

static char* gtkTreeGetStartingAttrib(Ihandle* ih)
{
  GtkTreeModel* model = gtk_tree_view_get_model(GTK_TREE_VIEW(ih->handle));
  GtkTreePath* pathStarting  = (GtkTreePath*)iupAttribGet(ih, "_IUPTREE_STARTINGITEM");
  GtkTreeIter  iterRoot, iterStarting;
  char* id = iupStrGetMemory(10);

  gtk_tree_model_get_iter_first(model, &iterRoot);
  gtk_tree_model_get_iter(model, &iterStarting, pathStarting);

  ih->data->id_control = -1;
  gtkTreeFindNodeFromID(ih, model, iterRoot, iterStarting);
  sprintf(id, "%d", ih->data->id_control);

  return id;
}

static int gtkTreeSetStartingAttrib(Ihandle* ih, const char* name_id)
{
  GtkTreePath* pathStarting;
  GtkTreeModel* model = gtk_tree_view_get_model(GTK_TREE_VIEW(ih->handle));
  GtkTreeSelection* selected = gtk_tree_view_get_selection(GTK_TREE_VIEW(ih->handle));
  GtkTreeIter iterStarting = gtkTreeFindNodeFromString(ih, name_id);
  if (!iterStarting.stamp)
    return 0;

  pathStarting  = gtk_tree_model_get_path(model, &iterStarting);
  iupAttribSetStr(ih, "_IUPTREE_STARTINGITEM", (char*)pathStarting);

  gtk_tree_selection_unselect_all(selected);
  gtk_tree_selection_select_path(selected, pathStarting);
  gtk_tree_view_set_cursor(GTK_TREE_VIEW(ih->handle), pathStarting, NULL, FALSE);

  return 0;
}

static char* gtkTreeGetMarkedAttrib(Ihandle* ih, const char* name_id)
{
  GtkTreeSelection* selected = gtk_tree_view_get_selection(GTK_TREE_VIEW(ih->handle));
  GtkTreeIter iterItem = gtkTreeFindNodeFromString(ih, name_id);
  if (!iterItem.stamp)
    return 0;

  if(gtk_tree_selection_iter_is_selected(selected, &iterItem))
    return "YES";
  else
    return "NO";
}

static int gtkTreeSetMarkedAttrib(Ihandle* ih, const char* name_id, const char* value)
{
  GtkTreeSelection* selected = gtk_tree_view_get_selection(GTK_TREE_VIEW(ih->handle));
  GtkTreeIter iterItem = gtkTreeFindNodeFromString(ih, name_id);
  if (!iterItem.stamp)
    return 0;

  gtk_tree_selection_set_mode(selected, GTK_SELECTION_MULTIPLE);

  if (iupStrBoolean(value))
    gtk_tree_selection_select_iter(selected, &iterItem);
  else
    gtk_tree_selection_unselect_iter(selected, &iterItem);

  return 0;
}

static int gtkTreeSetDelNodeAttrib(Ihandle* ih, const char* name_id, const char* value)
{
  if(iupStrEqualNoCase(value, "SELECTED"))  /* selectec here means the specified one */
  {
    GtkTreeModel* model = gtk_tree_view_get_model(GTK_TREE_VIEW(ih->handle));
    GtkTreeIter   iterItem = gtkTreeFindNodeFromString(ih, name_id);
    GtkTreeIter   iterRoot;
    GtkTreePath*  pathRoot, *pathItem;
    GtkTreeSelection* selected = gtk_tree_view_get_selection(GTK_TREE_VIEW(ih->handle));
    int equal;

    if (!iterItem.stamp)
      return 0;

    gtk_tree_model_get_iter_first(model, &iterRoot);
    pathRoot = gtk_tree_model_get_path(model, &iterRoot);
    pathItem = gtk_tree_model_get_path(model, &iterItem);

    gtk_tree_selection_set_mode(selected, GTK_SELECTION_SINGLE);

    equal = gtk_tree_path_compare(pathItem, pathRoot) == 0;
    gtk_tree_path_free(pathRoot);
    gtk_tree_path_free(pathItem);

    /* the root node can't be deleted */
    if (!equal)
      return 0;

    /* deleting the specified node (and it's children) */
    gtk_tree_store_remove(GTK_TREE_STORE(model), &iterItem);
  }
  else if(iupStrEqualNoCase(value, "CHILDREN"))  /* children of the specified one */
  {
    GtkTreeModel* model = gtk_tree_view_get_model(GTK_TREE_VIEW(ih->handle));
    GtkTreeIter   iterItem = gtkTreeFindNodeFromString(ih, name_id);
    GtkTreeIter   iterChild, iterRoot;
    GtkTreePath*  pathRoot, *pathItem;
//    GtkTreeSelection* selected = gtk_tree_view_get_selection(GTK_TREE_VIEW(ih->handle));
    int hasChildren, equal;

    if (!iterItem.stamp)
      return 0;

    gtk_tree_model_get_iter_first(model, &iterRoot);
    pathRoot = gtk_tree_model_get_path(model, &iterRoot);
    pathItem = gtk_tree_model_get_path(model, &iterItem);

//    gtk_tree_selection_set_mode(selected, GTK_SELECTION_SINGLE);

    equal = gtk_tree_path_compare(pathItem, pathRoot) == 0;
    gtk_tree_path_free(pathRoot);
    gtk_tree_path_free(pathItem);

    /* the root node can't be deleted */
    if (!equal)
      return 0;

    hasChildren = gtk_tree_model_iter_children(model, &iterChild, &iterItem);

    /* deleting the selected node's children */
    while(hasChildren)
      hasChildren = gtk_tree_store_remove(GTK_TREE_STORE(model), &iterChild);
  }
  else if(iupStrEqualNoCase(value, "MARKED"))  /* Delete the array of marked nodes */
  {
    GtkTreeModel* model = gtk_tree_view_get_model(GTK_TREE_VIEW(ih->handle));
    GtkTreeSelection* selected = gtk_tree_view_get_selection(GTK_TREE_VIEW(ih->handle));
    GtkTreeIter iterRoot;
    GtkTreePath* pathRoot;
    GList *rr_list = NULL;
    GList *node;

    gtk_tree_selection_selected_foreach(selected, (GtkTreeSelectionForeachFunc)gtkTreeSelected_Foreach_Func, &rr_list);
    gtk_tree_model_get_iter_first(model, &iterRoot);
    pathRoot = gtk_tree_model_get_path(model, &iterRoot);

    for(node = rr_list; node != NULL; node = node->next)
    {
      GtkTreePath* path = gtk_tree_row_reference_get_path(node->data);
      if (path)
      {
        GtkTreeIter iter;
        if (gtk_tree_model_get_iter(model, &iter, path) && (gtk_tree_path_compare(path, pathRoot) != 0))
        {
          gtk_tree_store_remove(GTK_TREE_STORE(model), &iter);
        }
        gtk_tree_path_free(path);
      }
    }
    g_list_foreach(rr_list, (GFunc) gtk_tree_row_reference_free, NULL);
    g_list_free(rr_list);

    gtk_tree_path_free(pathRoot);

    gtk_tree_selection_set_mode(selected, GTK_SELECTION_SINGLE);
  }

  return 0;
}

static int gtkTreeSetRenameAttrib(Ihandle* ih, const char* name_id, const char* value)
{  
  if(IupGetInt(ih, "SHOWRENAME"))
    ;//iupdrvTreeSetTitleAttrib(ih, name_id, value);
  else
    gtkTreeRenameNode_CB(ih);

  return 1;
}

static int gtkTreeSetImageExpandedAttrib(Ihandle* ih, const char* name_id, const char* value)
{
  GtkTreeStore*  store = GTK_TREE_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(ih->handle)));
  GdkPixbuf* pixExpand = iupImageGetImage(value, ih, 0, "TREEIMAGE");
  GtkTreeIter iterItem = gtkTreeFindNodeFromString(ih, name_id);
  if (!iterItem.stamp)
    return 0;

  gtk_tree_store_set(store, &iterItem, IUPGTK_TREE_IMAGE_EXPANDED, pixExpand, -1);

  return 1;
}

static int gtkTreeSetImageAttrib(Ihandle* ih, const char* name_id, const char* value)
{
  GtkTreeStore*  store = GTK_TREE_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(ih->handle)));
  GdkPixbuf*  pixImage = iupImageGetImage(value, ih, 0, "TREEIMAGE");
  GtkTreeIter iterItem = gtkTreeFindNodeFromString(ih, name_id);
  if (!iterItem.stamp)
    return 0;

  gtk_tree_store_set(store, &iterItem, IUPGTK_TREE_IMAGE_COLLAPSED, pixImage, -1);
  gtk_tree_store_set(store, &iterItem, IUPGTK_TREE_IMAGE_LEAF, pixImage, -1);

  return 1;
}

static int gtkTreeSetImageBranchExpandedAttrib(Ihandle* ih, const char* value)
{
  GtkTreeModel*  model = gtk_tree_view_get_model(GTK_TREE_VIEW(ih->handle));
  GdkPixbuf* pixExpand = iupImageGetImage(value, ih, 0, "TREEIMAGEEXPANDED");
  GtkTreeIter iterRoot;
  int mode = IUPGTK_TREE_IMAGE_EXPANDED;

  gtk_tree_model_get_iter_first(model, &iterRoot);

  /* Update all images, starting at root node */
  gtkTreeUpdateImages(model, iterRoot, pixExpand, mode);

  return 1;
}

static int gtkTreeSetImageBranchCollapsedAttrib(Ihandle* ih, const char* value)
{
  GtkTreeModel*  model = gtk_tree_view_get_model(GTK_TREE_VIEW(ih->handle));
  GdkPixbuf* pixCollap = iupImageGetImage(value, ih, 0, "TREEIMAGECOLLAPSED");
  GtkTreeIter iterRoot;
  int mode = IUPGTK_TREE_IMAGE_COLLAPSED;

  gtk_tree_model_get_iter_first(model, &iterRoot);

  /* Update all images, starting at root node */
  gtkTreeUpdateImages(model, iterRoot, pixCollap, mode);

  return 1;
}

static int gtkTreeSetImageLeafAttrib(Ihandle* ih, const char* value)
{
  GtkTreeModel* model = gtk_tree_view_get_model(GTK_TREE_VIEW(ih->handle));
  GdkPixbuf*  pixLeaf = iupImageGetImage(value, ih, 0, "TREEIMAGELEAF");
  GtkTreeIter iterRoot;
  int mode = IUPGTK_TREE_IMAGE_LEAF;

  gtk_tree_model_get_iter_first(model, &iterRoot);

  /* Update all images, starting at root node */
  gtkTreeUpdateImages(model, iterRoot, pixLeaf, mode);

  return 1;
}

static int gtkTreeSetBgColorAttrib(Ihandle* ih, const char* value)
{
  unsigned char r, g, b;

  GtkScrolledWindow* scrolled_window = (GtkScrolledWindow*)iupAttribGet(ih, "_IUP_EXTRAPARENT");
  if (scrolled_window)
  {
    /* ignore given value, must use only from parent for the scrollbars */
    char* parent_value = iupBaseNativeParentGetBgColor(ih);

    if (iupStrToRGB(parent_value, &r, &g, &b))
    {
      GtkWidget* sb;

      if (!GTK_IS_SCROLLED_WINDOW(scrolled_window))
        scrolled_window = (GtkScrolledWindow*)iupAttribGet(ih, "_IUPGTK_SCROLLED_WINDOW");

      iupgtkBaseSetBgColor((GtkWidget*)scrolled_window, r, g, b);

#if GTK_CHECK_VERSION(2, 8, 0)
      sb = gtk_scrolled_window_get_hscrollbar(scrolled_window);
      if (sb) iupgtkBaseSetBgColor(sb, r, g, b);

      sb = gtk_scrolled_window_get_vscrollbar(scrolled_window);
      if (sb) iupgtkBaseSetBgColor(sb, r, g, b);
#endif
    }
  }

  if (!iupStrToRGB(value, &r, &g, &b))
    return 0;

  {
    GtkCellRenderer* renderer_txt = (GtkCellRenderer*)iupAttribGet(ih, "_IUPGTK_RENDERER_TEXT");
    GtkCellRenderer* renderer_img = (GtkCellRenderer*)iupAttribGet(ih, "_IUPGTK_RENDERER_IMG");
    GdkColor color;
    iupgdkColorSet(&color, r, g, b);
    g_object_set(G_OBJECT(renderer_txt), "cell-background-gdk", &color, NULL);
    g_object_set(G_OBJECT(renderer_img), "cell-background-gdk", &color, NULL);
  }

  iupdrvBaseSetBgColorAttrib(ih, value);   /* use given value for contents */

  /* no need to update internal image cache in GTK */

  return 1;
}

static int gtkTreeSetFgColorAttrib(Ihandle* ih, const char* value)
{
  unsigned char r, g, b;
  if (!iupStrToRGB(value, &r, &g, &b))
    return 0;

  iupgtkBaseSetFgColor(ih->handle, r, g, b);

  {
    GtkCellRenderer* renderer_txt = (GtkCellRenderer*)iupAttribGet(ih, "_IUPGTK_RENDERER_TEXT");
    GdkColor color;
    iupgdkColorSet(&color, r, g, b);
    g_object_set(G_OBJECT(renderer_txt), "foreground-gdk", &color, NULL);
    g_object_get(G_OBJECT(renderer_txt), "foreground-gdk", &color, NULL);
    color.blue = 0;
  }

  return 1;
}


/***********************************************************************************************/


static int gtkTreeSetRenameCaretPos(Ihandle* ih)
{
  GtkCellEditable* editable = (GtkCellEditable*)iupAttribGet(ih, "_IUPTREE_EDITNAME");
  int pos = 1;

  sscanf(IupGetAttribute(ih, "RENAMECARET"), "%i", &pos);
  if (pos < 1) pos = 1;
  pos--; /* IUP starts at 1 */

  gtk_editable_set_position(GTK_EDITABLE(editable), pos);

  return 1;
}

static int gtkTreeSetRenameSelectionPos(Ihandle* ih)
{
  GtkCellEditable* editable = (GtkCellEditable*)iupAttribGet(ih, "_IUPTREE_EDITNAME");
  int start = 1, end = 1;

  if (iupStrToIntInt(IupGetAttribute(ih, "RENAMESELECTION"), &start, &end, ':') != 2) 
    return 0;

  if(start < 1 || end < 1) 
    return 0;

  start--; /* IUP starts at 1 */
  end--;

  gtk_editable_select_region(GTK_EDITABLE(editable), start, end);

  return 1;
}

/*****************************************************************************/
/* SIGNALS                                                                   */
/*****************************************************************************/
static void gtkTreeCellTextEditingStarted(GtkCellRenderer *cell, GtkCellEditable *editable, const gchar *path, Ihandle *ih)
{
  iupAttribSetStr(ih, "_IUPTREE_EDITNAME", (char*)editable);
  
  (void)cell;
  (void)path;
}

void gtkTreeCellTextEdited(GtkCellRendererText *cell, gchar *path_string, gchar *new_text, Ihandle* ih)
{
  gtkTreeRename_CB(ih, path_string, new_text);
  (void)cell;
}

static int gtkTreeDragDrop_CB(Ihandle* ih)
{
  IFniiii cbDragDrop = (IFniiii)IupGetCallback(ih, "DRAGDROP_CB");
  int drag_str = iupAttribGetInt(ih, "_IUPTREE_DRAGID");
  int drop_str = iupAttribGetInt(ih, "_IUPTREE_DROPID");
  int   isshift_str = iupAttribGetInt(ih, "_IUPGTKTREE_ISSHIFT");
  int iscontrol_str = iupAttribGetInt(ih, "_IUPGTKTREE_ISCONTROL");

  if(cbDragDrop)
  {
    cbDragDrop(ih, drag_str, drop_str, isshift_str, iscontrol_str);
    return IUP_DEFAULT;
  }

  return IUP_IGNORE;
}

static void gtkTreeDragEnd(GtkWidget *widget, GdkDragContext *drag_context, Ihandle* ih)
{
  if(((GtkTreePath*)iupAttribGet(ih, "_IUPTREE_DRAGITEM") != NULL) &&
     ((GtkTreePath*)iupAttribGet(ih, "_IUPTREE_DROPITEM") != NULL))
  {
    int kind;
    GtkTreeIter iterDrag, iterDrop;
    GtkTreeModel* model = gtk_tree_view_get_model(GTK_TREE_VIEW(ih->handle));

    gtk_tree_model_get_iter(model, &iterDrag, (GtkTreePath*)iupAttribGet(ih, "_IUPTREE_DRAGITEM"));
    gtk_tree_model_get_iter(model, &iterDrop, (GtkTreePath*)iupAttribGet(ih, "_IUPTREE_DROPITEM"));

    /* Only process if the new parent is a branch */
    gtk_tree_model_get(model, &iterDrop, IUPGTK_TREE_KIND, &kind, -1);

    if(kind == ITREE_BRANCH)
    {
      GtkTreePath *pathNew;
      GtkTreeIter  iterNew;
      GtkTreeSelection* selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(ih->handle));

      /* Copy the dragged item to the new position. After, remove it */
      iterNew = gtkTreeCopyBranch(ih, iterDrag, iterDrop);
      gtk_tree_store_remove(GTK_TREE_STORE(model), &iterDrag);

      /* DragDrop Callback */
      gtkTreeDragDrop_CB(ih);

      /* expand the new parent and set the item dropped as the new item selected */
      gtk_tree_view_expand_row(GTK_TREE_VIEW(ih->handle), (GtkTreePath*)iupAttribGet(ih, "_IUPTREE_DROPITEM"), FALSE);

      pathNew = gtk_tree_model_get_path(model, &iterNew);
      gtk_tree_selection_select_path(selection, pathNew);

      gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(ih->handle), pathNew, NULL, FALSE, 0, 0);
      gtk_tree_view_set_cursor(GTK_TREE_VIEW(ih->handle), pathNew, NULL, FALSE);

      gtk_tree_path_free(pathNew);
    }
  }

  gtk_tree_path_free((GtkTreePath*)iupAttribGet(ih, "_IUPTREE_DRAGITEM"));
  gtk_tree_path_free((GtkTreePath*)iupAttribGet(ih, "_IUPTREE_DROPITEM"));

  iupAttribSetStr(ih, "_IUPTREE_DRAGITEM", NULL);
  iupAttribSetStr(ih, "_IUPTREE_DROPITEM", NULL);

  (void)widget;
  (void)drag_context;
}

static gboolean gtkTreeDragDrop(GtkTreeView *widget, GdkDragContext *drag_context, gint x, gint y, guint time, Ihandle* ih)
{
  GtkTreeModel    *model = gtk_tree_view_get_model(GTK_TREE_VIEW(ih->handle));
  GtkTreePath  *pathDrop = gtkTreeGetDropPath(widget, x, y);
  GtkTreeIter   iterRoot;

  gtk_tree_model_get_iter_first(model, &iterRoot);

  /* pathDrop is valid and the source is different to the destination */
  if(pathDrop && gtk_tree_path_compare(pathDrop, (GtkTreePath*)iupAttribGet(ih, "_IUPTREE_DRAGITEM")) != 0)
  {
    GtkTreeIter iterItem;
    gtk_tree_model_get_iter(model, &iterItem, pathDrop);

    ih->data->id_control = -1;
    gtkTreeFindNodeFromID(ih, model, iterRoot, iterItem);
    iupAttribSetInt(ih, "_IUPTREE_DROPID", ih->data->id_control);
  }
  else
  {
    GtkTreePath* pathRoot = gtk_tree_model_get_path(model, &iterRoot);

    /* Drag destination is root - drop item is null */
    if(gtk_tree_path_compare(pathRoot, (GtkTreePath*)iupAttribGet(ih, "_IUPTREE_DRAGITEM")) == 0)
    {
      gtk_tree_path_free(pathRoot);
      iupAttribSetStr(ih, "_IUPTREE_DROPITEM", NULL);
      gtk_drag_finish(drag_context, FALSE, FALSE, time);

      return TRUE;
    }
    else  /* Drop destination is inside of the treeview area, but it is not child of Root */
    {
      pathDrop = pathRoot;
      iupAttribSetInt(ih, "_IUPTREE_DROPID", 0);
    }          
  }

  iupAttribSetStr(ih, "_IUPTREE_DROPITEM", (char*)pathDrop);
  gtk_drag_finish(drag_context, TRUE, FALSE, time);

  return TRUE;
}

static void gtkTreeDragBegin(GtkWidget *widget, GdkDragContext *drag_context, Ihandle* ih)
{
  GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(ih->handle));
  GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(ih->handle));
  GtkTreeIter iterItem, iterRoot;

  gtk_tree_model_get_iter_first(model, &iterRoot);

  if (gtk_tree_selection_get_selected (selection, NULL, &iterItem))
  {
    GtkTreePath* pathDrag = gtk_tree_model_get_path(model, &iterItem);
    iupAttribSetStr(ih, "_IUPTREE_DRAGITEM", (char*)pathDrag);

    ih->data->id_control = -1;
    gtkTreeFindNodeFromID(ih, model, iterRoot, iterItem);
    iupAttribSetInt(ih, "_IUPTREE_DRAGID", ih->data->id_control);
  }

  (void)drag_context;
  (void)widget;
}

static int gtkTreeSelection_CB(Ihandle* ih)
{
  IFnii cbSelec = (IFnii)IupGetCallback(ih, "SELECTION_CB");
  int oldpos = iupAttribGetInt(ih, "_IUPTREE_OLDVALUE");
  int curpos = IupGetInt(ih, "VALUE");

  if(cbSelec)
  {
    if(oldpos != curpos)
    {
      cbSelec(ih, oldpos, 0);  /* unselected */
      cbSelec(ih, curpos, 1);  /*   selected */

      iupAttribSetInt(ih, "_IUPTREE_OLDVALUE", curpos);

      return IUP_DEFAULT;
    }
  }

  return IUP_IGNORE;
}

static void gtkTreeSelectionChanged(GtkTreeSelection* selection, Ihandle* ih)
{
  GtkTreeIter iter;
  GtkTreeModel* tree_model;

  if(gtk_tree_selection_get_selected(selection, &tree_model, &iter))
  {
    gtkTreeSelection_CB(ih);
  }
}

static void gtkTreeRowExpanded(GtkTreeView* tree_view, GtkTreeIter *iter, GtkTreePath *path, Ihandle* ih)
{
  /* BRANCHOPEN_CB callback */
  IFni cbBranchOpen  = (IFni)IupGetCallback(ih, "BRANCHOPEN_CB");
  cbBranchOpen(ih, IupGetInt(ih, "VALUE"));

  (void)path;
  (void)iter;
  (void)tree_view;
}

static void gtkTreeRowCollapsed(GtkTreeView* tree_view, GtkTreeIter *iter, GtkTreePath *path, Ihandle* ih)
{
  /* BRANCHCLOSE_CB callback */
  IFni cbBranchClose  = (IFni)IupGetCallback(ih, "BRANCHCLOSE_CB");
  cbBranchClose(ih, IupGetInt(ih, "VALUE"));

  (void)path;
  (void)iter;
  (void)tree_view;
}

static void gtkTreeRowActived(GtkTreeView* tree_view, GtkTreePath *path, GtkTreeViewColumn *column, Ihandle* ih)
{
  /* EXECUTELEAF_CB callback */
  IFni cbExecuteLeaf  = (IFni)IupGetCallback(ih, "EXECUTELEAF_CB");
  GtkTreeIter iter;
  GtkTreeModel* model = gtk_tree_view_get_model(GTK_TREE_VIEW(ih->handle));
  int kind;  /* used for nodes defined as branches, but do not have children */

  gtk_tree_model_get_iter(model, &iter, path);
  gtk_tree_model_get(model, &iter, IUPGTK_TREE_KIND, &kind, -1);

  /* just to leaf nodes */
  if(gtk_tree_model_iter_has_child(model, &iter) == 0 && kind == ITREE_LEAF)
    cbExecuteLeaf(ih, IupGetInt(ih, "VALUE"));

  (void)column;
  (void)tree_view;
}

static gboolean gtkTreeButtonPressEvent(GtkWidget *treeview, GdkEventButton *event, Ihandle* ih)
{
  /* single click with the right mouse button */
  if (event->type == GDK_BUTTON_PRESS && event->button == 3)
  {
    /* select row if no row is selected or only one other row is selected */
    GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));
    GtkTreePath *path;

    /* Get tree path for row that was clicked */
    if (gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(treeview), (gint) event->x, (gint) event->y, &path, NULL, NULL, NULL))
    {
      gtk_tree_selection_unselect_all(selection);
      gtk_tree_selection_select_path(selection, path);
      gtk_tree_path_free(path);
    }

    gtkTreeRightClick_CB(ih);

    return TRUE;
  }
  else if (event->type == GDK_2BUTTON_PRESS && event->button == 1)
  {
    GtkTreeViewColumn* column;
    GtkTreePath *path;

    if (gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(treeview), (gint) event->x, (gint) event->y, &path, &column, NULL, NULL))
    {
      if(IupGetInt(ih, "SHOWRENAME"))
        gtkTreeShowRename_CB(ih);
      else
        gtkTreeRenameNode_CB(ih);

      return TRUE;
    }
  }
  else if (event->type == GDK_BUTTON_PRESS && !iupAttribGet(ih, "_IUPGTKTREE_ISSHIFT")
                                           && !iupAttribGet(ih, "_IUPGTKTREE_ISCONTROL"))
  {
    /* used when multiple selection mode was set, but not used - change to single selection mode  */
    GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));
    gtk_tree_selection_set_mode(selection, GTK_SELECTION_SINGLE);
  }
  
  return iupgtkButtonEvent(treeview, event, ih);
}

static gboolean gtkTreeKeyReleaseEvent(GtkWidget *widget, GdkEventKey *evt, Ihandle *ih)
{
  if((evt->keyval == GDK_Shift_L || evt->keyval == GDK_Shift_R) && ih->data->tree_shift)
  {
    GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(ih->handle));

    if(gtk_tree_selection_get_mode(selection) == GTK_SELECTION_MULTIPLE &&
       gtk_tree_selection_count_selected_rows(selection) < 2)
    {
      gtk_tree_selection_set_mode(selection, GTK_SELECTION_SINGLE);
    }
    else
    {
      /* Multi Selection Callback */
      gtkTreeMultiSelection_CB(ih);
    }

    iupAttribSetInt(ih, "_IUPGTKTREE_ISSHIFT", 0);
  }
  else if((evt->keyval == GDK_Control_L || evt->keyval == GDK_Control_R) && ih->data->tree_ctrl)
  {
    iupAttribSetInt(ih, "_IUPGTKTREE_ISCONTROL", 0);
  }

  /* In editing-started mode, check if the user set RENAMECARET and RENAMESELECTION attributes */
  if(iupAttribGet(ih, "_IUPTREE_EDITNAME") != NULL)
  {
    if(IupGetAttribute(ih, "RENAMECARET"))
      gtkTreeSetRenameCaretPos(ih);

    if(IupGetAttribute(ih, "RENAMESELECTION"))
      gtkTreeSetRenameSelectionPos(ih);

    iupAttribSetStr(ih, "_IUPTREE_EDITNAME", NULL);
  }

  (void)widget;
 
  return TRUE;
}

static gboolean gtkTreeKeyPressEvent(GtkWidget *widget, GdkEventKey *evt, Ihandle *ih)
{
  if (evt->keyval == GDK_F2)
  {
    if(IupGetInt(ih, "SHOWRENAME"))
      gtkTreeShowRename_CB(ih);
    else
      gtkTreeRenameNode_CB(ih);

    return TRUE;
  }
  else if (evt->keyval == GDK_Return || evt->keyval == GDK_KP_Enter)
  {
    gtkTreeOpenCloseEvent(ih);
    return TRUE;
  }
  else if((evt->keyval == GDK_Shift_L || evt->keyval == GDK_Shift_R) /*&& (evt->state & GDK_SHIFT_MASK)*/ && ih->data->tree_shift)
  {
    GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(ih->handle));
    
    gtk_tree_selection_set_mode(selection, GTK_SELECTION_MULTIPLE);
    iupAttribSetInt(ih, "_IUPGTKTREE_ISSHIFT", 1);
    
    return TRUE;
  }
  else if((evt->keyval == GDK_Control_L || evt->keyval == GDK_Control_R) /*&& (evt->state & GDK_CONTROL_MASK)*/ && ih->data->tree_ctrl)
  {
    GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(ih->handle));
    
    gtk_tree_selection_set_mode(selection, GTK_SELECTION_MULTIPLE);
    iupAttribSetInt(ih, "_IUPGTKTREE_ISCONTROL", 1);
    
    return TRUE;
  }

  return iupgtkKeyPressEvent(widget, evt, ih);
}

/*****************************************************************************/

static int gtkTreeMapMethod(Ihandle* ih)
{
  GtkScrolledWindow* scrolled_window = NULL;
  GtkTreeStore *store;
  GtkCellRenderer *renderer_img, *renderer_txt;
  GtkTreeSelection* selection;
  GtkTreeViewColumn *column;

  store = gtk_tree_store_new(7, GDK_TYPE_PIXBUF, GDK_TYPE_PIXBUF, GDK_TYPE_PIXBUF,
                                G_TYPE_STRING, G_TYPE_INT, GDK_TYPE_COLOR, G_TYPE_POINTER);

  ih->handle = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));

  g_object_unref(store);

  if (!ih->handle)
    return IUP_ERROR;

  scrolled_window = (GtkScrolledWindow*)gtk_scrolled_window_new(NULL, NULL);
  iupAttribSetStr(ih, "_IUP_EXTRAPARENT", (char*)scrolled_window);

  /* Column and renderers */
  column = gtk_tree_view_column_new();
  iupAttribSetStr(ih, "_IUPGTK_COLUMN",   (char*)column);

  renderer_img = gtk_cell_renderer_pixbuf_new();
  gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(column), renderer_img, FALSE);
  gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(column), renderer_img, "pixbuf", IUPGTK_TREE_IMAGE_LEAF,
                                                              "pixbuf-expander-open", IUPGTK_TREE_IMAGE_EXPANDED,
                                                            "pixbuf-expander-closed", IUPGTK_TREE_IMAGE_COLLAPSED, NULL);
  iupAttribSetStr(ih, "_IUPGTK_RENDERER_IMG", (char*)renderer_img);

  renderer_txt = gtk_cell_renderer_text_new();
  gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(column), renderer_txt, TRUE);
  gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(column), renderer_txt, "text", IUPGTK_TREE_TITLE,
                                                                     "is-expander", IUPGTK_TREE_KIND,
                                                                  "foreground-gdk", IUPGTK_TREE_COLOR, NULL);
  iupAttribSetStr(ih, "_IUPGTK_RENDERER_TEXT", (char*)renderer_txt);

  if(ih->data->show_rename)
    g_object_set(G_OBJECT(renderer_txt), "editable", TRUE, NULL);

  g_object_set(G_OBJECT(renderer_txt), "xpad", 0, NULL);
  g_object_set(G_OBJECT(renderer_txt), "ypad", 0, NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(ih->handle), column);

  gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(ih->handle), FALSE);
  gtk_tree_view_set_enable_search(GTK_TREE_VIEW(ih->handle), FALSE);

  if (iupAttribGetInt(ih, "HIDELINES"))
    gtk_tree_view_set_enable_tree_lines(GTK_TREE_VIEW(ih->handle), FALSE);
  else
    gtk_tree_view_set_enable_tree_lines(GTK_TREE_VIEW(ih->handle), TRUE);

  if (iupAttribGetInt(ih, "HIDEBUTTONS"))
    gtk_tree_view_set_show_expanders(GTK_TREE_VIEW(ih->handle), FALSE);
  else
    gtk_tree_view_set_show_expanders(GTK_TREE_VIEW(ih->handle), TRUE);

  gtk_container_add((GtkContainer*)scrolled_window, ih->handle);
  gtk_widget_show((GtkWidget*)scrolled_window);
  gtk_scrolled_window_set_shadow_type(scrolled_window, GTK_SHADOW_IN); 

  gtk_scrolled_window_set_policy(scrolled_window, GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(ih->handle));
 
  gtk_tree_selection_set_mode(selection, GTK_SELECTION_SINGLE);
  gtk_tree_view_set_reorderable(GTK_TREE_VIEW(ih->handle), FALSE);

  /* callbacks */
  g_signal_connect(selection,            "changed", G_CALLBACK(gtkTreeSelectionChanged), ih);
  
  g_signal_connect(renderer_txt, "editing-started", G_CALLBACK(gtkTreeCellTextEditingStarted), ih);
  g_signal_connect(renderer_txt,          "edited", G_CALLBACK(gtkTreeCellTextEdited), ih);

  g_signal_connect(G_OBJECT(ih->handle),       "row-expanded", G_CALLBACK(gtkTreeRowExpanded), ih);
  g_signal_connect(G_OBJECT(ih->handle),      "row-collapsed", G_CALLBACK(gtkTreeRowCollapsed), ih);
  g_signal_connect(G_OBJECT(ih->handle),      "row-activated", G_CALLBACK(gtkTreeRowActived), ih);
  g_signal_connect(G_OBJECT(ih->handle),    "key-press-event", G_CALLBACK(gtkTreeKeyPressEvent), ih);
  g_signal_connect(G_OBJECT(ih->handle),  "key-release-event", G_CALLBACK(gtkTreeKeyReleaseEvent), ih);
  g_signal_connect(G_OBJECT(ih->handle), "button-press-event", G_CALLBACK(gtkTreeButtonPressEvent), ih);
  g_signal_connect(G_OBJECT(ih->handle),         "drag-begin", G_CALLBACK(gtkTreeDragBegin), ih);
  g_signal_connect(G_OBJECT(ih->handle),          "drag-drop", G_CALLBACK(gtkTreeDragDrop), ih);
  g_signal_connect(G_OBJECT(ih->handle),           "drag-end", G_CALLBACK(gtkTreeDragEnd), ih);

  /* add to the parent, all GTK controls must call this. */
  iupgtkBaseAddToParent(ih);

  if (!iupStrBoolean(iupAttribGetStr(ih, "CANFOCUS")))
    GTK_WIDGET_FLAGS(ih->handle) &= ~GTK_CAN_FOCUS;

  gtk_widget_realize((GtkWidget*)scrolled_window);
  gtk_widget_realize(ih->handle);

  gtkTreeAddRootNode(ih);

  return IUP_NOERROR;
}

void iupdrvTreeInitClass(Iclass* ic)
{
  /* Driver Dependent Class functions */
  ic->Map = gtkTreeMapMethod;

  /* Visual */
  iupClassRegisterAttribute(ic, "BGCOLOR", NULL, gtkTreeSetBgColorAttrib, IUPAF_SAMEASSYSTEM, "TXTBGCOLOR", IUPAF_DEFAULT);
  iupClassRegisterAttribute(ic, "FGCOLOR", NULL, gtkTreeSetFgColorAttrib, IUPAF_SAMEASSYSTEM, "TXTFGCOLOR", IUPAF_DEFAULT);

  /* IupTree Attributes - GENERAL */
  iupClassRegisterAttribute(ic, "EXPANDALL",  NULL, gtkTreeSetExpandAllAttrib,  NULL, "NO", IUPAF_WRITEONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "SHOWDRAGDROP", NULL, gtkTreeSetShowDragDropAttrib, NULL, "NO", IUPAF_DEFAULT);
  iupClassRegisterAttribute(ic, "INDENTATION",  gtkTreeGetIndentationAttrib, gtkTreeSetIndentationAttrib, NULL, NULL, IUPAF_DEFAULT);
  iupClassRegisterAttribute(ic, "COUNT", gtkTreeGetCountAttrib, NULL, NULL, NULL, IUPAF_NO_DEFAULTVALUE|IUPAF_READONLY|IUPAF_NO_INHERIT);

  /* IupTree Attributes - IMAGES */
  iupClassRegisterAttributeId(ic, "IMAGE", NULL, gtkTreeSetImageAttrib, IUPAF_WRITEONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "IMAGEEXPANDED", NULL, gtkTreeSetImageExpandedAttrib, IUPAF_WRITEONLY|IUPAF_NO_INHERIT);

  iupClassRegisterAttribute(ic, "IMAGELEAF",            NULL, gtkTreeSetImageLeafAttrib, IUPAF_SAMEASSYSTEM, "IMGLEAF", IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "IMAGEBRANCHCOLLAPSED", NULL, gtkTreeSetImageBranchCollapsedAttrib, IUPAF_SAMEASSYSTEM, "IMGCOLLAPSED", IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "IMAGEBRANCHEXPANDED",  NULL, gtkTreeSetImageBranchExpandedAttrib, IUPAF_SAMEASSYSTEM, "IMGEXPANDED", IUPAF_NO_INHERIT);

  /* IupTree Attributes - NODES */
  iupClassRegisterAttributeId(ic, "STATE",  gtkTreeGetStateAttrib,  gtkTreeSetStateAttrib, IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "DEPTH",  gtkTreeGetDepthAttrib,  NULL, IUPAF_READONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "KIND",   gtkTreeGetKindAttrib,   NULL, IUPAF_READONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "PARENT", gtkTreeGetParentAttrib, NULL, IUPAF_READONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "COLOR",  gtkTreeGetColorAttrib,  gtkTreeSetColorAttrib, IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "NAME",   gtkTreeGetTitleAttrib,   gtkTreeSetTitleAttrib, IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "TITLE",   gtkTreeGetTitleAttrib,   gtkTreeSetTitleAttrib, IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "USERDATA",   gtkTreeGetUserDataAttrib,   gtkTreeSetUserDataAttrib, IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "CHILDCOUNT",   gtkTreeGetChildCountAttrib,   NULL, IUPAF_READONLY|IUPAF_NO_INHERIT);

  /* IupTree Attributes - MARKS */
  iupClassRegisterAttributeId(ic, "MARKED",   gtkTreeGetMarkedAttrib,   gtkTreeSetMarkedAttrib,   IUPAF_NO_DEFAULTVALUE|IUPAF_NO_INHERIT);

  iupClassRegisterAttribute  (ic, "VALUE",    gtkTreeGetValueAttrib,    gtkTreeSetValueAttrib,    NULL, NULL, IUPAF_NO_DEFAULTVALUE|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute  (ic, "STARTING", gtkTreeGetStartingAttrib, gtkTreeSetStartingAttrib, NULL, NULL, IUPAF_NO_DEFAULTVALUE|IUPAF_NO_INHERIT);

  /* IupTree Attributes - ACTION */
  iupClassRegisterAttributeId(ic, "DELNODE", NULL, gtkTreeSetDelNodeAttrib, IUPAF_WRITEONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "RENAME",  NULL, gtkTreeSetRenameAttrib,  IUPAF_NO_DEFAULTVALUE|IUPAF_NO_INHERIT);
}

// rever gtkTreeFindParentNode
