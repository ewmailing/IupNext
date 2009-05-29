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

#include "iup_drvinfo.h"
#include "iupgtk_drv.h"

enum
{
  IUPGTK_TREE_IMAGE,
  IUPGTK_TREE_HAS_IMAGE,
  IUPGTK_TREE_IMAGE_EXPANDED,
  IUPGTK_TREE_HAS_IMAGE_EXPANDED,
  IUPGTK_TREE_TITLE,
  IUPGTK_TREE_KIND,
  IUPGTK_TREE_COLOR,
  IUPGTK_TREE_FONT,
  IUPGTK_TREE_USERDATA
};

static GtkTreeIter gtkTreeInvalidIter = {0,0,0,0};

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
  gboolean has_image, has_image_expanded;
  PangoFontDescription* font;
  void* userdata;
  GdkColor color = {0L,0,0,0};
  GdkPixbuf* image, *image_expanded;

  gtk_tree_model_get(GTK_TREE_MODEL(store), &hItem, IUPGTK_TREE_IMAGE,      &image,
                                                    IUPGTK_TREE_HAS_IMAGE,      &has_image,
                                                    IUPGTK_TREE_IMAGE_EXPANDED,  &image_expanded,
                                                    IUPGTK_TREE_HAS_IMAGE_EXPANDED,  &has_image_expanded,
                                                    IUPGTK_TREE_TITLE,  &title,
                                                    IUPGTK_TREE_KIND,  &kind,
                                                    IUPGTK_TREE_COLOR, &color, 
                                                    IUPGTK_TREE_FONT, &font, 
                                                    IUPGTK_TREE_USERDATA, &userdata,
                                                    -1);

  gtk_tree_store_append(store, &iterNewItem, &htiNewParent);

  gtk_tree_store_set(store, &iterNewItem, IUPGTK_TREE_IMAGE,      image,
                                          IUPGTK_TREE_HAS_IMAGE,  has_image,
                                          IUPGTK_TREE_IMAGE_EXPANDED,  image_expanded,
                                          IUPGTK_TREE_HAS_IMAGE_EXPANDED, has_image_expanded,
                                          IUPGTK_TREE_TITLE,  title,
                                          IUPGTK_TREE_KIND,  kind,
                                          IUPGTK_TREE_COLOR, &color, 
                                          IUPGTK_TREE_FONT, font,
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

  return gtkTreeInvalidIter;
}

