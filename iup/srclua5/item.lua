------------------------------------------------------------------------------
-- Item class 
------------------------------------------------------------------------------
local ctrl = {
  nick = "item",
  parent = WIDGET,
  creation = "S-",
  callback = {
    action = "", 
    highlight_cb = "",
  }
} 

function ctrl.createElement(class, param)
   return Item(param.title)
end
   
iupRegisterWidget(ctrl)
iupSetClass(ctrl, "iup widget")
