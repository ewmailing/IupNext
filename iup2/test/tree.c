/*IupTree Example in C 
Creates a tree with some branches and leaves. 
Two callbacks are registered: one deletes marked nodes when the Del key is pressed, 
and the other, called when the right mouse button is pressed, opens a menu with options. */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iup.h>
#include <iupcontrols.h>

int mnu_addleaf(void)
{
  static char attr[10];
  Ihandle* tree = IupGetHandle("tree");
  int id = IupGetInt(tree,IUP_VALUE);
  sprintf(attr,"ADDLEAF%d",id);
  IupSetAttribute(tree,attr,"");
  IupSetAttribute(tree, IUP_REDRAW, IUP_YES);
  return IUP_DEFAULT;
}

int mnu_addbranch(void)
{
  static char attr[10];
  Ihandle* tree = IupGetHandle("tree");
  int id = IupGetInt(tree,IUP_VALUE);
  sprintf(attr,"ADDBRANCH%d",id);
  IupSetAttribute(tree,attr,"");
  IupSetAttribute(tree, IUP_REDRAW, IUP_YES);
  return IUP_DEFAULT;
}

int mnu_removenode(void)
{
  Ihandle* tree = IupGetHandle("tree");
  IupSetAttribute(tree,IUP_DELNODE,IUP_MARKED);
  IupSetAttribute(tree, IUP_REDRAW, IUP_YES);
  return IUP_DEFAULT;
}

int mnu_renamenode(void)
{
  Ihandle* tree = IupGetHandle("tree");
  IupSetAttribute(tree,"RENAME","YES");
  return IUP_DEFAULT;
}

int showrename_cb(Ihandle* h, int id)
{
  printf("showrename_cb (%d)\n", id);
//  IupSetAttribute(h,"CARET","3");
//  IupSetAttribute(h,"SELECTION","4:6");
  return IUP_DEFAULT;
}

int selection_cb(Ihandle *self, int id, int status)
{
  printf("selection_cb (%d - %d)\n", id, status);
  return IUP_DEFAULT;
}

int multiselection_cb(Ihandle *self, int* ids, int n)
{
  int i;
  printf("multiselection_cb (");
  for (i = 0; i < n; i++)
    printf("%d ", ids[i]);
  printf(")\n");
  return IUP_DEFAULT;
}

int executeleaf_cb(Ihandle* h, int id)
{
  printf("executeleaf_cb (%d)\n", id);
  return IUP_DEFAULT;
}

int renamenode_cb(Ihandle* h, int id, char* name)
{
  /* called only when SHOWRENAME=NO */
  printf("renamenode_cb (%d=%s)\n", id, name);
  return IUP_DEFAULT;
}

int rename_cb(Ihandle* h, int id, char* name)
{
  printf("rename_cb (%d=%s)\n", id, name);
  if (strcmp(name, "fool") == 0)
  {
    IupMessage("Message", "No Rename Fool.");
    return IUP_IGNORE;
  }
  return IUP_DEFAULT;
}

int branchopen_cb(Ihandle* h, int id)
{
  printf("branchopen_cb (%d)\n", id);
  return IUP_DEFAULT;
}

int branchclose_cb(Ihandle* h, int id)
{
  printf("branchclose_cb (%d)\n", id);
  return IUP_DEFAULT;
}

int dragdrop_cb(Ihandle* h, int drag_id, int drop_id, int isshift, int iscontrol)
{
  printf("dragdrop_cb (%d)->(%d)\n", drag_id, drop_id);
  return IUP_DEFAULT;
}

/* Callback called when a key is hit */
int k_any_cb(Ihandle* h, int c)
{
  if (c == K_DEL) 
  {
    IupSetAttribute(h,IUP_DELNODE,IUP_MARKED);
    IupSetAttribute(h, IUP_REDRAW, IUP_YES);
  }

  return IUP_DEFAULT;
}

