------------------------------------------------------------------------------
-- Text class 
------------------------------------------------------------------------------
local ctrl = {
  nick = "text",
  parent = WIDGET,
  creation = "-", 
  callback = {
    action = "ns",
    caret_cb = "nnn", 
    valuechanged_cb = "",  -- used by many other controls
  }
}

function ctrl.createElement(class, param)
   return Text()
end

iupRegisterWidget(ctrl)
iupSetClass(ctrl, "iup widget")
