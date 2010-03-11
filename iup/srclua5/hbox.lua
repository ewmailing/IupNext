------------------------------------------------------------------------------
-- HBox class 
------------------------------------------------------------------------------
local ctrl = {
  nick = "hbox",
  parent = BOX,
  creation = "-",
  callback = {}
}

function ctrl.append(handle, elem)
  Append(handle, elem)
end

function ctrl.createElement(class, param)
   return Hbox()
end

iupRegisterWidget(ctrl)
iupSetClass(ctrl, "iup widget")