static void gtkTreeInvertAllNodeMarking(Ihandle* ih, GtkTreeModel* modelTree, GtkTreeSelection* selection, GtkTreeIter iterItem)
{
  GtkTreeIter iterChild;
  int hasItem = TRUE;

  while(hasItem)
  {
    if(gtk_tree_selection_iter_is_selected(selection, &iterItem))
      gtk_tree_selection_unselect_iter(selection, &iterItem);
    else
      gtk_tree_selection_select_iter(selection, &iterItem);

    /* Check whether we have child items */
    if(gtk_tree_model_iter_has_child(modelTree, &iterItem))
    {
      gtk_tree_model_iter_children(modelTree, &iterChild, &iterItem);  /* get the firstchild */
      gtkTreeInvertAllNodeMarking(ih, modelTree, selection, iterChild);
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
    ih->data->id_control++;   /* not the real id since it counts only the visible ones */

    /* StateID founded! */
    if(iterItem.user_data == iterNode.user_data)
      return iterItem;

    path = gtk_tree_model_get_path(modelTree, &iterItem);

    /* Check whether we have child items and it is expanded (visible) */
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

  return gtkTreeInvalidIter; /* invalid since gtk_tree_model_iter_next returned false */
}

static GtkTreeIter gtkTreeFindNewVisibleNode(Ihandle* ih, GtkTreeModel* modelTree, GtkTreeIter iterItem)
{
  GtkTreeIter iterChild;
  GtkTreePath* path;
  int hasItem = TRUE;

  while(hasItem)
  {
    /* ID control to traverse items */
    ih->data->id_control--;   /* not the real id since it counts only the visible ones */

    /* StateID founded! */
    if(ih->data->id_control < 0)
      return iterItem;

    path = gtk_tree_model_get_path(modelTree, &iterItem);

    /* Check whether we have child items and it is expanded (visible) */
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

  return gtkTreeInvalidIter; /* invalid since gtk_tree_model_iter_next returned false */
}

static GtkTreeIter gtkTreeGetLastVisibleNode(Ihandle* ih, GtkTreeModel* modelTree, GtkTreeIter iterItem)
{
  GtkTreeIter iterChild, iterPrev = gtkTreeInvalidIter;
  GtkTreePath* path = gtk_tree_model_get_path(modelTree, &iterItem);

  /* Check whether we have child items and it is expanded (visible) */
  if(gtk_tree_model_iter_has_child(modelTree, &iterItem) && gtk_tree_view_row_expanded(GTK_TREE_VIEW(ih->handle), path))
  {
    int hasItem = TRUE;
    gtk_tree_model_iter_children(modelTree, &iterChild, &iterItem);  /* get the firstchild */

    while(hasItem)
    {
      iterPrev = iterChild;

      /* Go to next sibling item */
      hasItem = gtk_tree_model_iter_next(modelTree, &iterChild);
    }

    iterItem = gtkTreeGetLastVisibleNode(ih, modelTree, iterPrev);
  }
  gtk_tree_path_free(path);

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

  return gtkTreeInvalidIter; /* invalid since gtk_tree_model_iter_next returned false */
}

static int gtkTreeGetNodeId(Ihandle* ih, GtkTreeIter iterItem)
{
  GtkTreeModel* model = gtk_tree_view_get_model(GTK_TREE_VIEW(ih->handle));
  GtkTreeIter iterRoot;
  gtk_tree_model_get_iter_first(model, &iterRoot);

  ih->data->id_control = -1;
  gtkTreeFindNodeFromID(ih, model, iterRoot, iterItem);
  return ih->data->id_control;
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

  return gtkTreeInvalidIter; /* invalid since gtk_tree_model_iter_next returned false */
}

static GtkTreeIter gtkTreeFindNodeFromString(Ihandle* ih, const char* name_id)
{
  GtkTreeModel* model = gtk_tree_view_get_model(GTK_TREE_VIEW(ih->handle));

  if (name_id[0])
  {
      GtkTreeIter iterRoot;
     iupStrToInt(name_id, &ih->data->id_control);
     gtk_tree_model_get_iter_first(model, &iterRoot);
     return gtkTreeFindNode(ih, model, iterRoot);
  }
  else
  {
    GtkTreeIter iterItem;
    GtkTreePath* pathFocus;
    gtk_tree_view_get_cursor(GTK_TREE_VIEW(ih->handle), &pathFocus, NULL);
    gtk_tree_model_get_iter(model, &iterItem, pathFocus);
    gtk_tree_path_free(pathFocus);
    return iterItem;
  }
}

/*****************************************************************************/
/* MANIPULATING IMAGES                                                       */
/*****************************************************************************/
static void gtkTreeUpdateImages(Ihandle* ih, GtkTreeModel* modelTree, GtkTreeIter iterItem, int mode)
{
  GtkTreeIter iterChild;
  int hasItem = TRUE;
  int kind;

  while(hasItem)
  {
    gtk_tree_model_get(modelTree, &iterItem, IUPGTK_TREE_KIND, &kind, -1);

    if (kind == ITREE_BRANCH)
    {
      if (mode == ITREE_UPDATEIMAGE_EXPANDED)
      {
        gboolean has_image_expanded = FALSE;
        gtk_tree_model_get(modelTree, &iterItem, IUPGTK_TREE_HAS_IMAGE_EXPANDED, &has_image_expanded, -1);
        if (!has_image_expanded)
          gtk_tree_store_set(GTK_TREE_STORE(modelTree), &iterItem, IUPGTK_TREE_IMAGE_EXPANDED, ih->data->def_image_expanded, -1);
      }
      else if(mode == ITREE_UPDATEIMAGE_COLLAPSED)
      {
        gboolean has_image = FALSE;
        gtk_tree_model_get(modelTree, &iterItem, IUPGTK_TREE_HAS_IMAGE, &has_image, -1);
        if (!has_image)
          gtk_tree_store_set(GTK_TREE_STORE(modelTree), &iterItem, IUPGTK_TREE_IMAGE, ih->data->def_image_collapsed, -1);
      }

      if (gtk_tree_model_iter_has_child(modelTree, &iterItem))
      {

        /* Recursively traverse child items */
        gtk_tree_model_iter_children(modelTree, &iterChild, &iterItem);
        gtkTreeUpdateImages(ih, modelTree, iterChild, mode);
      }
    }
    else 
    {
      if (mode == ITREE_UPDATEIMAGE_LEAF)
      {
        gboolean has_image = FALSE;
        gtk_tree_model_get(modelTree, &iterItem, IUPGTK_TREE_HAS_IMAGE, &has_image, -1);
        if (!has_image)
          gtk_tree_store_set(GTK_TREE_STORE(modelTree), &iterItem, IUPGTK_TREE_IMAGE, ih->data->def_image_leaf, -1);
      }
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
void iupdrvTreeAddNode(Ihandle* ih, const char* name_id, int kind, const char* title, int add)
{
  GtkTreeStore* store = GTK_TREE_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(ih->handle)));
  GtkTreeIter iterPrev = gtkTreeFindNodeFromString(ih, name_id);
  GtkTreeIter iterNewItem, iterParent;
  GtkTreePath* path;
  GdkColor color = {0L,0,0,0};
  int kindPrev;

  if (!iterPrev.user_data)
    return;

  gtk_tree_model_get(GTK_TREE_MODEL(store), &iterPrev, IUPGTK_TREE_KIND, &kindPrev, -1);

  if (kindPrev == ITREE_BRANCH && add)
    gtk_tree_store_insert(store, &iterNewItem, &iterPrev, 0);  /* iterPrev is parent of the new item (firstchild of it) */
  else
    gtk_tree_store_insert_after(store, &iterNewItem, NULL, &iterPrev);  /* iterPrev is sibling of the new item */

  iupgtkGetColor(iupAttribGetStr(ih, "FGCOLOR"), &color);

  /* set the attributes of the new node */
  gtk_tree_store_set(store, &iterNewItem, IUPGTK_TREE_HAS_IMAGE, FALSE,
                                          IUPGTK_TREE_HAS_IMAGE_EXPANDED, FALSE,
                                          IUPGTK_TREE_TITLE, title,
                                          IUPGTK_TREE_KIND, kind,
                                          IUPGTK_TREE_COLOR, &color, -1);

  if (kind == ITREE_LEAF)
    gtk_tree_store_set(store, &iterNewItem, IUPGTK_TREE_IMAGE, ih->data->def_image_leaf, -1);
  else
    gtk_tree_store_set(store, &iterNewItem, IUPGTK_TREE_IMAGE, ih->data->def_image_collapsed,
                                            IUPGTK_TREE_IMAGE_EXPANDED, ih->data->def_image_expanded, -1);

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
    {
      iupAttribSetStr(ih, "_IUPTREE_IGNORE_BRANCHOPEN_CB", "1");
      gtk_tree_view_expand_row(GTK_TREE_VIEW(ih->handle), path, FALSE);
    }
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
  gtk_tree_store_set(store, &iterRoot, IUPGTK_TREE_IMAGE, ih->data->def_image_collapsed,
                                       IUPGTK_TREE_HAS_IMAGE, FALSE,
                                       IUPGTK_TREE_IMAGE_EXPANDED, ih->data->def_image_expanded,
                                       IUPGTK_TREE_HAS_IMAGE_EXPANDED, FALSE,
                                       IUPGTK_TREE_KIND, ITREE_BRANCH,
                                       IUPGTK_TREE_COLOR, &color, -1);

  path = gtk_tree_model_get_path(GTK_TREE_MODEL(store), &iterRoot);
  /* MarkStart node */
  iupAttribSetStr(ih, "_IUPTREE_MARKSTART_NODE", (char*)path);

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

  if (!iter.user_data)
    return;

  path = gtk_tree_model_get_path(model, &iter);
  if (path)
  {
    gtk_tree_model_get(model, &iter, IUPGTK_TREE_KIND, &kind, -1);

    if (kind == ITREE_LEAF)  /* leafs */
    {
      gtk_tree_view_row_activated(GTK_TREE_VIEW(ih->handle), path, (GtkTreeViewColumn*)iupAttribGet(ih, "_IUPGTK_COLUMN"));     
    }
    else  /* branches */
    {
      if (gtk_tree_view_row_expanded(GTK_TREE_VIEW(ih->handle), path))
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

static gboolean gtkTreeSelected_Iter_Func(GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, GList **rowref_list)
{
  GtkTreeRowReference *rowref;
  GtkTreeIter iterParent;
  if (!gtk_tree_model_iter_parent(model, &iterParent, iter)) /* the root node can't be deleted */
    return FALSE; /* do not stop walking the store, call us with next row */

  rowref = gtk_tree_row_reference_new(model, path);
  *rowref_list = g_list_append(*rowref_list, rowref);

  return FALSE; /* do not stop walking the store, call us with next row */
}

/*****************************************************************************/
/* CALLBACKS                                                                 */
/*****************************************************************************/
static int gtkTreeCallMultiSelectionCb(Ihandle* ih)
{
  IFnIi cbMulti = (IFnIi)IupGetCallback(ih, "MULTISELECTION_CB");
  IFnii cbSelec = (IFnii)IupGetCallback(ih, "SELECTION_CB");
  if (cbMulti || cbSelec)
  {
    GtkTreeModel* model = gtk_tree_view_get_model(GTK_TREE_VIEW(ih->handle));
    GtkTreeSelection* selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(ih->handle));
    GtkTreeIter iterRoot;
    GList *rr_list = NULL;
    GList *node;
    int* id_rowItem;
    int count_selected_rows, i = 0;

    gtk_tree_model_get_iter_first(model, &iterRoot);

    gtk_tree_selection_selected_foreach(selection, (GtkTreeSelectionForeachFunc)gtkTreeSelected_Foreach_Func, &rr_list);
    count_selected_rows = g_list_length(rr_list);
    id_rowItem = malloc(sizeof(int) * count_selected_rows);

    for(node = rr_list; node != NULL; node = node->next)
    {
      GtkTreePath* path = gtk_tree_row_reference_get_path(node->data);
      if (path)
      {
        GtkTreeIter iterItem;
        gtk_tree_model_get_iter(model, &iterItem, path);

        id_rowItem[i] = gtkTreeGetNodeId(ih, iterItem);
        i++;

        gtk_tree_path_free(path);
      }
    }

    g_list_foreach(rr_list, (GFunc) gtk_tree_row_reference_free, NULL);
    g_list_free(rr_list);

    if (cbMulti)
      cbMulti(ih, id_rowItem, count_selected_rows);
    else
    {
      for (i=0; i<count_selected_rows; i++)
        cbSelec(ih, id_rowItem[i], 1);
    }

    free(id_rowItem);

    return IUP_DEFAULT;
  }

  return IUP_IGNORE;
}

static int gtkTreeCallRenameCb(Ihandle* ih, gchar *path_string, gchar* new_text)
{
  IFnis cbRename = (IFnis)IupGetCallback(ih, "RENAME_CB");
  if (new_text)
  {
    GtkTreeModel* model = gtk_tree_view_get_model(GTK_TREE_VIEW(ih->handle));
    GtkTreeIter iterItem;

    gtk_tree_model_get_iter_from_string(model, &iterItem, path_string);
    if (!iterItem.user_data)
      return IUP_IGNORE;

    if (cbRename)
      cbRename(ih, gtkTreeGetNodeId(ih, iterItem), new_text);

    gtk_tree_store_set(GTK_TREE_STORE(model), &iterItem, IUPGTK_TREE_TITLE, new_text, -1);

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
  if (!iterItem.user_data)
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
  GtkTreeIter iterParent;

  if (!iterItem.user_data)
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

    if (!gtk_tree_model_iter_parent(model, &iterParent, &iterBrotherItem))
      return 0;
  }
  else if (curDepth > newDepth)  /* Bottom -> Up */
  {
    GtkTreeIter iterNextParent;
    /* When the new depth is less than the current depth, 
    simply define a new parent to the node */
    ih->data->id_control = curDepth - newDepth + 1;  /* add 1 (one) to reach the level of its new parent */

    /* MarkStart the search by the parent of the current node */
    iterParent = iterItem;
    while(ih->data->id_control != 0)
    {
      /* Setting the new parent */
      gtk_tree_model_iter_parent(model, &iterNextParent, &iterParent);
      iterParent = iterNextParent;
      ih->data->id_control--;
    }
  }
  else /* same depth, nothing to do */
    return 0;

  /* without parent, nothing to do */
  if (!iterParent.user_data)
    return 0;

  /* Copying the node and its children to the new position */
  gtkTreeCopyBranch(ih, iterItem, iterParent);

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
  if (!iterItem.user_data)
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

  if (!iterItem.user_data)
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
  GtkTreeIter iterParent;
  char* str;

  if (!iterItem.user_data)
    return NULL;

  if (!gtk_tree_model_iter_parent(model, &iterParent, &iterItem))
    return NULL;

  str = iupStrGetMemory(10);
  sprintf(str, "%d", gtkTreeGetNodeId(ih, iterParent));
  return str;
}

static char* gtkTreeGetChildCountAttrib(Ihandle* ih, const char* name_id)
{
  GtkTreeModel* model = gtk_tree_view_get_model(GTK_TREE_VIEW(ih->handle));
  GtkTreeIter iterItem = gtkTreeFindNodeFromString(ih, name_id);
  char* str;

  if (!iterItem.user_data)
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

  if (!iterItem.user_data)
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

  if (!iterItem.user_data)
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

  if (!iterItem.user_data)
    return 0;

  path = gtk_tree_model_get_path(model, &iterItem);
  if(iupStrEqualNoCase(value, "COLLAPSED"))
    gtk_tree_view_collapse_row(GTK_TREE_VIEW(ih->handle), path);
  else if(iupStrEqualNoCase(value, "EXPANDED"))
    gtk_tree_view_expand_row(GTK_TREE_VIEW(ih->handle), path, FALSE);

  gtk_tree_path_free(path);

  return 0;
}

static char* gtkTreeGetTitle(GtkTreeModel* model, GtkTreeIter iterItem)
{
  char* title;
  gtk_tree_model_get(model, &iterItem, IUPGTK_TREE_TITLE, &title, -1);
  return title;
}

static char* gtkTreeGetTitleAttrib(Ihandle* ih, const char* name_id)
{
  GtkTreeModel* model = gtk_tree_view_get_model(GTK_TREE_VIEW(ih->handle));
  GtkTreeIter iterItem = gtkTreeFindNodeFromString(ih, name_id);
  if (!iterItem.user_data)
    return NULL;
  return gtkTreeGetTitle(model, iterItem);
}

static int gtkTreeSetTitleAttrib(Ihandle* ih, const char* name_id, const char* value)
{
  GtkTreeStore* store = GTK_TREE_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(ih->handle)));
  GtkTreeIter iterItem = gtkTreeFindNodeFromString(ih, name_id);
  if (!iterItem.user_data)
    return 0;
  gtk_tree_store_set(store, &iterItem, IUPGTK_TREE_TITLE, value, -1);
  return 0;
}

static int gtkTreeSetTitleFontAttrib(Ihandle* ih, const char* name_id, const char* value)
{
  PangoFontDescription* fontdesc = NULL;
  GtkTreeStore* store = GTK_TREE_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(ih->handle)));
  GtkTreeIter iterItem = gtkTreeFindNodeFromString(ih, name_id);
  if (!iterItem.user_data)
    return 0;
  if (value)
    fontdesc = iupgtkGetPangoFontDesc(value);
  gtk_tree_store_set(store, &iterItem, IUPGTK_TREE_FONT, fontdesc, -1);
  return 0;
}

static char* gtkTreeGetTitleFontAttrib(Ihandle* ih, const char* name_id)
{
  PangoFontDescription* fontdesc;
  GtkTreeModel* model = gtk_tree_view_get_model(GTK_TREE_VIEW(ih->handle));
  GtkTreeIter iterItem = gtkTreeFindNodeFromString(ih, name_id);
  if (!iterItem.user_data)
    return NULL;
  gtk_tree_model_get(model, &iterItem, IUPGTK_TREE_TITLE, &fontdesc, -1);
  return iupgtkFindPangoFontDesc(fontdesc);
}

static char* gtkTreeGetUserDataAttrib(Ihandle* ih, const char* name_id)
{
  GtkTreeModel* model = gtk_tree_view_get_model(GTK_TREE_VIEW(ih->handle));
  GtkTreeIter iterItem = gtkTreeFindNodeFromString(ih, name_id);
  char* userdata;
  if (!iterItem.user_data)
    return NULL;
  gtk_tree_model_get(model, &iterItem, IUPGTK_TREE_USERDATA, &userdata, -1);
  return userdata;
}

static int gtkTreeSetUserDataAttrib(Ihandle* ih, const char* name_id, const char* value)
{
  GtkTreeStore* store = GTK_TREE_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(ih->handle)));
  GtkTreeIter iterItem = gtkTreeFindNodeFromString(ih, name_id);
  if (!iterItem.user_data)
    return 0;
  gtk_tree_store_set(store, &iterItem, IUPGTK_TREE_USERDATA, value, -1);
  return 0;
}

