------------------------------------------------------------------------------
-- Gauge class 
------------------------------------------------------------------------------
local ctrl = {
  nick = "gauge",
  parent = WIDGET,
  creation = "",
  callback = {},
  include = "iupcontrols.h",
}

function ctrl.createElement(class, param)
   return Gauge(param.action)
end

iupRegisterWidget(ctrl)
iupSetClass(ctrl, "iup widget")
