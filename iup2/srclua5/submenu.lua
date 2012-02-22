------------------------------------------------------------------------------
-- Submenu class 
------------------------------------------------------------------------------
local ctrl = {
  nick = "submenu",
  parent = iup.WIDGET,
  creation = "Si",
  callback = {
--    open_cb = "",       -- already registered by the menu
--    menuclose_cb = "",  -- already registered by the menu
  }
} 

function ctrl.createElement(class, arg)
  return iup.Submenu(arg.title, arg[1])
end
   
iup.RegisterWidget(ctrl)
iup.SetClass(ctrl, "iup widget")
