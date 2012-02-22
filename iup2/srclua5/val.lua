------------------------------------------------------------------------------
-- Val class 
------------------------------------------------------------------------------
local ctrl = {
  nick = "val",
  parent = iup.WIDGET,
  creation = "s",
  callback = {
    mousemove_cb = "d",
    button_press_cb = "d",
    button_release_cb = "d",
  },
  include = "iupval.h",
}

function ctrl.createElement(class, arg)
   return iup.Val(arg[1])
end

iup.RegisterWidget(ctrl)
iup.SetClass(ctrl, "iup widget")