static char* gtkTreeGetValueAttrib(Ihandle* ih)
{
  GtkTreeModel* model = gtk_tree_view_get_model(GTK_TREE_VIEW(ih->handle));
  GtkTreeIter iterFocus;
  GtkTreePath* pathFocus;
  char* str;

  gtk_tree_view_get_cursor(GTK_TREE_VIEW(ih->handle), &pathFocus, NULL);
  gtk_tree_model_get_iter(model, &iterFocus, pathFocus);
  gtk_tree_path_free(pathFocus);

  if(!iterFocus.user_data)
    return 0;

  str = iupStrGetMemory(16);
  sprintf(str, "%d", gtkTreeGetNodeId(ih, iterFocus));
  return str;
}

static int gtkTreeSetMarkAttrib(Ihandle* ih, const char* value)
{
  GtkTreeModel* model = gtk_tree_view_get_model(GTK_TREE_VIEW(ih->handle));
  GtkTreeSelection* selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(ih->handle));
  GtkTreeIter iterRoot;

  if (ih->data->mark_mode==ITREE_MARK_SINGLE)
    return 0;

  gtk_tree_model_get_iter_first(model, &iterRoot);

  if(iupStrEqualNoCase(value, "BLOCK"))
  {
    GtkTreePath* pathFocus;
    gtk_tree_view_get_cursor(GTK_TREE_VIEW(ih->handle), &pathFocus, NULL);
    gtk_tree_selection_select_range(selection, (GtkTreePath*)iupAttribGet(ih, "_IUPTREE_MARKSTART_NODE"), pathFocus);
    gtk_tree_path_free(pathFocus);
  }
  else if(iupStrEqualNoCase(value, "CLEARALL"))
    gtk_tree_selection_unselect_all(selection);
  else if(iupStrEqualNoCase(value, "MARKALL"))
    gtk_tree_selection_select_all(selection);
  else if(iupStrEqualNoCase(value, "INVERTALL")) /* INVERTALL *MUST* appear before INVERT, or else INVERTALL will never be called. */
    gtkTreeInvertAllNodeMarking(ih, model, selection, iterRoot);
  else if(iupStrEqualPartial(value, "INVERT"))
  {
    /* iupStrEqualPartial allows the use of "INVERTid" form */
    GtkTreeIter iterSelected = gtkTreeFindNodeFromString(ih, &value[strlen("INVERT")]);

    if (!iterSelected.user_data)
      return 0;

    if(gtk_tree_selection_iter_is_selected(selection, &iterSelected))
      gtk_tree_selection_unselect_iter(selection, &iterSelected);
    else
      gtk_tree_selection_select_iter(selection, &iterSelected);
  }
  else
  {
    GtkTreePath *path1, *path2;
    GtkTreeIter iterItem1, iterItem2;
    char str1[50], str2[50];
    if (iupStrToStrStr(value, str1, str2, '-')!=2)
      return 0;

    iterItem1 = gtkTreeFindNodeFromString(ih, str1);
    if (!iterItem1.user_data)  
      return 0;
    iterItem2 = gtkTreeFindNodeFromString(ih, str2);
    if (!iterItem2.user_data)  
      return 0;

    path1 = gtk_tree_model_get_path(model, &iterItem1);
    path2 = gtk_tree_model_get_path(model, &iterItem2);
    gtk_tree_selection_select_range(selection, path1, path2);
    gtk_tree_path_free(path1);
    gtk_tree_path_free(path2);
  }

  return 1;
} 

