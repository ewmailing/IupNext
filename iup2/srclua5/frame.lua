------------------------------------------------------------------------------
-- Frame class 
------------------------------------------------------------------------------
local ctrl = {
  nick = "frame",
  parent = iup.WIDGET,
  creation = "i",
  callback = {}
}

function ctrl.createElement(class, arg)
   return iup.Frame(arg[1])
end

iup.RegisterWidget(ctrl)
iup.SetClass(ctrl, "iup widget")
