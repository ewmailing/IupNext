------------------------------------------------------------------------------
-- BackgroundBox class 
------------------------------------------------------------------------------
local ctrl = {
  nick = "backgroundbox",
  parent = iup.WIDGET,
  subdir = "elem",
  creation = "I",
  funcname = "BackgroundBox",
  callback = {
    action = "ff",
    }
}

function ctrl.createElement(class, param)
   return iup.BackgroundBox(param[1])
end

iup.RegisterWidget(ctrl)
iup.SetClass(ctrl, "iup widget")
