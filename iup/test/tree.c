/*IupTree Example in C 
Creates a tree with some branches and leaves. 
Two callbacks are registered: one deletes marked nodes when the Del key is pressed, 
and the other, called when the right mouse button is pressed, opens a menu with options. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "iup.h"
#include "iupcontrols.h"
#include "iupkey.h"


/* Callback called when a leaf is added by the menu. */
static int addleaf(void)
{
  static char attr[10];
  Ihandle* tree = IupGetHandle("tree");
  int id = IupGetInt(tree, "VALUE");
  sprintf(attr, "ADDLEAF%d", id);
  IupSetAttribute(tree, attr, "");
  IupSetAttribute(tree, "REDRAW", "YES");
  return IUP_DEFAULT;
}

/* Callback called when a branch is added by the menu. */
static int addbranch(void)
{
  static char attr[10];
  Ihandle* tree = IupGetHandle("tree");
  int id = IupGetInt(tree, "VALUE");
  sprintf(attr, "ADDBRANCH%d", id);
  IupSetAttribute(tree, attr, "");
  IupSetAttribute(tree, "REDRAW", "YES");
  return IUP_DEFAULT;
}

static int text_cb(Ihandle* self, int c, char *after)
{
  if (c == K_ESC)
    return IUP_CLOSE;

  if (c == K_CR)
  {
    Ihandle *tree = IupGetHandle("tree");
    IupSetAttribute(tree, "NAME",   after);
    IupSetAttribute(tree, "REDRAW", "YES");
    return IUP_CLOSE;
  }
  
  return IUP_DEFAULT;
}

/* Callback called when a node is removed by the menu. */
static int removenode(void)
{
  Ihandle* tree = IupGetHandle("tree");
  IupSetAttribute(tree, "DELNODE", "MARKED");
  IupSetAttribute(tree, "REDRAW",  "YES");
  return IUP_DEFAULT;
}

int renamenode(void)
{
  Ihandle* tree = IupGetHandle("tree");
  IupSetAttribute(tree, "RENAME", "YES");
  return IUP_DEFAULT;
}

int showrename_cb(Ihandle* ih, int id)
{
  (void)ih;
  printf("showrename_cb (%d)\n", id);
  return IUP_DEFAULT;
}

int selection_cb(Ihandle *ih, int id, int status)
{
  (void)ih;
  printf("selection_cb (%d - %d)\n", id, status);
  return IUP_DEFAULT;
}

int multiselection_cb(Ihandle *ih, int* ids, int n)
{
  int i;
  (void)ih;
  printf("multiselection_cb (");
  for (i = 0; i < n; i++)
    printf("%d ", ids[i]);
  printf(")\n");
  return IUP_DEFAULT;
}

static int executeleaf_cb(Ihandle* h, int id)
{
  printf("executeleaf_cb (%d)\n", id);
  return IUP_DEFAULT;
}

static int renamenode_cb(Ihandle* h, int id, char* name)
{
  printf("renamenode_cb (%d=%s)\n", id, name);
  return IUP_DEFAULT;
}

static int rename_cb(Ihandle* h, int id, char* name)
{
  printf("rename_cb (%d=%s)\n", id, name);
  if (strcmp(name, "fool") == 0)
    return IUP_IGNORE;
  return IUP_DEFAULT;
}

static int branchopen_cb(Ihandle* h, int id)
{
  printf("branchopen_cb (%d)\n", id);
  return IUP_DEFAULT;
}

static int branchclose_cb(Ihandle* h, int id)
{
  printf("branchclose_cb (%d)\n", id);
  return IUP_DEFAULT;
}

static int dragdrop_cb(Ihandle* h, int drag_id, int drop_id, int shift, int control)
{
  printf("dragdrop_cb (%d)->(%d)\n", drag_id, drop_id);
  return IUP_DEFAULT;
}

/* Callback called when a key is hit */
static int k_any_cb(Ihandle* h, int c)
{
  if (c == K_DEL) 
  {
    IupSetAttribute(h, "DELNODE", "MARKED");
    IupSetAttribute(h, "REDRAW",  "YES");
  }

  return IUP_DEFAULT;
}

static int selectnode(Ihandle* h)
{
  Ihandle* tree = IupGetHandle("tree");
  IupSetAttribute(tree, "VALUE",  IupGetAttribute(h, "TITLE"));
  IupSetAttribute(tree, "REDRAW", "YES");
  return IUP_DEFAULT;
}

/* Callback called when the right mouse button is pressed */
static int rightclick_cb(Ihandle* h, int id)
{
  Ihandle *popup_menu;

  static char id_string[10];

  popup_menu = IupMenu(
    IupItem ("Add Leaf","addleaf"),
    IupItem ("Add Branch","addbranch"),
    IupItem ("Rename Node","renamenode"),
    IupItem ("Remove Node","removenode"),
    IupSubmenu("Selection", IupMenu(
      IupItem ("ROOT", "selectnode"),
      IupItem ("LAST", "selectnode"),
      IupItem ("PGUP", "selectnode"),
      IupItem ("PGDN", "selectnode"),
      IupItem ("NEXT", "selectnode"),
      IupItem ("PREVIOUS", "selectnode"),
      IupSeparator(),
      IupItem ("INVERT", "selectnode"),
      IupItem ("BLOCK", "selectnode"),
      IupItem ("CLEARALL", "selectnode"),
      IupItem ("MARKALL", "selectnode"),
      IupItem ("INVERTALL", "selectnode"),
      NULL)),
    NULL);
    
  IupSetFunction("selectnode", (Icallback) selectnode);
  IupSetFunction("addleaf",    (Icallback) addleaf);
  IupSetFunction("addbranch",  (Icallback) addbranch);
  IupSetFunction("removenode", (Icallback) removenode);
  IupSetFunction("renamenode", (Icallback) renamenode);

  sprintf(id_string, "%d", id);
  IupSetAttribute(h, "VALUE", id_string);
  IupPopup(popup_menu, IUP_MOUSEPOS, IUP_MOUSEPOS);

  IupDestroy(popup_menu);

  return IUP_DEFAULT;
}

