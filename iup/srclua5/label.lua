------------------------------------------------------------------------------
-- Label class 
------------------------------------------------------------------------------
local ctrl = {
  nick = "label",
  parent = WIDGET,
  creation = "S",
  callback = {}
}

function ctrl.createElement(class, param)
   return Label(param.title)
end

iupRegisterWidget(ctrl)
iupSetClass(ctrl, "iup widget")
