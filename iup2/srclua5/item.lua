------------------------------------------------------------------------------
-- Item class 
------------------------------------------------------------------------------
local ctrl = {
  nick = "item",
  parent = iup.WIDGET,
  creation = "S-",
  callback = {
    action = "", 
    highlight_cb = "",
  }
} 

function ctrl.createElement(class, arg)
   return iup.Item(arg.title)
end
   
iup.RegisterWidget(ctrl)
iup.SetClass(ctrl, "iup widget")