/* Initializes IupTree and registers callbacks */
static void init_tree(void)
{
  Ihandle* tree = IupTree(); 

  IupSetCallback(tree, "EXECUTELEAF_CB", (Icallback) executeleaf_cb);
  IupSetCallback(tree, "RENAMENODE_CB",  (Icallback) renamenode_cb);
  IupSetCallback(tree, "RENAME_CB",      (Icallback) rename_cb);
  IupSetCallback(tree, "BRANCHCLOSE_CB", (Icallback) branchclose_cb);
  IupSetCallback(tree, "BRANCHOPEN_CB",  (Icallback) branchopen_cb);
  IupSetCallback(tree, "DRAGDROP_CB",    (Icallback) dragdrop_cb);
  IupSetCallback(tree, "RIGHTCLICK_CB",  (Icallback) rightclick_cb);
  IupSetCallback(tree, "K_ANY",          (Icallback) k_any_cb);

  IupSetCallback(tree, "SHOWRENAME_CB", (Icallback) showrename_cb);
  IupSetCallback(tree, "SELECTION_CB", (Icallback) selection_cb);
  IupSetCallback(tree, "MULTISELECTION_CB", (Icallback) multiselection_cb);

  IupSetHandle("tree", tree);
}

/* Initializes the dlg */
static void init_dlg(void)
{
  Ihandle* tree = IupGetHandle("tree");
  Ihandle* box = IupVbox(IupHbox(tree, NULL), NULL);
  Ihandle* dlg = IupDialog(box) ;
  IupSetAttribute(dlg,  "TITLE",   "IupTree");
//  IupSetAttribute(tree, "SIZE",    "QUARTERxTHIRD");
  IupSetAttribute(box,  "MARGIN",  "10x10");
//  IupSetAttribute(dlg,  "BGCOLOR", "192 192 192");
  IupSetHandle("dlg", dlg);
}

/* Initializes the IupTree’s attributes */
static void init_tree_atributes(void)
{
  Ihandle* tree = IupGetHandle("tree");

//  IupSetAttribute(tree, "FONT",         "COURIER_NORMAL_10");

  /* Notice that the tree is create from bottom to top */
  /* the current node is the ROOT */
  IupSetAttribute(tree, "NAME",         "Figures");  /* name of the root, id=0 */
  IupSetAttribute(tree, "ADDBRANCH",    "3D");    /* 3D=1 */
  IupSetAttribute(tree, "ADDBRANCH",    "2D");    /* add to the root, so it will be before "3D", now 2D=1, 3D=2 */
  IupSetAttribute(tree, "ADDLEAF",      "test");  /* add to the root, also before "2D", now test=1, 2D=2, 3D=3 */

  IupSetAttribute(tree, "ADDBRANCH1",   "parallelogram");  /* add after "test", now test=1, parallelogram=2, 2D=3, 3D=4  */
  IupSetAttribute(tree, "ADDLEAF2",     "diamond");
  IupSetAttribute(tree, "ADDLEAF2",     "square");
  IupSetAttribute(tree, "ADDBRANCH1",   "triangle");       /* add after "test" */
  IupSetAttribute(tree, "ADDLEAF2",     "scalenus");
  IupSetAttribute(tree, "ADDLEAF2",     "isoceles");
  IupSetAttribute(tree, "ADDLEAF2",     "equilateral");

  IupSetAttribute(tree, "VALUE",        "6");

  IupSetAttribute(tree, "CTRL",         "YES");
  IupSetAttribute(tree, "SHIFT",        "YES");
  IupSetAttribute(tree, "ADDEXPANDED",  "NO");
  IupSetAttribute(tree, "SHOWDRAGDROP", "YES");
  IupSetAttribute(tree, "SHOWRENAME",   "YES");

  IupSetAttribute(tree, "REDRAW", "YES");
}

void TreeTest(void)
{
  Ihandle* dlg;
  
  init_tree();                            /* Initializes IupTree */
  init_dlg();                             /* Initializes the dlg */
  dlg = IupGetHandle("dlg");              /* Retrieves the dlg handle */
  IupShowXY(dlg, IUP_CENTER, IUP_CENTER); /* Displays the dlg */
  init_tree_atributes();                  /* Initializes attributes, can be done here or anywhere */
}

#ifndef BIG_TEST
int main(int argc, char* argv[])
{
  IupOpen(&argc, &argv);
  IupControlsOpen();

  TreeTest();

  IupMainLoop();

  IupClose();

  return EXIT_SUCCESS;
}
#endif
