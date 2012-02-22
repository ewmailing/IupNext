------------------------------------------------------------------------------
-- Label class 
------------------------------------------------------------------------------
local ctrl = {
  nick = "label",
  parent = iup.WIDGET,
  creation = "S",
  callback = {}
}

function ctrl.createElement(class, arg)
   return iup.Label(arg.title)
end

iup.RegisterWidget(ctrl)
iup.SetClass(ctrl, "iup widget")
