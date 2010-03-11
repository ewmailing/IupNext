------------------------------------------------------------------------------
-- Val class 
------------------------------------------------------------------------------
local ctrl = {
  nick = "val",
  parent = WIDGET,
  creation = "s",
  callback = {
    mousemove_cb = "d",
    button_press_cb = "d",
    button_release_cb = "d",
  },
}

function ctrl.createElement(class, param)
   return Val(param[1])
end

iupRegisterWidget(ctrl)
iupSetClass(ctrl, "iup widget")