static int gtkTreeSetValueAttrib(Ihandle* ih, const char* value)
{
  GtkTreeModel* model = gtk_tree_view_get_model(GTK_TREE_VIEW(ih->handle));
  GtkTreeSelection* selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(ih->handle));
  GtkTreeIter iterRoot, iterItem;
  GtkTreePath* path;

  if (gtkTreeSetMarkAttrib(ih, value))
    return 0;

  gtk_tree_model_get_iter_first(model, &iterRoot);

  if (iupStrEqualNoCase(value, "ROOT"))
    iterItem = iterRoot;
  else if(iupStrEqualNoCase(value, "LAST"))
    iterItem = gtkTreeGetLastVisibleNode(ih, model, iterRoot);
  else if(iupStrEqualNoCase(value, "PGUP"))
  {
    GtkTreeIter iterPrev;

    GtkTreePath* pathFocus;
    gtk_tree_view_get_cursor(GTK_TREE_VIEW(ih->handle), &pathFocus, NULL);
    gtk_tree_model_get_iter(model, &iterPrev, pathFocus);
    gtk_tree_path_free(pathFocus);

    ih->data->id_control = -1;
    gtkTreeFindCurrentVisibleNode(ih, model, iterRoot, iterPrev);
    ih->data->id_control -= 10;  /* less 10 visible nodes */

    if(ih->data->id_control < 0)
      ih->data->id_control = 0;  /* Begin of tree = Root id */

    iterItem = gtkTreeFindNewVisibleNode(ih, model, iterRoot);
  }
  else if(iupStrEqualNoCase(value, "PGDN"))
  {
    GtkTreeIter iterNext;

    GtkTreePath* pathFocus;
    gtk_tree_view_get_cursor(GTK_TREE_VIEW(ih->handle), &pathFocus, NULL);
    gtk_tree_model_get_iter(model, &iterNext, pathFocus);
    gtk_tree_path_free(pathFocus);

    ih->data->id_control = -1;
    gtkTreeFindCurrentVisibleNode(ih, model, iterRoot, iterNext);
    ih->data->id_control += 10;  /* more 10 visible nodes */

    iterNext = gtkTreeFindNewVisibleNode(ih, model, iterRoot);

    if (ih->data->id_control >= 0)
      iterNext = gtkTreeGetLastVisibleNode(ih, model, iterRoot);

    iterItem = iterNext;
  }
  else if(iupStrEqualNoCase(value, "NEXT"))
  {
    GtkTreeIter iterNext;

    GtkTreePath* pathFocus;
    gtk_tree_view_get_cursor(GTK_TREE_VIEW(ih->handle), &pathFocus, NULL);
    gtk_tree_model_get_iter(model, &iterNext, pathFocus);
    gtk_tree_path_free(pathFocus);

    ih->data->id_control = -1;
    gtkTreeFindCurrentVisibleNode(ih, model, iterRoot, iterNext);
    ih->data->id_control++;  /* more 1 visible node */

    iterNext = gtkTreeFindNewVisibleNode(ih, model, iterRoot);

    if (ih->data->id_control >= 0)
      iterNext = gtkTreeGetLastVisibleNode(ih, model, iterRoot);

    iterItem = iterNext;
  }
  else if(iupStrEqualNoCase(value, "PREVIOUS"))
  {
    GtkTreeIter iterPrev;

    GtkTreePath* pathFocus;
    gtk_tree_view_get_cursor(GTK_TREE_VIEW(ih->handle), &pathFocus, NULL);
    gtk_tree_model_get_iter(model, &iterPrev, pathFocus);
    gtk_tree_path_free(pathFocus);

    ih->data->id_control = -1;
    gtkTreeFindCurrentVisibleNode(ih, model, iterRoot, iterPrev);
    ih->data->id_control--;   /* less 1 visible node */

    if (ih->data->id_control < 0)
      ih->data->id_control = 0;

    iterItem = gtkTreeFindNewVisibleNode(ih, model, iterRoot);
  }
  else
    iterItem = gtkTreeFindNodeFromString(ih, value);

  if (!iterItem.user_data)
    return 0;

  /* select */
  if (ih->data->mark_mode==ITREE_MARK_SINGLE)
  {
    iupAttribSetStr(ih, "_IUP_IGNORE_SELECTION", "1");
    gtk_tree_selection_select_iter(selection, &iterItem);
  }

  path = gtk_tree_model_get_path(model, &iterItem);
  gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(ih->handle), path, NULL, FALSE, 0, 0); /* scroll to visible */
  gtk_tree_view_set_cursor(GTK_TREE_VIEW(ih->handle), path, NULL, FALSE);  /* set focus */
  gtk_tree_path_free(path);

  iupAttribSetInt(ih, "_IUPTREE_OLDVALUE", gtkTreeGetNodeId(ih, iterItem));

  return 0;
} 

