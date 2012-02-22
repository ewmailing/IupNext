------------------------------------------------------------------------------
-- Spin class 
------------------------------------------------------------------------------
local ctrl = {
  nick = "spin",
  parent = iup.WIDGET,
  creation = "",
  callback = {
    spin_cb = "n",
  },
  include = "iupspin.h",
}

function ctrl.createElement(class, arg)
   return iup.Spin(arg.action)
end

iup.RegisterWidget(ctrl)
iup.SetClass(ctrl, "iup widget")
