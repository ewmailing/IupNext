------------------------------------------------------------------------------
-- Button class 
------------------------------------------------------------------------------
local ctrl = {
  nick = "button",
  parent = WIDGET,
  creation = "S-",
  callback = {
    action = "", 
  }
} 

function ctrl.createElement(class, param)
  return Button(param.title)
end

iupRegisterWidget(ctrl)
iupSetClass(ctrl, "iup widget")