static int gtkTreeSetMarkStartAttrib(Ihandle* ih, const char* name_id)
{
  GtkTreePath *pathMarkStart, *pathMarkStartPrev;
  GtkTreeModel* model = gtk_tree_view_get_model(GTK_TREE_VIEW(ih->handle));
  GtkTreeIter iterMarkStart = gtkTreeFindNodeFromString(ih, name_id);
  if (!iterMarkStart.user_data)
    return 0;

  pathMarkStart = gtk_tree_model_get_path(model, &iterMarkStart);

  pathMarkStartPrev = (GtkTreePath*)iupAttribGet(ih, "_IUPTREE_MARKSTART_NODE");
  if (pathMarkStartPrev)
    gtk_tree_path_free(pathMarkStartPrev);

  iupAttribSetStr(ih, "_IUPTREE_MARKSTART_NODE", (char*)pathMarkStart);

  return 1;
}

static char* gtkTreeGetMarkedAttrib(Ihandle* ih, const char* name_id)
{
  GtkTreeSelection* selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(ih->handle));
  GtkTreeIter iterItem = gtkTreeFindNodeFromString(ih, name_id);
  if (!iterItem.user_data)
    return 0;

  if(gtk_tree_selection_iter_is_selected(selection, &iterItem))
    return "YES";
  else
    return "NO";
}

static int gtkTreeSetMarkedAttrib(Ihandle* ih, const char* name_id, const char* value)
{
  GtkTreeSelection* selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(ih->handle));
  GtkTreeIter iterItem = gtkTreeFindNodeFromString(ih, name_id);
  if (!iterItem.user_data)
    return 0;

  iupAttribSetStr(ih, "_IUP_IGNORE_SELECTION", "1");

  if (iupStrBoolean(value))
    gtk_tree_selection_select_iter(selection, &iterItem);
  else
    gtk_tree_selection_unselect_iter(selection, &iterItem);

  return 0;
}

static int gtkTreeSetDelNodeAttrib(Ihandle* ih, const char* name_id, const char* value)
{
  if (iupStrEqualNoCase(value, "SELECTED"))  /* selectec here means the specified one */
  {
    GtkTreeModel* model = gtk_tree_view_get_model(GTK_TREE_VIEW(ih->handle));
    GtkTreeIter iterItem = gtkTreeFindNodeFromString(ih, name_id);
    GtkTreeIter iterParent;

    if (!iterItem.user_data)
      return 0;

    if (!gtk_tree_model_iter_parent(model, &iterParent, &iterItem)) /* the root node can't be deleted */
      return 0;

    /* deleting the specified node (and it's children) */
    gtk_tree_store_remove(GTK_TREE_STORE(model), &iterItem);
  }
  else if(iupStrEqualNoCase(value, "CHILDREN"))  /* children of the specified one */
  {
    GtkTreeModel* model = gtk_tree_view_get_model(GTK_TREE_VIEW(ih->handle));
    GtkTreeIter   iterItem = gtkTreeFindNodeFromString(ih, name_id);
    GtkTreeIter   iterChild, iterParent;
    int hasChildren;

    if (!iterItem.user_data)
      return 0;

    if (!gtk_tree_model_iter_parent(model, &iterParent, &iterItem)) /* the root node can't be deleted */
      return 0;

    hasChildren = gtk_tree_model_iter_children(model, &iterChild, &iterItem);

    /* deleting the selected node's children */
    while(hasChildren)
      hasChildren = gtk_tree_store_remove(GTK_TREE_STORE(model), &iterChild);
  }
  else if(iupStrEqualNoCase(value, "MARKED"))  /* Delete the array of marked nodes */
  {
    GtkTreeModel* model = gtk_tree_view_get_model(GTK_TREE_VIEW(ih->handle));
    GtkTreeSelection* selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(ih->handle));
    GList *rr_list = NULL;
    GList *node;

    gtk_tree_selection_selected_foreach(selection, (GtkTreeSelectionForeachFunc)gtkTreeSelected_Iter_Func, &rr_list);

    for(node = rr_list; node != NULL; node = node->next)
    {
      GtkTreePath* path = gtk_tree_row_reference_get_path(node->data);
      if (path)
      {
        GtkTreeIter iter;
        if (gtk_tree_model_get_iter(model, &iter, path))
          gtk_tree_store_remove(GTK_TREE_STORE(model), &iter);
        gtk_tree_path_free(path);
      }
      gtk_tree_row_reference_free(node->data);
    }
    g_list_free(rr_list);
  }

  return 0;
}

static int gtkTreeSetRenameAttrib(Ihandle* ih, const char* value)
{  
  if (ih->data->show_rename)
  {
    GtkTreePath* path;
    IFni cbShowRename = (IFni)IupGetCallback(ih, "SHOWRENAME_CB");

    gtk_tree_view_get_cursor(GTK_TREE_VIEW(ih->handle), &path, NULL);

    if (cbShowRename)
    {
      GtkTreeIter iterItem;
      GtkTreeModel* model = gtk_tree_view_get_model(GTK_TREE_VIEW(ih->handle));
      gtk_tree_model_get_iter(model, &iterItem, path);
      cbShowRename(ih, gtkTreeGetNodeId(ih, iterItem));
    }

    gtk_tree_view_set_cursor(GTK_TREE_VIEW(ih->handle), path, NULL, TRUE);
    gtk_widget_grab_focus(ih->handle);
    gtk_tree_path_free(path);
  }
  else
  {
    IFnis cbRenameNode = (IFnis)IupGetCallback(ih, "RENAMENODE_CB");
    if (cbRenameNode)
    {
      GtkTreePath* path;
      GtkTreeIter iterItem;
      GtkTreeModel* model = gtk_tree_view_get_model(GTK_TREE_VIEW(ih->handle));
      gtk_tree_view_get_cursor(GTK_TREE_VIEW(ih->handle), &path, NULL);
      gtk_tree_path_free(path);
      gtk_tree_model_get_iter(model, &iterItem, path);
      cbRenameNode(ih, gtkTreeGetNodeId(ih, iterItem), gtkTreeGetTitle(model, iterItem));  
    }
  }

  (void)value;
  return 0;
}

