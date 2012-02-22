------------------------------------------------------------------------------
-- Timer class 
------------------------------------------------------------------------------
local ctrl = {
  nick     = "timer",
  parent   = iup.WIDGET,
  creation = "",
  callback = {
    action_cb = "", 
  },
} 

function ctrl.createElement(class, arg)
  return iup.Timer()
end
   
iup.RegisterWidget(ctrl)
iup.SetClass(ctrl, "iup widget")