int mnu_selectnode(Ihandle* h)
{
  Ihandle* tree = IupGetHandle("tree");
  IupSetAttribute(tree,IUP_VALUE,IupGetAttribute(h, IUP_TITLE));
  IupSetAttribute(tree, IUP_REDRAW, IUP_YES);
  return IUP_DEFAULT;
}

/* Callback called when the right mouse button is pressed */
int rightclick_cb(Ihandle* h, int id)
{
  Ihandle *popup_menu;

  static char id_string[10];

  popup_menu = IupMenu(
    IupItem ("Add Leaf","mnu_addleaf"),
    IupItem ("Add Branch","mnu_addbranch"),
    IupItem ("Rename Node","mnu_renamenode"),
    IupItem ("Remove Node","mnu_removenode"),
    IupSubmenu("Selection", IupMenu(
      IupItem ("ROOT", "mnu_selectnode"),
      IupItem ("LAST", "mnu_selectnode"),
      IupItem ("PGUP", "mnu_selectnode"),
      IupItem ("PGDN", "mnu_selectnode"),
      IupItem ("NEXT", "mnu_selectnode"),
      IupItem ("PREVIOUS", "mnu_selectnode"),
      IupSeparator(),
      IupItem ("INVERT", "mnu_selectnode"),
      IupItem ("BLOCK", "mnu_selectnode"),
      IupItem ("CLEARALL", "mnu_selectnode"),
      IupItem ("MARKALL", "mnu_selectnode"),
      IupItem ("INVERTALL", "mnu_selectnode"),
      NULL)),
    NULL);

  sprintf(id_string,"%d",id);
  IupSetAttribute(h, IUP_VALUE, id_string);
  IupPopup(popup_menu,IUP_MOUSEPOS,IUP_MOUSEPOS);

  IupDestroy(popup_menu);

  return IUP_DEFAULT;
}

int dropfiles_cb(Ihandle *ih, const char* filename, int num, int x, int y)
{
  printf("DROPFILES_CB(%s, %d, %d, %d)\n", filename, num, x, y);
  return IUP_DEFAULT;
}

/* Initializes IupTree and registers callbacks */
void init_tree(void)
{
  Ihandle* tree = IupTree(); 

  IupSetAttribute(tree, "EXECUTELEAF_CB", "executeleaf_cb");
  IupSetFunction("executeleaf_cb", (Icallback) executeleaf_cb);
  IupSetAttribute(tree, "RENAMENODE_CB", "renamenode_cb") ;
  IupSetFunction("renamenode_cb", (Icallback) renamenode_cb);
  IupSetAttribute(tree, "RENAME_CB", "rename_cb") ;
  IupSetFunction("rename_cb", (Icallback) rename_cb);
  IupSetAttribute(tree, "BRANCHCLOSE_CB", "branchclose_cb") ;
  IupSetFunction("branchclose_cb", (Icallback) branchclose_cb);
  IupSetAttribute(tree, "BRANCHOPEN_CB", "branchopen_cb") ;
  IupSetFunction("branchopen_cb", (Icallback) branchopen_cb);
  IupSetAttribute(tree, "DRAGDROP_CB", "dragdrop_cb") ;
  IupSetFunction("dragdrop_cb", (Icallback) dragdrop_cb);
  IupSetAttribute(tree, "SHOWRENAME_CB", "showrename_cb") ;
  IupSetFunction("showrename_cb", (Icallback) showrename_cb);
  IupSetAttribute(tree, "RIGHTCLICK_CB", "rightclick_cb") ;
  IupSetFunction("rightclick_cb", (Icallback) rightclick_cb);
  IupSetAttribute(tree, "SELECTION_CB", "selection_cb") ;
  IupSetFunction("selection_cb", (Icallback) selection_cb);
  IupSetAttribute(tree, "MULTISELECTION_CB", "multiselection_cb") ;
  IupSetFunction("multiselection_cb", (Icallback) multiselection_cb);

  IupSetAttribute(tree, IUP_K_ANY, "k_any_cb");
  IupSetFunction("k_any_cb", (Icallback) k_any_cb);

  /* for the right click menu */
  IupSetFunction("mnu_addleaf", (Icallback)mnu_addleaf);
  IupSetFunction("mnu_addbranch", (Icallback)mnu_addbranch);
  IupSetFunction("mnu_removenode", (Icallback)mnu_removenode);
  IupSetFunction("mnu_renamenode", (Icallback)mnu_renamenode);
  IupSetFunction("mnu_selectnode", (Icallback) mnu_selectnode);

  IupSetHandle("tree",tree);
}

