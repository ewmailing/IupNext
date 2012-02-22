------------------------------------------------------------------------------
-- SpinBox class 
------------------------------------------------------------------------------
local ctrl = {
  nick = "spinbox",
  parent = iup.WIDGET,
  creation = "i",
  callback = {
    spin_cb = "n",
  },
  include = "iupspin.h",
}

function ctrl.createElement(class, arg)
   return iup.Spinbox(arg[1])
end

iup.RegisterWidget(ctrl)
iup.SetClass(ctrl, "iup widget")
