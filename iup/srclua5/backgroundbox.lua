------------------------------------------------------------------------------
-- BackgroundBox class 
------------------------------------------------------------------------------
local ctrl = {
  nick = "backgroundbox",
  parent = iup.WIDGET,
  creation = "I",
  funcname = "BackgroundBox",
  callback = {}
}

function ctrl.createElement(class, param)
   return iup.Background(param[1])
end

iup.RegisterWidget(ctrl)
iup.SetClass(ctrl, "iup widget")