/* Initializes the dialog */
void init_dlg(void)
{
  Ihandle* tree = IupGetHandle("tree");
  Ihandle* box = IupHbox(tree, /*IupButton("Test", "xx"), */ NULL);
  Ihandle* dlg = IupDialog(box) ;
  IupSetAttribute(dlg, IUP_TITLE, "IupTree");
  IupSetAttribute(tree, IUP_SIZE,"50x100");
  IupSetAttribute(box, IUP_MARGIN, "10x10");
  IupSetAttribute(box, IUP_GAP, "10");
  IupSetHandle("dlg",dlg);
  IupSetCallback(tree, "DROPFILES_CB", (Icallback)dropfiles_cb);
}

/* Initializes the IupTree’s attributes */
void init_tree_atributes(void)
{
  Ihandle* tree = IupGetHandle("tree");

//  IupSetAttribute(tree, "FONT",IUP_COURIER_NORMAL_10);
  IupSetAttribute(tree, "NAME","Figures");
  IupSetAttribute(tree, "ADDBRANCH","3D");
  IupSetAttribute(tree, "ADDBRANCH","2D");
  IupSetAttribute(tree, "ADDLEAF","test");
  IupSetAttribute(tree, "ADDBRANCH1","parallelogram");
  IupSetAttribute(tree, "ADDLEAF2","diamond");
  IupSetAttribute(tree, "ADDLEAF2","square");
  IupSetAttribute(tree, "ADDBRANCH1","triangle");
  IupSetAttribute(tree, "ADDLEAF2","scalenus");
  IupSetAttribute(tree, "ADDLEAF2","isoceles");
  IupSetAttribute(tree, "ADDLEAF2","equilateral");

  IupSetAttribute(tree, "VALUE","6");
  IupSetAttribute(tree, "CTRL",IUP_YES);
  IupSetAttribute(tree, "SHIFT",IUP_YES);
  IupSetAttribute(tree, "ADDEXPANDED", IUP_NO);
  IupSetAttribute(tree, "SHOWDRAGDROP", IUP_YES);
  IupSetAttribute(tree, "SHOWRENAME", "YES");

//  IupSetAttribute(tree, "IMAGELEAF", "IMGBLANK");
//  IupSetAttribute(tree, "IMAGELEAF", "IMGPAPER");

  IupSetAttribute(tree, IUP_REDRAW, IUP_YES);
}

/* Main program */
int main(int argc, char **argv)
{
  Ihandle* dlg ;
  
  IupOpen(&argc, &argv);                            /* IUP initialization */
  IupControlsOpen();                    /* Initializes the controls library */

  init_tree();                          /* Initializes IupTree */
  init_dlg();                           /* Initializes the dialog */
  dlg = IupGetHandle("dlg");            /* Retrieves the dialog handle */
  IupShowXY(dlg,IUP_CENTER,IUP_CENTER); /* Displays the dialog */
  init_tree_atributes();                /* Initializes attributes, can be done here or anywhere */

  IupMainLoop();                         /* Main loop */
  IupDestroy(dlg);
  IupControlsClose();                    /* Ends the controls library */
  IupClose();                            /* Ends IUP */

  return 0;
}
