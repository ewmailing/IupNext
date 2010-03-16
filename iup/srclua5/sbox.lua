------------------------------------------------------------------------------
-- Sbox class 
------------------------------------------------------------------------------
local ctrl = {
  nick = "sbox",
  parent = WIDGET,
  creation = "I",
  callback = {}
}

function ctrl.createElement(class, param)
   return Sbox(param[1])
end

iupRegisterWidget(ctrl)
iupSetClass(ctrl, "iup widget")
