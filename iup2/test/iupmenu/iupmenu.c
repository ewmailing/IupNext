

#include <stdio.h>

#include <iup.h>


int item_help_cb (void)
{
  IupMessage ("Warning", "Only Help and Exit items performs an operation") ;
  return IUP_DEFAULT ;
}
    
int item_exit_cb (void)
{  
  return IUP_CLOSE;
}
  
int equilatero(void){
  printf("OK!\n");  
  return IUP_DEFAULT;
}

int selection(Ihandle *item){
  if(item)
    printf("%s\n", IupGetAttribute(item, IUP_TITLE));
  else
    printf("Menu closed\n");
  return IUP_DEFAULT;
}
    
Ihandle *menu, *menu_file, *menu_edit, *menu_help ; 
Ihandle *menu_create ; 
Ihandle *menu_triangle ; 
    
static int killfocus(Ihandle *self, Ihandle* newfocus)
{
  printf("KILLFOCUS: saiu de %s e entrou em %s\n", self?IupGetClassName(self):NULL, newfocus? IupGetClassName(newfocus):NULL);
  return IUP_DEFAULT;
}

static int getfocus(Ihandle *self)
{
  printf("GETFOCUS : entrou em %s\n", self?IupGetClassName(self):NULL);
  return IUP_DEFAULT;
}

int main(int argc, char **argv)
{
  Ihandle *dlg ;
  Ihandle *text ;
  Ihandle *submenu_file, *submenu_edit, *submenu_help ;
  Ihandle *submenu_create ;
  Ihandle *submenu_triangle ;
  Ihandle *item_new, *item_open, *item_close, *item_exit ;
  Ihandle *item_copy, *item_paste ;
  Ihandle *item_line, *item_circle ;
  Ihandle *item_equilateral, *item_isoceles, *item_scalenus ;
  Ihandle *item_help ;

  IupOpen(&argc, &argv);
  IupSetFunction ("item_help_act", (Icallback) item_help_cb) ;
  IupSetFunction ("selection", (Icallback) selection) ;
  IupSetFunction ("item_exit_act", (Icallback) item_exit_cb) ;
  IupSetFunction ("equilatero", (Icallback) equilatero) ;
  IupSetFunction ("killfocus", (Icallback)  killfocus) ;
  IupSetFunction ("getfocus", (Icallback)  getfocus) ;
  
  text = IupText("hehehe") ;
  IupSetAttribute(text, IUP_KILLFOCUS_CB, "killfocus");
  IupSetAttribute(text, IUP_GETFOCUS_CB, "getfocus");
  

  IupSetAttributes (text, "VALUE = \"This text is here only to compose\", EXPAND = YES") ;
  item_new = IupItem ("New", NULL) ;
  item_open = IupItem ("Open", NULL) ;
  item_close = IupItem ("Close", NULL) ;
  item_exit = IupItem ("Exit", "item_exit_act") ;

  item_copy = IupItem ("Copy", NULL) ;
  item_paste = IupItem ("Paste", NULL) ;
  item_scalenus = IupItem ("Scalenus", NULL) ;
  item_isoceles = IupItem ("Isoceles", NULL) ;
  item_equilateral = IupItem ("Equilateral", "equilatero") ;

  menu_triangle = IupMenu (item_equilateral, item_isoceles, item_scalenus, NULL) ;
  submenu_triangle = IupSubmenu ("Triangle", menu_triangle) ;
  item_line = IupItem ("Line", NULL) ;
  item_circle = IupItem ("Circle", NULL) ;

  menu_create = IupMenu (item_line, item_circle, submenu_triangle, NULL) ;
  submenu_create = IupSubmenu ("Create", menu_create) ;
  item_help = IupItem ("Help", "item_help_act") ;
  menu_file = IupMenu (item_new, item_open, item_close, IupSeparator(), item_exit, NULL) ;
  menu_edit = IupMenu (item_copy, item_paste, IupSeparator(), submenu_create, NULL) ;
  menu_help = IupMenu (item_help, NULL);
            
  submenu_file = IupSubmenu ("File", menu_file) ;
  submenu_edit = IupSubmenu ("Edit", menu_edit) ;
  submenu_help = IupSubmenu ("Help", menu_help) ;

  menu = IupMenu (submenu_file, submenu_edit, submenu_help, NULL) ;

  IupSetAttribute(menu, "HIGHLIGHT_CB", "selection");
  IupSetAttribute(menu, IUP_OPEN_CB, "selection");
  
  IupSetHandle ("menu", menu) ;

  dlg = IupDialog (IupVbox(text, IupButton("Naada", "nadda"), NULL)) ;
  IupSetAttributes (dlg, "TITLE=\"IupSubmenu Example\", SIZE = QUARTERxEIGHTH") ;
  IupSetAttribute(dlg, "MENU", "menu");  
  IupShowXY (dlg, IUP_CENTER, IUP_CENTER) ;

  IupMainLoop () ;
  IupDestroy(dlg);
  IupClose () ;  
  return 0 ;
}





