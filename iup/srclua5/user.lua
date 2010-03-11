
------------------------------------------------------------------------------
-- User class 
------------------------------------------------------------------------------
local ctrl = {
  nick = "user",
  parent = WIDGET,
  creation = "",
  callback = {}
}

function ctrl.createElement(class, param)
   return User()
end

iupRegisterWidget(ctrl)
iupSetClass(ctrl, "iup widget")
