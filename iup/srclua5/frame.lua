------------------------------------------------------------------------------
-- Frame class 
------------------------------------------------------------------------------
local ctrl = {
  nick = "frame",
  parent = WIDGET,
  creation = "I",
  callback = {}
}

function ctrl.createElement(class, param)
   return Frame(param[1])
end

iupRegisterWidget(ctrl)
iupSetClass(ctrl, "iup widget")
