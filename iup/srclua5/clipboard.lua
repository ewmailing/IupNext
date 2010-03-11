------------------------------------------------------------------------------
-- Clipboard class 
------------------------------------------------------------------------------
local ctrl = {
  nick     = "clipboard",
  parent   = WIDGET,
  creation = "",
  callback = {
  },
} 

function ctrl.createElement(class, param)
  return Clipboard()
end
   
iupRegisterWidget(ctrl)
iupSetClass(ctrl, "iup widget")
