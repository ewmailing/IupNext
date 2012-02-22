------------------------------------------------------------------------------
-- ColorBrowser class 
------------------------------------------------------------------------------
local ctrl = {
  nick = "colorbrowser",
  parent = iup.WIDGET,
  creation = "",
  callback = {
    drag_cb = "ccc",
    change_cb = "ccc",
  },
  funcname = "ColorBrowser",
  include = "iupcb.h",
}

function ctrl.createElement(class, arg)
   return iup.ColorBrowser(arg.action)
end

iup.RegisterWidget(ctrl)
iup.SetClass(ctrl, "iup widget")
