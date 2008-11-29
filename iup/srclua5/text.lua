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
  }
}

function ctrl.createElement(class, arg)
   return Text()
end

iupRegisterWidget(ctrl)
iupSetClass(ctrl, "iup widget")
