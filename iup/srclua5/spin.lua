------------------------------------------------------------------------------
-- Spin class 
------------------------------------------------------------------------------
local ctrl = {
  nick = "spin",
  parent = WIDGET,
  creation = "",
  callback = {
    spin_cb = "n",
  },
  include = "iupspin.h"
}

function ctrl.createElement(class, arg)
   return Spin(arg.action)
end

iupRegisterWidget(ctrl)
iupSetClass(ctrl, "iup widget")
