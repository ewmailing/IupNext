------------------------------------------------------------------------------
-- Separator class 
------------------------------------------------------------------------------
local ctrl = {
  nick = "separator",
  parent = WIDGET,
  creation = "",
  callback = {}
}

function ctrl.createElement(class, param)
   return Separator()
end

iupRegisterWidget(ctrl)
iupSetClass(ctrl, "iup widget")
