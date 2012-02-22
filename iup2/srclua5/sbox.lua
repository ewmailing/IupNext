------------------------------------------------------------------------------
-- Sbox class 
------------------------------------------------------------------------------
local ctrl = {
	nick = "sbox",
  parent = iup.WIDGET,
	creation = "i",
  callback = {},
  include="iupsbox.h"
}

function ctrl.createElement(class, arg)
   return iup.Sbox(arg[1])
end

iup.RegisterWidget(ctrl)
iup.SetClass(ctrl, "iup widget")
