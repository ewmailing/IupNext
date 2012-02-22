------------------------------------------------------------------------------
-- Multiline class 
------------------------------------------------------------------------------
local ctrl = {
  nick = "multiline",
  parent = iup.WIDGET,
  creation = "-",
  callback = {
    action = "ns", 
  },
  funcname = "MultiLine",
} 

function ctrl.createElement(class, arg)
   return iup.MultiLine()
end
   
iup.RegisterWidget(ctrl)
iup.SetClass(ctrl, "iup widget")
