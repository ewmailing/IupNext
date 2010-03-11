
------------------------------------------------------------------------------
-- Normalizer class 
------------------------------------------------------------------------------
local ctrl = {
  nick = "normalizer",
  parent = WIDGET,
  creation = "-",
  callback = {}
}

function ctrl.setAttributes(object, param)
  local handle = rawget(object, "handle")
  local n = #param
  for i = 1, n do
    if iupGetClass(param[i]) == "iup handle" then 
      object.addcontrol = param[i]
    end
  end
  WIDGET.setAttributes(object, param)
end

function ctrl.createElement(class, param)
   return Normalizer()
end

iupRegisterWidget(ctrl)
iupSetClass(ctrl, "iup widget")
