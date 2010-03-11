------------------------------------------------------------------------------
-- Frame class 
------------------------------------------------------------------------------
local ctrl = {
  nick = "frame",
  parent = WIDGET,
  creation = "i",
  callback = {}
}

function ctrl.createElement(class, param)
   return Frame(param[1])
end

iupRegisterWidget(ctrl)
iupSetClass(ctrl, "iup widget")
