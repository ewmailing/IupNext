------------------------------------------------------------------------------
-- Expander class 
------------------------------------------------------------------------------
local ctrl = {
  nick = "expander",
  parent = iup.WIDGET,
  creation = "I",
  callback = {
    action = "", 
    openclose_cb = "n", 
    extrabutton_cb = "nn", 
  }
}

function ctrl.createElement(class, param)
  return iup.Expander(param[1])
end

iup.RegisterWidget(ctrl)
iup.SetClass(ctrl, "iup widget")
