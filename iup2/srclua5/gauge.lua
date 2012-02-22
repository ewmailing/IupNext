------------------------------------------------------------------------------
-- Gauge class 
------------------------------------------------------------------------------
local ctrl = {
  nick = "gauge",
  parent = iup.WIDGET,
  creation = "",
  callback = {},
  include = "iupgauge.h",
}

function ctrl.createElement(class, arg)
   return iup.Gauge(arg.action)
end

iup.RegisterWidget(ctrl)
iup.SetClass(ctrl, "iup widget")
