------------------------------------------------------------------------------
-- Sbox class 
------------------------------------------------------------------------------
local ctrl = {
	nick = "sbox",
  parent = WIDGET,
	creation = "i",
  callback = {}
}

function ctrl.createElement(class, param)
   return Sbox(param[1])
end

iupRegisterWidget(ctrl)
iupSetClass(ctrl, "iup widget")
