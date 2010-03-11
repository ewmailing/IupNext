------------------------------------------------------------------------------
-- VBox class 
------------------------------------------------------------------------------
local ctrl = {
  nick = "vbox",
  parent = BOX,
  creation = "-",
  callback = {}
}

function ctrl.append (handle, elem)
  Append(handle, elem)
end

function ctrl.createElement(class, param)
   return Vbox()
end

iupRegisterWidget(ctrl)
iupSetClass(ctrl, "iup widget")