static int gtkTreeSetImageExpandedAttrib(Ihandle* ih, const char* name_id, const char* value)
{
  int kind;
  GtkTreeStore*  store = GTK_TREE_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(ih->handle)));
  GdkPixbuf* pixExpand = iupImageGetImage(value, ih, 0, "IMAGE");
  GtkTreeIter iterItem = gtkTreeFindNodeFromString(ih, name_id);
  if (!iterItem.user_data)
    return 0;

  gtk_tree_model_get(GTK_TREE_MODEL(store), &iterItem, IUPGTK_TREE_KIND, &kind, -1);

  if (kind == ITREE_BRANCH)
  {
    if (pixExpand)
      gtk_tree_store_set(store, &iterItem, IUPGTK_TREE_IMAGE_EXPANDED, pixExpand, 
                                           IUPGTK_TREE_HAS_IMAGE_EXPANDED, TRUE, -1);
    else
      gtk_tree_store_set(store, &iterItem, IUPGTK_TREE_IMAGE_EXPANDED, ih->data->def_image_expanded, 
                                           IUPGTK_TREE_HAS_IMAGE_EXPANDED, FALSE, -1);
  }

  return 1;
}

static int gtkTreeSetImageAttrib(Ihandle* ih, const char* name_id, const char* value)
{
  GtkTreeStore* store = GTK_TREE_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(ih->handle)));
  GdkPixbuf* pixImage = iupImageGetImage(value, ih, 0, "IMAGE");
  GtkTreeIter iterItem = gtkTreeFindNodeFromString(ih, name_id);
  if (!iterItem.user_data)
    return 0;

  if (pixImage)
  {
    gtk_tree_store_set(store, &iterItem, IUPGTK_TREE_IMAGE, pixImage, 
                                         IUPGTK_TREE_HAS_IMAGE, TRUE, -1);
  }
  else
  {
    int kind;
    gtk_tree_model_get(GTK_TREE_MODEL(store), &iterItem, IUPGTK_TREE_KIND, &kind, -1);
    if (kind == ITREE_BRANCH)
      gtk_tree_store_set(store, &iterItem, IUPGTK_TREE_IMAGE, ih->data->def_image_collapsed, 
                                           IUPGTK_TREE_HAS_IMAGE, FALSE, -1);
    else
      gtk_tree_store_set(store, &iterItem, IUPGTK_TREE_IMAGE, ih->data->def_image_leaf, 
                                           IUPGTK_TREE_HAS_IMAGE, FALSE, -1);
  }

  return 1;
}

static int gtkTreeSetImageBranchExpandedAttrib(Ihandle* ih, const char* value)
{
  GtkTreeIter iterRoot;
  GtkTreeModel* model = gtk_tree_view_get_model(GTK_TREE_VIEW(ih->handle));
  ih->data->def_image_expanded = iupImageGetImage(value, ih, 0, "IMAGEBRANCHEXPANDED");

  gtk_tree_model_get_iter_first(model, &iterRoot);

  /* Update all images, starting at root node */
  gtkTreeUpdateImages(ih, model, iterRoot, ITREE_UPDATEIMAGE_EXPANDED);

  return 1;
}

static int gtkTreeSetImageBranchCollapsedAttrib(Ihandle* ih, const char* value)
{
  GtkTreeIter iterRoot;
  GtkTreeModel*  model = gtk_tree_view_get_model(GTK_TREE_VIEW(ih->handle));
  ih->data->def_image_collapsed = iupImageGetImage(value, ih, 0, "IMAGEBRANCHCOLLAPSED");

  gtk_tree_model_get_iter_first(model, &iterRoot);

  /* Update all images, starting at root node */
  gtkTreeUpdateImages(ih, model, iterRoot, ITREE_UPDATEIMAGE_COLLAPSED);

  return 1;
}

