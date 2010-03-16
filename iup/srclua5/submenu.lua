------------------------------------------------------------------------------
-- Submenu class 
------------------------------------------------------------------------------
local ctrl = {
  nick = "submenu",
  parent = WIDGET,
  creation = "SI",
  callback = {
--    open_cb = "",       -- already registered by the menu
--    menuclose_cb = "",  -- already registered by the menu
  }
} 

function ctrl.createElement(class, param)
  return Submenu(param.title, param[1])
end
   
iupRegisterWidget(ctrl)
iupSetClass(ctrl, "iup widget")
