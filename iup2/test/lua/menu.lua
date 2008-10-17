--IupMenu Example in IupLua 
--Creates a dialog with a menu with two submenus. 

-- IupMenu example

-- Creates a text, sets its value and turns on text readonly mode 
text = iup.text {readonly = "YES", value = "Selecting show or hide will affect this text"}

-- Creates items, sets its shortcut keys and deactivates edit item
item_show = iup.item {title = "Show", key = "S"}
item_hide = iup.item {title = "Hide", key = "H"}
item_edit = iup.item {title = "Edit", key = "E", active = "NO"}
item_exit = iup.item {title = "Exit", key = "x"}

-- Creates two menus
menu_file = iup.menu {item_exit}
menu_text = iup.menu {item_show, item_hide, item_edit}

-- Creates two submenus
submenu_file = iup.submenu {menu_file; title = "File"}
submenu_text = iup.submenu {menu_text; title = "Text"}

-- Creates main menu with two submenus
menu = iup.menu {submenu_file, submenu_text}
menu.menuselect_cb = function(self)
  print("CHAMOU")
  print(self.title)
end
                                
-- Creates dialog with a text, sets its title and associates a menu to it 
dlg = iup.dialog {text
      ; title ="IupMenu Example", menu = menu}

-- Shows dialog in the center of the screen 
dlg:showxy (iup.CENTER,iup.CENTER)

function item_show:action ()
  text.visible = "YES"
  
  return IUP_DEFAULT
end

function item_hide:action ()
  text.visible = "NO"
 end

function item_exit:action ()
  dlg:destroy()
end

