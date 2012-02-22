------------------------------------------------------------------------------
-- List class 
------------------------------------------------------------------------------
local ctrl = {
  nick = "list",
  parent = iup.WIDGET,
  creation = "-",
  callback = {
     action = "snn", 
     multiselect_cb = "s",
     edit_cb = "ns",
   }
} 

function ctrl.createElement(class, arg)
   return iup.List()
end
   
iup.RegisterWidget(ctrl)
iup.SetClass(ctrl, "iup widget")
