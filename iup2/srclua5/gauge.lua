------------------------------------------------------------------------------
-- Gauge class 
------------------------------------------------------------------------------
local ctrl = {
  nick = "gauge",
  parent = WIDGET,
  creation = "",
  callback = {},
  include = "iupgauge.h",
}

function ctrl.createElement(class, arg)
   return Gauge(arg.action)
end

iupRegisterWidget(ctrl)
iupSetClass(ctrl, "iup widget")
