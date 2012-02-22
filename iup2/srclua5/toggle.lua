------------------------------------------------------------------------------
-- Toggle class 
------------------------------------------------------------------------------
local ctrl = {
  nick = "toggle",
  parent = iup.WIDGET,
  creation = "S-",
  callback = {
    action = "n",
  }
} 

function ctrl.createElement(class, arg)
  return iup.Toggle(arg.title)
end
   
iup.RegisterWidget(ctrl)
iup.SetClass(ctrl, "iup widget")