static int gtkTreeSetImageLeafAttrib(Ihandle* ih, const char* value)
{
  GtkTreeIter iterRoot;
  GtkTreeModel* model = gtk_tree_view_get_model(GTK_TREE_VIEW(ih->handle));
  ih->data->def_image_leaf = iupImageGetImage(value, ih, 0, "IMAGELEAF");

  gtk_tree_model_get_iter_first(model, &iterRoot);

  /* Update all images, starting at root node */
  gtkTreeUpdateImages(ih, model, iterRoot, ITREE_UPDATEIMAGE_LEAF);

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

void iupdrvTreeUpdateMarkMode(Ihandle *ih)
{
  GtkTreeSelection* selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(ih->handle));
  gtk_tree_selection_set_mode(selection, (ih->data->mark_mode==ITREE_MARK_SINGLE)? GTK_SELECTION_SINGLE: GTK_SELECTION_MULTIPLE);
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

static void gtkTreeCellTextEdited(GtkCellRendererText *cell, gchar *path_string, gchar *new_text, Ihandle* ih)
{
  gtkTreeCallRenameCb(ih, path_string, new_text);
  (void)cell;
}

static int gtkTreeCallDragDropCb(Ihandle* ih)
{
  IFniiii cbDragDrop = (IFniiii)IupGetCallback(ih, "DRAGDROP_CB");
  if (cbDragDrop)
  {
    int drag_str = iupAttribGetInt(ih, "_IUPTREE_DRAGID");
    int drop_str = iupAttribGetInt(ih, "_IUPTREE_DROPID");
    int   isshift_str = 0;
    int iscontrol_str = 0;
    char key[5];
    iupdrvGetKeyState(key);
    if (key[0] == 'S')
      isshift_str = 1;
    if (key[1] == 'C')
      iscontrol_str = 1;

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
      gtkTreeCallDragDropCb(ih);

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
    iupAttribSetInt(ih, "_IUPTREE_DROPID", gtkTreeGetNodeId(ih, iterItem));
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
  GtkTreeIter iterItem;

  if (gtk_tree_selection_get_selected (selection, NULL, &iterItem))
  {
    GtkTreePath* pathDrag = gtk_tree_model_get_path(model, &iterItem);
    iupAttribSetStr(ih, "_IUPTREE_DRAGITEM", (char*)pathDrag);
    iupAttribSetInt(ih, "_IUPTREE_DRAGID", gtkTreeGetNodeId(ih, iterItem));
  }

  (void)drag_context;
  (void)widget;
}

static void gtkTreeSelectionChanged(GtkTreeSelection* selection, Ihandle* ih)
{
  IFnii cbSelec;
  int is_ctrl = 0;

  if (ih->data->mark_mode == ITREE_MARK_MULTIPLE)
  {
    char key[5];
    iupdrvGetKeyState(key);
    if (key[0] == 'S')
      return;
    else if (key[1] == 'C')
      is_ctrl = 1;
  }

  cbSelec = (IFnii)IupGetCallback(ih, "SELECTION_CB");
  if (cbSelec)
  {
    int curpos;
    GtkTreeIter iterFocus;

    if (iupAttribGet(ih, "_IUP_IGNORE_SELECTION"))
    {
      iupAttribSetStr(ih, "_IUP_IGNORE_SELECTION", NULL);
      return;
    }

    {
      GtkTreePath* pathFocus;
      GtkTreeModel* model = gtk_tree_view_get_model(GTK_TREE_VIEW(ih->handle));
      gtk_tree_view_get_cursor(GTK_TREE_VIEW(ih->handle), &pathFocus, NULL);
      gtk_tree_model_get_iter(model, &iterFocus, pathFocus);
      gtk_tree_path_free(pathFocus);
    }
    curpos = gtkTreeGetNodeId(ih, iterFocus);

    if (is_ctrl) 
      cbSelec(ih, curpos, gtk_tree_selection_iter_is_selected(selection, &iterFocus));
    else
    {
      int oldpos = iupAttribGetInt(ih, "_IUPTREE_OLDVALUE");
      if(oldpos != curpos)
      {
        cbSelec(ih, oldpos, 0);  /* unselected */
        cbSelec(ih, curpos, 1);  /*   selected */

        iupAttribSetInt(ih, "_IUPTREE_OLDVALUE", curpos);
      }
    }
  }
}

static gboolean gtkTreeTestExpandRow(GtkTreeView* tree_view, GtkTreeIter *iterItem, GtkTreePath *path, Ihandle* ih)
{
  IFni cbBranchOpen = (IFni)IupGetCallback(ih, "BRANCHOPEN_CB");
  if (cbBranchOpen)
  {
    if (iupAttribGet(ih, "_IUPTREE_IGNORE_BRANCHOPEN_CB"))
    {
      iupAttribSetStr(ih, "_IUPTREE_IGNORE_BRANCHOPEN_CB", NULL);
      return FALSE;
    }

    if (cbBranchOpen(ih, gtkTreeGetNodeId(ih, *iterItem)) == IUP_IGNORE)
      return TRUE;  /* prevent the change */
  }

  (void)path;
  (void)tree_view;
  return FALSE;
}

static gboolean gtkTreeTestCollapseRow(GtkTreeView* tree_view, GtkTreeIter *iterItem, GtkTreePath *path, Ihandle* ih)
{
  IFni cbBranchClose = (IFni)IupGetCallback(ih, "BRANCHCLOSE_CB");
  if (cbBranchClose)
  {
    if (cbBranchClose(ih, gtkTreeGetNodeId(ih, *iterItem)) == IUP_IGNORE)
      return TRUE;
  }

  (void)path;
  (void)tree_view;
  return FALSE;
}

static void gtkTreeRowActived(GtkTreeView* tree_view, GtkTreePath *path, GtkTreeViewColumn *column, Ihandle* ih)
{
  IFni cbExecuteLeaf  = (IFni)IupGetCallback(ih, "EXECUTELEAF_CB");
  GtkTreeIter iterItem;
  GtkTreeModel* model = gtk_tree_view_get_model(GTK_TREE_VIEW(ih->handle));
  int kind;  /* used for nodes defined as branches, but do not have children */

  gtk_tree_model_get_iter(model, &iterItem, path);
  gtk_tree_model_get(model, &iterItem, IUPGTK_TREE_KIND, &kind, -1);

  /* just to leaf nodes */
  if(gtk_tree_model_iter_has_child(model, &iterItem) == 0 && kind == ITREE_LEAF)
    cbExecuteLeaf(ih, gtkTreeGetNodeId(ih, iterItem));

  (void)column;
  (void)tree_view;
}

static int gtkTreeConvertXYToPos(Ihandle* ih, int x, int y)
{
  GtkTreePath* path;
  if (gtk_tree_view_get_dest_row_at_pos((GtkTreeView*)ih->handle, x, y, &path, NULL))
  {
    GtkTreeIter iterItem;
    GtkTreeModel* model = gtk_tree_view_get_model(GTK_TREE_VIEW(ih->handle));
    gtk_tree_model_get_iter(model, &iterItem, path);
    gtk_tree_path_free (path);
    return gtkTreeGetNodeId(ih, iterItem);
  }
  return -1;
}

static gboolean gtkTreeButtonEvent(GtkWidget *treeview, GdkEventButton *evt, Ihandle* ih)
{
  if (evt->type == GDK_BUTTON_PRESS && evt->button == 3)  /* right single click */
  {
    IFni cbRightClick  = (IFni)IupGetCallback(ih, "RIGHTCLICK_CB");
    if (cbRightClick)
    {
      int id = gtkTreeConvertXYToPos(ih, (int)evt->x, (int)evt->y);
      if (id != -1)
        cbRightClick(ih, id);
      return TRUE;
    }
  }
  else if (evt->type == GDK_2BUTTON_PRESS && evt->button == 1)  /* left double click */
  {
    GtkTreePath *path;

    if (gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(treeview), (gint) evt->x, (gint) evt->y, &path, NULL, NULL, NULL))
    {
      GtkTreeIter iter;
      GtkTreeModel* model = gtk_tree_view_get_model(GTK_TREE_VIEW(ih->handle));
      int kind;  /* used for nodes defined as branches, but do not have children */

      gtk_tree_model_get_iter(model, &iter, path);
      gtk_tree_model_get(model, &iter, IUPGTK_TREE_KIND, &kind, -1);

      if (kind == ITREE_BRANCH)
      {
        if (gtk_tree_view_row_expanded(GTK_TREE_VIEW(ih->handle), path))
          gtk_tree_view_collapse_row(GTK_TREE_VIEW(ih->handle), path);
        else
          gtk_tree_view_expand_row(GTK_TREE_VIEW(ih->handle), path, FALSE);
      }

      gtk_tree_path_free(path);
    }
  }
  else if (evt->type == GDK_BUTTON_RELEASE && evt->button == 1)  /* left single release */
  {
    if (ih->data->mark_mode==ITREE_MARK_MULTIPLE && (evt->state & GDK_SHIFT_MASK))
      gtkTreeCallMultiSelectionCb(ih); /* Multi Selection Callback */
  }
  
  return iupgtkButtonEvent(treeview, evt, ih);
}

static gboolean gtkTreeKeyReleaseEvent(GtkWidget *widget, GdkEventKey *evt, Ihandle *ih)
{
  /* In editing-started mode, check if the user set RENAMECARET and RENAMESELECTION attributes */
  if(iupAttribGet(ih, "_IUPTREE_EDITNAME") != NULL)
  {
    if(IupGetAttribute(ih, "RENAMECARET"))
      gtkTreeSetRenameCaretPos(ih);

    if(IupGetAttribute(ih, "RENAMESELECTION"))
      gtkTreeSetRenameSelectionPos(ih);

    iupAttribSetStr(ih, "_IUPTREE_EDITNAME", NULL);
  }

  if (ih->data->mark_mode==ITREE_MARK_MULTIPLE && (evt->state & GDK_SHIFT_MASK))
  {
    if (evt->keyval == GDK_Up || evt->keyval == GDK_Down || evt->keyval == GDK_Home || evt->keyval == GDK_End)
      gtkTreeCallMultiSelectionCb(ih); /* Multi Selection Callback */
  }

  (void)widget;
  return TRUE;
}

static gboolean gtkTreeKeyPressEvent(GtkWidget *widget, GdkEventKey *evt, Ihandle *ih)
{
  if (evt->keyval == GDK_F2)
  {
    gtkTreeSetRenameAttrib(ih, NULL);
    return TRUE;
  }
  else if (evt->keyval == GDK_Return || evt->keyval == GDK_KP_Enter)
  {
    gtkTreeOpenCloseEvent(ih);
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

  store = gtk_tree_store_new(9, GDK_TYPE_PIXBUF, G_TYPE_BOOLEAN, GDK_TYPE_PIXBUF, G_TYPE_BOOLEAN,
                                G_TYPE_STRING, G_TYPE_INT, GDK_TYPE_COLOR, PANGO_TYPE_FONT_DESCRIPTION, G_TYPE_POINTER);

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
  gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(column), renderer_img, "pixbuf", IUPGTK_TREE_IMAGE,
                                                              "pixbuf-expander-open", IUPGTK_TREE_IMAGE_EXPANDED,
                                                            "pixbuf-expander-closed", IUPGTK_TREE_IMAGE, NULL);
  iupAttribSetStr(ih, "_IUPGTK_RENDERER_IMG", (char*)renderer_img);

  renderer_txt = gtk_cell_renderer_text_new();
  gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(column), renderer_txt, TRUE);
  gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(column), renderer_txt, "text", IUPGTK_TREE_TITLE,
                                                                     "is-expander", IUPGTK_TREE_KIND,
                                                                     "font-desc", IUPGTK_TREE_FONT,
                                                                  "foreground-gdk", IUPGTK_TREE_COLOR, NULL);
  iupAttribSetStr(ih, "_IUPGTK_RENDERER_TEXT", (char*)renderer_txt);

  if (ih->data->show_rename)
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

  g_signal_connect(G_OBJECT(ih->handle), "enter-notify-event", G_CALLBACK(iupgtkEnterLeaveEvent), ih);
  g_signal_connect(G_OBJECT(ih->handle), "leave-notify-event", G_CALLBACK(iupgtkEnterLeaveEvent), ih);
  g_signal_connect(G_OBJECT(ih->handle), "focus-in-event",     G_CALLBACK(iupgtkFocusInOutEvent), ih);
  g_signal_connect(G_OBJECT(ih->handle), "focus-out-event",    G_CALLBACK(iupgtkFocusInOutEvent), ih);
  g_signal_connect(G_OBJECT(ih->handle), "show-help",          G_CALLBACK(iupgtkShowHelp), ih);
  g_signal_connect(G_OBJECT(ih->handle), "motion-notify-event",G_CALLBACK(iupgtkMotionNotifyEvent), ih);

  g_signal_connect(G_OBJECT(ih->handle),    "test-expand-row", G_CALLBACK(gtkTreeTestExpandRow), ih);
  g_signal_connect(G_OBJECT(ih->handle),  "test-collapse-row", G_CALLBACK(gtkTreeTestCollapseRow), ih);
  g_signal_connect(G_OBJECT(ih->handle),      "row-activated", G_CALLBACK(gtkTreeRowActived), ih);
  g_signal_connect(G_OBJECT(ih->handle),    "key-press-event", G_CALLBACK(gtkTreeKeyPressEvent), ih);
  g_signal_connect(G_OBJECT(ih->handle),  "key-release-event", G_CALLBACK(gtkTreeKeyReleaseEvent), ih);
  g_signal_connect(G_OBJECT(ih->handle), "button-press-event", G_CALLBACK(gtkTreeButtonEvent), ih);
  g_signal_connect(G_OBJECT(ih->handle), "button-release-event",G_CALLBACK(gtkTreeButtonEvent), ih);
  //g_signal_connect(G_OBJECT(ih->handle),         "drag-begin", G_CALLBACK(gtkTreeDragBegin), ih);
  //g_signal_connect(G_OBJECT(ih->handle),          "drag-drop", G_CALLBACK(gtkTreeDragDrop), ih);
  //g_signal_connect(G_OBJECT(ih->handle),           "drag-end", G_CALLBACK(gtkTreeDragEnd), ih);

  /* add to the parent, all GTK controls must call this. */
  iupgtkBaseAddToParent(ih);

  if (!iupStrBoolean(iupAttribGetStr(ih, "CANFOCUS")))
    GTK_WIDGET_FLAGS(ih->handle) &= ~GTK_CAN_FOCUS;

  gtk_widget_realize((GtkWidget*)scrolled_window);
  gtk_widget_realize(ih->handle);

  /* Initialize the default images */
  ih->data->def_image_leaf = iupImageGetImage("IMGLEAF", ih, 0, "IMAGELEAF");
  ih->data->def_image_collapsed = iupImageGetImage("IMGCOLLAPSED", ih, 0, "IMAGEBRANCHCOLLAPSED");
  ih->data->def_image_expanded = iupImageGetImage("IMGEXPANDED", ih, 0, "IMAGEBRANCHEXPANDED");

  gtkTreeAddRootNode(ih);

  /* configure for DRAG&DROP of files */
  if (IupGetCallback(ih, "DROPFILES_CB"))
    iupAttribSetStr(ih, "DRAGDROP", "YES");

  IupSetCallback(ih, "_IUP_XY2POS_CB", (Icallback)gtkTreeConvertXYToPos);

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
  iupClassRegisterAttribute(ic, "DRAGDROP", NULL, iupgtkSetDragDropAttrib, NULL, NULL, IUPAF_NO_INHERIT);

  /* IupTree Attributes - IMAGES */
  iupClassRegisterAttributeId(ic, "IMAGE", NULL, gtkTreeSetImageAttrib, IUPAF_WRITEONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "IMAGEBRANCHEXPANDED", NULL, gtkTreeSetImageExpandedAttrib, IUPAF_WRITEONLY|IUPAF_NO_INHERIT);

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
  iupClassRegisterAttributeId(ic, "TITLEFONT", gtkTreeGetTitleFontAttrib, gtkTreeSetTitleFontAttrib, IUPAF_NO_INHERIT);

  /* IupTree Attributes - MARKS */
  iupClassRegisterAttributeId(ic, "MARKED",   gtkTreeGetMarkedAttrib,   gtkTreeSetMarkedAttrib,   IUPAF_NO_DEFAULTVALUE|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute  (ic, "MARK",    NULL,    gtkTreeSetMarkAttrib,    NULL, NULL, IUPAF_WRITEONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute  (ic, "STARTING", NULL, gtkTreeSetMarkStartAttrib, NULL, NULL, IUPAF_NO_DEFAULTVALUE|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute  (ic, "MARKSTART", NULL, gtkTreeSetMarkStartAttrib, NULL, NULL, IUPAF_NO_DEFAULTVALUE|IUPAF_NO_INHERIT);

  iupClassRegisterAttribute  (ic, "VALUE",    gtkTreeGetValueAttrib,    gtkTreeSetValueAttrib,    NULL, NULL, IUPAF_NO_DEFAULTVALUE|IUPAF_NO_INHERIT);

  /* IupTree Attributes - ACTION */
  iupClassRegisterAttributeId(ic, "DELNODE", NULL, gtkTreeSetDelNodeAttrib, IUPAF_WRITEONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "RENAME",  NULL, gtkTreeSetRenameAttrib,  NULL, NULL, IUPAF_WRITEONLY|IUPAF_NO_INHERIT);
}

// mudar GtkTreeIter iterItem para usar ponteiros
