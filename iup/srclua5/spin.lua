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
}

function ctrl.createElement(class, param)
   return Spin(param.action)
end

iupRegisterWidget(ctrl)
iupSetClass(ctrl, "iup widget")
